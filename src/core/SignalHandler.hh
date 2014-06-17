/** @class SignalHandler
 *  Catches UNIX signals sent to the main application by the operating system.
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 *  This is a singleton which handles UNIX signals sent by the
 *  operating system.  It must be initialized in the main() function
 *  of the program by calling SignalHandler::Init().  Then SIGINT
 *  (sent by Ctrl-C) will be trapped by this handler.  All other
 *  signals remain connected to the default handlers.
 *
 *  When SIGINT is first received, a flag is set and execution resumes
 *  as normal.  The flag can be checked by calling
 *  SignalHandler::IsTermRequested().  If it returns true, then the
 *  current operation should be aborted as soon as it is possible
 *  without losing data.  In general, the only objects which need to
 *  check this flag are subclasses of Producer.  If the user really
 *  needs to abort now, they can hit Ctrl-C again, and the second time
 *  SIGINT is received, the application terminates immediately with no cleanup.
 *
 *  Note that this class only contains static methods and cannot be instantiated.
 */

#ifndef __RAT_SignalHandler__
#define __RAT_SignalHandler__

#include <signal.h>

namespace RAT {


class SignalHandler {
public:

  /** Initialize signal handler.
   * 
   *  After this call, this class will catch SIGINT.
   */
  static void Init();

  /** If true, you should stop whatever you are doing after the
      current event. */
  static bool IsTermRequested() { return sigintPending; }

  /** Send a SIGINT to the handler.
   *
   *  In general, this method is only called by the operating system
   *  in response to a SIGINT.  You should not call it unless you want to
   *  simulate a Ctrl-C.
   * 
   *  The first time this is received, the termination flag is set.
   *  The current SIGINT handler is also changed back to the default
   *  OS handler so that the second SIGINT will cause the default
   *  behavior (usually immediate termination of the application).
   *  Execution resumes without interruption.
   */
  static void SigInt();

private:
  /** This class cannot be instantiated. */
  SignalHandler() { /* Do nothing */ };


  /** Set to true if a SIGINT has been received. */
  static bool sigintPending;
};


} // namespace RAT

#endif
