#include "plugin-outgassing.hh"

void Outgassing::load_plugin() {
	std::cout << "Loading Outgassing plug-in..." << std::endl;
	IPlugin::get_run()->add_section_interrupt(this);
	IPlugin::get_run()->add_element_link_interrupt(this);
	IPlugin::get_run()->add_pre_simulation_interrupt(this);
	IPlugin::get_run()->add_start_step_interrupt(this);
	IPlugin::get_run()->add_end_step_interrupt(this);
	IPlugin::get_run()->add_post_simulation_interrupt(this);
	outgassing_material = new tds_material("outgassing", 1.0, 0.0);
	IPlugin::get_run()->add_material(outgassing_material);
}
void Outgassing::interrupt_section_creation(section_identifier& _new_section) {
	std::cout << "Outgassing was given the opportunity to interrupt the creation of section '"
	          << IPlugin::get_run()->section(_new_section.section_id).name() << "'." << std::endl;
	std::istringstream iss(_new_section.section->name());
	std::string name_root;
	if (!(iss >> name_root)) {
		throw Errors::OutgassingException("Section with an empty name?");
	}
	if (name_root == "outgassing") {
		std::cout << "Section '" << _new_section.section->name() << "' has had the outgassing material applied." << std::endl;
		_new_section.section->material(&(get_run()->material("outgassing")));
		outgassing_section_found = true;
		outgassing_sections.push_back(_new_section.section);
		outgassed_quantities_by_section[_new_section.section] = 0.0f;
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
				throw Errors::OutgassingException("This model has at least 1 source element touching an outgassing element, which is not allowed.");
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
				throw Errors::OutgassingException("This model has at least 1 source element touching an outgassing element, which is not allowed.");
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
	if (!outgassing_section_found) {
		throw Errors::OutgassingException("No outgassing sections were found. If outgassing is not required, please disable the outgassing plug-in.");
	}
}
void Outgassing::interrupt_post_simulation() {
	summarise_outgassing();
}

bool Outgassing::is_outgassing_section(tds_section* _outgassing_section) {
	return (std::find(outgassing_sections.begin(), outgassing_sections.end(), _outgassing_section) != outgassing_sections.end());
}
void Outgassing::store_outgassing_quantity(tds_section* _outgassing_section, double _outgassed_quantity) {
	outgassed_quantities_by_section[_outgassing_section] += _outgassed_quantity;
}
double Outgassing::get_total_outgassed_from_section(tds_section* _outgassed_section) {
	if (!is_outgassing_section(_outgassed_section)) {
		return 0.0f;
	}
	return outgassed_quantities_by_section.at(_outgassed_section);
}
void Outgassing::summarise_outgassing() {
        
        double unlabelled_sum = 0.0f;
        double total_sum = 0.0f;
        
	std::cout << std::endl;
	std::cout << "***" << std::endl;
	std::cout << "************************************************************" << std::endl;
	std::cout << "***" << std::endl;
	std::cout << "***  Outgassing Output" << std::endl;
	std::cout << "***" << std::endl;
	// First, go through the numbered sections
	typedef std::map<int,tds_section*>::iterator los_it_type;
	for(los_it_type it = labelled_outgassing_sections.begin(); it != labelled_outgassing_sections.end(); it++) {
                total_sum += get_total_outgassed_from_section(it->second);
		std::ostringstream oss;
		oss << "outgassing section " << it->first;
		std::cout << "***  Total released from " << std::right << std::setw(22) << oss.str() << ": " << std::setw(12) << get_total_outgassed_from_section(it->second) << std:: endl;
		// iterator->first = key
		// iterator->second = value
	}
	std::cout << "***" << std::endl;
	// Then, go through the unnumbered sections
	typedef std::vector<tds_section*>::iterator uos_it_type;
	for(uos_it_type it = outgassing_sections.begin(); it != outgassing_sections.end(); it++) {
                unlabelled_sum += get_total_outgassed_from_section((*it));
	}
        {
                total_sum += unlabelled_sum;
		std::ostringstream oss;
		oss << "unlabelled sections";
		std::cout << "***  Total released from " << std::right << std::setw(22) << oss.str() << ": " << std::setw(12) << unlabelled_sum << std:: endl;
        }
        
	std::cout << "***" << std::endl;
	// Then show total
	std::cout << "***  Total quantity outgassed: " << std::setw(12) << total_sum << std::endl;
	std::cout << "***" << std::endl;
	std::cout << "************************************************************" << std::endl;
	std::cout << "***" << std::endl;
	std::cout << std::endl;

}

tds_outgassing_element_link::tds_outgassing_element_link(tds_element* _M, tds_element* _N, tds_section* _outgassing_section, bool _M_is_outgassing):tds_element_link(_M,_N),outgassing_section_(_outgassing_section) {
	_M_is_outgassing ? outgassing_element_ = _M : outgassing_element_ = _N;
	outgassing_plugin_ = dynamic_cast<Outgassing*>(IPlugin::get_plugin(POutgassing));
}
tds_outgassing_element_link::~tds_outgassing_element_link() {};

/// Stores locally and globally the outgassing of a given quantity (not a flow rate!)
/** This method is usually called by the derived <flow_rate> method, which will find the flow rate
    and multiply it by <tds_run::delta_t>. It adds the quantity to both a local (for this element
    link) outgassing tracker and a global (in the <Outgassing> object) tracker for this outgassing
    section
*/
void tds_outgassing_element_link::outgass_quantity(double _outgassing_quantity) {
	outgassed_quantity_ += _outgassing_quantity;
	outgassing_plugin().store_outgassing_quantity(outgassing_section_pointer(),_outgassing_quantity);
}
double tds_outgassing_element_link::flow_rate(bool _AB) {
	// Make sure to only ever store the outgassing quantity when the
	// flag changes, i.e. once per timestep - per element.
	if (_AB != flagAB()) {

                // Here we will use a worst-case scenario, with the maximal amount of
                // outgassing possible: all the contaminant in the external elements
                double total_outgassable_quantity
                        = (elementN().contamination(_AB) - elementM().contamination(_AB))
                        * ((&elementM() != outgassing_element_) ? elementM().size() : elementN().size());
                // This calculation works on the basis of the outgassing element having
                // contamination of zero, so N.cont() - M.cont() will equal either
                // N.cont() or -M.cont(), with the sign matching what is needed for a
                // flow_rate() return value.
                
                flow_rate_ = total_outgassable_quantity / IPlugin::get_run()->delta_t();
                
                // make sure to store the correct direction of outgassing being positive!
		outgass_quantity(fabs(total_outgassable_quantity));
		flagAB(_AB);
                
	}
	return flow_rate_;
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
