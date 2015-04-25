#include <RAT/DBTable.hh>

namespace RAT {


DBTable::DBTable()
  : tblname(""), index(""), run_begin(0), run_end(0), bytes(0)
{
  this->tblname = "";
  this->index = "";
  table.reset(json::TOBJECT);
}

DBTable::DBTable(json::Value &jsonDoc)
  : tblname(""), index(""), run_begin(0), run_end(0), bytes(0)
{
  tblname = jsonDoc["name"].cast<std::string>();
  if (jsonDoc.isMember("index"))
    index = jsonDoc["index"].cast<std::string>();
  
  if (jsonDoc.isMember("run_range"))
    SetRunRange(jsonDoc["run_range"][(unsigned) 0].cast<int>(), jsonDoc["run_range"][(unsigned) 1].cast<int>());
    
  table = jsonDoc;
}

DBTable::DBTable(std::string _tblname, std::string _index)  
  : tblname(_tblname), index(_index), run_begin(0), run_end(0), bytes(0)
{
  table.reset(json::TOBJECT);
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
  else {
    json::Value val = table[name];
    switch (val.getType()) {
        case json::TINTEGER:
        case json::TUINTEGER:
        case json::TBOOL:
            return DBTable::INTEGER;
        case json::TREAL:
            return DBTable::DOUBLE;
        case json::TSTRING:
            return DBTable::STRING;
        case json::TARRAY:
            if (val.getArraySize() > 0) {
                switch (val[0].getType()) {
                    case json::TINTEGER:
                    case json::TUINTEGER:
                    case json::TBOOL:
                        return DBTable::INTEGER_ARRAY;
                    case json::TREAL:
                        return DBTable::DOUBLE_ARRAY;
                    case json::TSTRING:
                        return DBTable::STRING_ARRAY;
                    default: 
                        return DBTable::JSON;
                }
            }
            return DBTable::JSON;
        default:
            return DBTable::JSON;
    }
  }
}

std::vector<std::string> DBTable::GetFieldList() const
{
  std::vector<std::string> fields = table.getMembers();

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

int DBTable::GetI(const std::string &name) const {
    if (!table.isMember(name))
        throw DBNotFoundError(tblname, index, name);
    try {
        return table[name].cast<int>();
    } catch (...) {   
        throw DBWrongTypeError(tblname, index, name, INTEGER, GetFieldType(name));
    }
}

double DBTable::GetD(const std::string &name) const {
    if (!table.isMember(name))
        throw DBNotFoundError(tblname, index, name);
    try {
        return table[name].cast<double>();
    } catch (...) {   
        throw DBWrongTypeError(tblname, index, name, INTEGER, GetFieldType(name));
    }
}

std::string DBTable::GetS(const std::string &name) const {
    if (!table.isMember(name))
        throw DBNotFoundError(tblname, index, name);
    else if (table[name].getType() != json::TSTRING)
        throw DBWrongTypeError(tblname, index, name, STRING, GetFieldType(name));
    else
        return table[name].cast<std::string>();
}

std::vector<std::string> DBTable::GetSArray(const std::string &name) const {
    if (!table.isMember(name))
        throw DBNotFoundError(tblname, index, name);
    else if (GetFieldType(name) != STRING_ARRAY)
        throw DBWrongTypeError(tblname, index, name, STRING_ARRAY, GetFieldType(name));
    else
        return table[name].toVector<std::string>();
}

json::Value DBTable::GetJSON(const std::string &name) const {
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
    const json::Value &json_array = table[name];
    return json_array.toVector<int>();
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
    const json::Value &json_array = table[name];
    return json_array.toVector<double>();
  }    
}




} // namespace RAT
