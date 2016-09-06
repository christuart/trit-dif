#ifndef PLUGINFWD_HH
#define PLUGINFWD_HH
class IPlugin;
struct plugin_file {
	std::string file_name;
	bool needed_after_initialisation;
};
#endif
