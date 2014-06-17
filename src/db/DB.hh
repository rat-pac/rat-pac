/** @class DB
 *  Database of constants
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 *  RATDB is the database of constants that will be used by RAT for
 *  all adjustable parameters. "Parameters" in this sense includes
 *  many things, such as:
 *
 *   - physical properties of materials (density, optics, etc.)
 *   - geometric configuration of parts of the detector (dimensions and
 *     locations of the acrylic vessel, PMTs, ...)
 *   - calibration constants
 *   - lookup tables
 *   - control parameters which specify how processing should be done
 *
 *  Note that RATDB is NOT intended to hold actual event information,
 *  just everything else.
 *
 *  Despite the suggestive name, RATDB is not really a proper
 *  Database, like MySQL or Oracle. Rather it is just a simple set of
 *  C++ classes used to read parameters from various backends (text
 *  files and CouchDB servers) and make the data easily
 *  available to other parts of the RAT application.
 *
 *  @section Org   Data Organization
 *
 *  Information in RATDB is organized into a two-level namespace of @i
 *  tables and @i fields.  Tables have uppercase names like @c DAQ, @c
 *  @c IBD, and @c PMTLOCATION.  Related tables can be given the same
 *  name and distinguished by an index string like "vacuum" or
 *  "acrylic".  The convention for writing this out is to put the
 *  index in brackets, after the table name: @c GEO[tank], @c
 *  MEDIA[acrylic].  Tables without an index (like @p IBD) implicitly
 *  have an index which is the empty string.
 *
 *  Tables contain fields, which have lowercase names like @p
 *  sampling_time and @p emin.  A field contains value, which can have
 *  one of 6 data types:
 *    - integer, float, string
 *    - array of integers, array of doubles, array of strings
 *
 *  @section VRange   Run Ranges
 *
 *  Frequently, you want to have several sets of the same constants
 *  which have different validity ranges.  RATDB organizes the notion
 *  of validity range into 3 @i planes (think of a stack of transparencies):
 *    - user plane: This sits on top, and is where constants that
 *  users want to override in their macros go.
 *    - run plane: This is where constants that change with time
 *  (like attenuation lengths) go.
 *    - default plane: This is where a baseline set of constants
 *  live.  Fields are only retrieved from the default plane if they
 *  cannot be found on the user plane or the time plane.
 *
 *  The plane on which a table lives is specified by a few special
 *  fields in the table.  The range of run numbers (inclusive) are
 *  contained the field @c run_range.  The special value [0,0] is
 *  used to indicate the default plane and [-1,-1] is used for
 *  the user plane.
 *
 *  @section Usage
 *
 *  To use RATDB, you obtain a pointer to the global database (unless
 *  you want to create your own RATDB with some private data).  Then
 *  you obtain a link to the table you wish to access.  Finally you
 *  query the link for specific fields.  The shortest example of this
 *  is:
@code
DB *db = DB::Get();
DBLinkPtr lmedia = db->GetLink("MEDIA", "acrylic");  // Link to MEDIA[acrylic]
double rindex = lmedia->GetD("index_of_refraction");
@endcode
 *  
 *  You can hold onto DBLinkPtr objects as long as you like.  If
 *  you expect you will need to access the same table at various
 *  points in your code, you can obtain a link pointer in your
 *  constructor and reuse it in other methods.
 *
 */

#ifndef __RAT_DB__
#define __RAT_DB__

#include <string>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <RAT/HTTPDownloader.hh>
#include <RAT/DBLink.hh>
#include <RAT/DBFieldCallback.hh>
#include <RAT/smart_ptr.hpp>

namespace RAT {


class DB; // Forward decl to allow for static DB member inside itself
class DBTable;

class DBTableKey {
public:
  DBTableKey() : name(""), index(""), run(0) {};
  DBTableKey (const std::string &_name, const std::string &_index, const int _run=0)
    : name(_name), index(_index), run(_run) {};
  
  std::string name;
  std::string index;
  int run;
  
  bool operator< (const DBTableKey &rhs) const {
    return (name < rhs.name) || (name == rhs.name && index < rhs.index) 
      || (name == rhs.name && index == rhs.index && run < rhs.run);
  };
  
  bool operator== (const DBTableKey &rhs) const {
    return (name == rhs.name) && (index == rhs.index) && (run == rhs.run);
  };
};

typedef std::map<DBTableKey, simple_ptr_nocopy<DBTable> > DBTableSet;
typedef simple_ptr_nocopy<DBLink> DBLinkPtr;
typedef std::map<std::string, DBLinkPtr> DBLinkGroup;

class DB : public DBFieldCallback {
public:

