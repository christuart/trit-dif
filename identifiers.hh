#ifndef IDENTIFIERS_HH
#define IDENTIFIERS_HH
class tds_material;
class tds_section;
class tds_node;
class tds_element;
class tds_element_link;

struct material_identifier {
	int material_id;
	tds_material* material;
};
struct section_identifier {
	int section_id;
	tds_section* section;
};
struct node_identifier {
	int node_id;
	tds_node* node;
};
struct element_identifier {
	int element_id;
	int section_id;
	int section_element_id;
	tds_element* element;
};
struct element_link_identifier {
	tds_element_link* element_link;
};
#endif
