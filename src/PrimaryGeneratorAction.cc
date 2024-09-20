#include "PrimaryGeneratorAction.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorMessenger.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction(DetectorConstruction* det)
:G4VUserPrimaryGeneratorAction(),
 fDetector(det), fRadiusZr(-1.), fLengthZr(0.), fRotateZr(0.), fGunMessenger(nullptr)
{
  fParticleGun  = new G4ParticleGun(1);
  fParticleGun2 = new G4GeneralParticleSource();
  SetDefaultKinematic();
    
  //create a messenger for this class
  fGunMessenger = new PrimaryGeneratorMessenger(this);  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
  delete fParticleGun2;
  delete fGunMessenger;  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::SetDefaultKinematic()
{
  G4ParticleDefinition* particle
           = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticleEnergy(14*MeV);  
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(1.,0.,0.));
  G4double position = -0.45*(fDetector->GetWorldSizeXY());
  fParticleGun->SetParticlePosition(G4ThreeVector(position,0.*cm,0.*cm));
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // 只在首次进行参数获取
  if(fRadiusZr < 0.0){
    fRadiusZr = fDetector->GetActRadius();
    fLengthZr = fDetector->GetActLength();
    fRotateZr = fDetector->GetActRotate();
  }
  
  G4double posX,posY,posZ;
  posZ = 180.*mm;
  
  // 正入射 圆面
  if(abs(fRotateZr) < 0.00001){
    do{
      posX = fRadiusZr * (2*G4UniformRand()-1);
      posY = fRadiusZr * (2*G4UniformRand()-1);
    } while(posX*posX + posY*posY > fRadiusZr*fRadiusZr);
  }
  else{ //侧入射
    posX = fRadiusZr * (2*G4UniformRand()-1);
    posY = 0.5 * fLengthZr * (2*G4UniformRand()-1);
  }

  fParticleGun->SetParticlePosition(G4ThreeVector(posX, posY, posZ));
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0.,0.,-1.));
  fParticleGun->GeneratePrimaryVertex(anEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
