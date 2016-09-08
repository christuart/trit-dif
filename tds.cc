#include "tds.hh"
#include "plugins.hh"

extern MessageBuffer exceptions;
extern MessageBuffer warnings;
extern DebugMessageBuffer debugging;
extern standard_cout_listener console_out;
extern standard_cerr_listener console_err;
extern error_log_listener error_log;

/******************** TDS METHODS ********************/









tds::tds():sections_() {
	element_dimensions_ = 0x0;
	tds_log = MessageBuffer(MBTdsLog,"MODEL       ");
	tds_log.add_listener(&console_out);
}

tds::~tds(){
	empty_model();
}

int tds::add_section(tds_section* new_section) {
	LOG(tds_log,"adding a new section");
	sections_.push_back(new_section);
	return sections_.size()-1;
}
int tds::add_material(tds_material* new_material) {
	LOG(tds_log,"adding a new material: " << (*new_material).name());
	material_map_[(*new_material).name()] = new_material;
	materials_.push_back(new_material);
	return materials_.size()-1;
}
int tds::add_node(tds_node* new_node) {
	nodes_.push_back(new_node);
	return nodes_.size()-1;
}
int tds::add_element(tds_element* new_element) {
	elements_.push_back(new_element);
	return elements_.size()-1;
}

void tds::clean_sections() {
	for (int i=0; i<sections_.size(); ++i) delete sections_[i];
	sections_.resize(0);
}
void tds::clean_materials() {
	for (int i=0; i<materials_.size(); ++i) delete materials_[i];
	materials_.resize(0);
}
void tds::clean_nodes() {
	for (int i=0; i<nodes_.size(); ++i) delete nodes_[i];
	nodes_.resize(0);
}
void tds::clean_elements() {
	for (int i=0; i<elements_.size(); ++i) {
		
		delete elements_[i];
	}
	elements_.resize(0);
}
void tds::clean_inactive_elements() {
	for (int i=0; i<elements_.size(); ++i) {
		int j = i;
		// We're going to erase, and then continue moving through.
		// That means that we will go from
		// (1, 2, 3*, 4*, 5, 6*, 7) where * means empty
		// to
		// (1, 2, 5, 6*, 7)
		// and need to access 5 using index 3, not index 5.
		// So whilst looking for how many to delete, i doesn't change,
		// and j does.
		while (j < elements_.size() && element(j).n_neighbours() == 0) {
			delete elements_[j];
			++j;
		}
		if (j > i) {
			elements_.erase(elements_.begin()+i,elements_.begin()+j); // start inclusive end exclusive
		}
	}
}
void tds::empty_model() {
	clean_sections();
	clean_materials();
	clean_elements();
	clean_nodes();
}

/// Allocates at least enough memory for the expected number of materials
/** This is necessary to prevent reallocation occurring during the program.
    Reallocation would invalidate any pointers or iterators for the vector.
    Since it is only a vector of pointers, this wouldn't necessarily be
    disastrous but should probably be avoided if possible.
    */
void tds::expected_materials(int _n) { materials_.reserve(_n); }
/// Allocates at least enough memory for the expected number of sections
/** This is necessary to prevent reallocation occurring during the program.
    Reallocation would invalidate any pointers or iterators for the vector.
    Since it is only a vector of pointers, this wouldn't necessarily be
    disastrous but should probably be avoided if possible.
    */
void tds::expected_sections(int _n) { sections_.reserve(_n); }
/// Allocates at least enough memory for the expected number of nodes
/** This is necessary to prevent reallocation occurring during the program.
    Reallocation would invalidate any pointers or iterators for the vector.
    Since it is only a vector of pointers, this wouldn't necessarily be
    disastrous but should probably be avoided if possible.
    */
void tds::expected_nodes(int _n) { nodes_.reserve(_n); }
/// Allocates at least enough memory for the expected number of elements
/** This is necessary to prevent reallocation occurring during the program.
    Reallocation would invalidate any pointers or iterators for the vector.
    Since it is only a vector of pointers, this wouldn't necessarily be
    disastrous but should probably be avoided if possible.
    */
void tds::expected_elements(int _n) { elements_.reserve(_n); }

void tds::output_model_summary(bool show_materials, bool show_sections, bool show_elements, bool show_element_links, bool show_nodes) {
	LOG(tds_log,"We now have " << n_materials() << " materials, " << n_sections() << " sections, "
	    << n_elements() << " elements and " << n_nodes() << " nodes.");
	if (show_materials) {
		for (int i = 0; i < n_materials(); ++i) {
			LOG(tds_log, "Material " << (i+1) << ": " << material(i).name() << " - "
			    << material(i).density() << "kg/m^3 - " << material(i).diffusion_constant()
			    << "m^2/s");
		}
	}
	if (show_sections) {
		for (int i = 0; i < n_sections(); ++i) {
			LOG(tds_log, "Section " << (i+1) << ": " << section(i).material().name()
			    << " - " << section(i).n_elements() << " elements");
		}
	}
	if (show_elements) {
		for (int i = 0; i < n_elements(); ++i) {
			std::ostringstream oss;
			oss <<"Element " << (i+1) << " (" << &element(i) << "): " << element(i).material().name()
			    << " - " << element(i).n_nodes() << " nodes - "
			    << element(i).n_neighbours() << " neighbours - contamination at "
			    << element(i).contamination() << std::endl
			    << "Origin at [" << element(i).origin(0) << ", " << element(i).origin(1)
			    << ", " << element(i).origin(2) << "]" << std::endl;
			int j;
			for (j = 0; j < element(i).n_nodes()-1; j++) {
				oss << "Node " << (j+1) << ": "
				    << &(element(i).node(j))
				    << std::endl;
			}
			if (element(i).n_nodes() > 0)
				oss << "Node " << (j+1) << ": "
				    << &(element(i).node(j));
			if (show_element_links) {
				for (j = 0; j < element(i).n_neighbours(); ++j) {
					oss << std::endl
					    << "\tNeighbour " << (j+1) << ": "
					    << (element(i).neighbour(j).neighbour_of(&element(i)))
					    << " area " << (element(i).neighbour(j).interface_area())
					    << " multiplier " << (element(i).neighbour(j).a_n_dot_eMN_over_modMN());
				}
			}
			LOGMULTI(tds_log,oss.str());
		}
	}
	if (show_nodes) {
		for (int i = 0; i < n_nodes(); i++) {
			LOGMULTI(tds_log,"Node " << (i+1) << " (" << &node(i) << "):" << std::endl
				 << "[" << node(i).position(0) << ","
			         << node(i).position(1) << "," << node(i).position(2) << "]" << std::endl
				 << node(i).n_elements() << " elements");
		}
	}
	
}

void tds::register_element_type(int element_type) {
	switch (element_type) {
	case 1: // 2 node line
		element_dimensions_.set(one_d);
		break;
	case 2: // 3 node triangle
	case 3: // 4 node quadrangle
		element_dimensions_.set(two_d);
		break;
	case 4: // 4 node tetrahedron
	case 5: // 8 node hexahedron
	case 6: // 6 node prism
	case 7: // 5 node pyramid
		element_dimensions_.set(three_d);
		break;
	case 15: // 1 node point
		LOG(warnings,"Detected a single node point 'element'.");
		break;
	default: // non-linear elements
		element_dimensions_.set(second_order_or_worse);
		LOG(warnings,"Detected non-linear elements.");
	}
}









/******************** TDS_RUN METHODS ********************/









tds_run::tds_run():tds(),units_set_(false){
	run_file_processing_output = MessageBuffer(MBRunFileProcessing, "RUN FILE    ");
	units_processing_output = MessageBuffer(MBUnitsProcessing, "UNITS       ");
	simulation_output = MessageBuffer(MBSimulationOutput, "SIMULATION  ");
	run_file_processing_output.add_listener(&console_out);
	units_processing_output.add_listener(&console_out);
}

tds_run::~tds_run(){
	IPlugin::empty_plugins();
}

// void tds_run::tdsfile_close(){
//	std::cout<<"close tds file"<<std::endl;
// 	tdsfile_.close();	
// }

// void tds_run::tdsfile_open(std::string filename){
// 	std::cout<<"opening a tds file"<<std::endl;
// 	tdsfile_close();
// 	clean_sections();
// 	put_filename(filename);
// 	tdsfile_.open(filename.c_str());
// 	read_file();
// }

// void tds_run::read_file(){
// 	std::cout<<"read file and add sections"<<std::endl;
// 	//Add in a section
// 	add_section(new tds_section());
// }

