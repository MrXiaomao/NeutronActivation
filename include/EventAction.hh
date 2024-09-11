//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

#include "Parameter.hh"
#include <map>
#include <vector>
using namespace std;
using namespace myConsts;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
class G4ParticleDefinition;

class EventAction : public G4UserEventAction
{
  public:
    EventAction();
   ~EventAction();

  public:
    virtual void BeginOfEventAction(const G4Event*);
    virtual void EndOfEventAction(const G4Event*);
    
    void AddEdep (G4double Edep);
    void AddEflow(G4double Eflow);          
    void AddNewDacayTime(const G4ParticleDefinition* patticle,G4double time);
    void AddIronEdep (G4double edep,G4double time);
    void AddTimeEdep (G4double edep,G4double time);
    G4String GetParentDecayIon(G4double time); //�жϸôγ�������ʱ��������˥��ĸ������

    //ĸ��˥��ʱ�̣�<��������ʱ��>
    std::map<G4String, G4double> fIonDecayTime;
    
    // ĸ��˥��ʱ�䣬��ʱ�䴰�ڵ��ܳ�����
    vector<TimeEdep> fIonDecayTimeEnergy; 
    
    //��ĸ��˥���¼���ʱ�䴰�ڣ�̽�����е��ܳ���������
    std::map<G4String, G4double> fIonEdep; 

  private:
    G4double fTotalEnergyDeposit;
    G4double fTotalEnergyFlow;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
