/** @class DBTable
 *  Table of fields in database
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 *  This class actually holds the contents of a RATDB table in memory.
 *  Normally it is created by DB_TextLoader, the backend which
 *  reads the .ratdb text file format and loads the data into memory.
 */
#ifndef __RAT_DBTable__
#define __RAT_DBTable__

#include <string>
#include <vector>
#include <RAT/hash.hpp>
#include <RAT/DBFieldCallback.hh>
#include <RAT/HashFunc.hh>
#include <RAT/json.hh>

namespace RAT {

class DBTable {
public:

  /** Create new empty table. */
  DBTable();
  
  /** Create new table from json object. */
  DBTable(json::Value &jsonDoc);

  /** Create new table with specific name and index. */
  DBTable(std::string tblname, std::string index="");

  /** Destroy table. */
  ~DBTable();

  std::string GetName() const { return tblname; };
  void SetName(std::string _tblname) { tblname = _tblname; };

  std::string GetIndex() const { return index; };
  void SetIndex(std::string _index) { index = _index; }


  // Validity range methods

  /** Get run number for which this table is valid. */
  int GetRunBegin() const { return run_begin; };
  /** Get run number for which this table is valid. */
  int GetRunEnd() const { return run_end; };

  /** Returns true if validity range flags this as a user-plane table. */
  bool IsUser() const { return (run_begin == -1) && (run_end == -1); };
  /** Returns true if validity range flags this as a default-plane table. */
  bool IsDefault() const { return (run_begin == 0) && (run_end == 0); };
  /** Returns true if this table is valid for the run given */
  bool IsValidRun(const int run) { return (run >= run_begin) && (run <= run_end); };

  /** Set run range for which this table is valid.  Begin and end are inclusive*/
  void SetRunRange(int _run_begin, int _run_end) { 
    run_begin = _run_begin; run_end = _run_end;
  }
  /** Set this as a user-override table */
  void SetUser() { SetRunRange(-1, -1); };
  /** Set this as a default table */
  void SetDefault() {SetRunRange(0, 0); };

  // Methods for manipulating the fields in this table

  /** Data type of field. */
  enum FieldType { NOTFOUND, /**< Type for fields that don't exist */
		   INTEGER, DOUBLE, STRING,
		   INTEGER_ARRAY, DOUBLE_ARRAY, STRING_ARRAY,
		   JSON };
  
  /** Get data type of field in this table.
   *  @returns DBTable::NOTFOUND if the field does not exist, otherwise
   *           returns field type.
   */
  FieldType GetFieldType(std::string name) const;
  
  /** Get a vector of field names.  Call GetFieldType() to determine the type of each.*/
  std::vector<std::string> GetFieldList() const;
  
  /** Get value of integer field.
   *
   *  @warning Due to the hash table implementation used here, this
   *  method raises an assertion error if @p name is not a valid field.
   *  Always use GetFieldType() to check if field exists if you are
   *  not sure!
   */
  int GetI(const std::string &name) const;

  /** Get value of double field.
   *
   *  @warning Due to the hash table implementation used here, this
   *  method raises an assertion error if @p name is not a valid field.
   *  Always use GetFieldType() to check if field exists if you are
   *  not sure!
   */
  double GetD(const std::string &name) const;

  /** Get value of string field.
   *
   *  @warning Due to the hash table implementation used here, this
   *  method raises an assertion error if @p name is not a valid field.
   *  Always use GetFieldType() to check if field exists if you are
   *  not sure!
   */
  std::string GetS(const std::string &name) const;

  /** Get value of integer array field.
   *
   *  @warning Due to the hash table implementation used here, this
   *  method raises an assertion error if @p name is not a valid field.
   *  Always use GetFieldType() to check if field exists if you are
   *  not sure!
   */
  std::vector<int> GetIArray(const std::string &name) const;

