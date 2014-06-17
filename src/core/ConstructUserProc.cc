#include <RAT/Processor.hh>

namespace RAT {

// To add one or more user processors to your build of RAT, copy this
// file and add code to construct your RATUserProc subclass.

Processor *construct_user_proc(std::string /*userProcName*/)
{
  return 0;  // No user processors by default
}


} // namespace RAT
