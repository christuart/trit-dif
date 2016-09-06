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
void IPlugin::interrupt_node_creation(node_identifier&) {}
void IPlugin::interrupt_element_creation(element_identifier&) {}
void IPlugin::interrupt_element_link_creation(element_link_identifier&) {}
void IPlugin::interrupt_pre_simulation() {}
void IPlugin::interrupt_start_step(int _step, double _time) {}
void IPlugin::interrupt_end_step(int _step, double _time) {}
void IPlugin::interrupt_post_simulation() {}


void IPlugin::replace_material(material_identifier& _old_material, tds_material* _new_material) {
	for (int i=0; i < get_run()->n_sections(); ++i) {
		if (&(get_run()->section(i).material()) == _old_material.material)
			get_run()->section(i).material(_new_material);
	}
	get_run()->change_material_pointer(_old_material.material_id,_new_material);
	delete _old_material.material;
	_old_material.material = _new_material;
}
void IPlugin::replace_section(section_identifier& _old_section, tds_section* _new_section) {
	get_run()->change_section_pointer(_old_section.section_id,_new_section);
	delete _old_section.section;
	_old_section.section = _new_section;
}
void IPlugin::replace_node(node_identifier& _old_node, tds_node* _new_node) {
	for (int i=0; i < get_run()->n_elements(); ++i) {
		for (int j=0; j < get_run()->element(i).n_nodes(); ++j) {
			if (&(get_run()->element(i).node(j)) == _old_node.node)
				get_run()->element(i).node(j,_new_node);
		}
	}
	get_run()->change_node_pointer(_old_node.node_id,_new_node);
	delete _old_node.node;
	_old_node.node = _new_node;
}
void IPlugin::replace_element(element_identifier& _old_element, tds_element* _new_element) {
	for (int i=0; i < get_run()->n_nodes(); ++i) {
		for (int j=0; j < get_run()->node(i).n_elements(); ++j) {
			if (&(get_run()->node(i).element(j)) == _old_element.element)
				get_run()->node(i).element(j,_new_element);
		}
	}
	get_run()->section(_old_element.section_id).element(_old_element.section_element_id, _new_element);
	get_run()->change_element_pointer(_old_element.element_id,_new_element);
	delete _old_element.element;
	_old_element.element = _new_element;
}
void IPlugin::replace_element_link(element_link_identifier& _old_element_link, tds_element_link* _new_element_link) {
	for (int i=0; i < _old_element_link.element_link->elementM().n_neighbours(); ++i) {
		if (&(_old_element_link.element_link->elementM().neighbour(i)) == _old_element_link.element_link)
			_old_element_link.element_link->elementM().neighbour(i, _new_element_link);
	}
	for (int i=0; i < _old_element_link.element_link->elementN().n_neighbours(); ++i) {
		if (&(_old_element_link.element_link->elementN().neighbour(i)) == _old_element_link.element_link)
			_old_element_link.element_link->elementN().neighbour(i, _new_element_link);
	}
	delete _old_element_link.element_link;
	_old_element_link.element_link = _new_element_link;
}

void IPlugin::store_plugin(IPlugin* _plugin) {
	plugin plugin_type = _plugin->plugin_identifier();
	if (plugin_loaded(plugin_type)) {
		std::ostringstream oss;
		oss << "Tried to store multiple of same plugin type: " << plugin_type;
		throw Errors::PluginException(oss.str());
	}
	IPlugin::plugin_map_.insert(std::pair<plugin,IPlugin*>(plugin_type,_plugin));
}
IPlugin* IPlugin::get_plugin(plugin _plugin_type) {
	if (!plugin_loaded(_plugin_type)) {
		std::ostringstream oss;
		oss << "Requested unloaded plugin type: " << _plugin_type;
		throw Errors::PluginException(oss.str());
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
