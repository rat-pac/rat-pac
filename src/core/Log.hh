/** @class Log
 *  Logging of informational, warning, and error messages.
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 *  Log is a singleton which handles all the message reporting to
 *  the user, ensuring that messages are displayed and sent to a log
 *  file on disk as needed.  You should not use @c cerr and @c cout!
 *
 *  @section Initialization
 *
 *  In the main() function, Log::Init() is called to open the log
 *  file and set the verbosity levels for display to the screen and
 *  for writing to disk.  The levels are (in ascending verbosity):
 *   - Log::WARN: Something has gone wrong.
 *   - Log::INFO: Short information messages about what the
 *  program is currently doing.
 *   - Log::DETAIL: Longer messages describing the settings being
 *  used.  This should be sufficient to know what the program did and
 *  what information it used.
 *   - Log::DEBUG: Very long messages relevant only for
 *  troubleshooting a software malfunction.
 *
 *  By default, WARN and INFO are shown to the screen, and WARN, INFO,
 *  and DETAIL are written to disk.  Messages with a verbosity higher
 *  than the current setting are discarded.
 *
 *  @section Usage 
 *
 *  To send messages to the logging system, there are 4 global
 *  ostream-like objects: @c warn, @c info, @c detail, @c debug.  You
 *  can send strings, numbers, etc to them using the << operator:

@verbatim
info << "Adding FitCentroid to event loop.\n";
warn << "No seed found for fit, using default.\n";
detail << "Fit converged in " << iterations << " iterations.\n";
debug << "Hit tube list: \n";
@endverbatim

 *  You do not need if-statements to check the current log level, as
 *  this will be done for you.  Note that while these objects behave
 *  much like the ostream cout and cerr, they are not subclasses of
 *  ostream.  For example @c endl does not work on these objects.  You
 *  should use @c "\n" or @c newline instead.  (For the curious, these
 *  are STLplus textio objects.)
 *
 *  @section Die  When Really Bad Stuff Happens
 *
 *  If your code encounters a major problem, it is best to bail out
 *  immediately and tell the user why. For this purpose, use the Die()
 *  method:

@verbatim
Log::Die("Could not open " + filename + " for input.");
@endverbatim

 *  This will print that message to the "warn" log stream and then
 *  terminate the program. It is generally best to use Die() if you
 *  want to make very certain the user sees your error.  Warnings that do not
 *  terminate the program are usually not noticed in the flood of
 *  output that is sent to the screen.
 *
 *  @section Assert  Good for sanity checks
 *
 *  Works like standard assert(), but lets you print something to the user
 *  before dying if condition fails.
 */

#ifndef __RAT_Log__
#define __RAT_Log__

#include <string>
#include <sstream>
#include <vector>
#include <utility>

#include "dprintf.hpp"
#include "fileio.hpp"
#include "stringio.hpp"
#include "multiio.hpp"
#include "string_utilities.hpp"

#include <TObject.h>
#include <TMap.h>
#include <TObjString.h>
#include <RAT/json.hh>

namespace RAT {

class Log {
public:
  
  /** Verbosity levels */
  enum Level { WARN=0,   /**< Warning messages */
	       INFO=1,   /**< Short informational messages */
	       DETAIL=2, /**< Longer messages related to settings and
			      normal operation */
	       DEBUG=3   /**< Messages only of use to troubleshoot
			      broken code */
	     };


  /** Initialize the logging system.
   *
   *  @param  _filename  Name of log file to open for writing.
   *  @param  display    Verbosity level for display on screen
   *  @param  log        Verbosity level for writing to log file.
   *  @param  use_buffer Keep copy of log in memory buffer?
   */
  static bool Init(std::string _filename, 
		   Level display=INFO, Level log=DETAIL,
		   bool use_buffer=true);

  /** Get the current verbosity level for display on screen. */
  static int GetDisplayLevel() { return display_level; };
  
  /** Set verbosity level for display on screen. */
  static void SetDisplayLevel(Level level);

