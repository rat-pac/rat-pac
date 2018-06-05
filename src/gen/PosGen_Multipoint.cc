#include <RAT/PosGen_Multipoint.hh>
#include <RAT/Log.hh>
#include <sstream>
#include <RAT/DB.hh>
#include <RAT/RadicalInverse.hh>
#include <RAT/StringUtil.hh>
#include <CLHEP/Units/PhysicalConstants.h>

namespace RAT {


PosGen_Multipoint::PosGen_Multipoint(const char *arg_dbname)
  : GLG4PosGen(arg_dbname), 
    fType(MULTIPOINT_TABLE), fTableName(""), fTableIndex(""), fNextPosIndex(0)
{
}


void PosGen_Multipoint::GeneratePosition( G4ThreeVector &argResult )
{
  if (fPos.size() == 0)
    RAT::Log::Die("PosGen_Multipoint: No points in list.  Did you configure generator?");

  argResult = fPos[fNextPosIndex];
  fNextPosIndex = (fNextPosIndex + 1) % fPos.size();
}

void PosGen_Multipoint::SetState( G4String newValues )
{
  newValues = strip_default(newValues);
  if (newValues.length() == 0) {
    // print help and current state
    G4cout << "Current state of this PosGen_Multipoint:\n"
	   << " \"" << GetState() << "\"\n" << G4endl;
    G4cout << "Format of argument to PosGen_Multipoint::SetState: \n"
      " \"uniform [# of points] [inner radius] [outer radius]\"" << G4endl;
    G4cout << "   or" << G4endl;
    G4cout << "\"table [name of table]\""<< G4endl;
    RAT::Log::Die("PosGen_Multipoint requres arguments");
  }

  std::istringstream is(newValues.c_str());

  std::string typestr;

  is >> typestr;
  
  if (typestr == "uniform") {
    fType = MULTIPOINT_UNIFORM;
    int numPoints;
    is >> numPoints >> fInnerRadius >> fOuterRadius;

    if (is.fail()) {
      RAT::Log::Die("PosGen_Multipoint: Could not parse one int and two doubles from config string");
    }

    // Swap to make radius range physical
    if (fOuterRadius < fInnerRadius) {
      double temp = fOuterRadius;
      fOuterRadius = fInnerRadius;
      fInnerRadius = temp;
    }

    LoadUniformPoints(numPoints, fInnerRadius, fOuterRadius);
  } else if (typestr == "table") {
    std::string tableStr;
    is >> tableStr;

    if (is.fail()) {
      RAT::Log::Die("PosGen_Multipoint: Could not parse a table name from config string.");
    }

    // Extract table name and index
    if (!DB::ParseTableName(tableStr, fTableName, fTableIndex))
      RAT::Log::Die("PosGen_Multipoint: Incorrectly formatted table name " + tableStr);

    LoadTablePoints(fTableName, fTableIndex);
  } else {
    RAT::Log::Die("PosGen_Multipoint: Unknown multipoint type " + newValues);
  }
}

G4String PosGen_Multipoint::GetState() const
{

  if (fType == MULTIPOINT_TABLE)
    return std::string("table ") + fTableName + "[" + fTableIndex + "]";
  else if (fType == MULTIPOINT_UNIFORM)
    return dformat ("uniform %d %f %f", 
		    fPos.size(), fInnerRadius, fOuterRadius);
  else
    RAT::Log::Die("PosGen_Multipoint: Inconsisted type in object");

  return "";
}


void PosGen_Multipoint::LoadUniformPoints(unsigned numPoints,
					  double innerRadius,
					  double outerRadius)
{
  double inner3 = pow(innerRadius, 3.0);
  double range3 = pow(outerRadius, 3.0) - inner3;
  
  fPos.resize(numPoints);
  for (unsigned i=0; i < numPoints; i++) {
    double radius   = pow( inner3 + RadicalInverse(i, 2) * range3, 1.0/3.0 );
    double phi      = RadicalInverse(i, 3) * 2 * CLHEP::pi;
    double costheta = 2.0 * RadicalInverse(i, 5) - 1.0;
    double theta    = acos(costheta);

    fPos[i].setRThetaPhi(radius, theta, phi);
  }
}
 
void PosGen_Multipoint::LoadTablePoints(std::string tableName,
					std::string tableIndex)
{
  DBLinkPtr ltable = DB::Get()->GetLink(tableName, tableIndex);

  const std::vector<double> &x = ltable->GetDArray("x");
  const std::vector<double> &y = ltable->GetDArray("y");
  const std::vector<double> &z = ltable->GetDArray("z");
  
  if ( x.size() != y.size() || y.size() != z.size() )
    return;
    //RAT::Log::Die(dformat(
    // "PosGen_Multipoint: x(%d), y(%d), z(%d) lengths don't match in table.",
    // x.size(), y.size(), z.size()));
  
  fPos.resize(x.size());
  for (unsigned i=0; i < fPos.size(); i++)
    fPos[i].set(x[i], y[i], z[i]);
}


} // namespace RAT
