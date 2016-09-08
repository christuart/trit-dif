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

	int S = IPlugin::get_run()->n_sections();
	int N;
	double timestep = IPlugin::get_run()->delta_t();
	tds_element* this_element;
	double decay_factor = exp(-timestep*contaminant_decay_constant);
	for (int i=0; i < S; ++i) {
		// Note simple model with constant source so no need to update them
		if (!IPlugin::get_run()->section(i).material().is_source()) {
			N = IPlugin::get_run()->section(i).n_elements();
			for (int j=0; j < N; ++j) {
				this_element = &(IPlugin::get_run()->section(i).element(j));
				this_element->contamination(decay_factor*this_element->contamination(!this_element->flagAB()));
			}
		}
	}
}
