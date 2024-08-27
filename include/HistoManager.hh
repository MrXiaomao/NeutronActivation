
// $Id: HistoManager.hh 66586 2012-12-21 10:48:39Z ihrivnac $
// 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef HistoManager_h
#define HistoManager_h 1

#include "globals.hh"
#include <map>

#include "g4root.hh"
//#include "g4xml.hh"

const G4int kMaxHisto1 = 24;
const G4int kMaxHisto2 = 254;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class HistoManager
{
  public:
   HistoManager();
  ~HistoManager();

  private:
    void Book();

private:
    G4String fFileName;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
