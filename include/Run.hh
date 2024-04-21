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
/// \file Run.hh
/// \brief Definition of the Run class
//
// $Id: Run.hh 71375 2013-06-14 07:39:33Z maire $
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef Run_h
#define Run_h 1

#include "G4Run.hh"
#include "G4VProcess.hh"
#include "globals.hh"
#include <map>
#include <vector>
#include <stdio.h>
using namespace std;

class DetectorConstruction;
class G4ParticleDefinition;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class Run : public G4Run
{
  public:
    Run(DetectorConstruction*);
   ~Run();

  public:
    void SetPrimary(G4ParticleDefinition* particle, G4double energy);         
    void CountProcesses(const G4VProcess* process);
    void ParticleCount(G4String, G4double, G4double); 
    void AddEdep(G4double edep);
    void AddIronEdep(std::map<G4String, G4double>& ironSpec);
    void AddTrackEdep_Time(G4double energy, G4double time);
    void AddEflow (G4double eflow);                   
    void ParticleFlux(G4String, G4double);

    G4int GetIonId (G4String);

    virtual void Merge(const G4Run*);
    void EndOfRun();     
   
  private:
    struct ParticleData {
     ParticleData()
       : fCount(0), fEmean(0.), fEmin(0.), fEmax(0.), fTmean(-1.) {}
     ParticleData(G4int count, G4double ekin, G4double emin, G4double emax,
                  G4double meanLife)
       : fCount(count), fEmean(ekin), fEmin(emin), fEmax(emax),
         fTmean(meanLife) {}
     G4int     fCount;
     G4double  fEmean;
     G4double  fEmin;
     G4double  fEmax;
     G4double  fTmean;
    };
    struct EdepTime{
      EdepTime():Edep(0.0),depTime(0.0){}
      EdepTime(G4double a, G4double b):Edep(a),depTime(b){}
      G4double Edep;
      G4double depTime;
    };
     
  private:
    // utility function
    void Merge(std::map<G4String,ParticleData>& destinationMap,
               const std::map<G4String,ParticleData>& sourceMap) const;

    void Merge(std::map<G4String, vector<G4double>>& destinationMap,
               const std::map<G4String, vector<G4double>>& sourceMap) const;
    
    void Merge(vector<EdepTime>& destinationEdep,
                const vector<EdepTime>& sourceEdep) const;

    static std::map<G4String,G4int> fgIonMap;
    static G4int fgIonId;

    DetectorConstruction* fDetector;
    G4ParticleDefinition* fParticle;
    G4double              fEkin;

    G4double fEnergyDeposit, fEnergyDeposit2;
    G4double fEnergyFlow,    fEnergyFlow2;            
    std::map<G4String,G4int>        fProcCounter;
    std::map<G4String,ParticleData> fParticleDataMap1;                    
    std::map<G4String,ParticleData> fParticleDataMap2;
    std::map<G4String, vector<G4double>> fIronSpectrum;
    vector<EdepTime> fEdep_time;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

