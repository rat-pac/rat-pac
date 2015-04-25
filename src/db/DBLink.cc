#include <RAT/DB.hh>
#include <string>
#include <vector>
#include <RAT/Log.hh>
#include <RAT/DS/RunStore.hh>

namespace RAT {


DBLink::DBLink(DB *_db, std::string _tblname, std::string _index, int _currentRun)
{
  db = _db;
  tblname = _tblname;
  index = _index;
  currentRun = _currentRun;
}


DBLink::~DBLink()
{
  // We don't own the db or table pointers, but we do need to inform
  // the db object of our destruction, if it still exists
  if (db)
    db->RemoveLink(this);
}

int DBLink::GetI(const std::string &name)
{
  return Pick<int>(name, DBTable::INTEGER);
}


double DBLink::GetD(const std::string &name)
{
  return Pick<double>(name, DBTable::DOUBLE);
}

std::string DBLink::GetS(const std::string &name)
{
  return Pick<std::string>(name, DBTable::STRING);
}

std::vector<int> DBLink::GetIArray(const std::string &name)
{
  return Pick< std::vector<int> >(name, DBTable::INTEGER_ARRAY);
}


std::vector<float> DBLink::GetFArrayFromD(const std::string &name)
{
  std::vector<float> returnVal;
  std::vector<double> tempVal;
  
  tempVal = DBLink::GetDArray(name);
  returnVal = DBLink::DArrayToFArray(tempVal);
  return returnVal;

}

std::vector<float> DBLink::DArrayToFArray(const std::vector<double> &input)
{
  int nval = input.size();
  std::vector<float> returnVal;
  returnVal.resize(nval);
  for(int i=0;i<nval;++i){
    returnVal[i] = (float)input[i];
  }
  return returnVal;
}

std::vector<double> DBLink::GetDArray(const std::string &name)
{
  return Pick< std::vector<double> >(name, DBTable::DOUBLE_ARRAY);
}

std::vector<std::string> DBLink::GetSArray(const std::string &name)
{
  return Pick< std::vector<std::string> >(name, DBTable::STRING_ARRAY);
}

json::Value DBLink::GetJSON(const std::string &name)
{
  return Pick< json::Value >(name, DBTable::JSON);
}

// Helper function to make Pick() easier to read
// Type equality is as you would expect, but JSON equals everything except NOTFOUND
inline bool equivalent(DBTable::FieldType a, DBTable::FieldType b)
{
  if (a == b) return true;
  else if (a == DBTable::JSON && b != DBTable::NOTFOUND) return true;
  else if (b == DBTable::JSON && a != DBTable::NOTFOUND) return true;
  else return false;
}

template <class T>
T DBLink::Pick(std::string fieldname, DBTable::FieldType ftype) const
{
  DBTable *tbl;
  
  // First try user plane
  tbl = db->GetUserTable(tblname, index);
  if (!tbl || !equivalent(tbl->GetFieldType(fieldname), ftype)) {
    // Then try the run plane
    tbl = db->GetRunTable(tblname, index, currentRun);
    if(!tbl || !equivalent(tbl->GetFieldType(fieldname), ftype)) {
      
      // Finally try default plane
      tbl = db->GetDefaultTable(tblname, index);
      if (!tbl || !equivalent(tbl->GetFieldType(fieldname), ftype))
        throw DBNotFoundError(tblname, index, fieldname);
    }
  }
    
  // Make class explicit to satisfy Sun CC 5.3
  T value = tbl->DBTable::Get<T>(fieldname);

  // Trace DB accesses
  Log::TraceDBAccess(tblname, index, fieldname, value);
  
  return value;
}


}