void tds_run::make_analysis() {

	LOG(simulation_output, "Checking the model...");
	if ((n_sections() == 0) || (n_elements() == 0) || (n_nodes() == 0))
		throw Errors::MissingInputDataException("Model was incomplete at simulation start.");
	
	LOG(simulation_output, "Running the model...");

	// This is a very basic implementation of 'make_analysis' which has constant
	// even source and no outgassing

	// Allow the user to specify '-1' as the final element
	for (int i=0; i < tracked_elements()->size(); ++i) {
		if (tracked_element(i) < 0) tracked_element(i,tracked_element(i)+n_elements());
	}

	// Set AB flags (just in case!)
	bool this_flag = false;
	for (int i=0; i < n_elements(); ++i) {
		element(i).flagAB(this_flag);
		for (int j=0; j < element(i).n_neighbours(); ++j) {
			// Links should start with the opposite flag, so that they
			// calculate the flow_rate properly the first step.
			element(i).neighbour(j).set_flag_against(&element(i));
		}
	}

	// Set the source contamination
	if (settings.contamination_mode_time == "constant" && settings.contamination_mode_space == "constant") {
		double source_contamination = settings.contamination;
		for (int i=0; i < n_sections(); ++i) {
			if (section(i).material().is_source()) {
				LOG(simulation_output, "Section " << i << " has " << section(i).n_elements() << " elements and is getting fed the initial contamination.");
				for (int j=0; j < section(i).n_elements(); ++j) {
					section(i).element(j).debug_contamination();
					section(i).element(j).contamination(source_contamination);
					section(i).element(j).flagAB(!section(i).element(j).flagAB());
					section(i).element(j).contamination(source_contamination);
					section(i).element(j).flagAB(!section(i).element(j).flagAB());
				}
				LOG(simulation_output, "Contaminations set.");
			}
		}
	}

	// The next step is to let plug-ins change the setup. This happens
	// here, before the timing code, because timing code would otherwise
	// be badly calculated.
	interrupt_pre_simulation();
	
	double time = 0.0;
	double next_time_recording = tracking_interval();

	std::string tracking_file_address_;
	std::string contaminations_file_address_;
	bool usedTrackingFile = false;
	try {
		trackingfile_.exceptions(ofstream::failbit | ofstream::badbit);
		tracking_file_address_ = std::string(tracking_file_address());
		trackingfile_.open(tracking_file_address_.c_str());
		usedTrackingFile = true;
		trackingfile_ << "element, time, contamination" << std::endl;
		trackingfile_ << std::scientific;
		for (int i=0; i < tracked_elements()->size(); ++i)
			trackingfile_ << (tracked_element(i)) << ", " << time << ", " << element(tracked_element(i)-1).contamination() << std::endl;

		// Let's do some timing
		const int trail_length = 5;
		int history_count = 0;
		int history_index = trail_length-1;
		double step_times[trail_length]; // values stored will be ms per step per element.
		uint64 start_checkmark = GetTimeMs64();
		uint64 last_checkmark = start_checkmark;
	
		int reporting_interval = ceil(steps()/100.0f);
		reporting_interval = std::min(reporting_interval,1+(25000000/n_elements()));
		// 2.5x10^7 is a figure chosen to give acceptable initial reporting intervals on the development computer that was used.
		// performance on other computers may vary, but hopefully the adaptive timing will cope with variations anyway so
		// there should be no problem.
	
		// std::cout << "rasegaerhershsdtyui" << std::endl;
		// std::cout << steps() << std::endl;
		// std::cout << reporting_interval << std::endl;
		// std::cout << reporting_interval << std::endl;
		// std::cout << (1) % reporting_interval<< std::endl;
		// std::cout << "ok" << std::endl;
                
		for (int step = 0; step < steps(); ++step) {
		
			// Every step we need to:
			// 0.5) Let plug-ins take any required start-step actions
			// 1) Update every element (each element decides what it needs to do to consitute an update)
			// 1.5) Let plug-ins take any required end-step actions
			// 2) Switch all of the flags in the elements (BUT NOT IN THE LINKS!)
			// 3a) Output any outputs that have been requested and any warnings to std::cout
			// 3b) Output any outputs that have been requested to the output file
			// It may be necessary for speed's sake to put these outputs into arrays or buffers
			// and output only at the end of simulation.

			// 0.5) Plug-in start-step
			interrupt_start_step(step,time);
		
			// 1) Update elements
			for (int i=0; i < n_sections(); ++i) {
				// Note simple model with constant source so no need to update them
				if (!section(i).material().is_source()) {
					for (int j=0; j < section(i).n_elements(); ++j) {
						section(i).element(j).update(delta_t());
					}
				}
			}
			// 1.5) Plug-in end-step
			interrupt_end_step(step,time);
		
			time += delta_t();
		
			// 2) Switch flags in elements
			//this_flag != this_flag;
			//for (int i=0; i < n_elements(); ++i) {
			//	element(i).flagAB(this_flag);
			//}
			//
			// 3)
			if (next_time_recording < time) {
				for (int i=0; i < tracked_elements()->size(); ++i) {
					trackingfile_ << (tracked_element(i)) << ", " << time
					              << ", " << element(tracked_element(i)-1).contamination()
						//<< ", " << element(tracked_element(i)-1).contamination(false)
						//<< ", " << element(tracked_element(i)-1).contamination(true)
					              << std::endl;
				}
				next_time_recording += tracking_interval();
			}
			// for (int i=0; i < n_elements(); ++i)
			// 	element(i).debug_contamination();
			double remaining_time, elapsed_time;
			if ((step+1) % reporting_interval == 0) {

				++history_index;
				history_index %= trail_length;

				uint64 now = GetTimeMs64();
				step_times[history_index] = (now - last_checkmark)/(1.0*(reporting_interval*n_elements()));
				history_count = std::min(trail_length,++history_count);
				elapsed_time = (now - start_checkmark)*1e-3;
				// Added an extra 50% on the prediction, because it almost always underestimates
				// This is as a result of the assumed initial sparseness of the model.
				remaining_time = n_elements() * average_historic_time(step_times, history_count) * 1.5 * (0.001) * (steps() - step);
				/*
				  std::cout << "remaining_time = " << 1.5 << " * " << (0.001) << " * " << average_historic_time(step_times, history_count) << " * (" << steps() << " - " << step << ") * " << n_elements() << std::endl;
				  std::cout << "remaining_time = " << 1.5 * (0.001) << " * " << average_historic_time(step_times, history_count) << " * (" << (steps() - step) << ") * " << n_elements() << std::endl;
				  std::cout << "remaining_time = " << 1.5 * (0.001) * average_historic_time(step_times, history_count) << " * " << ((steps() - step) * n_elements()) << std::endl;
				  std::cout << "remaining_time = " << 1.5 * (0.001) * average_historic_time(step_times, history_count) * ((steps() - step) * n_elements()) << std::endl;
				*/
				
				LOG(simulation_output, "Step: "
				    << std::right << std::setw(12) << (step+1) << "/"
				    << std::left << std::setw(12) << steps() << std::left
				    << "        sim time: "
				    << std::setw(13) << format_time(time)
				    << "        elapsed: "
				    << std::setw(13) << format_time(elapsed_time)
				    << "        remaining (est.): "
				    << std::setw(13) << format_time(remaining_time)
				    << "        total (est.): "
				    << std::setw(13) << format_time(elapsed_time+remaining_time));
				Fl::check();
				// Add in some smoothed adaptive behaviour - aim for every ~2 seconds
				double r = (now - last_checkmark)/2000.0;
				r -= 1;
				r /= history_count*2;
				r += 1;
				r = std::fmax(0.5,std::fmin(r,2.0));
				reporting_interval = std::ceil(reporting_interval/r);
				last_checkmark = now;
						
			}
		}
		// Out of the element-for-loops and the time-for-loops, allow plug-ins
		// to take the post simulation actions
		interrupt_post_simulation();

		// Show how long the entire simulation took
		double elapsed_time = (GetTimeMs64() - start_checkmark)*1e-3;
		LOG(simulation_output, "Total elapsed time: " << format_time(elapsed_time));;
	
		trackingfile_.close();

		// Contaminations: final values at all elements
		contaminations_file_address_ = contaminations_file_address();
		contaminationsfile_.exceptions(ofstream::failbit | ofstream::badbit);
		contaminationsfile_.open(contaminations_file_address_.c_str());
		contaminationsfile_ << "model: " << basename_ << "; config: " << configname_ << "; delta_t: "
		                    << delta_t() << "; time steps: " << steps() << "; final time: " << time
		                    << "s" << std::endl;
		contaminationsfile_ << "element, x, y, z, contamination" << std::endl;
		contaminationsfile_ << std::scientific;
		for (int i=0; i < n_elements(); ++i)
			contaminationsfile_ << (i+1) << ", " << element(i).origin(0) << ", " << element(i).origin(1)
			                    << ", " << element(i).origin(2) << ", " << element(i).contamination()
			                    << std::endl;
		contaminationsfile_.close();
	} catch (ofstream::failure& e) {
		// These files can only be open if the exception was thrown while they were being built up
		// so if they are open, we might as well get rid of the incomplete file after we close them.
		LOGMULTI(warnings, "File exception whilst running simulation:" << std::endl
		    << e.what());
		if (!usedTrackingFile) {
			LOG(warnings, "Error is probably in accessing '" << tracking_file_address_ << "'.");
		}
		else {
			LOG(warnings, "Error is probably in accessing '" << contaminations_file_address_ << "'.");
		}
		if (trackingfile_.is_open()) {
			trackingfile_.close();
			if (remove(tracking_file_address_.c_str()) != 0) {
				LOG(warnings, "Failed to remove possible garbage tracking file.");;
			}
		}
		if (contaminationsfile_.is_open()) {
			contaminationsfile_.close();
			if (remove(contaminations_file_address_.c_str()) != 0) {
				LOG(warnings, "Failed to remove possible garbage contaminations file.");
			}
		}
	} catch (Errors::LowSimulationAccuracyException& e) {
		LOG(warnings, e.what());
	}
}

