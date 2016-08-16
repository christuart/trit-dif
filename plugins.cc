#include "plugins.hh"

IPlugin::IPlugin() {}
IPlugin::~IPlugin() {}
plugin IPlugin::plugin_identifier() { return PUndefined; }
void IPlugin::load_plugin() {}
void IPlugin::interrupt_material_creation(material_identifier) {}
void IPlugin::interrupt_section_creation(int, tds_section*) {}
void IPlugin::interrupt_node_creation(int, tds_node*) {}
void IPlugin::interrupt_element_creation(int, int, int, tds_element*) {}
void IPlugin::interrupt_element_link_creation(tds_element_link*) {}
void IPlugin::interrupt_pre_simulation() {}
void IPlugin::interrupt_start_step() {}
void IPlugin::interrupt_end_step() {}
void IPlugin::interrupt_post_simulation() {}
