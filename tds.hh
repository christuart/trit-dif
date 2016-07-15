#ifndef TDS_HH
#define TDS_HH

#include <list>
#include <iostream>
#include "utilities.hh"
#include "gui.h"
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include <stdint.h>
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include <forward_list>
#include <bitset>
#include <algorithm>

#include "conversions.hh"
#include "tds_parts.hh"
#include "vector_ops.hh"

class tds;
class tds_run;
class tds_display;
class tds_batch;

const int one_d = 0;
const int two_d = 1;
const int three_d = 2;
const int second_order_or_worse = 3;


class tds_element {
public:
private:
	// size_ may be a length, area or volume depending on what dimension mode we are in
	float size_,contaminationA_,contaminationB_;
	bool flagAB_;
	tds_material* material_;
	tds_nodes nodes_;
	tds_links neighbours_;
	vector<float> origin_;
protected:
public:
	tds_element(tds_nodes _nodes, tds_material* _material, float _contamination);
	tds_element(tds_nodes _nodes, tds_material* _material, vector<float> _origin, float _contamination);
	tds_element(tds_nodes _nodes, tds_material* _material, float _origin_x, float _origin_y, float _origin_z, float _contamination);
	virtual ~tds_element();
	//adders
	void add_element_link(tds_element_link* new_element_link);
	//setters
	inline void flagAB(bool _AB) { flagAB_ = _AB; }
	inline void size(float _size) { size_=_size; }
	inline void contamination(float _contamination) { if(flagAB()) contaminationA_=_contamination; else contaminationB_=_contamination;}
	inline void origin(float _x, float _y, float _z) { origin_.clear(); origin_.push_back(_x);
		                                           origin_.push_back(_y); origin_.push_back(_z);  }
	//getters
	inline bool flagAB() { return flagAB_; }
	inline float size() { return size_; }
	virtual inline float contamination() { if (flagAB()) return contaminationB_; else return contaminationA_; }
	virtual inline float contamination(bool _AB) { if (_AB) return contaminationB_; else return contaminationA_; }
	inline float origin(int i) { return origin_[i]; }
	inline vector<float>& origin() { return origin_; }
	inline tds_material& material() { return *material_; }
	inline tds_node& node(int i) { return *nodes_[i]; }
	inline int n_nodes() { return nodes_.size(); }
	inline tds_element_link& neighbour(int i) { return *neighbours_[i]; }
	inline int n_neighbours() { return neighbours_.size(); }

	// Will add the correct quantity to contamination and switch flag
	void transfer_contaminant(float _quantity);
	// Finds the centre of mass (at present) to be used in calculating distances between elements
	void set_origin_from_nodes();
	// Will calculate and apply the total amounts to be transferred to the element
	// This is virtual in order to allow different types of element to be of child
	// classes in the future (e.g. to clean out the basic update() function and have
	// class tds_decaying_element : public tds_element {
	//         virtual void update(float delta_t) {
	//                  tds_element::update(delta_t);
	//                  do_decay(delta_T);
	//         }
	// }
	// or to have a tds_source_element that has non constant value or that records much
	// how tritium entered the material or outgassing that records how much left etc.
	virtual void update(float delta_t);
	// gives each node in the element a reference to it, so that element links may later be built
	void propogate_into_nodes();
	// calculates and stores the length or area of volume of the element in size_
	void calculate_size();

	void debug_contamination();
};

class tds_material {
public:
private:
	std::string material_name_, material_density_unit_, material_diffusion_constant_unit_;
	float material_density_, material_diffusion_constant_;
protected:
public:
	tds_material(std::string _name, float _density, float _diffusion_constant);
	virtual ~tds_material();
	//setters
	inline void density(float _density) { material_density_ = _density; }
	inline void diffusion_constant(float _diffusion_constant) { material_diffusion_constant_ = _diffusion_constant; }
	inline void material_density_unit(std::string _unit) { material_density_unit_ = _unit; }
	inline void material_diffusion_constant_unit(std::string _unit) { material_diffusion_constant_unit_ = _unit; }
	//getters
	inline float density() { return material_density_; }
	inline float diffusion_constant() { return material_diffusion_constant_; }
	inline std::string material_density_unit() { return material_density_unit_; }
	inline std::string material_diffusion_constant_unit() { return material_diffusion_constant_unit_; }
	inline std::string name() { return material_name_; }
	inline bool is_source() { return material_name_=="source"; }
};


	
class tds_section {
public: 
private:
	tds_elements elements_;
	tds_material* material_;
protected:
public:
	tds_section(tds_material* _material);
	virtual ~tds_section();
	//adders
	void add_element(tds_element* new_element);
	//cleaners
	void clean_elements();
	//setters
	inline void material(tds_material* _material) { material_ = _material; }
	//getters
	inline tds_material& material() { return *material_; }
	inline tds_element& element(int i) { return *elements_[i]; }
	inline int n_elements() { return elements_.size(); }
};