void tds_run::clear_units() {
	if (units_set_) {
		delete(&units());
		units_set_ = false;
	}
}
void tds_run::set_units_from_file(const char* units_file_address_) {

	if (!units_set_) {
		units(new conversion(units_file_address_,&units_processing_output));
		units_set_ = true;
	} else {
		LOG(warnings,"Units have already been set, but the program has asked to set them again. Request ignored.");
	}
	
}

void tds_run::initialise() {
	
	int progress;
	uint64 now;
	uint64 checkmark = GetTimeMs64();

	// Before we can read in any data, we should look for
	// any settings about (measurement) units we can find
	set_units_from_file(units_file_address().c_str());
	// cout << "7.85 g/cm^3 in SI units is: " << units().convert_density_from("g/cm^3",7.85f) << std::endl;
	
	// Now read in data
	// First open all the necessary files (no point in getting half way through
	// processing only to find a file we need is missing)
	{
		// We start a new scope so that the fstream destructors are called as soon as
		// we are done with them.
		std::ifstream materialsfile_;
		std::ifstream sectionsfile_;
		std::ifstream nodesfile_;
		std::ifstream elementsfile_;
		
		materialsfile_.open(materials_file_address());
		if (!materialsfile_.good()) throw Errors::MissingInputDataException(std::string(materials_file_address()) + " couldn't be opened.");
		sectionsfile_.open(sections_file_address());
		if (!sectionsfile_.good()) throw Errors::MissingInputDataException(std::string(sections_file_address()) + " couldn't be opened.");
		elementsfile_.open(elements_file_address());
		if (!elementsfile_.good()) throw Errors::MissingInputDataException(std::string(elements_file_address()) + " couldn't be opened.");
		nodesfile_.open(nodes_file_address());
		if (!nodesfile_.good()) throw Errors::MissingInputDataException(std::string(nodes_file_address()) + " couldn't be opened.");

		int materials_count, sections_count, elements_count, nodes_count;
	
		std::string line;
		
		//Let's start off by populating tds with some materials
		if (std::getline(materialsfile_, line)) {
			std::istringstream sizess(line);
			if (!(sizess >> materials_count)) {
				// This doesn't require a throw, should probabling be in a warning() method
				LOG(warnings, "Materials count not found, segmentation violations will occur after "
				    << "vector expands to accommodate many materials.");
			} else {
				LOG(run_file_processing_output, "Expanding materials vector to accommodate " << materials_count
				    << " materials.");
				expected_materials(materials_count+3); // not forgetting error, source and outgassing = +3
			}
			while (std::getline(materialsfile_, line)) {
				std::istringstream iss(line);
				std::string _name, _density_unit, _diffusion_constant_unit;
				double _density, _diffusion_constant;
		
				if (!(iss >> _name >> _density >> _density_unit >> _diffusion_constant >> _diffusion_constant_unit)) {
					LOG(tds_log, "Invalid line, skipping. (material)");
					continue;
				}

				//std::cout << "We obtained five values [" << _name << ", " << _density << ", " << _density_unit << ", " << _diffusion_constant << ", " << _diffusion_constant_unit << "].\n";
		
				// let's make all names lower case
				std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
		
				tds_material* _m = new tds_material(_name, units().convert_density_from(_density_unit,_density), units().convert_diffusion_constant_from(_diffusion_constant_unit,_diffusion_constant));
				material_identifier _id;
				_id.material_id = add_material(_m);
				_id.material = _m;
				interrupt_material(_id);
			
			}
			if (materialsfile_.bad())
				throw Errors::MissingInputDataException("Error occurred during reading from " + std::string(materials_file_address()));
		} else {
			throw Errors::MissingInputDataException("No data read from " + std::string(materials_file_address()) + ". Is the config name good?");
		}
		if (n_materials() == 0)
			throw Errors::MissingInputDataException("No materials found in " + std::string(materials_file_address()) + ". Is the config name good?");
	
		// we'll add an error material for when a bad name is given
		// and source/outgassings materials for the regions which will
		// behave in a special manner. UPDATE: these last two are
		// being moved to the plug-ins which will use them.
		add_material(new tds_material("error", 1.0, 0.0));
		add_material(new tds_material("source", 1.0, 0.0));
	
		//Now we've got materials, let's get the physical sections which use them
		if (std::getline(sectionsfile_, line)) {
			std::istringstream sizess(line);
			if (!(sizess >> sections_count)) {
				LOG(warnings,"Sections count not found, segmentation violations will occur after "
				    << "vector expands to accommodate many sections.");
			} else {
				LOG(tds_log, "Expanding sections vector to accommodate "
				    << sections_count << " sections.");
				expected_sections(sections_count);
			}
			while (std::getline(sectionsfile_, line)) {
				std::istringstream iss(line);
				int _dim, _id;
				std::string _name;
		
				if (!(iss >> _dim >> _id >> _name)) {
					LOG(tds_log, "Invalid line, skipping. (section)");
					continue;
				}
				{
					std::string _extra;
					while (iss >> _extra) {
						_name += " " + _extra;
					}
				}
		
				// Get rid of the double quotes Gmsh puts in, and put to lower case
				_name.erase(_name.end()-1); _name.erase(_name.begin());
				std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);

				if (_id != n_sections()+1)
					throw Errors::BadInputDataException("Section ordering invalid - maybe adding the same file twice.");
				tds_section* _s = new tds_section(_name, &material(_name));
				section_identifier _s_id;
				_s_id.section_id = add_section(_s);
				_s_id.section = _s;
				interrupt_section(_s_id);
			
				LOG(tds_log, "Now " << n_sections() << " sections.");
			}
			if (sectionsfile_.bad())
				throw Errors::MissingInputDataException("Error occurred during reading from " + std::string(sections_file_address()));
		} else {
			throw Errors::MissingInputDataException("No data read from " + std::string(sections_file_address()) + ". Is the model name good?");
		}
		if (n_sections() == 0)
			throw Errors::MissingInputDataException("No sections found in " + std::string(sections_file_address()) + ". Is the model name good?");
	
		//Next we'll get some nodes in
		if (std::getline(nodesfile_, line)) {
			std::istringstream sizess(line);
			if (!(sizess >> nodes_count)) {
				// This requires a warning()
				LOG(warnings, "Nodes count not found, segmentation violations will occur after "
				    << "vector expands to accommodate many nodes.");
			} else {
				LOG(tds_log, "Expanding nodes vector to accommodate "
				    << nodes_count << " nodes.");
				expected_nodes(nodes_count);
			}
			while (std::getline(nodesfile_, line)) {
				std::istringstream iss(line);
				int id;
				double _x, _y, _z;
		
				if (!(iss >> id >> _x >> _y >> _z)) {
					LOG(tds_log, "Invalid line, skipping. (node)");
					continue;
				}

				//std::cout << "We obtained four values [" << id << ", " << _x << ", " << _y << ", " << _z << "].\n";

				if (id != n_nodes()+1)
					throw Errors::BadInputDataException("Node ordering invalid - maybe adding the same file twice.");
				tds_node* _n = new tds_node(_x,_y,_z);
				node_identifier _n_id;
				_n_id.node_id = add_node(_n);
				_n_id.node = _n;
				interrupt_node(_n_id);
			}
			if (nodesfile_.bad())
				throw Errors::MissingInputDataException("Error occurred during reading from " + std::string(nodes_file_address()));
		} else {
			throw Errors::MissingInputDataException("No data read from " + std::string(nodes_file_address()) + ". Is the model name good?");
		}
		if (n_nodes() == 0)
			throw Errors::MissingInputDataException("No nodes found in " + std::string(nodes_file_address()) + ". Is the model name good?");
	
		//Now finally we'll read the elements
		if (std::getline(elementsfile_, line)) {
			std::istringstream sizess(line);
			if (!(sizess >> elements_count)) {
				LOG(warnings, "Elements count not found, segmentation violations will occur after "
				    << "vector expands to accommodate many elements.");
			} else {
				LOG(tds_log, "Expanding elements vector to accommodate "
				    << elements_count << " elements.");
				expected_elements(elements_count);
			}
			while (std::getline(elementsfile_, line)) {
				std::istringstream iss(line);
				int id, type, n_tags, section_id, entity_id;
				std::ostringstream extra_tags;
		
				if (!(iss >> id >> type >> n_tags)) {
					LOG(tds_log, "Invalid line, skipping. (element)");
					continue;
				}
				register_element_type(type);
				if (id != n_elements()+1)
					throw Errors::BadInputDataException("Element ordering invalid - maybe adding the same file twice");
				int tags_processed = 0;
				iss >> section_id >> entity_id;
				for (int tags_processed = 2; tags_processed < n_tags; tags_processed++) {
					extra_tags << iss << " ";
				}
				if (extra_tags.str().length() > 0)
					LOG(warnings,"Unprocessed node tags: " << extra_tags.str());
				tds_nodes _element_nodes;
				int this_node;
				while (iss >> this_node) {
					_element_nodes.push_back(&(node(this_node-1))); // don't forget that msh is 1-indexed and arrays are 0-indexed
				}
				tds_element* new_element = new tds_element(_element_nodes,&(section(section_id-1)),0.0);
				element_identifier _e_id;
				_e_id.element_id = add_element(new_element);
				_e_id.section_id = section_id-1;
				_e_id.section_element_id = section(section_id-1).add_element(new_element);
				_e_id.element = new_element;
				interrupt_element(_e_id);
			
				// the interruption could change the element pointer e.g. if it
				// replaces the element with a derived class, so we should read
				// it back again:
				new_element = _e_id.element;

				// Once the element has been created, and plugins have had a chance to play around with it, it should be placed in the nodes that form it
				(*new_element).propogate_into_nodes();

				//std::cout << "We obtained some values [" << id << ", " << type << ", " << n_tags << ", " << section_id << ", " << entity_id << "].\n";
			}
			if (elementsfile_.bad())
				throw Errors::MissingInputDataException("Error occurred during reading from " + std::string(elements_file_address()));
		} else {
			throw Errors::MissingInputDataException("No data read from " + std::string(elements_file_address()) + ". Is the model name good?");
		}
		if (n_elements() == 0)
			throw Errors::MissingInputDataException("No elements found in " + std::string(elements_file_address()) + ". Is the model name good?");
	
		sectionsfile_.close();
		elementsfile_.close();
		nodesfile_.close();
	} // end the file reading scope; files will close now even if exception was thrown.

	// Every element now has a list of nodes that it make it,
	// and every node has a list of every element that makes
	// use of it. In 1-D we may now simply make our way through
	// the list of elements, adding links and removing from
	// each node's list of elements as we go
	
	LOG(tds_log, "Producing element links");
	int element_link_count = 0;
	if (!(element_dimensions(two_d) || element_dimensions(three_d) || element_dimensions(second_order_or_worse)) &&
	    (element_dimensions(one_d))
	    ) {
		// We don't want to use the global elements list, but instead
		// iterate through each section's list of elements. This allows
		// us to treat source and/or outgassing sections differently
		// from the rest.
		int n, m, o, t = 0;
		for (int s = 0; s < n_sections(); s++ ) {
			n = section(s).n_elements();
			t += n;
			if (section(s).material().is_source()) {
				for (int i=0; i < n; i++) {
					m = section(s).element(i).n_nodes();
					for (int j=0; j < m; j++) {
						o = section(s).element(i).node(j).n_elements();
						for (int k=o; k > 0; k--) {
							if (section(s).element(i).node(j).element(k-1).material().is_source()) {
								// std::cout << "No need to give a link from source to source, skipping." << std::endl;
							} else if (&section(s).element(i) == &(section(s).element(i).node(j).element(k-1))) {
								// std::cout << "Skipping - don't need to link to self!" << std::endl;
							} else {
								// std::cout << "Making link from " << &section(s).element(i) << " to "
								//           << &(section(s).element(i).node(j).element(k-1)) << std::endl;
								tds_element_link* new_link = new tds_element_link(&section(s).element(i),
								                                                  &(section(s).element(i).node(j).element(k-1)));
								element_link_count++;
								// std::cout << "Made link" << std::endl;
								element_link_identifier _el_id;
								_el_id.element_link = new_link;
								interrupt_element_link(_el_id);
								// Reread pointer in case changed by interruption:
								new_link = _el_id.element_link;
								section(s).element(i).add_element_link(new_link);
								section(s).element(i).node(j).element(k-1).add_element_link(new_link);
							}
							section(s).element(i).node(j).remove_last_element();
						}
					}
				}
			} else {
				for (int i=0; i < n; i++) {
					m = section(s).element(i).n_nodes();
					for (int j=0; j < m; j++) {
						o = section(s).element(i).node(j).n_elements();
						// std::cout << "Element " << &section(s).element(i) << " links to node "
						//           << &(section(s).element(i).node(j)) << " which currently links to "
						//           << o << " more elements." << std::endl;
						for (int k=o; k > 0; k--) {
							if (&section(s).element(i) == &(section(s).element(i).node(j).element(k-1))) {
								//std::cout << "Skipping - don't need to link to self!" << std::endl;
							} else {
								// std::cout << "Making link from " << &section(s).element(i) << " to "
								//           << &(section(s).element(i).node(j).element(k-1)) << std::endl;
								tds_element_link* new_link = new tds_element_link(&section(s).element(i),
								                                                  &(section(s).element(i).node(j).element(k-1)));
								element_link_count++;
								// std::cout << "Made link" << std::endl;
								element_link_identifier _el_id;
								_el_id.element_link = new_link;
								interrupt_element_link(_el_id);
								// Reread pointer in case changed by interruption:
								new_link = _el_id.element_link;
								section(s).element(i).add_element_link(new_link);
								section(s).element(i).node(j).element(k-1).add_element_link(new_link);
							}
							section(s).element(i).node(j).remove_last_element();
						}
					}
				}
			}
		}
		LOG(tds_log, t << " elements trawled for links.");
		LOG(tds_log, "Made " << element_link_count << " element link objects.");
	} else if (!(element_dimensions(three_d) || element_dimensions(second_order_or_worse)) &&
	           (element_dimensions(two_d))
	           ) {

		// 2D is nice because whatever shape the element is,
		// it can only build an interface from two nodes.
		
		int n, m, o, p, t = 0, c;
		for (int s = 0; s < n_sections(); s++ ) {
			LOG(tds_log, "Starting section " << s << " (" << section(s).name() << ")");
			n = section(s).n_elements();
			t += n;
			//if (section(s).material().is_source()) {
				// probably do something slightly different here as more exclusions can be made for elements in source sections
			//} else {
			{
				for (int i=0; i < n; ++i) {
					//std::cout << "Starting i of " << i << " i.e. element " << &section(s).element(i) << std::endl;
					m = section(s).element(i).n_nodes();
					for (int j=0; j < m; ++j) {
						//std::cout << "\tStarting j: node " << j << std::endl;
						o = section(s).element(i).node(j).n_elements();
						for (int k=0; k < o; ++k) {
							//std::cout << "\t\tStarting k of " << k << " i.e. element " << &section(s).element(i).node(j).element(k) << std::endl;
							// don't try and link to yourself, or to something already linked:
							if (
							    (&section(s).element(i) != &section(s).element(i).node(j).element(k)) &&
							    (section(s).element(i).is_linked_to(
							                                         ( &section(s).element(i).node(j).element(k) )
							                                         )==false)
							    ) {
								p = section(s).element(i).node(j).element(k).n_nodes();
								c = 1; // we know that one node is the same already
								for (int l=0; (l < p && c < 2); ++l) { // we only need two common nodes to connect elements in 2D
									// ignore that one node
									if (&section(s).element(i).node(j) != &section(s).element(i).node(j).element(k).node(l)) {
										for (int q=0; (q < m && c < 2); ++q) {
											if (&section(s).element(i).node(j).element(k).node(l) == &section(s).element(i).node(q)) {
												++c;
											}
										}
									}
								}
								if (c >= 2) {
									// std::cout << "Making link from " << &section(s).element(i) << " to "
									//           << &(section(s).element(i).node(j).element(k)) << std::endl;
									tds_element_link* new_link = new tds_element_link(&section(s).element(i),
									                                                  &(section(s).element(i).node(j).element(k)));
									element_link_count++;
									// std::cout << "Made link" << std::endl;
									element_link_identifier _el_id;
									_el_id.element_link = new_link;
									interrupt_element_link(_el_id);
									// Reread pointer in case changed by interruption:
									new_link = _el_id.element_link;
									section(s).element(i).add_element_link(new_link);
									section(s).element(i).node(j).element(k).add_element_link(new_link);
								}									
							}
							// std::cout << "\t\tFinished with k of " << k << std::endl;
						}
					}
				}
			}
		}
		
	} else if (!element_dimensions(second_order_or_worse) &&
	           (element_dimensions(three_d))
	           ) {

		// 3D is less nice because the interface may have any number
		// of nodes from 3 upwards. Still, let's get cracking...

		int n, m, o, p, t = 0, c;
		for (int s = 0; s < n_sections(); s++ ) {
			LOG(tds_log, "Starting section " << s << " (" << section(s).name() << ")");
			n = section(s).n_elements();
			t += n;
			for (int i=0; i < n; ++i) {
				// std::cout << "Starting i of " << i << " i.e. element " << &section(s).element(i) << std::endl;
				m = section(s).element(i).n_nodes();
				for (int j=0; j < m; ++j) {
					// std::cout << "\tStarting j: node " << j << std::endl;
					o = section(s).element(i).node(j).n_elements();
					for (int k=0; k < o; ++k) {
						// std::cout << "\t\tStarting k of " << k << " i.e. element " << &section(s).element(i).node(j).element(k) << std::endl;
						// don't try and link to yourself, or to something already linked:
						if (
						    (&section(s).element(i) != &section(s).element(i).node(j).element(k)) &&
						    (section(s).element(i).is_linked_to(
											( &section(s).element(i).node(j).element(k) )
											)==false)
						    ) {
							p = section(s).element(i).node(j).element(k).n_nodes();
							// c is a count of the common nodes
							c = 1; // we know that one node is the same already
							for (int l=0; (l < p); ++l) {
								// ignore that one node
								if (&section(s).element(i).node(j) != &section(s).element(i).node(j).element(k).node(l)) {
									for (int q=0; (q < m); ++q) {
										if (&section(s).element(i).node(j).element(k).node(l) == &section(s).element(i).node(q)) {
											++c;
										}
									}
								}
							}
							if (c >= 3) {
								// std::cout << "Making link from " << &section(s).element(i) << " to "
								//            << &(section(s).element(i).node(j).element(k)) << std::endl;
								tds_element_link* new_link = new tds_element_link(&section(s).element(i),
														  &(section(s).element(i).node(j).element(k)));
								element_link_count++;
								// std::cout << "Made link" << std::endl;
								element_link_identifier _el_id;
								_el_id.element_link = new_link;
								interrupt_element_link(_el_id);
								// Reread pointer in case changed by interruption:
								new_link = _el_id.element_link;
								section(s).element(i).add_element_link(new_link);
								section(s).element(i).node(j).element(k).add_element_link(new_link);
							}
							// std::cout << "found " << c << " nodes in common." << std::endl;
						} else {
							// std::cout << "skipped: k="<<k<<";j="<<j<<";i="<<i << std::endl;
						}
					}
				}
			}
		}
		
	} else {
		throw Errors::BadInputDataException("Element links could not be made, only linear in 1-D, 2-D and 3-D programmed.");
	}

	// There will be some elements which have not produced any links. These will not take
	// part in the simulation as a result. These come about from
	//  a) elements in the .msh file of 1-D or 2-D (or 0-D... silly gmsh) when a 2-D or
	//     3-D (or 1-D) mesh was present
	//  b) weirdly defined meshes with e.g. a single floating element
	// To avoid wasting memory and time (update will still get called for these elements,
	// but won't do anything) we will clean (remove) these.
	clean_inactive_elements();

	//output_model_summary(true,true,true,true,true);
	output_model_summary(false,false,false,false,false);

	if (settings.tracking_mode == "all") {
		settings.tracking_list = new std::vector<int>();
		for (int i = 1; i <= n_elements(); ++i)
			settings.tracking_list->push_back(i);
	} else if (settings.tracking_mode == "first-last") {
		settings.tracking_list = new std::vector<int>();
		int i;
		for (i = 1; (i <= settings.tracking_n && i <= n_elements()); ++i) {
			settings.tracking_list->push_back(i);
		}
		for (i = std::max(std::max(1,n_elements()+1-settings.tracking_n),i); (i <= n_elements()); ++i) {
			settings.tracking_list->push_back(i);
		}
	}
	now = GetTimeMs64();
	progress = (now - checkmark)/1000;
	LOG(run_file_processing_output, "Initialisation took " << progress << " seconds.");
}
void tds_run::direct_simulation_output_to_cout() {
	simulation_output.add_listener(&console_out);
}

