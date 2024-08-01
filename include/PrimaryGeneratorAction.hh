
#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"

class G4Event;
class DetectorConstruction;
class PrimaryGeneratorMessenger;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction(DetectorConstruction*);    
   ~PrimaryGeneratorAction() override;

  public:
    void SetDefaultKinematic();  
    void SetRndmBeam(G4double value)  {fRndmBeam = value;}
    void SetTimeExposure(G4double value)  {fTimeExposure = value;}

    void GeneratePrimaries(G4Event*) override;
         
    G4ParticleGun* GetParticleGun() {return fParticleGun;}
    G4double GetTimeExposure() {return fTimeExposure;}

  private:
    G4ParticleGun*             fParticleGun  = nullptr;
    G4GeneralParticleSource* fParticleGun2;
    DetectorConstruction*      fDetector     = nullptr;
    G4double                   fRndmBeam     = 0.;
    G4double                   fTimeExposure = 0.;
    PrimaryGeneratorMessenger* fGunMessenger = nullptr;     
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
