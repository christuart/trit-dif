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
}
