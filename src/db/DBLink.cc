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
  return Get<int>(name);
}


double DBLink::GetD(const std::string &name)
{
  return Get<double>(name);
}

std::string DBLink::GetS(const std::string &name)
{
  return Get<std::string>(name);
}

bool DBLink::GetZ(const std::string &name)
{
  return Get<bool>(name);
}


std::vector<int> DBLink::GetIArray(const std::string &name)
{
  return Get<std::vector<int> >(name);
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
  return Get<std::vector<double> >(name);
}

std::vector<std::string> DBLink::GetSArray(const std::string &name)
{
  return Get<std::vector<std::string> >(name);
}

std::vector<bool> DBLink::GetZArray(const std::string &name)
{
  return Get<std::vector<bool> >(name);
}


json::Value DBLink::GetJSON(const std::string &name)
{
  return Get<json::Value>(name);
}


}
