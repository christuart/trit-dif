#include "utilities.hh"

/*===========================================================================*/
void cSeparateValues(vector<string>& keys, string& text, string sep, string no){
int S0=0, S=0, i, ni; // values separated by 'sep' (exclusing '[no]sep')
 while ((i=text.find(sep,S))!=string::npos){
  if (!no.empty() && (ni=text.rfind(no,i))!=string::npos)
   if(i==no.length()+ni) goto skip; keys.push_back(text.substr(S0,i-S0));
  S0=i+sep.length(); skip:S=i+sep.length(); // guides.
 } keys.push_back(text.substr(S0,text.length()-S0));
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 cCommandLine                              */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
// debug of flags should be done by the user.
void cCommandLine::FClassify(string name){ // int pos;
 if (name.length()>0 && name[0]=='-'){ // flag(s)
  if (name.length()>1 && name[1]=='-') addFlag(name.substr(2));
  else  for (unsigned i=1; i<name.length(); ++i) addFlag(name.substr(i,1));
 } else addArg(name); // argument
}

/*===========================================================================*/
void cCommandLine::addArg(string name){ if (!name.empty()){
 FPList.push_back(clParameter(cltArgument,name)); ++ FnArgs;
} }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cCommandLine::addFlag(string name){ if (!name.empty()){
 FPList.push_back(clParameter(cltFlag,name)); ++ FnFlags;
} }

/*===========================================================================*/
cCommandLine::cCommandLine(string line):FnFlags(0),FnArgs(0){
vector<string> val; cSeparateValues(val,line," ","\\");
 for (unsigned i=0; i<val.size(); ++i) // multiple spaces.
  if (!val[i].empty()) FClassify(val[i]);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cCommandLine::cCommandLine(int nArg, char** vArg):FnFlags(0),FnArgs(0){
 for (unsigned i=0; i<nArg; ++i) FClassify(vArg[i]);
}

/*===========================================================================*/
int cCommandLine::arg(unsigned count, unsigned start){
 for (unsigned i=start, N=1; i<FPList.size(); ++i) // all.
  if (FPList[i].type==cltArgument){ if (N==count) return i; else ++N; }
 return -1; // not found;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int cCommandLine::flag(unsigned count, unsigned start){
 for (unsigned i=start, N=1; i<FPList.size(); ++i) // all.
  if (FPList[i].type==cltFlag){ if (N==count) return i; else ++N; }
 return -1; // not found;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int cCommandLine::flag(string name, unsigned start){
vector<string> mul; cSeparateValues(mul,name,"|");
 for (unsigned i=start; i<FPList.size(); ++i) // all.
  if (FPList[i].type==cltFlag && cContains(mul,FPList[i].arg)>=0)
   return i; return -1; // not found.
}

// /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
// /*                                 cPipeOut                                  */
// /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

// /*===========================================================================*/
// // Put 'cmd' into pipe.
// void cPipeOut::Execute(const string cmd){
//  fputs(cmd.c_str(),FPipe); fflush(FPipe);
// }

// /*===========================================================================*/
// // Constructor: open pipe to 'application' with 'parameters'.
// // Example 1: sPipeOut("gnuplot -persist");
// // Example 2: sPipeout("pawX11");
// cPipeOut::cPipeOut(const string commandline){
//  FPipe = popen(commandline.c_str(),"w");
//  if (FPipe==NULL) throw Exception("popen","cPipeOut::cPipeOut(const string)",
//   "Cannot open pipe for application ("+commandline+")");
// }

// /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// cPipeOut::~cPipeOut(){ // Destructor.
//  if (pclose(FPipe) == -1) throw Exception("pclose","cPipeOut::~cPipeOut()",
//   "Problem(s) closing pipe for application");
// }

// /*===========================================================================*/
// // Put 'Arg' control flag into pipe.
// cPipeOut &cPipeOut::operator<<(cFlag flag){
//  switch (flag){ // Use cFlag definitions.
//   case cTab:  Execute("\t"); break;
//   case cEndl: Execute("\n"); break;
//   default: throw Exception("cFlag","cPipeOut::operator<<(cFlag)","Unknown type");
// } return *this; }


// /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
// /*                                   cPipe                                   */
// /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

// /*===========================================================================*/
// void cPipe::OnChild(string &command, string &args){
//   if (dup2(FOUT[mWRITE],fileno(stdout))<0) throw Exception(
//    "Invalid Operation","cPipe::OnChild(string)","Fail to redirect stdout");
//   close(FOUT[mWRITE]); close(FOUT[mREAD]); // unused end of pipe...............
//   if (dup2(FIN[mREAD],fileno(stdin))<0) throw Exception(
//    "Invalid Operation","cPipe::OnChild(string)","Fail to redirect stdin");
//   close(FIN[mREAD]); close(FIN[mWRITE]); // unused end of pipe.................
//   if (dup2(FERR[mWRITE],fileno(stderr))<0) throw Exception(
//    "Invalid Operation","cPipe::OnChild(string)","Fail to redirect stderr");
//   close(FERR[mWRITE]); close(FERR[mREAD]); // unused end of pipe...............
//   fflush(stdout); fflush(stdin); fflush(stderr);
//   if (execlp(command.c_str(),args.c_str(),(char*)NULL)<0) // replace exe image.
//    throw Exception("Invalid Operation","cPipe::OnChild(string)",
//     "Fail to run command: "+command);
// }

// /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// void cPipe::OnParent(){ // close unused end of read, write, err pipes.
//  close(FOUT[mWRITE]); close(FIN[mREAD]); close(FERR[mWRITE]);
// }

// /*===========================================================================*/
// cPipe::cPipe(string command, string args){
//  if (pipe(FOUT)<0 || pipe(FIN)<0 || pipe(FERR)) throw Exception(
//   "Invalid Operation","cPipe::cPipe(string)","Fail to open pipes");
//  if ((FPID=fork())<0) throw Exception("Invalid Operation",
//   "cPipe::cPipe(string)","Fail to create a child - fork");
//  else if (FPID==0) OnChild(command,args); // child process.
//  else OnParent(); // parent process.
// }

// /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// cPipe &cPipe::operator>>(ostringstream &arg){ char bff;
//  while (read(FOUT[mREAD],&bff,1)>0) arg << bff; return *this;
// }

// /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// cPipe &cPipe::operator>>(string &arg){ ostringstream o;
//  this->operator>>(o); arg=o.str(); return *this;
// }

// /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// cPipe &cPipe::operator<<(cFlag arg){ // cEof for child.
//  switch (arg){ // Use cFlag definitions.
//   case cEof: close(FIN[mWRITE]); break;
//   case cTab:  this->operator<<("\t"); break;
//   case cEndl: this->operator<<("\n"); break;
//   default: throw Exception("cFlag","cPipe::operator<<(cFlag)","Unknown type");
// } return *this; }
