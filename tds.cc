#include "tds.hh"

/******************** VECTOR METHODS ********************/
void operator+=(vector<float>& u, const vector<float>& v) { u = u+v; }
void operator-=(vector<float>& u, const vector<float>& v) { u = u-v; }
void operator*=(vector<float>& u, const vector<float>& v) { u = u*v; }
void operator*=(vector<float>& u, const float& v) { u = u*v; }
void operator*=(const float& v, vector<float>& u) { u = u*v; }

vector<float> operator+(const vector<float>& u, const vector<float>& v) {
	vector<float> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] += v[i];
	}
	return w;
}
vector<float> operator-(const vector<float>& u, const vector<float>& v) {
	vector<float> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] -= v[i];
	}
	return w;
}
vector<float> operator*(const vector<float>& u, const vector<float>& v) {
	vector<float> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] *= v[i];
	}
	return w;
}
vector<float> operator*(const vector<float>& u, const float& v) {
	vector<float> w;
	w.resize(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = v*u[i];
	}
	return w;
}
vector<float> operator*(const float& v, const vector<float>& u) {
	return u*v;
}
float dot(const vector<float>& u, const vector<float>& v) {
	float w = 0.0f;
	for (int i = 0; i < u.size() && i < v.size(); i++) {
		w += v[i]*u[i];
	}
	return w;
}
vector<float> cross(const vector<float>& u, const vector<float>& v) {
	vector<float> w;
	w.reserve(3);
	for (int i = 0; i < u.size() && i < v.size(); i++) {
		
	}
	return w;
}
float magnitude(vector<float>& u) {
	float m = 0.0f;
	for (int i = 0; i < u.size(); i++) {
		m += u[i]*u[i];
	}
	return sqrt(m);
}
void normalise(vector<float>& u) {
	float m = magnitude(u);
	u *= (1/m);
}

/******************** TDS_NODE METHODS ********************/
tds_node::tds_node(float _x, float _y, float _z) {
	position_.reserve(3);
	position_.push_back(_x);
	position_.push_back(_y);
	position_.push_back(_z);
}
tds_node::~tds_node() {
}

void tds_node::add_element(tds_element* new_element) {
	std::cout<<"adding new element for node"<<endl;
	// elements_.push_front(new_element);
	elements_.push_back(new_element);
}
void tds_node::clean_elements() {
	// while (!elements_empty()) elements_.pop_front();
}
void tds_node::remove_last_element() {
	elements_.pop_back();
}

/******************** TDS_ELEMENT METHODS ********************/
tds_element::tds_element(tds_nodes _nodes, tds_material* _material, float _contamination):nodes_(_nodes),material_(_material),contaminationA_(_contamination) {
	// no specific centre point has been provided, so use COM for the element type
	// e.g. triangular element r_COM = r_A + (2/3) * (r_AB + 0.5 * r_BC)
	origin_.reserve(3);
	set_origin_from_nodes();
	flagAB(false);
}

tds_element::tds_element(tds_nodes _nodes, tds_material* _material, vector<float> _origin, float _contamination):nodes_(_nodes),material_(_material),origin_(_origin),contaminationA_(_contamination) {
	flagAB(false);
}
tds_element::tds_element(tds_nodes _nodes, tds_material* _material, float _origin_x, float _origin_y, float _origin_z, float _contamination):nodes_(_nodes),contaminationA_(_contamination) {
	origin_.reserve(3);
	origin(_origin_x, _origin_y, _origin_z);
	flagAB(false);
}

tds_element::~tds_element(){
}

void tds_element::add_element_link(tds_element_link* new_element_link) {
	neighbours_.push_back(new_element_link);
}

void tds_element::transfer_contaminant(float _quantity) {
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
		std::cout << "!!! Looking for origin of " << nodes_.size() << " noded element, not programmed yet " << endl;
	}
	std::cout << "Average of vectors is [" << x << "," << y << "," << z << "]." << std::endl;
	
	origin(x,y,z); std::cout << x << std::endl;
	std::cout << "asdasdasdasd " << origin().size() << std::endl;
	debug(&origin());
}

