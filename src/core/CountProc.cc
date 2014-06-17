#include <RAT/CountProc.hh>
#include <RAT/Log.hh>

namespace RAT {


CountProc::CountProc() : Processor("count")
{
  dscount = 0;
  evcount = 0;
  updateInterval = 1; // Print count message for every event
}

CountProc::~CountProc()
{
  /* Do nothing */

  info << dformat("CountProc: Total # of events %d (%d triggered events)\n",
		  dscount, evcount);

}

void CountProc::SetI(std::string param, int value)
{
  if (param == "update")
    if (value > 0)
      updateInterval = value;
    else
      throw ParamInvalid(param, "update interval must be > 0");
  else
    throw ParamUnknown(param);
}


Processor::Result CountProc::DSEvent(DS::Root *ds)
{
  dscount++;
  evcount += ds->GetEVCount();

  if (dscount % updateInterval == 0)
    info << dformat("CountProc: Event %d (%d triggered events)\n",
		    dscount, evcount);
  return OK;
}


} // namespace RAT
