
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
  fActRadius    = 1*cm;
  fWorldSizeX   = 2.*fActThickness;
  fWorldSizeYZ  = 2.*fActRadius;

  // materials
  DefineMaterials();
  SetAbsorMaterial("G4_Co");

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
  // specific element name for thermal neutronHP
  // (see G4ParticleHPThermalScatteringNames.cc)

  G4int ncomponents, natoms;

  // pressurized water
  G4Element* H  = new G4Element("TS_H_of_Water" ,"H" , 1., 1.0079*g/mole);
  G4Element* O  = new G4Element("Oxygen"        ,"O" , 8., 16.00*g/mole);
  G4Material* H2O = 
  new G4Material("Water_ts", 1.000*g/cm3, ncomponents=2,
                            kStateLiquid, 593*kelvin, 150*bar);
  H2O->AddElement(H, natoms=2);
  H2O->AddElement(O, natoms=1);
  H2O->GetIonisation()->SetMeanExcitationEnergy(78.0*eV);

  // heavy water
  G4Isotope* H2 = new G4Isotope("H2",1,2);
  G4Element* D  = new G4Element("TS_D_of_Heavy_Water", "D", 1);
  D->AddIsotope(H2, 100*perCent);  
  G4Material* D2O = new G4Material("HeavyWater", 1.11*g/cm3, ncomponents=2,
                        kStateLiquid, 293.15*kelvin, 1*atmosphere);
  D2O->AddElement(D, natoms=2);
  D2O->AddElement(O, natoms=1);

  //LaBr3
  G4Element* La  = new G4Element("Lanthanum" ,"La" , 57., 138.9*g/mole);
  G4Element* Br  = new G4Element("Bromine" ,"Br" , 35., 79.9*g/mole);
  G4Element* Ce  = new G4Element("Cerium" ,"Ce" , 58., 140.116*g/mole);
  fLaBr3Material = new G4Material("LaBr3", 5.2*g/cm3, ncomponents=3,
                            kStateLiquid, 293*kelvin, 1*atmosphere);
  fLaBr3Material->AddElement(La, 0.7495);
  fLaBr3Material->AddElement(Br, 0.2005);
  fLaBr3Material->AddElement(Ce, 0.05);
  
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
  fCH2Material = new G4Material("CH2", 0.953*g/cm3, ncomponents=2,
                            kStateSolid, 293*kelvin, 1*atmosphere);
  fCH2Material->AddElement(CC, natoms=1);
  fCH2Material->AddElement(H, natoms=2);

  // graphite
  G4Isotope* C12 = new G4Isotope("C12", 6, 12);  
  G4Element* C   = new G4Element("TS_C_of_Graphite","C", ncomponents=1);
  C->AddIsotope(C12, 100.*perCent);
  G4Material* graphite = 
  new G4Material("graphite", 2.27*g/cm3, ncomponents=1,
                         kStateSolid, 293*kelvin, 1*atmosphere);
  graphite->AddElement(C, natoms=1);

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
  
  G4Box*
  sWorld = new G4Box("World",                                 //name
              fWorldSizeX,fWorldSizeYZ,fWorldSizeYZ);   //dimensions

  G4LogicalVolume*
  lWorld = new G4LogicalVolume(sWorld,                  //shape
                             fWorldMaterial,            //material
                             "World");                  //name

  fWorldVolume = new G4PVPlacement(0,                   //no rotation
                            G4ThreeVector(),            //at (0,0,0)
                            lWorld,                     //logical volume
                            "World",                    //name
                            0,                          //mother volume
                            false,                      //no boolean operation
                            0);                         //copy number
                            
  // Activator
  //
  // G4Box* sAbsor = new G4Box("Absorber",                       //name
  //       fActThickness/2, c/2, fActRadius/2);   //dimensions

  //①Cap
  // G4double fActRadius = fActRadius; //42.1*0.5;
  G4double LengthZrCap = 18.71*mm;
  G4double thicknessZr = fActThickness; //1.0*mm;

  G4Tubs* ZrSideSolid =    
    new G4Tubs("ZrSideSolid", fActRadius-thicknessZr, fActRadius, LengthZrCap*0.5, 0.0*deg, 360*deg); 
  G4Tubs* ZrTopSolid =    
    new G4Tubs("ZrSideSolid", 0., fActRadius, thicknessZr*0.5, 0.0*deg,360*deg); 

  G4ThreeVector translation_Zr = G4ThreeVector(0, 0, LengthZrCap*0.5 + thicknessZr*0.5);
  G4UnionSolid* ZrSolid = new G4UnionSolid("ZrSolid", ZrSideSolid, ZrTopSolid, 0, translation_Zr);
  fLActivator = new G4LogicalVolume(ZrSolid,                        //shape
                                fActiveMaterial,                //material
                                fActiveMaterial->GetName());    //name
  G4double posZ1 = LengthZrCap*0.5;
    new G4PVPlacement(0,                         //no rotation
                    G4ThreeVector(0.,0.,posZ1),             //at (0,0,0)
                    fLActivator,                     //logical volume
                    fActiveMaterial->GetName(),   //name
                    lWorld,                      //mother  volume
                    false,                       //no boolean operation
                    0);                          //copy number

  // Activator2 ,圆锥台
  G4double LengthActivator2 = 8.35*mm;
  G4Cons* Activator2 =  new G4Cons("Activator2",26.4*mm, 27.4*mm, fActRadius-thicknessZr, fActRadius, 8.35*mm*0.5, 0.,360.*deg);
  G4LogicalVolume*
  lActivator2 = new G4LogicalVolume(Activator2,                  //shape
                             fActiveMaterial,            //material
                             "Activator2");                  //name

  G4double posZ2 = posZ1 - LengthZrCap*0.5 - LengthActivator2*0.5;
    new G4PVPlacement(0,                         //no rotation
                    G4ThreeVector(0.,0.,posZ2),             //at (0,0,0)
                    lActivator2,                     //logical volume
                    "Activator2",   //name
                    lWorld,                      //mother  volume
                    false,                       //no boolean operation
                    0);                          //copy number

  // PMT外部Zr套筒
  G4double LengthActivator3 = 33.94*mm;
  G4Tubs* Activator3 =    
    new G4Tubs("Activator3", 26.4*mm, 27.4*mm, LengthActivator3*0.5, 0.0*deg, 360*deg);
  G4LogicalVolume*
  lActivator3 = new G4LogicalVolume(Activator3,                  //shape
                             fActiveMaterial,            //material
                             "Activator3");                  //name
  G4double posZ3 = posZ2 - LengthActivator3*0.5 - LengthActivator2*0.5;
    new G4PVPlacement(0,                         //no rotation
                    G4ThreeVector(0.,0.,posZ3),             //at (0,0,0)
                    lActivator3,                     //logical volume
                    "Activator3",   //name
                    lWorld,                      //mother  volume
                    false,                       //no boolean operation
                    0);                          //copy number

  // LaBr3
  //
  G4double LengthLaBr3 = 1.5*25.4*mm;
  G4Tubs* LaBr3Solid =    
    new G4Tubs("LaBr3Solid", 0.0, LengthLaBr3*0.5, LengthLaBr3*0.5, 0.0*deg, 360*deg); 
  fLLaBr3 = new G4LogicalVolume(LaBr3Solid,                        //shape
                                fLaBr3Material,                //material
                                fLaBr3Material->GetName());    //name

  G4double posZLaBr3 = posZ1 - 1.0*mm - 0.5*LengthLaBr3 + LengthZrCap*0.5;

    new G4PVPlacement(0,                         //no rotation
                    G4ThreeVector(0.,0.,posZLaBr3),   //at (0,0,0)
                    fLLaBr3,                     //logical volume
                    fLaBr3Material->GetName(),   //name
                    lWorld,                      //mother  volume
                    false,                       //no boolean operation
                    0);                          //copy number

  //PMT
  //
  G4double thickPMT = 1.0*mm;
  G4double LengthPMT = 2.0*25.4*mm;
  G4Tubs* PMTOuterSolid =    
    new G4Tubs("PMTOuterSolid", 0.0, LengthPMT*0.5, LengthPMT*0.5, 0.0*deg, 360*deg); 
  G4Tubs* PMTAirSolid =    
    new G4Tubs("PMTAirSolid", 0.0, (LengthPMT-thickPMT)*0.5, (LengthPMT-thickPMT)*0.5, 0.0*deg, 360*deg);

  G4SubtractionSolid* PMTSolid =  new G4SubtractionSolid("PMTSolid", PMTOuterSolid, PMTAirSolid);
  fLPMT = new G4LogicalVolume(PMTSolid,                        //shape
                              fPMTMaterial,                //material
                              "PMT");    //name

  G4double posZPMT = posZLaBr3 - LengthLaBr3*0.5 - LengthPMT*0.5;
    new G4PVPlacement(0,                         //no rotation
                    G4ThreeVector(0.,0.,posZPMT),   //at (0,0,0)
                    fLPMT,                     //logical volume
                    fPMTMaterial->GetName(),   //name
                    lWorld,                      //mother  volume
                    false,                       //no boolean operation
                    0);                          //copy number

  //CH2
  G4double CapThick = 2.0*mm;
  G4double LengthCH2 = 30.9*mm - CapThick;
  G4Tubs* CH2SolidSide =    
    new G4Tubs("CH2SolidSide", LengthPMT*0.5, 54.8*mm*0.5, LengthCH2*0.5, 0.0*deg, 360*deg); 
  G4Tubs* CH2SolidCap =    
    new G4Tubs("CH2SolidCap", 0., 54.8*mm*0.5, 2.0*mm*0.5, 0.0*deg, 360*deg);

  G4ThreeVector translation_CH2 = G4ThreeVector(0, 0, -LengthCH2*0.5 - CapThick*0.5);
  G4UnionSolid* CH2Solid = new G4UnionSolid("CH2Solid", CH2SolidSide, CH2SolidCap, 0, translation_CH2);

  fLCH2 = new G4LogicalVolume(CH2Solid,                        //shape
                              fCH2Material,                //material
                              "CH2");    //name

  G4double posZCH2 = posZ3 - LengthCH2*0.5 - LengthActivator3*0.5;
    new G4PVPlacement(0,                         //no rotation
                    G4ThreeVector(0.,0.,posZCH2),   //at (0,0,0)
                    fLCH2,                     //logical volume
                    fCH2Material->GetName(),   //name
                    lWorld,                      //mother  volume
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
