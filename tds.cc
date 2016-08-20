#include "tds.hh"
#include "plugins.hh"

/******************** TDS METHODS ********************/









tds::tds():sections_() {
	element_dimensions_ = 0x0;
}

tds::~tds() {
	std::cout<<"cleaning sections"<<std::endl;
	clean_sections();
}

int tds::add_section(tds_section* new_section) {
	std::cout<<"adding a new section"<<std::endl;
	sections_.push_back(new_section);
	return sections_.size()-1;
}
int tds::add_material(tds_material* new_material) {
	std::cout << "adding a new material: " << (*new_material).name() << std::endl;
	material_map_[(*new_material).name()] = new_material;
	materials_.push_back(new_material);
	return materials_.size()-1;
}
int tds::add_node(tds_node* new_node) {
	// std::cout<<"adding a new node"<<std::endl;
	nodes_.push_back(new_node);
	return nodes_.size()-1;
}
int tds::add_element(tds_element* new_element) {
	// std::cout<<"adding a new element"<<std::endl;
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
	for (int i=0; i<elements_.size(); ++i) delete elements_[i];
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
		while (element(j).n_neighbours() == 0 && j < elements_.size())
			++j;
		if (j > i)
			elements_.erase(elements_.begin()+i,elements_.begin()+j); // start inclusive end exclusive
	}
}
void tds::expected_materials(int _n) { materials_.reserve(_n); }
void tds::expected_sections(int _n) { sections_.reserve(_n); }
void tds::expected_nodes(int _n) { nodes_.reserve(_n); }
void tds::expected_elements(int _n) { elements_.reserve(_n); }

void tds::output_model_summary(bool show_materials, bool show_sections, bool show_elements, bool show_element_links, bool show_nodes) {
	std::cout << std::endl << "We now have " << n_materials() << " materials, " << n_sections() << " sections, "
	          << n_elements() << " elements and " << n_nodes() << " nodes." << std::endl;
	if (show_materials) {
		for (int i = 0; i < n_materials(); i++) {
			std::cout << "Material " << (i+1) << ": " << material(i).name() << " - "
			          << material(i).density() << "kg/m^3 - " << material(i).diffusion_constant()
			          << "m^2/s" << std::endl;
		}
	}
	if (show_sections) {
		for (int i = 0; i < n_sections(); i++) {
			// std::cout << "Section " << (i+1) << " address:" << &section(i) << std::endl;
			// std::cout << "Section " << (i+1) << " material address:" << &(section(i).material()) << std::endl;
			std::cout << "Section " << (i+1) << ": " << section(i).material().name()
			          << " - " << section(i).n_elements() << " elements" << std::endl;
		}
	}
	if (show_elements) {
		for (int i = 0; i < n_elements(); i++) {
			std::cout << "Element " << (i+1) << " (" << &element(i) << "): " << element(i).material().name()
			          << " - " << element(i).n_nodes() << " nodes - "
			          << element(i).n_neighbours() << " neighbours - contamination at "
			          << element(i).contamination() << std::endl;
			std::cout << "\tOrigin at [" << element(i).origin(0) << ", " << element(i).origin(1)
			          << ", " << element(i).origin(2) << "]" << std::endl;
			for (int j = 0; j < element(i).n_nodes(); j++) {
				std::cout << "\tNode " << (j+1) << ": "
				          << &(element(i).node(j))
				          << std::endl;
			}
			if (show_element_links) {
				for (int j = 0; j < element(i).n_neighbours(); j++) {
					std::cout << "\tNeighbour " << (j+1) << ": "
					          << (element(i).neighbour(j).neighbour_of(&element(i)))
					          << " area " << (element(i).neighbour(j).interface_area())
					          << " multiplier " << (element(i).neighbour(j).a_n_dot_eMN_over_modMN())
					          << std::endl;
				}
			}
		}
	}
	if (show_nodes) {
		for (int i = 0; i < n_nodes(); i++) {
			std::cout << "Node " << (i+1) << " (" << &node(i) << "):" << std::endl;
			std::cout << "[" << node(i).position(0) << ","
			          << node(i).position(1) << "," << node(i).position(2) << "]" << std::endl;
			std::cout << node(i).n_elements() << " elements" << std::endl;
		}
	}
	std::cout << std::endl;
	
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
		std::cerr << "Detected 3-D stuff. This hasn't necessarily been programmed for yet." << std::endl;
		break;
	case 15: // 1 node point
		std::cerr << "Detected a single node point 'element' :/" << std::endl;
		break;
	default: // non-linear elements
		element_dimensions_.set(second_order_or_worse);
		std::cerr << "Detected non-linear elements :(" << std::endl;
	}
}









