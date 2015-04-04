/**
 * @file rat.cc
 * @author Stan Seibert <volsung@physics.utexas.edu>
 */

//#define HAS_G4DAE

#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <G4UImanager.hh>
#include <G4UIterminal.hh>
#include <G4UItcsh.hh>
#include <Randomize.hh>
#include <globals.hh>
#include <G4VisExecutive.hh>
#include <G4UIExecutive.hh>
#ifdef HAS_G4DAE
#include <G4DAEParser.hh> // provides option to dump geometry to DAE/Collada file
#endif

// time.h must come after Randomize.hh on RedHat 7.3 
#include <time.h>

#include <RAT/Config.hh>
#include <RAT/Log.hh>
#include <RAT/RunManager.hh>
#include <RAT/InROOTProducer.hh>
#include <RAT/InNetProducer.hh>
#include <RAT/ProcBlock.hh>
#include <RAT/ProcBlockManager.hh>
#include <RAT/PythonProc.hh>
#include <RAT/SignalHandler.hh>
#include <RAT/DB.hh>
#include <RAT/DBMessenger.hh>
#include <RAT/TrackingMessenger.hh>
#include <RAT/OutROOTProc.hh>
#include <RAT/getopt.h>
#include <TStopwatch.h>
#include <TRandom.h>

#include <iostream>
#include <string>

using namespace std;
using namespace RAT;

class CmdOptions {
public:
  CmdOptions() : seed(-1), save_macro(true), input_filename(""),
                 output_filename(""), run(0) {}
  long seed;
  bool save_macro;
  std::string input_filename;
  std::string output_filename;
  std::vector<std::string> python_processors;
  int run;
};


int OutROOTProc::run_num = -1;
CmdOptions parse_command_line(int argc, char** argv);
void help();
std::string get_short_hostname();
std::string get_long_hostname();

