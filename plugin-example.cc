#include "plugin-example.hh"

void Example::load_plugin() {
	std::cout << "Loading Example plug-in..." << std::endl;
	IPlugin::get_run()->add_material_interrupt(this);
	IPlugin::get_run()->add_section_interrupt(this);
	IPlugin::get_run()->add_node_interrupt(this);
	IPlugin::get_run()->add_element_interrupt(this);
	IPlugin::get_run()->add_element_link_interrupt(this);
	IPlugin::get_run()->add_pre_simulation_interrupt(this);
	IPlugin::get_run()->add_start_step_interrupt(this);
	IPlugin::get_run()->add_end_step_interrupt(this);
	IPlugin::get_run()->add_post_simulation_interrupt(this);
}
void Example::interrupt_material_creation(material_identifier& _new_material) {
	std::cout << "Example Plug-In was given the opportunity to interrupt the creation of material '"
	          << IPlugin::get_run()->material(_new_material.material_id).name() << "'." << std::endl; 
}
void Example::interrupt_section_creation(section_identifier& _new_section) {
	std::cout << "Example Plug-In was given the opportunity to interrupt the creation of section '"
	          << IPlugin::get_run()->section(_new_section.section_id).material().name() << "'." << std::endl; 
}
void Example::interrupt_node_creation(node_identifier& _new_node) {
	std::cout << "Example Plug-In was given the opportunity to interrupt the creation of node '"
	          << _new_node.node_id << "'." << std::endl; 
}
void Example::interrupt_element_creation(element_identifier& _new_element) {
	std::cout << "Example Plug-In was given the opportunity to interrupt the creation of element '"
	          << _new_element.element_id << "'." << std::endl; 
}
void Example::interrupt_element_link_creation(element_link_identifier& _new_element_link) {
	std::cout << "Example plug-in was given the opportunity to interrupt the creation of the link between '"
	          << &(_new_element_link.element_link->elementM()) << "' and '" << &(_new_element_link.element_link->elementN()) << "'." << std::endl; 
}
void Example::interrupt_pre_simulation() {
	std::cout << "Pre-simulation, Example plug-in can interrupt." << std::endl;
}
void Example::interrupt_start_step(int step, double) {
	if (step % 1000 == 0) std::cout << "Example plug-in marking the start of step " << step << std::endl;
}
void Example::interrupt_end_step(int step, double) {
	if (step % 1000 == 0) std::cout << "Example plug-in marking the end of step " << step << std::endl;
}
void Example::interrupt_post_simulation() {
	std::cout << "Post-simulation, Example plug-in can interrupt." << std::endl;
}

double tds_example_element_link::flow_rate(bool _AB) {
	return 0.0f;
}
