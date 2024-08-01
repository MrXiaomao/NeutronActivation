// $Id: SteppingAction.cc 71404 2013-06-14 16:56:38Z maire $
// 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "SteppingAction.hh"
#include "DetectorConstruction.hh"
#include "Run.hh"
#include "EventAction.hh"
#include "TrackingAction.hh"
#include "HistoManager.hh"

#include "G4RunManager.hh"
#include "Parameter.hh"             
using namespace myConsts;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(DetectorConstruction* det, EventAction* event,TrackingAction* track)
: G4UserSteppingAction(), fDetector(det), fEventAction(event),fTrackAction(track),fScoringVolume(0)
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::~SteppingAction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* aStep)
{
  // count processes
  // 
  const G4StepPoint* endPoint = aStep->GetPostStepPoint();
  const G4VProcess* process   = endPoint->GetProcessDefinedStep();
  Run* run = static_cast<Run*>(
        G4RunManager::GetRunManager()->GetNonConstCurrentRun());
  run->CountProcesses(process);

  if (!fScoringVolume) {
    fScoringVolume = fDetector->GetScoringVolume();   
  }
  
  //写入最新的衰变时间
  G4double decayTime = aStep->GetPostStepPoint()->GetGlobalTime();
  if(process->GetProcessName() == "RadioactiveDecay") {
    fEventAction->AddNewDacayTime(aStep->GetTrack()->GetParticleDefinition(),decayTime);
  }

  // get volume of the current step
  G4LogicalVolume* volume 
    = aStep->GetPreStepPoint()->GetTouchableHandle()
      ->GetVolume()->GetLogicalVolume();

  if(volume != fScoringVolume) return;


  // energy deposit
  //
  G4double edepStep = aStep->GetTotalEnergyDeposit();
  if (edepStep <= 0.) return; 

  
  fEventAction->AddEdep(edepStep);
  fTrackAction->AddTrackEdep(edepStep);
  //只统计衰变之类的放射性，因此这里把短时间都不做统计
  if(decayTime>gGountBeginTime){
    fEventAction->AddTimeEdep(edepStep,decayTime);
  }
  
  
 //longitudinal profile of deposited energy
 //randomize point of energy deposition
 //
 G4ThreeVector prePoint  = aStep->GetPreStepPoint() ->GetPosition();
 G4ThreeVector postPoint = aStep->GetPostStepPoint()->GetPosition();
 G4ThreeVector point = prePoint + G4UniformRand()*(postPoint - prePoint);
 G4double x = point.x();
 G4double xshifted = x + 0.5*fDetector->GetAbsorThickness();
 G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
 analysisManager->FillH1(2, xshifted, edepStep);   
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