void tds_run::process_plugins() {
	
	IPlugin::set_run(this);

	// Define a map of the strings that can be placed after 'activate-plugin' in
	// the .run file. Note that multiple strings could point to the same plugin
	// e.g. for Am.E. vs Br.E. spellings
	std::map<std::string,plugin> plugin_strings;
	plugin_strings["outgassing"] = POutgassing;
	plugin_strings["decay"] = PDecay;
	plugin_strings["example"] = PExample;
	
	// Check for known conflicts from the combinations of plugins
	// (none known at this time - code written before any plugins existed!)


	// Start adding plugins to the map in IPlugin and initialising them.
	for (int i=0; i < settings.activated_plugins.size(); ++i) {
		std::string plugin_text = settings.activated_plugins.at(i);
		if (plugin_strings.count(plugin_text) == 0) {
			throw Errors::PluginNotInMapException(plugin_text);
		}
		switch (plugin_strings[plugin_text]) {
		case PExample:
			IPlugin::store_plugin(new Example()); break;
		case POutgassing:
			IPlugin::store_plugin(new Outgassing()); break;
		default:
			throw Errors::PluginNotInSwitchException(plugin_text);
		}
	}
	for (std::map<plugin,IPlugin*>::iterator it = IPlugin::get_plugin_iterator();
	     it!=IPlugin::get_plugin_iterator_end();
	     ++it)
		it->second->load_plugin();
}

