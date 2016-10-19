// RAT::VertexGen_Isotope
//18-July-2008 JR Wilson

/** @class RAT::VertexGen_Isotope
 *
 * Vertex Generator - events of given particle (or ion) type, with energies sampled from Isotope via DB
 *
 * @author Jeanne Wilson <j.wilson2@physics.ox.ac.uk>
 *
 * first version : 18-July-2008
 *
 * This vertex generator (to be used in composite generators like combo of coincidence) produces isotropic particles
 * or ions with kinetic energies sampled from a data-base defined distribution. An option to limit the range
 * of the energy distribution is available
 */
#ifndef __RAT_VertexGen_Isotope__
#define __RAT_VertexGen_Isotope__

#include <RAT/GLG4VertexGen.hh>
#include "RAT/DB.hh"
#include <RAT/IsotopeMessenger.hh>

#include <G4Event.hh>
#include <G4ThreeVector.hh>
#include <globals.hh>
#include <vector>

namespace RAT {
    
    class VertexGen_Isotope : public GLG4VertexGen {
    public:
        
        VertexGen_Isotope(const char *arg_dbname="isotope");
        virtual 		~VertexGen_Isotope();
        /** generate and add new vertex to this event.  Position and
         * time of vertex are offset from 0 by dx and dt.  (These
         * are usually derived from GLG4PosGen and GLG4TimeGen.)
         */
        virtual void 	GeneratePrimaryVertex( G4Event* argEvent,
                                              G4ThreeVector& dx,
                                              G4double dt);
        /** set the state for generator. Format: pname specname
         * where pname is the particle name, and specname is the database Isotope name
         */
        virtual void 	SetState( G4String newValues );
        /** return current state */
        virtual 		G4String GetState();
        
        /** Set up the Isotope array in memory - create normalised cumulative magnitude for easy**/
        
        void SetIsotopeA(    double AAm =  ADEFAULT);
        void SetIsotopeZ(    double ZAm =  ZDEFAULT);
        void SetIsotopeE(    double EAm =  EDEFAULT);
        
        inline double GetIsotopeA()   {return valueA;} ;
        inline double GetIsotopeZ()   {return valueZ;} ;
        inline double GetIsotopeE()   {return valueE;} ;
        
    private:
        G4String	_particle;			// name of the particle type
        G4ParticleDefinition* _pDef;	// particle definition
        G4String	_Isotope;			// name of the Isotope to use
        
        double valueE,valueA,valueZ;
        
        IsotopeMessenger* messenger;

//        double ADEFAULT,ZDEFAULT,EDEFAULT;
        
        static const double ADEFAULT;
        static const double ZDEFAULT;
        static const double EDEFAULT;

    };
    
} // namespace RAT

#endif
