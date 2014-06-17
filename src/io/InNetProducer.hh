#ifndef __RAT_InNetProducer__
#define __RAT_InNetProducer__

#include <RAT/Producer.hh>
#include <globals.hh>

class G4UIcmdWithAnInteger;

namespace RAT {


class InNetProducer : public Producer {
public:
  InNetProducer();
  InNetProducer(ProcBlock *block);
  virtual ~InNetProducer();

  virtual bool Listen(int port, int event_limit);

  // override G4UImessenger (from Producer) methods
  virtual G4String GetCurrentValue(G4UIcommand * command);
  virtual void SetNewValue(G4UIcommand * command,G4String newValue);


protected:
  void Init();

  G4UIcmdWithAnInteger *fListenCmd;
};


} // namespace RAT

#endif
