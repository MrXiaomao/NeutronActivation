
#include "TrackingAction.hh"

#include "Run.hh"
#include "EventAction.hh"
#include "HistoManager.hh"

#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4StepStatus.hh"
#include "G4ParticleTypes.hh"
#include "G4IonTable.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "Parameter.hh"
using namespace myConsts;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

TrackingAction::TrackingAction(EventAction* event)
:G4UserTrackingAction(), fEventAction(event)
{
   fTimeBirth = fTimeEnd = 0.;
   fTrackEdep = 0.0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

TrackingAction::~TrackingAction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
  Run* run = static_cast<Run*>(
       G4RunManager::GetRunManager()->GetNonConstCurrentRun());

  G4ParticleDefinition* particle = track->GetDefinition();
  G4String name     = particle->GetParticleName();
  G4double meanLife = particle->GetPDGLifeTime();
  G4double ekin     = track->GetKineticEnergy();
  fTimeBirth        = track->GetGlobalTime();
  fTrackEdep        = 0.0;
  
  if(fTimeBirth > gTarckMaxTimeLimit) {
     G4Track* aTrack = (G4Track*)track;
     aTrack->SetTrackStatus(fStopAndKill);
  }

  //count secondary particles
  if (track->GetTrackID() > 1)  run->ParticleCount(name,ekin,meanLife);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void TrackingAction::PostUserTrackingAction(const G4Track* track)
{
  Run* run = static_cast<Run*>(
       G4RunManager::GetRunManager()->GetNonConstCurrentRun());
  
  G4AnalysisManager* analysis = G4AnalysisManager::Instance();
  
  const G4ParticleDefinition* particle = track->GetParticleDefinition();
  G4String name     = particle->GetParticleName();
  G4double meanLife = particle->GetPDGLifeTime();
  G4double ekin     = track->GetKineticEnergy();
  fTimeEnd         = track->GetGlobalTime();
  if ((particle->GetPDGStable())&&(ekin == 0.)) fTimeEnd = DBL_MAX;
  
  // count population of ions with meanLife > 0.
  if ((G4IonTable::IsIon(particle))&&(meanLife != 0.)) {
    G4int id = run->GetIonId(name);
    G4double unit = analysis->GetH1Unit(id);
    G4double tmin = analysis->GetH1Xmin(id)*unit;
    G4double tmax = analysis->GetH1Xmax(id)*unit;
    G4double binWidth = analysis->GetH1Width(id)*unit;

    G4double t1 = std::max(fTimeBirth,tmin);
    G4double t2 = std::min(fTimeEnd  ,tmax);
    for (G4double time = t1; time<t2; time+= binWidth)
       analysis->FillH1(id,time);
  }

  run->ParticleFlux(name,ekin);
  
  // limit the particle life
  if(fTrackEdep>gTrackEdepThreshold && 
     fTimeBirth>gGountBeginTime && fTimeBirth<gTarckMaxTimeLimit){
         // run->AddTrackEdep_Time(fTrackEdep,fTimeBirth);
         //get the Event Number
         // G4int eventNumber =
         // G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
         // G4cout<<"eventID = "<<eventNumber<<", EndTrack TotalEnergy="<<fTrackEdep/keV
         //       <<" fTimeBirth="<< std::setprecision(8)<< std::setw(12)<<fTimeBirth<<G4endl;
     } 

  // keep only emerging particles
  G4StepStatus status = track->GetStep()->GetPostStepPoint()->GetStepStatus();
  if (status != fWorldBoundary) return; 

  fEventAction->AddEflow(ekin);

 // histograms: energy flow and activities of emerging particles
 
 G4int ih1 = 0, ih2 = 0; 
 G4String type   = particle->GetParticleType();      
 G4double charge = particle->GetPDGCharge();
 G4double time   = track->GetGlobalTime();
 if (charge > 3.)  {ih1 = 10; ih2 = 20;}
 else if (particle == G4Gamma::Gamma())       {ih1 = 4;  ih2 = 14;}
 else if (particle == G4Electron::Electron()) {ih1 = 5;  ih2 = 15;}
 else if (particle == G4Positron::Positron()) {ih1 = 5;  ih2 = 15;}
 else if (particle == G4Neutron::Neutron())   {ih1 = 6;  ih2 = 16;}
 else if (particle == G4Proton::Proton())     {ih1 = 7;  ih2 = 17;}
 else if (particle == G4Deuteron::Deuteron()) {ih1 = 8;  ih2 = 18;}
 else if (particle == G4Alpha::Alpha())       {ih1 = 9;  ih2 = 19;}
 else if (type == "nucleus")                  {ih1 = 10; ih2 = 20;}
 else if (type == "baryon")                   {ih1 = 11; ih2 = 21;}
 else if (type == "meson")                    {ih1 = 12; ih2 = 22;}
 else if (type == "lepton")                   {ih1 = 13; ih2 = 23;};
 if (ih1 > 0) analysis->FillH1(ih1,ekin);
 if (ih2 > 0) analysis->FillH1(ih2,time);

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
