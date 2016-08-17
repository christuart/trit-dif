#include "plugins.hh"

//initialise the private static members of IPlugin
tds_run* IPlugin::tds_run_;
std::map<plugin,IPlugin*> IPlugin::plugin_map_;

IPlugin::IPlugin() {}
IPlugin::~IPlugin() {}
plugin IPlugin::plugin_identifier() { return PUndefined; }
void IPlugin::load_plugin() {}
void IPlugin::interrupt_material_creation(material_identifier&) {}
void IPlugin::interrupt_section_creation(section_identifier&) {}
void IPlugin::interrupt_node_creation(int, tds_node*) {}
void IPlugin::interrupt_element_creation(int, int, int, tds_element*) {}
void IPlugin::interrupt_element_link_creation(tds_element_link*) {}
void IPlugin::interrupt_pre_simulation() {}
void IPlugin::interrupt_start_step() {}
void IPlugin::interrupt_end_step() {}
void IPlugin::interrupt_post_simulation() {}

void IPlugin::store_plugin(IPlugin* _plugin) {
	plugin plugin_type = _plugin->plugin_identifier();
	if (plugin_loaded(plugin_type)) {
		std::cerr << "Tried to store multiple of same plugin type: " << plugin_type << std::endl;
		throw;
	}
	IPlugin::plugin_map_.insert(std::pair<plugin,IPlugin*>(plugin_type,_plugin));
}
IPlugin* IPlugin::get_plugin(plugin _plugin_type) {
	if (!plugin_loaded(_plugin_type)) {
		std::cerr << "Requested unloaded plugin type: " << _plugin_type << std::endl;
		throw;
	}
	return IPlugin::plugin_map_.at(_plugin_type);
}
std::map<plugin,IPlugin*>::iterator IPlugin::get_plugin_iterator() {
	return IPlugin::plugin_map_.begin();
}
std::map<plugin,IPlugin*>::iterator IPlugin::get_plugin_iterator_end() {
	return IPlugin::plugin_map_.end();
}
bool IPlugin::plugin_loaded(plugin _plugin_type) {
	return (IPlugin::plugin_map_.count(_plugin_type) == 1);
}
void IPlugin::set_run(tds_run* _tds_run) {
	tds_run_ = _tds_run;
}
tds_run* IPlugin::get_run() {
	return tds_run_;
}
