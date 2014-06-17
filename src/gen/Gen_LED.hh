#ifndef __RAT_Gen_LED__
#define __RAT_Gen_LED__

#include <vector>
#include <CLHEP/Random/RandGeneral.h>
#include "GLG4Gen.hh"

class G4ParticleDefinition;

namespace RAT {

class Gen_LED : public GLG4Gen {
public:
  Gen_LED();
  virtual ~Gen_LED();
  virtual void GenerateEvent(G4Event *event);
  virtual void ResetTime(double offset=0.0);
  virtual bool IsRepeatable() const { return true; };

  virtual void SetState(G4String state);
  virtual G4String GetState() const;
  
  void SetLEDParameters(G4String state);

  virtual void SetTimeState(G4String state);
  virtual G4String GetTimeState() const;
  virtual void SetVertexState(G4String state);
  virtual G4String GetVertexState() const;
  virtual void SetPosState(G4String state);
  virtual G4String GetPosState() const;


protected:
  G4String stateStr;
  GLG4TimeGen *timeGen;

  std::vector<double> led_x, led_y, led_z, led_wavelength;
	std::vector<int> photons_per_LED;
  bool iso_mode;
	bool ang_dist_mode;
	bool multi_ang_dist_mode;
  bool mono_wl_mode;
  bool unif_mode;
	bool oneLED;//fire one LED per event?

  CLHEP::RandGeneral *rand_angle, *rand_wl, *rand_time;
  float angle_min, angle_max, wl_min, wl_max, time_min, time_max;
	std::vector<CLHEP::RandGeneral* > rand_angles;
	std::vector<double> angle_mins, angle_maxs;

  int photons_per_event;
  int next_led;
  G4ParticleDefinition *photonDef;
};

} // namespace RAT

#endif
