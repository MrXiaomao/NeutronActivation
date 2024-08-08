// $Id: SteppingAction.cc 71404 2013-06-14 16:56:38Z maire $
// 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "SteppingAction.hh"
#include "DetectorConstruction.hh"
#include "Run.hh"
#include "EventAction.hh"
#include "TrackingAction.hh"
#include "StackAction.hh"
#include "HistoManager.hh"
#include "G4HadronicProcess.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"

#include "G4RunManager.hh"
#include "Parameter.hh"             
using namespace myConsts;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// 中子能量,MeV 
G4double en_2n_Zr90[9] =      {14.8, 15.3, 16.3, 17.8, 18.1, 19.3, 19.4, 19.9, 20.6};
// 反应道激发态截面占比, Zr90(n,2n)Zr89m
G4double cs_2nration_Zr89m[9] = {0.19, 0.21, 0.20, 0.19, 0.21, 0.19, 0.19, 0.17, 0.17};

// 中子能量,MeV 
G4double en_2n_Ce140[8] = {13.47, 13.64, 13.88, 14.05, 14.28, 14.47, 14.68, 14.86};
// 反应道激发态截面, Ce140(n,2n)Ce139m
G4double cs_2nRatio_Ce139m[8] = {0.451150157, 0.500972384, 0.483178654, 0.44856661, 0.456, 
                                0.482989403, 0.528856826, 0.598006645};

