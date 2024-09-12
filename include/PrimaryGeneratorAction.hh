
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
    void SetRndmBeam(G4double )  {;}
    void SetTimeExposure(G4double )  {;}

    virtual void GeneratePrimaries(G4Event*);
         
    G4ParticleGun* GetParticleGun() {return fParticleGun;}
    G4double GetTimeExposure() {return fLengthZr;}

  private:
    G4ParticleGun*             fParticleGun;
    G4GeneralParticleSource* fParticleGun2;
    DetectorConstruction*      fDetector;
    G4double                   fRadiusZr; //镐帽半径
    G4double                   fLengthZr; //锆帽长度
    G4double                   fRotateZr; //锆帽旋转角度
    PrimaryGeneratorMessenger* fGunMessenger;     
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
