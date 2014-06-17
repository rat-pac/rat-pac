#include <RAT/StringUtil.hh>

namespace RAT {


std::string strip(const std::string &s, const std::string &stripchars)
{
  std::string result = s;
  int i= result.find_first_not_of(stripchars);
  if (i < 0 || i >= (int)result.length()) {
    return std::string("");
  }
  result = result.substr(i);
  i = result.find_last_not_of(stripchars);
  if (i < 0 || i >= (int)result.length()) {
    return std::string("");
  }
  result.resize(i+1);
  return result;
}

std::string strip_default(const std::string &s)
{
  return strip(s, " \t\"");
}



} // namespace RAT
