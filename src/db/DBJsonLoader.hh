#ifndef __RAT_DBJsonLoader__
#define __RAT_DBJsonLoader__

#include <string>
#include <vector>
#include <iostream>
#include <RAT/json.hh>

namespace RAT {

class DBTable;


/* A $#%!! class with static functions to make rootcint happy */
class DBJsonLoader
{
public:
  /** Returns a list of all tables found in JSON text file @p filename. */
  static std::vector<RAT::DBTable *> parse(const std::string &filename);

  /** Returns a list of all tables found in JSON string. */
  static std::vector<RAT::DBTable *> parseString(const std::string &data);
  
  /** Converts a JSON document to a RATDB table */
  static RAT::DBTable *convertTable(json::Value &jsonDoc);
  
};

} // namespace RAT

#endif
