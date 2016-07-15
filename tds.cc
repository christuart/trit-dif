#include "tds.hh"

/******************** TDS_ELEMENT METHODS ********************/
tds_element::tds_element(tds_nodes _nodes, tds_material* _material, float _contamination):nodes_(_nodes),material_(_material),contaminationA_(_contamination),contaminationB_(_contamination) {
	// no specific centre point has been provided, so use COM for the element type
	// e.g. triangular element r_COM = r_A + (2/3) * (r_AB + 0.5 * r_BC)
	origin_.reserve(3);
	set_origin_from_nodes();
	calculate_size();
	flagAB(false);
}

tds_element::tds_element(tds_nodes _nodes, tds_material* _material, std::vector<float> _origin, float _contamination):nodes_(_nodes),material_(_material),origin_(_origin),contaminationA_(_contamination) {
	calculate_size();
	flagAB(false);
}
tds_element::tds_element(tds_nodes _nodes, tds_material* _material, float _origin_x, float _origin_y, float _origin_z, float _contamination):nodes_(_nodes),contaminationA_(_contamination) {
	origin_.reserve(3);
	origin(_origin_x, _origin_y, _origin_z);
	calculate_size();
	flagAB(false);
}

tds_element::~tds_element(){
}

void tds_element::add_element_link(tds_element_link* new_element_link) {
	neighbours_.push_back(new_element_link);
}

void tds_element::transfer_contaminant(float _quantity) {
	// std::cout << "Received quantity of " << _quantity << " and have size() of "
	//           << size() << std::endl;
	contamination(contamination()+_quantity/size());
	flagAB(!flagAB());
}
void tds_element::set_origin_from_nodes() {
	std::cout << "Setting origin from nodes." << std::endl;
	origin_.reserve(3);
	int n_nodes = nodes_.size();
	float x,y,z;
	std::cout << "There are " << n_nodes << " nodes." << std::endl;
	
	switch (n_nodes) {
	case 2:
	case 3:
	case 4:
		// can use the mean of the vectors for basic elements
		x = 0;
		y = 0;
		z = 0;
		for (int i = 0; i < n_nodes; i++) {
			x += node(i).position(0);
			y += node(i).position(1);
			z += node(i).position(2);
		}
		std::cout << "Sum of vectors is [" << x << "," << y << "," << z << "]." << std::endl;
		x /= n_nodes;
		y /= n_nodes;
		z /= n_nodes;
		break;
	default:
		std::cout << "!!! Looking for origin of " << nodes_.size() << " noded element, not programmed yet " << std::endl;
	}
	std::cout << "Average of vectors is [" << x << "," << y << "," << z << "]." << std::endl;
	
	origin(x,y,z); std::cout << x << std::endl;
	std::cout << "asdasdasdasd " << origin().size() << std::endl;
	debug(&origin());
}

