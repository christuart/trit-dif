#include "tds_parts.hh"

/******************** TDS_NODE METHODS ********************/
tds_node::tds_node(float _x, float _y, float _z) {
	position_.reserve(3);
	position_.push_back(_x);
	position_.push_back(_y);
	position_.push_back(_z);
}
tds_node::~tds_node() {
}

void tds_node::add_element(tds_element* new_element) {
	std::cout<<"adding new element for node"<<std::endl;
	// elements_.push_front(new_element);
	elements_.push_back(new_element);
}
void tds_node::clean_elements() {
	// while (!elements_empty()) elements_.pop_front();
}
void tds_node::remove_last_element() {
	elements_.pop_back();
}

