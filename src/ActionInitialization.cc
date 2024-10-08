// $Id: ActionInitialization.cc 68058 2013-03-13 14:47:43Z gcosmo $
//
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
//<unistd.h>中的access可以判断文件是否存在，<>中的可以创建文件。
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
  // 清除历史输出文件
  // const char *path = "../OutPut/";
  // G4String G4path = path;
  //判断文件夹是否存在，不存在则创建
  if (access(path, 0) == -1){   //如果文件夹不存在
    mkdir(path,0777);
  }
  /*else{
    // 删除目录中的所有内容
    std::string command = "rm -r " + std::string(path);  // 构造删除命令
    int result = std::system(command.c_str());  // 调用系统命令
    if (result == 0) {
        G4cout << "成功删除文件夹：" << path << G4endl;
    } else {
        G4cout << "删除文件夹失败：" << path << G4endl;
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
