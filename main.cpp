#include <time.h>
#include <string>
#include "test.h"
#include <iostream>
#include <iomanip>
#include "exceptions.hh"
#include "tds.hh"

UserInterface* UI=NULL;
tds_display* tds=NULL;
tds_batch* tds_b=NULL;
tds_run* tds_t=NULL;
tds_run* tds_r=NULL;
void clear_pointers();

// Global Message Buffers
MessageBuffer exceptions = MessageBuffer(MBUnhandledException, "EXCEPTION   ");
MessageBuffer warnings = MessageBuffer(MBWarnings, "WARNING     ");
DebugMessageBuffer debugging = DebugMessageBuffer(MBDebug, "DEBUG       ");
// Global Message Listeners
standard_cout_listener console_out;
standard_cerr_listener console_err;
error_log_listener error_log;

/*===========================================================================*/
// GUI (FLUID) INTERFACE PROCEDURES.
void userAction(Fl_Widget* sender){ tds->action(sender); }
void userAction(selection sel, Ca_Canvas *sender){ tds->action(sel,sender); }
//selection as xmin,xmax,ymin,ymax
void mark_data_dirty(){ tds->mark_data_dirty(); }
/*===========================================================================*/

void show_preamble();
void show_usage();

int main(int nArg, char** vArg){
	try {
		show_preamble();

		int argsize = nArg;
		std::string textfile("config_file.conf"); // currently unused, could maybe set defaults in here?
		std::string prefix;
		cCommandLine cl(nArg,vArg);// decompose the command line.
	
		// std::cout << "Number of arguments: " << cl.size() << std::endl;
		// for (unsigned i=0; i < cl.size(); ++i) {
		// 	std::cout << "Argument " << i+1 << ": " << cl[i].arg << std::endl;
		// }

		console_out.error_listener(&console_err);
		console_err.out_listener(&console_out);
		
		debugging.add_listener(&console_out);
		exceptions.add_listener(&console_err);
		warnings.add_listener(&console_err);
		
		int t = cl.flag("t|test");
		int b = cl.flag("b|batch");
		int v = cl.flag("v|viewer");
		int version_flag = cl.flag("version");
		int run = cl.flag("run");

		if (version_flag >= 0) {
			return 0;
		} else if (run >= 0) {
			if (cl.size() != 3) {
				// output usage information
				show_usage();
				return -1;
			} else {
				try {
					tds_r = new tds_run();
					tds_r->read_run_file(cl[2].arg);
					tds_r->process_plugins();
					tds_r->initialise();
					tds_r->direct_simulation_output_to_cout();
					tds_r->make_analysis();
					clear_pointers();
					return 0;
				}
				catch (Errors::BadRunFileException& e) {
					std::cout << e.what() << std::endl;
				}
				catch (Errors::AnalysisException& e) {
					std::cout << e.what() << std::endl;
				}
				clear_pointers();
				return 0;
			}
		} else {
			if (v >= 0) {
				try {
					UI = new UserInterface(); UI->start_showing_window();
					tds = new tds_display(UI);
					int err=Fl::run();
					clear_pointers();
					return err;
				}
				catch (Errors::UIException& e) {
					std::cerr << e.what() << std::endl;
				}
				clear_pointers();
				return 0;
			} else if (t>=0) {
				// looks like we're testing functionality rather than using the display or batch processing
				std::string basename = "simple2d";
				std::string configname = "simple";
				std::string outputname = "output";
				float delta_t = 3600.0*24.0;
				float recording_interval = 3600*24*365.24;
				float finish_time;
				int steps = 10;
				std::vector<int> element_ids;
				std::cout << "Testing..." << std::endl;
				element_ids.push_back(1);
				element_ids.push_back(2);
				element_ids.push_back(3);
				element_ids.push_back(4);
				element_ids.push_back(5);
				bool basename_set = false, configname_set = false, outputname_set = false;
				for (unsigned i=0; i < cl.size(); ++i) {
					if (i < cl.size()-1) {
						if (cl[i].arg == 'm' && !basename_set) { // model
							basename = cl[++i].arg; basename_set = true;
						} else if (cl[i].arg == 'c' && !configname_set) { // config
							configname = cl[++i].arg; configname_set = true;
						} else if (cl[i].arg == 'o' && !outputname_set) { // output
							outputname = cl[++i].arg; outputname_set = true;
						} else if (cl[i].arg == 'd') { // delta_t
							std::istringstream iss(cl[++i].arg);
							iss >> delta_t;
						} else if (cl[i].arg == 's') { // steps
							std::istringstream iss(cl[++i].arg);
							iss >> steps;
						} else if (cl[i].arg == 'e') { // elements (to track)
							element_ids.resize(0);
							int this_id;
							std::istringstream iss(cl[++i].arg);
							std::cout << "Tracking ids: " << iss.str() << std::endl;
							while (iss >> this_id) {
								std::cout << "Tracking " << this_id << std::endl;
								element_ids.push_back(this_id);
							}
						} else if (cl[i].arg == 'r') { // recording interval
							std::istringstream iss(cl[++i].arg);
							iss >> recording_interval;
						} else if (cl[i].arg == 'f') { // finish time
							std::istringstream iss(cl[++i].arg);
							iss >> finish_time;
							steps = ceil(finish_time/delta_t);
						}		
					}
				}
				std::cout << "basename: " << basename << std::endl;
				std::cout << "configname: " << configname << std::endl;
				std::cout << "outputname: " << outputname << std::endl;
				tds_t = new tds_run();
				tds_t->basename(basename);
				tds_t->configname(configname);
				tds_t->outputname(outputname);
				tds_t->initialise();
				// run with five 1 day steps and no tracked elements for first tests.
				tds_t->delta_t(delta_t);
				tds_t->steps(steps);
				tds_t->tracking_interval(recording_interval);
				tds_t->tracked_elements(element_ids);
				tds_t->make_analysis();
				clear_pointers();
				return 0;
			} else if (b >= 0) {
				//std::string textfile(vArg[2]);
				//std::string testoutput = "test.txt";
				//TFile *f = new TFile("out.root","RECREATE","tds batch file output");
				//std::string rootout(vArg[3]);// = "output.root";
		
				////////THIS SHOULD BE SHIFTED TO MAKE 1 input = 1 output
				std::string rootfile(vArg[2]);
				//cout<<"take in "<<rootfile<<std::endl;
				//std::string append = "reduced.root";
				//std::string rootout(vArg[2]+append);
				int filechain = 0;
				int start_from = 0;
				std::string exroot;
				if(argsize>3) {
					const char* start_str = vArg[3];
					if(EOF==sscanf(start_str,"%d",&start_from)){}
					std::cout<<"start at file "<<start_from<<std::endl;
					filechain = start_from;
				}
				//std::cout<<"file chain starts from "<<filechain<<std::endl;
				int end_at = 10000;
				if(argsize>4) {
					const char* end_str = vArg[4];
					if(EOF==sscanf(end_str,"%d",&end_at)) {}
					std::cout<<"end at file "<<end_at<<std::endl;
				}
				if(argsize>5) {
					const char* pre_str = vArg[5];
					prefix = reinterpret_cast<const char*>(pre_str);
				}
				else { prefix = ""; }
				int tot_pulses = 0;
				if(argsize>6) {///to restart reduction use this parameter to ensure pulse_entries leaves are ok
					const char* pulses_entry = vArg[6];
					if(EOF==sscanf(pulses_entry,"%d",&tot_pulses)) {}
					std::cout<<"re-starting reduction at pulse "<<tot_pulses<<std::endl;////FIX SO AUTOMATIC!!!
				}
				bool chain_batch = true;
				//int slab_entries,pulse_entries = 0;
				int skip = -1;///skip for corrupt file
				while(chain_batch) {
					if(filechain==skip) {}
					else {
						//std::cout<<"in chain_batch "<<filechain<<std::endl;
						std::stringstream filenum;
						std::string fnum,fadd;
						if(filechain<1000){filenum<<0;}//places a 0 infront of 0-999
						if(filechain<100){filenum<<0;}//places a 0 infront of 0-99
						if(filechain<10){filenum<<0;}//places a 0 infront of 0-9
						filenum << filechain;
						fnum = filenum.str();
						std::string append = "_"+fnum+"_reduced.root";
						std::string rootout(vArg[2]+append);
						if(argsize>5) {
							rootout = prefix+"tds"+append;
						}
						std::cout<<"output reduced to "<<rootout<<std::endl;
						exroot = rootout;
						tds_b = new tds_batch(textfile,rootout);
						fadd = rootfile+fnum+".root";
						std::string root_file = fadd;
						//std::cout<<"rootfile = "<<root_file<<std::endl;
						TString r_file = fadd;
						TFile* r_f = new TFile(r_file);
						if(argsize>4 && filechain<end_at && r_f->IsZombie()) {std::cout<<"file missing but chain not finsihed, searching ahead..."<<std::endl;}
						else if(r_f->IsZombie()) {std::cout<<"no more files to reduce"<<std::endl;chain_batch=false;}
						else {
							std::cout<<"batch file running on: "<<root_file<<std::endl;
							//tot_pulses+=tds_b->run_batch(root_file,true);
							tot_pulses+=tds_b->run_batch(root_file,true,filechain,tot_pulses);
							//if(filechain==0){tot_pulses+=tds_b->run_batch(root_file,true);}//,tot_pulses);}
							//else{tot_pulses+=tds_b->run_batch(root_file,false);}//create file with 1st one, append after that
							r_f->Close();
						}
						//if (filechain==3){chain_batch=false;}
						//chain_batch = false;
					}
					filechain++;
					if(filechain>end_at){break;}
				}
				std::cout<<"total root files reduced = "<<filechain-1<<", last file was "<<exroot<<std::endl;
				std::cout<<"number of pulses found = "<<tot_pulses<<std::endl;
				//std::cout<<"start at file "<<start_from<<" up to "<<end_at<<std::endl;
				return 0;
			} else {
				show_usage();
			}
		}
	}
	catch (std::exception& e){
		LOG(exceptions,"(unhandled exception) " << e.what());
	}
	clear_pointers();
};

