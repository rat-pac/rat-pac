///////////////////////////////////////////////////////////////////////////////
/// \file TestUser.cc
/// \brief An example User Processor
///
/// This example demonstrates how to write a User Processor. MyUserProc
/// (declared and defined below) just creates a 1-D histogram of the number of
/// photoelectrons in each event, filled it as events are generated and writes
/// it to a file called "numpe.root".
///
/// (Of course, you could easily do this just by writing the events to
/// disk using the outroot processor and making the histogram offline.
/// But then you wouldn't get to see how user processors work!)
///
///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <TFile.h>
#include <TH1F.h>
#include <RAT/Processor.hh>
#include <RAT/Log.hh>
#include <RAT/DS/Root.hh>

using namespace RAT;

// Class declaration
// (if this gets big, you should move it to its own header file)
class MyUserProc : public Processor {
public:
  MyUserProc();
  virtual ~MyUserProc();

  virtual Processor::Result DSEvent(DS::Root *ds);
  
  // Override this method instead if you want to be called for
  // every triggered DAQ event, rather than every physics event
  // Only use one of these methods!
  // virtual Processor::Result Event(DS::Root *ds, DS::EV *ev);

protected:
  TFile *f;
  TH1F *hNumPE;
};


namespace RAT {
  // Override user processor utility function
  Processor *construct_user_proc(std::string /*userProcName*/) {
    // If you have several different user processors, check the
    // userProcName parameter and allocate the proper one.  Only one
    // kind of user processor in this example, so we ignore userProcName.
    return new MyUserProc;
  }
} // namespace RAT


// Class definition
MyUserProc::MyUserProc() : Processor("user") {
  // Construct all your private objects and do any initialization
  // This gets called when your processor is created and
  // added to the event loop in the command file.
  // (Nothing is running yet!)

  f = TFile::Open("numpe.root", "RECREATE");
  Log::Assert(f != NULL, "MyUserProc: Unable to open ROOT output file.");

  hNumPE = new TH1F("hNumPE", "Number of photoelectrons per event", 100, 0, 1000);
}

MyUserProc::~MyUserProc() {
  // Delete your private objects and print any summary information.
  // This gets called when the event loop is cleared by the user or at
  // the end of execution.

  f->cd();
  hNumPE->Write();
  f->Close();
  delete f; // deletes hNumPE too -- ROOT does weird stuff like this
}

Processor::Result MyUserProc::DSEvent(DS::Root *ds) {
  // Process one event. This is called once per DETECTOR event, and not once
  // per PHYSICS event, i.e. once for every RAT::EV in the list.

  hNumPE->Fill(ds->GetMC()->GetNumPE());

  // Tell the event loop handler the result of processing.
  return Processor::OK;
}

