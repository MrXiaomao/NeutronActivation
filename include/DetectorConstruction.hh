
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
   G4double GetActLength()       {return fActLength;};
   G4Material* GetActMaterial()  {return fActiveMaterial;};

   G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }

   G4double GetWorldSizeXY()   {return fWorldSizeXY;};
   G4double GetWorldSizeZ()  {return fWorldSizeZ;};

   void PrintParameters();

  private:
   //中子活化靶,Zr
   G4double           fActThickness; // 锆帽厚度
   G4double           fActRadius; // 锆帽半径
   G4double          fActLength; // 锆帽长度
   G4double           fActRotate; // 锆帽旋转角度
   G4Material*        fActiveMaterial;
   G4LogicalVolume*   fLActivator;

   // LaBr3探测器
   G4Material*        fLaBr3Material;
   G4LogicalVolume*   fLLaBr3;

  // AlAlloy Al合金
  G4Material*        fAlAlloyMaterial;
  G4LogicalVolume*   fLAlAlloy;

  // MgO反射层
  G4Material*        fMgOMaterial;
  G4LogicalVolume*   fLMgO;

  // PMT光电倍增管
  G4Material*        fPMTMaterial;
  G4LogicalVolume*   fLPMT;
  
  // 聚乙烯
  G4Material*        fCH2Material;
  G4LogicalVolume*   fLCH2;

  //SiO2 石英玻璃
  G4Material*        fSiO2Material;
  G4LogicalVolume*   fLSiO2;

  G4double           fWorldSizeXY;
  G4double           fWorldSizeZ;
  G4Material*        fWorldMaterial;
  G4VPhysicalVolume* fWorldVolume;                        

  DetectorMessenger* fDetectorMessenger;

private:

   void               DefineMaterials();
   G4VPhysicalVolume* ConstructVolumes();  
  
protected:
    G4LogicalVolume* fScoringVolume;        
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
