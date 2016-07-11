#ifndef CONVERSION_HH
#define CONVERSION_HH

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

typedef enum {
    DIM_LENGTH,
    DIM_TIME, // cannot reuse name time
    DIM_DENSITY,
    DIM_DIFFUSION_CONSTANT,
    DIM_VOLUME,
    DIM_AREA
} enum_dimensions;

class conversion {
	public:
	private:
	std::string units_file_;
	std::map<std::string,float> length_units_;
	std::map<std::string,float> time_units_;
	std::map<std::string,float> density_units_;
	std::map<std::string,float> diffusion_constant_units_;
	std::map<std::string,float> volume_units_;
	std::map<std::string,float> area_units_;
	std::map<std::string,int> dimensions_;
	protected:
	public:
	conversion(std::string _units_file);
	//setters
	inline void units_file(std::string _units_file) { units_file_ = _units_file; }
	//getters
	inline std::string units_file() { return units_file_; }
	float convert_length_from(std::string _unit, float _length);
	float convert_length_to(std::string _unit, float _length);
	float convert_time_from(std::string _unit, float _time);
	float convert_time_to(std::string _unit, float _time);
	float convert_density_from(std::string _unit, float _density);
	float convert_density_to(std::string _unit, float _density);
	float convert_diffusion_constant_from(std::string _unit, float _diffusion_constant);
	float convert_diffusion_constant_to(std::string _unit, float _diffusion_constant);
	float convert_volume_from(std::string _unit, float _volume);
	float convert_volume_to(std::string _unit, float _volume);
	float convert_area_from(std::string _unit, float _area);
	float convert_area_to(std::string _unit, float _area);
	void initialise();

};

#endif
