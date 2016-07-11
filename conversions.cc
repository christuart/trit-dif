#include "conversions.hh"

conversion::conversion(std::string _units_file):units_file_(_units_file) {
	initialise();
}

void conversion::initialise() {
	std::ifstream units_file_stream (units_file().c_str());
	std::string line;
	while (std::getline(units_file_stream, line)) {
		std::istringstream iss(line);
		std::string _dimension, _unit;
		double _value;
		
		if (!(iss >> _dimension >> _unit >> _value)) {
			std::cerr << "Invalid line, skipping.\n";
			continue;			
		}
		std::cout << "We obtained three values [" << _dimension << ", " << _unit << ", " << _value << "].\n";
	}
	units_file_stream.close();
}