void clear_pointers() {
	delete tds; tds=NULL;
	delete tds_r; tds_r=NULL;
	delete tds_t; tds_t=NULL;
	delete tds_b; tds_b=NULL;
	delete UI; UI=NULL;
	std::cout << std::endl << std::endl; // ensures that next console line displays cleanly
}

void show_preamble() {
	
	std::cout << std::endl;
	std::cout << "***" << std::endl;
	std::cout << "************************************************************" << std::endl;
	std::cout << "***" << std::endl;
	std::cout << "***  trit-dif program written by Chris Stuart" << std::endl;
	std::cout << "***  Supervised by Anthony Hollingsworth at CCFE" << std::endl;
	std::cout << "***" << std::endl;
	std::cout << "***  This version: " << VERSION << std::endl;
	std::cout << "***" << std::endl;
	std::cout << "***  Find out more on the GitHub repository at " << std::endl;
	std::cout << "***  http://github.com/christuart/trit-dif/" << std::endl;
	std::cout << "***" << std::endl;
	std::cout << "************************************************************" << std::endl;
	std::cout << "***" << std::endl;
	std::cout << std::endl;

}
void show_usage() {

	std::cout << std::endl;
	std::cout << "Usage: trit-dif [[--run <instruction file> ] | [-v|--viewer] | [-h|--help ]]" << std::endl;
	std::cout << std::endl << std::left;
	std::cout << std::setw(4) << "" << std::setw(12) << "--run" << "Provide the program with a set of instructions in a '.run' file" << std::endl;
	std::cout << std::setw(4) << "-v" << std::setw(12) <<  "--viewer" << "Open the graphical user interface" << std::endl;
	std::cout << std::setw(4) << "-h" << std::setw(12) <<  "--help" << "Show this usage information" << std::endl;
	std::cout << "Alternative, older ways of using the program may be found in the documentation or the source." << std::endl;
	std::cout << std::endl;

}
