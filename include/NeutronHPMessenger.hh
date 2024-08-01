
// $Id: NeutronHPMessenger.hh 66241 2012-12-13 18:34:42Z gunter $
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef NeutronHPMessenger_h
#define NeutronHPMessenger_h 1

#include "globals.hh"
#include "G4UImessenger.hh"

class HadronElasticPhysicsHP;
class G4UIdirectory;
class G4UIcmdWithABool;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class NeutronHPMessenger: public G4UImessenger
{
  public:
    NeutronHPMessenger(HadronElasticPhysicsHP*);
   ~NeutronHPMessenger();
    
    virtual void SetNewValue(G4UIcommand*, G4String);
    
  private:    
    HadronElasticPhysicsHP*  fNeutronPhysics;
    
    G4UIdirectory*     fPhysDir;      
    G4UIcmdWithABool*  fThermalCmd;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