/******************** TDS_RUN METHODS ********************/









tds_run::tds_run():tds(){
}

tds_run::~tds_run(){
	//tdsfile_close();
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

	std::cout << "Running the model..." << std::endl;

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
				std::cout << "Section " << i << " has " << section(i).n_elements() << " elements and is getting fed the initial contamination." << std::endl;
				for (int j=0; j < section(i).n_elements(); ++j) {
					section(i).element(j).debug_contamination();
					section(i).element(j).contamination(source_contamination);
					section(i).element(j).flagAB(!section(i).element(j).flagAB());
					section(i).element(j).contamination(source_contamination);
					section(i).element(j).flagAB(!section(i).element(j).flagAB());
				}
				std::cout << "Contaminations set." << std::endl;
			}
		}
	}

	// The next step is to let plug-ins change the setup. This happens
	// here, before the timing code, because timing code would otherwise
	// be badly calculated.
	interrupt_pre_simulation();
	
	double time = 0.0;
	double next_time_recording = tracking_interval();
	
	trackingfile_.open(tracking_file_address());
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
				
			std::cout << "Step: "
			          << std::right << std::setw(12) << (step+1) << "/"
			          << std::left << std::setw(12) << steps() << std::left
			          << "        sim time: "
			          << std::setw(13) << format_time(time)
			          << "        elapsed: "
			          << std::setw(13) << format_time(elapsed_time)
			          << "        remaining (est.): "
			          << std::setw(13) << format_time(remaining_time)
			          << "        total (est.): "
			          << std::setw(13) << format_time(elapsed_time+remaining_time)
			          << std::endl;

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
	std::cout << "Total elapsed time: " << format_time(elapsed_time) << std::endl;
	
	trackingfile_.close();

	// Contaminations: final values at all elements
	contaminationsfile_.open(contaminations_file_address());
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
}

void tds_run::set_units_from_file(const char* units_file_address_) {

	if (!units_set_) {
		//conversion _conversion(units_file_address_);
		units(new conversion(units_file_address_));
		units_set_ = true;
	} else {
		std::cerr << "Units have already been set, but the program has asked to set them again. Request ignored." << std::endl;
	}
	
}

