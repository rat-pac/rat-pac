// RAT::Coincidence_Gen 
// 16-July-2008 JR Wilson

/** @class RAT::Coincidence_Gen
*
* Top Level Event Producer - coincidences of different interactions separated in time and space in same event
*
* @author Jeanne Wilson <j.wilson2@physics.ox.ac.uk>
*
* first version : 16-July-2008
*
* This generator simulates 2 different decays at different positions and different times in the same 
* specified event window. It can be used for the purposes of studying pile-up as it will efficiently
* produce a sample in which each event is a coincidence.
* There is also the option of restricting the events generated to those with summed primary particle 
* energy in a given range. 
*/

#ifndef __RAT_Coincidence_Gen__
#define __RAT_Coincidence_Gen__

#include <globals.hh>
#include <G4String.hh>
#include <GLG4Gen.hh>

class G4Event;
class GLG4PosGen;

namespace RAT {

  class CoincidenceMessenger;

  class Coincidence_Gen : public GLG4Gen {
  public:
    Coincidence_Gen();
    virtual ~Coincidence_Gen();
    virtual void GenerateEvent(G4Event *event);
    virtual void ResetTime(double offset=0.0);
    virtual bool IsRepeatable() const { return true; };

	/** Basic state - first interaction type and position */
    virtual void SetState(G4String state);
    virtual G4String GetState() const;

    /** time window over which 2 events considered coincident */
		virtual void SetTimeWindow(double window) { fTimeWindow = window; _expTimingTimeWindow=true;};
    virtual double GetTimeWindow() const { return fTimeWindow; };
	
	/** restriction on total (generated) energy of pair */
	virtual void SetEnergyRange(G4String newValues);
	virtual double  LoEnergyLimit() const { return fLoEnergy; };
	virtual double  HiEnergyLimit() const { return fHiEnergy; };
	
    /** Overall time state - time from one coincidence event to next */
	virtual void SetTimeState(G4String state);
    virtual G4String GetTimeState() const;
	
	/** position generator for base interaction */
    virtual void SetPosState(G4String state);
    virtual G4String GetPosState() const;
	
	/** vertex generator for base interaction */
  	virtual void SetVertexState(G4String state);
  	virtual G4String GetVertexState() const;

	/** add an extra interaction */
	virtual void AddExtra(G4String state);
	/** return state of added interation nint */
	virtual G4String GetExtraState(int nint) const;
	/** return how many extra states have been added */
	virtual int GetNExtra() const { return nExtra; };
	
	/** set position generator for most recently added extra interaction */
	virtual void SetExtraPosState(G4String state);
	/** get name of position generator for extra interaction, nint */
	virtual G4String GetExtraPosState(int nint) const;
	
	/** set vertex generator for most recently added extra interaction */
	virtual void SetExtraVertexState(G4String state);
	/** get name of vertex generator for extra interaction nint */
	virtual G4String GetExtraVertexState(int nint) const;
	
	/** Set exponential time constants to separate decays. 
	* Default is to give each vertex a random time in the event window but if this option is called
	* the user must set a time constant, T, for each added vertex. 
	* First vertex is at t = 0, the rest have times selected from exponentials exp(-t/T)
	* each exponential applies to the time since last vertex */
	virtual void SetExponentials(G4String state);
	virtual void SetExpoForceWindow(G4bool state);
	

  protected:

    /** Generator initialization, specified by the user */
    G4String stateStr;
	/** limit the maximum number of extra generators to 5
	* Additional generators */
	int _maxExtra;
	G4String stateStrExtra[5];

  	/** The overall time generator - time from one double event to next */
	GLG4TimeGen *timeGen;
	
  	/** The initial vertex generator */
	GLG4VertexGen *vertexGen;
	/** The additional vertex generators */
	GLG4VertexGen *vertexGenExtra[5];
	
    /** The initial position generator */
    GLG4PosGen* posGen;
	/** The additional position generators */
    GLG4PosGen* posGenExtra[5];
	
	/** the length of the time window for coincidences */
	double fTimeWindow;	
	/** choose exponential timing structure (either all vertices have random times, or all are given decay constants*/
	bool   _exptiming;
	bool   _expTimingTimeWindow;
	bool   _expTimingTimeWindowForce;
	/** the value of an exponential decay factor for subsequent decays - default is zero, only used if set */
	double fExponent[5];
	/** the number of extra interations */
	int    nExtra;
	/** the range restrictions on the total energy generated for the total event */
	double fLoEnergy;
	double fHiEnergy;
   
    /** Allows the user to change parameters via the command line. */
    CoincidenceMessenger* messenger;
	
  };

} // namespace RAT

#endif
