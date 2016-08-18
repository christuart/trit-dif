#include "plugin-outgassing.hh"

tds_outgassing_element_link::tds_outgassing_element_link(tds_element* _M, tds_element* _N):tds_element_link(_M,_N){
	}

double tds_outgassing_element_link::flow_rate(bool _AB) {
	return 0.0f;
}

void Outgassing::load_plugin() {
	tds_nodes n;
	n.push_back(new tds_node(1.0f,0.0f,0.0f));
	n.push_back(new tds_node(-1.0f,0.0f,0.0f));
	tds_element* e = new tds_element(n, new tds_material("aoisdn",1.0f,1e-20),0.0f);
	tds_element_link* l = new tds_outgassing_element_link(e,e);
	std::cout << "Loading Outgassing plug-in..." << std::endl;
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
void Outgassing::interrupt_material_creation(material_identifier& _new_material) {
	std::cout << "Outgassing was given the opportunity to interrupt the creation of material '"
	          << IPlugin::get_run()->material(_new_material.material_id).name() << "'." << std::endl; 
}
void Outgassing::interrupt_section_creation(section_identifier& _new_section) {
	std::cout << "Outgassing was given the opportunity to interrupt the creation of section '"
	          << IPlugin::get_run()->section(_new_section.section_id).material().name() << "'." << std::endl; 
}
void Outgassing::interrupt_node_creation(node_identifier& _new_node) {
	std::cout << "Outgassing was given the opportunity to interrupt the creation of node '"
	          << _new_node.node_id << "'." << std::endl; 
}
void Outgassing::interrupt_element_creation(element_identifier& _new_element) {
	std::cout << "Outgassing was given the opportunity to interrupt the creation of element '"
	          << _new_element.element_id << "'." << std::endl; 
}
void Outgassing::interrupt_element_link_creation(element_link_identifier& _new_element_link) {
	std::cout << "Outgassing was given the opportunity to interrupt the creation of the link between '"
	          << &(_new_element_link.element_link->elementM()) << "' and '" << &(_new_element_link.element_link->elementN()) << "'." << std::endl; 
}
void Outgassing::interrupt_pre_simulation() {
	std::cout << "Pre-simulation Outgassing plug-in can interrupt." << std::endl;
}
void Outgassing::interrupt_start_step(int step, double) {
	if (step % 1000 == 0) std::cout << "Oh hello!" << std::endl;
}
void Outgassing::interrupt_end_step(int step, double) {
	if (step % 1000 == 0) std::cout << "Oh hello!" << std::endl;
}
void Outgassing::interrupt_post_simulation() {
	std::cout << "Post-simulation Outgassing plug-in can interrupt." << std::endl;
}
