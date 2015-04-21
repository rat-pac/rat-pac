#include <RAT/Log.hh>
#include <RAT/fileio.hpp>
#include <iostream>
#include <stdlib.h>

using namespace std;

namespace RAT {

//********************************************************************
// The purpose of this code is to override the default cout/cerr
// objects and redirect them through the Log system.  Note that
// this is not circular because Log uses STLplus TextIO rather than
// iostreams.  TextIO ultimately goes out through stderr/stdout,
// bypassing cout/cerr entirely.
//********************************************************************

// GCC 2.96 ostream is not quite C++ spec
#if defined(__GNUC__) && __GNUC__ < 3
#define int_type int
#endif

class otext_streambuf : public std::streambuf {
public:
  otext_streambuf(otext *_out) { out = _out; };

  virtual int_type overflow (int_type c) {
    if (c != EOF) {
      (*out) << (char) c;
    }
    return c;
  };
protected:
  otext *out;
};

otext_streambuf warn_streambuf(&warn);
otext_streambuf info_streambuf(&info);
//********************************************************************

//**** Definition of the global logging objects.
omtext warn(ferr); // default to screen output
omtext info(fout); // default to screen output
omtext detail;
omtext debug;

//**** Static member variables
omtext *Log::outstreams[4] = { &warn, &info, &detail, &debug };
std::string Log::filename;
oftext Log::logfile;
ostext Log::logbuffer;
bool Log::use_buffer = true;
int Log::display_level = Log::DEBUG;
int Log::log_level = Log::DETAIL;
std::string Log::macro;
bool Log::enable_dbtrace = false;
TMap *Log::dbtrace = 0;
std::vector<std::pair<std::string, TObject*> > Log::objects;

// Empty constructor.  No one should call this!
Log::Log()
{
}

//**** Member functions

  bool Log::Init(std::string _filename, Level display, Level log,
		 bool _use_buffer)
{
  // Redirect cout and cerr through Log
  cerr.rdbuf(&warn_streambuf);
  cout.rdbuf(&info_streambuf);

  filename = _filename;
  logfile.open(filename, 0);
  display_level = display;
  log_level = log;
  Log::use_buffer = _use_buffer;
  dbtrace = new TMap();
  dbtrace->SetOwner();
  enable_dbtrace = true;

  SetupIO();
  return true; // FIXME: How do I figure out if logfile didn't open?
}

void Log::SetDisplayLevel(Level level)
{
  display_level = level;
  SetupIO();
}

void Log::SetLogLevel(Level level)
{
  log_level = level;
  SetupIO();
}

void Log::Die(std::string message, int return_code)
{
  warn << message << newline;
  exit(return_code);
}

void Log::Assert(bool condition, std::string message, int return_code)
{
  if (!condition) {
    warn << message << newline;
    exit(return_code);
  }
}

void Log::SetupIO()
{
  for (int i=WARN; i <= DEBUG; i++) {
    ClearOMText(outstreams[i]);
    if (display_level >= i) {
      if (i == WARN)
	outstreams[i]->add(ferr); // special case to ensure warn is unbuffered
      else
	outstreams[i]->add(fout);
    }
    if (log_level >= i) {
      outstreams[i]->add(logfile);
      if (use_buffer)
	outstreams[i]->add(logbuffer);
    }
  }
}

void Log::ClearOMText(omtext *out)
{
  int count = out->device_count();
  for (int i = 0; i < count; i++)
    out->remove(0); // Always remove head device
}


} // namespace RAT