  /** Obtain pointer to global database
   *
   *  Unless you want a private database, this is the method you
   *  should use to obtain a pointer to DB.  If the global database
   *  has not yet been constructed, it will be constructed when you
   *  call this function.
  */
  static inline DB *Get()
      { return primary == 0 ? primary = new DB : primary; };

  /** Parse a table name written in the "TABLE" or "TABLE[index]" format.
   *
   *  Handy method for when you are getting a table name from user
   *  input.  Separates the table name from the index name.
   *
   *  @param[in]  descriptor  String in the form "TABLE" or
   *  "TABLE[index]"
   *  @param[out] table       Just the table name
   *  @param[out] index       Just the index name.  Empty string if no
   *  index.
   * 
   *  @returns True if @p descriptor was a properly formatted table name.
   */
  static bool ParseTableName(std::string descriptor, 
			     std::string &table, std::string &index);
			     
	/** Reads a file and returns all the RATDB tables found inside.
	 *  Works with both RATDB native and JSON files */
  static std::vector<RAT::DBTable *> ReadRATDBFile(const std::string &filename);

protected:

  /** Pointer to global DB instance */
  static DB *primary;

public:
  /** Create new database in memory with no tables. */
  DB();
  virtual ~DB();

  /** Load a DB text file or directory of text files in search path.
   *
   *  If passed the name of a file, this function will load all the
   *  DB tables it contains into memory.  If passed the name of a
   *  directory, all of the files in that directory ending in .ratdb
   *  will be loaded.
   *
   *  The current directory is searched first, then the $GLG4DATA
   *  directory.
   *
   *  If printFullPath is true, then an info message is printed
   *  to stdout (and logged) indicating the path of the
   *  file that is loaded.  Shows whether $GLG4DATA was used.
   */
  int Load(std::string filename, bool printPath=false);

  /** Put a DBTable into the database.  Returns 1 if success and 0 if failure. 
   The DB class will take over responsibility for freeing the table memory
   when needed. */
  int LoadTable(DBTable *table);

  /** Load DB text file of tables into memory. 
   *
   *  This function does not search in $GLG4DATA automatically, so it
   *  must be given a file name relative to the current directory or
   *  an absolute path.
   */
  int LoadFile(std::string filename);

  /** Load all of the DB files in a directory.
   *
   *  Searches @p dirname for all files matching @p pattern.  Does not
   *  search $GLG4DATA automatically, so a path relative to current
   *  directory or an absolute path much be given.
   */
  int LoadAll(std::string dirname, std::string pattern="*.ratdb");

  /** Load standard tables into memory.
   *
   *  Currently, the standard tables are $GLG4DATA/ *.ratdb. 
   */
  int LoadDefaults();

  /** Delete all tables from memory. */
  void Clear();

  /** Set URL of CouchDB server containing RATDB tables */
  void SetServer(std::string url);

  /** Set default run number for new links. **/
  void SetDefaultRun(int _run);

  /** Get the default run number used for new links **/
  int GetDefaultRun() const;

  /** Obtain a link to a particular table and index with default run. 
   *
   *  Note that even if your table does not have an index value,
   *  you should still use this method, but with an empty index.
   *  (The default value)
   */
  DBLinkPtr GetLink(std::string tblname, std::string index="");

  /** Obtain a link to a particular table and index with given run. 
   *
   *  If you have an empty index, pass "" as the index value.
   */
  DBLinkPtr GetLink(std::string tblname, std::string index, int run);


  /** Obtain a link to all tables with the same name, but different
   *  indexes.
   * 
   *  LinkGroups are useful if you want to step through all the
   *  instances of the same kind of table, like GEO.
   */
  DBLinkGroup GetLinkGroup(std::string tblname);

  // Manually set fields on the user plane.  Also for user use.
  // DO THIS AFTER loading files from disk, or your changes will be
  // stomped on later.

  /** Set integer field in user plane, no table index. */
  void SetI(std::string tblname, std::string fieldname, int val)
  { SetI(tblname, "", fieldname, val); };

  /** Set integer field in user plane, with table index. */
  void SetI(std::string tblname, std::string index, std::string fieldname, int val);

  /** Set double field in user plane, no table index. */
  void SetD(std::string tblname, std::string fieldname, double val)
  { SetD(tblname, "", fieldname, val); };
  /** Set double field in user plane, with table index. */
  void SetD(std::string tblname, std::string index, std::string fieldname, double val);

  /** Set string field in user plane, no table index. */
  void SetS(std::string tblname, std::string fieldname, std::string val)
  { SetS(tblname, "", fieldname, val); };
  /** Set string field in user plane, with table index. */
  void SetS(std::string tblname, std::string index, std::string fieldname, std::string val);
  

