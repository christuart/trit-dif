#include <time.h>
#include <string>
#include "gui.h"
#include <iostream>
#include "tds.hh"

using namespace std;

UserInterface* UI=NULL;
tds_display* tds=NULL;
tds_batch* tds_b=NULL;

/*===========================================================================*/
// GUI (FLUID) INTERFACE PROCEDURES.
void userAction(Fl_Widget* sender){ tds->action(sender); }
void userAction(selection sel, Ca_Canvas *sender){ tds->action(sel,sender); }
//selection as xmin,xmax,ymin,ymax
/*===========================================================================*/

int main(int nArg, char** vArg){
	int argsize = nArg;
	string textfile("config_file.conf");
	string prefix;
	cCommandLine cl(nArg,vArg);// decompose the command line.
	int t=cl.flag("t|test");
	int b=cl.flag("b|batch");
	if (t>=0) {
		// looks like we're testing functionality rather than using the display or batch processing
		string basename = "simple";
		
	} else if (b >= 0) {
		//string textfile(vArg[2]);
		//string testoutput = "test.txt";
		//TFile *f = new TFile("out.root","RECREATE","tds batch file output");
		//string rootout(vArg[3]);// = "output.root";
		
		////////THIS SHOULD BE SHIFTED TO MAKE 1 input = 1 output
		string rootfile(vArg[2]);
		//cout<<"take in "<<rootfile<<endl;
		//string append = "reduced.root";
		//string rootout(vArg[2]+append);
		int filechain = 0;
		int start_from = 0;
		std::string exroot;
		if(argsize>3) {
			const char* start_str = vArg[3];
			if(EOF==sscanf(start_str,"%d",&start_from)){}
			cout<<"start at file "<<start_from<<endl;
			filechain = start_from;
		}
		//cout<<"file chain starts from "<<filechain<<endl;
		int end_at = 10000;
		if(argsize>4) {
			const char* end_str = vArg[4];
			if(EOF==sscanf(end_str,"%d",&end_at)) {}
			cout<<"end at file "<<end_at<<endl;
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
			cout<<"re-starting reduction at pulse "<<tot_pulses<<endl;////FIX SO AUTOMATIC!!!
		}
		bool chain_batch = true;
		//int slab_entries,pulse_entries = 0;
		int skip = -1;///skip for corrupt file
		while(chain_batch) {
			if(filechain==skip) {}
			else {
				//cout<<"in chain_batch "<<filechain<<endl;
				std::stringstream filenum;
				std::string fnum,fadd;
				if(filechain<1000){filenum<<0;}//places a 0 infront of 0-999
				if(filechain<100){filenum<<0;}//places a 0 infront of 0-99
				if(filechain<10){filenum<<0;}//places a 0 infront of 0-9
				filenum << filechain;
				fnum = filenum.str();
				string append = "_"+fnum+"_reduced.root";
				string rootout(vArg[2]+append);
				if(argsize>5) {
					rootout = prefix+"tds"+append;
				}
				cout<<"output reduced to "<<rootout<<endl;
				exroot = rootout;
				tds_b = new tds_batch(textfile,rootout);
				fadd = rootfile+fnum+".root";
				std::string root_file = fadd;
				//cout<<"rootfile = "<<root_file<<endl;
				TString r_file = fadd;
				TFile* r_f = new TFile(r_file);
				if(argsize>4 && filechain<end_at && r_f->IsZombie()) {cout<<"file missing but chain not finsihed, searching ahead..."<<endl;}
				else if(r_f->IsZombie()) {cout<<"no more files to reduce"<<endl;chain_batch=false;}
				else {
					cout<<"batch file running on: "<<root_file<<endl;
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
		cout<<"total root files reduced = "<<filechain-1<<", last file was "<<exroot<<endl;
		cout<<"number of pulses found = "<<tot_pulses<<endl;
		//cout<<"start at file "<<start_from<<" up to "<<end_at<<endl;
		return 0;
	}
	else{
		UI = new UserInterface(); UI->show();
		tds = new tds_display(UI);
		int err=Fl::run();
		return err;
	}
	delete tds;
	delete tds_b;
};