void tds_run::read_run_file(std::string run_file_name) {

	uint64 checkmark = GetTimeMs64();
	uint64 now;
	int progress;
	ensure_ending(run_file_name,".run");
        LOG(run_file_processing_output,"Using instruction file: '" << run_file_name << "'");

	// Set defaults before reading from the file
	settings = run_settings();
	settings.model_name = "simple2d";
	settings.config_name = "simple";
	settings.output_name = "output";
	settings.delta_t = 3600.0*24.0;
	settings.tracking_interval = 3600*24*365.24;
	settings.simulation_length = 3600*24*30.0;
	settings.tracking_list = new std::vector<int>();
	
	
	//Run file reading not yet fully implemented!

	// most importantly, the checks to make sure that the user has
	// specified all the required fields in their .run file before
	// we go ahead with the simulation are not yet in place.

	// Let's start off by
	//populating tds with some materials
        std::ifstream run_file_(run_file_name);
        std::string line, setting, value;
        int version;
        std::istringstream line_processing;
	if (std::getline(run_file_, line)) {
		line_processing.str(line);
		if (!(line_processing >> setting >> value)) {
			std::ostringstream oss; oss << "Expecting run mode, received:\n\t"
					       << line << std::endl;
                        throw Errors::BadRunFileException(oss.str());
		} else {
			LOG(run_file_processing_output,"Found request for setting '" << setting << "' with '" << value << "'");
			if (setting != "mode") {
				throw Errors::BadRunFileException("First instruction was not the run mode, instead found '" + setting + "'.");
			} else {
				// Should probably give a warning() or something about the
				// fact that run mode is not actually used...
			}
		}
	} else {
                throw Errors::BadRunFileException("No instructions found.");
	}
	line_processing.clear();
	if (std::getline(run_file_, line)) {
		line_processing.str(line);
		if (!(line_processing >> setting >> value)) {
			std::ostringstream oss; oss << "Expecting run file version, received:\n\t"
					       << line << std::endl;
                        throw Errors::BadRunFileException(oss.str());
		} else {
			LOG(run_file_processing_output,"Found request for setting '" << setting << "' with '" << value << "'");
			if (setting != "version") {
				throw Errors::BadRunFileException("Second instruction was not the run file version, instead found '" + setting + "'.");
			} else {
				// Should give a warning() about the lack of run file
				// versioning in place
			}
		}
	} else {
                throw Errors::BadRunFileException("No instructions found except run mode.");
        }
	line_processing.clear();
	int line_n = 2;
	bool config_given = false;
	while (std::getline(run_file_, line)) {
		line_processing.str(line);
		++line_n;
		// This if statement reads in another line of the instruction file
		// However, it works slightly differently from the lines for mode and
		// version, because there may be more than three words - the 'value'
		// could consist of "30 seconds" for example, or even a list of
		// element IDs for tracking. std::getline(in,out) puts the whole of
		// rest of the line after just the 'setting' part has been taken out
		// by the first statement.
		if (!(line_processing >> setting && std::getline(line_processing,value))) {
			std::ostringstream oss; oss << "Confused when separating out key and value from line " << line_n << ":\n\t" << line_processing.str();
			throw Errors::BadRunFileException(oss.str());
		} else {
			trim(value);
		}
		std::istringstream interpreter;
		// Following is the list of available instructions in a .run file
                if (setting == "models-directory") {
	                ensure_ending(value, "/");
	                LOG(run_file_processing_output,"Setting the models directory: " << value);
	                settings.model_directory = value;
                } else if (setting == "config-directory") {
	                ensure_ending(value, "/");
	                LOG(run_file_processing_output,"Setting the config directory: " << value);
	                settings.config_directory = value;
                } else if (setting == "output-directory") {
	                ensure_ending(value, "/");
	                LOG(run_file_processing_output,"Setting the output directory: " << value);
	                settings.output_directory = value;
                } else if (setting == "gmsh-bin-directory") {
	                ensure_ending(value, "/");
	                LOG(run_file_processing_output,"Setting the gmsh directory: " << value);
	                settings.gmsh_bin_directory = value;
                } else if (setting == "model-name") {
	                LOG(run_file_processing_output,"Setting the model name: " << value);
	                settings.model_name = value;
                } else if (setting == "config-name") {
	                LOG(run_file_processing_output,"Setting the config name: " << value);
	                settings.config_name = value;
	                config_given = true;
	                LOG(run_file_processing_output,"Setting the units from " << units_file_address());  
	                clear_units();
	                set_units_from_file(units_file_address().c_str());
                } else if (setting == "output-name") {
	                LOG(run_file_processing_output,"Setting the output name: " << value);
	                settings.output_name = value;
                } else if (setting == "delta-t") {
	                LOG(run_file_processing_output,"Setting the time step: " << value);
	                if (config_given) {
		                std::string unit;
		                interpreter.str(value);
		                if (!(interpreter >> settings.delta_t >> unit)) {
			                throw Errors::BadRunFileException("Confused by a value of '" + value + "' for the time step.");
		                } else {
			                settings.delta_t = units().convert_time_from(unit,settings.delta_t);
		                }
	                } else
				throw Errors::EarlyRunFileUnitsException(line);
                } else if (setting == "tracking-interval") {
	                LOG(run_file_processing_output,"Setting the time interval for recording contaminations: " << value);
	                if (config_given) {
		                std::string unit;
		                interpreter.str(value);
		                if (!(interpreter >> settings.tracking_interval >> unit)) {
			                throw Errors::BadRunFileException("Confused by a value of '" + value + "' for the tracking interval.");
		                } else {
			                settings.tracking_interval = units().convert_time_from(unit,settings.tracking_interval);
		                }
	                } else
				throw Errors::EarlyRunFileUnitsException(line);
                } else if (setting == "simulation-length") {
	                LOG(run_file_processing_output,"Setting the minimum total simulation time: " << value);
	                if (config_given) {
		                std::string unit;
		                interpreter.str(value);
		                if (!(interpreter >> settings.simulation_length >> unit)) {
			                throw Errors::BadRunFileException("Confused by a value of '" + value + "' for the simulation length.");
		                } else
			                settings.simulation_length = units().convert_time_from(unit,settings.simulation_length);
	                } else
				throw Errors::EarlyRunFileUnitsException(line);
                } else if (setting == "contamination-mode-time") {
	                LOG(run_file_processing_output,"Setting the contamination mode for time: " << value);
	                settings.contamination_mode_time = value;
                } else if (setting == "contamination-mode-space") {
	                LOG(run_file_processing_output,"Setting the contamination mode for space: " << value);
	                settings.contamination_mode_space = value;
                } else if (setting == "contamination") {
	                LOG(run_file_processing_output,"Setting the basic contamination: " << value);
	                if (config_given) {
		                std::string unit;
		                interpreter.str(value);
		                if (!(interpreter >> settings.contamination >> unit)) {
			                throw Errors::BadRunFileException("Confused by a value of '" + value + "' for the source contamination.");
		                } else
			                settings.contamination = units().convert_contamination_from(unit,settings.contamination);
	                } else
				throw Errors::EarlyRunFileUnitsException(line);
                } else if (setting == "contaminations-file") {
	                LOG(run_file_processing_output,"Setting the contaminations file name: " << value);
	                settings.contaminations_file = value;
                } else if (setting == "activate-plugin") {
	                LOG(run_file_processing_output,"Adding a plug-in to activate: " << value);
	                settings.activated_plugins.push_back(value);
                } else if (setting == "plugin-file") {
	                LOG(run_file_processing_output,"Setting the file for a plugin: " << value);
	                std::string plugin;
	                std::string init_only;
	                std::string file;
	                interpreter.str(value);
	                if (!(interpreter >> plugin >> init_only >> file)) {
			                throw Errors::BadRunFileException("Confused by a request for '" + value + "' as a plug-in. Remember: \"plugin-name init-only file-name\". Use 'none' if no file required by the plug-in.");
	                } else {
		                settings.plugin_files[plugin].file_name = file;
		                settings.plugin_files[plugin].needed_after_initialisation = (init_only != "init-only");
	                }
                } else if (setting == "tracking-mode") {
	                LOG(run_file_processing_output,"Setting the tracking mode: " << value);
			// Probably want a warning() or exception here if the tracking_mode
			// doesn't exist.
	                settings.tracking_mode = value;
                } else if (setting == "track") {
	                LOG(run_file_processing_output,"Setting the tracked ids: " << value);
	                interpreter.str(value);
	                if (settings.tracking_mode == "ids") {
		                settings.tracking_list->clear();
		                int id;
		                while (interpreter >> id)
			                settings.tracking_list->push_back(id);
	                } else {
		                if (!(interpreter >> settings.tracking_n))
			                throw Errors::BadRunFileException("Confused by a value of '" + value + "' for the number of elements to track at each end of the elements vector.");
	                }
                } else {
			LOG(warnings,"(Didn't know what to do with '" << setting << "'.)");
		}
                line_processing.clear();
	}
	
	// Set the number of simulation steps from the step size and simulation length
	steps(ceil(settings.simulation_length/settings.delta_t));
	// Now get the simulation to initialise using the data we have brought in
	// from the instructions file

	now = GetTimeMs64();
	progress = (now - checkmark)/1000;
	LOG(run_file_processing_output,"Instructions read in " << progress << " seconds.");
	
}
std::string tds_run::generate_run_file() {
	std::ostringstream oss;
	oss << std::left;
	// First two lines are always the mode (unused) and version (currently writing "one"...)
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "mode" << "simulation" << std::endl;
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "version" << "1" << std::endl;
	// Now let's put in the directories
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "models-directory" << settings.model_directory << std::endl;
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "config-directory" << settings.config_directory << std::endl;
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "output-directory" << settings.output_directory << std::endl;
	// Now the file names
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "model-name" << settings.model_name << std::endl;
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "config-name" << settings.config_name << std::endl;
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "output-name" << settings.output_name << std::endl;
	// Now put in the data that every run file includes
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "delta-t"
	    << units().generate_appropriate_time_input_string(settings.delta_t) << std::endl;
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "tracking-interval"
	    << units().generate_appropriate_time_input_string(settings.tracking_interval) << std::endl;
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "simulation-length"
	    << units().generate_appropriate_time_input_string(settings.simulation_length) << std::endl;
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "contamination-mode-time" << settings.contamination_mode_time << std::endl;
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "contamination-mode-space" << settings.contamination_mode_space << std::endl;
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "contamination"
	    << units().generate_appropriate_contamination_input_string(settings.contamination) << std::endl;
	// Now put in the lines which depend on how many plugins are in use
	for (int i=0; i < settings.activated_plugins.size(); ++i) {
		oss << std::setw(RUN_FILE_KEY_WIDTH) << "activate-plugin" << settings.activated_plugins.at(i) << std::endl;
		if (settings.plugin_files.count(settings.activated_plugins.at(i)) == 1) {
			oss << std::setw(RUN_FILE_KEY_WIDTH) << "plugin-file"
			    << settings.activated_plugins.at(i) << " "
			    << (settings.plugin_files[settings.activated_plugins.at(i)].needed_after_initialisation ? "always" : "init-only" ) << " "
			    << settings.plugin_files[settings.activated_plugins.at(i)].file_name
			    << std::endl;
		}
	}
	// Now put in the tracking mode information
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "tracking-mode" << settings.tracking_mode << std::endl;
	oss << std::setw(RUN_FILE_KEY_WIDTH) << "track";
	if (settings.tracking_mode == "ids") {
		for (int i=0; i < settings.tracking_list->size(); ++i) {
			oss << settings.tracking_list->at(i) << " ";
		}
		oss << std::endl;
	} else {
		oss << settings.tracking_n << std::endl;
	}
	// Now convert it to a string to return
	return oss.str();
}

