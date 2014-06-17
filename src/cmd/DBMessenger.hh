#ifndef __RAT_DBMessenger__
#define __RAT_DBMessenger__

#include <G4UImessenger.hh>
#include <G4UIcmdWithAString.hh>
#include <G4UIcmdWithAnInteger.hh>
#include <G4UIcommand.hh>
#include <G4String.hh>
#include <string>
#include <RAT/DB.hh>

namespace RAT {

class DBMessenger : public G4UImessenger {
public:
  DBMessenger() { Init(DB::Get()); };
  DBMessenger(DB *dbToUse) { Init(dbToUse); };
  ~DBMessenger();

  G4String GetCurrentValue(G4UIcommand * command);
  void SetNewValue(G4UIcommand * command, G4String newValue);

  void Load(std::string filename);
  void Set(std::string tbl_descriptor, std::string field, std::string value);
  void Server(std::string url);
  void Run(int run);
  
protected:
  void Init(DB *dbToUse);

  DB *db;
  G4UIcmdWithAString *loadCmd;
  G4UIcommand *setCmd;
  G4UIcmdWithAString *serverCmd;
  G4UIcmdWithAnInteger *runCmd;
};


} // namespace RAT

#endif