  /** Set integer array field in user plane, no table index. */
  void SetIArray(std::string tblname, std::string fieldname, const std::vector<int> &val)
  { SetIArray(tblname, "", fieldname, val); };
  /** Set integer array field in user plane, with table index. */
  void SetIArray(std::string tblname, std::string index, std::string fieldname, const std::vector<int> &val);

  /** Set double array field in user plane, no table index. */
  void SetDArray(std::string tblname, std::string fieldname, const std::vector<double> &val)
  { SetDArray(tblname, "", fieldname, val); };
  /** Set double array field in user plane, with table index. */
  void SetDArray(std::string tblname, std::string index, std::string fieldname, const std::vector<double> &val);

  /** Set string array field in user plane, no table index. */
  void SetSArray(std::string tblname, std::string fieldname, const std::vector<std::string> &val)
  { SetSArray(tblname, "", fieldname, val); };
  /** Set string array field in user plane, with table index. */
  void SetSArray(std::string tblname, std::string index, std::string fieldname, const std::vector<std::string> &val);
  

  /************************DBLink interface********************/
  // This is the low level interface that DBLinks use.
  // You should not call these methods.

  /** Get pointer to a table in user plane: do not use unless you know
      what you are doing! */
  DBTable *GetUserTable(std::string tblname, std::string index)
      { return FindTable(tblname, index, -1); };

  /** Get pointer to a table in run plane: do not use unless you know
      what you are doing! */
  DBTable *GetRunTable(std::string tblname, std::string index, int runNumber)
      { return FindTable(tblname, index, runNumber); };
      
  /** Get pointer to a table in default plane: do not use unless you know
      what you are doing! */
  DBTable *GetDefaultTable(std::string tblname, std::string index)
      { return FindTable(tblname, index, 0); };

  /** Remove a table link from the list of outstanding links: do not
   *  use unless you know what you are doing!
   *
   *  This method should only be used by DBLink objects to remove
   *  themselves from the list during their destructor call.  See
   *  DB::links member for more information.
   */
  void RemoveLink(DBLink *link);

  /** Number of still active links to this database.
   *
   *  This is a count of the number of links which have been given out
   *  GetLink() and GetLinkGroup().  See DB::links member for more
   *  information about what this means.  This method is really just
   *  for debugging purposes.
   */
  int NumLinks() { return links.size(); };
  
  virtual std::vector<int> FetchIArray(const std::string &tableID, const std::string &fieldname);
  virtual std::vector<double> FetchDArray(const std::string &tableID, const std::string &fieldname);
  
protected:
  /** Obtain a pointer to a table loaded in memory. 
   *
   *  @returns Pointer to table if found, otherwise 0.
   */
  DBTable *FindTable(std::string tblname, std::string index, int runNumber);

  /** Obtain a pointer to a table in memory, or create it if
   *  not found.
   * 
   *  @returns Pointer to existing table, or pointer to new table,
   *  which is also added to @p tblset.
   */
  DBTable *FindOrCreateTable(std::string tblname, std::string index, int runNumber);

  /** List of DBLinks which have been issued by GetLink() or
   *  GetLinkGroup().
   *
   *  In order to allow a DBLink to do local caching of values,
   *  the DB object must retain pointers to all of the previously
   *  issued link objects.  Then, if something changes, like new
   *  tables are loaded or become valid, DB can go out and expire
   *  the relevant cached values inside the affected DBLinks.
   *
   *  In the DBLink destructor, RemoveLink() is called to remove
   *  the link from the list before its memory is reclaimed.
   */
  std::list<RAT::DBLink *> links;

  /** URL to CouchDB server.  Empty string means no server will be
   *  used. */
  std::string server;
  
  /** Cache of table names present on the CouchDB server.  Check this before issuing a query. */
  std::set<std::string> tableNamesOnServer;
  
  /** Cache of full table keys (name, index, run) that are *not* on the server.
      Important to cut down on needless network traffic from future queries once
      a rejection has been received. */
  std::set<RAT::DBTableKey> tablesNotOnServer;
  
  /** Helper class to download files over HTTP/HTTPS */
  HTTPDownloader downloader;

  /** Set of all tables.  Run 0 signifies default and run -1 is user-override. */
  DBTableSet tables;
  
  /** FIFO of tables fetched from the server.  Second element of pair is
      indicates if the size from this key "is real".  When the same table
      is added for multiple runs, only the last run should be set to true. */
  std::deque< std::pair<RAT::DBTableKey, bool> > tablesFromServer;

  /** Current run number, used to set all new DBLinks. */
  int run;
};

} // namespace RAT

#endif
