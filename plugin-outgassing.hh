#ifndef OUTGASSING_HH
#define OUTGASSING_HH
#include "plugins.hh"

class Outgassing : public IPlugin {
public:
	inline plugin plugin_identifier() { return POutgassing; }
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

class tds_outgassing_element_link : public tds_element_link {

public:
	tds_outgassing_element_link(tds_element* _M, tds_element* _N);
	
	double flow_rate(bool _AB);
	
};

#endif
