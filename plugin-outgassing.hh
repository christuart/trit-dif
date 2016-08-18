#ifndef OUTGASSING_HH
#define OUTGASSING_HH
#include "plugins.hh"

class Outgassing : public IPlugin {
	
private:
	tds_material* outgassing_material_;
	std::vector<double> outgassing_totals;
	std::vector<tds_section*> outgassing_sections; // contains all the outgassing sections found
	std::map<int,tds_section*> labelled_outgassing_sections; // contains only the outgassing sections labelled with numbers

	bool outgassing_section_found;
	
public:
	inline plugin plugin_identifier() { return POutgassing; }
	void load_plugin();
	void interrupt_section_creation(section_identifier& _new_section);
	void interrupt_element_link_creation(element_link_identifier& _new_element_link);
	void interrupt_pre_simulation();
	void interrupt_post_simulation();
};

class tds_outgassing_element_link : public tds_element_link {

public:
	tds_outgassing_element_link(tds_element* _M, tds_element* _N);
	
	double flow_rate(bool _AB);
	
};

#endif
