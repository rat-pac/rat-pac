#include "RAT/Sampling.hh"

namespace RAT {

G4PhysicsOrderedFreeVector*
Integrate_MPV_to_POFV( G4MaterialPropertyVector* inputVector )
{
    G4PhysicsOrderedFreeVector *aPhysicsOrderedFreeVector
      = new G4PhysicsOrderedFreeVector();
    
    // Retrieve the first intensity point in vector
    // of (photon momentum, intensity) pairs 

    unsigned int i = 0;
    G4double currentIN = (*inputVector)[i];

    if (currentIN >= 0.0)
      {

	// Create first (photon momentum, Scintillation 
	// Integral pair  

	G4double currentPM = inputVector->Energy(i);

	G4double currentCII = 0.0;

	aPhysicsOrderedFreeVector->
	  InsertValues(currentPM , currentCII);

	// Set previous values to current ones prior to loop
	G4double prevPM  = currentPM;
	G4double prevCII = currentCII;
	G4double prevIN  = currentIN;

	// loop over all (photon momentum, intensity)
	// pairs stored for this material  
	while(i < inputVector->GetVectorLength()-1)
	  {
	      i++;
	      currentPM = inputVector->Energy(i);
	      currentIN=(*inputVector)[i];

	      currentCII = 0.5 * (prevIN + currentIN);

	      currentCII = prevCII +
		(currentPM - prevPM) * currentCII;

	      aPhysicsOrderedFreeVector->
		InsertValues(currentPM, currentCII);

	      prevPM  = currentPM;
	      prevCII = currentCII;
	      prevIN  = currentIN;
	  }

      }

    return aPhysicsOrderedFreeVector;
}

} // namespace RAT
