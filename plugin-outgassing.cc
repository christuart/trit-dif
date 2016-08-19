#include "plugin-outgassing.hh"

void Outgassing::load_plugin() {
	std::cout << "Loading Outgassing plug-in..." << std::endl;
	IPlugin::get_run()->add_section_interrupt(this);
	IPlugin::get_run()->add_element_link_interrupt(this);
	IPlugin::get_run()->add_pre_simulation_interrupt(this);
	IPlugin::get_run()->add_start_step_interrupt(this);
	IPlugin::get_run()->add_end_step_interrupt(this);
	IPlugin::get_run()->add_post_simulation_interrupt(this);
}
void Outgassing::interrupt_section_creation(section_identifier& _new_section) {
	std::cout << "Outgassing was given the opportunity to interrupt the creation of section '"
	          << IPlugin::get_run()->section(_new_section.section_id).name() << "'." << std::endl;
	std::istringstream iss(_new_section.section->name());
	std::string name_root;
	if (!(iss >> name_root)) {
		std::cerr << "(outgassing) Section with an empty name?" << std::endl;
		throw;
	}
	if (name_root == "outgassing") {
		std::cout << "Section '" << _new_section.section->name() << "' has had the outgassing material applied." << std::endl;
		_new_section.section->material(&(get_run()->material("outgassing")));
		outgassing_section_found = true;
		outgassing_sections.push_back(_new_section.section);
		std::string name_body;
		if (iss >> name_body) {
			iss.clear();
			iss.str(name_body);
			int label;
			if (iss >> label) {
				labelled_outgassing_sections[label] = _new_section.section;
				outgassing_sections_labels[_new_section.section] = label;
			}
		}
	}
}
void Outgassing::interrupt_element_link_creation(element_link_identifier& _new_element_link) {
	std::string materialM, materialN;
	materialM = _new_element_link.element_link->elementM().material().name();
	materialN = _new_element_link.element_link->elementN().material().name();
	if (materialM == "outgassing") {
		if (materialN != "outgassing") {
			if (materialN == "source") {
				std::cerr << "This model has at least 1 source element touching an outgassing element, which is not allowed." << std::endl;
				throw;
			}
			tds_outgassing_element_link* replacement_link = new tds_outgassing_element_link(
				                                                &(_new_element_link.element_link->elementM()),
				                                                &(_new_element_link.element_link->elementN()),
				                                                &(_new_element_link.element_link->elementM().section()),
				                                                true
				          );
			IPlugin::replace_element_link(_new_element_link, replacement_link);
		}
	} else if (materialN == "outgassing") {
		if (materialM == "source") {
			std::cerr << "This model has at least 1 source element touching an outgassing element, which is not allowed." << std::endl;
			throw;
		}
		tds_outgassing_element_link* replacement_link = new tds_outgassing_element_link(
			                                                &(_new_element_link.element_link->elementM()),
			                                                &(_new_element_link.element_link->elementN()),
			                                                &(_new_element_link.element_link->elementN().section()),
			                                                false
			          );
		IPlugin::replace_element_link(_new_element_link, replacement_link);
		
	}
}
void Outgassing::interrupt_pre_simulation() {
	// Outgassing needs to verify that there were some outgassing sections found.
}
void Outgassing::interrupt_post_simulation() {
	std::cout << "Post-simulation Outgassing plug-in can interrupt." << std::endl;
}

tds_outgassing_element_link::tds_outgassing_element_link(tds_element* _M, tds_element* _N, tds_section* _outgassing_section, bool _M_is_outgassing):tds_element_link(_M,_N),outgassing_section_(_outgassing_section) {
	_M_is_outgassing ? outgassing_element_ = _M : outgassing_element_ = _N;
}

double tds_outgassing_element_link::flow_rate(bool _AB) {
	return 0.0f;
}
short tds_outgassing_element_link::positive_flow(tds_element* whoami) {
	if (whoami != outgassing_element_) {
		if (elementM_ == whoami) return 1; else return -1;
	} else {
		return 0;
	}
}
bool tds_outgassing_element_link::is_base() {
	return false;
}
