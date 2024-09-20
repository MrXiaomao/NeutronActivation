#include "G4Types.hh"
#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

#include "G4UImanager.hh"
#include "Randomize.hh"

#include "DetectorConstruction.hh"
// #include "PhysicsList.hh"
#include "My_QGSP_BERT_HP.hh"
#include "ActionInitialization.hh"
#include "SteppingVerbose.hh"

#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
 
int main(int argc,char** argv) { 
  G4String macro  = "nothing";
  G4double ratote = 0.0; //锆帽旋转角度
  for ( G4int i=1; i<argc; i=i+2 ) {
    if      ( G4String(argv[i]) == "-m" ) macro   = argv[i+1];
    else if ( G4String(argv[i]) == "-Rotate" ) ratote = atof(argv[i+1]);
  }

  G4UIExecutive* ui = 0;
  if(macro == "nothing"){
    ui = new G4UIExecutive(argc, argv);
  }

  //choose the Random engine
  G4Random::setTheEngine(new CLHEP::RanecuEngine);
 
  // Construct the default run manager
#ifdef G4MULTITHREADED
  G4MTRunManager* runManager = new G4MTRunManager;
  G4int nThreads = G4Threading::G4GetNumberOfCores();
//  if (argc==3) nThreads = G4UIcommand::ConvertToInt(argv[2]);
  runManager->SetNumberOfThreads(1);
#else
  //my Verbose output class
  G4VSteppingVerbose::SetInstance(new SteppingVerbose);
  G4RunManager* runManager = new G4RunManager;
#endif

  // set mandatory initialization classes
  DetectorConstruction* det= new DetectorConstruction(ratote);
  runManager->SetUserInitialization(det);
  
  // PhysicsList* phys = new PhysicsList;
  G4VUserPhysicsList* phys = new My_QGSP_BERT_HP;
  runManager->SetUserInitialization(phys);
  
  runManager->SetUserInitialization(new ActionInitialization(det));    

  // Initialize visualization
  //
  G4VisManager* visManager = nullptr; 

  // get the pointer to the User Interface manager 
  G4UImanager* UI = G4UImanager::GetUIpointer();  

  if (macro!= "nothing")   // batch mode  
  {
    G4String command = "/control/execute ";
    UI->ApplyCommand(command+macro);
  }
  else           //define visualization and UI terminal for interactive mode
    { 
      visManager = new G4VisExecutive;
      visManager->Initialize();
      // interactive mode
      UI->ApplyCommand("/control/execute init_vis.mac");
      ui->SessionStart();
      delete ui;
    }

  // job termination 
  //
  delete visManager;
  delete runManager;

  return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
