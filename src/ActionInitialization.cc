/// \file ActionInitialization.cc
/// \brief Implementation of the ActionInitialization class

#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "TrackingAction.hh"
#include "SteppingAction.hh"
#include "SteppingVerbose.hh"
#include "StackAction.hh"
//<unistd.h>�е�access�����ж��ļ��Ƿ���ڣ�<>�еĿ��Դ����ļ���
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::ActionInitialization(DetectorConstruction* detector)
 : G4VUserActionInitialization(),
   fDetector(detector)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::~ActionInitialization()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::BuildForMaster() const
{
  std::ostringstream os;
  os << "../OutPut";
	os << fDetector->GetActRotate();
  os << "/";
  G4String path = os.str();
  // �����ʷ����ļ�
  // const char *path = "../OutPut/";
  // G4String G4path = path;
  //�ж��ļ����Ƿ���ڣ��������򴴽�
  if (access(path, 0) == -1){   //����ļ��в�����
    mkdir(path,0777);
  }
  /*else{
    // ɾ��Ŀ¼�е���������
    std::string command = "rm -r " + std::string(path);  // ����ɾ������
    int result = std::system(command.c_str());  // ����ϵͳ����
    if (result == 0) {
        G4cout << "�ɹ�ɾ���ļ��У�" << path << G4endl;
    } else {
        G4cout << "ɾ���ļ���ʧ�ܣ�" << path << G4endl;
    }
  }
  mkdir(G4path,0777);
  */
  RunAction* runAction = new RunAction(fDetector, 0);
  SetUserAction(runAction);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::Build() const
{
  PrimaryGeneratorAction* primary = new PrimaryGeneratorAction(fDetector);
  SetUserAction(primary);
    
  RunAction* runAction = new RunAction(fDetector, primary );
  SetUserAction(runAction);
  
  EventAction* event = new EventAction();
  SetUserAction(event);  
  
  TrackingAction* trackingAction = new TrackingAction(event);
  SetUserAction(trackingAction);

  StackAction* stackAct = new StackAction();
  SetUserAction(stackAct);

  SteppingAction* steppingAction = new SteppingAction(fDetector, event, trackingAction, stackAct);
  SetUserAction(steppingAction);
}  

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSteppingVerbose* ActionInitialization::InitializeSteppingVerbose() const
{
  return new SteppingVerbose();
}  

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
