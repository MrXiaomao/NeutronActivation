
#include "EventAction.hh"

#include "Run.hh"
#include "HistoManager.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"

#include <fstream>
#include <stdio.h>
using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction()
:G4UserEventAction(),
 fTotalEnergyDeposit(0.), fTotalEnergyFlow(0.)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event*)
{
  fTotalEnergyDeposit = 0.;
  fTotalEnergyFlow = 0.; 
  fDecaytimeSpectrum[0] = 0.0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::AddEdep(G4double Edep)
{
  fTotalEnergyDeposit += Edep;
}

void EventAction::AddTimeEdep(G4double edep,G4double time)
{
  // cout<<"time ="<<time/1000<<"ms "<<edep<<endl;
  G4double time_microsecond = time/1000;
  G4int pos = 0;

  int iterator_i = 0;
  std::map<G4double, G4double>::const_iterator itc;
  for (itc = fDecaytimeSpectrum.begin(); 
       itc != fDecaytimeSpectrum.end(); ++itc) {
       if(time_microsecond >= itc->first) pos = iterator_i;
       iterator_i++;
  }
  fDecaytimeSpectrum[pos] += edep;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::AddEflow(G4double Eflow)
{
  fTotalEnergyFlow += Eflow;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::AddNewDacayTime(G4double time)
{
  G4double time_microsecond = time;

  std::map<G4double, G4double>::iterator it = fDecaytimeSpectrum.find(time_microsecond);
  //如果是新的时间，则新增
  if ( it == fDecaytimeSpectrum.end()) {
    fDecaytimeSpectrum[time_microsecond] = 0.0;
  }

  // cout<<"AddNewDacayTime "<<time/1000<<"ms "<<" "<<time_microsecond<<endl;
  // for ( const auto& timeDep : fDecaytimeSpectrum ) {
  //         long long time = timeDep.first;
  //         G4double depE = timeDep.second;
  //         cout<<"AddNewDacayTime "
  //             <<setiosflags(ios::left)<<setw(13)<<time
  //             <<setiosflags(ios::left)<<setw(12)<<depE<<G4endl;
  // }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* evt)
{
  Run* run = static_cast<Run*>(
             G4RunManager::GetRunManager()->GetNonConstCurrentRun());
             
  run->AddEdep (fTotalEnergyDeposit);             
  run->AddEflow(fTotalEnergyFlow);

  //当统计到有数据时才进行累加，只有零时刻有数据则不统计。
  if(fDecaytimeSpectrum.size()>1){
    run->AddTimeEdep(fDecaytimeSpectrum);
      
      /*fstream datafile;
      datafile.open("TimeDepEvent.txt", ios::out | ios::app);
      if (!datafile.fail())
      {
        for ( const auto& timeDep : fDecaytimeSpectrum ) {
          G4double time = timeDep.first;
          G4double depE = timeDep.second;
          datafile<<setiosflags(ios::left)<<setw(13)<<evt->GetEventID()
                  <<setiosflags(ios::left)<<setw(13)<<time
                  <<setiosflags(ios::left)<<setw(12)<<depE<<G4endl;
        }
        datafile.close();
      }*/
  }

  G4AnalysisManager::Instance()->FillH1(1,fTotalEnergyDeposit);
  G4AnalysisManager::Instance()->FillH1(3,fTotalEnergyFlow);  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
