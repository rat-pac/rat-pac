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
		   INTEGER, DOUBLE, BOOLEAN, STRING,
		   INTEGER_ARRAY, DOUBLE_ARRAY, BOOLEAN_ARRAY, STRING_ARRAY,
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

  /** Get value of bool field.
   *
   *  @warning Due to the hash table implementation used here, this
   *  method raises an assertion error if @p name is not a valid field.
   *  Always use GetFieldType() to check if field exists if you are
   *  not sure!
   */
  bool GetZ(const std::string &name) const;


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

  /** Get value of bool array field.
   *
   *  @warning Due to the hash table implementation used here, this
   *  method raises an assertion error if @p name is not a valid field.
   *  Always use GetFieldType() to check if field exists if you are
   *  not sure!
   */
  std::vector<bool> GetZArray(const std::string &name) const;


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
  template <typename T> inline T Get(const std::string &name) const;
  
  /** Set any type that is convertable to a json::Value */ 
  template <typename T> inline void Set(const std::string &name, const T &value) {
    table[name] = value; 
  } 

  /** Set a deferred integer array field that will be fetched on demand */
  void SetIArrayDeferred(std::string name, DBFieldCallback *callback) {
    iatbl_deferred[name] = callback;
  };

  /** Set a deferred double array field that will be fetched on demand */
  void SetDArrayDeferred(std::string name, DBFieldCallback *callback) {
    datbl_deferred[name] = callback;
  };

protected:
  std::string tblname; /**< Name of table */
  std::string index;   /**< Index of table */
  int run_begin;       /**< First run in which this table is valid */
  int run_end;         /**< Last run in which this table is valid */
  
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
inline bool RAT::DBTable::Get<bool>(const std::string &name) const
{
  return GetZ(name);
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
inline std::vector<bool>
RAT::DBTable::Get<std::vector<bool> >(const std::string &name) const
{
  return GetZArray(name); 
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
