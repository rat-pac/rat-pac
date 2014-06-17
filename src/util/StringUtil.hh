#ifndef __RAT_StringUtil__
#define __RAT_StringUtil__

#include <string>

namespace RAT {

// strips leading and trailing characters from s. This function by 
// Glenn Horton-Smith.
std::string strip(const std::string &s, const std::string &stripchars);
// strip spaces, tabs, and quotation marks
std::string strip_default(const std::string &s);

} // namespace RAT

#endif
