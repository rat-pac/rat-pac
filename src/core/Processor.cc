#include <RAT/Processor.hh>
#include <vector>

using namespace std;

namespace RAT {


// Default implementations
void Processor::SetI(std::string param, int /*value*/)
{
  throw Processor::ParamUnknown(param);
}

void Processor::SetF(std::string param, float /*value*/)
{
  throw Processor::ParamUnknown(param);
}

void Processor::SetD(std::string param, double /*value*/)
{
  throw Processor::ParamUnknown(param);
}

void Processor::SetS(std::string param, std::string /*value*/)
{
  throw Processor::ParamUnknown(param);
}


Processor::Result Processor::DSEvent(DS::Root *ds)
{
  // If this not overridden, assume processor wants to see individual events

  Processor::Result overall_result = Processor::OK;

  for (int i=0; i < ds->GetEVCount(); i++) {
    Processor::Result res = Event(ds, ds->GetEV(i));

    if (res == Processor::ABORT)
      return Processor::ABORT;
    else if (res == Processor::FAIL)
      overall_result = Processor::FAIL; // failure is OR of ev failures
  }

  return overall_result;
}
  
Processor::Result Processor::Event(DS::Root */*ds*/, DS::EV */*ev*/)
{
  // no-op, but we need to return something
  return Processor::OK;
}


} // namespace RAT
