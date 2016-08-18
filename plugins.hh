#ifndef PLUGINS_HH
#define PLUGINS_HH

#include "tds.hh"

enum plugin {
	PUndefined,
	POutgassing,
	PExample,
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
	virtual void interrupt_node_creation(node_identifier& _new_node);
	virtual void interrupt_element_creation(element_identifier& _new_element);
	virtual void interrupt_element_link_creation(element_link_identifier& _new_element_link);
	virtual void interrupt_pre_simulation();
	virtual void interrupt_start_step(int _step, double _time);
	virtual void interrupt_end_step(int _step, double _time);
	virtual void interrupt_post_simulation();

	static void replace_material(material_identifier& _old_material, tds_material* _new_material);
	static void replace_section(section_identifier& _old_section, tds_section* _new_section);
	static void replace_node(node_identifier& _old_node, tds_node* _new_node);
	static void replace_element(element_identifier& _old_element, tds_element* _new_element);
	static void replace_element_link(element_link_identifier& _old_element_link, tds_element_link* _new_element_link);

	static void store_plugin(IPlugin* _plugin);
	static IPlugin* get_plugin(plugin _plugin_type);
	static std::map<plugin,IPlugin*>::iterator get_plugin_iterator();
	static std::map<plugin,IPlugin*>::iterator get_plugin_iterator_end();
	static bool plugin_loaded(plugin _plugin_type);
	static void set_run(tds_run* _tds_run);
	static tds_run* get_run();
	
};

#include "plugin-example.hh"
#include "plugin-outgassing.hh"

#endif
