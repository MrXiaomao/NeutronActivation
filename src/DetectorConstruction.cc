//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file DetectorConstruction.cc
/// \brief Implementation of the DetectorConstruction class
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4UnionSolid.hh"

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
  fActThickness = 1*cm;
  fActRadius    = 1*cm;
  fWorldSizeX     = 1.2*fActThickness;
  fWorldSizeYZ    = 1.2*fActRadius;

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
  fLaBr3Material = new G4Material("LaBr3", 5.2*g/cm3, ncomponents=2,
                            kStateLiquid, 593*kelvin, 150*bar);
  fLaBr3Material->AddElement(La, natoms=1);
  fLaBr3Material->AddElement(Br, natoms=3);

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
  fWorldSizeX     = 1.2*fActRadius;
  fWorldSizeYZ    = 1.2*fActRadius;
  
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
  G4double radius_LaBr3 = fActRadius; //1.5*25.4*mm*0.5;
  G4double thicknessZr = fActThickness; //2.0*mm;

  G4Tubs* ZrSideSolid =    
    new G4Tubs("ZrSideSolid", radius_LaBr3, radius_LaBr3 + thicknessZr, radius_LaBr3, 0.0*deg, 360*deg); 
  G4Tubs* ZrTopSolid =    
    new G4Tubs("ZrSideSolid", 0., radius_LaBr3 + thicknessZr, thicknessZr*0.5, 0.0*deg,360*deg); 

  G4ThreeVector translation_Zr = G4ThreeVector(0, 0, radius_LaBr3 + thicknessZr*0.5);
  G4UnionSolid* ZrSolid = new G4UnionSolid("ZrSolid", ZrSideSolid, ZrTopSolid, 0, translation_Zr);
  fLActivator = new G4LogicalVolume(ZrSolid,                        //shape
                                fActiveMaterial,                //material
                                fActiveMaterial->GetName());    //name
                                
            new G4PVPlacement(0,                         //no rotation
                            G4ThreeVector(),             //at (0,0,0)
                            fLActivator,                     //logical volume
                            fActiveMaterial->GetName(),   //name
                            lWorld,                      //mother  volume
                            false,                       //no boolean operation
                            0);                          //copy number

  // LaBr3
  //
  G4Tubs* LaBr3Solid =    
    new G4Tubs("LaBr3Solid", 0.0, radius_LaBr3, radius_LaBr3, 0.0*deg, 360*deg); 
  fLLaBr3 = new G4LogicalVolume(LaBr3Solid,                        //shape
                                fLaBr3Material,                //material
                                fLaBr3Material->GetName());    //name
                                
            new G4PVPlacement(0,                         //no rotation
                            G4ThreeVector(),             //at (0,0,0)
                            fLLaBr3,                     //logical volume
                            fLaBr3Material->GetName(),   //name
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
