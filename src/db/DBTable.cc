#include <RAT/DBTable.hh>

namespace RAT {


DBTable::DBTable()
  : tblname(""), index(""), run_begin(0), run_end(0), bytes(0)
{
  this->tblname = "";
  this->index = "";
}

DBTable::DBTable(std::string _tblname, std::string _index)  
  : tblname(_tblname), index(_index), run_begin(0), run_end(0), bytes(0)
{
}

DBTable::~DBTable()
{
  // Do nothing
}

DBTable::FieldType DBTable::GetFieldType(std::string name) const
{
  if (iatbl_deferred.present(name)) return DBTable::INTEGER_ARRAY;
  else if (datbl_deferred.present(name)) return DBTable::DOUBLE_ARRAY;
  else if (arrayTypeCache.present(name)) return arrayTypeCache[name];
  else if (!table.isMember(name)) return DBTable::NOTFOUND;
  // Have to handle this special way because json-cpp is quite
  // free about pretending real values with no fractional part are integers
  // which messes with our type system
  else if (table[name].type() == Json::realValue) return DBTable::DOUBLE;
  else if (table[name].isInt()) return DBTable::INTEGER;
  else if (table[name].isString()) return DBTable::STRING;
  else return DBTable::JSON;
}

std::vector<std::string> DBTable::GetFieldList() const
{
  std::vector<std::string> fields = table.getMemberNames();

  stlplus::hash< std::string, DBFieldCallback*, pyhash>::const_iterator ia_deferred_iter = iatbl_deferred.begin();
  while (ia_deferred_iter != iatbl_deferred.end()) {
    fields.push_back(ia_deferred_iter->first);
    ia_deferred_iter++;
  }

  stlplus::hash< std::string, DBFieldCallback*, pyhash>::const_iterator da_deferred_iter = datbl_deferred.begin();
  while (da_deferred_iter != datbl_deferred.end()) {
    fields.push_back(da_deferred_iter->first);
    da_deferred_iter++;
  }
  
  return fields;
}

int DBTable::GetI(const std::string &name) const
{
  if (!table.isMember(name))
    throw DBNotFoundError(tblname, index, name);
  else if (!table[name].isInt())
    throw DBWrongTypeError(tblname, index, name, INTEGER, GetFieldType(name));
  else
    return table[name].asInt();
}

double DBTable::GetD(const std::string &name) const
{
  if (!table.isMember(name))
    throw DBNotFoundError(tblname, index, name);
  else if (!table[name].isDouble())
    throw DBWrongTypeError(tblname, index, name, DOUBLE, GetFieldType(name));
  else
    return table[name].asDouble();  
}

std::string DBTable::GetS(const std::string &name) const
{
  if (!table.isMember(name))
    throw DBNotFoundError(tblname, index, name);
  else if (!table[name].isString())
    throw DBWrongTypeError(tblname, index, name, STRING, GetFieldType(name));
  else
    return table[name].asString();
}

std::vector<std::string> DBTable::GetSArray(const std::string &name) const
{
  if (!table.isMember(name))
    throw DBNotFoundError(tblname, index, name);
  else if (GetFieldType(name) != STRING_ARRAY)
    throw DBWrongTypeError(tblname, index, name, STRING_ARRAY, GetFieldType(name));
  else {
    const Json::Value &json_array = table[name];
    return json_array.asStringArray();
  }
}

Json::Value DBTable::GetJSON(const std::string &name) const
{
  if (!table.isMember(name))
    throw DBNotFoundError(tblname, index, name);
  else
    return table[name];
}


std::vector<int> DBTable::GetIArray(const std::string &name) const {
  // Fetch if deferred
  DBTable *me = const_cast<DBTable *>(this); // grumble, grumble
  if (me->iatbl_deferred.present(name)) {
    me->SetIArray(name, me->iatbl_deferred[name]->FetchIArray(GetS("_id"), name));
    me->iatbl_deferred.erase(name);
  }
  
  if (!table.isMember(name))
    throw DBNotFoundError(tblname, index, name);
  else if (GetFieldType(name) != INTEGER_ARRAY)
    throw DBWrongTypeError(tblname, index, name, INTEGER_ARRAY, GetFieldType(name));
  else {
    const Json::Value &json_array = table[name];
    return json_array.asIntArray();
  }
}

std::vector<double> DBTable::GetDArray(const std::string &name) const {
  // Fetch if deferred
  DBTable *me = const_cast<DBTable *>(this); // grumble, grumble
  if (me->datbl_deferred.present(name)) {
    me->SetDArray(name, me->datbl_deferred[name]->FetchDArray(GetS("_id"), name));
    me->datbl_deferred.erase(name);
  }
    
  if (!table.isMember(name))
    throw DBNotFoundError(tblname, index, name);
  else if (GetFieldType(name) != DOUBLE_ARRAY)
    throw DBWrongTypeError(tblname, index, name, DOUBLE_ARRAY, GetFieldType(name));
  else {
    const Json::Value &json_array = table[name];
    return json_array.asDoubleArray();
  }    
}




} // namespace RAT
