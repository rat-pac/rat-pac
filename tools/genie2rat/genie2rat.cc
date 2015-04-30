// takes the output of gevgen_atmo and creates a RAT root file
//
// genie2rat -i [input genie filename] -o [output root filename] (-N [number of events to process])

#include <unistd.h>
#include <getopt.h>
#include <string>

#include <TFile.h>
#include <TTree.h>
#include <TIterator.h>
#include <TVector3.h>

#include "PDG/PDGCodes.h"
#include "EVGCore/EventRecord.h"
#include "GHEP/GHepParticle.h"
#include "Ntuple/NtpMCEventRecord.h"

#include <RAT/DS/Root.hh>
#include <RAT/DS/MC.hh>
#include <RAT/DS/MCParticle.hh>

extern char *optarg;
extern int optind, opterr, optopt;
std::string input_filename;
std::string output_filename;
int num_events;
void parse_command_line(int argc, char** argv);

int main(int argc, char ** argv)
{

  parse_command_line(argc, argv);

  TFile file(input_filename.c_str(),"READ");
  TTree *tree = dynamic_cast <TTree*> (file.Get("gtree"));
  genie::NtpMCEventRecord *eventbranch;
  tree->SetBranchAddress("gmcrec", &eventbranch);

  if (num_events <= 0){
    num_events = (int) tree->GetEntries();
  }

  // set up output file
  TFile *outfile = new TFile(output_filename.c_str(),"RECREATE");
  TTree *outtree = new TTree("T","RAT Tree");
  RAT::DS::Root *branchDS = new RAT::DS::Root();
  outtree->Branch("ds",branchDS->ClassName(),&branchDS,32000,99);

  // Loop over all events
  double time = 0;
  for(int i = 0; i < num_events; i++) {

    // get next tree entry
    tree->GetEntry(i);

    // get the GENIE event
    genie::EventRecord &event = *(eventbranch->event);
    double x = 1000.0*event.Vertex()->X(); // distances from GENIE are in meters
    double y = 1000.0*event.Vertex()->Y(); // distances from GENIE are in meters
    double z = 1000.0*event.Vertex()->Z(); // distances from GENIE are in meters

    RAT::DS::Root *ds = new RAT::DS::Root();
    RAT::DS::MC *mc = ds->GetMC();

    TTimeStamp utc(time,0);
    mc->SetUTC(utc); // GENIE does not give global time, so we just add arbitrary times for RAT 
    mc->SetID(i);

    // Loop over all particles in this event

    genie::GHepParticle *genieparticle;
    TIter event_iter(&event);

    while((genieparticle=dynamic_cast<genie::GHepParticle *>(event_iter.Next())))
    {
      // check if it is the initial state neutrino
      if (genieparticle->Status() == genie::kIStInitialState){ 
        RAT::DS::MCParticle *ratparent = mc->AddNewMCParent();
        ratparent->SetPDGCode(genieparticle->Pdg());
        ratparent->SetTime(0); // GENIE does not give particles time separate from event
        ratparent->SetPosition(TVector3(x,y,z)); // GENIE outputs particle distance from event in fm, basically 0
        ratparent->SetMomentum(TVector3(1000.0*genieparticle->Px(),1000.0*genieparticle->Py(),1000.0*genieparticle->Pz())); // GENIE outputs momentum and energy in GeV
        ratparent->SetKE(1000.0*genieparticle->KinE());
      }
      if (genieparticle->Status() == genie::kIStStableFinalState){
        // skip if it is a GENIE special particle (final state unsimulated hadronic energy etc)
        if (genieparticle->Pdg() > 2000000000){
          continue;
        }
        RAT::DS::MCParticle *ratparticle = mc->AddNewMCParticle();
        ratparticle->SetPDGCode(genieparticle->Pdg());
        ratparticle->SetTime(0); // GENIE does not give particles time separate from event
        ratparticle->SetPosition(TVector3(x,y,z)); // GENIE outputs particle distance from event in fm, basically 0  
        ratparticle->SetMomentum(TVector3(1000.0*genieparticle->Px(),1000.0*genieparticle->Py(),1000.0*genieparticle->Pz())); // GENIE outputs momentum and energy in GeV
        ratparticle->SetKE(1000.0*genieparticle->KinE());
      }
    }// end loop over particles 

    eventbranch->Clear();
    *branchDS = *ds;
    outtree->Fill();

    time += 1; // GENIE does not calculate time of events so we arbitrarily add a second
  }//end loop over events

  // close input GHEP event file
  file.Close();
  outfile->Write();
  outfile->Close();


  return 0;
}

void parse_command_line(int argc, char **argv)
{
  input_filename = "gntp.1.ghep.root";
  output_filename = "output.root";
  num_events = -1;

  static struct option opts[] = { {"input", 1, NULL, 'i'},
                                  {"output", 1, NULL, 'o'},
                                  {"num-events", 1, NULL, 'N'} };
  int option_index = 0;
  int c = getopt_long(argc, argv, "i:o:N", opts, &option_index);
  while (c != -1) {
        switch (c) {
          case 'i': input_filename = optarg; break;
          case 'o': output_filename = optarg; break;
          case 'N': num_events = atol(optarg); break;
          default: exit(1);
        }
        c = getopt_long(argc, argv, "i:o:N", opts, &option_index);
  }
}
