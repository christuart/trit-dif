#include "conversions.hh"

conversion::conversion(std::string _units_file):units_file_(_units_file) {
	dimensions_["length"] = DIM_LENGTH;
	dimensions_["time"] = DIM_TIME;
	dimensions_["density"] = DIM_DENSITY;
	dimensions_["diffusion_constant"] = DIM_DIFFUSION_CONSTANT;
	dimensions_["volume"] = DIM_VOLUME;
	dimensions_["area"] = DIM_AREA;
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
		if (dimensions_.count(_dimension) == 0) {
			std::cerr << "Invalid dimension was read from unit file (" << _dimension << ") for the unit '" << _unit << "'" << std::endl;
			continue;
		}
		std::cout << "We obtained three values [" << _dimension << ", " << _unit << ", " << _value << "].\n";
		
		int dim_code = dimensions_[_dimension];
		switch (dim_code) {
		case DIM_DENSITY:
			density_units_[_unit] = _value;
			break;
		case DIM_DIFFUSION_CONSTANT:
			diffusion_constant_units_[_unit] = _value;
			break;
		default:
			std::cerr << "Unhandled dimension was read from unit file (" << _dimension << ") for the unit '" << _unit << "'" << std::endl;
		}
		
	}
	units_file_stream.close();
}

float conversion::convert_density_from(std::string _unit, float _density) {
	if (density_units_.count(_unit) == 0) {
		std::cerr << "No density unit '" << _unit << "' was found." << std::endl;
		return 0.0;
	}
	return _density * density_units_[_unit];
}
float conversion::convert_density_to(std::string _unit, float _density) {
	if (density_units_.count(_unit) == 0) {
		std::cerr << "No density unit '" << _unit << "' was found." << std::endl;
		return 0.0;
	}
	return _density / density_units_[_unit];
}
float conversion::convert_diffusion_constant_from(std::string _unit, float _diffusion_constant) {
	if (diffusion_constant_units_.count(_unit) == 0) {
		std::cerr << "No diffusion constant unit '" << _unit << "' was found." << std::endl;
		return 0.0;
	}
	return _diffusion_constant * diffusion_constant_units_[_unit];
}
float conversion::convert_diffusion_constant_to(std::string _unit, float _diffusion_constant) {
	if (diffusion_constant_units_.count(_unit) == 0) {
		std::cerr << "No diffusion_constant unit '" << _unit << "' was found." << std::endl;
		return 0.0;
	}
	return _diffusion_constant / diffusion_constant_units_[_unit];
}
float conversion::convert_time_from(std::string _unit, float _time) {
	if (time_units_.count(_unit) == 0) {
		std::cerr << "No time unit '" << _unit << "' was found." << std::endl;
		return 0.0;
	}
	return _time * time_units_[_unit];
}
float conversion::convert_time_to(std::string _unit, float _time) {
	if (time_units_.count(_unit) == 0) {
		std::cerr << "No time unit '" << _unit << "' was found." << std::endl;
		return 0.0;
	}
	return _time / time_units_[_unit];
}