void tds_element::update(float delta_t) {//method to update parameters
	float total_flow = 0.0f;
	for (int i = 0; i < n_neighbours(); i++) {
		total_flow += neighbour(i).flow_rate(this->flagAB()) * neighbour(i).positive_flow(this);
	}
	// std::cout << "Flow rate into " << this << " is " << total_flow << std::endl;
	// std::cout << "Takes contamination from " << contamination();
	transfer_contaminant(total_flow * delta_t);
	// std::cout << " to " << contamination() << std::endl;
}
void tds_element::propogate_into_nodes() {
	std::cout << "Propogating at element " << this << " i.e. through " << n_nodes() << " nodes." << std::endl;
	for (int i=0; i < n_nodes(); i++) {
		node(i).add_element(this);
	}
}
void tds_element::calculate_size() {
	std::vector<float> vecPQ, vecPR;
	switch (n_nodes()) {
	case 2:
		vecPQ.resize(3);
		vecPQ = node(1).position() - node(0).position();
		if (vecPQ.at(1) == 0.0 && vecPQ.at(2) == 0.0) {
			size(abs(vecPQ.at(1)-vecPQ.at(0)));
		} else {
			vecPQ *= vecPQ;
			size(sqrt(vecPQ.at(0)+vecPQ.at(1)+vecPQ.at(2)));
		}
		break;
	case 3:
		// triangle PQR, area is 1/2 mod(PQ x PR)
		vecPQ.resize(3);
		vecPQ = node(1).position() - node(0).position();
		vecPR.resize(3);
		vecPR = node(2).position() - node(0).position();
		if (vecPQ.at(2) == 0.0 && vecPR.at(2) == 0.0) {
			//2D cross product nice and simple
			size(0.5 * abs(vecPQ.at(1)*vecPR.at(0) - vecPQ.at(0)*vecPR.at(1)));
		} else {
			//3D cross product instead boooooo
			size(
			     0.5 *
			     sqrt(
			          (vecPQ.at(1)*vecPR.at(2)-vecPQ.at(2)*vecPR.at(1))
			          *(vecPQ.at(1)*vecPR.at(2)-vecPQ.at(2)*vecPR.at(1)) + 
			          (vecPQ.at(2)*vecPR.at(0)-vecPQ.at(0)*vecPR.at(2))
			          *(vecPQ.at(2)*vecPR.at(0)-vecPQ.at(0)*vecPR.at(2)) + 
			          (vecPQ.at(0)*vecPR.at(1)-vecPQ.at(1)*vecPR.at(0))
			          *(vecPQ.at(0)*vecPR.at(1)-vecPQ.at(1)*vecPR.at(0))
			          )
			     );
		}
		break;
	case 4:
		std::cerr << "!!! HAVEN'T PROGRAMMED AREA/VOLUME OF QUADRANGLES OR TETRAHEDRA YET!" << std::endl;
		break;
	default:
		std::cerr << "!!! HAVEN'T PROGRAMMED AREA/VOLUME OF THIS ELEMENT SHAPE YET!" << std::endl;
	}
}
void tds_element::debug_contamination() {
	// std::cout << "Address: " << this << "; flagAB: " << flagAB() << "; Cont A: " << contaminationA_ << "; Cont B: " << contaminationB_ << std::endl;
}

/******************** TDS_MATERIAL METHODS ********************/
tds_material::tds_material(std::string _name, float _density, float _diffusion_constant):material_name_(_name),material_density_(_density),material_diffusion_constant_(_diffusion_constant) {
}

tds_material::~tds_material() {
}

/******************** TDS_SECTION METHODS ********************/
tds_section::tds_section(tds_material* _material):elements_(),material_(_material) {	
}

tds_section::~tds_section() {
	clean_elements();
}

void tds_section::add_element(tds_element* new_element) {
	std::cout<<"adding new element"<<std::endl;
	elements_.push_back(new_element);
}

void tds_section::clean_elements() {
	for (int i=0; i<elements_.size(); ++i) delete elements_[i];
	elements_.resize(0);
}

