#ifndef CONVERSION_HH
#define CONVERSION_HH

#define LOG_TIME_ACCURACY 0.1f

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>

typedef enum {
    DIM_LENGTH,
    DIM_TIME, // cannot reuse name time
    DIM_DENSITY,
    DIM_DIFFUSION_CONSTANT,
    DIM_AREA,
    DIM_VOLUME,
    DIM_CONTAMINATION
} enum_dimensions;

class conversion {
	public:
	private:
	std::string units_file_;
	std::map<std::string,double> length_units_;
	std::map<std::string,double> time_units_;
	std::map<std::string,double> density_units_;
	std::map<std::string,double> diffusion_constant_units_;
	std::map<std::string,double> area_units_;
	std::map<std::string,double> volume_units_;
	std::map<std::string,double> contamination_units_;
	std::map<std::string,int> dimensions_;
	protected:
	public:
	conversion(std::string _units_file);
	//setters
	inline void units_file(std::string _units_file) { units_file_ = _units_file; }
	//getters
	inline std::string units_file() { return units_file_; }
	
	double convert_length_from(std::string _unit, double _length);
	double convert_length_to(std::string _unit, double _length);
	double convert_time_from(std::string _unit, double _time);
	double convert_time_to(std::string _unit, double _time);
	double convert_density_from(std::string _unit, double _density);
	double convert_density_to(std::string _unit, double _density);
	double convert_diffusion_constant_from(std::string _unit, double _diffusion_constant);
	double convert_diffusion_constant_to(std::string _unit, double _diffusion_constant);
	double convert_area_from(std::string _unit, double _area);
	double convert_area_to(std::string _unit, double _area);
	double convert_volume_from(std::string _unit, double _volume);
	double convert_volume_to(std::string _unit, double _volume);
	double convert_contamination_from(std::string _unit, double _contamination);
	double convert_contamination_to(std::string _unit, double _contamination);
	std::string generate_appropriate_time_input_string(double _time);
	std::string generate_appropriate_contamination_input_string(double _contamination);
	void initialise();

};

#endif
