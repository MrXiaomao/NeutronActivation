
#ifndef OpNoviceStackingAction_H
#define OpNoviceStackingAction_H 1

#include "globals.hh"
#include "G4UserStackingAction.hh"

#include "G4StackManager.hh"
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class StackAction : public G4UserStackingAction
{
  public:
    StackAction();
    virtual ~StackAction();

  public:
    virtual G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack);
    virtual void NewStage();
    virtual void PrepareNewEvent();
    //获取当前径迹总数
    inline G4int GetTrackNum(){
      G4int Num = 0;
      Num = stackManager->GetNTotalTrack();
      return Num;
    };

    void AddNewTrack(G4Track* aTrack);
  private:
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