void tds_run::add_material_interrupt(IPlugin* _interrupter) {
	material_interrupts_.push_back(_interrupter);
}
void tds_run::add_section_interrupt(IPlugin* _interrupter) {
	section_interrupts_.push_back(_interrupter);
}
void tds_run::add_node_interrupt(IPlugin* _interrupter) {
	node_interrupts_.push_back(_interrupter);
}
void tds_run::add_element_interrupt(IPlugin* _interrupter) {
	element_interrupts_.push_back(_interrupter);
}
void tds_run::add_element_link_interrupt(IPlugin* _interrupter) {
	element_link_interrupts_.push_back(_interrupter);
}
void tds_run::add_pre_simulation_interrupt(IPlugin* _interrupter) {
	pre_simulation_interrupts_.push_back(_interrupter);
}
void tds_run::add_start_step_interrupt(IPlugin* _interrupter) {
	step_start_interrupts_.push_back(_interrupter);
}
void tds_run::add_end_step_interrupt(IPlugin* _interrupter) {
	step_end_interrupts_.push_back(_interrupter);
}
void tds_run::add_post_simulation_interrupt(IPlugin* _interrupter) {
	post_simulation_interrupts_.push_back(_interrupter);
}

void tds_run::change_section_pointer(int _i, tds_section* _new_section) {
	if (_i < n_sections()) { sections_[_i] = _new_section; }
	else {
		std::ostringstream oss; oss << "Trying to change section pointer " << _i << " out of " << n_sections() << ".";
		throw Errors::VectorOutOfBoundsException(oss.str());
	}
}
void tds_run::change_material_pointer(int _i, tds_material* _new_material) {
	if (_i < n_materials()) { materials_[_i] = _new_material; }
	else {
		std::ostringstream oss; oss << "Trying to change material pointer " << _i << " out of " << n_materials() << ".";
		throw Errors::VectorOutOfBoundsException(oss.str());
	}
}
void tds_run::change_node_pointer(int _i, tds_node* _new_node) {
	if (_i < n_nodes()) { nodes_[_i] = _new_node; }
	else {
		std::ostringstream oss; oss << "Trying to change node pointer " << _i << " out of " << n_nodes() << ".";
		throw Errors::VectorOutOfBoundsException(oss.str());
	}
}
void tds_run::change_element_pointer(int _i, tds_element* _new_element) {
	if (_i < n_elements()) { elements_[_i] = _new_element; }
	else {
		std::ostringstream oss; oss << "Trying to change element pointer " << _i << " out of " << n_elements() << ".";
		throw Errors::VectorOutOfBoundsException(oss.str());
	}
}