  /** Get value of double array field.
   *
   *  @warning Due to the hash table implementation used here, this
   *  method raises an assertion error if @p name is not a valid field.
   *  Always use GetFieldType() to check if field exists if you are
   *  not sure!
   */
  std::vector<double> GetDArray(const std::string &name) const;

  /** Get value of string array field.
   *
   *  @warning Due to the hash table implementation used here, this
   *  method raises an assertion error if @p name is not a valid field.
   *  Always use GetFieldType() to check if field exists if you are
   *  not sure!
   */
  std::vector<std::string> GetSArray(const std::string &name) const;


  /** Get a JSON value for any field.
  
      This returns a json::Value for any field.  This includes
      arbitrary JSON values that cannot be parsed by RATDB into
      standard C++ types.
  */
  json::Value GetJSON(const std::string &name) const;

  /** Template version of the Get method.
   *
   *  Provided as a convenience to the DBLink implementation.  Fetches
   *  field based on type of @p T.
   */
  template <class T> inline T Get(const std::string &name) const;

  /** Create integer field if does not already exist, and set value. */
  inline void SetI(std::string name, int val) { 
    if (GetFieldType(name) == NOTFOUND)
      bytes += 4;
    table[name] = json::Value(val);
  };

  /** Create double field if does not already exist, and set value. */
  inline void SetD(std::string name, double val) { 
    if (GetFieldType(name) == NOTFOUND)
      bytes += 8;
    table[name] = json::Value(val);
  };
  
  /** Create string field if does not already exist, and set value. */
  inline void SetS(std::string name, std::string val) {
    if (GetFieldType(name) == STRING)
      bytes -= GetS(name).size();
    bytes += val.size();
    table[name] = json::Value(val);
  };

  /** Create integer array field if does not already exist, and set value. 
   *  The array is copied into new storage.
   */
  inline void SetIArray(std::string name, const std::vector<int> &val) {
    if (GetFieldType(name) != NOTFOUND)
      bytes -= 4 * GetIArray(name).size();
    bytes += 4 * val.size();

    json::Value tmpArray(val);
    table[name] = tmpArray;
    arrayTypeCache[name] = INTEGER_ARRAY;
  };

  /** Set a deferred integer array field that will be fetched on demand */
  void SetIArrayDeferred(std::string name, DBFieldCallback *callback) {
    iatbl_deferred[name] = callback;
  };

  /** Create double array field if does not already exist, and set value.
   *  The array is copied into new storage.
   */
  inline void SetDArray(std::string name, const std::vector<double> &val) {
    if (GetFieldType(name) != NOTFOUND)
      bytes -= 8 * GetIArray(name).size();
    bytes += 8 * val.size();
    
    json::Value tmpArray(val);
    table[name] = tmpArray;
    arrayTypeCache[name] = DOUBLE_ARRAY;
  };

  /** Set a deferred double array field that will be fetched on demand */
  void SetDArrayDeferred(std::string name, DBFieldCallback *callback) {
    datbl_deferred[name] = callback;
  };

  /** Create string array field if does not already exist, and set value.
   *  The array is copied into new storage.
   */
  inline void SetSArray(std::string name, const std::vector<std::string> &val) { 
    if (GetFieldType(name) == STRING_ARRAY) {
      std::vector<std::string> sarray = GetSArray(name);
      for (unsigned i=0; i < sarray.size(); i++)
        bytes -= sarray[i].size();
    }
    
    for (unsigned i=0; i < val.size(); i++)
      bytes += val[i].size();
        
    json::Value tmpArray(val);
    table[name] = tmpArray;
    arrayTypeCache[name] = STRING_ARRAY;
  };
  
  /** Add a raw JSON value to the database.  Homogeneous arrays should be 
      added using one of the SetIArray/SetDArray/SetSArray methods or they
      will not be fetchable via GetIArray/GetDArray/GetSArray later. */
  inline void SetJSON(const std::string &name, const json::Value &value) {
    // For now JSON values are exempt from size accounting
    table[name] = value; 
  }
  