int main(int argc, char** argv) {
    TStopwatch runTime;
    runTime.Start();
    CmdOptions options;

    options = parse_command_line(argc, argv);

    info << "RAT, version " << RATVERSION << newline;

    info << "Status messages enabled: info ";
    detail << "detail ";
    debug << "debug ";
    info << "\n";

    pid_t pid = getpid();
    info << "Hostname: " << get_long_hostname() << " PID: " << pid << newline;

    // Seed random number generator using current time and process id
    // (Use both just in case multiple processes started at same time)
    time_t start_time = time(NULL);
    if (options.seed == -1) // No override by command switch
      options.seed = start_time ^ (pid << 16);
    detail << "Seeding random number generator: " << options.seed << newline;
    CLHEP::HepRandom::setTheSeed(options.seed);
    gRandom->SetSeed(options.seed);

    // initialize RATDB and load all .ratdb files in $GLG4DATA,
    // usually set to $RATROOT/data
    DB *rdb = DB::Get();
    if ( getenv("GLG4DATA") != NULL )
      rdb->LoadAll(string(getenv("GLG4DATA")));
    else
      rdb->LoadAll(string("data"));
    DBMessenger *rdb_messenger = new DBMessenger();

    if (options.run > 0) {
      info << "Setting run number: " << options.run << newline;
      rdb->SetDefaultRun(options.run);
    }

    try { // Catch database errors
      // Set default input and output files
      if (options.input_filename != "") {
	rdb->SetS("IO", "", "default_input_filename", options.input_filename);
	info << "Setting default input file to " << options.input_filename << "\n";
      }
      if (options.output_filename != "") {
	rdb->SetS("IO", "", "default_output_filename", options.output_filename);
	info << "Setting default output file to " << options.output_filename << "\n";
      }

    // Main analysis block -- will contain user-constructed analysis sequence
    ProcBlock *mainBlock = new ProcBlock;
    // Process block manager -- Supplies user commands to construct analysis
    // sequence and actually does the processor creation
    ProcBlockManager *blockManager = new ProcBlockManager(mainBlock);
    TrackingMessenger *trackingMessenger = new TrackingMessenger();
  
    // Build event producers
    RunManager* runManager = new RunManager(mainBlock);
    InROOTProducer *inroot = new InROOTProducer(mainBlock);
    InNetProducer *innet = new InNetProducer(mainBlock);
    // RATFsim *fsim = new RATFsim(analysisStack); // SOMEDAY!

    // Setup signal handler to intercept Ctrl-C and quit event loop
    // nicely (closing files and all that).
    SignalHandler::Init();

    // Initialize the user interface
    G4UImanager* theUI = G4UImanager::GetUIpointer();

#ifdef HAS_G4DAE
    //Start up DAE exporter
    G4DAEParser dae_parser;
#endif

    // Add any python processors specified on the command line
    for (unsigned i=0; i < options.python_processors.size(); i++) {
      Processor *p = new PythonProc;
      p->SetS("class", options.python_processors[i]);
      mainBlock->DeferAppend(p);
    }

    // interactive or batch according to command-line args
    if (RAT::optind - argc == 0) {
      // Interactive mode

      // G4UIterminal is a (dumb) terminal.
      // ..but it can be made smart by adding a "shell" to it
      G4UIExecutive* theSession = new G4UIExecutive(argc,argv);
      theUI -> ApplyCommand("/control/execute prerun.mac");
      theSession -> SessionStart();
      delete theSession;
    }
    else {
      // Batch mode, with optional user interaction

      G4String command = "/control/execute ";
      for (int iarg=RAT::optind; iarg<argc; iarg++) {
	// process list of macro files; "-" means interactive user session
	G4String fileName = argv[iarg];
	if ( fileName == "-" ) {
	  // interactive session requested
	  G4UIExecutive* theSession = new G4UIExecutive(argc,argv);
	  theSession -> SessionStart();
	  delete theSession;
	}
        else {
	  if (options.save_macro) {
	    // Read file contents and log them
	    ifstream macro(fileName);
	    string macroline;
	    while (macro.good()) {
	      getline(macro, macroline);
	      Log::AddMacro(macroline+"\n");
	    }
	  }

	  // execute given file
	  theUI -> ApplyCommand(command+fileName);
	}
      }
    }

    // User exit or macros finished, clean up

    // Hack to close GLG4sim output file if used
    theUI->ApplyCommand("/event/output_file");

    delete blockManager;

    // Report on CPU usage early to ensure it is captured by the log
    // before any ROOT files are closed
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    float usertime = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
    float systime = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;

    info << dformat("Run time:  %8.2f sec\n", runTime.RealTime());
    info << dformat("CPU usage: user %8.2f sec\tsys %6.2f sec\n",
		    usertime, systime);

    delete mainBlock; // implicitly deletes all processor instances the user
                      // built, and closes up associated files

    delete runManager;
    delete inroot;
    delete innet;

    delete rdb_messenger;
    delete trackingMessenger;

    }
    catch (DBNotFoundError &e) {
      Log::Die("DB: Field " + e.table + "[" + e.index + "]." + e.field
		  + " lookup failure.  Does not exist or has wrong type.");
    }

    return 0;
}

#define OPT_NO_SAVE_MACRO 1000