void tds_run::interrupt_material(material_identifier& _new_material) {
	for (int i=0; i < material_interrupts_.size(); ++i)
		material_interrupts_.at(i)->interrupt_material_creation(_new_material);
}
void tds_run::interrupt_section(section_identifier& _new_section) {
	for (int i=0; i < section_interrupts_.size(); ++i)
		section_interrupts_.at(i)->interrupt_section_creation(_new_section);
}
void tds_run::interrupt_node(node_identifier& _new_node) {
	for (int i=0; i < node_interrupts_.size(); ++i)
		node_interrupts_.at(i)->interrupt_node_creation(_new_node);
}
void tds_run::interrupt_element(element_identifier& _new_element) {
	for (int i=0; i < element_interrupts_.size(); ++i)
		element_interrupts_.at(i)->interrupt_element_creation(_new_element);
}
void tds_run::interrupt_element_link(element_link_identifier& _new_element_link) {
	for (int i=0; i < element_link_interrupts_.size(); ++i)
		element_link_interrupts_.at(i)->interrupt_element_link_creation(_new_element_link);
}
void tds_run::interrupt_pre_simulation() {
	for (int i=0; i < pre_simulation_interrupts_.size(); ++i)
		pre_simulation_interrupts_.at(i)->interrupt_pre_simulation();
}
void tds_run::interrupt_start_step(int _step, double _time) {
	for (int i=0; i < step_start_interrupts_.size(); ++i)
		step_start_interrupts_.at(i)->interrupt_start_step(_step,_time);
}
void tds_run::interrupt_end_step(int _step, double _time) {
	for (int i=0; i < step_end_interrupts_.size(); ++i)
		step_end_interrupts_.at(i)->interrupt_end_step(_step,_time);
}
void tds_run::interrupt_post_simulation() {
	for (int i=0; i < post_simulation_interrupts_.size(); ++i)
		post_simulation_interrupts_.at(i)->interrupt_post_simulation();
}









/******************** TDS_DISPLAY METHODS ********************/









tds_display::tds_display(UserInterface *gui):GUI_(gui){

	// Initialise buffers
	gui_status = MessageBuffer(MBGuiStatus,"STATUS      ");
	gui_alerts = MessageBuffer(MBGuiStatus,"ALERTS      ");
	gui_actions = MessageBuffer(MBGuiActions,"GUI         ");
	
	// Set buffers etc in main window
	GUI_->txdsp_run_file_name->buffer(BRunFileName);
	GUI_->txedt_run_file_contents->buffer(BRunFileContents);
	GUI_->txdsp_model_dir->buffer(BModelDirectory);
	GUI_->txdsp_model_name->buffer(BModelName);
	GUI_->txdsp_settings_dir->buffer(BSettingsDirectory);
	GUI_->txdsp_settings_name->buffer(BSettingsName);
	GUI_->txdsp_output_dir->buffer(BOutputDirectory);
	GUI_->txdsp_output_name->buffer(BOutputName);
	GUI_->txdsp_status_bar->buffer(BStatusBar); // just while testing don't kill me!
	GUI_->txdsp_data_dirty->buffer(BDataDirty);
	// Set buffers etc in new files window
	GUI_->txdsp_new_model_dir->buffer(BModelDirectory);
	GUI_->txdsp_new_settings_dir->buffer(BSettingsDirectory);
	GUI_->txdsp_new_output_dir->buffer(BOutputDirectory);
	GUI_->txedt_new_output_name->buffer(BOutputName);

	gui_console.browser(GUI_->brwsr_run_output);

	// Prepare the tds_display messaging systems
	//   - those stored in tds_display
	gui_status.add_listener(&BStatusBar);
	gui_status.add_listener(&console_out);
	gui_status.add_listener(&gui_console);
	gui_alerts.add_listener(&console_out);
	gui_alerts.add_listener(&gui_console);
        gui_actions.add_listener(&console_out);
	//   - those stored in tds_run
	run_file_processing_output.add_listener(&gui_console);
	units_processing_output.add_listener(&gui_console);
	simulation_output.add_listener(&gui_console);
	//   - those stored globally
	warnings.add_listener(&BStatusBar);
	warnings.add_listener(&gui_console);
	warnings.add_listener(&console_err);
	exceptions.add_listener(&BStatusBar);
	exceptions.add_listener(&gui_console);
	debugging.add_listener(&gui_console);
	
	LOG(gui_status,"Status bar initiated");
	
	
	// read in previously used file name (probably from config file)
	std::string run_file_name = "example.run";
	run_file(run_file_name.c_str());

	populate_from_run_file();
	
}

