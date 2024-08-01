// $Id: SteppingVerbose.cc 71654 2013-06-19 18:20:04Z maire $
// 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "SteppingVerbose.hh"

#include "G4SteppingManager.hh"
#include "G4ParticleTypes.hh"
#include "G4UnitsTable.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingVerbose::SteppingVerbose()
 : G4SteppingVerbose()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingVerbose::~SteppingVerbose()
{} 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingVerbose::TrackingStarted()
{  
  CopyState();
  
  G4int prec = G4cout.precision(3);
  
  //Step zero
  //  
  if( verboseLevel > 0 ){
    G4cout << std::right << std::setw( 6) << "Step#" << "  "
           << std::right << std::setw( 4) << "X" << "    "
           << std::right << std::setw( 4) << "Y" << "    "
           << std::right << std::setw( 4) << "Z" << "    "
           << std::right << std::setw( 15) << "Time(ns)" << "    "
           << std::right << std::setw( 6) << "KineE" << "      "
           << std::right << std::setw( 9) << "dEStep"
           << std::right << std::setw(10) << "StepLeng"  
           << std::right << std::setw(10) << "TrakLeng"
           << std::right << std::setw(10) << "Volume"
           << std::right << std::setw(10) << "Process"
           << G4endl;             

    G4cout << std::right << std::setw(6) << fTrack->GetCurrentStepNumber()
        << std::right << std::setw(6) << G4BestUnit(fTrack->GetPosition().x(),"Length")
        << std::right << std::setw(6) << G4BestUnit(fTrack->GetPosition().y(),"Length")
        << std::right << std::setw(6) << G4BestUnit(fTrack->GetPosition().z(),"Length")
        << std::right << std::setprecision(8) << std::setw(12) << fStep->GetPreStepPoint()->GetGlobalTime() //G4BestUnit(fTrack->GetGlobalTime(),"Time")
        << std::right << std::setprecision(6) << std::setw(8) << G4BestUnit(fTrack->GetKineticEnergy(),"Energy")
        << std::right << std::setprecision(6) << std::setw(8) << G4BestUnit(fStep->GetTotalEnergyDeposit(),"Energy")
        << std::right << std::setprecision(2) << std::setw(5) << G4BestUnit(fStep->GetStepLength(),"Length")
        << std::right << std::setprecision(2) << std::setw(5) << G4BestUnit(fTrack->GetTrackLength(),"Length")
        << std::right << std::setprecision(2) << std::setw(5) << fTrack->GetVolume()->GetName()
        << "   initStep" << G4endl;        
  }
  G4cout.precision(prec);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingVerbose::StepInfo()
{  
  CopyState();
    
  G4int prec = G4cout.precision(3);

  if( verboseLevel >= 1 ){
    if( verboseLevel >= 4 ) VerboseTrack();
    if( verboseLevel >= 3 ){
      G4cout << G4endl;    
      G4cout << std::right << std::setw( 5) << "#Step#"     << " "
             << std::setw( 8) << "X"          << "    "
             << std::setw( 8) << "Y"          << "    "  
             << std::setw( 8) << "Z"          << "    "
             << std::setw( 9) << "KineE"      << " "
             << std::setw( 9) << "dEStep"     << " "  
             << std::setw(10) << "StepLeng"     
             << std::setw(10) << "TrakLeng" 
             << std::setw(10) << "Volume"    << "  "
             << std::setw(10) << "Process"   << G4endl;                  
    }

    G4cout << std::right << std::setw( 6) << fTrack->GetCurrentStepNumber()
        << std::right << std::setw(6) << G4BestUnit(fTrack->GetPosition().x(),"Length")
        << std::right << std::setw(6) << G4BestUnit(fTrack->GetPosition().y(),"Length")
        << std::right << std::setw(6) << G4BestUnit(fTrack->GetPosition().z(),"Length")
        << std::right << std::setprecision(8) << std::setw(12) << fStep->GetPostStepPoint()->GetGlobalTime()
        << std::right << std::setprecision(6) << std::setw(8) << G4BestUnit(fTrack->GetKineticEnergy(),"Energy")
        << std::right << std::setprecision(6) << std::setw(8) << G4BestUnit(fStep->GetTotalEnergyDeposit(),"Energy")
        << std::right << std::setprecision(2) << std::setw(5) << G4BestUnit(fStep->GetStepLength(),"Length")
        << std::right << std::setprecision(2) << std::setw(5) << G4BestUnit(fTrack->GetTrackLength(),"Length")
        << std::right << std::setw(10) << fTrack->GetVolume()->GetName();

    const G4VProcess* process 
                      = fStep->GetPostStepPoint()->GetProcessDefinedStep();
    G4String procName = " UserLimit";
    if (process) procName = process->GetProcessName();
    if (fStepStatus == fWorldBoundary) procName = "OutOfWorld";
    G4cout << "   " << std::setw(10) << procName;
    G4cout << G4endl;

    if (verboseLevel == 2) {
      const std::vector<const G4Track*>* secondary 
                                    = fStep->GetSecondaryInCurrentStep();
      size_t nbtrk = (*secondary).size();
      if (nbtrk) {
        G4cout << "\n    :----- List of secondaries ----------------" << G4endl;
        G4cout.precision(4);
        for (size_t lp=0; lp<(*secondary).size(); lp++) {
          G4cout << "   "
                 << std::setw(13)                 
                 << (*secondary)[lp]->GetDefinition()->GetParticleName()
                 << ":  energy ="
                 << std::setw(6)
                 << G4BestUnit((*secondary)[lp]->GetKineticEnergy(),"Energy")
                 << "  time ="
                 << std::setw(6)
                 << G4BestUnit((*secondary)[lp]->GetGlobalTime(),"Time");
          G4cout << G4endl;
        }
              
        G4cout << "    :------------------------------------------\n" << G4endl;
      }
    }
    
  }
  G4cout.precision(prec);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
