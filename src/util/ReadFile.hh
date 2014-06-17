#ifndef __RAT_ReadFile__
#define __RAT_ReadFile__

#include <string>

namespace RAT {
  
  // Reads a text file into a string.  The file can optionally be bzip2 compressed.
  // Returns 0 if success, negative if file cannot be opened
  int ReadFile(const std::string &filename, std::string &filecontents);
}

#endif
