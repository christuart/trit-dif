#ifndef PLUGINS_HH
#define PLUGINS_HH

#include "tds.hh"

enum plugin {
	PUndefined,
	POutgassing,
	PDecay
};

class IPlugin {

private:
	static tds_run* tds_run_;
	static std::map<plugin,IPlugin*> plugin_map_;
public:
	IPlugin();
	virtual ~IPlugin();

	virtual plugin plugin_identifier();

	virtual void load_plugin();
	virtual void interrupt_material_creation(material_identifier& _new_material);
	virtual void interrupt_section_creation(section_identifier& _new_section);
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

	static void store_plugin(IPlugin* _plugin);
	static IPlugin* get_plugin(plugin _plugin_type);
	static std::map<plugin,IPlugin*>::iterator get_plugin_iterator();
	static std::map<plugin,IPlugin*>::iterator get_plugin_iterator_end();
	static bool plugin_loaded(plugin _plugin_type);
	static void set_run(tds_run* _tds_run);
	static tds_run* get_run();
	
};

#include "plugin-outgassing.hh"

#endif
