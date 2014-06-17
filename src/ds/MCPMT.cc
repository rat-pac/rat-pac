#include <RAT/DS/MCPMT.hh>

ClassImp(RAT::DS::MCPMT)

namespace RAT {
  namespace DS {

Float_t MCPMT::GetCharge() const {
  Float_t charge = 0.0;
  for (unsigned int i=0; i < photon.size(); i++)
    charge += photon[i].GetCharge();
  return charge;
}
    
  } // namespace DS
} // namespace RAT

