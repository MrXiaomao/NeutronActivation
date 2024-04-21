#include "Run.hh"
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "HistoManager.hh"

#include "G4Threading.hh"
#include "G4AutoLock.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include <algorithm>  //sort
#include <fstream>

// #include "/home/xiaomao/Software/hdf5/hdf5-1.13.2/c++/src/H5Cpp.h"
// #include "H5Cpp.h"
// using namespace H5;
// #include <h5cpp/hdf5.hpp>
#include "Hdf5Function.h"
// mutex in a file scope

namespace {
  //Mutex to lock updating the global ion map
  G4Mutex ionIdMapMutex = G4MUTEX_INITIALIZER;
}  

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

std::map<G4String,G4int> Run::fgIonMap;
G4int Run::fgIonId = kMaxHisto1;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Run::Run(DetectorConstruction* det)
: G4Run(),
  fDetector(det), fParticle(nullptr), fEkin(0.)
{
  fEnergyDeposit = fEnergyDeposit2 = 0.;
  fEnergyFlow    = fEnergyFlow2    = 0.;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Run::~Run()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::Merge(std::map<G4String, ParticleData>& destinationMap,
                const std::map<G4String, ParticleData>& sourceMap) const
{
  for ( const auto& particleData : sourceMap ) {
    G4String name = particleData.first;
    const ParticleData& localData = particleData.second;   
    if ( destinationMap.find(name) == destinationMap.end()) {
      destinationMap[name]
       = ParticleData(localData.fCount, 
                      localData.fEmean, 
                      localData.fEmin, 
                      localData.fEmax,
                      localData.fTmean);
    }
    else {
      ParticleData& data = destinationMap[name];   
      data.fCount += localData.fCount;
      data.fEmean += localData.fEmean;
      G4double emin = localData.fEmin;
      if (emin < data.fEmin) data.fEmin = emin;
      G4double emax = localData.fEmax;
      if (emax > data.fEmax) data.fEmax = emax;
      data.fTmean = localData.fTmean;
    }   
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::Merge(std::map<G4String, vector<G4double>>& destinationMap,
            const std::map<G4String, vector<G4double>>& sourceMap) const
{
  std::map<G4String, vector<G4double>>::const_iterator itc;
  for (itc = sourceMap.begin(); itc != sourceMap.end(); ++itc) 
  {
    std::map<G4String, vector<G4double>>::iterator it = destinationMap.find(itc->first);

    //如果是新的时间，是则新增，否则合并到同一道址（视为同一事件）
    if ( it == fIronSpectrum.end()) {
      destinationMap[itc->first] = itc->second;
    }
    else{
      // 合并两个容器;
      G4int TotalSize = destinationMap[itc->first].size() + itc->second.size();
      destinationMap[itc->first].resize(TotalSize);
      destinationMap[itc->first].insert(destinationMap[itc->first].end(),itc->second.begin(),itc->second.end());
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::SetPrimary(G4ParticleDefinition* particle, G4double energy)
{ 
  fParticle = particle;
  fEkin = energy;
}
 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::CountProcesses(const G4VProcess* process) 
{
  G4String procName = process->GetProcessName();
  std::map<G4String,G4int>::iterator it = fProcCounter.find(procName);
  if ( it == fProcCounter.end()) {
    fProcCounter[procName] = 1;
  }
  else {
    fProcCounter[procName]++; 
  }
}
                  
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::ParticleCount(G4String name, G4double Ekin, G4double meanLife)
{
  std::map<G4String, ParticleData>::iterator it = fParticleDataMap1.find(name);
  if ( it == fParticleDataMap1.end()) {
    fParticleDataMap1[name] = ParticleData(1, Ekin, Ekin, Ekin, meanLife);
  }
  else {
    ParticleData& data = it->second;
    data.fCount++;
    data.fEmean += Ekin;
    //update min max
    G4double emin = data.fEmin;
    if (Ekin < emin) data.fEmin = Ekin;
    G4double emax = data.fEmax;
    if (Ekin > emax) data.fEmax = Ekin;
    data.fTmean = meanLife;
  }   
}
                 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::AddEdep(G4double edep)
{ 
  fEnergyDeposit += edep;
  fEnergyDeposit2 += edep*edep;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::AddTimeEdep(std::map<G4String, G4double> timeSpec)
{
  for(const auto& timeDep : timeSpec){
    std::map<G4String, vector<G4double>>::iterator it = fIronSpectrum.find(timeDep.first);
    if ( it == fIronSpectrum.end()) {
      vector<G4double> spec;
      spec.push_back(timeDep.second);
      fIronSpectrum[timeDep.first] = spec;
    }
    else{
      fIronSpectrum[timeDep.first].push_back(timeDep.second);
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::AddEflow(G4double eflow)
{ 
  fEnergyFlow += eflow;
  fEnergyFlow2 += eflow*eflow;
}                  
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::ParticleFlux(G4String name, G4double Ekin)
{
  std::map<G4String, ParticleData>::iterator it = fParticleDataMap2.find(name);
  if ( it == fParticleDataMap2.end()) {
    fParticleDataMap2[name] = ParticleData(1, Ekin, Ekin, Ekin, -1*ns);
  }
  else {
    ParticleData& data = it->second;
    data.fCount++;
    data.fEmean += Ekin;
    //update min max
    G4double emin = data.fEmin;
    if (Ekin < emin) data.fEmin = Ekin;
    G4double emax = data.fEmax;
    if (Ekin > emax) data.fEmax = Ekin;
    data.fTmean = -1*ns;
  }   
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4int Run::GetIonId(G4String ionName)
{
   G4AutoLock lock(&ionIdMapMutex);
      // updating the global ion map needs to be locked

   std::map<G4String,G4int>::const_iterator it = fgIonMap.find(ionName);
   if ( it == fgIonMap.end()) {
     fgIonMap[ionName] = fgIonId;
     if (fgIonId < kMaxHisto2) fgIonId++;
   }
   return fgIonMap[ionName];
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::Merge(const G4Run* run)
{
  const Run* localRun = static_cast<const Run*>(run);
  
  //primary particle info
  //
  fParticle = localRun->fParticle;
  fEkin     = localRun->fEkin;
  
  // accumulate sums
  //
  fEnergyDeposit   += localRun->fEnergyDeposit;  
  fEnergyDeposit2  += localRun->fEnergyDeposit2;
  fEnergyFlow      += localRun->fEnergyFlow;
  fEnergyFlow2     += localRun->fEnergyFlow2;
      
  //map: processes count
  for ( const auto& procCounter : localRun->fProcCounter ) {
    G4String procName = procCounter.first;
    G4int localCount = procCounter.second;
    if ( fProcCounter.find(procName) == fProcCounter.end()) {
      fProcCounter[procName] = localCount;
    }
    else {
      fProcCounter[procName] += localCount;
    }  
  }
  
  //map: created particles count
  Merge(fParticleDataMap1, localRun->fParticleDataMap1);    
  
  //map: particles flux count       
  Merge(fParticleDataMap2, localRun->fParticleDataMap2);    
  
  //map: IronName,dep
  Merge(fIronSpectrum,localRun->fIronSpectrum);

  G4Run::Merge(run); 
} 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
bool cmp(pair<G4double, G4double> a , pair<G4int, G4double> b)
{
	return a.first<b.first; //升序
}

void Run::EndOfRun() 
{
  G4int prec = 5, wid = prec + 2;  
  G4int dfprec = G4cout.precision(prec);
  
  //run condition
  //
  G4Material* material = fDetector->GetAbsorMaterial();
  G4double density = material->GetDensity();
   
  G4String Particle = fParticle->GetParticleName();    
  G4cout << "\n The run is " << numberOfEvent << " "<< Particle << " of "
         << G4BestUnit(fEkin,"Energy") << " through " 
         << G4BestUnit(fDetector->GetAbsorThickness(),"Length") << " of "
         << material->GetName() << " (density: " 
         << G4BestUnit(density,"Volumic Mass") << ")" << G4endl;

  if (numberOfEvent == 0) { G4cout.precision(dfprec);   return;}
             
  //frequency of processes
  //
  G4cout << "\n Process calls frequency :" << G4endl;
  G4int index = 0;
  for ( const auto& procCounter : fProcCounter ) {
     G4String procName = procCounter.first;
     G4int    count    = procCounter.second;
     G4String space = " "; if (++index%3 == 0) space = "\n";
     G4cout << " " << std::setw(20) << procName << "="<< std::setw(7) << count
            << space;
  }
  G4cout << G4endl;
  
  //particles count
  //
  G4cout << "\n List of generated particles:" << G4endl;
     
 for ( const auto& particleData : fParticleDataMap1 ) {
    G4String name = particleData.first;
    ParticleData data = particleData.second;
    G4int count = data.fCount;
    G4double eMean = data.fEmean/count;
    G4double eMin = data.fEmin;
    G4double eMax = data.fEmax;
    G4double meanLife = data.fTmean;
         
    G4cout << "  " << std::setw(13) << name << ": " << std::setw(7) << count
           << "  Emean = " << std::setw(wid) << G4BestUnit(eMean, "Energy")
           << "\t( "  << G4BestUnit(eMin, "Energy")
           << " --> " << G4BestUnit(eMax, "Energy") << ")";
    if (meanLife >= 0.)
      G4cout << "\tmean life = " << G4BestUnit(meanLife, "Time")   << G4endl;
    else {
      G4cout << "\tstable" << G4endl;
    }
 }

  G4cout << "\n List of Activted particles:" << G4endl;  
  for ( const auto& particleData : fParticleDataMap1 ) {
    G4String name = particleData.first;
    ParticleData data = particleData.second;
    G4int count = data.fCount;
    G4double meanLife = data.fTmean;
         
    if (meanLife >= 0.){
      G4cout << "  " << std::setw(13) << name << ": " << std::setw(7) << count
             << "\tmean life = " << G4BestUnit(meanLife, "Time")   << G4endl;
    }
 }

  //time-deposit,keV
  //按照key进行排序，升序
  // vector< pair<G4String, G4double> > sortTimeDep(fIronSpectrum.begin(),fIronSpectrum.end());//利用vector容器储存后再进行排序。 
  // sort(sortTimeDep.begin(),sortTimeDep.end(),cmp);
/*
  G4String FILE_NAME = "energyDep.h5";
  H5File *file = new H5File(FILE_NAME, H5F_ACC_TRUNC);

  Group *group = new Group(file->createGroup("Data"));
  

  double h5data[] = {1.0,2.0,3.0,4.0,5.0};
  hsize_t dims[1] = {5};
  DataSpace dataspace(1,dims);
  DataSet *dataset = new DataSet(
      file->createDataSet("/Data/Data1", PredType::NATIVE_DOUBLE, dataspace));

  dataset->write(h5data,PredType::NATIVE_DOUBLE);

  delete dataset;
  delete group;
  delete file;
*/
  double h5data[] = {1.1,2.8,3.6,4.1,5.0};
  Hdf5WriteValue write;
  write.CreateNewFile("first.h5");
  //
  write.CreateDataspace(1, 1, 5);
  write.CreateGroup("groupA");
  write.CreateDoubleDataset("datasetA");
  write.WriteDoubleValue(h5data);

  // 将各个核素的能量沉积数据输出
  G4String outPutPath = "../OutPut/";
  G4String fileName;
  fstream datafile;
  for ( const auto& oneIroEdep : fIronSpectrum ) {
    vector<G4double> energyEdp = oneIroEdep.second;
    fileName = outPutPath + oneIroEdep.first + ".Spectrum";
    // G4cout<<" fileName = "<<fileName<<G4endl;
    datafile.open(fileName, ios::out|ios::ate);
    if (!datafile.fail())
    {
      for(vector<G4double>::iterator iter = energyEdp.begin(); iter != energyEdp.end(); iter++)
      {
        if((*iter)>0.1*CLHEP::keV){
          datafile<<(*iter)<<G4endl;
        } 
      }
    }
    datafile.close();
  }

  // compute mean Energy deposited and rms
  //
  G4int TotNbofEvents = numberOfEvent;
  fEnergyDeposit /= TotNbofEvents; fEnergyDeposit2 /= TotNbofEvents;
  G4double rmsEdep = fEnergyDeposit2 - fEnergyDeposit*fEnergyDeposit;
  if (rmsEdep>0.) rmsEdep = std::sqrt(rmsEdep);
  else            rmsEdep = 0.;
  
  G4cout << "\n Mean energy deposit per event = "
         << G4BestUnit(fEnergyDeposit,"Energy") << ";  rms = "
         << G4BestUnit(rmsEdep,      "Energy") 
         << G4endl;
  
  // compute mean Energy flow and rms
  //
  fEnergyFlow /= TotNbofEvents; fEnergyFlow2 /= TotNbofEvents;
  G4double rmsEflow = fEnergyFlow2 - fEnergyFlow*fEnergyFlow;
  if (rmsEflow>0.) rmsEflow = std::sqrt(rmsEflow);
  else             rmsEflow = 0.;
  
  G4cout << " Mean energy flow per event    = "
         << G4BestUnit(fEnergyFlow,"Energy") << ";  rms = "
         << G4BestUnit(rmsEflow,   "Energy") 
         << G4endl;

 //particles flux
 //
 G4cout << "\n List of particles emerging from the target :" << G4endl;
     
 for ( const auto& particleData : fParticleDataMap2 ) {
    G4String name = particleData.first;
    ParticleData data = particleData.second;
    G4int count = data.fCount;
    G4double eMean = data.fEmean/count;
    G4double eMin = data.fEmin;
    G4double eMax = data.fEmax;
    G4double Eflow = data.fEmean/TotNbofEvents;        
         
    G4cout << "  " << std::setw(13) << name << ": " << std::setw(7) << count
           << "  Emean = " << std::setw(wid) << G4BestUnit(eMean, "Energy")
           << "\t( "  << G4BestUnit(eMin, "Energy")
           << " --> " << G4BestUnit(eMax, "Energy") 
           << ") \tEflow/event = " << G4BestUnit(Eflow, "Energy") << G4endl;
 }
 
 //histogram Id for populations
 //
 G4cout << "\n histo Id for populations :" << G4endl;

 // Update the histogram titles according to the ion map
 // and print new titles
 G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
 for ( const auto& ionMapElement : fgIonMap ) {
    G4String ionName = ionMapElement.first;
    G4int    h1Id   = ionMapElement.second;
    // print new titles
    G4cout << " " << std::setw(20) << ionName << "  id = "<< std::setw(3) << h1Id
           << G4endl;

    // update histogram ids
    if ( ! analysisManager->GetH1(h1Id) ) continue;
       // Skip inactive histograms, this is not necessary 
       // but it  makes the code safe wrt modifications in future 
    G4String title = analysisManager->GetH1Title(h1Id);
    title = ionName + title;
    analysisManager->SetH1Title(h1Id, title);
 } 
 G4cout << G4endl;
 
  //normalize histograms
  G4int ih = 2;
  G4double binWidth = analysisManager->GetH1Width(ih);
  G4double fac = (1./(numberOfEvent*binWidth))*(mm/MeV);
  analysisManager->ScaleH1(ih,fac);
  
  for (ih=14; ih<24; ih++) {
    binWidth = analysisManager->GetH1Width(ih);
    G4double unit = analysisManager->GetH1Unit(ih);  
    fac = (second/(binWidth*unit));    
    analysisManager->ScaleH1(ih,fac);
  }  
  
  //remove all contents in fProcCounter, fCount 
  fProcCounter.clear();
  fParticleDataMap1.clear();
  fParticleDataMap2.clear();
  fgIonMap.clear();
                          
  //restore default format         
  G4cout.precision(dfprec);   
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
