#include "tds_parts.hh"
#define TOLERANCE 0.00001f
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
double get_3Dplanar_area(tds_nodes shared_nodes, std::vector<double> &e_n) {
	// Adapted from area3D_Polygon() by Dan Sunday
	// Found at: http://geomalgorithms.com/a01-_area.html#2D%20Polygons
	int n = shared_nodes.size();
	if (n < 3) {
		std::cerr << "Trying to find area of 'surface' defined by too few nodes." << std::endl;
		throw;
	}

	shared_nodes.push_back(shared_nodes.at(0));
	shared_nodes.push_back(shared_nodes.at(1));
	
	double area = 0;
	double an, ax, ay, az; // abs value of normal and its coords
	int coord;           // coord to ignore: 1=x, 2=y, 3=z
	int i, j, k;         // loop indices

    // select largest abs coordinate to ignore for projection
    ax = fabs(e_n.at(0));    // abs x-coord
    ay = fabs(e_n.at(1));     // abs y-coord
    az = fabs(e_n.at(2));     // abs z-coord

    coord = 3;                    // ignore z-coord
    if (ax > ay) {
        if (ax > az) coord = 1;   // ignore x-coord
    }
    else if (ay > az) coord = 2;  // ignore y-coord

    // compute area of the 2D projection
    switch (coord) {
      case 1:
        for (i=1, j=2, k=0; i<n; i++, j++, k++)
            area += (shared_nodes.at(i)->position(1) * (shared_nodes.at(j)->position(2) - shared_nodes.at(k)->position(2)));
        break;
      case 2:
        for (i=1, j=2, k=0; i<n; i++, j++, k++)
            area += (shared_nodes.at(i)->position(2) * (shared_nodes.at(j)->position(0) - shared_nodes.at(k)->position(0)));
        break;
      case 3:
        for (i=1, j=2, k=0; i<n; i++, j++, k++)
            area += (shared_nodes.at(i)->position(0) * (shared_nodes.at(j)->position(1) - shared_nodes.at(k)->position(1)));
        break;
    }
    switch (coord) {    // wrap-around term
      case 1:
        area += (shared_nodes.at(n)->position(1) * (shared_nodes.at(1)->position(2) - shared_nodes.at(n-1)->position(2)));
        break;
      case 2:
        area += (shared_nodes.at(n)->position(2) * (shared_nodes.at(1)->position(0) - shared_nodes.at(n-1)->position(0)));
        break;
      case 3:
        area += (shared_nodes.at(n)->position(0) * (shared_nodes.at(1)->position(1) - shared_nodes.at(n-1)->position(1)));
        break;
    }

    switch (coord) {
      case 1:
        area *= (1.0f / (2 * e_n.at(0)));
        break;
      case 2:
        area *= (1.0f / (2 * e_n.at(1)));
        break;
      case 3:
        area *= (1.0f / (2 * e_n.at(2)));
    }
    return area;
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
	case 5:
	case 6:
	case 8:
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
	std::vector<double> vecPQ, vecPR, vecPS, c;
	double d;
	std::vector<ordering4> orders;
	if (n_nodes() == 2) {
		vecPQ.resize(3);
		vecPQ = node(1).position() - node(0).position();
		if (vecPQ.at(1) == 0.0 && vecPQ.at(2) == 0.0) {
			size(fabs(vecPQ.at(1)-vecPQ.at(0)));
		} else {
			vecPQ *= vecPQ;
			size(sqrt(vecPQ.at(0)+vecPQ.at(1)+vecPQ.at(2)));
		}
	} else if (n_nodes() == 3) {
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
	} else if (n_nodes() == 4) {
		//std::cerr << "!!! HAVEN'T PROGRAMMED AREA/VOLUME OF QUADRANGLES OR TETRAHEDRA YET!" << std::endl;
		vecPQ = node(1).position() - node(0).position();
		vecPR = node(2).position() - node(0).position();
		vecPS = node(3).position() - node(0).position();
		// debug(&vecPQ);
		// debug(&vecPR);
		// debug(&vecPS);
		// if quadrangle, all 3 vectors are planar, so PQ . PRxPS = 0. This happens to be part of the volume formula, so let's calculate that first
		c.resize(3);
		c = cross(vecPQ,vecPR);
		// debug(&c);
		d = dot(c,vecPS);
		// std::cout << d << std::endl;
		// std::cout << 0.00001f * magnitude(vecPQ) * magnitude(vecPR) * magnitude(vecPS) << std::endl;
		if (d < TOLERANCE * magnitude(vecPQ) * magnitude(vecPR) * magnitude(vecPS)) {
			// for quadrangles, use this: http://geomalgorithms.com/a01-_area.html#2D%20Polygons
			// (link from http://stackoverflow.com/a/717367)
			// formula for arbitary quadrilateral, but careful of intersections:
			// 1/2 | ob x ac |
			// find four areas from node ordering and pick the largest.
			d =  0.0f;
			orders.push_back(make_order(0, 1, 2, 3));
			orders.push_back(make_order(0, 2, 1, 3));
			orders.push_back(make_order(2, 0, 1, 3));
			orders.push_back(make_order(1, 0, 2, 3));
			orders.push_back(make_order(1, 2, 0, 3));
			for (int i = 0; i < 5; ++i) {
				// ignore that PQ, PR namings are off here, may as well reuse variables
				// vecPQ =   node( (offset == 0) ? 3 : (0 - ((offset < 0) ? 1 : 0)) ).position()
				// 	- node( (offset == 1) ? 3 : (1 - ((offset < 1) ? 1 : 0)) ).position();
				// vecPR =   node( (offset == 2) ? 3 : (2 - ((offset < 2) ? 1 : 0)) ).position()
				// 	- node( (offset == 3) ? 3 : (3 - ((offset < 3) ? 1 : 0)) ).position();
				vecPQ =   node( orders.at(i).p1 ).position()
					- node( orders.at(i).p2 ).position();
				vecPR =   node( orders.at(i).p3 ).position()
					- node( orders.at(i).p4 ).position();
				c = cross(vecPQ,vecPR);
				debug(&c);
				d = std::max(d, magnitude(c));
			}
			size(d / 2.0f);
			//std::cout << "element with 4 nodes, quad, size: " << size() << std::endl;
			
		} else {
			// For tetrahedra:
			// let's call the four vertices a, b, c and d.
			// now let's set d as the origin, giving us vectors oa, ob, oc.
			// the volume is equal to 1/6 | oa . (ob x oc) |
			size(fabs(d / 6.0f));
			//std::cout << "element with 4 nodes, tet, size: " << size() << std::endl;
		}
	} else if (n_nodes() == 5) {
		std::cerr << "!!! HAVEN'T PROGRAMMED AREA/VOLUME OF PYRAMIDAL ELEMENTS YET!" << std::endl;
		// This will involve a 1/3 * base * height method
		// The most difficult part will be identifying the base nodes vs point node
		// This will be done by checking parallelism WHICH mustn't be too sensitive
		// to floating point error!
		// 
		// Logic behind finding the point node (non-base node):
		// 
		throw;
	} else if (n_nodes() == 8) {
		// for (int i = 0; i < 8; ++i)
		// 	std::cout << "Node " << i << ": ["
		// 	          << node(i).position(0) << ", "
		// 	          << node(i).position(1) << ", "
		// 	          << node(i).position(2) << "]" << std::endl;
		double magPQ, magPR, magPS;
		std::vector<double> c2,c3;
		int diagonal, times_found_self_diagonal;
		int ii,jj,kk;
		for (int i = 1; i < 8; ++i) { // which corner are we checking to see if it is diag opposite
			ii = 1+((i-1)%7); // always = i because i < 8
			// std::cout << "See if node " << ii << " is the diag opposite node to node 0." << std::endl;
			vecPQ = node(ii).position() - node(0).position();
			magPQ = magnitude(vecPQ);
			times_found_self_diagonal = 0;
			for (int j = i+1; j < i+6; ++j) {
				jj = 1+((j-1)%7);
				// std::cout << "\tUse node " << jj << std::endl;
				vecPR = node(jj).position() - node(0).position();
				magPR = magnitude(vecPR);
				c = cross(vecPQ,vecPR);
				for (int k = j+1; k < i+7; ++k) {
					kk = 1+((k-1)%7);
					// std::cout << "\t\tand node " << kk << " to try making a plane." << std::endl;
					vecPS = node(kk).position() - node(0).position();
					magPS = magnitude(vecPS);
					d = dot(vecPS,c);
					if (fabs(d) < TOLERANCE * magPQ * magPR * magPS) { // three vecs are planar
						// std::cout << "\t\t\tThey make a plane..." << std::endl;
						c2 = cross(vecPQ,vecPS);
						if (dot(c,c2) < 0) { // PQ is in the middle, so diagonal
							++times_found_self_diagonal;
							// std::cout << "\t\t\t...and node " << ii << " was middle vector: " << times_found_self_diagonal << std::endl;
							if (times_found_self_diagonal == 3) {
								// This only happens when we find the diag opp node
								diagonal = ii;
								i = 8;
								k = i+7;
								j = i+6;
							}
						} else {
							// std::cout << "\t\t\t...but node " << ii << " wasn't middle vector." << std::endl;
							// node i is not the diagonally opposite node, so move on
							k = i+7;
							j = i+6;
						}
					} else {
						// std::cout << "\t\t\tNope: " << fabs(d) << std::endl;
					}
				}
			}		
		}
		// Two things may have happened by now:
		//  -1 We found the diagonally opposite node, meaning that the element is a parallelipiped.
		//     We can move directly to the edge finding algorithm, ignoring the diagonally opp node
		//  -2 We didn't find a diagonally opposite node, so the element is not a parallelipiped.
		//     We can move directly to the edge finding algorithm, because we won't stumble on
		//     unexpected extra planes
		if (times_found_self_diagonal != 3) {
			diagonal = 8;
			//std::cerr << "Didn't manage to find the diagonally opposite node." << std::endl;
			//throw;
		}
		// Start with any vector which isn't the diagonally opposite node
		int start_edge_candidate = 1+ (diagonal%7);
		vecPQ = node(start_edge_candidate).position() - node(0).position();
		magPQ = magnitude(vecPQ);
		// Get ready to store the nodes which make edges from node 0 and
		// the nodes which make diagonals
		std::vector<int> edge_nodes;
		std::vector<int> diagonal_nodes;
		bool start_edge_candidate_is_edge = false; // we don't know yet
		for (int i = 1; i < 6; ++i) {
			int ii = 1+((start_edge_candidate+i-1) % 7);
			if (ii == diagonal) continue;
			vecPR = node(ii).position() - node(0).position();
			magPR = magnitude(vecPR);
			c = cross(vecPQ,vecPR);
			for (int j=1; (i+j < 7 && i > 0); ++j) {
				int jj = 1+((j + ii - 1) % 7);
				if (jj == diagonal) continue;
				vecPS = node(jj).position() - node(0).position();
				magPS = magnitude(vecPS);
				d = dot(vecPS,c);
				if (fabs(d) < TOLERANCE * magPQ * magPR * magPS) {
					// We have found three vectors through nodes leading from node 0 which are co-planar
					// That means that none of the 3 nodes can be the node diagonally opposite to node 0
/*					candidates.erase(std::remove(candidates.begin(), candidates.end(), start_edge_candidate), candidates.end());
					candidates.erase(std::remove(candidates.begin(), candidates.end(), i), candidates.end());
					candidates.erase(std::remove(candidates.begin(), candidates.end(), j), candidates.end());
					std::cout << std::endl << "C: "; for (int iii = 0; iii < candidates.size(); ++iii) std::cout << candidates.at(iii) << ", " << std::flush;*/
					// The three nodes and node 0 describe a quadrilateral face. We want to find which
					// nodes 
					// try to identify edges vs diagonals
					// this is achieved by looking at the angles between them.
					// we will always find an unintentional extra plane which includes an
					// edge, a diagonal and the opposite diagonal, (draw it out to understand)
					// with that diagonal being marked as an edge.
					c2 = cross(vecPQ,vecPS);
					if (dot(c,c2) < 0) {
						// This means that cross 1 and cross 2 were in different directions, so PQ is the diag
						// and our 'start_edge_candidate' is known not to be an edge. We therefore want to
						// start the loop again, from i=1, with a new vecPQ that we know is an edge.
						edge_nodes.clear();
						diagonal_nodes.clear(); // If I understand correctly, this will already be empty.
						start_edge_candidate = ii;
						vecPQ = node(start_edge_candidate).position() - node(0).position();
						magPQ = magnitude(vecPQ);
						i = 0; // ++i will happen before we start the loop again
						continue;
						// This continue is unnecessary, but it makes the point that the loop
						// sequence is being broken; it will increment j, and then check that
						// (j < 8 && i > 0) is still true. This will not be the case, so the j
						// for loop finishes, and the i for loop "moves on". It in fact starts
						// again, incrementing i to '1' and looping through i up to 6.
					} else {
						// we know that PQ is an edge
						if (!start_edge_candidate_is_edge) {
							start_edge_candidate_is_edge = true;
							edge_nodes.push_back(start_edge_candidate);
						}
						// now we need to find the third cross product to work out which is the other edge
						c3 = cross(vecPR,vecPS);
						if (dot(c2,c3) > 0) {
							// this means that PQ to PR, PQ to PS and PR to PS are all the same
							// direction of rotation, so PR must be in the middle
							edge_nodes.push_back(jj);
							diagonal_nodes.push_back(ii);
						} else {
							// this means that PQ to PR and PQ to PS a rotation in one direction
							// whilst PR to PS is rotation in the opposite; PS is in the middle
							edge_nodes.push_back(ii);
							diagonal_nodes.push_back(jj);
						}
					}
				}
			}
			// std::cout << std::endl << "Finishing the ii of " << ii << " and found " << edge_nodes.size() << " edges and " << diagonal_nodes.size() << " diagonals.";
		}
		// Now that we have two diagonal corners, we will get 3 faces at each, using them to calculate
		// the volume via the divergence rule (Nürnberg, 2013). This could have been done as a part of the
		// previous loop, but the slight sacrifice in model initialisation speed is worth the extra clarity
		// in the code, as why the algorithm works is not clear by looking at it - no point in making it
		// difficult to tell what the algorithm is even doing too.
		// 
		// In the above algorithm, as well as the diagonal we should have identified 3 edges from node 0
		// and 2 diagonals. We need to now find that third diagonal.
		if (edge_nodes.size() != 3) {
			std::cerr << "Didn't find three nodes that make edges from node 0." << std::endl;
			throw;
		}
		if (diagonal_nodes.size() != 2) {
			std::cerr << "Didn't find the first two nodes that make diagonals from node 0." << std::endl;
			throw;
		}
		// The third diagonal is clearly the last node
		std::vector<int> candidates;
		for (int i = 1; i < 8; ++i) candidates.push_back(i);
		for (int i = 0; i < 2; ++i) {
			candidates.erase(std::remove(candidates.begin(), candidates.end(), edge_nodes.at(i)), candidates.end());
			candidates.erase(std::remove(candidates.begin(), candidates.end(), diagonal_nodes.at(i)), candidates.end());
		}
		candidates.erase(std::remove(candidates.begin(), candidates.end(), edge_nodes.at(2)), candidates.end());
		candidates.erase(std::remove(candidates.begin(), candidates.end(), diagonal), candidates.end());
		if (candidates.size() == 1) {
			// We found the last diagonal.
			diagonal_nodes.push_back(candidates.back());
		} else {
			std::cerr << "Failed to find the last diagonal node." << std::endl;
			throw;
		}
		
			
		double vol_sum = 0.0f;
		for (int i = 0; i < 3; ++i) {
			vecPQ = node(edge_nodes.at(   i   )).position() - node(0).position();
			vecPR = node(edge_nodes.at((i+1)%3)).position() - node(0).position();
			c = cross(vecPQ,vecPR);
			normalise(c); // this now makes it the n vec for that face
			vol_sum += dot(c,node(0).position());
			vecPQ = node(diagonal_nodes.at(   i   )).position() - node(diagonal).position();
			vecPR = node(diagonal_nodes.at((i+1)%3)).position() - node(diagonal).position();
			c = cross(vecPQ,vecPR);
			normalise(c); // this now makes it the n vec for that face
			vol_sum += dot(c,node(diagonal).position());
		}
		// The Nürnberg algorithm looks at triangular faces, so we would have to divide each quad face
		// into two. It then divides the sum by 6. The terms in the sum for the two triangles would both
		// equal the one term we have for each quad, so we need to multiply our answer by 2 at the end.
		// We will instead divide by only 3 to achieve the same effect.
		size(fabs(vol_sum/3.0f));
		// std::cout << "Volume of this hexahedron: " << size() << std::endl;
	} else {
		// then for 3-D planar polygons >4 look at the 3-D polygon area using projection
		// onto 2D - really clever! http://geomalgorithms.com/a01-_area.html#2D%20Polygons
		std::cerr << "!!! HAVEN'T PROGRAMMED AREA/VOLUME OF THIS ELEMENT SHAPE YET!" << std::endl;
		throw;
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
	modMN(magnitude(flux_vector()));

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
	if (shared_nodes_.size() == 1) {
		// norm_vector and flux_vector are always the same direction in 1-D
		norm_vector(flux_vector());
		norm_vector_ *= (1.0f/modMN());
		if (norm_vector(1) != 0.0f || norm_vector(2) != 0.0f) {
			std::cout << "!!! non 1-D elements had a 1 node interface -- not physically accurate" << std::endl;
		}
		interface_area(1.0f);
	} else if (shared_nodes_.size() == 2) {
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
		// std::cout << "norm_vector().size() = " << norm_vector().size() << std::endl;
		// std::cout << "Jamaica: " << magnitude(norm_vector()) << std::endl;
		interface_area(magnitude(norm_vector()));
		norm_vector_ *= (1.0f/interface_area());
		// now make sure it is in the outward direction to follow standard conventions
		if (dot(norm_vector(),flux_vector()) < 0.0f) norm_vector_ *= -1;
	} else if (shared_nodes_.size() > 2) {
		std::vector<double> a,b,c;
		a = shared_node(1).position() - shared_node(0).position();
		b = shared_node(2).position() - shared_node(0).position();
		c = cross(a,b);
		norm_vector_ = c * (1.0f/magnitude(c));
		if (dot(flux_vector(),norm_vector()) < 0.0f)
			norm_vector_ *= -1.0f;
		interface_area(fabs(get_3Dplanar_area(shared_nodes_,norm_vector())));
		// std::cerr << "!!! haven't implemented 3d element link initialisation" << std::endl;
	} else {
		std::cerr << "Trying to initialise element link between two elements with no known common nodes." << std::endl;
		throw;
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
