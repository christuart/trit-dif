#ifndef OUTGASSING_HH
#define OUTGASSING_HH
#include "plugins.hh"

class Outgassing : public IPlugin {
	
private:
	tds_material* outgassing_material;
	std::vector<double> outgassing_totals;
	std::vector<tds_section*> outgassing_sections; // contains all the outgassing sections found
	std::map<int,tds_section*> labelled_outgassing_sections; // contains only the outgassing sections labelled with numbers
	std::map<tds_section*,int> outgassing_sections_labels; // reverse of the one-one map before, to provide bi-directionality
	std::map<tds_section*,double> outgassed_quantities_by_section;

	bool outgassing_section_found;
	
public:
	// Implementing the IPlugin interface:
	inline plugin plugin_identifier() { return POutgassing; }
	void load_plugin();
	void interrupt_section_creation(section_identifier& _new_section);
	void interrupt_element_link_creation(element_link_identifier& _new_element_link);
	void interrupt_pre_simulation();
	void interrupt_post_simulation();

	// Outgassing specific methods:
	bool is_outgassing_section(tds_section*);
	void store_outgassing_quantity(tds_section* _outgassing_section, double _outgassed_quantity);
	double get_total_outgassed_from_section(tds_section*);
	void summarise_outgassing();
};
	
class tds_outgassing_element_link : public tds_element_link {
private:
	Outgassing* outgassing_plugin_;
	tds_section* outgassing_section_;
	tds_element* outgassing_element_;
	double outgassed_quantity_;
public:
	
private:
	inline tds_section* outgassing_section_pointer() { return outgassing_section_; }
	inline Outgassing& outgassing_plugin() { return *(outgassing_plugin_); }
public:
	tds_outgassing_element_link(tds_element* _M, tds_element* _N, tds_section* _outgassing_section, bool _M_is_outgassing);
	~tds_outgassing_element_link();

	void outgass_quantity(double _outgassing_flow);
	inline double outgassed_quantity() { return outgassed_quantity_; }
	
	double flow_rate(bool _AB);
	short positive_flow(tds_element* whoami);
	
	bool is_base();
	
};

namespace Errors {
	class OutgassingException : public PluginRuntimeException {
	public:
		explicit OutgassingException(const std::string& what_arg):PluginRuntimeException("(outgassing) " + what_arg) {}
	};
}
#endif
