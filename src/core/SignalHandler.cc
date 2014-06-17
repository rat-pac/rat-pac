#include <RAT/SignalHandler.hh>
#include <stdlib.h>

namespace RAT {


void signal_handler(int signo)
{
  if (signo == SIGINT)
    SignalHandler::SigInt();
}

bool SignalHandler::sigintPending = false;

void SignalHandler::Init()
{
  sigintPending = false;
  signal(SIGINT, signal_handler);
}

void SignalHandler::SigInt()
{
  if (!sigintPending) {
    sigintPending = true;

    signal(SIGINT, SIG_DFL); // Reset signal handler so next Ctrl-C terminates
                             // immediately if the user is in a hurry.
  } 
}

} // namespace RAT
