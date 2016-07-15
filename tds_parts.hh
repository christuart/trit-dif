#ifndef TDS_PARTS_HH
#define TDS_PARTS_HH

#include "vector_ops.hh"

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

class tds_node {
public:
private:
	std::vector<float> position_;
	tds_elements elements_;
	// used during construction only, because links can't be made
	// until both elements exist, so the node needs to gradually
	// learn which elements it is linked to.
protected:
public:
	tds_node(float _x, float _y, float _z);
	tds_node(std::vector<float> _position);
	~tds_node();
	//adders
	void add_element(tds_element* new_element);
	//setters
	inline void position(std::vector<float> _position) { position_ = _position; }
	inline void position(int i, float _p) { position_[i] = _p; }
	//getters
	inline std::vector<float> position() { return position_; }
	inline float position(int i) { return position_[i]; }
	inline tds_element& element(int i) { return *elements_[i]; }
	inline bool elements_empty() { return elements_.empty(); }
	inline int n_elements() { return elements_.size(); }
	void clean_elements();
	void remove_last_element();
};

class tds_element {
public:
private:
	// size_ may be a length, area or volume depending on what dimension mode we are in
	float size_,contaminationA_,contaminationB_;
	bool flagAB_;
	tds_material* material_;
	tds_nodes nodes_;
	tds_links neighbours_;
	std::vector<float> origin_;
protected:
public:
	tds_element(tds_nodes _nodes, tds_material* _material, float _contamination);
	tds_element(tds_nodes _nodes, tds_material* _material, std::vector<float> _origin, float _contamination);
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
	inline std::vector<float>& origin() { return origin_; }
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
	std::vector<float> norm_vector_, flux_vector_;
	float interface_area_, flow_rate_, modMN_, a_n_dot_eMN_over_modMN_;
	bool flagAB_;
protected:
	//setters
	inline void norm_vector(std::vector<float>& _norm_vector) { norm_vector(_norm_vector.at(0),_norm_vector.at(1),_norm_vector.at(2)); }
	inline void norm_vector(float _x, float _y, float _z) { norm_vector_.resize(3);
		                                                norm_vector(0,_x);
		                                                norm_vector(1,_y);
		                                                norm_vector(2,_z); }
	inline void norm_vector(int i, float _f) { norm_vector_[i]=_f; }
	inline void flux_vector(std::vector<float>& _flux_vector) { flux_vector(_flux_vector[0],_flux_vector[1],_flux_vector[2]); }
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
	inline std::vector<float>& norm_vector() { return norm_vector_; }
	inline float norm_vector(int i) { return norm_vector_[i]; }
	inline std::vector<float>& flux_vector() { return flux_vector_; }
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

#endif
