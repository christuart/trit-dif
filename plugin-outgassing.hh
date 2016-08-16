#ifndef OUTGASSING_HH
#define OUTGASSING_HH
#include "plugins.hh"

class Outgassing : IPlugin {
	inline plugin plugin_identifier() { return POutgassing; }
	void load_plugin();
};

class tds_outgassing_element_link : public tds_element_link {

public:
	tds_outgassing_element_link(tds_element* _M, tds_element* _N);
	
	double flow_rate(bool _AB);
	
};

#endif