void tds_element::update(float delta_T) {//method to update parameters
	float total_flow = 0.0f;
	for (int i = 0; i < n_neighbours(); i++) {
		total_flow += neighbour(i).flow_rate(this->flagAB()) * neighbour(i).positive_flow(this);
	}
}
void tds_element::propogate_into_nodes() {
	std::cout << "Propogating at element " << this << " i.e. through " << n_nodes() << " nodes." << std::endl;
	for (int i=0; i < n_nodes(); i++) {
		node(i).add_element(this);
	}
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
	std::cout<<"adding new element"<<endl;
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
	vector<float> asd = elementM_->origin();

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
			std::cout << "!!! non 1-D elements had a 1 node interface -- not physically accurate" << endl;
		}
		interface_area(1.0f);
		break;
	case 2:
		// first get the vector along the edge, but rotated 90deg, i.e. [y; -x]
		norm_vector(0,shared_node(1).position(1) - shared_node(0).position(1));
		norm_vector(1,shared_node(0).position(0) - shared_node(1).position(0));
		if (shared_node(0).position(2) != 0.0f || shared_node(1).position(2) != 0.0f) {
			std::cout << "!!! non 2-D elements had a 2 node interface -- not physically accurate" << endl;
		}
		// make use of this rotated vector as a measure of interface length, then normalise it
		modMN(magnitude(flux_vector()));
		interface_area(magnitude(norm_vector()));
		norm_vector_ *= (1/interface_area());
		// now make sure it is in the outward direction to follow standard conventions
		if (dot(norm_vector(),flux_vector()) < 0.0f) norm_vector_ *= -1;
		break;
	case 3:
		std::cout << "!!! haven't implemented 3d element link initialisation" << endl;
		break;
	}
	
	// calculate the geometry multiplier to turn D * (diff in contamination) into flow rate
	a_n_dot_eMN_over_modMN(interface_area() * dot(norm_vector(),flux_vector())/(modMN()*modMN()) );
}
float tds_element_link::flow_rate(bool _AB) {
	// AB flag system used because the same flow needn't be calculated twice for M->N and N->M
	// instead we calculate it and store it, and only recalculate when the flag switches
	if (_AB != flagAB()) {
		flow_rate_ = ( a_n_dot_eMN_over_modMN() *
		               (elementN().contamination() - elementM().contamination()) *
		               elementM().material().diffusion_constant() );
		flagAB(_AB);
	}
	return flow_rate_;
}
	
/******************** TDS METHODS ********************/
tds::tds():sections_(){
	element_dimensions_ = 0x0;
}

tds::~tds(){
	std::cout<<"cleaning sections"<<endl;
	clean_sections();
}