// each element has a list of neighbours, the cells that they exchange with via diffusion
// these neighbours are of class tds_element_link. each tds_element_link contains two elements,
// and refers to a specific direction of flow, positive when going from N into M. This means that
// you halve the number of link instances, but need to check whether the updating element is
// stored as M or N with the method "bool positive_flow(tds_element* whoami);"

class tds_element_link {
public:
private:
	tds_element *elementM_, *elementN_; // note that each pointer needs its own asterisk!
	tds_nodes shared_nodes_;
	vector<float> norm_vector_, flux_vector_;
	float interface_area_, flow_rate_, modMN_, a_n_dot_eMN_over_modMN_;
	bool flagAB_;
protected:
	//setters
	inline void norm_vector(vector<float>& _norm_vector) { norm_vector(_norm_vector.at(0),_norm_vector.at(1),_norm_vector.at(2)); }
	inline void norm_vector(float _x, float _y, float _z) { norm_vector_.resize(3);
		                                                norm_vector(0,_x);
		                                                norm_vector(1,_y);
		                                                norm_vector(2,_z); }
	inline void norm_vector(int i, float _f) { norm_vector_[i]=_f; }
	inline void flux_vector(vector<float>& _flux_vector) { flux_vector(_flux_vector[0],_flux_vector[1],_flux_vector[2]); }
	inline void flux_vector(int i, float _f) { flux_vector_[i]=_f; }
	inline void flux_vector(float _x, float _y, float _z) { flux_vector_.resize(3);
		                                                flux_vector(0,_x);
		                                                flux_vector(1,_y);
		                                                flux_vector(2,_z); }
	inline void interface_area(float _area) { interface_area_ = _area; }
	inline void modMN(float _modMN) { modMN_ = _modMN; }
	inline void a_n_dot_eMN_over_modMN(float _f) { a_n_dot_eMN_over_modMN_ = _f; }
	inline void flagAB(bool _AB) { flagAB_ = _AB; }
public:
	tds_element_link(tds_element* _M, tds_element* _N);
	virtual ~tds_element_link();
	void initialise();

	//setters
	void set_flag_against(tds_element* _element);
	//getters
	inline vector<float>& norm_vector() { return norm_vector_; }
	inline float norm_vector(int i) { return norm_vector_[i]; }
	inline vector<float>& flux_vector() { return flux_vector_; }
	inline float flux_vector(int i) { return flux_vector_[i]; }
	inline float interface_area() { return interface_area_; }
	inline float modMN() { return modMN_; }
	inline float a_n_dot_eMN_over_modMN() { return a_n_dot_eMN_over_modMN_; }
	inline tds_node& shared_node(int i) { return *shared_nodes_[i]; }
	inline tds_element& elementM() { return *elementM_; }
	inline tds_element& elementN() { return *elementN_; }
	float diffusion_constant();
	virtual float flow_rate(bool _AB); // virtual: may well want to do some kind of convection-analogue at the surfaces or something
	inline bool flagAB() { return flagAB_; }
	// positive flow is defined as from N to M, so M's contamination rises
	inline short positive_flow(tds_element* whoami) { if (elementM_ == whoami) return 1; else return -1; }
	inline tds_element* element(bool m_or_n) { if (m_or_n) return elementN_; else return elementM_; }
	inline tds_element* neighbour_of(tds_element* whoami) { if (elementM_ == whoami) return elementN_; else return elementM_; }
	
};

class tds {
public: 
private:
	tds_sections sections_;
	tds_materials materials_;
	tds_nodes nodes_;
	tds_elements elements_;
	conversion* units_;
	std::bitset<8> element_dimensions_;

	std::map<std::string,tds_material*> material_map_;
	
	std::string tds_name_,size_unit_,size_x_unit_,size_y_unit_,size_z_unit_;
	float size_,size_x_,size_y_,size_z_;
	int dimensions;

protected:
public:
	tds();
	virtual ~tds();
	
	//adders
	void add_section(tds_section* new_section);
	void add_material(tds_material* new_material);
	void add_node(tds_node* new_node);
	void add_element(tds_element* new_element);
	//cleaners
	void clean_sections();
	void clean_materials();
	void clean_nodes();
	void clean_elements();
	
