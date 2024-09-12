
#include "G4Types.hh"

#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4SteppingVerbose.hh"
#include "Randomize.hh"

#include "DetectorConstruction.hh"
// #include "PhysicsList.hh"
#include "QGSP_BERT_HP.hh"
#include "ActionInitialization.hh"

#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"

#include "G4ParticleHPManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc,char** argv) {
  G4String macro  = "nothing";
  G4double ratote = 0.0; //锆帽旋转角度
  for ( G4int i=1; i<argc; i=i+2 ) {
    if      ( G4String(argv[i]) == "-m" ) macro = argv[i+1];
    else if ( G4String(argv[i]) == "-Rotate" ) ratote = atof(argv[i+1]);
  }
  
  //detect interactive mode (if no arguments) and define UI session
  G4UIExecutive* ui = 0;
  if(macro == "nothing") ui = new G4UIExecutive(argc,argv);

  //choose the Random engine
  G4Random::setTheEngine(new CLHEP::RanecuEngine);

  //use G4SteppingVerboseWithUnits
  G4int precision = 4;
  G4SteppingVerbose::UseBestUnit(precision);

  //construct the run manager
  auto runManager = G4RunManagerFactory::CreateRunManager();
  runManager->SetNumberOfThreads(1);

  // set mandatory initialization classes
  DetectorConstruction* det= new DetectorConstruction(ratote);
  runManager->SetUserInitialization(det);

  // PhysicsList* phys = new PhysicsList;
  G4VUserPhysicsList* phys = new QGSP_BERT_HP;
  runManager->SetUserInitialization(phys);

  runManager->SetUserInitialization(new ActionInitialization(det));
  /*
  // Replaced HP environmental variables with C++ calls
  G4ParticleHPManager::GetInstance()->SetSkipMissingIsotopes( false );
  G4ParticleHPManager::GetInstance()->SetDoNotAdjustFinalState( true );
  G4ParticleHPManager::GetInstance()->SetUseOnlyPhotoEvaporation( true );
  G4ParticleHPManager::GetInstance()->SetNeglectDoppler( false );
  G4ParticleHPManager::GetInstance()->SetProduceFissionFragments( false );
  G4ParticleHPManager::GetInstance()->SetUseWendtFissionModel( false );
  G4ParticleHPManager::GetInstance()->SetUseNRESP71Model( false );
  */
  //initialize visualization
  G4VisManager* visManager = nullptr;

  //get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  if (ui)  {
   //interactive mode
   visManager = new G4VisExecutive;
   visManager->Initialize();
   ui->SessionStart();
   delete ui;
  }
  else  {
   //batch mode
   G4String command = "/control/execute ";
   UImanager->ApplyCommand(command+macro);
  }

  //job termination
  delete visManager;
  delete runManager;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
