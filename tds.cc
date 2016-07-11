#include "tds.hh"

void operator+=(vector<float>& u, const vector<float>& v) { u = u+v; }
void operator-=(vector<float>& u, const vector<float>& v) { u = u-v; }
void operator*=(vector<float>& u, const vector<float>& v) { u = u*v; }
void operator*=(vector<float>& u, const float& v) { u = u*v; }
void operator*=(const float& v, vector<float>& u) { u = u*v; }

vector<float> operator+(const vector<float>& u, const vector<float>& v) {
	vector<float> w;
	w.reserve(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] += v[i];
	}
	return w;
}
vector<float> operator-(const vector<float>& u, const vector<float>& v) {
	vector<float> w;
	w.reserve(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] -= v[i];
	}
	return w;
}
vector<float> operator*(const vector<float>& u, const vector<float>& v) {
	vector<float> w;
	w.reserve(u.size());
	for (int i = 0; i < u.size(); i++) {
		w[i] = u[i];
		if (i < v.size()) w[i] *= v[i];
	}
	return w;
}
vector<float> operator*(const vector<float>& u, const float& v) {
	vector<float> w;
	w.reserve(u.size());
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

tds_element::tds_element(tds_nodes _nodes, tds_material* _material, float _contamination):nodes_(_nodes),material_(_material),contaminationA_(_contamination) {
	// no specific centre point has been provided, so use COM for the element type
	// e.g. triangular element r_COM = r_A + (2/3) * (r_AB + 0.5 * r_BC)
	set_origin_from_nodes();
	flagAB(false);
}

tds_element::tds_element(tds_nodes _nodes, tds_material* _material, vector<float> _origin, float _contamination):nodes_(_nodes),material_(_material),origin_(_origin),contaminationA_(_contamination) {
	flagAB(false);
}
tds_element::tds_element(tds_nodes _nodes, tds_material* _material, float _origin_x, float _origin_y, float _origin_z, float _contamination):nodes_(_nodes),contaminationA_(_contamination) {
	origin_.reserve(3);
	origin(1,_origin_x);
	origin(2,_origin_y);
	origin(3,_origin_z);
	flagAB(false);
}

tds_element::~tds_element(){
}

void tds_element::transfer_contaminant(float _quantity) {
	contamination(contamination()+_quantity/size());
	flagAB(!flagAB());
}
void tds_element::set_origin_from_nodes() {
	origin_.reserve(3);
	int n_nodes = nodes_.size();
	float x,y,z;
	
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
		x /= n_nodes;
		y /= n_nodes;
		z /= n_nodes;
		break;
	default:
		std::cout << "!!! Looking for origin of " << nodes_.size() << " noded element, not programmed yet " << endl;
	}
	
	origin(0,x);
	origin(1,y);
	origin(2,z);
}

void tds_element::update(float delta_T) {//method to update parameters
	float total_flow = 0.0f;
	for (int i = 0; i < n_neighbours(); i++) {
		total_flow += neighbour(i).flow_rate(this->flagAB()) * neighbour(i).positive_flow(this);
	}
}
 

tds_section::tds_section():elements_(){	
}

tds_section::~tds_section(){
	clean_elements();
}

void tds_section::add_element(tds_element* new_element){
	std::cout<<"adding new element"<<endl;
	elements_.push_back(new_element);
}

void tds_section::clean_elements(){
	for (int i=0; i<elements_.size(); ++i) delete elements_[i];
	elements_.resize(0);
}

tds_element_link::tds_element_link(tds_element* _M, tds_element* _N):elementM_(_M),elementN_(_N) {
	this->initialise();
}
tds_element_link::~tds_element_link() {
}
void tds_element_link::initialise() {
	norm_vector_.reserve(3);
	flux_vector_.reserve(3);

	// get flux vector, which is the vector from COM of one element to the COM of the next
	flux_vector(elementN_->origin()-elementM_->origin());

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
	if (_AB != flagAB()) {
		flow_rate_ = ( a_n_dot_eMN_over_modMN() *
		               (elementN().contamination() - elementM().contamination()) *
		               elementM().material().diffusion_constant() );
		flagAB(_AB);
	}
	return flow_rate_;
}
	
tds::tds():sections_(){
}

tds::~tds(){
	std::cout<<"cleaning sections"<<endl;
	clean_sections();
}

void tds::add_section(tds_section* new_section){
	std::cout<<"adding a new section"<<endl;
	sections_.push_back(new_section);
}

void tds::clean_sections(){
	for (int i=0; i<sections_.size(); ++i) delete sections_[i];
	sections_.resize(0);
}

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

void tds_run::make_analysis(int event_num, float thresh_u, float thresh_l, bool rms, float noise, float m_baseline, bool man_base, bool pretrig){
	std::cout<<"analyse timeline"<<endl;
}

void tds_run::basename(std::string _basename) {
	basename_ = _basename;
}

void tds_run::initialise() {

	// Before we can read in any data, we should look for
	// any settings about units we can find
	conversion _conversion((basename() + ".units").c_str());
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
		std::string _name, _density_unit, _diffusion_unit;
		double _density, _diffusion_constant;
		
		if (!(iss >> _name >> _density >> _density_unit >> _diffusion_constant >> _diffusion_unit)) {
			std::cerr << "Invalid line, skipping.\n";
			continue;
		}

		std::cout << "We obtained five values [" << _name << ", " << _density << ", " << _density_unit << ", " << _diffusion_constant << ", " << _diffusion_unit << "].\n";

		//tds_material _m(_name, _density, _diffusion_constant);
	}
	//Now we've got materials, let's get the physical sections which use them
	while (std::getline(sectionsfile_, line)) {
		std::istringstream iss(line);
		int dim, id;
		std::string name;
		
		if (!(iss >> dim >> id >> name)) {
			std::cerr << "Invalid line, skipping.\n";
			continue;
		}

		std::cout << "We obtained three values [" << dim << ", " << id << ", " << name << "].\n";
	}
	cout << "gonna close me some files" << endl;
	sectionsfile_.close();
	elementsfile_.close();
	nodesfile_.close();
}

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
