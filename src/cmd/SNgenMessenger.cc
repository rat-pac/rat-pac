// RAT::SNgenMessenger
// 10-Feb-2006 Bill Seligman

// Provide user commands to allow the user to change
// the SNgen parameters via the command line.

#include <RAT/SNgenMessenger.hh>
#include <RAT/SNgen.hh>

#include <G4UIcommand.hh>
#include <G4UIdirectory.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4String.hh>

namespace RAT {
    
    SNgenMessenger::SNgenMessenger(SNgen* e) :
    sngen(e)
    {
        // Commands will in a /generator/supernova/ directory
        G4UIdirectory* dir = new G4UIdirectory("/generator/supernova/");
        dir->SetGuidance("Control the physics parameters of the elastic-scattering generator");
        
        wmaCmd = new G4UIcmdWithADouble("/generator/supernova/wma", this);
        wmaCmd->SetGuidance("Sets the value of sine-squared theta (the weak mixing angle)");
        wmaCmd->SetParameterName("sin2th",false);
        wmaCmd->SetDefaultValue( sngen->GetMixingAngle() );
        
        vmuCmd = new G4UIcmdWithADouble("/generator/supernova/vmu", this);
        vmuCmd->SetGuidance("Sets the value of the neutrino magnetic moment (units of Bohr magnetons)");
        vmuCmd->SetParameterName("vmu",false);
        vmuCmd->SetDefaultValue( sngen->GetMagneticMoment() );
        
        IBDAmpCmd = new G4UIcmdWithADouble("/generator/supernova/ibd", this);
        IBDAmpCmd->SetGuidance("Sets the fractional amount of supernova Inverse Beta Decay's ");
        IBDAmpCmd->SetParameterName("IBDAmp",false);
        IBDAmpCmd->SetDefaultValue( sngen->GetIBDAmplitude() );
        
        ESAmpCmd = new G4UIcmdWithADouble("/generator/supernova/es", this);
        ESAmpCmd->SetGuidance("Sets the fractional amount of supernova Elastic Scattering");
        ESAmpCmd->SetParameterName("ESAmp",false);
        ESAmpCmd->SetDefaultValue( sngen->GetESAmplitude()  );
        
        CCAmpCmd = new G4UIcmdWithADouble("/generator/supernova/cc", this);
        CCAmpCmd->SetGuidance("Sets the fractional amount of supernova Charge Current");
        CCAmpCmd->SetParameterName("CCAmp",false);
        CCAmpCmd->SetDefaultValue(  sngen->GetCCAmplitude()  );
        
        ICCAmpCmd = new G4UIcmdWithADouble("/generator/supernova/icc", this);
        ICCAmpCmd->SetGuidance("Sets the fractional amount of supernova Inverse Charge Current ");
        ICCAmpCmd->SetParameterName("ICCAmp",false);
        ICCAmpCmd->SetDefaultValue(sngen->GetICCAmplitude()  );
        
        NCAmpCmd = new G4UIcmdWithADouble("/generator/supernova/nc", this);
        NCAmpCmd->SetGuidance("Sets the fractional amount of supernova Neutral Current");
        NCAmpCmd->SetParameterName("NCAmp",false);
        NCAmpCmd->SetDefaultValue( sngen->GetNCAmplitude());
        
        ModelCmd = new G4UIcmdWithADouble("/generator/supernova/model", this);
        ModelCmd->SetGuidance("Sets the fractional amount based on model (1: livermore, 2:gkvm)");
        ModelCmd->SetParameterName("Model",false);
        ModelCmd->SetDefaultValue( sngen->GetModel());
        
        
        
    }
    
    SNgenMessenger::~SNgenMessenger() {;}
    
    void SNgenMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
    {
        if ( command == wmaCmd )
        {
            G4double wma = wmaCmd->GetNewDoubleValue( newValue );
            sngen->SetMixingAngle( wma );
        }
        else if ( command == vmuCmd )
        {
            G4double vmu = vmuCmd->GetNewDoubleValue( newValue );
            sngen->SetNeutrinoMoment( vmu );
        }else  if ( command == IBDAmpCmd )
        {
            double IBDAmpl = IBDAmpCmd->GetNewDoubleValue( newValue );
            sngen->SetIBDAmplitude ( IBDAmpl );
        }
        else if ( command == ESAmpCmd )
        {
            double ESAmpl = ESAmpCmd->GetNewDoubleValue( newValue );
            sngen->SetESAmplitude ( ESAmpl );
        }
        else if ( command == CCAmpCmd )
        {
            double CCAmpl = CCAmpCmd->GetNewDoubleValue( newValue );
            sngen->SetCCAmplitude ( CCAmpl );
        }
        else if ( command == ICCAmpCmd )
        {
            double ICCAmpl = ICCAmpCmd->GetNewDoubleValue( newValue );
            sngen->SetICCAmplitude ( ICCAmpl );
        }
        else if ( command == NCAmpCmd )
        {
            double NCAmpl = NCAmpCmd->GetNewDoubleValue( newValue );
            sngen->SetNCAmplitude ( NCAmpl );
        }
        else if ( command == ModelCmd )
        {
            int MAmpl = int(ModelCmd->GetNewDoubleValue( newValue ));
            sngen->SetModel ( MAmpl );
        }
        else
        {
            G4cerr << "Error: Invalid SNgenMessenger \"set\" command" << G4endl;
        }
    }
    
    G4String SNgenMessenger::GetCurrentValue(G4UIcommand* command)
    {
        if ( command == wmaCmd )
            return wmaCmd->ConvertToString( sngen->GetMixingAngle() );
        else if ( command == vmuCmd )
            return vmuCmd->ConvertToString( sngen->GetMagneticMoment() );
        else if ( command == IBDAmpCmd )
            return IBDAmpCmd->ConvertToString( sngen->GetIBDAmplitude() );
        else if ( command == ESAmpCmd )
            return ESAmpCmd->ConvertToString( sngen->GetESAmplitude() );
        else if ( command == CCAmpCmd )
            return CCAmpCmd->ConvertToString( sngen->GetCCAmplitude() );
        else if ( command == ICCAmpCmd )
            return ICCAmpCmd->ConvertToString( sngen->GetICCAmplitude() );
        else if ( command == NCAmpCmd )
            return NCAmpCmd->ConvertToString( sngen->GetNCAmplitude() );
        else if ( command == ModelCmd )
            return ModelCmd->ConvertToString( sngen->GetModel() );
        // Error if we reach here.
        return G4String("Error: Invalid SNgenMessenger \"get\" command");
    }
    
} // namespace RAT