// 中子能量,MeV 
G4double en_np_Y89[4] = {13.5, 14.0, 14.5, 15.0};
// 反应道激发态截面，Zr90(n,np)Y89m
G4double cs_npRatio_Y89m[4] = {0.237, 0.314, 0.427, 0.424};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(DetectorConstruction* det, EventAction* event,TrackingAction* track, StackAction* stack)
: G4UserSteppingAction(), 
fDetector(det), fEventAction(event),fTrackAction(track),fStackAction(stack),
fScoringVolume(0)
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::~SteppingAction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* aStep)
{
  CountAndFixedPhysics(aStep);
  // count processes
  // 
  const G4StepPoint* endPoint = aStep->GetPostStepPoint();
  const G4VProcess* process   = endPoint->GetProcessDefinedStep();
  Run* run = static_cast<Run*>(
        G4RunManager::GetRunManager()->GetNonConstCurrentRun());
  run->CountProcesses(process);

  if (!fScoringVolume) {
    fScoringVolume = fDetector->GetScoringVolume();   
  }
  
  //写入最新的衰变时间
  G4double decayTime = aStep->GetPostStepPoint()->GetGlobalTime();
  if(process->GetProcessName() == "RadioactiveDecay") {
    fEventAction->AddNewDacayTime(aStep->GetTrack()->GetParticleDefinition(),decayTime);
  }

  // get volume of the current step
  G4LogicalVolume* volume 
    = aStep->GetPreStepPoint()->GetTouchableHandle()
      ->GetVolume()->GetLogicalVolume();

  if(volume != fScoringVolume) return;


  // energy deposit
  //
  G4double edepStep = aStep->GetTotalEnergyDeposit();
  if (edepStep <= 0.) return; 

  
  fEventAction->AddEdep(edepStep);
  fTrackAction->AddTrackEdep(edepStep);
  //只统计衰变之类的放射性，因此这里把短时间都不做统计
  if(decayTime>gGountBeginTime){
    fEventAction->AddTimeEdep(edepStep,decayTime);
  }
  
  
 //longitudinal profile of deposited energy
 //randomize point of energy deposition
 //
 G4ThreeVector prePoint  = aStep->GetPreStepPoint() ->GetPosition();
 G4ThreeVector postPoint = aStep->GetPostStepPoint()->GetPosition();
 G4ThreeVector point = prePoint + G4UniformRand()*(postPoint - prePoint);
 G4double x = point.x();
 G4double xshifted = x + 0.5*fDetector->GetAbsorThickness();
 G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
 analysisManager->FillH1(2, xshifted, edepStep);   
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void  SteppingAction::CountAndFixedPhysics(const G4Step* aStep)
{
 Run* run 
   = static_cast<Run*>(G4RunManager::GetRunManager()->GetNonConstCurrentRun());
         
  // count processes
  // 
  const G4StepPoint* endPoint = aStep->GetPostStepPoint();
  G4VProcess* process   = 
                   const_cast<G4VProcess*>(endPoint->GetProcessDefinedStep());
  run->CountProcesses(process);
  
  // check that an real interaction occured (eg. not a transportation)
  G4StepStatus stepStatus = endPoint->GetStepStatus();
  G4bool transmit = (stepStatus==fGeomBoundary || stepStatus==fWorldBoundary);
  if (transmit) return;
  
  //energy-momentum balance initialisation
  //
  const G4StepPoint* prePoint = aStep->GetPreStepPoint();
  G4double Q             = - prePoint->GetKineticEnergy();
  G4ThreeVector Pbalance = - prePoint->GetMomentum();
  
  //initialisation of the nuclear channel identification
  //
  G4ParticleDefinition* particle = aStep->GetTrack()->GetDefinition();
  G4String partName = particle->GetParticleName();
  G4String nuclearChannel = partName;
  G4HadronicProcess* hproc = dynamic_cast<G4HadronicProcess*>(process);
  const G4Isotope* target = NULL;
  
  //当不存在强子核反应过程，则直接跳过
  if (hproc) target = hproc->GetTargetIsotope();
  else return;

  G4String targetName = "XXXX";  
  if (target) targetName = target->GetName();
  nuclearChannel += " + " + targetName + " --> ";
  // if (targetName == "XXXX") run->SetTargetXXX(true);
    
  //scattered primary particle (if any)
  //
  G4AnalysisManager* analysis = G4AnalysisManager::Instance();
  G4int ih = 1;
  if (aStep->GetTrack()->GetTrackStatus() == fAlive) {
    G4double energy = endPoint->GetKineticEnergy();      
    analysis->FillH1(ih,energy);
    //
    G4ThreeVector momentum = endPoint->GetMomentum();
    Q        += energy;
    Pbalance += momentum;
    //
    nuclearChannel += partName + " + ";
  }  
  
  //secondaries
  //
  const std::vector<const G4Track*>* secondary 
                                    = aStep->GetSecondaryInCurrentStep();  
  for (size_t lp=0; lp<(*secondary).size(); lp++) {
    particle = (*secondary)[lp]->GetDefinition(); 
    G4String name   = particle->GetParticleName();
    // G4String type   = particle->GetParticleType();      
    // G4double energy = (*secondary)[lp]->GetKineticEnergy();
    // run->ParticleCount(name,energy);

    //count e- from internal conversion together with gamma
    if (particle == G4Electron::Electron()) particle = G4Gamma::Gamma();
    //particle flag
    fParticleFlag[particle]++;
  }

  // nuclear channel
  const G4int kMax = 16;  
  const G4String conver[] = {"0","","2 ","3 ","4 ","5 ","6 ","7 ","8 ","9 ",
                             "10 ","11 ","12 ","13 ","14 ","15 ","16 "};
  std::map<G4ParticleDefinition*,G4int>::iterator ip;               
  for (ip = fParticleFlag.begin(); ip != fParticleFlag.end(); ip++) {
    particle = ip->first;
    G4String name = particle->GetParticleName();      
    G4int nb = ip->second;
    if (nb > kMax) nb = kMax;   
    G4String Nb = conver[nb];    
    if (particle == G4Gamma::Gamma()) {
    //  run->CountGamma(nb);
     Nb = "N ";
     name = "gamma or e-";
    } 
    if (ip != fParticleFlag.begin()) nuclearChannel += " + ";
    nuclearChannel+= Nb + name;
  }

  G4String oldChannel = nuclearChannel;
    //Zr90(n,2n)Zr89
  if(oldChannel.find("2 neutron") != G4String::npos && (targetName == "Zr90"))
    {
        //抽样决定产生基态和激发态两种粒子
        G4double trial = G4UniformRand(); 
        G4double exiEnergy = 0.0;

        // 根据能量线性插值给出激发态比例
        G4double neutronEnergy = aStep->GetPreStepPoint()->GetKineticEnergy();
        G4double ratio = 0.0;
        for(int i=0; i<9; i++){
          if(neutronEnergy <= en_2n_Zr90[0]) {
            ratio = cs_2nration_Zr89m[0];
            break;
          }
          if(neutronEnergy >= en_2n_Zr90[8]){
            ratio = cs_2nration_Zr89m[8];
            break;
          }
          if(neutronEnergy <= en_2n_Zr90[i]){
            ratio = cs_2nration_Zr89m[i-1] + 
              (cs_2nration_Zr89m[i] - cs_2nration_Zr89m[i-1])*(neutronEnergy - en_2n_Zr90[i-1])
              /(en_2n_Zr90[i] - en_2n_Zr90[i-1]);
            break;
          }
        }

        //抽样给出基态/激发态的反应道
        if(trial<ratio) {
          exiEnergy = 587.82*CLHEP::keV;
          if(oldChannel.find("2 neutron + N gamma or e-") != G4String::npos){
              oldChannel.replace(oldChannel.find("or e-"), 5, "or e- + Zr89[587.82]");
          }
          else if(oldChannel.find("2 neutron") != G4String::npos){
             oldChannel.replace(oldChannel.find("2 neutron"), 9, "2 neutron + Zr89[587.82]");
          }
        }
        else   {
          if(oldChannel.find("2 neutron + N gamma or e-") != G4String::npos){
              oldChannel.replace(oldChannel.find("or e-"), 5, "or e- + Zr89");
          }
          else if(oldChannel.find("2 neutron") != G4String::npos){
             oldChannel.replace(oldChannel.find("2 neutron"), 9, "2 neutron + Zr89");
          }
        }
        oldChannel = "new: " + oldChannel;
        // run->CountNuclearChannel(oldChannel, Q); //新增一个统计Zr89[587.82]

        //产生新的粒子
        G4ParticleDefinition* ion = G4IonTable::GetIonTable()->GetIon(40, 89, exiEnergy);
        
        G4String name     = ion->GetParticleName();
        G4double meanLife = ion->GetPDGLifeTime();
        run->ParticleCount(name, 1.0*CLHEP::keV, meanLife);
        G4ThreeVector momentumDirection = aStep->GetPreStepPoint()->GetMomentumDirection();
        G4double kineticEnergy = 1.0*CLHEP::keV;
        G4DynamicParticle* const residualNucleus = new G4DynamicParticle(ion, momentumDirection,  kineticEnergy);
        G4double time = aStep->GetTrack()->GetGlobalTime();
        G4ThreeVector currentPosition = aStep->GetPreStepPoint()->GetPosition();
        G4Track* aSecondary = new G4Track(residualNucleus,  time, currentPosition);
        aSecondary->SetParentID(0);//aStep->GetTrack()->GetTrackID()
        aSecondary->SetTrackID(1); //fStackAction->GetTrackNum()+1
        fStackAction->AddNewTrack(aSecondary); 
    }

    //Ce140(n,2n)Ce139
  if(nuclearChannel.find("2 neutron") != G4String::npos && (targetName == "Ce140"))
    {
        //抽样决定产生基态和激发态两种粒子
        G4double trial = G4UniformRand(); 
        G4double exiEnergy = 0.0;

        // 根据能量线性插值给出激发态比例
        G4double neutronEnergy = aStep->GetPreStepPoint()->GetKineticEnergy();
        G4double ratio = 0.0;
        for(int i=0; i<8; i++){
          if(neutronEnergy <= en_2n_Ce140[0]) {
            ratio = cs_2nRatio_Ce139m[0];
            break;
          }
          if(neutronEnergy >= en_2n_Ce140[7]){
            ratio = cs_2nRatio_Ce139m[7];
            break;
          }
          if(neutronEnergy <= en_2n_Ce140[i]){
            ratio = cs_2nRatio_Ce139m[i-1] + 
              (cs_2nRatio_Ce139m[i] - cs_2nRatio_Ce139m[i-1])*(neutronEnergy - en_2n_Ce140[i-1])
              /(en_2n_Ce140[i] - en_2n_Ce140[i-1]);
            break;
          }
        }

        //抽样给出基态/激发态的反应道
        if(trial<ratio) {
          exiEnergy = 754.24*CLHEP::keV;
          // 必须先判断带N gamma or e-的，再判断不带的
          if(oldChannel.find("2 neutron + N gamma or e-") != G4String::npos){
              oldChannel.replace(oldChannel.find("or e-"), 5, "or e- + Ce139[754.24]");
          }
          else if(oldChannel.find("2 neutron") != G4String::npos){
             oldChannel.replace(oldChannel.find("2 neutron"), 9, "2 neutron + Ce139[754.24]");
          }
        }
        else   {
          if(oldChannel.find("2 neutron + N gamma or e-") != G4String::npos){
              oldChannel.replace(oldChannel.find("or e-"), 5, "or e- + Ce139");
          }
          else if(oldChannel.find("2 neutron") != G4String::npos){
             oldChannel.replace(oldChannel.find("2 neutron"), 9, "2 neutron + Ce139");
          }
        }
        oldChannel = "new: " + oldChannel;
        // run->CountNuclearChannel(oldChannel, Q); //新增一个统计Ce139[754.24]

        G4ParticleDefinition* ion = G4IonTable::GetIonTable()->GetIon(58, 139, exiEnergy);
        G4String name = ion->GetParticleName();
        G4double meanLife = ion->GetPDGLifeTime();
        run->ParticleCount(name, 1.0*CLHEP::keV, meanLife);

        G4ThreeVector momentumDirection = aStep->GetPreStepPoint()->GetMomentumDirection();
        G4double kineticEnergy = 0.01*CLHEP::keV;  //这里动能不影响粒子衰变，但是需要大于零确保它能继续运动下去，另外不能太大，否则会出现多余能量沉积
        G4DynamicParticle* const residualNucleus = new G4DynamicParticle(ion, momentumDirection,  kineticEnergy);
        G4double time = aStep->GetTrack()->GetGlobalTime();
        G4ThreeVector currentPosition = aStep->GetPreStepPoint()->GetPosition();
        G4Track* aSecondary = new G4Track(residualNucleus,  time, currentPosition);
        aSecondary->SetParentID(0);//aStep->GetTrack()->GetTrackID()
        aSecondary->SetTrackID(1); //fStackAction->GetTrackNum()+1
        fStackAction->AddNewTrack(aSecondary);
  }

    //Zr90(n,np)Y89m
  if(nuclearChannel.find("proton + neutron") != G4String::npos && (targetName == "Zr90"))
    {
        //抽样决定产生基态和激发态两种粒子
        G4double trial = G4UniformRand();
        G4double exiEnergy = 0.0;

        // 根据能量线性插值给出激发态比例
        G4double neutronEnergy = aStep->GetPreStepPoint()->GetKineticEnergy();
        G4double ratio = 0.0;
        for(int i=0; i<4; i++){
          if(neutronEnergy <= en_np_Y89[0]) {
            ratio = cs_npRatio_Y89m[0];
            break;
          }
          if(neutronEnergy >= en_np_Y89[3]){
            ratio = cs_npRatio_Y89m[3];
            break;
          }
          if(neutronEnergy<en_np_Y89[i]){
            ratio = cs_npRatio_Y89m[i-1] + 
              (cs_npRatio_Y89m[i] - cs_npRatio_Y89m[i-1])*(neutronEnergy - en_np_Y89[i-1])
              /(en_np_Y89[i] - en_np_Y89[i-1]);
            break;
          }
        }
        
        if(trial<ratio) {
          exiEnergy = 908.97*CLHEP::keV;
          if(oldChannel.find("proton + neutron + N gamma or e-") != G4String::npos){
              oldChannel = oldChannel.replace(oldChannel.find("or e-"), 5, "or e- + Y89[908.97]");
          }
          else if(oldChannel.find("proton + neutron") != G4String::npos){
             oldChannel = oldChannel.replace(oldChannel.find("proton + neutron"), 16, "proton + neutron + Y89[908.97]");
          }
        }
        else{
          if(oldChannel.find("proton + neutron + N gamma or e-") != G4String::npos){
              oldChannel = oldChannel.replace(oldChannel.find("or e-"), 5, "or e- + Y89");
          }
          else if(oldChannel.find("proton + neutron") != G4String::npos){
             oldChannel = oldChannel.replace(oldChannel.find("proton + neutron"), 16, "proton + neutron + Y89");
          }
        }

        oldChannel = "new: " + oldChannel;
        // run->CountNuclearChannel(oldChannel, Q); //新增一个统计Y89[908.97]

        G4ParticleDefinition* ion = G4IonTable::GetIonTable()->GetIon(39, 89, exiEnergy);
        G4String name     = ion->GetParticleName();
        G4double meanLife = ion->GetPDGLifeTime();
        run->ParticleCount(name, 1.0*CLHEP::keV, meanLife);

        G4ThreeVector momentumDirection = aStep->GetPreStepPoint()->GetMomentumDirection();
        G4double kineticEnergy = 0.01*CLHEP::keV;
        G4DynamicParticle* const residualNucleus = new G4DynamicParticle(ion, momentumDirection,  kineticEnergy);
        G4double time = aStep->GetTrack()->GetGlobalTime();
        G4ThreeVector currentPosition = aStep->GetPreStepPoint()->GetPosition();
        G4Track* aSecondary = new G4Track(residualNucleus,  time, currentPosition);
        aSecondary->SetParentID(0);//aStep->GetTrack()->GetTrackID()
        aSecondary->SetTrackID(1); //fStackAction->GetTrackNum()+1
        fStackAction->AddNewTrack(aSecondary);
    }
 
  // run->CountNuclearChannel(nuclearChannel, Q);
  fParticleFlag.clear();
}