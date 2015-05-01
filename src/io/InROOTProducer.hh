#ifndef __RAT_InROOTProducer__
#define __RAT_InROOTProducer__

#include <string>
#include <RAT/Producer.hh>
#include <globals.hh>

class G4UIcmdWithAString;
class G4UIcommand;

namespace RAT {


class InROOTProducer : public Producer {
public:
  InROOTProducer();
  InROOTProducer(ProcBlock *block);
  virtual ~InROOTProducer();

  virtual bool ReadEvents(G4String filename);

  // override G4UImessenger (from Producer) methods
  virtual G4String GetCurrentValue(G4UIcommand * command);
  virtual void SetNewValue(G4UIcommand * command,G4String newValue);


protected:
  void Init();

  G4UIcmdWithAString *readCmd;
  G4UIcommand *readDefaultCmd;
};

} // namespace RAT

#endif
