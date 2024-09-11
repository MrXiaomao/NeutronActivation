
#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;
class DetectorMessenger;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:

    DetectorConstruction(G4double);
   ~DetectorConstruction() override;

  public:

    G4VPhysicalVolume* Construct() override;

    G4Material* 
    MaterialWithSingleIsotope(G4String, G4String, G4double, G4int, G4int);

    void SetActThickness(G4double);
    void SetActRaius   (G4double);
    void SetActMaterial (G4String);

  public:  

   G4double GetActThickness()    {return fActThickness;};
   G4double GetActRotate()    {return fActRotate;};
   G4double GetActRadius()       {return fActRadius;};
   G4Material* GetActMaterial()  {return fActiveMaterial;};

   G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }

   G4double GetWorldSizeXY()   {return fWorldSizeXY;};
   G4double GetWorldSizeZ()  {return fWorldSizeZ;};

   void PrintParameters();

  private:

   //中子活化靶,Zr
   G4double           fActThickness = 0.;
   G4double           fActRadius = 0.;
   G4double           fActRotate = 0.0; //锆帽旋转角度
   G4Material*        fActiveMaterial = nullptr;
   G4LogicalVolume*   fLActivator = nullptr;

   // LaBr3探测器
   G4Material*        fLaBr3Material = nullptr;
   G4LogicalVolume*   fLLaBr3 = nullptr;

  // AlAlloy Al合金
    G4Material*        fAlAlloyMaterial = nullptr;
   G4LogicalVolume*   fLAlAlloy = nullptr;

  // MgO反射层
   G4Material*        fMgOMaterial = nullptr;
   G4LogicalVolume*   fLMgO = nullptr;

   // PMT光电倍增管
   G4Material*        fPMTMaterial = nullptr;
   G4LogicalVolume*   fLPMT = nullptr;
   
    // 聚乙烯
   G4Material*        fCH2Material = nullptr;
   G4LogicalVolume*   fLCH2 = nullptr;

   G4double           fWorldSizeXY = 0.;
   G4double           fWorldSizeZ = 0.;
   G4Material*        fWorldMaterial = nullptr;
   G4VPhysicalVolume* fWorldVolume = nullptr;                        

   DetectorMessenger* fDetectorMessenger = nullptr;

  private:

   void               DefineMaterials();
   G4VPhysicalVolume* ConstructVolumes();  

  protected:
    G4LogicalVolume* fScoringVolume;   
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
