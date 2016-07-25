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
	inline std::bitset<8> element_dimensions() { return element_dimensions_; }

	void register_element_type(int element_type);
	void output_model_summary(bool show_materials, bool show_sections, bool show_elements, bool show_element_links, bool show_nodes);
};

class tds_run: public tds {
public:
private:
	std::ifstream materialsfile_;
	std::ifstream sectionsfile_;
	std::ifstream nodesfile_;
	std::ifstream elementsfile_;
	std::ifstream sourcefile_;
	std::ifstream simparamsfile_;
	std::ofstream trackingfile_;
	std::ofstream contaminationsfile_;
	std::string basename_;
	std::string configname_;
	std::string outputname_;
	bool units_set_;
	float initial_contamination;
	float delta_t_;
	int steps_;
	float tracking_interval_;
	std::vector<int>* tracked_elements_;
	struct plugin_file {
		std::string file_name;
		bool needed_after_initialisation;
	};
	struct run_settings {
		run_settings():model_directory("models/"),
		               config_directory("config/"),
		               output_directory("output/"),
		               delta_t(60.0),
		               simulation_length(31556736.0),
		               contamination_mode_time("constant"),
		               contamination_mode_space("constant"),
		               tracking_mode("all"),
		               tracking_interval(604800.0) {
			activated_plugins.resize(0);
			plugin_files.clear();
			//tracking_list->resize(0);
		}
		std::string model_directory;
		std::string config_directory;
		std::string output_directory;
		std::string gmsh_bin_directory;
		std::string model_name;
		std::string config_name;
		std::string output_name;
		float delta_t;
		float simulation_length;
		std::string contamination_mode_time;
		std::string contamination_mode_space;
		float contamination;
		std::string contaminations_file;
		std::vector<std::string> activated_plugins;
		std::map<std::string, plugin_file> plugin_files;
		std::string tracking_mode;
		std::vector<int>* tracking_list;
		int tracking_n;
		float tracking_interval;
	} settings;
		
protected:
public:
	tds_run();
	virtual ~tds_run();
	void check_coincidence();
	void make_analysis();
	void set_units_from_file(const char* units_file_address_);
	void initialise();
	//setters
	inline void basename(std::string _basename) { settings.model_name = find_replace(settings.model_directory,"",_basename); basename_ = settings.model_directory + settings.model_name; };
	inline void configname(std::string _configname) { settings.config_name = std::string(find_replace(settings.config_directory,"",_configname)); configname_ = settings.config_directory + settings.config_name; };
	inline void outputname(std::string _outputname) { settings.output_name = find_replace(settings.output_directory,"",_outputname); outputname_ = settings.output_directory + settings.output_name; };
	inline void delta_t(float _delta_t) { settings.delta_t = _delta_t; }
	inline void steps(int _steps) { steps_ = _steps; }
	inline void tracking_interval(float _tracking_interval) { settings.tracking_interval = _tracking_interval; }
	inline void tracked_elements(std::vector<int>& _tracked_elements) { settings.tracking_list = &_tracked_elements; }
	inline void tracked_element(int i, int element) { settings.tracking_list->at(i) = element; }
	//getters
	inline std::string basename() { return settings.model_name; };
	inline std::string configname() { return settings.config_name; };
	inline std::string outputname() { return settings.output_name; };
	inline float delta_t() { return settings.delta_t; }
	inline int steps() { return steps_; }
	inline float tracking_interval() { return settings.tracking_interval; }
	inline int tracked_element(int i) { return settings.tracking_list->at(i); }
	inline std::vector<int>* tracked_elements() { return settings.tracking_list; }
	
	inline const char* units_file_address() { std::string temp = settings.config_directory + configname() + ".units"; return temp.c_str(); }
	inline const char* materials_file_address() { std::string temp = settings.config_directory + configname() + ".materials"; return temp.c_str(); }
	inline std::string sections_file_address() { std::string temp = settings.model_directory + basename() + ".sections"; return temp.c_str(); }
	inline std::string nodes_file_address() { std::string temp = settings.model_directory + basename() + ".nodes"; return temp.c_str(); }
	inline std::string elements_file_address() { std::string temp = settings.model_directory + basename() + ".elements"; return temp.c_str(); }
	inline std::string contaminations_file_address() { std::string temp = settings.output_directory + outputname() + ".contaminations"; return temp.c_str(); }
	inline std::string tracking_file_address() { std::string temp = settings.output_directory + outputname() + ".tracking"; return temp.c_str(); }


	void read_run_file(std::string run_file_name);
};

class tds_display: public tds_run {
private:
	UserInterface *GUI_;
	std::vector<double> X_, Y_;//for the timelines
	std::vector<std::string> text_;
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
	std::ifstream infile_;
	const char* rootfile_name_;
protected:
public:
	//getters
	inline const char* rootfile_name() { return rootfile_name_; }
	//setters
	inline void rootfile_name(const char* _rootfile_name) { rootfile_name_=_rootfile_name; }
	tds_batch(std::string infilename, std::string rootout);
	virtual ~tds_batch();
	int run_batch(std::string filename, bool recreate, int filechain, int n_tot);
	inline std::ifstream &inputfile() { return infile_; }
};

#endif
