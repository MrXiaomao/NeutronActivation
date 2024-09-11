
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
   ~PrimaryGeneratorAction();

  public:
    void SetDefaultKinematic();  
    void SetRndmBeam(G4double value)  {fRndmBeam = value;}
    void SetTimeExposure(G4double value)  {fTimeExposure = value;}

    virtual void GeneratePrimaries(G4Event*);
         
    G4ParticleGun* GetParticleGun() {return fParticleGun;}
    G4double GetTimeExposure() {return fTimeExposure;}

  private:
    G4ParticleGun*             fParticleGun;
    G4GeneralParticleSource* fParticleGun2;
    DetectorConstruction*      fDetector;
    G4double                   fRndmBeam;
    G4double                   fTimeExposure;
    PrimaryGeneratorMessenger* fGunMessenger;     
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