void tds_run::initialise() {

	// Before we can read in any data, we should look for
	// any settings about (measurement) units we can find
	set_units_from_file(units_file_address());
	// cout << "7.85 g/cm^3 in SI units is: " << units().convert_density_from("g/cm^3",7.85f) << std::endl;
	
	// Now read in data
	// First open all the necessary files (no point in getting half way through
	// processing only to find a file we need is missing)
	// std::cout << "gonna open me some files" << std::endl;
	materialsfile_.open(materials_file_address());
	sectionsfile_.open(sections_file_address());
	elementsfile_.open(elements_file_address());
	nodesfile_.open(nodes_file_address());

	int materials_count, sections_count, elements_count, nodes_count;
	
	//std::cout << "gonna read me some files" << std::endl;
	std::string line;
		
	//Let's start off by populating tds with some materials
	if (std::getline(materialsfile_, line)) {
		std::istringstream sizess(line);
		if (!(sizess >> materials_count)) {
			std::cerr << "Materials count not found, segmentation violations will occur after "
			          << "vector expands to accommodate many materials." << std::endl;
		} else {
			std::cout << "Expanding materials vector to accommodate " << materials_count
			          << " materials." << std::endl;
			expected_materials(materials_count+3); // not forgetting error, source and outgassing = +3
		}
		while (std::getline(materialsfile_, line)) {
			std::istringstream iss(line);
			std::string _name, _density_unit, _diffusion_constant_unit;
			double _density, _diffusion_constant;
		
			if (!(iss >> _name >> _density >> _density_unit >> _diffusion_constant >> _diffusion_constant_unit)) {
				std::cerr << "Invalid line, skipping. (material)\n";
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
	} else {
		std::cerr << "No materials found? Is the config name good?" << std::endl;
		throw;
	}
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
			std::cerr << "Sections count not found, segmentation violations will occur after "
			          << "vector expands to accommodate many sections." << std::endl;
		} else {
			std::cout << "Expanding sections vector to accommodate " << sections_count
			          << " sections." << std::endl;
			expected_sections(sections_count);
		}
		while (std::getline(sectionsfile_, line)) {
			std::istringstream iss(line);
			int _dim, _id;
			std::string _name;
		
			if (!(iss >> _dim >> _id >> _name)) {
				std::cerr << "Invalid line, skipping. (section)\n";
				continue;
			}
			{
				std::string _extra;
				while (iss >> _extra) {
					_name += " " + _extra;
				}
			}

			//std::cout << "We obtained three values [" << _dim << ", " << _id << ", " << _name << "].\n";
		
			// Get rid of the double quotes Gmsh puts in, and put to lower case
			_name.erase(_name.end()-1); _name.erase(_name.begin());
			std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);

			if (_id != n_sections()+1) {
				std::cerr << "Section ordering invalid - are you adding the same file a second time?" << std::endl;
				continue;
			}
			// std::cout << "Currently " << n_sections() << " sections. Adding another." << std::endl;
			tds_section* _s = new tds_section(_name, &material(_name));
			section_identifier _s_id;
			_s_id.section_id = add_section(_s);
			_s_id.section = _s;
			interrupt_section(_s_id);
			
			std::cout << "Now " << n_sections() << " sections." << std::endl;
		}
	} else {
		std::cerr << "No sections found? Is the model name good?" << std::endl;
		throw;
	}
	//Next we'll get some nodes in
	if (std::getline(nodesfile_, line)) {
		std::istringstream sizess(line);
		if (!(sizess >> nodes_count)) {
			std::cerr << "Nodes count not found, segmentation violations will occur after "
			          << "vector expands to accommodate many nodes." << std::endl;
		} else {
			std::cout << "Expanding nodes vector to accommodate " << nodes_count
			          << " nodes." << std::endl;
			expected_nodes(nodes_count);
		}
		while (std::getline(nodesfile_, line)) {
			std::istringstream iss(line);
			int id;
			double _x, _y, _z;
		
			if (!(iss >> id >> _x >> _y >> _z)) {
				std::cerr << "Invalid line, skipping. (node)\n";
				continue;
			}

			//std::cout << "We obtained four values [" << id << ", " << _x << ", " << _y << ", " << _z << "].\n";

			if (id != n_nodes()+1) {
				std::cerr << "Node ordering invalid - are you adding the same file a second time?" << std::endl;
				continue;
			}
			tds_node* _n = new tds_node(_x,_y,_z);
			node_identifier _n_id;
			_n_id.node_id = add_node(_n);
			_n_id.node = _n;
			interrupt_node(_n_id);
		}
	} else {
		std::cerr << "No nodes found? Is the model name good?" << std::endl;
		throw;
	}
	//Now finally we'll read the elements
	if (std::getline(elementsfile_, line)) {
		std::istringstream sizess(line);
		if (!(sizess >> elements_count)) {
			std::cerr << "Elements count not found, segmentation violations will occur after "
			          << "vector expands to accommodate many elements." << std::endl;
		} else {
			std::cout << "Expanding elements vector to accommodate " << elements_count
			          << " elements." << std::endl;
			expected_elements(elements_count);
		}
		while (std::getline(elementsfile_, line)) {
			std::istringstream iss(line);
			int id, type, n_tags, section_id, entity_id;
			std::ostringstream extra_tags;
		
			if (!(iss >> id >> type >> n_tags)) {
				std::cerr << "Invalid line, skipping. (element)\n";
				continue;
			}
			register_element_type(type);
			if (id != n_elements()+1) {
				std::cerr << "Element ordering invalid - are you adding the same file a second time?" << std::endl;
				continue;
			}
			int tags_processed = 0;
			iss >> section_id >> entity_id;
			for (int tags_processed = 2; tags_processed < n_tags; tags_processed++) {
				extra_tags << iss << " ";
			}
			if (extra_tags.str().length() > 0)
				std::cerr << "Unprocessed tags: " << extra_tags.str() << std::endl;
			tds_nodes _element_nodes;
			int this_node;
			while (iss >> this_node) {
				//std::cout << "Found that element " << id << " includes node " << this_node << std::endl;
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
	} else {
		std::cerr << "No elements found? Is the model name good?" << std::endl;
		throw;
	}
	// std::cout << "gonna close me some files" << std::endl;
	sectionsfile_.close();
	elementsfile_.close();
	nodesfile_.close();

	// Every element now has a list of nodes that it make it,
	// and every node has a list of every element that makes
	// use of it. In 1-D we may now simply make our way through
	// the list of elements, adding links and removing from
	// each node's list of elements as we go
	
	std::cout << "Producing element links" << std::endl;
	int element_link_count = 0;
	std::cout << "Discovered dimensions bitset is: " << element_dimensions() << std::endl;
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
						// std::cout << "Source element " << &section(s).element(i) << " links to node "
						//           << &(section(s).element(i).node(j)) << " which currently links to "
						//           << o << " more elements." << std::endl;
						for (int k=o; k > 0; k--) {
							if (section(s).element(i).node(j).element(k-1).material().is_source()) {
								//std::cout << "No need to give a link from source to source, skipping." << std::endl;
							} else if (&section(s).element(i) == &(section(s).element(i).node(j).element(k-1))) {
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
		std::cout << t << " elements trawled for links." << std::endl;
		std::cout << "Made " << element_link_count << " element link objects." << std::endl;
	} else if (!(element_dimensions(three_d) || element_dimensions(second_order_or_worse)) &&
	           (element_dimensions(two_d))
	           ) {

		// 2D is nice because whatever shape the element is,
		// it can only build an interface from two nodes.
		
		int n, m, o, p, t = 0, c;
		for (int s = 0; s < n_sections(); s++ ) {
			std::cout << "Starting section " << s << " (" << section(s).name() << ")" << std::endl;
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
			std::cout << "Starting section " << s << " (" << section(s).name() << ")" << std::endl;
			n = section(s).n_elements();
			t += n;
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
							// c is a count of the common nodes
							c = 1; // we know that one node is the same already
							for (int l=0; (l < p); ++l) {
								// ignore that one node
								if (&section(s).element(i).node(j) != &section(s).element(i).node(j).element(k).node(l)) {
									for (int q=0; (q < m && c < 2); ++q) {
										if (&section(s).element(i).node(j).element(k).node(l) == &section(s).element(i).node(q)) {
											++c;
										}
									}
								}
							}
							if (c >= 3) {
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
		
	} else {
		std::cout << "Element links could not be made, only linear in 1-D, 2-D and 3-D programmed." << std::endl;
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
			std::cerr << "No known plugin '" << plugin_text << "'." << std::endl;
			continue;
		}
		switch (plugin_strings[plugin_text]) {
		case PExample:
			IPlugin::store_plugin(new Example()); break;
		case POutgassing:
			IPlugin::store_plugin(new Outgassing()); break;
		default:
			std::cerr << "Plugin not yet implemented: " << plugin_text << std::endl;
			throw;
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
        
	std::cout << "Using instruction file: '" << run_file_name << "'" << std::endl;

	// Set defaults before reading from the file
	settings.model_name = "simple2d";
	settings.config_name = "simple";
	settings.output_name = "output";
	settings.delta_t = 3600.0*24.0;
	settings.tracking_interval = 3600*24*365.24;
	settings.simulation_length = 3600*24*30.0;
	settings.tracking_list = new std::vector<int>();
	
	
	std::cout << "Run file reading not yet fully implemented!" << std::endl;

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
                        std::cerr << "Error at first line of '" << run_file_name
                                  << "'. Expecting run mode." << std::endl;
                        std::cerr << "Instead got: " << line << std::endl;
                        throw;
		} else {
			std::cout << "Found request for setting '" << setting << "' with:" << std::endl;
                        std::cout << "\t" << value << std::endl;
		}
	} else {
                std::cerr << "Error at first line of '" << run_file_name
                          << "'. Expecting run mode." << std::endl;
                throw;
        }
	line_processing.clear();
	if (std::getline(run_file_, line)) {
		line_processing.str(line);
		if (!(line_processing >> setting >> value)) {
                        std::cerr << "Error at second line of '" << run_file_name
                                  << "'. Expecting .run file format version." << std::endl;
                        std::cerr << "Instead got: " << line_processing.str() << std::endl;
                        throw;
		} else {
			std::cout << "Found request for setting '" << setting << "' with:" << std::endl;
                        std::cout << "\t" << value << std::endl;
		}
	} else {
                std::cerr << "Error at second line of '" << run_file_name
                          << "'. Expecting  .run file format version." << std::endl;
                throw;
        }
	line_processing.clear();
	int line_n = 2;
	bool config_given = false;
	while (std::getline(run_file_, line)) {
		line_processing.str(line);
		++line_n;
		if (!(line_processing >> setting && std::getline(line_processing,value))) {
                        std::cerr << "Error at line " << line_n << " of '" << run_file_name
                                  << "'." << std::endl;
                        std::cerr << "Line: " << line_processing.str() << std::endl;
                        throw;
		} else {
			trim(value);
			std::cout << "Found request for setting '" << setting << "' with:" << std::endl;
                        std::cout << "\t" << value << std::endl;
		}
		std::istringstream interpreter;
		// Following is the list of available instructions in a .run file
                if (setting == "models-directory") {
	                ensure_ending(value, "/");
	                std::cout << "\tSetting the models directory: " << value << std::endl;
	                settings.model_directory = value;
                } else if (setting == "config-directory") {
	                ensure_ending(value, "/");
	                std::cout << "\tSetting the config directory: " << value << std::endl;
	                settings.config_directory = value;
                } else if (setting == "output-directory") {
	                ensure_ending(value, "/");
	                std::cout << "\tSetting the output directory: " << value << std::endl;
	                settings.output_directory = value;
                } else if (setting == "gmsh-bin-directory") {
	                ensure_ending(value, "/");
	                std::cout << "\tSetting the gmsh directory: " << value << std::endl;
	                settings.gmsh_bin_directory = value;
                } else if (setting == "model-name") {
	                std::cout << "\tSetting the model name: " << value << std::endl;
	                settings.model_name = value;
                } else if (setting == "config-name") {
	                std::cout << "\tSetting the config name: " << value << std::endl;
	                settings.config_name = value;
	                config_given = true;
	                std::cout << "\tSetting the units from " << units_file_address() << std::endl;
	                set_units_from_file(units_file_address());
                } else if (setting == "output-name") {
	                std::cout << "\tSetting the output name: " << value << std::endl;
	                settings.output_name = value;
                } else if (setting == "delta-t") {
	                std::cout << "\tSetting the time step: " << value << std::endl;
	                if (config_given) {
		                std::string unit;
		                interpreter.str(value);
		                if (!(interpreter >> settings.delta_t >> unit)) {
			                std::cerr << "\t\tThat value didn't work for that setting." << std::endl;
		                } else {
			                settings.delta_t = units().convert_time_from(unit,settings.delta_t);
		                }
	                } else
		                std::cerr << "Trying to set a numeric value before the config has been set - wouldn't know what to do with the units yet!" << std::endl;
                } else if (setting == "tracking-interval") {
	                std::cout << "\tSetting the time interval for recording contaminations: " << value << std::endl;
	                if (config_given) {
		                std::string unit;
		                interpreter.str(value);
		                if (!(interpreter >> settings.tracking_interval >> unit)) {
			                std::cerr << "\t\tThat value didn't work for that setting." << std::endl;
		                } else {
			                settings.tracking_interval = units().convert_time_from(unit,settings.tracking_interval);
		                }
	                } else
		                std::cerr << "Trying to set a numeric value before the config has been set - wouldn't know what to do with the units yet!" << std::endl;
                } else if (setting == "simulation-length") {
	                std::cout << "\tSetting the minimum total simulation time: " << value << std::endl;
	                if (config_given) {
		                std::string unit;
		                interpreter.str(value);
		                if (!(interpreter >> settings.simulation_length >> unit)) {
			                std::cerr << "\t\tThat value didn't work for that setting." << std::endl;
		                } else
			                settings.simulation_length = units().convert_time_from(unit,settings.simulation_length);
	                } else
		                std::cerr << "Trying to set a numeric value before the config has been set - wouldn't know what to do with the units yet!" << std::endl;
                } else if (setting == "contamination-mode-time") {
	                std::cout << "\tSetting the contamination mode for time: " << value << std::endl;
	                settings.contamination_mode_time = value;
                } else if (setting == "contamination-mode-space") {
	                std::cout << "\tSetting the contamination mode for space: " << value << std::endl;
	                settings.contamination_mode_space = value;
                } else if (setting == "contamination") {
	                std::cout << "\tSetting the basic contamination: " << value << std::endl;
	                if (config_given) {
		                std::string unit;
		                interpreter.str(value);
		                if (!(interpreter >> settings.contamination >> unit)) {
			                std::cerr << "\t\tThat value didn't work for that setting." << std::endl;
		                } else
			                settings.contamination = units().convert_contamination_from(unit,settings.contamination);
	                } else
		                std::cerr << "Trying to set a numeric value before the config has been set - wouldn't know what to do with the units yet!" << std::endl;
                } else if (setting == "contaminations-file") {
	                std::cout << "\tSetting the contaminations file name: " << value << std::endl;
	                settings.contaminations_file = value;
                } else if (setting == "activate-plugin") {
	                std::cout << "\tAdding a plug-in to activate: " << value << std::endl;
	                settings.activated_plugins.push_back(value);
                } else if (setting == "plugin-file") {
	                std::cout << "\tSetting the file for a plugin: " << value << std::endl;
	                std::string plugin;
	                std::string init_only;
	                std::string file;
	                interpreter.str(value);
	                if (!(interpreter >> plugin >> init_only >> file)) {
		                std::cerr << "\t\tThat value didn't work for that setting." << std::endl;
	                } else {
		                settings.plugin_files[plugin].file_name = file;
		                settings.plugin_files[plugin].needed_after_initialisation = (init_only != "init-only");
	                }
                } else if (setting == "tracking-mode") {
	                std::cout << "\tSetting the tracking mode: " << value << std::endl;
	                settings.tracking_mode = value;
                } else if (setting == "track") {
	                std::cout << "\tSetting the config name: " << value << std::endl;
	                interpreter.str(value);
	                if (settings.tracking_mode == "ids") {
		                delete settings.tracking_list;
		                settings.tracking_list = new std::vector<int>();
		                int id;
		                while (interpreter >> id)
			                settings.tracking_list->push_back(id);
	                } else {
		                if (!(interpreter >> settings.tracking_n))
			                std::cerr << "\t\tThat value didn't work for that setting." << std::endl;
	                }
                } else {
                        std::cerr << "\t(Didn't know what to do with '" << setting << "'.)" << std::endl;
		}
                line_processing.clear();
	}

	// Process the plugins, which have been stored as a list of string plugin names and file names
	process_plugins();
	
	// Set the number of simulation steps from the step size and simulation length
	steps(ceil(settings.simulation_length/settings.delta_t));
	// Now get the simulation to initialise using the data we have brought in
	// from the instructions file

	now = GetTimeMs64();
	progress = (now - checkmark)/1000;
	std::cout << "Instructions read in " << progress << " seconds. Initialising..." << std::endl;
	checkmark = now;
	this->initialise();
	now = GetTimeMs64();
	progress = (now - checkmark)/1000;
	std::cout << "Initialisation took " << progress << " seconds." << std::endl;
	

	
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
	sections_[_i] = _new_section;
}
void tds_run::change_material_pointer(int _i, tds_material* _new_material) {
	materials_[_i] = _new_material;
}
void tds_run::change_node_pointer(int _i, tds_node* _new_node) {
	nodes_[_i] = _new_node;
}
void tds_run::change_element_pointer(int _i, tds_element* _new_element) {
	elements_[_i] = _new_element;
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
	GUI_->RootfileComment->buffer(FRootfileComments);
	GUI_->TimelineComment->buffer(TimelineComment);
	GUI_->RootfileName->buffer(FRootfileName);
	FRootfileComments.text("\n \n \t Choose a file");
}

tds_display::~tds_display(){
	GUI_->plotH->clear();
}

void tds_display::dialog_open(){
	const char *filePtr=fl_file_chooser("Input File",NULL,"",0);
	if(filePtr){
		std::cout<<"open"<<std::endl;
		filename(filePtr);
		load_section(0);
		FRootfileName.text(filePtr);
	}
}

void tds_display::load_event(){
	int e_number = int(GUI_->event->value()), c_number = int(GUI_->channel->value());
	std::cout<<"event to load = "<<e_number<<std::endl;
}

void tds_display::load_section(int chnum){
	int c_number = int(GUI_->channel->value());
	std::cout<<"loading a section"<<std::endl;
	load_section(c_number,chnum);
}

void tds_display::load_section(unsigned int ch_n, int chnum){
	std::cout<<"load section"<<std::endl;
	FRootfileComments.text(display_tl_info().c_str());
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

void tds_display::action(selection sel, Fl_Widget *sender){
	std::cout<<"un zoom"<<std::endl;
}

void tds_display::action(Fl_Widget *sender){
	std::cout<<"display action"<<std::endl;
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
