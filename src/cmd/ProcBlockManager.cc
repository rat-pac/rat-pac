#include <RAT/ProcBlockManager.hh>
#include <RAT/ProcBlock.hh>
#include <RAT/DBTextLoader.hh>
#include <RAT/Log.hh>

#include <globals.hh>
#include <G4UIdirectory.hh>
#include <G4UIparameter.hh>
#include <G4UIcommand.hh>

// Processors
#include <RAT/PythonProc.hh>
#include <RAT/CountProc.hh>
#include <RAT/OutROOTProc.hh>
#include <RAT/OutNetProc.hh>
#include <RAT/PruneProc.hh>
#include <RAT/FitCentroidProc.hh>
#include <RAT/FitPathProc.hh>
#include <RAT/SimpleDAQProc.hh>

namespace RAT {

// Helper func defined in ConstructUserProc.cc and overridden by user
Processor *construct_user_proc(std::string userProcName);

ProcBlockManager::ProcBlockManager(ProcBlock *theMainBlock)
{
  lastProc = 0;

  // Build UI commands
  G4UIdirectory* DebugDir = new G4UIdirectory("/rat/");
  DebugDir->SetGuidance(" control commands");
  
  G4UIparameter *aParam;
  
  // add processor command
  procCmd = new G4UIcommand("/rat/proc", this);
  procCmd->SetGuidance("Add a processor to the analysis stack");
  aParam = new G4UIparameter("procname", 's', false);  // required
  procCmd->SetParameter(aParam);

  // add processor to the end
  procLastCmd = new G4UIcommand("/rat/proclast", this);
  procLastCmd->SetGuidance("Add a processor to the end of the analysis stack, after command-line processors");
  aParam = new G4UIparameter("procname", 's', false);  // required
  procLastCmd->SetParameter(aParam);

  // set proc parameter command
  setCmd = new G4UIcommand("/rat/procset", this);
  procCmd->SetGuidance("Set parameter for most recent processor");
  aParam = new G4UIparameter("param", 's', false); //required
  setCmd->SetParameter(aParam);
  aParam = new G4UIparameter("newvalue", 's', false); //required
  setCmd->SetParameter(aParam);

  // ----------------Create processor allocator table-----------------

  // I/O
  procAllocators["outroot"] = new ProcAllocatorTmpl<OutROOTProc>;
  procAllocators["outnet"] = new ProcAllocatorTmpl<OutNetProc>;

  // Fitters
  procAllocators["fitcentroid"] = new ProcAllocatorTmpl<FitCentroidProc>;
  procAllocators["fitpath"] = new ProcAllocatorTmpl<FitPathProc>;

  // DAQ
  procAllocators["simpledaq"]= new ProcAllocatorTmpl<SimpleDAQProc>;

  // Misc
  procAllocators["count"] = new ProcAllocatorTmpl<CountProc>;
  procAllocators["prune"] = new ProcAllocatorTmpl<PruneProc>;

  // Escape Hatch
  procAllocators["python"] = new ProcAllocatorTmpl<PythonProc>;

  // -----------------------------------------------------------------

  // Register main block
  mainBlock = theMainBlock; // convenience pointer since we only process
                            // events through the main block.  Also lets us
                            // quickly check if the user has forgotten to close
                            // an if statement
  blocks.push(theMainBlock);
}


ProcBlockManager::~ProcBlockManager()
{
  // UI commands
  delete procCmd;

  // ProcAllocators
  std::map<std::string, ProcAllocator *>::iterator allocator = procAllocators.begin();
  while (allocator != procAllocators.end()) {
    delete allocator->second; // allocator points to pair<>,
                            // Processor object is the second item in pair
    allocator++;
  }

  // No need to delete blocks because the main block is owned
  // externally, and by construction all sub blocks are contained
  // within the main block
}

G4String ProcBlockManager::GetCurrentValue(G4UIcommand * command)
{
  Log::Die("Get value not supported on "+command->GetCommandPath());
  return G4String("You should never see this.");
}

void ProcBlockManager::SetNewValue(G4UIcommand * command, G4String newValue)
{
  // procCmd
  if (command == procCmd) {
    if (!DoProcCmd(newValue, false))
      Log::Die("Unknown processor: " + newValue);
  } else if (command == procLastCmd) {
    if (!DoProcCmd(newValue, true))
      Log::Die("Unknown processor: " + newValue);
  } else if (command == setCmd) {
    if (lastProc != 0) {

      try {
	DoProcSetCmd(newValue);
      } catch (Processor::ParamUnknown &pu) {
	Log::Die(lastProc->name + ": Unknown parameter " + pu.param);
      } catch (Processor::ParamInvalid &pi) {
	Log::Die(lastProc->name + ":  " + pi.msg);
      }

    } else
      Log::Die("ProcBlockManager: "
		  "Cannot use /rat/procset until after /rat/proc");
  } else
    Log::Die("ProcBlockManager: Invalid command "
		+command->GetCommandPath());
}


bool ProcBlockManager::DoProcCmd(std::string procname, bool last)
{
    // Is this a user processor? (starts with "user")
    if(procname.find("user") == 0) {
	lastProc = construct_user_proc(procname);
	if (lastProc == 0)
	    return false;
    // Do we have a processor with this name?
    } else if (procAllocators.count(procname) > 0)
      // If so, ask the appropriate process allocator to create a new one
      // and give it to us.
      //
      // Crazy syntax required because we have a list of pointers to functors,
      // rather than just a list of functors.  Had to use pointers because
      // of the polymorphism.  C++ makes OOP ugly.
      lastProc = (*procAllocators[procname])();
    else
	return false;

    ProcBlock *currBlock = blocks.top();

    if (last)
      currBlock->DeferAppend(lastProc);
    else
      currBlock->AddProcessor(lastProc);
    
    return true;
}

void ProcBlockManager::DoProcSetCmd(std::string cmdstring)
{
  Tokenizer t(cmdstring);

  if (t.Next() != Tokenizer::TYPE_IDENTIFIER)
    Log::Die("DoProcSetCmd: Invalid param name in /rat/procset "
		+ cmdstring);
  std::string param(t.Token());

  try { 
    switch (t.Next()) {
    case Tokenizer::TYPE_INTEGER: 
      lastProc->SetI(param, t.AsInt());
      break;
    case Tokenizer::TYPE_DOUBLE:
      lastProc->SetD(param, t.AsDouble()); 
      break;
    case Tokenizer::TYPE_STRING:
      lastProc->SetS(param, t.Token());
      break;
    default:
      Log::Die("DoProcSetCmd: Invalid value in /rat/procset "
		  + cmdstring);
    }
  } catch (Processor::ParamUnknown &pu) {
    Log::Die("Parameter unknown: " + pu.param);
  } catch (Processor::ParamInvalid &pi) {
    Log::Die("Invalid value for parameter " + pi.param + ": " + pi.msg);
  }
}


} // namespace RAT
