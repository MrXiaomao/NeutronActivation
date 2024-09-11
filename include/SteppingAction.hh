// $Id: SteppingAction.hh 66241 2012-12-13 18:34:42Z gunter $
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"
#include "G4LogicalVolume.hh"
#include <map>

class G4ParticleDefinition;
class DetectorConstruction;
class EventAction;
class TrackingAction;
class StackAction;
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class SteppingAction : public G4UserSteppingAction
{
  public:
    SteppingAction(DetectorConstruction*,EventAction*,TrackingAction*,  StackAction*);
   ~SteppingAction();

    virtual void UserSteppingAction(const G4Step*);
    void  CountAndFixedPhysics(const G4Step* aStep);//统计强子核反应以及对部分重要强子反应做修正
    static void GeneratedataFileName(DetectorConstruction*);
  private:
    std::map<G4ParticleDefinition*,G4int> fParticleFlag;    
    DetectorConstruction* fDetector;
    EventAction*         fEventAction;
    TrackingAction*      fTrackAction;
    StackAction*      fStackAction;
    G4LogicalVolume* fScoringVolume;

    G4double lasttime;
    G4double lastedep;
    static G4String fileWholePath;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
