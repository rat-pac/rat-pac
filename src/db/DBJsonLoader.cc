#include <RAT/DBJsonLoader.hh>
#include <RAT/DBTable.hh>
#include <RAT/Log.hh>
#include <RAT/DBExceptions.hh>
#include <RAT/ReadFile.hh>

namespace RAT
{

std::vector<RAT::DBTable *> DBJsonLoader::parse(const std::string &filename)
{
  std::string contents;
  
  if(ReadFile(filename, contents) < 0)
    throw FileNotFoundError(filename);
  
  return parseString(contents);
}


std::vector<RAT::DBTable *> DBJsonLoader::parseString(const std::string &data)
{
  std::vector<RAT::DBTable *> tables;
  
  json::Reader reader(data);
  json::Value jsonDoc;

  while (reader.getValue(jsonDoc)) {        
    // Copy the fields into a RATDB table
    Log::Assert(jsonDoc.getType() == json::TOBJECT, "RATDB:: Non-object JSON document found.");
    DBTable *newTable = convertTable(jsonDoc);
    tables.push_back(newTable);    
  }
  
  return tables;
}


RAT::DBTable *DBJsonLoader::convertTable(json::Value &jsonDoc)
{

  return new DBTable(jsonDoc);
  
}


} // namespace RAT
