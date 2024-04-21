
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
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::AddEdep(G4double Edep)
{
  fTotalEnergyDeposit += Edep;
}

void EventAction::AddTimeEdep(G4double edep,G4double time)
{
  G4String ionName = GetParentDecayIon(time);
  if(ionName != "") fIonEdep[ionName] += edep;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::AddEflow(G4double Eflow)
{
  fTotalEnergyFlow += Eflow;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// 添加核素衰变事件的母核名称以及时刻
// 如果两次衰变的时间相隔很短，则认为子核衰变是瞬发的，应该视为同一事件。
void EventAction::AddNewDacayTime(const G4ParticleDefinition* particle,G4double time)
{
  // 不统计瞬发的核衰变事件
  if(time < gGountBeginTime) return;

  G4String name = particle->GetParticleName();

  //如果两次衰变的时间间隔大于指定数值，则认为是两个事件
  G4bool istimeExit = false;
  for ( const auto& IonDecayTime : fIonDecayTime) {
    G4double deltaTime = time - IonDecayTime.second;
    if(deltaTime>0. && deltaTime < gTimeWidth) {
      istimeExit = true;
      // G4cout<<" istimeExit=true "<<deltaTime<<G4endl;
      break;
    }
  }

  // 如果时刻不属于任何一次核事件，则添加该衰变的母核名称以及时刻
  if(istimeExit)  return;

  G4double meanLife = particle->GetPDGLifeTime();
  if ((G4IonTable::IsIon(particle))&&(meanLife != 0.)) {
      fIonDecayTime.insert(pair<G4String, G4double>(name,time));
  }
}

// 功能：查找该时刻属于哪一次母核的级联衰变事件
// time：当前沉积能量的时刻
G4String EventAction::GetParentDecayIon(G4double time)
{
  G4String ionName = "";
 
  // 如果time小于设定值，则认为是活化产生瞬发粒子，不统计能量
  if(time < gGountBeginTime) {
    return ionName;
  }

  // 在当前事件中的所有母核衰变事件，查找time在哪一次衰变事件的时间窗内，并返回相应的母核名称
  // 注意沉积能量的时刻一定位于母核衰变时刻之后，也就是只考虑deltaT>0.0
  G4bool istimeExit = false;
  // 记录离该时刻最近的那次衰变事件，母核名称，以及离母核衰变时刻的时间差。
  G4String nearlyIronName = "";
  G4double minDelta = 0.;
  nearlyIronName = fIonDecayTime.begin()->first;
  minDelta = (fIonDecayTime.begin())->second;
  
  for ( const auto& IonDecayTime : fIonDecayTime) {
    G4double deltaT = time - IonDecayTime.second;
    
    // 更新该次沉积能量前最近的一次核衰变事件
    if(deltaT>=0. && minDelta>deltaT){
      minDelta = deltaT;
      nearlyIronName = IonDecayTime.first;
    }

    // 统计衰变时刻的时间窗内，是否有该核素出现
    if(deltaT>=0. && deltaT < gTimeWidth) {
      istimeExit = true;
      ionName = IonDecayTime.first;
      nearlyIronName = ionName;
      minDelta = deltaT;
      // G4cout<<"GetParentDecayIon iron="<<ionName<<", deltaT = "<<(time - IonDecayTime.second)/CLHEP::ns<<" ns, ";
      break;
    }
  }
  
  //若这一次沉积能量时刻不属于任何一次衰变核素的衰变事件，则打印异常
  if(!istimeExit && fIonDecayTime.size()>0){
    // G4cout<<"NearlyDecayIronName = "<<nearlyIronName<<", deltaTime = "<<minDelta<<G4endl;
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
    run->AddIronEdep(fIonEdep);

    /*
    fstream datafile;
    datafile.open("../OutPut/TimeDepEvent.txt", ios::out | ios::app);
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
    }*/
  }

  // G4AnalysisManager::Instance()->FillH1(1,fTotalEnergyDeposit);
  // G4AnalysisManager::Instance()->FillH1(3,fTotalEnergyFlow);  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