	//setters
	inline void tds_name(std::string tds_n) { tds_name_=tds_n; }
	inline void size_unit(std::string sz_unit){ size_unit_=sz_unit; }
	inline void size(float sz){ size_=sz; }
	inline void size_x_unit(std::string sz_x_unit){ size_x_unit_=sz_x_unit; }
	inline void size_x(float sz_x){ size_x_=sz_x; }
	inline void size_y_unit(std::string sz_y_unit){ size_y_unit_=sz_y_unit; }
	inline void size_y(float sz_y){ size_y_=sz_y; }
	inline void size_z_unit(std::string sz_z_unit){ size_z_unit_=sz_z_unit; }
	inline void size_z(float sz_z){ size_z_=sz_z; }
	inline void units(conversion* _units) { units_ = _units; }
	void expected_materials(int _n);
	void expected_sections(int _n);
	void expected_nodes(int _n);
	void expected_elements(int _n);
	//getters
	inline std::string tds_name() { return tds_name_; }
	inline std::string size_unit() { return size_unit_; }
	inline float size() { return size_; }
	inline float size_x() { return size_x_; }
	inline float size_y() { return size_y_; }
	inline float size_z() { return size_z_; }
	inline std::string size_x_unit() { return size_x_unit_; }
	inline std::string size_y_unit() { return size_y_unit_; }
	inline std::string size_z_unit() { return size_z_unit_; }
	inline conversion& units() { return *units_; }
	inline int n_sections() { return sections_.size(); }
	inline tds_section& section(int i) { return *sections_[i]; }
	inline int n_materials() { return materials_.size(); }
	inline tds_material& material(int i) { return *materials_[i]; }
	inline tds_material& material(std::string s) {
		std::map<std::string,tds_material*>::iterator _m = material_map_.find(s);
		if (_m != material_map_.end()) {
			return *(_m->second);
		} else {
			std::cerr << "Undefined material '" << s << "' replaced with 'error'" << std::endl;
			return *material_map_["error"];
		}
	}
	inline int n_nodes() { return nodes_.size(); }
	inline tds_node& node(int i) { return *nodes_[i]; }
	inline int n_elements() { return elements_.size(); }
	inline tds_element& element(int i) { return *elements_[i]; }
	inline bool element_dimensions(int bit_number) { return element_dimensions_.test(bit_number); }
	inline bitset<8> element_dimensions() { return element_dimensions_; }

	void register_element_type(int element_type);
	void output_model_summary(bool show_materials, bool show_sections, bool show_elements, bool show_element_links, bool show_nodes);
};

class tds_run: public tds {
public:
private:
	ifstream materialsfile_;
	ifstream sectionsfile_;
	ifstream nodesfile_;
	ifstream elementsfile_;
	ifstream sourcefile_;
	ifstream simparamsfile_;
	ofstream trackingfile_;
	ofstream contaminationsfile_;
	std::string basename_;
	std::string configname_;
	std::string outputname_;
protected:
public:
	tds_run();
	virtual ~tds_run();
	void check_coincidence();
	void make_analysis(float delta_t, int _steps, float recording_interval, vector<int>& tracked_elements);
	void initialise();
	//setters
	inline void basename(std::string _basename) { basename_ = _basename; };
	inline void configname(std::string _configname) { configname_ = _configname; };
	inline void outputname(std::string _outputname) { outputname_ = _outputname; };
	//getters
	inline std::string basename() { return basename_; };
	inline std::string configname() { return configname_; };
	inline std::string outputname() { return outputname_; };
};

class tds_display: public tds_run {
private:
	UserInterface *GUI_;
	vector<double> X_, Y_;//for the timelines
	vector<string> text_;
	const char *filename_;
	Fl_Text_Buffer FRootfileComments;
	Fl_Text_Buffer FRootfileName;
	Fl_Text_Buffer TimelineComment;
	std::string e_info_,tl_info_;
protected:
	void dialog_open();
	void load_event();
	void load_section(int chnum);
	void load_section(unsigned int c, int chnum);
	void resize_plot(int section);
	void makeZoomBox(selection sel,int event,int section);
public:
	tds_display(UserInterface *gui);
	virtual ~tds_display();
	inline UserInterface &GUI(){ return *GUI_; }
	void action(Fl_Widget *sender);
	void action(selection sel, Fl_Widget *sender);
	void plot();
	//Setters
	inline void display_e_info(std::string e_comment) { e_info_=e_comment; }
	inline void display_tl_info(std::string tl_comment) { tl_info_=tl_comment; }
	inline void filename(const char *f_name) { filename_=f_name; }
	//Getters
	inline std::string display_e_info() { return e_info_; }
	inline std::string display_tl_info() { return tl_info_; }
	inline const char *filename() { return filename_; }
}; 

class tds_batch: public tds_run {
private:
	ifstream infile_;
	const char* rootfile_name_;
protected:
public:
	//getters
	inline const char* rootfile_name() { return rootfile_name_; }
	//setters
	inline void rootfile_name(const char* _rootfile_name) { rootfile_name_=_rootfile_name; }
	tds_batch(string infilename, string rootout);
	virtual ~tds_batch();
	int run_batch(string filename, bool recreate, int filechain, int n_tot);
	inline ifstream &inputfile() { return infile_; }
};

#endif
