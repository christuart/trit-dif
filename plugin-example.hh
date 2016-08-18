#ifndef EXAMPLE_HH
#define EXAMPLE_HH
#include "plugins.hh"

class Example : public IPlugin {
public:
	// The plugin_identifier() override must be implemented
	// the 'plugin' enumeration should be extended with a
	// new entry for your plugin
	inline plugin plugin_identifier() { return PExample; }

	// The following methods are all optionally overriden.
	void load_plugin();
	void interrupt_material_creation(material_identifier& _new_material);
	void interrupt_section_creation(section_identifier& _new_section);
	void interrupt_node_creation(node_identifier& _new_node);
	void interrupt_element_creation(element_identifier& _new_element);
	void interrupt_element_link_creation(element_link_identifier& _new_element_link);
	void interrupt_pre_simulation();
	void interrupt_start_step(int _step, double);
	void interrupt_end_step(int _step, double);
	void interrupt_post_simulation();
};

// As an example, you may create a derived class from the
// element links, which has 0 flow rate always.
class tds_example_element_link : public tds_element_link {

public:
	double flow_rate(bool _AB);
	
};

#endif
