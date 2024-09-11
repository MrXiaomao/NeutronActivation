#include "EventAction.hh"

#include "Run.hh"
#include "HistoManager.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"

#include "G4ParticleDefinition.hh"
#include "G4IonTable.hh"

#include "Parameter.hh"
#include <fstream>
#include <stdio.h>
using namespace std;
using namespace myConsts;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction()
:G4UserEventAction(),
 fTotalEnergyDeposit(0.), fTotalEnergyFlow(0.)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event*)
{
  fTotalEnergyDeposit = 0.;
  fTotalEnergyFlow = 0.; 
  fIonDecayTime.clear();
  fIonEdep.clear();
  fIonDecayTimeEnergy.clear();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::AddEdep(G4double Edep)
{
  fTotalEnergyDeposit += Edep;
}


void EventAction::AddIronEdep(G4double edep,G4double time)
{
  G4String ionName = GetParentDecayIon(time);
  if(ionName != "") fIonEdep[ionName] += edep;
}

void EventAction::AddTimeEdep(G4double edep,G4double time)
{
  // AddIronEdep(edep,time);

  // �б�ǰ�¼��Ƿ�����ĳһ�����¼�
  G4bool isNotNewTime = false;
  vector<TimeEdep>::iterator iter;
  for( iter = fIonDecayTimeEnergy.begin(); iter != fIonDecayTimeEnergy.end(); iter++)    
  {
    G4double deltaTime = time - iter->depTime;
    if( deltaTime >= 0. && deltaTime < gTimeWidth){
      isNotNewTime = true;
      iter->Edep += edep;
      break;
    }
  }
  TimeEdep point = TimeEdep(time,edep);
  if(!isNotNewTime) fIonDecayTimeEnergy.push_back(point);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::AddEflow(G4double Eflow)
{
  fTotalEnergyFlow += Eflow;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// ��Ӻ���˥���¼���ĸ�������Լ�ʱ��
// �������˥���ʱ������̣ܶ�����Ϊ�Ӻ�˥����˲���ģ�Ӧ����Ϊͬһ�¼���
void EventAction::AddNewDacayTime(const G4ParticleDefinition* particle,G4double time)
{
  // ��ͳ��˲���ĺ�˥���¼�
  if(time < gGountBeginTime) return;

  // ��¼����ĸ��˥��ʱ��
  G4bool isNotNewTime = false;
  for ( const auto& DecayTime : fIonDecayTimeEnergy) {
    G4double deltaTime = time - DecayTime.depTime;
    if(deltaTime>=0. && deltaTime < gTimeWidth) {
      isNotNewTime = true;
      break;
    }
  }
  if(!isNotNewTime) fIonDecayTimeEnergy.push_back(TimeEdep(time,0.));

  G4String name = particle->GetParticleName();

  //�������˥���ʱ��������ָ����ֵ������Ϊ�������¼�
  G4bool istimeExit = false;
  for ( const auto& IonDecayTime : fIonDecayTime) {
    G4double deltaTime = time - IonDecayTime.second;
    if(deltaTime>=0. && deltaTime < gTimeWidth) {
      istimeExit = true;
      break;
    }
  }

  // ���ʱ�̲������κ�һ�κ��¼�������Ӹ�˥���ĸ�������Լ�ʱ��
  if(istimeExit)  return;

  G4double meanLife = particle->GetPDGLifeTime();
  if ((G4IonTable::IsIon(particle))&&(meanLife != 0.)) {
      fIonDecayTime.insert(pair<G4String, G4double>(name,time));
  }
}

// ���ܣ����Ҹ�ʱ��������һ��ĸ�˵ļ���˥���¼�
// time����ǰ����������ʱ��
G4String EventAction::GetParentDecayIon(G4double time)
{
  G4String ionName = "";
 
  // ���timeС���趨ֵ������Ϊ�ǻ����˲�����ӣ���ͳ������
  if(time < gGountBeginTime) {
    return ionName;
  }

  // �ڵ�ǰ�¼��е�����ĸ��˥���¼�������time����һ��˥���¼���ʱ�䴰�ڣ���������Ӧ��ĸ������
  // ע�����������ʱ��һ��λ��ĸ��˥��ʱ��֮��Ҳ����ֻ����deltaT>=0.0
  G4bool istimeExit = false;
  // ��¼���ʱ��������Ǵ�˥���¼���ĸ�����ƣ��Լ���ĸ��˥��ʱ�̵�ʱ��
  G4String nearlyIronName = "";
  G4double minDelta = 0.;
  nearlyIronName = fIonDecayTime.begin()->first;
  minDelta = (fIonDecayTime.begin())->second;
  
  for ( const auto& IonDecayTime : fIonDecayTime) {
    G4double deltaT = time - IonDecayTime.second;
    
    // ���¸ôγ�������ǰ�����һ�κ�˥���¼�
    if(deltaT>=0. && minDelta>deltaT){
      minDelta = deltaT;
      nearlyIronName = IonDecayTime.first;
    }

    // ͳ��˥��ʱ�̵�ʱ�䴰�ڣ��Ƿ��иú��س���
    if(deltaT>=0. && deltaT < gTimeWidth) {
      istimeExit = true;
      ionName = IonDecayTime.first;
      nearlyIronName = ionName;
      minDelta = deltaT;
      // G4cout<<"GetParentDecayIon iron="<<ionName<<", deltaT = "<<(time - IonDecayTime.second)/CLHEP::ns<<" ns, ";
      break;
    }
  }
  
  //����һ�γ�������ʱ�̲������κ�һ��˥����ص�˥���¼������ӡ�쳣
  if(!istimeExit && fIonDecayTime.size()>0){
    // G4cout<<"NearlyDecayIronName = "<<nearlyIronName<<", deltaTime = "<<minDelta<<G4endl;
  }

  return ionName;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* )
{
  Run* run = static_cast<Run*>(
             G4RunManager::GetRunManager()->GetNonConstCurrentRun());
             
  run->AddEdep (fTotalEnergyDeposit);             
  run->AddEflow(fTotalEnergyFlow);

  //��ͳ�Ƶ�������ʱ�Ž����ۼӣ�ֻ����ʱ����������ͳ�ơ�
  if(fIonDecayTime.size()>0){
    run->AddIronEdep(fIonEdep);
  }
  if(fIonDecayTimeEnergy.size()>0){
    run->AddEventEdep_Time(fIonDecayTimeEnergy);
    // for(auto timeEn:fIonDecayTimeEnergy){
    //   G4int eventNumber = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    //   G4cout<<"eventID = "<<eventNumber<<", EndOfEvent "<<timeEn.depTime<<" ns,  "<<timeEn.Edep/CLHEP::keV<<G4endl;
    // }
  }
  G4AnalysisManager::Instance()->FillH1(1,fTotalEnergyDeposit);
  G4AnalysisManager::Instance()->FillH1(3,fTotalEnergyFlow);  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
