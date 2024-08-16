
#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4RunManager.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4UnitsTable.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction():
  fScoringVolume(0)
{
  // default geometrical parameters
  fActThickness = 1*mm;
  fActRadius    = 1.905*cm;
  fWorldSizeX   = 2.*fActThickness;
  fWorldSizeYZ  = 2.*fActRadius;

  // materials
  DefineMaterials();
  SetAbsorMaterial("G4_Zr");

  // create commands for interactive definition of the geometry
  fDetectorMessenger = new DetectorMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{ delete fDetectorMessenger;}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  return ConstructVolumes();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefineMaterials()
{
  G4int ncomponents, natoms;

  //LaBr3
  G4Element* La  = new G4Element("Lanthanum" ,"La" , 57., 138.9*g/mole);
  G4Element* Br  = new G4Element("Bromine" ,"Br" , 35., 79.9*g/mole);
  G4Element* Ce  = new G4Element("Cerium" ,"Ce" , 58., 140.116*g/mole);
  fLaBr3Material = new G4Material("LaBr3", 5.2*g/cm3, ncomponents=3,
                            kStateLiquid, 293*kelvin, 1*atmosphere);
  fLaBr3Material->AddElement(La, 0.7495);
  fLaBr3Material->AddElement(Br, 0.2005);
  fLaBr3Material->AddElement(Ce, 0.05);
  
  //MgO反射层
  G4Element* Mg  = new G4Element("Magnesium" ,"Mg" , 12., 24.3050*g/mole);
  G4Element* O  = new G4Element("Oxygen"        ,"O" , 8., 16.00*g/mole);
  fMgOMaterial = new G4Material("MgO", 3.58*g/cm3, ncomponents=2,
                            kStateSolid, 293*kelvin, 1*atmosphere);
  fMgOMaterial->AddElement(Mg, 1);
  fMgOMaterial->AddElement(O, 1);

  //Al合金
  G4Element* Al  = new G4Element("Aluminium" ,"Al" , 13., 26.98*g/mole);  
  fAlAlloyMaterial = new G4Material("AlAlloy", 2.65*g/cm3, ncomponents=1,
                            kStateSolid, 293*kelvin, 1*atmosphere);
  fAlAlloyMaterial->AddElement(Al,1);

  //PMT
  G4Element* Na  = new G4Element("Sodium" ,"Na" , 11., 22.9898*g/mole);
  G4Element* Ca  = new G4Element("Calcium" ,"Ca" , 20., 40.078*g/mole);
  G4Element* Si  = new G4Element("Silicon" ,"Si" , 14., 28.0855*g/mole);

  fPMTMaterial = new G4Material("PMT", 2.5*g/cm3, ncomponents=4,
                            kStateSolid, 293*kelvin, 1*atmosphere);
  fPMTMaterial->AddElement(Na, natoms=1);
  fPMTMaterial->AddElement(O, natoms=14);
  fPMTMaterial->AddElement(Si, natoms=6);
  fPMTMaterial->AddElement(Ca, natoms=1);
  
  //CH2
  G4Element* CC  = new G4Element("Carbon" ,"C" , 6., 12.00*g/mole);
  G4Element* H  = new G4Element("Hydrogen" ,"H" , 1., 1.0079*g/mole);
  fCH2Material = new G4Material("CH2", 0.953*g/cm3, ncomponents=2,
                            kStateSolid, 293*kelvin, 1*atmosphere);
  fCH2Material->AddElement(CC, natoms=1);
  fCH2Material->AddElement(H, natoms=2);

  // example of vacuum
  fWorldMaterial = new G4Material("Galactic", 1, 1.01*g/mole,
            universe_mean_density, kStateGas, 2.73*kelvin, 3.e-18*pascal);

 ///G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Material* DetectorConstruction::MaterialWithSingleIsotope( G4String name,
                           G4String symbol, G4double density, G4int Z, G4int A)
{
 // define a material from an isotope
 //
 G4int ncomponents;
 G4double abundance, massfraction;

 G4Isotope* isotope = new G4Isotope(symbol, Z, A);
 
 G4Element* element  = new G4Element(name, symbol, ncomponents=1);
 element->AddIsotope(isotope, abundance= 100.*perCent);
 
 G4Material* material = new G4Material(name, density, ncomponents=1);
 material->AddElement(element, massfraction=100.*perCent);

 return material;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::ConstructVolumes()
{
  // Cleanup old geometry
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  // World
  //
  fWorldSizeX     = 4.*fActRadius;
  fWorldSizeYZ    = 4.*fActRadius;
  
  G4Box*   sWorld = new G4Box("World",   fWorldSizeX,fWorldSizeYZ,fWorldSizeYZ);  
  G4LogicalVolume*  lWorld = new G4LogicalVolume(sWorld,  fWorldMaterial,  "World");   
  fWorldVolume = new G4PVPlacement(0,                   //no rotation
                            G4ThreeVector(),            //at (0,0,0)
                            lWorld,                     //logical volume
                            "World",                    //name
                            0,                          //mother volume
                            false,                      //no boolean operation
                            0);                         //copy number

  G4double radius_LaBr3 = 1.5 * 25.4 * 0.5 *mm; //LaBr3探测器直径
  G4double thickness_MgO = 0.2*mm; //MgO粉厚度，光反射层。
  G4double thickness_AlAlloy = 1.5*mm; // 铝合金厚度
  G4double thickness_ZrCap = fActThickness; // 锆帽厚度

  //① Zr Cap
  G4double LengthZrCap = radius_LaBr3 * 2.0 + thickness_MgO + thickness_AlAlloy + thickness_ZrCap;
  G4double radius_ZrCap = radius_LaBr3 + thickness_MgO + thickness_AlAlloy + thickness_ZrCap; //1.0*mm;
  G4double posZ1 = LengthZrCap*0.5;

  G4Tubs* ZrCup =  new G4Tubs("ZrCup", 0., radius_ZrCap, LengthZrCap*0.5, 0.0*deg, 360*deg); 
  fLActivator = new G4LogicalVolume(ZrCup, fActiveMaterial, fActiveMaterial->GetName());
  new G4PVPlacement(0,                         //no rotation
                  G4ThreeVector(0.,0.,posZ1),             //at (0,0,0)
                  fLActivator,                     //logical volume
                  fActiveMaterial->GetName(),   //name
                  lWorld,                      //mother  volume
                  false,                       //no boolean operation
                  0);                          //copy number

  // 铝合金外壳
  G4double LengthAlAlloy = radius_LaBr3 * 2.0 + thickness_MgO + thickness_AlAlloy;
  G4double radius_AlAlloy = radius_LaBr3 + thickness_MgO + thickness_AlAlloy; //1.0*mm;
  G4double posZ2 =  -thickness_ZrCap*0.5;
  G4Tubs* AlAlloy_Tub =  new G4Tubs("AlAlloy_Tub", 0., radius_AlAlloy, LengthAlAlloy*0.5, 0.0*deg, 360*deg);
  fLAlAlloy = new G4LogicalVolume(AlAlloy_Tub,  fAlAlloyMaterial,  "AlAlloy");
  new G4PVPlacement(0,                         //no rotation
                    G4ThreeVector(0.,0.,posZ2),             //at (0,0,0)
                    fLAlAlloy,                     //logical volume
                    "AlAlloy_phy",   //name
                    fLActivator,          //mother  volume
                    false,                     //no boolean operation
                    0);                           //copy number

  // MgO层
  G4double LengthMgO = radius_LaBr3 * 2.0 + thickness_MgO;
  G4double radius_MgO = radius_LaBr3 + thickness_MgO;
  G4double posZ3 =  - thickness_AlAlloy*0.5;
  G4Tubs* MgO_Tub = new G4Tubs("MgO_Tub", 0., radius_MgO, LengthMgO*0.5, 0.0*deg, 360*deg);
  fLMgO = new G4LogicalVolume(MgO_Tub, fMgOMaterial, "fLMgO");
  new G4PVPlacement(0,                         //no rotation
                  G4ThreeVector(0.,0.,posZ3),             //at (0,0,0)
                  fLMgO,                     //logical volume
                  "MgO_Phys",                //name
                  fLAlAlloy,                      //mother  volume
                  false,                       //no boolean operation
                  0);                          //copy number

  // LaBr3
  //
  G4double posZLaBr3 = - thickness_MgO*0.5;
  G4Tubs* LaBr3Solid = new G4Tubs("LaBr3Solid", 0.0, radius_LaBr3, radius_LaBr3, 0.0*deg, 360*deg); 
  fLLaBr3 = new G4LogicalVolume(LaBr3Solid, fLaBr3Material, fLaBr3Material->GetName());
  new G4PVPlacement(0,                         //no rotation
                  G4ThreeVector(0.,0.,posZLaBr3),   //at (0,0,0)
                  fLLaBr3,                     //logical volume
                  fLaBr3Material->GetName(),   //name
                  fLMgO,                      //mother  volume
                  false,                       //no boolean operation
                  0);                          //copy number

  PrintParameters();
  fScoringVolume = fLLaBr3;
  //always return the root volume
  //
  return fWorldVolume;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::PrintParameters()
{
  G4cout << "\n The Activator is " << G4BestUnit(fActRadius*2,"Length")
         << " of " << fActiveMaterial->GetName() 
         << "\n \n" << fActiveMaterial << G4endl;

  G4cout << "\n The LaBr3 is " << G4BestUnit(fActRadius*2,"Length")
        << " of " << fLaBr3Material->GetName()
        << "\n \n" << fLaBr3Material << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetAbsorMaterial(G4String materialChoice)
{
  // search the material by its name
  G4Material* pttoMaterial =
     G4NistManager::Instance()->FindOrBuildMaterial(materialChoice);   
  
  if (pttoMaterial) { 
    fActiveMaterial = pttoMaterial;
    if(fLActivator) { fLActivator->SetMaterial(fActiveMaterial); }
    G4RunManager::GetRunManager()->PhysicsHasBeenModified();
  } else {
    G4cout << "\n--> warning from DetectorConstruction::SetMaterial : "
           << materialChoice << " not found" << G4endl;
  }              
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetActThickness(G4double value)
{
  fActThickness = value;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetActRaius(G4double value)
{
  fActRadius = value;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
