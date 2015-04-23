#include <RAT/PosGen_RegexFill.hh>
#include <RAT/Log.hh>

#include <G4VSolid.hh>
#include <G4VoxelLimits.hh>
#include <G4GeometryTolerance.hh>
#include <G4TransportationManager.hh>
#include <Randomize.hh>

#include <sstream>

using namespace std;

namespace RAT {

void PosGen_RegexFill::SetState(G4String regex) {
    regex = trim(regex);

    G4VPhysicalVolume *worldPhys = 
        G4TransportationManager::GetTransportationManager()->
        GetNavigatorForTracking()->
        GetWorldVolume();
        
    G4LogicalVolume *worldLog = worldPhys->GetLogicalVolume();
    
    FindVolumes(worldLog,regex,fVolumes);
    
    if (fVolumes.size() == fVolumeCumu.size()) {
        Log::Die("PosGen_RegexFill::SetState: the regex \"" + regex + "\" did not match any volumes!");
    }
    
    fState += regex + " ";
    
    fVolumeCumu.resize(fVolumes.size());
    fVolumeCumu[0] = fVolumes[0].solidVolume;
    for (size_t i = 1; i < fVolumes.size(); i++) {
        fVolumeCumu[i] = fVolumeCumu[i-1] + fVolumes[i].solidVolume;
    }
}

void PosGen_RegexFill::FindVolumes(G4LogicalVolume *mother, G4String &regex, vector<FillVolume> &found) {
    for (int i = 0; i < mother->GetNoDaughters(); i++) {
        G4VPhysicalVolume *daughterPhys = mother->GetDaughter(i);
        G4LogicalVolume *daughterLog = daughterPhys->GetLogicalVolume();
        // this projects from daughter to mother coordinates
        G4AffineTransform transform(daughterPhys->GetRotation(), daughterPhys->GetTranslation());
        if (daughterPhys->GetName() == regex) { //daughterPhys matches the regular expression
            found.resize(found.size()+1);
            FillVolume &vol = found[found.size()-1];
            
            vol.phys = daughterPhys;
            vol.log = daughterLog;
            vol.transform = transform;
            G4VSolid *solid = daughterLog->GetSolid();
            vol.solidVolume = solid->GetCubicVolume(); //FIXME rumor has it that GetCubicVolume is terribly inaccurate
            for (int j = 0; j < daughterLog->GetNoDaughters(); j++) {
                G4VPhysicalVolume *grandPhys = daughterLog->GetDaughter(j);
                G4AffineTransform grandTransform(grandPhys->GetRotation(), grandPhys->GetTranslation());
                G4VSolid *grandSolid = grandPhys->GetLogicalVolume()->GetSolid();
                vol.solidVolume -= grandSolid->GetCubicVolume();
                vol.daughters.push_back(make_pair(grandSolid,grandTransform.Inverse()));
            }
            
            G4VoxelLimits inf;
            G4AffineTransform ident;
            solid->CalculateExtent(kXAxis, inf, ident, vol.x0, vol.x1);
            solid->CalculateExtent(kYAxis, inf, ident, vol.y0, vol.y1);
            solid->CalculateExtent(kZAxis, inf, ident, vol.z0, vol.z1);
            vol.boundVolume = (vol.x1-vol.x0)*(vol.y1-vol.y0)*(vol.z1-vol.z0);
            
        }
        size_t j = found.size();
        FindVolumes(daughterLog, regex, found);
        for(; j < found.size(); j++) { //apply this mother->daughterPhys transform to all found matches
            found[j].transform *= transform; 
        }
    }
}

G4String PosGen_RegexFill::GetState() const {
    return fState;
}

void PosGen_RegexFill::GeneratePosition(G4ThreeVector& pos) {
    double randVolume = G4UniformRand() * fVolumeCumu.back();
    size_t volidx = 0;
    for ( ; volidx < fVolumes.size(); volidx++) {
        if (randVolume <= fVolumeCumu[volidx]) break;
    }
    FillVolume &vol = fVolumes[volidx];
    G4VSolid *solid = vol.log->GetSolid();
    //FIXME do we want to keep trying forever?
    for (;;) {
        //Solid works in "local" coordinates, so generate position, test, then project to global
        G4ThreeVector trial(
            vol.x0 + G4UniformRand() * (vol.x1 - vol.x0),
            vol.y0 + G4UniformRand() * (vol.y1 - vol.y0),
            vol.z0 + G4UniformRand() * (vol.z1 - vol.z0));
        if (solid->Inside(trial)) { // inside the volume but maybe in a daughter
            bool valid = true;
            for (size_t j = 0; j < vol.daughters.size(); j++) {
                if (vol.daughters[j].first->Inside(vol.daughters[j].second.TransformPoint(trial))) {
                    valid = false; // point was inside a daughter
                    break;
                }
            }
            if (!valid) continue;
            // Convert to global coordinates
            pos = vol.transform.TransformPoint(trial);
            return;
        }
    }
}

}

