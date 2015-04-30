#include <RAT/DB.hh>
#include <RAT/DBTable.hh>
#include <RAT/DBTextLoader.hh>
#include <RAT/DBJsonLoader.hh>
#include <iostream>
#include <sstream>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <RAT/Log.hh>
#include <RAT/json_reader.hh>
#include <Byteswap.h> // From ROOT 
#include <TPython.h>

using namespace std;

namespace RAT {

DB *DB::primary(0);

const int maxServerBytes = 200000000; // 200 MB

DB::DB() : server(""), run(1)
{
  // Nothing to do
}

DB::~DB()
{
  // Smart pointers will free tables automatically, nothing needed here anymore
  
  // Since this database is going away, disable all the existing links.
  // Prevents links from trying to remove themselves from nonexistent
  // databases
  list<DBLink *>::iterator l;
  for (l = links.begin(); l != links.end(); l++)
    (*l)->Unlink();
}

int DB::Load(std::string filename, bool printPath)
{
  // Try to get file info assuming the name is literal
  struct stat s;
  if (stat(filename.c_str(), &s) == 0) {
    if (printPath)
      cout << "DB: Loading " << filename << "\n";
    if (S_ISDIR(s.st_mode))
      return LoadAll(filename);
    else
      return LoadFile(filename);
  } else {
    char *glg4data = getenv("GLG4DATA");
    if (glg4data != 0) {
      string newfilename = string(glg4data) + "/" + filename;
      if (printPath)
        cout << "DB: Loading " << newfilename << "\n";
      if (stat(newfilename.c_str(), &s) == 0) {
        if (S_ISDIR(s.st_mode))
  	     return LoadAll(newfilename);
        else
  	     return LoadFile(newfilename);
      }
    }

    Log::Die("DB: Cannot open " + filename + ".\nIs $GLG4DATA set?");
  }

  Log::Die("DB: Cannot open " + filename);
  return 0; // Never get here
}

int DB::LoadTable(DBTable *table) 
{
  DBTableKey id(table->GetName(), table->GetIndex());

  if (table->IsDefault()) {
    id.run = 0; // default signifier
    
    DBTable *oldtable = FindTable(id.name, id.index, id.run);
    if (oldtable) {
      cerr << "DB: Replacing default " << id.name << "[" << id.index
	   << "] in database.\n"
	   << endl;
    }
    
    tables[id] = table;
  } else if (table->IsUser()) {
    id.run = -1;
    
    DBTable *oldtable = FindTable(id.name, id.index, id.run);
    if (oldtable) {
      cerr << "DB: Replacing user " << id.name << "[" << id.index
	   << "] in database.\n"
	   << endl;
    }
    
    tables[id] = table;
  }
  else { /* Actual run range */
    // Explicitly create smart pointer here to ensure that
    // storage is shared among all copies.
    simple_ptr_nocopy<DBTable> tablePtr(table);
    
    // Insert a pointer for each valid run
    for (int i=table->GetRunBegin(); i <= table->GetRunEnd(); i++) {
      id.run = i;
      
      DBTable *oldtable = FindTable(id.name, id.index, id.run);
      if (oldtable) {
	cerr << "DB: Replacing " << id.name << "[" << id.index << "]" << " for run " << id.run
	     << " in database.\n";
      }
      
      tables[id] = tablePtr;
    }
      
  }
  
  return 1;
}

int DB::LoadFile(std::string filename)
{
  // If this is a python file, run it to load whatever tables it contains
  if (filename.size() > 3 && filename.substr(filename.size() - 3, 3) == ".py") {
    TPython::ExecScript(filename.c_str());
    return 1;
  }

  // Otherwise try to read and parse it as RATDB or JSON file.
  vector<DBTable *> contents;
  try {
    contents = ReadRATDBFile(filename);
  } catch (FileError &e) {
    cerr << "DB: Error! Cannot open " << e.filename << "\n";
    return 0;
  }

  // NOTE: Smart pointers in tables mean that we do not need to 
  // explicitly delete old table before replacing with new one below!

  vector<DBTable *>::iterator itbl;

  for (itbl=contents.begin(); itbl != contents.end(); itbl++) {
    DBTable *table = (*itbl);  // Get rid of iterator mess
    LoadTable(table);
  }
      
  return 1;
}

int DB::LoadAll(std::string dirname, std::string pattern)
{
  pattern = dirname + "/" + pattern;
  glob_t g;

  if (glob(pattern.c_str(), 0, 0, &g) == 0) {
    for (unsigned i=0; i < g.gl_pathc; i++) {
      string path(g.gl_pathv[i]);
      cout << "DB: Loading " << path << " ... ";
      if (Load(path))
	cout << "Success!" << endl;
      else {
	cout << "Load Failed!" << endl;
	globfree(&g);
	return 0;
      }
    }
  }

  globfree(&g);
  return 1;
}

int DB::LoadDefaults()
{
  if ( getenv("GLG4DATA") != NULL )
    return LoadAll(string(getenv("GLG4DATA")));
  else
    return LoadAll(string("data"));
}


void DB::Clear()
{
  tables.clear();
}

void DB::SetServer(std::string url)
{
  server = url;
  
  tableNamesOnServer.clear();
  
  // Fetch list of names of tables on the server so we can avoid querying it for tables it does not store
  std::string name_url = dformat("%s/_design/ratdb/_view/name?group=true", server.c_str());
  std::string contents = downloader.Fetch(name_url);
  Json::Reader reader;
  Json::Value results;
  Log::Assert(reader.parse(contents, results, false), 
              "RATDB: Could not parse JSON response of table name query to server.");
  
  info << "RATDB: Tables on server include";
  Json::Value rows = results["rows"];
  for (unsigned i=0; i < rows.size(); i++) {
    std::string key = rows[i]["key"].asString();
    tableNamesOnServer.insert(key);
    info << " " << key;
  }
  
  info << "\n";
}

void DB::SetDefaultRun(int _run)
{
  run = _run;
}

int DB::GetDefaultRun() const
{
  return run;
}

DBLinkPtr DB::GetLink(std::string tblname, std::string index)
{
  return DB::GetLink(tblname, index, this->run);
}


DBLinkPtr DB::GetLink(std::string tblname, std::string index, int _run)
{
  debug << dformat("DB::GetLink(%s,%s, run=%d)\n", tblname.c_str(), index.c_str(), _run);

  // By using smart pointer here, user does not have to worry about
  // memory management
  DBLink *real_ptr = new DBLink(this, tblname, index, _run);
  links.push_back(real_ptr);
  return DBLinkPtr(real_ptr);
}


DBLinkGroup DB::GetLinkGroup(std::string tblname)
{
  DBTableSet::iterator i;
  DBLinkGroup group;

  // To figure out which tables exist, we have to scan all available
  // planes
  for (i = tables.begin(); i != tables.end(); i++) {
    DBTableKey id = i->first;
    if (id.name == tblname)
      group[id.index] = GetLink(tblname, id.index);  
  }
  
  // Also include tables from the server if they are present
  if (tableNamesOnServer.count(tblname) != 0) {
    // Fetch the range of keys corresponding to just this table name
    std::string url = dformat("%s/_design/ratdb/_view/linkgroup?group=true&startkey=[%%22%s%%22,%%22%%22]&endkey=[%%22%s%%22,{}]",
                              server.c_str(), tblname.c_str(), tblname.c_str());
    std::string contents = downloader.Fetch(url);

    Json::Reader reader;
    Json::Value results;
    Log::Assert(reader.parse(contents, results, false),
                "RATDB: Could not parse JSON response when building DBLink group.");
    Json::Value rows = results["rows"];
    for (unsigned idx=0; idx < rows.size(); idx++) {
      Json::Value row = rows[idx]["key"];
      // Key is a two element array, with index in entry 1
      std::string index = row[1].asString();
      if (group.count(index) == 0)
        group[index] = GetLink(tblname, index);
    }
  }

  return group;
}

std::vector<double> unpack_double(const std::string &contents)
{
  Log::Assert(contents.size() % sizeof(double) == 0, "Incorrectly size double array from CouchDB server");
  unsigned len = contents.size()/sizeof(double);
  const char *bytes = contents.c_str();
  std::vector<double> array(len);
  
  for (unsigned i=0; i < len; i++) {
    memcpy(&array[i], bytes + i * sizeof(double), sizeof(double));

  // ROOT defines R__BYTESWAP when platform is little-endian.  Since that is server byte-ordering
  // we swap when it is NOT defined
#ifndef R__BYTESWAP
    array[i] = Rbswap_64(array[i]);
#endif
  }
    
  return array;
}

std::vector<int> unpack_int(const std::string &contents)
{
  Log::Assert(contents.size() % sizeof(int) == 0, "Incorrectly size int array from CouchDB server");
  unsigned len = contents.size()/sizeof(int);
  const char *bytes = contents.c_str();
  std::vector<int> array(len);
  
  for (unsigned i=0; i < len; i++) {
    memcpy(&array[i], bytes + i * sizeof(int), sizeof(int));
    // ROOT defines R__BYTESWAP when platform is little-endian.  Since that is server byte-ordering
    // we swap when it is NOT defined
#ifndef R__BYTESWAP
    array[i] = Rbswap_32(array[i]);
#endif
  }
      
  return array;
}

std::vector<int> DB::FetchIArray(const std::string &tableID, const std::string &fieldname)
{
  std::string url = dformat("%s/%s/%s", server.c_str(), tableID.c_str(), fieldname.c_str());
  std::string contents = downloader.Fetch(url);
  return unpack_int(contents);
}

std::vector<double> DB::FetchDArray(const std::string &tableID, const std::string &fieldname)
{
  std::string url = dformat("%s/%s/%s", server.c_str(), tableID.c_str(), fieldname.c_str());
  std::string contents = downloader.Fetch(url);
  return unpack_double(contents);
}

DBTable *DB::FindTable(std::string tblname, std::string index, int runNumber)
{
  // First check local cache of tables
  DBTableKey id(tblname, index, runNumber);
  DBTableSet::iterator table = tables.find(id);

  if (table != tables.end())
    return (*table).second.pointer(); // Found in cache, return now
  
  // Return if the server does not have tables with this name
  // or if no server has been specified (the set will be empty)
  if (tableNamesOnServer.count(tblname) == 0)
    return 0;
    
  // Return if we already asked for this table from the server and
  // it did not have it
  if (tablesNotOnServer.count(id) > 0)
    return 0;
    
  // 1) Look for table on server
  std::string url = dformat("%s/_design/ratdb/_view/select?key=[%%22%s%%22,%%22%s%%22,%d]&include_docs=true",
                            server.c_str(), tblname.c_str(), index.c_str(),
                            runNumber);
  std::string contents = downloader.Fetch(url);
  
  // 2) Parse JSON document
  Json::Reader reader;
  Json::Value query;
  //warn << contents.str();
  Log::Assert(reader.parse(contents, query, false), 
              "RATDB: Could not parse JSON response from server.");
              
  // 3) Copy the fields into a RATDB table
  Log::Assert(query.isObject(), "RATDB:: Server returned non-object JSON document.");
  
  if (query["rows"].size() == 0) { // Did not find any matching tables on the server
    tablesNotOnServer.insert(id); // Remember this result for later
    return 0;
  }
    
  if (query["rows"].size() > 1) // Multiple documents?
    Log::Die("RATDB: Multiple documents on server for this run.  Aborting!");
    
  Json::Value jsonDoc = query["rows"][(unsigned) 0]["doc"];
  
  DBTable *newTable = DBJsonLoader::convertTable(jsonDoc);
  
  // 4) Grab fields from attachments if present
  if (jsonDoc.isMember("_attachments")) {
    Json::Value attachments = jsonDoc["_attachments"];
    std::vector<std::string> fieldnames = attachments.getMemberNames();
    for (unsigned idx=0; idx < fieldnames.size(); idx++) {
      // Fetch attachment
      const std::string &fieldname = fieldnames[idx];
      std::string content_type = attachments[fieldname]["content_type"].asString();
            
      if (content_type == "vnd.rat/array-double") {
        newTable->SetDArrayDeferred(fieldname, this);
      } else if (content_type == "vnd.rat/array-int") {
        newTable->SetIArrayDeferred(fieldname, this);
      } else {
        Log::Die(dformat("RATDB: Unknown attachment type %s for field %s", content_type.c_str(), fieldname.c_str()));
      }
    }
  }
  
  // 5) Flush out some tables if we need the space
  int newTableBytes = newTable->GetBytes();
  int serverTableBytes = 0;
  for (unsigned i=0; i < tablesFromServer.size(); i++) {
    if (tablesFromServer[i].second)
      serverTableBytes += tables[tablesFromServer[i].first]->GetBytes();
  }
  
  while (tablesFromServer.size() > 0 && serverTableBytes + newTableBytes > maxServerBytes) {
    std::pair<DBTableKey, bool> forDeletion = tablesFromServer.front();
    tablesFromServer.pop_front();
    int deleteSize = tables[forDeletion.first]->GetBytes();
    if (forDeletion.second)
      serverTableBytes -= deleteSize;
    tables.erase(forDeletion.first);
    cerr << "Evicting " << forDeletion.first.name << "[" << forDeletion.first.index << "], run " << forDeletion.first.run << " (" << deleteSize << " bytes)\n";
  }
    
  // 6) Add this table to the memory cache
  simple_ptr_nocopy<DBTable> newTablePtr(newTable);
  const vector<int> &run_range = newTable->GetIArray("run_range");
  for (int i=run_range[0]; i <= run_range[1]; i++) {
    id.run = i;
    tables[id] = newTablePtr;
    std::pair<DBTableKey, bool> sizeRecord(id, false);
    tablesFromServer.push_back(sizeRecord);
  }
  
  // Last entry in queue gets billed for all the space (since it isn't freed until the last is deleted)
  tablesFromServer.back().second = true;
  //cerr << "Memory cache = " << serverTableBytes << "\n";
  
  info << dformat("RATDB: Loaded table %s[%s] from server\n", newTable->GetName().c_str(),
  newTable->GetIndex().c_str());

  return newTable;
}

void DB::SetI(std::string tblname, std::string index, std::string fieldname, int val)
{
  DBTable *t = FindOrCreateTable(tblname, index, -1);
  t->SetI(fieldname, val);
}

void DB::SetD(std::string tblname, std::string index, std::string fieldname, double val)
{
  DBTable *t = FindOrCreateTable(tblname, index, -1);
  t->SetD(fieldname, val);
}


void DB::SetS(std::string tblname, std::string index, std::string fieldname, std::string val)
{
  DBTable *t = FindOrCreateTable(tblname, index, -1);
  t->SetS(fieldname, val);
}

  
void DB::SetIArray(std::string tblname, std::string index, std::string fieldname, const std::vector<int> &val)
{
  DBTable *t = FindOrCreateTable(tblname, index, -1);
  t->SetIArray(fieldname, val);
}

void DB::SetDArray(std::string tblname, std::string index, std::string fieldname, const std::vector<double> &val)
{
  DBTable *t = FindOrCreateTable(tblname, index, -1);
  t->SetDArray(fieldname, val);
}

void DB::SetSArray(std::string tblname, std::string index, std::string fieldname, const std::vector<std::string> &val)
{
  DBTable *t = FindOrCreateTable(tblname, index, -1);
  t->SetSArray(fieldname, val);
}


DBTable *DB::FindOrCreateTable(std::string tblname, std::string index, int runNumber)
{
  DBTable *table = FindTable(tblname, index, runNumber);

  if (table)
    return table;

  DBTableKey id(tblname, index, -1);
  table = new DBTable(tblname, index);
  table->SetUser();

  tables[id] = table;

  return table;
}

void DB::RemoveLink(DBLink *link)
{
  links.remove(link);
}


bool DB::ParseTableName(std::string descriptor,
			std::string &table, std::string &index)
{
  Tokenizer t(descriptor);

  try {
    // Get table name
    if (t.Next() != Tokenizer::TYPE_IDENTIFIER)
      return 0;

    table = t.Token();

    // Check for index in brackets
    Tokenizer::Type toktype = t.Next();
    if (toktype == Tokenizer::TYPE_EOF) {
      // No index, assume 0
      index = "";
      return 1;
    } else if (toktype == Tokenizer::TYPE_SYMBOL && t.Token() == "[") {
      // Look for index in brackets
      if (t.Next() == Tokenizer::TYPE_IDENTIFIER) {
        index = t.Token();

        // Make sure we got the rest
        if (t.Next() == Tokenizer::TYPE_SYMBOL && t.Token() == "]"
            && t.Next() == Tokenizer::TYPE_EOF)
          return 1;
        else
          return 0;
      } else
        return 0;
    } else
      return 0;
  } catch (ParseError &p) {
    return 0;
  }
}

std::vector<DBTable *> DB::ReadRATDBFile(const std::string &filename)
{
  std::vector<DBTable *> contents;
  
  try {
    contents = DBTextLoader::parse(filename);
  } catch (ProbablyJSONParseError &e) {
    contents = DBJsonLoader::parse(filename);
  }
  
  return contents;
}


} // namespace RAT
