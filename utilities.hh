#ifndef UTILITIES_HH
#define UTILITIES_HH

//#include "exception.h"
//#include "mathematic.h"
#include <string>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <pwd.h>
#include <pthread.h>

using namespace std;

class cPipeOut;
class cPipe;
class cCommandLine;


/*===========================================================================*/
void cSeparateValues(vector<string>&, string& , string, string no="");

/*===========================================================================*/
template <class Type> int cContains(vector<Type>& tgt, Type arg){
 for (unsigned i=0; i<tgt.size(); ++i) if (tgt[i]==arg) return i; return -1;
}

//===========================================================================*/
  //Writes to 'std::string' through 'std::ostringstream'.
  //Use 'std::stringstream' instead of 'std::string' to read more than one field.
template <class inType> string &operator<<(string &str, inType Data){
ostringstream out; out << Data; return str+=out.str();
}

/*===========================================================================*/
// Writes to 'std::string' through 'std::ostringstream'.
template <class Type> string cString(Type Data){
ostringstream out; out << Data; return out.str();
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 cCommandLine                              */
/*          Francisco Neves @ 2008.03.13 ( Last modified 2008.03.14)         */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
class cCommandLine {
public: enum clType { cltFlag=0, cltArgument=1 };
public: struct clParameter { clType type; string arg;
    clParameter(clType t, string a):type(t),arg(a){ } };
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    unsigned FnFlags, FnArgs;
    vector<clParameter> FPList;
    void FClassify(string name);
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    void addArg(string name);
    void addFlag(string name);
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cCommandLine(string line);
    cCommandLine(int nArg, char** vArg);
    inline unsigned size(){ return FPList.size(); }
    inline clParameter& operator[](unsigned i){ return FPList[i]; }
    inline unsigned nArgs(){ return FnArgs; }
    int arg(unsigned count, unsigned start=0);
    inline unsigned nFlags(){ return FnFlags; }
    int flag(unsigned count, unsigned start=0);
    int flag(string name, unsigned start=0);
};

// /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
// /*                                 cPipeOut                                  */
// /*          Francisco Neves @ 2006.02.07 ( Last modified 2006.02.08 )        */
// /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
// class cPipeOut{ // Output pipe for a file/application.
// private:
// 	FILE *FPipe;
// protected:
// 	void Execute(const string cmd);
// public:
// 	cPipeOut(const string commandline);
// 	~cPipeOut();
// 	cPipeOut &operator<<(cFlag flag);
// 	template <class argType> cPipeOut &operator<<(argType Arg);
// };

// /*===========================================================================*/
// // Put 'Arg' into pipe.
// template <class argType> cPipeOut &cPipeOut::operator<<(argType Arg){
// ostringstream out; out << Arg;  Execute(out.str());
//  return *this;
// }

// /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
// /*                                   cPipe                                   */
// /*          Francisco Neves @ 2008.02.12 ( Last modified 2008.02.20 )        */
// /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
// class cPipe{ // // Output/Input pipe for a file/application.
// public: enum cMode {mREAD=0, mWRITE=1};
// private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// 	int FOUT[2], FIN[2], FERR[2], FPID;
// protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//     void OnChild(string &comand, string &args);
//     void OnParent();
// public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//     cPipe(string command, string args="");
//     ~cPipe(){ close(FIN[mWRITE]); close(FOUT[mREAD]); close(FERR[mREAD]); }
//     void waitForChild(){ int status; int s; waitpid(FPID,&status,0); }
// 	template <class argType> cPipe &operator<<(argType arg);
//     cPipe &operator>>(ostringstream &arg);
//     cPipe &operator>>(string &arg);
// 	cPipe &operator<<(cFlag arg);
// };

// /*===========================================================================*/
// template <class argType> cPipe &cPipe::operator<<(argType arg){
// ostringstream bff; bff << arg; string out(bff.str());
//  write(FIN[mWRITE],out.c_str(),out.size());
//  return *this;
// }

#endif
