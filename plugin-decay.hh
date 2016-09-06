#ifndef DECAY_HH
#define DECAY_HH

#include "plugins.hh"

class Decay : public IPlugin {
	
public:
	// Implementing the IPlugin interface:
	inline plugin plugin_identifier() { return POutgassing; }
	void load_plugin();
	void interrupt_end_step();
	
};

#endif
