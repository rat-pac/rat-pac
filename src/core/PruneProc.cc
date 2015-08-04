#include <RAT/PruneProc.hh>
#include <RAT/Log.hh>

#include <vector>
#include <algorithm>
#include "string_utilities.hpp"

using namespace std;

namespace RAT {

PruneProc::PruneProc() : Processor("prune") {
  mc = mc_particle = mc_track = mc_pmt = mc_pmt_photon = mc_pmt_pulse
     = ev = ev_pmt = false;
}

PruneProc::~PruneProc() {}

void PruneProc::SetS(std::string param, std::string value) {
  if (param == "prune") {
    vector<string> parts = split(value, ",");
    for (unsigned int i = 0; i < parts.size(); i++) {
      vector<string> subparts = split(parts[i], ":");

      if (subparts.size() == 1)
	SetPruneState(parts[i], true); // normal case
      else {
	// mc.track case
	if (subparts[0] != "mc.track")
	  throw ParamInvalid(param, "Cannot use colon in " + parts[i]);

	SetPruneState("mc.track", true);
	track_cut.push_back(subparts[1]);
      }
    }
  } else
    throw ParamUnknown(param);
}

void PruneProc::SetPruneState(std::string item, bool state) {
  if (item == "mc")                     mc = state;
  else if (item == "mc.particle")       mc_particle = state;
  else if (item == "mc.track")          mc_track = state;
  else if (item == "mc.pmt")            mc_pmt = state;
  else if (item == "mc.pmt.photon")     mc_pmt_photon = state;
  else if (item == "mc.pmt.pulse")      mc_pmt_pulse = state;
  else if (item == "ev")                ev = state;
  else if (item == "ev.pmt")            ev_pmt = state;
  else
    Log::Die("prune: Set state on unknown list " + item);
}

bool PruneProc::GetPruneState(std::string item) {
  if (item == "mc")                     return mc;
  else if (item == "mc.particle")       return mc_particle;
  else if (item == "mc.track")          return mc_track;  
  else if (item == "mc.pmt")            return mc_pmt;
  else if (item == "mc.pmt.photon")     return mc_pmt_photon;
  else if (item == "mc.pmt.pulse")      return mc_pmt_pulse;
  else if (item == "ev")                return ev;
  else if (item == "ev.pmt")            return ev_pmt;
  else {
    Log::Die("prune: Get state on unknown list " + item);
    return false; // never get here
  }
}

Processor::Result PruneProc::DSEvent(DS::Root *ds) {
  if (mc)
    ds->PruneMC();
    
  if (ds->ExistMC()) {
    DS::MC *pmc = ds->GetMC();
    
    if (mc_particle)
      pmc->PruneMCParticle();

    if (mc_track) {
      if (track_cut.size() == 0)
        pmc->PruneMCTrack();
      else { // remove each listed particle name
	for (unsigned i=0; i < track_cut.size(); i++)
	  pmc->PruneMCTrack(track_cut[i]);
      }
    }

    if (mc_pmt)
      pmc->PrunePMT();

    if (mc_pmt_photon) {
      for (int i=0; i < pmc->GetMCPMTCount(); i++)
        pmc->GetMCPMT(i)->PruneMCPhoton();
    }
  }
  
  if (ev)
    ds->PruneEV();

  if (ev_pmt) {
    for (int i=0; i < ds->GetEVCount(); i++)
      ds->GetEV(i)->PrunePMT();
  }

  return Processor::OK;
}

} // namespace RAT

