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

#endif
