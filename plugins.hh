#ifndef PLUGINS_HH
#define PLUGINS_HH

#include "tds.hh"

enum plugin {
	POutgassing,
	PDecay
};

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

class IPlugin {

private:
	tds_run* tds_run_;
public:
	IPlugin();
	virtual ~IPlugin();

	virtual plugin plugin_identifier();

	virtual void load_plugin();
	virtual void interrupt_material_creation(material_identifier new_material_);
	virtual void interrupt_section_creation(int section_id_, tds_section* new_section_);
	virtual void interrupt_node_creation(int node_id_, tds_node* new_node_);
	virtual void interrupt_element_creation(int element_id_, int section_id_, int section_element_id_, tds_element* new_element_);
	virtual void interrupt_element_link_creation(tds_element_link* new_element_link_);
	virtual void interrupt_pre_simulation();
	virtual void interrupt_start_step();
	virtual void interrupt_end_step();
	virtual void interrupt_post_simulation();

	static void replace_material(material_identifier old_material_, tds_material* new_material_);
	static void replace_section(section_identifier old_section_, tds_section* new_section_);
	static void replace_node(node_identifier old_node_, tds_node* new_node_);
	static void replace_element(element_identifier old_element_, tds_element* new_element_);
	static void replace_element_link(element_link_identifier old_element_link_, tds_element_link* new_element_link_);
	
};

#endif
