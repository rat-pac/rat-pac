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

void findJsonDoc(const std::string &data, size_t &start, size_t &end)
{
  // Locate initial opening block
  start = data.find('{', start);
  if (start == std::string::npos) {
    end = start;
    return;
  }
  
  int depth_count = 1;
  end = start + 1;
  while (depth_count > 0 && end < data.size()) {
    if (data[end] == '{')
      depth_count++;
    else if (data[end] == '}')
      depth_count--;
      
    end++;
  }
  
  if (depth_count != 0)
    end = std::string::npos;
}

std::vector<RAT::DBTable *> DBJsonLoader::parseString(const std::string &data)
{
  std::vector<RAT::DBTable *> tables;
  
  Json::Reader reader;
  Json::Value jsonDoc;

  size_t start = 0;
  size_t end = 0;
  findJsonDoc(data, start, end);
  
  while (end != std::string::npos) {
    reader.parse(data.c_str() + start, data.c_str() + end, jsonDoc, false);          
    // Copy the fields into a RATDB table
    Log::Assert(jsonDoc.isObject(), "RATDB:: Non-object JSON document found.");
    
    DBTable *newTable = convertTable(jsonDoc);
    tables.push_back(newTable);
    
    start = end + 1;
    findJsonDoc(data, start, end);
  }
  
  return tables;
}


RAT::DBTable *DBJsonLoader::convertTable(const Json::Value &jsonDoc)
{
  std::string tblname = jsonDoc["name"].asString();
  std::string index = "";
  
  if (jsonDoc.isMember("index"))
    index = jsonDoc["index"].asString();
  
  DBTable *newTable = new DBTable(tblname, index);
  if (jsonDoc.isMember("run_range"))
    newTable->SetRunRange(jsonDoc["run_range"][(unsigned) 0].asInt(), 
                          jsonDoc["run_range"][(unsigned) 1].asInt());

  std::vector<int> iarray;
  std::vector<double> darray;
  std::vector<std::string> sarray;
  Json::ValueType elementType;

  std::vector<std::string> members = jsonDoc.getMemberNames();
  for (unsigned i=0; i < members.size(); i++) {
    const std::string fieldname = members[i];
    if (fieldname == "_attachments")
      continue;

    const Json::Value field = jsonDoc[fieldname];
  
    switch (field.type()) {
      case Json::intValue :
        newTable->SetI(fieldname, field.asInt());
        break;
      case Json::realValue :
        newTable->SetD(fieldname, field.asDouble());
        break;
      case Json::stringValue :
        newTable->SetS(fieldname, field.asString());
        break;
    
      case Json::arrayValue : 
        if (!field.isTypedArray())
          newTable->SetJSON(fieldname, field);
        else {
          elementType = field.arrayType();
          switch(elementType) {
            case Json::intValue :
              newTable->SetIArray(fieldname, field.asIntArray());
              break;
                    
            case Json::realValue :
              newTable->SetDArray(fieldname, field.asDoubleArray());
              break;
        
            case Json::stringValue :
              newTable->SetSArray(fieldname, field.asStringArray());
              break;
          
            default:
              newTable->SetJSON(fieldname, field);
          } // switch elementType
        }
        break;
      
      default:
        newTable->SetJSON(fieldname, field);
    }
  }
  
  return newTable;
}


} // namespace RAT
