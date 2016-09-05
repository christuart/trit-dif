#include "conversions.hh"

conversion::conversion(std::string _units_file, IMessageBuffer* _output):units_file_(_units_file),output_buffer_(_output) {
	dimensions_["length"] = DIM_LENGTH;
	dimensions_["time"] = DIM_TIME;
	dimensions_["density"] = DIM_DENSITY;
	dimensions_["diffusion_constant"] = DIM_DIFFUSION_CONSTANT;
	dimensions_["area"] = DIM_AREA;
	dimensions_["volume"] = DIM_VOLUME;
	dimensions_["contamination"] = DIM_CONTAMINATION;
	initialise();
}
conversion::~conversion() {}

void conversion::initialise() {
	std::ifstream units_file_stream (units_file().c_str());
	std::string line;
	length_units_ = std::map<std::string,double>();
	time_units_ = std::map<std::string,double>();
	density_units_ = std::map<std::string,double>();
	diffusion_constant_units_ = std::map<std::string,double>();
	volume_units_ = std::map<std::string,double>();
	area_units_ = std::map<std::string,double>();
	while (std::getline(units_file_stream, line)) {
		std::istringstream iss(line);
		std::string _dimension, _unit;
		double _value;
		
		if (!(iss >> _dimension >> _unit >> _value)) {
			LOG((*output_buffer_),"Invalid line, skipping.");
			continue;			
		}
		if (dimensions_.count(_dimension) == 0) {
			throw Errors::BadInputDataException("Invalid dimension '" + _dimension + "' used in the units file (at unit '" + _unit + "').");
		}
		LOG((*output_buffer_),"Processing new unit: [" << _dimension << ", " << _unit << ", " << _value << "].");
		
		int dim_code = dimensions_[_dimension];
		switch (dim_code) {
		case DIM_DENSITY:
			density_units_[_unit] = _value;
			break;
		case DIM_DIFFUSION_CONSTANT:
			diffusion_constant_units_[_unit] = _value;
			break;
		case DIM_TIME:
			time_units_[_unit] = _value;
			break;
		case DIM_LENGTH:
			length_units_[_unit] = _value;
			break;
		case DIM_AREA:
			area_units_[_unit] = _value;
			break;
		case DIM_VOLUME:
			volume_units_[_unit] = _value;
			break;
		case DIM_CONTAMINATION:
			contamination_units_[_unit] = _value;
			break;
		default:
			throw Errors::AlgorithmFailedException("Unhandled dimension '" + _dimension + "' processed from the units file (at unit '" + _unit + "').");
		}
		
	}
	units_file_stream.close();
	
}

double conversion::convert_density_from(std::string _unit, double _density) {
	if (density_units_.count(_unit) == 0) {
		throw Errors::ConvertingMissingUnitException("Requested density unit '" + _unit + "' was not found.");
	}
	return _density * density_units_[_unit];
}
double conversion::convert_density_to(std::string _unit, double _density) {
	if (density_units_.count(_unit) == 0) {
		throw Errors::ConvertingMissingUnitException("Requested density unit '" + _unit + "' was not found.");
	}
	return _density / density_units_[_unit];
}
double conversion::convert_diffusion_constant_from(std::string _unit, double _diffusion_constant) {
	if (diffusion_constant_units_.count(_unit) == 0) {
		throw Errors::ConvertingMissingUnitException("Requested diffusion constant unit '" + _unit + "' was not found.");
	}
	return _diffusion_constant * diffusion_constant_units_[_unit];
}
double conversion::convert_diffusion_constant_to(std::string _unit, double _diffusion_constant) {
	if (diffusion_constant_units_.count(_unit) == 0) {
		throw Errors::ConvertingMissingUnitException("Requested diffusion constant unit '" + _unit + "' was not found.");
	}
	return _diffusion_constant / diffusion_constant_units_[_unit];
}
double conversion::convert_time_from(std::string _unit, double _time) {
	if (time_units_.count(_unit) == 0) {
		throw Errors::ConvertingMissingUnitException("Requested time unit '" + _unit + "' was not found.");
	}
	return _time * time_units_[_unit];
}
double conversion::convert_time_to(std::string _unit, double _time) {
	if (time_units_.count(_unit) == 0) {
		throw Errors::ConvertingMissingUnitException("Requested time unit '" + _unit + "' was not found.");
	}
	return _time / time_units_[_unit];
}
double conversion::convert_contamination_from(std::string _unit, double _contamination) {
	if (contamination_units_.count(_unit) == 0) {
		throw Errors::ConvertingMissingUnitException("Requested contamination unit '" + _unit + "' was not found.");
	}
	return _contamination * contamination_units_[_unit];
}
double conversion::convert_contamination_to(std::string _unit, double _contamination) {
	if (contamination_units_.count(_unit) == 0) {
		throw Errors::ConvertingMissingUnitException("Requested contamination unit '" + _unit + "' was not found.");
	}
	return _contamination / contamination_units_[_unit];
}
std::string conversion::generate_appropriate_time_input_string(double _time) {
	std::ostringstream oss;
	double ltime = log10(_time);
	typedef std::map<std::string,double>::iterator it_type;
	it_type best_choice = time_units_.end();
	for (it_type it = time_units_.begin(); it != time_units_.end(); ++it) {
		if (log10(it->second) - ltime < LOG_TIME_ACCURACY) {
			if (best_choice == time_units_.end() || log10(it->second) - log10(best_choice->second) > LOG_TIME_ACCURACY) {
				best_choice = it;
			}
		}
	}
	if (best_choice != time_units_.end()) {
		oss << (_time / best_choice->second) << " " << best_choice->first;
	} else {
		oss << _time << " seconds";
	}
	return oss.str();
}
std::string conversion::generate_appropriate_contamination_input_string(double _contamination) {
	std::ostringstream oss;
	oss << _contamination << " au";
	return oss.str();
}
