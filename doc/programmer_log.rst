Logging
-------
The goal of the RAT::Log class is make it easy to output informational
messages to the user immediately at the console, while also ensuring they
get captured in a log file on disk. We also want it to be very easy for
programmers to output status messages.

The policy for RAT code is that all text output to the screen or intended
to be logged must go through the RAT::Log system. That means no one should
use cout, cerr, G4out or G4err in their RAT code.

All informational messages are classified into one of four categories (in
ascending order of detail):

1. warn -- Something unusual but not fatal has occurred. Fatal errors are
handled separately (see Die() below).

2. info -- Information about normal operation. Should not be lengthy or used
too frequently to avoid overloading the user.

3. detail -- Detailed information about software activities. A user who
wishes to know exactly what the program did should be able to get it from the
detail messages.

4. debug -- Output only of interest to those trying to debug the operation
of the software.

The logging system will keep track which of these will be output to the screen and which will be output to the log file.

Setting Up
``````````
If you are writing code to run inside the RAT application, the logging
system is already set up for you.  The user will have selected the name
of the log file, and their desired display and logging levels.  By default,
the user will see all warn and info messages, and all warn, info, and detail
messages will be written to the log file.

If you are using librat from your own application, then you will have to set
up the logging system yourself before you use it::

    #include "RATLog.hh"

    int main(int argc, char* argv[]) {
      RAT::Log::Init("mylogfile.log", RATLog::INFO, RATLog::DETAIL);
      // Do stuff here.
    }

The first parameter is the name of the file, the second is the maximum
level of detail you want to see at stdout, and the third is the maximum
level of detail you want to write to the log file.

Producing output
````````````````
Inside your code, writing messages is easy.  Just make sure to
#include "RAT/Log.hh" at the top, then use logging objects just like you
would cout or cerr::

    #include "RAT/Log.hh"

    namespace RAT {

    info << "Adding FitCentroid to event loop." << newline;
    warn << "No seed found for fit, using default." << newline;
    detail << "Fit converged in " << iterations << " iterations." << newline;
    debug << "Hit tube list: " << newline;

    }

The messages will be displayed on screen and/or written to the log file
according to the current user settings.  Nothing needs to be checked in
your code.  For example, if the user has selected "info" display and
"detail" logging, then on their screen, they will see::

    Adding FitCentroid to event loop.
    No seed found for fit, using default.

And in the log file, they will see::

    Adding FitCentroid to event loop.
    No seed found for fit, using default.
    Fit converged in 17 iterations.

The debug line did not appear anywhere because they did not select that level
of detail.

For complex formatted output with variables, C++ syntax like that used above
is really clumsy.  There is also a function included in RAT::Log called
dformat() which works just like C's printf(), but returns an STL string::

    detail << dformat("Fit converged in %d iterations.  Chi2 = %1.4f\n",
                       iterations, chi2);


See the [http://stlplus.sourceforge.net/stlplus/docs/dprintf.html STL+
documentation on dformat] for more details.

When really bad stuff happens...
````````````````````````````````
If your code encounters a major problem, it is best to bail out immediately
and tell the user why.  For this purpose, use the Die() method in the RAT::Log
class::

    RAT::Log::Die("Could not open " + filename + " for input.");

This will print that message to the "warn" log stream and then terminate the
program.

For convenience, there is also a function Assert() which aborts the program
with a log message when a condition fails::

    RAT::Log::Assert(2 + 2 == 5, "O'Brien was wrong");