void tds::add_section(tds_section* new_section){
	std::cout<<"adding a new section"<<endl;
	sections_.push_back(new_section);
}
void tds::add_material(tds_material* new_material){
	std::cout << "adding a new material: " << (*new_material).name() << std::endl;
	material_map_[(*new_material).name()] = new_material;
	materials_.push_back(new_material);
}
void tds::add_node(tds_node* new_node){
	std::cout<<"adding a new node"<<endl;
	nodes_.push_back(new_node);
}
void tds::add_element(tds_element* new_element){
	std::cout<<"adding a new element"<<endl;
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
			if (show_element_links) {
				for (int j = 0; j < element(i).n_neighbours(); j++) {
					std::cout << "\tNeighbour " << (j+1) << ": "
					          << (element(i).neighbour(j).neighbour_of(&element(i)))
					          << std::endl;
				}
			}
		}
	}
	if (show_nodes) {
		for (int i = 0; i < n_elements(); i++) {
			std::cout << "Node " << (i+1) << " (" << &node(i) << "): [" << node(i).position(0) << ","
			          << node(i).position(1) << "," << node(i).position(2) << "] - "
			          << node(i).n_elements() << " elements" << std::endl;
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
//	std::cout<<"close tds file"<<endl;
// 	tdsfile_.close();	
// }

// void tds_run::tdsfile_open(string filename){
// 	std::cout<<"opening a tds file"<<endl;
// 	tdsfile_close();
// 	clean_sections();
// 	put_filename(filename);
// 	tdsfile_.open(filename.c_str());
// 	read_file();
// }

// void tds_run::read_file(){
// 	std::cout<<"read file and add sections"<<endl;
// 	//Add in a section
// 	add_section(new tds_section());
// }

void tds_run::make_analysis(float deltaT, int _steps, tds_elements tracked_elements) {
	std::cout << "Run the model..." << endl;
}

void tds_run::initialise() {

	// Before we can read in any data, we should look for
	// any settings about (measurement) units we can find
	conversion _conversion((configname() + ".units").c_str());
	units(&_conversion);
	cout << "7.85 g/cm^3 in SI units is: " << units().convert_density_from("g/cm^3",7.85f) << endl;
	
	// Now read in data
	// First open all the necessary files (no point in getting half way through
	// processing only to find a file we need is missing)
	cout << "gonna open me some files" << endl;
	materialsfile_.open((basename() + ".materials").c_str());
	sectionsfile_.open((basename() + ".sections").c_str());
	elementsfile_.open((basename() + ".elements").c_str());
	nodesfile_.open((basename() + ".nodes").c_str());
	
	cout << "gonna read me some files" << endl;
	std::string line;
		
	//Let's start off by populating tds with some materials
	while (std::getline(materialsfile_, line)) {
		std::istringstream iss(line);
		std::string _name, _density_unit, _diffusion_constant_unit;
		double _density, _diffusion_constant;
		
		if (!(iss >> _name >> _density >> _density_unit >> _diffusion_constant >> _diffusion_constant_unit)) {
			std::cerr << "Invalid line, skipping.\n";
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
	while (std::getline(sectionsfile_, line)) {
		std::istringstream iss(line);
		int _dim, _id;
		std::string _name;
		
		if (!(iss >> _dim >> _id >> _name)) {
			std::cerr << "Invalid line, skipping.\n";
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
	while (std::getline(nodesfile_, line)) {
		std::istringstream iss(line);
		int id;
		float _x, _y, _z;
		
		if (!(iss >> id >> _x >> _y >> _z)) {
			std::cerr << "Invalid line, skipping.\n";
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
	while (std::getline(elementsfile_, line)) {
		std::istringstream iss(line);
		int id, type, n_tags, section_id, entity_id;
		std::ostringstream extra_tags;
		
		if (!(iss >> id >> type >> n_tags)) {
			std::cerr << "Invalid line, skipping.\n";
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
	std::cout << "gonna close me some files" << std::endl;
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
	if (!(element_dimensions(two_d) || element_dimensions(three_d) || element_dimensions(second_order_or_worse)) ||
	    !(element_dimensions(one_d))
	    ) {
		// We don't want to use the global elements list, but instead
		// iterate through each section's list of elements. This allows
		// us to treat source and/or outgassing sections differently
		// from the rest. (next to be implemented...)
		// for (int s = 0; s < n_sections(); s++ ) {
		int n, m, o;
		n = n_elements();
		for (int i=0; i < n; i++) {
			m = element(i).n_nodes();
			for (int j=0; j < m; j++) {
				o = element(i).node(j).n_elements();
				std::cout << "Element " << &element(i) << " links to node " << &(element(i).node(j)) << " which currently links to " << o << " more elements." << std::endl;
				for (int k=o; k > 0; k--) {
					if (&element(i) == &(element(i).node(j).element(k-1))) {
						std::cout << "Skipping - don't need to link to self!" << std::endl;
					} else {
						std::cout << "Making link from " << &element(i) << " to "
						          << &(element(i).node(j).element(k-1)) << std::endl;
						tds_element_link* new_link = new tds_element_link(&element(i),
						                                   &(element(i).node(j).element(k-1)));
						element_link_count++;
						std::cout << "Made link" << std::endl;
						element(i).add_element_link(new_link);
						element(i).node(j).element(k-1).add_element_link(new_link);
					}
					element(i).node(j).remove_last_element();
				}
			}
		}
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
		std::cout<<"open"<<endl;
		filename(filePtr);
		load_section(0);
		FRootfileName.text(filePtr);
	}
}

void tds_display::load_event(){
	int e_number = int(GUI_->event->value()), c_number = int(GUI_->channel->value());
	std::cout<<"event to load = "<<e_number<<endl;
}

void tds_display::load_section(int chnum){
	int c_number = int(GUI_->channel->value());
	std::cout<<"loading a section"<<endl;
	load_section(c_number,chnum);
}

void tds_display::load_section(unsigned int ch_n, int chnum){
	std::cout<<"load section"<<endl;
	FRootfileComments.text(display_tl_info().c_str());
}

void tds_display::resize_plot(int c){
	std::cout<<"resize plot"<<endl;
}

void tds_display::plot(){
	std::cout<<"make a plot"<<endl;		
}

void tds_display::makeZoomBox(selection sel,int event,int section){
	std::cout<<"zoom in"<<endl;
}

void tds_display::action(selection sel, Fl_Widget *sender){
	std::cout<<"un zoom"<<endl;
}

void tds_display::action(Fl_Widget *sender){
	std::cout<<"display action"<<endl;
}

/******************** TDS_BATCH METHODS ********************/
tds_batch::tds_batch(string filename, string rootout):infile_(filename.c_str()),rootfile_name_(rootout.c_str()),tds_run(){
}

tds_batch::~tds_batch(){
	infile_.close();
}

int tds_batch::run_batch(string filename, bool recreate, int filechain, int n_tot){
	int tot_element_num=0; 
	std::cout<<"make root file"<<endl;
	infile_.close();
	return tot_element_num;
}
