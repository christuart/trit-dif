#include "plugin-decay.hh"

extern MessageBuffer exceptions;
extern MessageBuffer warnings;
extern DebugMessageBuffer debugging;

Decay::Decay()/*:decay(MBPlugin,"DECAY       ")*/ {
}
void Decay::load_plugin() {
	decay = &debugging;
	LOG((*decay),"Loading Decay plug-in");
	IPlugin::get_run()->add_end_step_interrupt(this);

	file_handler decay_constant_file(ReadFromFile,get_plugin_file().file_name);
	int line = 1;
	try {
		while (decay_constant_file.key(line) != "decay-constant")
			++line;
	} catch (Errors::FileHandlerLineUnavailableException& e) {
		throw Errors::PluginFileException("Could not locate 'decay-constant'.",plugin_identifier());
	}
	LOG((*decay),"Contaminant decay constant given as " << decay_constant_file.value(line));
	std::istringstream iss;
	iss.str(decay_constant_file.value(line));
	iss >> contaminant_decay_constant;
}
void Decay::interrupt_end_step(const int _step, const double _time) {
	// need to be careful to get the correct frame's contamination:#
	// the normal element behaviour is, "If I am asked for a contamination,
	// I will provide the previous step's value. If I am asked to store a
	// contamination, I will store it as this step's value." Therefore, to
	// get a look at this step's value, we must use !flagAB() instead of
	// simply flagAB().

	/*
	int N = IPlugin::get_tds_run()->n_elements();
	double timestep = IPlugin::get_tds_run()
	
	for (int i=0; i < N; ++i) {
	*/	
	
}
