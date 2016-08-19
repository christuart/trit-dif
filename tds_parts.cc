#include "tds_parts.hh"

tds_part::tds_part() {}
tds_part::~tds_part() {}
bool tds_part::is_base() { return true; }









/******************** TDS_NODE METHODS ********************/









tds_node::tds_node(double _x, double _y, double _z) {
	position_.reserve(3);
	position_.push_back(_x);
	position_.push_back(_y);
	position_.push_back(_z);
}
tds_node::~tds_node() {
}

void tds_node::add_element(tds_element* new_element) {
	//std::cout<<"adding new element for node"<<std::endl;
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









tds_element::tds_element(tds_nodes _nodes, tds_section* _section, double _contamination):nodes_(_nodes),section_(_section),contaminationA_(_contamination),contaminationB_(_contamination) {
	// no specific centre point has been provided, so use COM for the element type
	// e.g. triangular element r_COM = r_A + (2/3) * (r_AB + 0.5 * r_BC)
	origin_.reserve(3);
	set_origin_from_nodes();
	calculate_size();
	material_ = &(_section->material());
	flagAB(false);
}

tds_element::tds_element(tds_nodes _nodes, tds_section* _section, std::vector<double> _origin, double _contamination):nodes_(_nodes),section_(_section),origin_(_origin),contaminationA_(_contamination) {
	calculate_size();
	material_ = &(_section->material());
	flagAB(false);
}
tds_element::tds_element(tds_nodes _nodes, tds_section* _section, double _origin_x, double _origin_y, double _origin_z, double _contamination):nodes_(_nodes),contaminationA_(_contamination) {
	origin_.reserve(3);
	origin(_origin_x, _origin_y, _origin_z);
	calculate_size();
	material_ = &(_section->material());
	flagAB(false);
}

tds_element::~tds_element(){
}

void tds_element::add_element_link(tds_element_link* new_element_link) {
	neighbours_.push_back(new_element_link);
}

void tds_element::transfer_contaminant(double _quantity) {
	// std::cout << "Received quantity of " << _quantity << " and have size() of "
	//            << size() << std::endl;
	contamination(contamination()+_quantity/size());
	flagAB(!flagAB());
}
void tds_element::set_origin_from_nodes() {
	//std::cout << "Setting origin from nodes." << std::endl;
	origin_.reserve(3);
	int n_nodes = nodes_.size();
	double x,y,z;
	// std::cout << "There are " << n_nodes << " nodes." << std::endl;
	
	switch (n_nodes) {
	case 2:
	case 3:
	case 4:
		// can use the mean of the vectors for basic elements
		// might be able to for other elements, need to think
		// about/do the integrations for these
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
		std::cerr << "!!! Looking for origin of " << nodes_.size() << " noded element, not programmed yet " << std::endl;
	}
	
	origin(x,y,z);
	// debug(&origin());
}

void tds_element::update(double delta_t) {//method to update parameters
	double total_flow = 0.0f;
	for (int i = 0; i < n_neighbours(); i++) {
		total_flow += neighbour(i).flow_rate(this->flagAB()) * neighbour(i).positive_flow(this);
		//std::cout << "Flow is now " << total_flow << std::endl;
	}
	// std::cout << "Flow rate into " << this << " is " << total_flow << std::endl;
	// std::cout << "Takes contamination from " << contamination();
	transfer_contaminant(total_flow * delta_t);
	// std::cout << " to " << contamination() << std::endl;
}
void tds_element::propogate_into_nodes() {
	//std::cout << "Propogating at element " << this << " i.e. through " << n_nodes() << " nodes." << std::endl;
	for (int i=0; i < n_nodes(); i++) {
		node(i).add_element(this);
	}
}
void tds_element::calculate_size() {
	std::vector<double> vecPQ, vecPR;
	switch (n_nodes()) {
	case 2:
		vecPQ.resize(3);
		vecPQ = node(1).position() - node(0).position();
		if (vecPQ.at(1) == 0.0 && vecPQ.at(2) == 0.0) {
			size(fabs(vecPQ.at(1)-vecPQ.at(0)));
		} else {
			vecPQ *= vecPQ;
			size(sqrt(vecPQ.at(0)+vecPQ.at(1)+vecPQ.at(2)));
		}
		break;
	case 3:
		// triangle PQR, area is 1/2 mod(PQ x PR)
		vecPQ.resize(3);
		vecPQ = node(1).position() - node(0).position();
		// std::cout << "PQ: [" << vecPQ.at(0) << "; " << vecPQ.at(1)
		//           << "; " << vecPQ.at(2) << "]" << std::endl;
		vecPR.resize(3);
		vecPR = node(2).position() - node(0).position();
		// std::cout << "PR: [" << vecPR.at(0) << "; " << vecPR.at(1)
		//           << "; " << vecPR.at(2) << "]" << std::endl;
		if (vecPQ.at(2) == 0.0 && vecPR.at(2) == 0.0) {
		// std::cout << "Calculating size for triangular element " << this << std::endl;
		// std::cout << "This is from size(0.5 * fabs(" << vecPQ.at(1) << "*" << vecPR.at(0)
		//           << " - " << vecPQ.at(0) << "*" << vecPR.at(1) << "))" << std::endl;
		// std::cout << "=size(0.5 * fabs(" << vecPQ.at(1)*vecPR.at(0)-vecPQ.at(0)*vecPR.at(1) << "))" << std::endl;
			//2D cross product nice and simple
			size(0.5 * fabs(vecPQ.at(1)*vecPR.at(0) - vecPQ.at(0)*vecPR.at(1)));
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
bool tds_element::is_linked_to(tds_element* _element) {
	//std::cout << "Checking at element " << this << " whether linked to " << _element << std::endl;
	for (int i=0; i < n_neighbours(); ++i) {
		//std::cout << "Link " << i << " is with " << neighbour(i).neighbour_of(this) << std::endl;
		if ((neighbour(i).neighbour_of(this)) == _element)
			return true;
	}
	return false;
}









/******************** TDS_MATERIAL METHODS ********************/









tds_material::tds_material(std::string _name, double _density, double _diffusion_constant):material_name_(_name),material_density_(_density),material_diffusion_constant_(_diffusion_constant) {
}

tds_material::~tds_material() {
}









/******************** TDS_SECTION METHODS ********************/









tds_section::tds_section(std::string _name, tds_material* _material):elements_(),name_(_name),material_(_material) {	
}

tds_section::~tds_section() {
	clean_elements();
}

int tds_section::add_element(tds_element* new_element) {
	// std::cout<<"adding new element"<<std::endl;
	elements_.push_back(new_element);
	return elements_.size() - 1;
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
	norm_vector_.resize(3);
	flux_vector_.resize(3);

	// get flux vector, which is the vector from COM of one element to the COM of the next
	flux_vector(elementN_->origin(0)-elementM_->origin(0),
	            elementN_->origin(1)-elementM_->origin(1),
	            elementN_->origin(2)-elementM_->origin(2));

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
		// std::cout << "Jamaica: finding interface area length from ("
		//           << shared_node(0).position(0) << ","
		//           << shared_node(0).position(1) << ") to ("
		// 	  << shared_node(1).position(0) << ","
		//           << shared_node(1).position(1) << ")" << std::endl;
		norm_vector(0,shared_node(1).position(1) - shared_node(0).position(1));
		norm_vector(1,shared_node(0).position(0) - shared_node(1).position(0));
		// std::cout << "Rotated vector: " << norm_vector(0)
		//           << "," << norm_vector(1) << " i.e. magnitude "
		//           << magnitude(norm_vector()) << std::endl;
		if (shared_node(0).position(2) != 0.0f || shared_node(1).position(2) != 0.0f) {
			std::cout << "!!! non 2-D elements had a 2 node interface -- not physically accurate" << std::endl;
		}
		// make use of this rotated vector as a measure of interface length, then normalise it
		modMN(magnitude(flux_vector()));
		// std::cout << "norm_vector().size() = " << norm_vector().size() << std::endl;
		// std::cout << "Jamaica: " << magnitude(norm_vector()) << std::endl;
		interface_area(magnitude(norm_vector()));
		norm_vector_ *= (1/interface_area());
		// now make sure it is in the outward direction to follow standard conventions
		if (dot(norm_vector(),flux_vector()) < 0.0f) norm_vector_ *= -1;
		break;
	case 3:
		std::cerr << "!!! haven't implemented 3d element link initialisation" << std::endl;
		break;
	}
	
	// calculate the geometry multiplier to turn D * (diff in contamination) into flow rate
	a_n_dot_eMN_over_modMN(interface_area() * dot(norm_vector(),flux_vector())/(modMN()*modMN()) );
}
double tds_element_link::flow_rate(bool _AB) {
	// AB flag system used because the same flow needn't be calculated twice for M->N and N->M
	// instead we calculate it and store it, and only recalculate when the flag switches
	if (_AB != flagAB()) {
		// std::cout << "From " << elementN_ << " (cont " << elementN().contamination(_AB)
		//            << ") to " << elementM_ << " (cont " << elementM().contamination(_AB)
		//            << ") with andotemnovermodmn = " << a_n_dot_eMN_over_modMN() << " amd D = " << diffusion_constant() << std::endl;
		flow_rate_ = ( a_n_dot_eMN_over_modMN() *
		               (elementN().contamination(_AB) - elementM().contamination(_AB)) *
		               diffusion_constant() );
		flagAB(_AB);
	}
	return flow_rate_;
}
short tds_element_link::positive_flow(tds_element* whoami) {
	if (elementM_ == whoami) {
		return 1;
	} else {
		return -1;
	}
}
double tds_element_link::diffusion_constant() {
	double D1 = elementM().material().diffusion_constant();
	double D2 = elementN().material().diffusion_constant();
	if (D1 > D2) return D1;
	return D2;
}
void tds_element_link::set_flag_against(tds_element* _element) {
	flagAB(!_element->flagAB());
}
