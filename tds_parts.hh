#ifndef TDS_PARTS_HH
#define TDS_PARTS_HH

#include <algorithm>
#include "vector_ops.hh"
#include "exceptions.hh"
#include <float.h>
#include <sstream>

class tds_part;
class tds_node;
class tds_element;
class tds_material;
class tds_section;
class tds_element_link;

typedef std::vector<tds_material*> tds_materials;
typedef std::vector<tds_section*> tds_sections;
typedef std::vector<tds_element*> tds_elements;
typedef std::vector<tds_node*> tds_nodes;
typedef std::vector<tds_element_link*> tds_links;

class tds_part {
public:
	tds_part();
	virtual ~tds_part();
	virtual bool is_base();
};

class tds_node : public tds_part {
public:
private:
	std::vector<double> position_;
	tds_elements elements_;
	// used during construction only, because links can't be made
	// until both elements exist, so the node needs to gradually
	// learn which elements it is linked to.
protected:
public:
	tds_node(double _x, double _y, double _z);
	tds_node(std::vector<double> _position);
	virtual ~tds_node();
	//adders
	void add_element(tds_element* new_element);
	//setters
	inline void position(std::vector<double> _position) { position_ = _position; }
	inline void position(int i, double _p) {
		if (i < 3) { position_[i] = _p; }
		else {
			std::ostringstream oss; oss << "Writing dimension " << i << ". (x,y,z,???)";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	//getters
	inline std::vector<double> position() { return position_; }
	inline double position(int i) {
		if (i < 3) { return position_[i]; }
		else {
			std::ostringstream oss; oss << "Reading dimension " << i << ". (x,y,z,???)";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline tds_element& element(int i) {
		if (i < n_elements()) { return *elements_[i]; }
		else {
			std::ostringstream oss; oss << "Reading element " << i << " out of " << n_elements() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline void element(int i, tds_element* _new_element) {
		if (i < n_elements()) { elements_[i] = _new_element; }
		else {
			std::ostringstream oss; oss << "Writing element " << i << " out of " << n_elements() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline bool elements_empty() { return elements_.empty(); }
	inline int n_elements() { return elements_.size(); }
	void clean_elements();
	void remove_last_element();

	static double get_3Dplanar_area(tds_nodes shared_nodes, std::vector<double> &e_n);
};

class tds_element : public tds_part {
public:
private:
	// size_ may be a length, area or volume depending on what dimension mode we are in
	double size_,contaminationA_,contaminationB_;
	bool flagAB_;
	tds_section* section_;
	tds_material* material_;
	tds_nodes nodes_;
	tds_links neighbours_;
	std::vector<double> origin_;
protected:
public:
	tds_element(tds_nodes _nodes, tds_section* _section, double _contamination);
	tds_element(tds_nodes _nodes, tds_section* _section, std::vector<double> _origin, double _contamination);
	tds_element(tds_nodes _nodes, tds_section* _section, double _origin_x, double _origin_y, double _origin_z, double _contamination);
	virtual ~tds_element();
	//adders
	void add_element_link(tds_element_link* new_element_link);
	//setters
	inline void flagAB(bool _AB) { flagAB_ = _AB; }
	inline void size(double _size) {
		if (_size < 0) {
			throw Errors::AlgorithmFailedException("Calculated negative size.");
		} else { size_=_size; }
	}
	inline void contamination(double _contamination) {
		// should check for invalid numbers here
		if (_contamination < 0) {
			throw Errors::NegativeContaminationException();
		} else if (_contamination < DBL_MAX) {
			if(flagAB()) contaminationA_=_contamination; else contaminationB_=_contamination;
		} else {
			throw Errors::InvalidContaminationException();
		}
	}
	inline void origin(double _x, double _y, double _z) { origin_.clear(); origin_.push_back(_x);
		                                           origin_.push_back(_y); origin_.push_back(_z);  }
	inline void section(tds_section* _section) { section_=_section; }
	//getters
	inline bool flagAB() { return flagAB_; }
	inline double size() { return size_; }
	virtual inline double contamination() { if (flagAB()) return contaminationB_; else return contaminationA_; }
	virtual inline double contamination(bool _AB) { if (_AB) return contaminationB_; else return contaminationA_; }
	inline double origin(int i) {
		if (i < 3) { return origin_[i]; }
		else {
			std::ostringstream oss; oss << "Reading origin dimension " << i << ". (x,y,z,???)";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline std::vector<double>& origin() { return origin_; }
	inline tds_section& section() { return *section_; }
	inline tds_material& material() { return *material_; }
	inline tds_node& node(int i) {
		if (i < n_nodes()) { return *nodes_[i]; }
		else {
			std::ostringstream oss; oss << "Reading element node " << i << " out of " << n_nodes() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline void node(int i, tds_node* _node) {
		if (i < n_nodes()) { nodes_[i] = _node; }
		else {
			std::ostringstream oss; oss << "Writing element node " << i << " out of " << n_nodes() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline int n_nodes() { return nodes_.size(); }
	inline tds_element_link& neighbour(int i) {
		if (i < n_neighbours()) { return *neighbours_[i]; }
		else {
			std::ostringstream oss; oss << "Reading element neighbour " << i << " out of " << n_neighbours() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline void neighbour(int i, tds_element_link* _element_link) {
		if (i < n_neighbours()) { neighbours_[i] = _element_link; }
		else {
			std::ostringstream oss; oss << "Writing element neighbour " << i << " out of " << n_neighbours() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline int n_neighbours() { return neighbours_.size(); }

	// Will add the correct quantity to contamination and switch flag
	void transfer_contaminant(double _quantity);
	// Finds the centre of mass (at present) to be used in calculating distances between elements
	void set_origin_from_nodes();
	// Will calculate and apply the total amounts to be transferred to the element
	// This is virtual in order to allow different types of element to be of child
	// classes in the future (e.g. to clean out the basic update() function and have
	// class tds_decaying_element : public tds_element {
	//         virtual void update(double delta_t) {
	//                  tds_element::update(delta_t);
	//                  do_decay(delta_T);
	//         }
	// }
	// or to have a tds_source_element that has non constant value or that records much
	// how tritium entered the material or outgassing that records how much left etc.
	virtual void update(double delta_t);
	// gives each node in the element a reference to it, so that element links may later be built
	void propogate_into_nodes();
	// calculates and stores the length or area of volume of the element in size_
	void calculate_size();

	void debug_contamination();
	bool is_linked_to(tds_element* _element);
};

class tds_material : public tds_part {
public:
private:
	std::string material_name_, material_density_unit_, material_diffusion_constant_unit_;
	double material_density_, material_diffusion_constant_;
protected:
public:
	tds_material(std::string _name, double _density, double _diffusion_constant);
	virtual ~tds_material();
	//setters
	inline void density(double _density) {
		if (_density > 0.0f) { material_density_ = _density; }
		else {
			throw Errors::InvalidMaterialPropertyException("Material density should be positive (" + name() + ")");
		}
	}
	inline void diffusion_constant(double _diffusion_constant) {
		if (_diffusion_constant > 0.0f) { material_diffusion_constant_ = _diffusion_constant; }
		else {
			throw Errors::InvalidMaterialPropertyException("Material density should be positive (" + name() + ")");
		}
	}
	inline void material_density_unit(std::string _unit) { material_density_unit_ = _unit; }
	inline void material_diffusion_constant_unit(std::string _unit) { material_diffusion_constant_unit_ = _unit; }
	//getters
	inline double density() { return material_density_; }
	inline double diffusion_constant() { return material_diffusion_constant_; }
	inline std::string material_density_unit() { return material_density_unit_; }
	inline std::string material_diffusion_constant_unit() { return material_diffusion_constant_unit_; }
	inline std::string name() { return material_name_; }
	inline bool is_source() { return material_name_=="source"; }
};
class tds_section : public tds_part {
public: 
private:
	std::string name_;
	tds_elements elements_;
	tds_material* material_;
protected:
public:
	tds_section(std::string _name, tds_material* _material);
	virtual ~tds_section();
	//adders
	int add_element(tds_element* new_element);
	//cleaners
	void clean_elements();
	//setters
	inline void name(std::string _name) { name_ = _name; }
	inline void material(tds_material* _material) { material_ = _material; }
	//getters
	inline std::string name() { return name_; }
	inline tds_material& material() { return *material_; }
	inline tds_element& element(int i) { return *elements_[i]; }
	inline void element(int i, tds_element* _new_element) {
		if (i < n_elements()) { elements_[i] = _new_element; }
		else {
			std::ostringstream oss; oss << "Writing section element " << i << " out of " << n_elements() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline int n_elements() { return elements_.size(); }
};

// each element has a list of neighbours, the cells that they exchange with via diffusion
// these neighbours are of class tds_element_link. each tds_element_link contains two elements,
// and refers to a specific direction of flow, positive when going from N into M. This means that
// you halve the number of link instances, but need to check whether the updating element is
// stored as M or N with the method "bool positive_flow(tds_element* whoami);"

class tds_element_link : public tds_part {
public:
private:
protected:
	tds_element *elementM_, *elementN_; // note that each pointer needs its own asterisk!
	tds_nodes shared_nodes_;
	std::vector<double> norm_vector_, flux_vector_;
	double interface_area_, flow_rate_, modMN_, a_n_dot_eMN_over_modMN_;
	bool flagAB_;
protected:
	//setters
	inline void norm_vector(std::vector<double>& _norm_vector) { norm_vector(_norm_vector.at(0),_norm_vector.at(1),_norm_vector.at(2)); }
	inline void norm_vector(double _x, double _y, double _z) { norm_vector_.resize(3);
		                                                norm_vector(0,_x);
		                                                norm_vector(1,_y);
		                                                norm_vector(2,_z); }
	inline void norm_vector(int i, double _f) {
		if (i < 3) { norm_vector_[i]=_f; }
		else {
			std::ostringstream oss; oss << "Writing norm vector dimension " << i << ". (x,y,z,???)";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline void flux_vector(std::vector<double>& _flux_vector) { flux_vector(_flux_vector[0],_flux_vector[1],_flux_vector[2]); }
	inline void flux_vector(int i, double _f) {
		if (i < 3) { flux_vector_[i]=_f; }
		else {
			std::ostringstream oss; oss << "Writing flux vector dimension " << i << ". (x,y,z,???)";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline void flux_vector(double _x, double _y, double _z) { flux_vector_.resize(3);
		                                                flux_vector(0,_x);
		                                                flux_vector(1,_y);
		                                                flux_vector(2,_z); }
	inline void interface_area(double _area) { interface_area_ = _area; }
	inline void modMN(double _modMN) { modMN_ = _modMN; }
	inline void a_n_dot_eMN_over_modMN(double _f) { a_n_dot_eMN_over_modMN_ = _f; }
	inline void flagAB(bool _AB) { flagAB_ = _AB; }
public:
	tds_element_link(tds_element* _M, tds_element* _N);
	virtual ~tds_element_link();
	void initialise();

	//setters
	void set_flag_against(tds_element* _element);
	//getters
	inline std::vector<double>& norm_vector() { return norm_vector_; }
	inline double norm_vector(int i) {
		if (i < 3) { return norm_vector_[i]; }
		else {
			std::ostringstream oss; oss << "Reading norm vector dimension " << i << ". (x,y,z,???)";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline std::vector<double>& flux_vector() { return flux_vector_; }
	inline double flux_vector(int i) {
		if (i < 3) { return flux_vector_[i]; }
		else {
			std::ostringstream oss; oss << "Reading flux vector dimension " << i << ". (x,y,z,???)";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline double interface_area() { return interface_area_; }
	inline double modMN() { return modMN_; }
	inline double a_n_dot_eMN_over_modMN() { return a_n_dot_eMN_over_modMN_; }
	inline tds_node& shared_node(int i) { return *shared_nodes_[i]; }
	inline tds_element& elementM() { return *elementM_; }
	inline tds_element& elementN() { return *elementN_; }
	double diffusion_constant();
	virtual double flow_rate(bool _AB); // virtual: may well want to do some kind of convection-analogue at the surfaces or something
	inline bool flagAB() { return flagAB_; }
	// positive flow is defined as from N to M, so M's contamination rises
	virtual short positive_flow(tds_element* whoami);
	inline tds_element* element(bool m_or_n) { if (m_or_n) return elementN_; else return elementM_; }
	inline tds_element* neighbour_of(tds_element* whoami) { if (elementM_ == whoami) return elementN_; else if (elementN_ == whoami) return elementM_; else throw Errors::NotANeighbourException(); }
	
};

#endif