tds_display::~tds_display(){
	// GUI_->plotH->clear();
}

void tds_display::open_run_file_dialog(){
	if (!previous_settings_were_saved()) {
		switch (fl_choice("The current settings are not saved in a '.run' file. Are you sure you wish to overwrite them by loading a new '.run' file?","No, cancel","Yes, overwrite",0)) {
		case 1:
			break;
		default:
			return;
		}
	}
	const char *filePtr = fl_file_chooser("Input File","Run Files (*.run)","",0);
	if(filePtr) {
		if (!get_file_exists(std::string(filePtr))) {
			// either needs a warning() or to change back to previous result, showing
			// an error box in the UI
			std::cerr << "Chosen file can no longer be found." << std::endl;
		} else {
			run_file(filePtr);
			populate_from_run_file();
		}
	}
}
void tds_display::save_run_file() {
	if (previous_settings_were_saved()) {
		fl_alert("No changes have been made");
	} else {
		BRunFileContents.savefile(run_file().c_str());
		mark_data_clean();
	}
}
void tds_display::preview_run_file() {
	// This will actually be used to show the run file window which will
	// contain a table with all the fields and values in, but for now,
	// it is creating the run file and putting it in the text editor in
	// the main window.
	std::string new_run_file = generate_run_file();
	BRunFileContents.text(new_run_file.c_str());
	populate_preview_browser(new_run_file);
	GUI_->wndw_run_file->show();
}
void tds_display::revert_run_file() {
	if (!previous_settings_were_saved()) {
		switch (fl_choice("The current settings are not saved in a '.run' file. Are you sure you wish to overwrite them by reverting to the original '.run' file?","No, cancel","Yes, overwrite",0)) {
		case 1:
			break;
		default:
			return;
		}
	}
	populate_from_run_file();
}
void tds_display::change_files() {
	generate_files_memento();
	GUI_->wndw_new_files->show();
}
void tds_display::finish_changing_files() {
	GUI_->wndw_new_files->hide();
}
void tds_display::run_loaded_simulation() {
	if (!struct_is_up_to_date()) {
	}
	LOG(gui_status,"Started simulating.");
	process_plugins();
	// really need to think about how some of these work for when the
	// tds_run class runs the analysis multiple times without being
	// hard reset every time
	initialise();
	direct_simulation_output_to_cout();
	make_analysis();
	LOG(gui_status,"Finished simulating.");
	empty_model();
	IPlugin::empty_plugins();
	LOG(gui_status,"Finished cleaning up after simulation.");
	
}
void tds_display::close_application() {
	GUI_->wndw_new_files->hide();
	GUI_->wndw_run_file->hide();
	GUI_->main_window->hide();
}

void tds_display::load_event(){
	// int e_number = int(GUI_->event->value()), c_number = int(GUI_->channel->value());
	// std::cout<<"event to load = "<<e_number<<std::endl;
}

void tds_display::load_section(int chnum){
	// int c_number = int(GUI_->channel->value());
	// std::cout<<"loading a section"<<std::endl;
	// load_section(c_number,chnum);
}

void tds_display::load_section(unsigned int ch_n, int chnum){
	// std::cout<<"load section"<<std::endl;
	// FRootfileComments.text(display_tl_info().c_str());
}

void tds_display::resize_plot(int c){
	std::cout<<"resize plot"<<std::endl;
}

void tds_display::plot(){
	std::cout<<"make a plot"<<std::endl;		
}

void tds_display::makeZoomBox(selection sel,int event,int section){
	std::cout<<"zoom in"<<std::endl;
}

void tds_display::populate_from_run_file() {
	BRunFileContents.loadfile(run_file().c_str());
	BRunFileContents.savefile(backup_run_file().c_str());
	prettify_run_file();
	read_run_file(run_file());
	model_directory(settings.model_directory); // using run_file_name instead until run file parsing written
	model_name(settings.model_name);
	config_directory(settings.config_directory);
	config_name(settings.config_name);
	output_directory(settings.output_directory);
	output_name(settings.output_name);
	mark_data_clean();
	mark_struct_up_to_date();
	LOG(gui_status, "Finished loading from '" << run_file() << "'");
}
void tds_display::prettify_run_file() {
	std::istringstream i_file, i_line;
	std::ostringstream o;
	std::string line, start, end;
	i_file.str(std::string(BRunFileContents.text()));
	o << std::left;
	while (std::getline(i_file,line)) {
		i_line.str(line);
		i_line >> start;
		std::getline(i_line,end);
		trim(end);
		o << std::setw(RUN_FILE_KEY_WIDTH) << start << end << std::endl;
		i_line.clear();
	}
	BRunFileContents.text(o.str().c_str());
}
void tds_display::generate_files_memento() {
	files_memento.model_directory = model_directory();
	files_memento.model_name = model_name();
	files_memento.config_directory = config_directory();
	files_memento.config_name = config_name();
	files_memento.output_directory = output_directory();
	files_memento.output_name = output_name();
	files_memento.data_clean = previous_settings_were_saved();
}	
void tds_display::restore_files_memento() {
	model_directory(files_memento.model_directory);
	model_name(files_memento.model_name);
	config_directory(files_memento.config_directory);
	config_name(files_memento.config_name);
	output_directory(files_memento.output_directory);
	output_name(files_memento.output_name);
	if (files_memento.data_clean) {
		mark_data_clean();
	} else {
		mark_data_dirty();
	}
}
void tds_display::update_gui_for_cleanliness() {
	std::vector<Fl_Widget*> only_dirty_buttons; // buttons which can only be used if dirty
	only_dirty_buttons.push_back(GUI_->btn_save_run_file);
	only_dirty_buttons.push_back(GUI_->btn_revert_run_file);
	for (int i=0; i < only_dirty_buttons.size(); ++i) {
		if (previous_settings_were_saved()) {
			only_dirty_buttons.at(i)->deactivate();
		} else {
			only_dirty_buttons.at(i)->activate();
		}
	}
	BDataDirty.text((previous_settings_were_saved()) ? "" : "*unsaved changes");
}
void tds_display::populate_preview_browser(std::string source) {
	std::istringstream iss;
	std::string line;
	iss.str(source);
	
	GUI_->brwsr_run_file_preview->clear();
	while (std::getline(iss,line)) {
		GUI_->brwsr_run_file_preview->add(line.c_str());
	}
}

void tds_display::action(selection sel, Fl_Widget *sender){
	std::cout<<"un zoom"<<std::endl;
}

void tds_display::action(Fl_Widget *sender){
	if (sender == GUI_->btn_open_run_file) {
		LOG(gui_actions, "Opening run file.");
		open_run_file_dialog();
	} else if (sender == GUI_->btn_save_run_file) {
		LOG(gui_actions, "Saving run file.");
		save_run_file();
	} else if (sender == GUI_->btn_preview_run_file) {
		LOG(gui_actions, "Generating run file preview.");
		preview_run_file();
	} else if (sender == GUI_->btn_revert_run_file) {
		LOG(gui_actions, "Reverting to run file.");
		revert_run_file();
	} else if (sender == GUI_->btn_new_files) {
		LOG(gui_actions, "Changing files/directories.");
		change_files();
	} else if (sender == GUI_->btn_revert_new_files) {
		LOG(gui_actions, "Reverting files/directories.");
		restore_files_memento();
	} else if (sender == GUI_->btn_save_new_files) {
		LOG(gui_actions, "Accepting new files/directories.");
		finish_changing_files();
	} else if (sender == GUI_->btn_run_simulation) {
		LOG(gui_actions, "Running the loaded simulation.");
		run_loaded_simulation();
	} else if (sender == GUI_->main_window) {
		LOG(gui_actions, "Closing application.");
		close_application();
	}
}









/******************** TDS_BATCH METHODS ********************/









tds_batch::tds_batch(std::string filename, std::string rootout):infile_(filename.c_str()),rootfile_name_(rootout.c_str()),tds_run(){
}

tds_batch::~tds_batch(){
	infile_.close();
}

int tds_batch::run_batch(std::string filename, bool recreate, int filechain, int n_tot){
	int tot_element_num=0; 
	std::cout<<"make root file"<<std::endl;
	infile_.close();
	return tot_element_num;
}
