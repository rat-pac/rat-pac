#include <RAT/Log.hh>
#include <RAT/DBMessenger.hh>
#include <RAT/DB.hh>
#include <RAT/DBTextLoader.hh>
#include <G4UIdirectory.hh>
#include <G4StateManager.hh>

using namespace std;

namespace RAT {


void DBMessenger::Init(DB *dbToUse)
{
  db = dbToUse;
  // Build UI commands
  G4UIdirectory* dbdir = new G4UIdirectory("/rat/db/");
  dbdir->SetGuidance("RATDB commands");
  
  // load database file command
  loadCmd = new G4UIcmdWithAString("/rat/db/load", this);
  loadCmd->SetParameterName("filename", false); // required
  loadCmd->SetGuidance("Load a file into the database");

  // set database attribute
  setCmd = new G4UIcommand("/rat/db/set", this);
  G4UIparameter *aParam;
  aParam = new G4UIparameter("table", 's', false); //required
  setCmd->SetParameter(aParam);
  aParam = new G4UIparameter("field", 's', false); //required
  setCmd->SetParameter(aParam);
  aParam = new G4UIparameter("newvalue", 's', false); //required
  setCmd->SetParameter(aParam);
  
  // connect to server command
  serverCmd = new G4UIcmdWithAString("/rat/db/server", this);
  serverCmd->SetParameterName("server_url", false); // required
  serverCmd->SetGuidance("Connect to CouchDB database");

  // set default run in database
  runCmd = new G4UIcmdWithAnInteger("/rat/db/run", this);
  runCmd->SetParameterName("run", false); // required
  runCmd->SetGuidance("Set the run number to use when loading database constants");
}

DBMessenger::~DBMessenger()
{
  delete loadCmd;
  delete setCmd;
  delete runCmd;
}

G4String DBMessenger::GetCurrentValue(G4UIcommand * /*unused*/)
{
  warn << "Get current value not implemented\n";
  return "";
}


void DBMessenger::SetNewValue(G4UIcommand * command, G4String newValue)
{
  G4ApplicationState state = G4StateManager::GetStateManager()->GetCurrentState();
  if (state != G4State_PreInit)
    Log::Die("Error: Cannot call " + command->GetCommandPath()  
	     + " after /run/initialize.");

  if (command == loadCmd) {
    Load(newValue);
  } else if (command == setCmd) {
    istringstream args(newValue);
    string tbl_descriptor, field, value;
    args >> tbl_descriptor >> field >> value;
    
    Set(tbl_descriptor, field, value);
  } else if (command == serverCmd) {
    Server(newValue);
  } else if (command == runCmd) {
    int run = G4UIcmdWithAnInteger::GetNewIntValue(newValue.c_str());
    if (run > 0)
      Run(run);
    else
      Log::Die("/rat/db/run requires run number greater than 0.");
  } else
    warn << "Invalid DB command\n";
}


void DBMessenger::Load(std::string filename)
{
  db->Load(filename, true /*printPath*/);
}

void DBMessenger::Set(std::string tbl_descriptor, std::string field, std::string val)
{
  string table;
  string index;

  if (DB::ParseTableName(tbl_descriptor, table, index)) {
    // Just need to figure out value now
    json::Reader reader(val);
    json::Value jval;
    reader.getValue(jval); //will throw parser_error and abort if malformed
    
    if (field[field.length()-1] == ']') {
        size_t idx = atoi(field.substr(field.rfind("[")+1).c_str());
        switch (jval.getType()) {
            case json::TINTEGER:
            case json::TUINTEGER:
            case json::TREAL:
            case json::TBOOL:
            case json::TSTRING:
            case json::TOBJECT:
            case json::TARRAY:
                DB::Get()->SetArrayIndex(table,index,field,idx,jval);
                break;
            case json::TNULL:
                Log::Die("DB: Null value not supported in RATDB");
        }
    } else {
        switch (jval.getType()) {
            case json::TINTEGER:
            case json::TUINTEGER:
            case json::TREAL:
            case json::TBOOL:
            case json::TSTRING:
            case json::TOBJECT:
            case json::TARRAY:
                DB::Get()->Set(table,index,field,jval);
                break;
            case json::TNULL:
                Log::Die("DB: Null value not supported in RATDB");
        }
    }
    info << "DB: Setting " << table << "[" << index << "]" << " " << field << " to " << val << newline;
  } else {
    Log::Die("Malformed table name: " + tbl_descriptor);
  }
}

void DBMessenger::Server(std::string url)
{
  info << "RATDB: Connecting to CouchDB server at " << url << "\n";
  
  db->SetServer(url);
}

void DBMessenger::Run(int run)
{
  info << "Setting run number: " << run << newline;
  db->SetDefaultRun(run);
}

} // namespace RAT
