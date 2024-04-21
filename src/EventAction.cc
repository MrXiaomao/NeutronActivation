
#include "EventAction.hh"

#include "Run.hh"
#include "HistoManager.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"

#include "G4ParticleDefinition.hh"
#include "G4IonTable.hh"

#include <fstream>
#include <stdio.h>
using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction()
:G4UserEventAction(),
 fTotalEnergyDeposit(0.), fTotalEnergyFlow(0.)
{
  fstream datafile;
  datafile.open("TimeDepEvent.txt", ios::out|ios::ate);
  datafile.close();
}

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
  // fIonDecayTime[0] = 0.0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::AddEdep(G4double Edep)
{
  fTotalEnergyDeposit += Edep;
}

void EventAction::AddTimeEdep(G4double edep,G4double time)
{
  edep /= CLHEP::keV;
  G4String ionName = GetParentDecayIon(time);
  // G4cout<<"AddTimeEdep "<<time<<" ns, dep= "<<edep<<"keV"<<G4endl;
  if(ionName != "") fIonEdep[ionName] += edep;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::AddEflow(G4double Eflow)
{
  fTotalEnergyFlow += Eflow;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// 记录核素衰变的放射性时间
// 如果两次衰变的时间相隔很短，则认为子核衰变是瞬发的，应该视为同一事件。
void EventAction::AddNewDacayTime(const G4ParticleDefinition* particle,G4double time)
{
  //如果两次衰变的时间间隔大于指定数值，则认为是两个事件
  G4bool istimeExit = false;
  for ( const auto& IonDecayTime : fIonDecayTime) {
    if(time - IonDecayTime.second < 20) {
      istimeExit = true;
      break;
    }
  }

  // 如果是一个新的时间，则添加
  if(istimeExit)  return;

  G4double meanLife = particle->GetPDGLifeTime();
  if ((G4IonTable::IsIon(particle))&&(meanLife != 0.)) {
      G4String name = particle->GetParticleName();
      fIonDecayTime.insert(pair<G4String, G4double>(name,time));
    // G4cout<<"IonName= "<<name<<" time="<<time<<"ns"<<G4endl;
  }
}

//查找该时刻属于哪一次衰变时间
G4String EventAction::GetParentDecayIon(G4double time)
{
  G4String ionName = "";
 
  // 如果time小于阈值，则认为是活化产生瞬发粒子的能量沉积
  G4double time_Threshold = 10*CLHEP::s;
  if(time < time_Threshold) {
    return ionName;
  }

  // 如果该次沉积能量的时间前面deltaT时间内不存在一次衰变时间，则认为不属于这次衰变事件。
  G4bool istimeExit = false;
  for ( const auto& IonDecayTime : fIonDecayTime) {
    if(time - IonDecayTime.second < 20) {
      istimeExit = true;
      ionName = IonDecayTime.first;
      // G4cout<<"GetParentDecayIon iron="<<ionName<<", deltaT = "<<(time - IonDecayTime.second)/CLHEP::ns<<" ns, ";
      break;
    }
  }
  
  //若这一次沉积能量时刻不属于任何一次衰变核素的衰变事件，则打印异常
  if(!istimeExit){
    if(time>time_Threshold) {
      G4cout<<"Warning from G4String EventAction::GetParentIon(G4double time): "<<time<<G4endl;
    }
  }

  return ionName;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* evt)
{
  Run* run = static_cast<Run*>(
             G4RunManager::GetRunManager()->GetNonConstCurrentRun());
             
  run->AddEdep (fTotalEnergyDeposit);             
  run->AddEflow(fTotalEnergyFlow);

  //当统计到有数据时才进行累加，只有零时刻有数据则不统计。
  if(fIonDecayTime.size()>0){
    run->AddTimeEdep(fIonEdep);

    fstream datafile;
    datafile.open("TimeDepEvent.txt", ios::out | ios::app);
    if (!datafile.fail())
    {
      for ( const auto& ironDep : fIonEdep ) {
        G4String ironName = ironDep.first;
        G4double depE = ironDep.second;
        datafile<<setiosflags(ios::left)<<setw(13)<<evt->GetEventID()
                <<setiosflags(ios::left)<<setw(13)<<ironName
                <<setiosflags(ios::left)<<setw(12)<<depE<<G4endl;
      }
      datafile.close();
    }
  }

  G4AnalysisManager::Instance()->FillH1(1,fTotalEnergyDeposit);
  G4AnalysisManager::Instance()->FillH1(3,fTotalEnergyFlow);  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
