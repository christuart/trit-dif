#include "plugin-outgassing.hh"

void Outgassing::load_plugin() {
	std::cout << "Loading Outgassing plug-in..." << std::endl;
	IPlugin::get_run()->add_section_interrupt(this);
	IPlugin::get_run()->add_element_link_interrupt(this);
	IPlugin::get_run()->add_pre_simulation_interrupt(this);
	IPlugin::get_run()->add_start_step_interrupt(this);
	IPlugin::get_run()->add_end_step_interrupt(this);
	IPlugin::get_run()->add_post_simulation_interrupt(this);
}
void Outgassing::interrupt_section_creation(section_identifier& _new_section) {
	std::cout << "Outgassing was given the opportunity to interrupt the creation of section '"
	          << IPlugin::get_run()->section(_new_section.section_id).material().name() << "'." << std::endl;
	std::istringstream iss(_new_section.section->name());
	std::string name_root;
	if (!(iss >> name_root)) {
		std::cerr << "(outgassing) Section with an empty name?" << std::endl;
		throw;
	}
	if (name_root == "outgassing") {
		outgassing_section_found = true;
		outgassing_sections.push_back(_new_section.section);
		std::string name_body;
		if (iss >> name_body) {
			iss.clear();
			iss.str(name_body);
			int label;
			if (iss >> label)
				labelled_outgassing_sections[label] = _new_section.section;
		}
	}
}
void Outgassing::interrupt_element_link_creation(element_link_identifier& _new_element_link) {
	std::cout << "Outgassing was given the opportunity to interrupt the creation of the link between '"
	          << &(_new_element_link.element_link->elementM()) << "' and '" << &(_new_element_link.element_link->elementN()) << "'." << std::endl; 
}
void Outgassing::interrupt_pre_simulation() {
	// Outgassing needs to verify that there were some outgassing sections found.
}
void Outgassing::interrupt_post_simulation() {
	std::cout << "Post-simulation Outgassing plug-in can interrupt." << std::endl;
}

tds_outgassing_element_link::tds_outgassing_element_link(tds_element* _M, tds_element* _N):tds_element_link(_M,_N){
}

double tds_outgassing_element_link::flow_rate(bool _AB) {
	return 0.0f;
}
