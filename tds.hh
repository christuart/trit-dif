#ifndef TDS_HH
#define TDS_HH

#define RUN_FILE_KEY_WIDTH 30

#include <typeinfo>

#include <list>
#include <iostream>
#include "utilities.hh"
#include "test.h"
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include <stdint.h>
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include <forward_list>
#include <bitset>
#include <algorithm>
#include <iomanip>

#include "exceptions.hh"
#include "conversions.hh"
#include "tds_parts.hh"
#include "vector_ops.hh"
#include "timing.hh"
#include "identifiers.hh"
#include "pluginfwd.hh"


class tds;
class tds_run;
class tds_display;
class tds_batch;

const int one_d = 0;
const int two_d = 1;
const int three_d = 2;
const int second_order_or_worse = 3;

class tds {
public:
private:
protected:
	tds_sections sections_;
	tds_materials materials_;
	tds_nodes nodes_;
	tds_elements elements_;
	conversion* units_;
	std::bitset<8> element_dimensions_;

	std::map<std::string,tds_material*> material_map_;
	
	std::string tds_name_,size_unit_,size_x_unit_,size_y_unit_,size_z_unit_;
	double size_,size_x_,size_y_,size_z_;
	int dimensions;

	MessageBuffer tds_log;

public:
	tds();
	virtual ~tds();
	
	//adders
	int add_section(tds_section* new_section);
	int add_material(tds_material* new_material);
	int add_node(tds_node* new_node);
	int add_element(tds_element* new_element);
	//cleaners
	void clean_sections();
	void clean_materials();
	void clean_nodes();
	void clean_elements();
	void clean_inactive_elements();
	
