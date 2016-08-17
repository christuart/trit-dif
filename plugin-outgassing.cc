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
}
void Outgassing::interrupt_material_creation(material_identifier& _new_material) {
	std::cout << "Outgassing was given the opportunity to interrupt the creation of material '"
	          << IPlugin::get_run()->material(_new_material.material_id).name() << "'." << std::endl; 
}