  /** Approximate number of bytes used by this table.  Not very accurate, but should be
      roughly proportional. */
  int GetBytes() const { return bytes; };

protected:
  std::string tblname; /**< Name of table */
  std::string index;   /**< Index of table */
  int run_begin;       /**< First run in which this table is valid */
  int run_end;         /**< Last run in which this table is valid */
  int bytes;           /**< Number of bytes required by values.  Approximate */
  
  /** JSON object storage of all fields, except callbacks */
  json::Value table;

  /** Stores the data type of homogeneous arrays to make GetFieldType easier */
  stlplus::hash< std::string, FieldType, pyhash> arrayTypeCache;

  /** Hashtable storage of callbacks for deferred integer array fields. */
  stlplus::hash< std::string, RAT::DBFieldCallback*, pyhash> iatbl_deferred;
  
  /** Hashtable storage of callbacks for deferred integer array fields. */
  stlplus::hash< std::string, RAT::DBFieldCallback*, pyhash> datbl_deferred;
  
};

/* Specialization of the above Get<> template */
template <>
inline int DBTable::Get<int>(const std::string &name) const
{ 
  return GetI(name);
}

template <>
inline double DBTable::Get<double>(const std::string &name) const
{
  return GetD(name);
}

template <>
inline std::string DBTable::Get<std::string>(const std::string &name) const
{
  return GetS(name);
}

template <>
inline std::vector<int>
RAT::DBTable::Get<std::vector<int> >(const std::string &name) const
{
  return GetIArray(name);
}

template <>
inline std::vector<double>
RAT::DBTable::Get<std::vector<double> >(const std::string &name) const
{
  return GetDArray(name);
}

template <>
inline std::vector<std::string>
RAT::DBTable::Get<std::vector<std::string> >(const std::string &name) const
{
  return GetSArray(name); 
}

template <>
inline json::Value
RAT::DBTable::Get<json::Value>(const std::string &name) const
{
  return GetJSON(name); 
}

/** Exception: Field not found in RATDB */
class DBNotFoundError {
public:
  /** Create new exception.
   *
   *  @param _table  Name of table in which field could not be found
   *  @param _index  Index of table in which field could not be found
   *  @param _field  Name of field which could not be found
   */
  DBNotFoundError (const std::string &_table, const std::string &_index, 
		      const std::string &_field) :
    table(_table), index(_index), field(_field) { };

  /** Compare if @p other has the same table, index and field as @p this. */
  bool operator== (const DBNotFoundError &other) const {
    return table == other.table && index == other.index && field == other.field;
  }

  std::string table;
  std::string index;
  std::string field;
};

/** Exception: Field has wrong type in RATDB.
    Inherits from DBNotFoundError to work with existing code that only expects that exception.
 */
class DBWrongTypeError : public DBNotFoundError {
public:
  /** Create new exception.
   *
   *  @param _table  Name of table in which field could not be found with requested type
   *  @param _index  Index of table in which field could not be found with requested type
   *  @param _field  Name of field which could not be found with requested type
   *  @param _requestedType DBTable::FieldType requested
   *  @param _actualType DBTable::FieldType actually present in database
   */
  DBWrongTypeError(const std::string &_table, const std::string &_index,
                   const std::string &_field, RAT::DBTable::FieldType _requestedType,
                   RAT::DBTable::FieldType _actualType)
    : DBNotFoundError(_table, _index, _field), 
      requestedType(_requestedType),
      actualType(_actualType)
      { };

  /** Compare if @p other has the same table, index and field as @p this. */
  bool operator== (const DBWrongTypeError &other) const {
    return table == other.table && index == other.index && field == other.field
           && requestedType == other.requestedType && actualType == other.actualType;
  }
      
  RAT::DBTable::FieldType requestedType;
  RAT::DBTable::FieldType actualType;
};

} // namespace RAT

#endif