	//setters
	inline void tds_name(std::string tds_n) { tds_name_=tds_n; }
	inline void size_unit(std::string sz_unit){ size_unit_=sz_unit; }
	inline void size(double sz){ size_=sz; }
	inline void size_x_unit(std::string sz_x_unit){ size_x_unit_=sz_x_unit; }
	inline void size_x(double sz_x){ size_x_=sz_x; }
	inline void size_y_unit(std::string sz_y_unit){ size_y_unit_=sz_y_unit; }
	inline void size_y(double sz_y){ size_y_=sz_y; }
	inline void size_z_unit(std::string sz_z_unit){ size_z_unit_=sz_z_unit; }
	inline void size_z(double sz_z){ size_z_=sz_z; }
	inline void units(conversion* _units) { units_ = _units; }
	void expected_materials(int _n);
	void expected_sections(int _n);
	void expected_nodes(int _n);
	void expected_elements(int _n);
	//getters
	inline std::string tds_name() { return tds_name_; }
	inline std::string size_unit() { return size_unit_; }
	inline double size() { return size_; }
	inline double size_x() { return size_x_; }
	inline double size_y() { return size_y_; }
	inline double size_z() { return size_z_; }
	inline std::string size_x_unit() { return size_x_unit_; }
	inline std::string size_y_unit() { return size_y_unit_; }
	inline std::string size_z_unit() { return size_z_unit_; }
	inline conversion& units() { return *units_; }
	inline int n_sections() { return sections_.size(); }
	inline tds_section& section(int i) {
		if (i < n_sections()) { return *sections_[i]; }
		else {
			std::ostringstream oss; oss << "Reading section " << i << " out of " << n_sections() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline int n_materials() { return materials_.size(); }
	inline tds_material& material(int i) {
		if (i < n_materials()) { return *materials_[i]; }
		else {
			std::ostringstream oss; oss << "Reading material " << i << " out of " << n_materials() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline tds_material& material(std::string s) {
		std::map<std::string,tds_material*>::iterator _m = material_map_.find(s);
		if (_m != material_map_.end()) {
			return *(_m->second);
		} else {
			std::cerr << "Undefined material '" << s << "' replaced with 'error'" << std::endl;
			return *material_map_["error"];
		}
	}
	inline int n_nodes() { return nodes_.size(); }
	inline tds_node& node(int i) {
		if (i < n_nodes()) { return *nodes_[i]; }
		else {
			std::ostringstream oss; oss << "Reading node " << i << " out of " << n_nodes() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline int n_elements() { return elements_.size(); }
	inline tds_element& element(int i) {
		if (i < n_elements()) { return *elements_[i]; }
		else {
			std::ostringstream oss; oss << "Reading element " << i << " out of " << n_elements() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline bool element_dimensions(int bit_number) { return element_dimensions_.test(bit_number); }
	inline std::bitset<8> element_dimensions() { return element_dimensions_; }

	void register_element_type(int element_type);
	void output_model_summary(bool show_materials, bool show_sections, bool show_elements, bool show_element_links, bool show_nodes);
};

class tds_run: public tds {
public:	
private:
protected:
	std::ifstream materialsfile_;
	std::ifstream sectionsfile_;
	std::ifstream nodesfile_;
	std::ifstream elementsfile_;
	std::ifstream sourcefile_;
	std::ifstream simparamsfile_;
	std::ofstream trackingfile_;
	std::ofstream contaminationsfile_;
	std::string basename_;
	std::string configname_;
	std::string outputname_;

	// These strings are needed to be stored to work around problem of providing a c_str from
	// a temporary std::string - the temp string went out of scope and then sometimes the
	// c_str was available, sometimes it wasn't
	// std::string units_file_address_, materials_file_address_, sections_file_address_, nodes_file_address_, elements_file_address_, contaminations_file_address_, tracking_file_address_;

	MessageBuffer run_file_processing_output;
	MessageBuffer units_processing_output;
	MessageBuffer simulation_output;
	
	std::vector<IPlugin*> material_interrupts_;
	std::vector<IPlugin*> section_interrupts_;
	std::vector<IPlugin*> node_interrupts_;
	std::vector<IPlugin*> element_interrupts_;
	std::vector<IPlugin*> element_link_interrupts_;
	std::vector<IPlugin*> pre_simulation_interrupts_;
	std::vector<IPlugin*> step_start_interrupts_;
	std::vector<IPlugin*> step_end_interrupts_;
	std::vector<IPlugin*> post_simulation_interrupts_;
	bool units_set_;
	double initial_contamination;
	double delta_t_;
	int steps_;
	double tracking_interval_;
	std::vector<int>* tracked_elements_;
	struct plugin_file {
		std::string file_name;
		bool needed_after_initialisation;
	};
	struct run_settings {
		run_settings():model_directory("models/"),
		               config_directory("config/"),
		               output_directory("output/"),
		               delta_t(60.0),
		               simulation_length(31556736.0),
		               contamination_mode_time("constant"),
		               contamination_mode_space("constant"),
		               contamination(1),
		               tracking_mode("all"),
		               tracking_interval(604800.0) {
			activated_plugins.resize(0);
			plugin_files.clear();
			//tracking_list->resize(0);
		}
		std::string model_directory;
		std::string config_directory;
		std::string output_directory;
		std::string gmsh_bin_directory;
		std::string model_name;
		std::string config_name;
		std::string output_name;
		double delta_t;
		double simulation_length;
		std::string contamination_mode_time;
		std::string contamination_mode_space;
		double contamination;
		std::string contaminations_file;
		std::vector<std::string> activated_plugins;
		std::map<std::string, plugin_file> plugin_files;
		std::string tracking_mode;
		std::vector<int>* tracking_list;
		int tracking_n;
		double tracking_interval;
	} settings;
	
public:
	tds_run();
	virtual ~tds_run();
	
	void check_coincidence();
	void make_analysis();
	void clear_units();
	void set_units_from_file(const char* units_file_address_);
	void initialise();
	void direct_simulation_output_to_cout();
	void read_run_file(std::string run_file_name);
	std::string generate_run_file();
	void process_plugins();
	
	void add_material_interrupt(IPlugin* _interrupter);
	void add_section_interrupt(IPlugin* _interrupter);
	void add_node_interrupt(IPlugin* _interrupter);
	void add_element_interrupt(IPlugin* _interrupter);
	void add_element_link_interrupt(IPlugin* _interrupter);
	void add_pre_simulation_interrupt(IPlugin* _interrupter);
	void add_start_step_interrupt(IPlugin* _interrupter);
	void add_end_step_interrupt(IPlugin* _interrupter);
	void add_post_simulation_interrupt(IPlugin* _interrupter);

	// Methods for altering the pointers in each vector of tds_<part>s
	void change_section_pointer(int _i, tds_section* _new_section);
	void change_material_pointer(int _i, tds_material* _new_material);
	void change_node_pointer(int _i, tds_node* _new_node);
	void change_element_pointer(int _i, tds_element* _new_element);
	
	//setters
	inline void basename(std::string _basename) { settings.model_name = find_replace(settings.model_directory,"",_basename); basename_ = settings.model_directory + settings.model_name; };
	inline void configname(std::string _configname) { settings.config_name = std::string(find_replace(settings.config_directory,"",_configname)); configname_ = settings.config_directory + settings.config_name; };
	inline void outputname(std::string _outputname) { settings.output_name = find_replace(settings.output_directory,"",_outputname); outputname_ = settings.output_directory + settings.output_name; };
	inline void delta_t(double _delta_t) { settings.delta_t = _delta_t; }
	inline void steps(int _steps) { steps_ = _steps; }
	inline void tracking_interval(double _tracking_interval) { settings.tracking_interval = _tracking_interval; }
	inline void tracked_elements(std::vector<int>& _tracked_elements) { settings.tracking_list = &_tracked_elements; }
	inline void tracked_element(int i, int element) { settings.tracking_list->at(i) = element; } // not sure that this method needs to/should exist?
	//getters
	inline std::string basename() { return settings.model_name; };
	inline std::string configname() { return settings.config_name; };
	inline std::string outputname() { return settings.output_name; };
	inline double delta_t() { return settings.delta_t; }
	inline int steps() { return steps_; }
	inline double tracking_interval() { return settings.tracking_interval; }
	inline int tracked_element(int i) {
		if (i < tracked_elements()->size()) { return settings.tracking_list->at(i); }
		else {
			std::ostringstream oss; oss << "Reading tracked element " << i << " out of " << tracked_elements()->size() << ".";
			throw Errors::VectorOutOfBoundsException(oss.str());
		}
	}
	inline std::vector<int>* tracked_elements() { return settings.tracking_list; }
	
	inline std::string units_file_address() { std::string units_file_address_ = settings.config_directory + configname() + ".units"; return units_file_address_; }
	inline std::string materials_file_address() { std::string materials_file_address_ = settings.config_directory + configname() + ".materials"; return materials_file_address_; }
	inline std::string sections_file_address() { std::string sections_file_address_ = settings.model_directory + basename() + ".sections"; return sections_file_address_; }
	inline std::string nodes_file_address() { std::string nodes_file_address_ = settings.model_directory + basename() + ".nodes"; return nodes_file_address_; }
	inline std::string elements_file_address() { std::string elements_file_address_ = settings.model_directory + basename() + ".elements"; return elements_file_address_; }
	inline std::string contaminations_file_address() { std::string contaminations_file_address_ = settings.output_directory + outputname() + "-" + get_timestamp() + ".contaminations"; std::cerr << "*" << contaminations_file_address_ << "* "; return contaminations_file_address_; }
	inline std::string tracking_file_address() { std::string tracking_file_address_ = settings.output_directory + outputname() + "-" + get_timestamp() + ".tracking"; return tracking_file_address_; }

private:
	void interrupt_material(material_identifier& _new_material);
	void interrupt_section(section_identifier& _new_section);
	void interrupt_node(node_identifier& _new_node);
	void interrupt_element(element_identifier& _new_element);
	void interrupt_element_link(element_link_identifier& _new_element_link);
	void interrupt_pre_simulation();
	void interrupt_start_step(int _step, double _time);
	void interrupt_end_step(int _step, double _time);
	void interrupt_post_simulation();
};

class tds_display: public tds_run {
private:
	UserInterface *GUI_;
	
	Fl_Text_Buffer BRunFileName,
		BRunFileContents,
		BModelDirectory,
		BModelName,
		BSettingsDirectory,
		BSettingsName,
		BOutputDirectory,
		BOutputName,
		BDataDirty;
	
	// confusing naming because this is a Fl_Text_Buffer but an
	// IMessageListener, not an IMessageBuffer...
	gui_status_bar_messages BStatusBar;
	
	gui_console_messages gui_console;
	
	MessageBuffer gui_status;
	MessageBuffer gui_alerts;
	MessageBuffer gui_actions;
	
	std::vector<double> X_, Y_; //for the timelines
	std::vector<std::string> text_; 
	struct FilesSettingsMemento {
		std::string model_directory;
		std::string model_name;
		std::string config_directory;
		std::string config_name;
		std::string output_directory;
		std::string output_name;
		bool data_clean;
	} files_memento;
	
	bool data_is_clean_; // Let's the user know if the saved file matches what they see
	bool struct_matches_gui_; // Let's the software know if the tds_run matches what the user sees
	
	std::string e_info_,tl_info_;
	
	void open_run_file_dialog();
	void save_run_file();
	void preview_run_file();
	void revert_run_file();
	void change_files();
	void finish_changing_files();
	void load_event();
	void load_section(int chnum);
	void load_section(unsigned int c, int chnum);
	void resize_plot(int section);
	void makeZoomBox(selection sel,int event,int section);
protected:
	// GUI actions (view)
	// GUI actions (controller)
	void populate_from_run_file();
	void prettify_run_file();
	void generate_files_memento();
	void restore_files_memento();
	void update_gui_for_cleanliness();
	void populate_preview_browser(std::string source);
	
	// Setting Getters
	inline std::string run_file() { return std::string(BRunFileName.text()); }
	inline std::string backup_run_file() { return std::string(BRunFileName.text()) + ".bak"; }
	inline std::string model_directory() { return std::string(BModelDirectory.text()); }
	inline std::string model_name() { return std::string(BModelName.text()); }
	inline std::string config_directory() { return std::string(BSettingsDirectory.text()); }
	inline std::string config_name() { return std::string(BSettingsName.text()); }
	inline std::string output_directory() { return std::string(BOutputDirectory.text()); }
	inline std::string output_name() { return std::string(BOutputName.text()); }
	// Setting Setters
	inline void run_file(std::string _run_file) {
		BRunFileName.text(_run_file.c_str());
	}
	inline void model_directory(std::string _model_directory) {
		BModelDirectory.text(_model_directory.c_str());
	}
	inline void model_name(std::string _model_name) {
		BModelName.text(_model_name.c_str());
	}
	inline void config_directory(std::string _config_directory) {
		BSettingsDirectory.text(_config_directory.c_str());
	}
	inline void config_name(std::string _config_name) {
		BSettingsName.text(_config_name.c_str());
	}
	inline void output_directory(std::string _output_directory) {
		BOutputDirectory.text(_output_directory.c_str());
	}
	inline void output_name(std::string _output_name) {
		BOutputName.text(_output_name.c_str());
	}
	
	inline void mark_data_clean() { data_is_clean_ = true; update_gui_for_cleanliness(); }
	inline void mark_struct_up_to_date() { struct_matches_gui_ = true; }
	inline bool previous_settings_were_saved() { return data_is_clean_; }
	inline bool struct_is_up_to_date() { return struct_matches_gui_; }
public:
	tds_display(UserInterface *gui);
	virtual ~tds_display();
	inline UserInterface &GUI(){ return *GUI_; }
	void action(Fl_Widget *sender);
	void action(selection sel, Fl_Widget *sender);
	void plot();
	//Setters
	inline void display_e_info(std::string e_comment) { e_info_=e_comment; }
	inline void display_tl_info(std::string tl_comment) { tl_info_=tl_comment; }
	inline void mark_data_dirty() { data_is_clean_ = false; struct_matches_gui_ = false; update_gui_for_cleanliness(); }
	//Getters
	inline std::string display_e_info() { return e_info_; }
	inline std::string display_tl_info() { return tl_info_; }
}; 

class tds_batch: public tds_run {
private:
	std::ifstream infile_;
	const char* rootfile_name_;
protected:
public:
	//getters
	inline const char* rootfile_name() { return rootfile_name_; }
	//setters
	inline void rootfile_name(const char* _rootfile_name) { rootfile_name_=_rootfile_name; }
	tds_batch(std::string infilename, std::string rootout);
	virtual ~tds_batch();
	int run_batch(std::string filename, bool recreate, int filechain, int n_tot);
	inline std::ifstream &inputfile() { return infile_; }
};

#endif