/******************** TDS_ELEMENT_LINK METHODS ********************/
tds_element_link::tds_element_link(tds_element* _M, tds_element* _N) {
	elementM_ = _M;
	elementN_ = _N;
	this->initialise();
}
tds_element_link::~tds_element_link() {
}
void tds_element_link::initialise() {
	norm_vector_.reserve(3);
	flux_vector_.reserve(3);

	// get flux vector, which is the vector from COM of one element to the COM of the next
	flux_vector(elementN_->origin(0)-elementM_->origin(0),
	            elementN_->origin(1)-elementM_->origin(1),
	            elementN_->origin(2)-elementM_->origin(2));
	std::vector<float> asd = elementM_->origin();

	// find the common nodes between the two elements
	shared_nodes_.clear();
	int n_M_nodes = elementM().n_nodes();
	int n_N_nodes = elementN().n_nodes();
	for (int i = 0; i < n_M_nodes; i++) {
		for (int j = 0; j < n_N_nodes; j++) {
			if (&(elementM().node(i)) == &(elementN().node(j))) { shared_nodes_.push_back(&(elementM().node(i))); }
		}
	}

	// find the normal vector and area at the interface between them
	// this is different depending on the dimensions of the problem
	// different dimensioned problems have different numbers of shared nodes
	switch (shared_nodes_.size()) {
	case 1:
		// norm_vector and flux_vector are always the same direction in 1-D
		norm_vector(flux_vector());
		modMN(magnitude(flux_vector()));
		norm_vector_ *= (1.0f/modMN());
		if (norm_vector(1) != 0.0f || norm_vector(2) != 0.0f) {
			std::cout << "!!! non 1-D elements had a 1 node interface -- not physically accurate" << std::endl;
		}
		interface_area(1.0f);
		break;
	case 2:
		// first get the vector along the edge, but rotated 90deg, i.e. [y; -x]
		norm_vector(0,shared_node(1).position(1) - shared_node(0).position(1));
		norm_vector(1,shared_node(0).position(0) - shared_node(1).position(0));
		if (shared_node(0).position(2) != 0.0f || shared_node(1).position(2) != 0.0f) {
			std::cout << "!!! non 2-D elements had a 2 node interface -- not physically accurate" << std::endl;
		}
		// make use of this rotated vector as a measure of interface length, then normalise it
		modMN(magnitude(flux_vector()));
		interface_area(magnitude(norm_vector()));
		norm_vector_ *= (1/interface_area());
		// now make sure it is in the outward direction to follow standard conventions
		if (dot(norm_vector(),flux_vector()) < 0.0f) norm_vector_ *= -1;
		break;
	case 3:
		std::cout << "!!! haven't implemented 3d element link initialisation" << std::endl;
		break;
	}
	
	// calculate the geometry multiplier to turn D * (diff in contamination) into flow rate
	a_n_dot_eMN_over_modMN(interface_area() * dot(norm_vector(),flux_vector())/(modMN()*modMN()) );
}
float tds_element_link::flow_rate(bool _AB) {
	// AB flag system used because the same flow needn't be calculated twice for M->N and N->M
	// instead we calculate it and store it, and only recalculate when the flag switches
	if (_AB != flagAB()) {
		// std::cout << "From " << elementN_ << " (cont " << elementN().contamination(_AB)
		//           << ") to " << elementM_ << " (cont " << elementM().contamination(_AB)
		//           << ") with andotemnovermodmn = " << a_n_dot_eMN_over_modMN() << " amd D = " << diffusion_constant() << std::endl;
		flow_rate_ = ( a_n_dot_eMN_over_modMN() *
		               (elementN().contamination(_AB) - elementM().contamination(_AB)) *
		               diffusion_constant() );
		flagAB(_AB);
	}
	return flow_rate_;
}
float tds_element_link::diffusion_constant() {
	float D1 = elementM().material().diffusion_constant();
	float D2 = elementN().material().diffusion_constant();
	if (D1 > D2) return D1;
	return D2;
}
void tds_element_link::set_flag_against(tds_element* _element) {
	flagAB(!_element->flagAB());
}
/******************** TDS METHODS ********************/
tds::tds():sections_(){
	element_dimensions_ = 0x0;
}

tds::~tds(){
	std::cout<<"cleaning sections"<<std::endl;
	clean_sections();
}

void tds::add_section(tds_section* new_section){
	std::cout<<"adding a new section"<<std::endl;
	sections_.push_back(new_section);
}
void tds::add_material(tds_material* new_material){
	std::cout << "adding a new material: " << (*new_material).name() << std::endl;
	material_map_[(*new_material).name()] = new_material;
	materials_.push_back(new_material);
}
void tds::add_node(tds_node* new_node){
	std::cout<<"adding a new node"<<std::endl;
	nodes_.push_back(new_node);
}
void tds::add_element(tds_element* new_element){
	std::cout<<"adding a new element"<<std::endl;
	elements_.push_back(new_element);
}

