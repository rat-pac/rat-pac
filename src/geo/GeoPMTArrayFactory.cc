#include <RAT/GeoPMTArrayFactory.hh>
#include <vector>
#include <RAT/Log.hh>

using namespace std;

namespace RAT {

  G4VPhysicalVolume *GeoPMTArrayFactory::Construct(DBLinkPtr table){
    info << "GeoPMTArrayFactory: Constructing volume " + table->GetIndex()  
         << newline;
    // Read PMT positions                   
    string pos_table_name = table->GetS("pos_table");
    DBLinkPtr lpos_table = DB::Get()->GetLink(pos_table_name);
    const vector<double> &pmt_x = lpos_table->GetDArray("x");
    const vector<double> &pmt_y = lpos_table->GetDArray("y");
    const vector<double> &pmt_z = lpos_table->GetDArray("z");
    return ConstructPMTs(table, pmt_x, pmt_y, pmt_z);
  }
    
} // namespace RAT