CmdOptions parse_command_line(int argc, char** argv) {
  static struct option opts[] = { {"quiet", 0, NULL, 'q'},
                                  {"help", 0, NULL, 'h'},
                                  {"verbose", 0, NULL, 'v'},
                                  {"debug", 0, NULL, 'd'},
                                  {"version", 0, NULL, 'V'},
                                  {"logfile", 1, NULL, 'l'},
                                  {"seed", 1, NULL, 's'},
                                  {"database", 1, NULL, 'b'},
                                  {"run", 1, NULL, 'r'},
                                  {"input", 1, NULL, 'i'},
                                  {"output", 1, NULL, 'o'},
                                  {"python", 1, NULL, 'p'},
                                  {"no-save-macro", 0, NULL, OPT_NO_SAVE_MACRO},
                                  {0, 0, 0, 0} };
  std::string short_options("qhdvVl:n:r:s:b:o:i:p:");

  CmdOptions options;
  int display_level = Log::INFO;
  bool debug_log = false;
  std::string logfilename = (std::string("rat.") + get_short_hostname() +
                             "." + ::to_string(getpid()) + ".log");

  int option_index = 0;
  int c = getopt_long(argc, argv, short_options.c_str(), opts, &option_index);
  int run;
  while (c != -1) {
    switch (c) {
    case 'q': 
      if (display_level > Log::WARN)
	display_level--;
      break;
    case 'v':
      if (display_level < Log::DEBUG)
	display_level++;
      break;
    case 'h': help(); exit(0); break;
    case 'V':
      cout << "RAT " << RATVERSION << std::endl;
      exit(0);
      break;
    case 'd': debug_log = true; break;
    case 'l': logfilename = RAT::optarg; break;
    case 'n': OutROOTProc::run_num = atoi(RAT::optarg); break;
    case 'r':
      run = strtol(RAT::optarg, NULL, 10);
      if (run <= 0)
        Log::Die("-r [num] option requires run number to be greater than 0");
      else
        options.run = run;
      break;
    case 's': options.seed = atol(RAT::optarg); break;
    case 'b': RAT::DB::Get()->SetServer(RAT::optarg); break;
    case 'i': options.input_filename = RAT::optarg; break;
    case 'o': options.output_filename = RAT::optarg; break;
    case 'p': options.python_processors.push_back(RAT::optarg); break;
    case OPT_NO_SAVE_MACRO: options.save_macro = false; break;
    }
    
    c = getopt_long(argc, argv, short_options.c_str(), opts, &option_index);
  }

  // Setup logging
  int log_level = \
    debug_log || display_level == Log::DEBUG ? Log::DEBUG : Log::DETAIL;
  Log::Init(logfilename, Log::Level(display_level), 
	       Log::Level(log_level));
  return options;
}


void help() {
    // Logging not yet initialized so use cout
    cout << "RAT " << RATVERSION << std::endl << std::endl;
    cout << "Usage: rat [-qvVh] [-s seedval] [-l logfile] [-n num] [-r run] macro1.mac macro2.mac ...\n";
    cout << " -q, --quiet     Quiet mode, only show warnings\n";
    cout << " -v, --verbose   Verbose mode, show more information. Can use multiple times\n";
    cout << " -V, --version   Show program version and exit\n";
    cout << " -h, --help      Display this help message and exit\n";
    cout << " -b, --database  URL to CouchDB database\n";
    cout << " -l, --log       Set log filename.  Defaults to rat.[hostname].[pid].log\n";
    cout << " -n              Append a number to the output filename.\n";
    cout << " -r, --run       Set the run ID number for detector properties.  Defaults to 1.\n";
    cout << " -s, --seed      Set random number seed.\n"
            "                 If no -s option is present, default is to seed with\n"
            "                 XOR of clock time and process id.\n";
    cout << " -i, --input     Set default input filename.  (Does not override filename in macro!\n";
    cout << " -o, --output    Set default output filename.  (Does not override filename in macro!\n";
    cout << " -p, --python=class Append python processor to event loop\n";
    cout << " --no-save-macro Do not save macro into output ROOT file.\n";
}

std::string get_short_hostname() {
  char c_hostname[256];
  gethostname(c_hostname, 256);
  std::string hostname(c_hostname);
  std::vector<std::string> parts = split(hostname, ".");

  if (parts.size() > 0)
    return parts[0];
  else
    return std::string("unknown");
}

std::string get_long_hostname() {
  char c_hostname[256];
  gethostname(c_hostname, 256);
  return std::string(c_hostname);
}

