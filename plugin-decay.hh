#ifndef DECAY_HH
#define DECAY_HH

#include "plugins.hh"

class Decay : public IPlugin {
private:
	MessageBuffer* decay;
	double contaminant_decay_constant;
public:
	Decay();
	// Implementing the IPlugin interface:
	inline plugin plugin_identifier() { return POutgassing; }
	void load_plugin();
	void interrupt_end_step(const int _step, const double _time);
	
};

#endif