void tds::clean_sections(){
	for (int i=0; i<sections_.size(); ++i) delete sections_[i];
	sections_.resize(0);
}
void tds::clean_materials(){
	for (int i=0; i<materials_.size(); ++i) delete materials_[i];
	materials_.resize(0);
}
void tds::clean_nodes(){
	for (int i=0; i<nodes_.size(); ++i) delete nodes_[i];
	nodes_.resize(0);
}
void tds::clean_elements(){
	for (int i=0; i<elements_.size(); ++i) delete elements_[i];
	elements_.resize(0);
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
		std::cout << "Detected some 2-D stuff!" << std::endl;
		break;
	case 4: // 4 node tetrahedron
	case 5: // 8 node hexahedron
	case 6: // 6 node prism
	case 7: // 5 node pyramid
		element_dimensions_.set(three_d);
		std::cout << "Detected some 3-D stuff!" << std::endl;
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

void tds_run::make_analysis(float delta_t, int _steps, float recording_interval, std::vector<int>& tracked_elements) {

	std::cout << "Running the model..." << std::endl;

	// This is a very basic implementation of 'make_analysis' which has constant
	// even source and no outgassing

	// Allow the user to specify '-1' as the final element
	for (int i=0; i < tracked_elements.size(); ++i) {
		if (tracked_elements[i] < 0) tracked_elements[i] += n_elements();
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
	float source_contamination = 1.0e13;
	for (int i=0; i < n_sections(); ++i) {
		if (section(i).material().is_source()) {
			std::cout << "Section " << i << " has " << section(i).n_elements() << " elements and is " << section(i).material().name() << " which is... source?????: " << section(i).material().is_source() << std::endl;
			for (int j=0; j < section(i).n_elements(); ++j) {
				section(i).element(j).debug_contamination();
				section(i).element(j).contamination(source_contamination);
				section(i).element(j).flagAB(!section(i).element(j).flagAB());
				section(i).element(j).contamination(source_contamination);
				section(i).element(j).flagAB(!section(i).element(j).flagAB());
			}
		}
	}
	for (int i=0; i < n_elements(); ++i)
		element(i).debug_contamination();

	float time = 0.0;
	float next_time_recording = recording_interval;
	
	trackingfile_.open(outputname_ + ".tracking");
	trackingfile_ << "element, time, contamination" << std::endl;
	trackingfile_ << std::scientific;
	for (int i=0; i < tracked_elements.size(); ++i)
		trackingfile_ << (tracked_elements[i]) << ", " << time << ", " << element(tracked_elements[i]-1).contamination() << std::endl;
	
	int reporting_interval = ceil(_steps/100);
	for (int step = 0; step < _steps; ++step) {
		
		// Every step we need to:
		// 1) Update every element (each element decides what it needs to do to consitute an update)
		// 2) Switch all of the flags in the elements (BUT NOT IN THE LINKS!)
		// 3a) Output any outputs that have been requested and any warnings to std::cout
		// 3b) Output any outputs that have been requested to the output file
		// It may be necessary for speed's sake to put these outputs into arrays or buffers
		// and output only at the end of simulation.

		// 1) Update elements
		for (int i=0; i < n_sections(); ++i) {
			// Note simple model with constant source so no need to update them
			if (!section(i).material().is_source()) {
				for (int j=0; j < section(i).n_elements(); ++j) {
					section(i).element(j).update(delta_t);
				}
			}
		}
		time += delta_t;
		
		// 2) Switch flags in elements
		//this_flag != this_flag;
		//for (int i=0; i < n_elements(); ++i) {
		//	element(i).flagAB(this_flag);
		//}
		//
		// 3)
		if (next_time_recording < time) {
			for (int i=0; i < tracked_elements.size(); ++i) {
				trackingfile_ << (tracked_elements[i]) << ", " << time
				              << ", " << element(tracked_elements[i]-1).contamination()
				              //<< ", " << element(tracked_elements[i]-1).contamination(false)
				              //<< ", " << element(tracked_elements[i]-1).contamination(true)
				              << std::endl;
			}
			next_time_recording += recording_interval;
		}
		for (int i=0; i < n_elements(); ++i)
			element(i).debug_contamination();
		if (step % reporting_interval == 0)
			std::cout << "Step " << (step+1) << " complete. Time now is " << time << "s." << std::endl;
	}
	trackingfile_.close();

	// Contaminations: final values at all elements
	contaminationsfile_.open(outputname_ + ".contaminations");
	contaminationsfile_ << "model: " << basename_ << "; config: " << configname_ << "; delta_t: "
	                    << delta_t << "; time steps: " << _steps << "; final time: " << time
	                    << "s" << std::endl;
	contaminationsfile_ << "element, x, y, z, contamination" << std::endl;
	contaminationsfile_ << std::scientific;
	for (int i=0; i < n_elements(); ++i)
		contaminationsfile_ << (i+1) << ", " << element(i).origin(0) << ", " << element(i).origin(1)
		                    << ", " << element(i).origin(2) << ", " << element(i).contamination()
		                    << std::endl;
	contaminationsfile_.close();
}

void tds_run::initialise() {

	// Before we can read in any data, we should look for
	// any settings about (measurement) units we can find
	conversion _conversion((configname() + ".units").c_str());
	units(&_conversion);
	// cout << "7.85 g/cm^3 in SI units is: " << units().convert_density_from("g/cm^3",7.85f) << std::endl;
	
	// Now read in data
	// First open all the necessary files (no point in getting half way through
	// processing only to find a file we need is missing)
	// std::cout << "gonna open me some files" << std::endl;
	materialsfile_.open((configname() + ".materials").c_str());
	sectionsfile_.open((basename() + ".sections").c_str());
	elementsfile_.open((basename() + ".elements").c_str());
	nodesfile_.open((basename() + ".nodes").c_str());

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
	}
	while (std::getline(materialsfile_, line)) {
		std::istringstream iss(line);
		std::string _name, _density_unit, _diffusion_constant_unit;
		double _density, _diffusion_constant;
		
		if (!(iss >> _name >> _density >> _density_unit >> _diffusion_constant >> _diffusion_constant_unit)) {
			std::cerr << "Invalid line, skipping. (material)\n";
			continue;
		}

		std::cout << "We obtained five values [" << _name << ", " << _density << ", " << _density_unit << ", " << _diffusion_constant << ", " << _diffusion_constant_unit << "].\n";
		
		// let's make all names lower case
		std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
		
		tds_material* _m = new tds_material(_name, units().convert_density_from(_density_unit,_density), units().convert_diffusion_constant_from(_diffusion_constant_unit,_diffusion_constant));
		add_material(_m);
	}
	// we'll add an error material for when a bad name is given
	// and source/outgassings materials for the regions which will
	// behave in a special manner!
	add_material(new tds_material("error", 1.0, 0.0));
	add_material(new tds_material("source", 1.0, 0.0));
	add_material(new tds_material("outgassing", 1.0, 0.0));
	
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
	}
	while (std::getline(sectionsfile_, line)) {
		std::istringstream iss(line);
		int _dim, _id;
		std::string _name;
		
		if (!(iss >> _dim >> _id >> _name)) {
			std::cerr << "Invalid line, skipping. (section)\n";
			continue;
		}

		std::cout << "We obtained three values [" << _dim << ", " << _id << ", " << _name << "].\n";
		
		// Get rid of the double quotes Gmsh puts in, and put to lower case
		_name.erase(_name.end()-1); _name.erase(_name.begin());
		std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);

		if (_id != n_sections()+1) {
			std::cerr << "Section ordering invalid - are you adding the same file a second time?" << std::endl;
			continue;
		}
		std::cout << "Currently " << n_sections() << " sections. Adding another." << std::endl;
		add_section(new tds_section(&material(_name)));
		std::cout << "Now " << n_sections() << " sections." << std::endl;
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
	}
	while (std::getline(nodesfile_, line)) {
		std::istringstream iss(line);
		int id;
		float _x, _y, _z;
		
		if (!(iss >> id >> _x >> _y >> _z)) {
			std::cerr << "Invalid line, skipping. (node)\n";
			continue;
		}

		std::cout << "We obtained four values [" << id << ", " << _x << ", " << _y << ", " << _z << "].\n";

		if (id != n_nodes()+1) {
			std::cerr << "Node ordering invalid - are you adding the same file a second time?" << std::endl;
			continue;
		}
		add_node(new tds_node(_x,_y,_z));
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
			std::cout << "Unprocessed tags: " << extra_tags.str() << std::endl;
		tds_nodes _element_nodes;
		int this_node;
		while (iss >> this_node) {
			std::cout << "Found that element " << id << " includes node " << this_node << std::endl;
			_element_nodes.push_back(&(node(this_node-1))); // don't forget that msh is 1-indexed and arrays are 0-indexed
		}
		std::cout << "all nodes ready to place into the element" << std::endl;
		tds_element* new_element = new tds_element(_element_nodes,&(section(section_id-1).material()),0.0);
		std::cout << "element created, with nodes" << std::endl;
		add_element(new_element);
		std::cout << "element stored, referencing it in its nodes" << std::endl;
		(*new_element).propogate_into_nodes();
		std::cout << "element also stored in section" << std::endl;
		section(section_id-1).add_element(new_element);
		std::cout << "all done!" << std::endl;

		std::cout << "We obtained some values [" << id << ", " << type << ", " << n_tags << ", " << section_id << ", " << entity_id << "].\n";
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
						std::cout << "Source element " << &section(s).element(i) << " links to node "
						          << &(section(s).element(i).node(j)) << " which currently links to "
						          << o << " more elements." << std::endl;
						for (int k=o; k > 0; k--) {
							if (section(s).element(i).node(j).element(k-1).material().is_source()) {
								std::cout << "No need to give a link from source to source, skipping." << std::endl;
							} else if (&section(s).element(i) == &(section(s).element(i).node(j).element(k-1))) {
								std::cout << "Skipping - don't need to link to self!" << std::endl;
							} else {
								std::cout << "Making link from " << &section(s).element(i) << " to "
								          << &(section(s).element(i).node(j).element(k-1)) << std::endl;
								tds_element_link* new_link = new tds_element_link(&section(s).element(i),
								                                                  &(section(s).element(i).node(j).element(k-1)));
								element_link_count++;
								std::cout << "Made link" << std::endl;
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
						std::cout << "Element " << &section(s).element(i) << " links to node "
						          << &(section(s).element(i).node(j)) << " which currently links to "
						          << o << " more elements." << std::endl;
						for (int k=o; k > 0; k--) {
							if (&section(s).element(i) == &(section(s).element(i).node(j).element(k-1))) {
								std::cout << "Skipping - don't need to link to self!" << std::endl;
							} else {
								std::cout << "Making link from " << &section(s).element(i) << " to "
								          << &(section(s).element(i).node(j).element(k-1)) << std::endl;
								tds_element_link* new_link = new tds_element_link(&section(s).element(i),
								                                                  &(section(s).element(i).node(j).element(k-1)));
								element_link_count++;
								std::cout << "Made link" << std::endl;
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
	} else {
		std::cout << "Element links could not be made, only 1-D programmed." << std::endl;
	}

	output_model_summary(true,true,true,true,true);
	
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