  /** Get the current verbosity level for writing to log file */
  static int GetLogLevel() { return log_level; };

  /** Set verbosity level for writing to log file. */
  static void SetLogLevel(Level level);

  /** Write @p message to @p warn stream and immediately terminate,
      sending @p return_code back to OS. */
  static void Die(std::string message, int return_code=1);

  /** Write @p message to @p warn stream and immediately terminate if @condition
      is not true.  @p return_code is returned to the OS to signal job failure */
  static void Assert(bool condition, std::string message, int return_code=1);

  /** Return reference to string containing entire log from this session */
  static const std::string &GetLogBuffer() { return logbuffer.get_string(); };

  /** Add macro commands to buffer.  Don't forget newlines! */
  static void AddMacro(const std::string &contents) {
    macro += contents;
  };
  /** Get the macro commands that have been run so far */
  static const std::string &GetMacro() { return macro; };

  /** Log a TObject.  Do not delete this pointer after you log it! */
  static void AddObject(const std::string &name, TObject *obj) { 
    objects.push_back(std::pair<std::string, TObject *>(name, obj)); 
  };

  /** Get an array of logged TObjects. Should be used by output processors
    just before closing their output file. */
  static std::vector< std::pair<std::string, TObject *> > GetObjects() { return objects; };

  /** Enable/Disable tracing of RATDB accesses.
   *
   * AddDBAccess() does nothing if tracing is disabled (default)
   */
  static void SetDBTraceState(bool state=true) { enable_dbtrace = state; };

  /** Get current DB tracing state. */
  static bool GetDBTraceState() { return enable_dbtrace; };

  /** Add a RATDB integer access to the DB trace. */
  inline static void TraceDBAccess(const std::string &table,
				   const std::string &index,
				   const std::string &field,
				   int value);
  /** Add a RATDB double access to the DB trace. */
  inline static void TraceDBAccess(const std::string &table,
				   const std::string &index,
				   const std::string &field, 
				   double value);
  /** Add a RATDB string access to the DB trace. */
  inline static void TraceDBAccess(const std::string &table,
				   const std::string &index,
				   const std::string &field,
				   const std::string &value);

  /** Add a RATDB integer array access to the DB trace. */
  inline static void TraceDBAccess(const std::string &table,
				   const std::string &index,
				   const std::string &field,
				   const std::vector<int> &value);
  /** Add a RATDB double array access to the DB trace. */
  inline static void TraceDBAccess(const std::string &table,
				   const std::string &index,
				   const std::string &field,
				   const std::vector<double> &value);
  /** Add a RATDB string access to the DB trace. */
  inline static void TraceDBAccess(const std::string &table,
				   const std::string &index,
				   const std::string &field,
				   const std::vector<std::string> &value);

  /** Add a RATDB JSON object access to the DB trace. */
  inline static void TraceDBAccess(const std::string &table,
				   const std::string &index,
				   const std::string &field,
				   const json::Value &value);

  static TMap *GetDBTraceMap() { return dbtrace; };

protected:
  /** This class cannot be instantiated. */
  Log();

  /** Add a string key/value pair to the dbtrace map */
  inline static void AddDBEntry(const std::string &key, const std::string &value);

  /** Reset output streams to according to verbosity levels.
   *
   *  Call this method after changing @c display_level or @c log_level
   *  to update where @c warn, @c info, @c detail, @c debug send
   *  messages.
   */
  static void SetupIO();

  /** Utility function to remove all output streams from an omtext
      object. */
  static void ClearOMText(omtext *out);

  /** Pointers to global output streams in order of verbosity. */
  static omtext *outstreams[4];

  static std::string filename;   /**< Name of log file. */
  static oftext logfile;         /**< Log file object */
  static ostext logbuffer;         /**< In-memory copy of logfile object */
  static bool use_buffer;        /**< Set if log file should be saved in memory */
  static int display_level;      /**< Current display verbosity */
  static int log_level;          /**< Current log file verbsoity */
  
  static std::string macro;      /**< Buffer of macro commands run so far */

  static bool enable_dbtrace;    /**< Enable RATDB tracing? */
  static TMap *dbtrace;          /**< Record of all accessed RATDB fields */

  static std::vector<std::pair<std::string, TObject *> > objects;  /**< Record of logged TObjects */
};

extern omtext warn;
extern omtext info;
extern omtext detail;
extern omtext debug;

/// Inline functions

void Log::AddDBEntry(const std::string &key, const std::string &value)
{
  if (enable_dbtrace && !dbtrace->FindObject(key.c_str()))
    dbtrace->Add(new TObjString(key.c_str()), new TObjString(value.c_str()));
}

// The parameter is type double so this can be used as a seed for to_ratdb_double_string()
inline std::string to_ratdb_float_string(double value)
{
  if (value == 0.0)
    return std::string("0.0");
  else
    return ::to_string(value);
}

inline std::string to_ratdb_double_string(double value)
{
  std::string str = to_ratdb_float_string(value);
  size_t pos = str.find("d");
  if (pos == std::string::npos)
    str += "d";
  return str;
}

inline std::string escape_ratdb_string(const std::string &value)
{
  if (value.find("\"") != std::string::npos)
    Log::Die("RATDB Trace Eror: value string with quotes inside!");
  return "\"" + value + "\"";
}

void Log::TraceDBAccess(const std::string &table, const std::string &index,
			const std::string &field, int value)
{
  AddDBEntry(table + "[" + index + "]."+field, ::to_string(value));
}

void Log::TraceDBAccess(const std::string &table, const std::string &index,
			const std::string &field, double value)
{
  AddDBEntry(table + "[" + index + "]."+field, to_ratdb_double_string(value));
}

void Log::TraceDBAccess(const std::string &table, const std::string &index,
			const std::string &field, const std::string &value)
{
  AddDBEntry(table + "[" + index + "]."+field, escape_ratdb_string(value));
}

void Log::TraceDBAccess(const std::string &table, const std::string &index,
			const std::string &field,
			const std::vector<int> &value)
{
  std::string key = table + "[" + index + "]."+field;
  if (!enable_dbtrace || dbtrace->FindObject(key.c_str()))
    return;

  std::string str_value("[ ");
  for (unsigned i=0; i < value.size(); i++) {
    str_value += ::to_string(value[i]) + ", ";
  }
  str_value += "]";

  AddDBEntry(key, str_value);
}

void Log::TraceDBAccess(const std::string &table, const std::string &index,
			const std::string &field,
			const std::vector<double> &value)
{
  std::string key = table + "[" + index + "]."+field;
  if (!enable_dbtrace || dbtrace->FindObject(key.c_str()))
    return;

  std::string str_value("[ ");
  for (unsigned i=0; i < value.size(); i++) {
    str_value += to_ratdb_double_string(value[i]) + ", ";
  }
  str_value += "]";

  AddDBEntry(key, str_value);  
}

void Log::TraceDBAccess(const std::string &table, const std::string &index,
			const std::string &field,
			const std::vector<std::string> &value)
{
  std::string key = table + "[" + index + "]."+field;
  if (!enable_dbtrace || dbtrace->FindObject(key.c_str()))
    return;

  std::string str_value("[ ");
  for (unsigned i=0; i < value.size(); i++) {
    str_value += escape_ratdb_string(value[i]) + ", ";
  }
  str_value += "]";

  AddDBEntry(key, str_value);  
}


void Log::TraceDBAccess(const std::string &table, const std::string &index,
			const std::string &field,
			const json::Value &value)
{
  std::string key = table + "[" + index + "]."+field;
  if (!enable_dbtrace || dbtrace->FindObject(key.c_str()))
    return;
    
  std::stringstream ss;
  json::Writer writer(ss);
  writer.putValue(value);
  
  AddDBEntry(key, ss.str());  
}

} // namespace RAT



#endif
