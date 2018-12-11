// ------------------------------------------------------------------------------------
//  ROOT macro that produces average RecHit energy from PFG ntuples
//
//  Author : Ken H
//  Written on May 24, 2018
// ------------------------------------------------------------------------------------
//  
// Pre-requisite :
//
//   You should have the PFG ntuple for the Run from which you want to do a measurement. 
//   Instruction on how to make PFG ntuples can be found here : FIXME link here 
//
//   You should have "Fig" directory for plots 
//
// Usage : 
//
//   $ root -b  
//   root> .L ana_PFStudy.C+
//   root> ana_PFStudy("/cms/data/store/user/hatake/HCAL/ntuples/10_2_x/pi50_trees_MCfull_CMSSW_10_2_0_pre3_*.root","hcal_timestudy_pi50_histograms.root")
//   or
//   root> ana_PFStudy("list_trees_pi50_MCfull_CMSSW_10_2_0_pre3.txt","hcal_timestudy_pi50_histograms.root")
//   or
//   from command line:
/*
     root.exe -b -q 'ana_PFStudy.C++("trees_relval_ttbar_phase2_age_new2_4500ultimate.root","hcal_noisestudy_histograms_age_new2_4500ultimate.root")'
     root.exe -b -q 'ana_PFStudy.C++("trees_relval_ttbar_phase2_age_org.root","hcal_noisestudy_histograms_age_org.root")'
     root.exe -b -q 'ana_PFStudy.C++("trees_relval_ttbar_phase2_noage.root","hcal_noisestudy_histograms_noage.root")'
 */
//    
// -----------------------------------------------------------------------------------
// 

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip> // for setw()
#include <algorithm> 

#include "TROOT.h"
#include "TF1.h"
#include "TMath.h"
#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TBranch.h"
#include "TString.h"
#include "TStyle.h"
#include "TInterpreter.h"
#include "TStyle.h"
#include "TLorentzVector.h"

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

// In order to use vector of vectors : vector<vector<data type> >
// ACLiC makes dictionary for this
// [ref] http://root.cern.ch/phpBB3/viewtopic.php?f=3&t=10236&p=44117#p44117
#ifdef __MAKECINT__
#pragma link C++ class std::vector < std::vector<int> >+;
#pragma link C++ class std::vector < std::vector<float> >+;
#endif

using namespace std;

bool DRAWPLOTS  = false;  // draw plots or not (make "Fig" directory first before turning this on)
bool VERBOSE    = false;  // print out mean +/- sigma for each channel or not

// Assemble a list of inputfiles

std::vector<std::string> GetInputFiles(std::string geoConfig)
{
  int numFiles = 20;
  std::string path = "/cms/data/store/user/bcaraway/condor/outputs/";
  std::string startName = "ttbar_10_4_";
  std::string midName = "_pt25_numEvent1000_CMSSW_10_4_0_pre2_";
  std::string endName = "_v01.root";
  std::vector<std::string> inputFiles;
  
  for( int iFile = 0 ; iFile<numFiles ; iFile++ )
    {
      std::ostringstream fileName;
      fileName << path << startName << geoConfig << midName << iFile << endName;
      inputFiles.push_back(fileName.str());
     
    }

  return inputFiles;

}

//
// Book histograms
//

void book1D(TList *v_hist, std::string name, int n, double min, double max);
void book1DProf(TList *v_hist, std::string name, int n, double min, double max, double ymin, double ymax, Option_t *option);

void book2D(TList *v_hist, std::string name, int xn, double xmin, double xmax, int yn, double ymin, double ymax); // added by Bryan

void bookHistograms(TList *v_hist);

//
// Fill histograms
//
void fill1D(TList *v_hist, std::string name, double value);
void fill1D(TList *v_hist, std::string name, double value, double value2);
void fill1DProf(TList *v_hist, std::string name, double value, double valuey);

void fill2D(TList *v_hist, std::string name, double valuex, double valuey);  // added by Bryan 

//
// Aux
//
void relabelProfA(TList *v_hist, std::string name);
void relabel2D   (TList *v_hist, std::string name); // added by Bryan
//
// Main analyzer
//
void PFCheckRun(std::vector<std::string> inputFiles, TString outfile, int maxevents=-1, int option=2) 
{ 

   cout << "[PF analyzer] Running option " << option << " for " << endl; 

   // fit pannel display option
   gStyle->SetOptFit(1011);

   //
   // Get the tree from the PFG ntuple 
   //
   TChain *ch = new TChain("hgcalTupleTree/tree");

   //std::string filename(rootfile);
   //std::string::size_type idx;
   //idx = filename.rfind('.');
   //std::string extension = filename.substr(idx+1);
   //std::string line;
   
   //if(idx != std::string::npos && extension=="txt")
   //  {
   //    std::cout << rootfile << " " << extension << std::endl;
   //    std::ifstream in(rootfile);
   //    while (std::getline(in, line)) {     // Process line
   //	 if (line.size()>0) ch->Add(line.c_str());
   //    }
   //  }
   //else
   //  {
   //    // No extension found
   //    ch->Add(rootfile);
   //  }


   for (unsigned int iFile=0; iFile<inputFiles.size(); ++iFile) {
    ch->Add(inputFiles[iFile].c_str());
    std::cout<<inputFiles[iFile]<<std::endl;
   }

   printf("%d;\n",ch->GetNtrees());
   printf("%lld;\n",ch->GetEntries());

   TTreeReader     fReader(ch);  //!the tree reader

   //
   // Set up TTreeReader's
   // -- use MakeSelector of root
   //
   // Readers to access the data (delete the ones you do not need).

   TTreeReaderArray<double> GenParEta = {fReader, "GenParEta"};
   TTreeReaderArray<double> GenParM = {fReader, "GenParM"};
   TTreeReaderArray<double> GenParPhi = {fReader, "GenParPhi"};
   TTreeReaderArray<double> GenParPt = {fReader, "GenParPt"};
   TTreeReaderArray<double> GeneralTracksD0 = {fReader, "GeneralTracksD0"};
   TTreeReaderArray<double> GeneralTracksDZ = {fReader, "GeneralTracksDZ"};
   TTreeReaderArray<double> GeneralTracksEta = {fReader, "GeneralTracksEta"};
   TTreeReaderArray<double> GeneralTracksPhi = {fReader, "GeneralTracksPhi"};
   TTreeReaderArray<double> GeneralTracksPt = {fReader, "GeneralTracksPt"};
   TTreeReaderArray<double> PFParEta = {fReader, "PFParEta"};
   TTreeReaderArray<double> PFParM = {fReader, "PFParM"};
   TTreeReaderArray<double> PFParPhi = {fReader, "PFParPhi"};
   TTreeReaderArray<double> PFParPt = {fReader, "PFParPt"};
   /*
   TTreeReaderArray<float> HBHERecHitEnergy = {fReader, "HBHERecHitEnergy"};
   TTreeReaderArray<float> HBHERecHitEta = {fReader, "HBHERecHitEta"};
   TTreeReaderArray<float> HBHERecHitPhi = {fReader, "HBHERecHitPhi"};
   TTreeReaderArray<float> HBHERecHitTime = {fReader, "HBHERecHitTime"};
   TTreeReaderArray<float> HGCRecHitEnergy = {fReader, "HGCRecHitEnergy"};
   TTreeReaderArray<float> HGCRecHitEta = {fReader, "HGCRecHitEta"};
   TTreeReaderArray<float> HGCRecHitPhi = {fReader, "HGCRecHitPhi"};
   TTreeReaderArray<float> HGCRecHitPosx = {fReader, "HGCRecHitPosx"};
   TTreeReaderArray<float> HGCRecHitPosy = {fReader, "HGCRecHitPosy"};
   TTreeReaderArray<float> HGCRecHitPosz = {fReader, "HGCRecHitPosz"};
   TTreeReaderArray<float> HGCSimHitsEnergy = {fReader, "HGCSimHitsEnergy"};
   TTreeReaderArray<float> HGCSimHitsEta = {fReader, "HGCSimHitsEta"};
   TTreeReaderArray<float> HGCSimHitsPhi = {fReader, "HGCSimHitsPhi"};
   TTreeReaderArray<float> HGCSimHitsPosx = {fReader, "HGCSimHitsPosx"};
   TTreeReaderArray<float> HGCSimHitsPosy = {fReader, "HGCSimHitsPosy"};
   TTreeReaderArray<float> HGCSimHitsPosz = {fReader, "HGCSimHitsPosz"};
   TTreeReaderArray<float> HGCSimHitsTime = {fReader, "HGCSimHitsTime"};
   TTreeReaderArray<float> SimTracksEta = {fReader, "SimTracksEta"};
   TTreeReaderArray<float> SimTracksPhi = {fReader, "SimTracksPhi"};
   TTreeReaderArray<float> SimTracksPt = {fReader, "SimTracksPt"};
   TTreeReaderArray<float> SimTracksR = {fReader, "SimTracksR"};
   TTreeReaderArray<float> SimTracksZ = {fReader, "SimTracksZ"};
   */
   TTreeReaderArray<float> PFParEcalEnergyFrac = {fReader, "PFParEcalEnergyFrac"};
   TTreeReaderArray<float> PFParHOEnergyFrac = {fReader, "PFParHOEnergyFrac"};
   TTreeReaderArray<float> PFParHcalEnergyFrac = {fReader, "PFParHcalEnergyFrac"};
   TTreeReaderArray<float> PFParHcalFrac1 = {fReader, "PFParHcalFrac1"};
   TTreeReaderArray<float> PFParHcalFrac2 = {fReader, "PFParHcalFrac2"};
   TTreeReaderArray<float> PFParHcalFrac3 = {fReader, "PFParHcalFrac3"};
   TTreeReaderArray<float> PFParHcalFrac4 = {fReader, "PFParHcalFrac4"};
   TTreeReaderArray<float> PFParHcalFrac5 = {fReader, "PFParHcalFrac5"};
   TTreeReaderArray<float> PFParHcalFrac6 = {fReader, "PFParHcalFrac6"};
   TTreeReaderArray<float> PFParHcalFrac7 = {fReader, "PFParHcalFrac7"};
   TTreeReaderArray<float> PFParTrackPt = {fReader, "PFParTrackPt"};
   TTreeReaderArray<int> GenParPdgId = {fReader, "GenParPdgId"};
   TTreeReaderArray<int> GenParStatus = {fReader, "GenParStatus"};
   TTreeReaderArray<int> GeneralTracksNValidHits = {fReader, "GeneralTracksNValidHits"};
   /*
   TTreeReaderArray<int> HBHERecHitAux = {fReader, "HBHERecHitAux"};
   TTreeReaderArray<int> HBHERecHitDepth = {fReader, "HBHERecHitDepth"};
   TTreeReaderArray<int> HBHERecHitFlags = {fReader, "HBHERecHitFlags"};
   TTreeReaderArray<int> HBHERecHitHPDid = {fReader, "HBHERecHitHPDid"};
   TTreeReaderArray<int> HBHERecHitIEta = {fReader, "HBHERecHitIEta"};
   TTreeReaderArray<int> HBHERecHitIPhi = {fReader, "HBHERecHitIPhi"};
   TTreeReaderArray<int> HBHERecHitRBXid = {fReader, "HBHERecHitRBXid"};
   TTreeReaderArray<int> HGCRecHitIndex = {fReader, "HGCRecHitIndex"};
   TTreeReaderArray<int> HGCRecHitLayer = {fReader, "HGCRecHitLayer"};
   TTreeReaderArray<int> HGCSimHitsCellU = {fReader, "HGCSimHitsCellU"};
   TTreeReaderArray<int> HGCSimHitsCellV = {fReader, "HGCSimHitsCellV"};
   TTreeReaderArray<int> HGCSimHitsIEta = {fReader, "HGCSimHitsIEta"};
   TTreeReaderArray<int> HGCSimHitsIPhi = {fReader, "HGCSimHitsIPhi"};
   TTreeReaderArray<int> HGCSimHitsIndex = {fReader, "HGCSimHitsIndex"};
   TTreeReaderArray<int> HGCSimHitsLayer = {fReader, "HGCSimHitsLayer"};
   TTreeReaderArray<int> HGCSimHitsSubdet = {fReader, "HGCSimHitsSubdet"};
   TTreeReaderArray<int> HGCSimHitsWaferU = {fReader, "HGCSimHitsWaferU"};
   TTreeReaderArray<int> HGCSimHitsWaferV = {fReader, "HGCSimHitsWaferV"};
   TTreeReaderArray<int> SimTracksCharge = {fReader, "SimTracksCharge"};
   TTreeReaderArray<int> SimTracksPID = {fReader, "SimTracksPID"};
   */
   TTreeReaderArray<int> PFParPdgId = {fReader, "PFParPdgId"};
   TTreeReaderArray<int> PFParStatus = {fReader, "PFParStatus"};
   TTreeReaderArray<double> PFParMET = {fReader, "PFParMET"};
   TTreeReaderArray<double> PFMET = {fReader, "PFMET"};
   TTreeReaderValue<UInt_t> bx = {fReader, "bx"};
   TTreeReaderValue<UInt_t> event = {fReader, "event"};
   TTreeReaderValue<UInt_t> ls = {fReader, "ls"};
   TTreeReaderValue<UInt_t> orbit = {fReader, "orbit"};
   TTreeReaderValue<UInt_t> run = {fReader, "run"};

   //
   // Define histograms to fill
   //
   TList *v_hist = new TList();
   
   bookHistograms(v_hist); // most of histograms booked here
   
   //
   // Loop over entries
   //
   unsigned int nentries = (Int_t)ch->GetEntries();
   cout << "[HGCal analyzer] The number of entries is: " << nentries << endl;

   bool debug=false;
   bool debug_met =false;
   bool print_prev = false;

   //---------------------------------------------------------------------------------------------------------
   // main event loop
   //---------------------------------------------------------------------------------------------------------
   
   TLorentzVector tlzv;      // for MET
   TLorentzVector tlzv_temp; // for MET
   
   int ievent=0;
   while (fReader.Next()) {
  
     // Progress indicator 
     ievent++;
     if(ievent%100==0) cout << "[HCAL analyzer] Processed " << ievent << " out of " << nentries << " events" << endl; 
     if (maxevents>0 && ievent>maxevents) break;
     
     //--------------------
     // Loop over PF candidates
     //--------------------
     
     tlzv.SetPtEtaPhiM(0.0,0.0,0.0,0.0);       // for MET
     tlzv_temp.SetPtEtaPhiM(0.0,0.0,0.0,0.0);  // for METw
     
     double PFMass = 0; // for hardcoded mass in GeV, PFM suffers from a rounding error.     
     

     // Begin Loop
     for (int ipfcand = 0, npfcand =  PFParPt.GetSize(); ipfcand < npfcand; ++ipfcand) {
       
       //if (PFParPdgId[ipfcand] == 7) std::cout << "PdgID: " << PFParPdgId[ipfcand] <<", with Mass: "<<PFParM[ipfcand]<<std::endl; // to debug PFParM
       
       // set the mass of PF cand

       
       if (PFParPdgId[ipfcand] == 1) PFMass = .13957 ; // charged hadron --> Pion
       if (PFParPdgId[ipfcand] == 2) PFMass = .000511 ; // electron
       if (PFParPdgId[ipfcand] == 3) PFMass = .1057 ; // muon
       if (PFParPdgId[ipfcand] == 4) PFMass = 0. ; // photon
       if (PFParPdgId[ipfcand] == 5) PFMass = .49761 ; // neutral hadron --> K0L
       if (PFParPdgId[ipfcand] == 6) PFMass = .13957 ; // HF Hadron --> Pion
       if (PFParPdgId[ipfcand] == 7) PFMass = 0. ; // HF EM Particle --> Photon
	 
       std::string strtmp;

       strtmp = "PFTask_hcalFrac1Zero_vs_pt";
       float zero1=0.;
       if (PFParHcalFrac1[ipfcand]==0.) zero1=1.; 
       fill1DProf(v_hist, strtmp, log10(PFParPt[ipfcand]),zero1);

       strtmp = "PFTask_hcalFrac2Zero_vs_pt";
       float zero2=0.;
       if (PFParHcalFrac2[ipfcand]==0.) zero2=1.; 
       fill1DProf(v_hist, strtmp, log10(PFParPt[ipfcand]),zero2);

       strtmp = "PFTask_hcalFrac3Zero_vs_pt";
       float zero3=0.;
       if (PFParHcalFrac3[ipfcand]==0.) zero3=1.; 
       fill1DProf(v_hist, strtmp, log10(PFParPt[ipfcand]),zero3);

       strtmp = "PFTask_hcalFrac4Zero_vs_pt";
       float zero4=0.;
       if (PFParHcalFrac4[ipfcand]==0.) zero4=1.; 
       fill1DProf(v_hist, strtmp, log10(PFParPt[ipfcand]),zero4);

       strtmp = "PFTask_hcalFrac5Zero_vs_pt";
       float zero5=0.;
       if (PFParHcalFrac5[ipfcand]==0.) zero5=1.; 
       fill1DProf(v_hist, strtmp, log10(PFParPt[ipfcand]),zero5);

       strtmp = "PFTask_hcalFrac6Zero_vs_pt";
       float zero6=0.;
       if (PFParHcalFrac6[ipfcand]==0.) zero6=1.;
       fill1DProf(v_hist, strtmp, log10(PFParPt[ipfcand]),zero6);

       strtmp = "PFTask_hcalFrac7Zero_vs_pt";
       float zero7=0.;
       if (PFParHcalFrac7[ipfcand]==0.) zero7=1.; 
       fill1DProf(v_hist, strtmp, log10(PFParPt[ipfcand]),zero7);

       strtmp = "PFTask_hcalFracAllZero_vs_pt";
       float zeroAll=0.;
       if (PFParHcalFrac1[ipfcand]==0.
	   && PFParHcalFrac2[ipfcand]==0. 
	   && PFParHcalFrac3[ipfcand]==0. 
	   && PFParHcalFrac4[ipfcand]==0. 
	   && PFParHcalFrac5[ipfcand]==0. 
	   && PFParHcalFrac6[ipfcand]==0. 
	   && PFParHcalFrac7[ipfcand]==0.) zeroAll=1.; 
       fill1DProf(v_hist, strtmp, log10(PFParPt[ipfcand]), zeroAll);
       ////
       //// Bryan's studies
       ////
       strtmp = "PFTask_PdgId_vs_pt";  
       fill2D(v_hist, strtmp, PFParPdgId[ipfcand] , PFParPt[ipfcand]);
       
       // Plotting PF eta
       //std::cout << "All" << std::endl;
       strtmp = "PFTask_PFEta_All";
       fill1D(v_hist, strtmp, PFParEta[ipfcand]);
       strtmp = "PFTask_PFEta_pt_All";
       fill1D(v_hist, strtmp, PFParEta[ipfcand],PFParPt[ipfcand]);
       
       if (PFParPdgId[ipfcand] == 1){  // for Charged Hadron
	 //std::cout << "ChargedHadron" << std::endl;
	 strtmp = "PFTask_PFEta_ChargedHadron";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand]);
	 strtmp = "PFTask_PFEta_pt_ChargedHadron";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand],PFParPt[ipfcand]);
       }

       if (PFParPdgId[ipfcand] == 5){  // for Neutral Hadron
	 //std::cout << "NeutralHadron" << std::endl;
	 strtmp = "PFTask_PFEta_NeutralHadron";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand]);
	 strtmp = "PFTask_PFEta_pt_NeutralHadron";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand],PFParPt[ipfcand]);
       }

       if (PFParPdgId[ipfcand] == 2){  // for Electron
	 //std::cout << "Electron" << std::endl;
	 strtmp = "PFTask_PFEta_Electron";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand]);
	 strtmp = "PFTask_PFEta_pt_Electron";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand],PFParPt[ipfcand]);
       }

       if (PFParPdgId[ipfcand] == 3){  // for Muon
	 //std::cout << "Muon" << std::endl;
	 strtmp = "PFTask_PFEta_Muon";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand]);
	 strtmp = "PFTask_PFEta_pt_Muon";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand],PFParPt[ipfcand]);
       }

       if (PFParPdgId[ipfcand] == 4){  // for Photon
	 //std::cout << "Photon" << std::endl;
	 strtmp = "PFTask_PFEta_Photon";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand]);
	 strtmp = "PFTask_PFEta_pt_Photon";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand],PFParPt[ipfcand]);
       }

       if (PFParPdgId[ipfcand] == 6) { // testing other pfcand in vector
	 //std::cout << "HFPhoton" << std::endl;
	 strtmp = "PFTask_PFEta_HFPhoton";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand]);
	 strtmp = "PFTask_PFEta_pt_HFPhoton";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand],PFParPt[ipfcand]);
       }
       
       if (PFParPdgId[ipfcand] == 7) { // testing other pfcand in vector
	 //std::cout << "HFHadron" << std::endl;
	 strtmp = "PFTask_PFEta_HFHadron";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand]);
	 strtmp = "PFTask_PFEta_pt_HFHadron";
	 fill1D(v_hist, strtmp, PFParEta[ipfcand],PFParPt[ipfcand]);
       }
       
       ///
       /// Adding up for MET
       ///
      
       
       if (debug_met){
	 std::cout << "Pt: " << PFParPt[ipfcand] << ", Eta: " << PFParEta[ipfcand] << ", Phi: " << PFParPhi[ipfcand] << ", Mass: " << PFMass << std::endl;
	 
       }
       tlzv_temp.SetPtEtaPhiM(PFParPt[ipfcand],PFParEta[ipfcand],PFParPhi[ipfcand],PFMass);
       tlzv += tlzv_temp;
       
       //
       // Endcap
       // 
       if ( fabs(PFParEta[ipfcand])<2.9&&fabs(PFParEta[ipfcand])>1.5){
	 //
	 // http://cmslxr.fnal.gov/source/DataFormats/ParticleFlowCandidate/interface/PFCandidate.h
	 //
	 // Charged hadrons
	 //
	 if ( PFParPdgId[ipfcand]==1 && fabs(PFParEta[ipfcand])<2.5 ){ 
	   if ( PFParPt[ipfcand]>5.){

	     if (debug) {
	     std::cout << PFParPt[ipfcand] << " " << PFParEta[ipfcand] << " " << PFParPdgId[ipfcand] << std::endl;
	     std::cout << PFParTrackPt[ipfcand] << " " << PFParEcalEnergyFrac[ipfcand] << " "
		       << PFParHcalEnergyFrac[ipfcand] << " " << PFParHOEnergyFrac[ipfcand] << std::endl;
	     std::cout << PFParHcalFrac1[ipfcand] << " "
		       << PFParHcalFrac2[ipfcand] << " " 
		       << PFParHcalFrac3[ipfcand] << " " 
		       << PFParHcalFrac4[ipfcand] << " " 
		       << PFParHcalFrac5[ipfcand] << " " 
		       << PFParHcalFrac6[ipfcand] << " " 
		       << PFParHcalFrac7[ipfcand] << std::endl;
	     std::cout << std::endl;
	     }
	   }
	   //--- 
	   if ( PFParPt[ipfcand]>5. ){
	     strtmp = "PFTask_Profile_ChargedHadron_Endcap_PtAbove5";
	     fill1DProf(v_hist, strtmp, -3, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	     fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);

	     if (debug) {	     
	     std::cout << PFParPt[ipfcand] << " " << PFParEta[ipfcand] << " " << PFParPdgId[ipfcand] << std::endl;	   
	     std::cout << PFParTrackPt[ipfcand] << " " << PFParEcalEnergyFrac[ipfcand] << " "
		       << PFParHcalEnergyFrac[ipfcand] << " " << PFParHOEnergyFrac[ipfcand] << std::endl;
	     std::cout << PFParHcalFrac1[ipfcand] << " "
		       << PFParHcalFrac2[ipfcand] << " " 
		       << PFParHcalFrac3[ipfcand] << " " 
		       << PFParHcalFrac4[ipfcand] << " " 
		       << PFParHcalFrac5[ipfcand] << " " 
		       << PFParHcalFrac6[ipfcand] << " " 
		     << PFParHcalFrac7[ipfcand] << std::endl;
	     std::cout << std::endl;
	     }
	     
	     strtmp = "PFTask_ChargedHadron_TrackPtRatio_PtAbove5";
	     fill1D(v_hist, strtmp, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);

	     if (PFParTrackPt[ipfcand]!=PFParPt[ipfcand]){
	       strtmp = "PFTask_Profile_ChargedHadron_Endcap_PtAbove5_Special";
	       fill1DProf(v_hist, strtmp, -3, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	       fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	       fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	       fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	       fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	       fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	       fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	       fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	       fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	       fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);
	     }

	   } else if ( PFParPt[ipfcand]>1.){
	     
	     strtmp = "PFTask_Profile_ChargedHadron_Endcap_Pt1To5";
	     fill1DProf(v_hist, strtmp, -3, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	     fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);

	     strtmp = "PFTask_ChargedHadron_TrackPtRatio_Pt1To5";
	     fill1D(v_hist, strtmp, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	     
	     if (PFParTrackPt[ipfcand]!=PFParPt[ipfcand]){
	       strtmp = "PFTask_Profile_ChargedHadron_Endcap_Pt1To5_Special";
	       fill1DProf(v_hist, strtmp, -3, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	       fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	       fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	       fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	       fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	       fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	       fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	       fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	       fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	       fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);
	     }

	   } else {
	     
	     strtmp = "PFTask_Profile_ChargedHadron_Endcap_PtBelow1";
	     fill1DProf(v_hist, strtmp, -3, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	     fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);
	     
	     strtmp = "PFTask_ChargedHadron_TrackPtRatio_PtBelow1";
	     fill1D(v_hist, strtmp, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);

	     if (PFParTrackPt[ipfcand]!=PFParPt[ipfcand]){
	       strtmp = "PFTask_Profile_ChargedHadron_Endcap_PtBelow1_Special";
	       fill1DProf(v_hist, strtmp, -3, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	       fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	       fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	       fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	       fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	       fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	       fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	       fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	       fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	       fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);
	     }

	   }
	   
	 //
	 // Neutral hadrons
	 //
	   
	 } else if ( PFParPdgId[ipfcand]==5 ){
	   if ( PFParPt[ipfcand]>5.){
	     strtmp = "PFTask_hcalProfile_NeutralHadron_Endcap_PtAbove5";
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);

	     /*
	     std::cout << PFParPt[ipfcand] << " " << PFParEta[ipfcand] << " " << PFParPdgId[ipfcand] << std::endl;	   
	     std::cout << PFParTrackPt[ipfcand] << " " << PFParEcalEnergyFrac[ipfcand] << " "
		       << PFParHcalEnergyFrac[ipfcand] << " " << PFParHOEnergyFrac[ipfcand] << std::endl;
	     std::cout << PFParHcalFrac1[ipfcand] << " "
		       << PFParHcalFrac2[ipfcand] << " " 
		       << PFParHcalFrac3[ipfcand] << " " 
		       << PFParHcalFrac4[ipfcand] << " " 
		       << PFParHcalFrac5[ipfcand] << " " 
		       << PFParHcalFrac6[ipfcand] << " " 
		     << PFParHcalFrac7[ipfcand] << std::endl;
	     std::cout << std::endl;
	     */

	   } else if ( PFParPt[ipfcand]>1.){
	     
	     strtmp = "PFTask_hcalProfile_NeutralHadron_Endcap_Pt1To5";
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);

	   } else {
	     
	     strtmp = "PFTask_hcalProfile_NeutralHadron_Endcap_PtBelow1";
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);
	     
	   }
	   
	 //
	 // PF electrons
	 //

	 } else if ( PFParPdgId[ipfcand]==2 ){
	   if ( PFParPt[ipfcand]>5.){
	     strtmp = "PFTask_Profile_PFElectron_Endcap_PtAbove5";
	     fill1DProf(v_hist, strtmp, -3, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	     fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);
	     	     
	   } else if ( PFParPt[ipfcand]>1.){
	     
	     strtmp = "PFTask_Profile_PFElectron_Endcap_Pt1To5";
	     fill1DProf(v_hist, strtmp, -3.,PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	     fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);

	   } else {
	     strtmp = "PFTask_Profile_PFElectron_Endcap_PtBelow1";
	     fill1DProf(v_hist, strtmp, -3, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	     fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);
	     
	   }
	 //
	 // PF photons
	 //
	  
	 } else if ( PFParPdgId[ipfcand]==4 ){
	   if ( PFParPt[ipfcand]>5.){
	     strtmp = "PFTask_Profile_PFPhoton_Endcap_PtAbove5";
	     fill1DProf(v_hist, strtmp, -3, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	     fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);
	     
	   } else if ( PFParPt[ipfcand]>1.){
	     
	     strtmp = "PFTask_Profile_PFPhoton_Endcap_Pt1To5";
	     fill1DProf(v_hist, strtmp, -3, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	     fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);

	   } else {
	     
	     strtmp = "PFTask_Profile_PFPhoton_Endcap_PtBelow1";
	     fill1DProf(v_hist, strtmp, -3, PFParTrackPt[ipfcand]/PFParPt[ipfcand]);
	     fill1DProf(v_hist, strtmp, -2, PFParEcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, -1, PFParHcalEnergyFrac[ipfcand]);
	     fill1DProf(v_hist, strtmp, 1., PFParHcalFrac1[ipfcand]);
	     fill1DProf(v_hist, strtmp, 2., PFParHcalFrac2[ipfcand]);
	     fill1DProf(v_hist, strtmp, 3., PFParHcalFrac3[ipfcand]);
	     fill1DProf(v_hist, strtmp, 4., PFParHcalFrac4[ipfcand]);
	     fill1DProf(v_hist, strtmp, 5., PFParHcalFrac5[ipfcand]);
	     fill1DProf(v_hist, strtmp, 6., PFParHcalFrac6[ipfcand]);
	     fill1DProf(v_hist, strtmp, 7., PFParHcalFrac7[ipfcand]);
	     
	   }
	   
	 } // PF photons
       
       } // Endcap
            
     } // PF candidiate loop
     bool debug_met_V2 = false; 

     if  (tlzv.Pt() > 200 && debug_met_V2){
       std::cout << "MET: "<<tlzv.Pt()<<std::endl;
     }
     fill1D(v_hist, "PFTask_MET", tlzv.Pt());  // for MET
     
     fill1D(v_hist, "PFParMET", PFParMET[0]);     // MET calculated before ntuple step
     fill1D(v_hist, "PFMET"   , PFMET[0]   );     // True MET

     if (tlzv.Pt() != PFMET[0]){ 
       std::cout << "\nCalculated MET: " << tlzv.Pt() <<"\nCalculated before ntuple MET: " <<PFParMET[0] << "\nTrue MET: "<<PFMET[0]<<std::endl;
     }
     ///
     /// DEBUG MET
     ///
     
     if (tlzv.Pt() > 200 && debug_met_V2){
       
       tlzv.SetPtEtaPhiM(0.0,0.0,0.0,0.0);       // for MET
       tlzv_temp.SetPtEtaPhiM(0.0,0.0,0.0,0.0);  // for MET

       for (int ipfcand = 0, npfcand =  PFParPt.GetSize(); ipfcand < npfcand; ++ipfcand) {
	 
	 if (PFParPdgId[ipfcand] == 1) PFMass = .13957 ; // charged hadron --> Pion
	 if (PFParPdgId[ipfcand] == 2) PFMass = .000511 ; // electron
	 if (PFParPdgId[ipfcand] == 3) PFMass = .1057 ; // muon
	 if (PFParPdgId[ipfcand] == 4) PFMass = 0. ; // photon
	 if (PFParPdgId[ipfcand] == 5) PFMass = .49761 ; // neutral hadron --> K0L
	 if (PFParPdgId[ipfcand] == 6) PFMass = .13957 ; // HF Hadron --> Pion
	 if (PFParPdgId[ipfcand] == 7) PFMass = 0. ; // HF EM Particle --> Photon

	 tlzv_temp.SetPtEtaPhiM(PFParPt[ipfcand],PFParEta[ipfcand],PFParPhi[ipfcand],PFMass);
	 tlzv += tlzv_temp;
	 
	 if (PFParPt[ipfcand] > 5){
	   std::cout << "PFID: "<< PFParPdgId[ipfcand] << ", Pt: " << PFParPt[ipfcand] << ", Eta: " << PFParEta[ipfcand] << ", Phi: " << PFParPhi[ipfcand] << ", Mass: " << PFMass << std::endl;
	   std::cout<< "Summed PT: " << tlzv.Pt() << std::endl;
	 }
       }
     }

     ///
     /// END DEBUG MET
     ///

   }   // Event loop ends
   //---------------------------------------------------------------------------------------------------------
   // main event loop ends
   //---------------------------------------------------------------------------------------------------------

   // output file for histograms
   TFile file_out(outfile,"RECREATE");
   
   v_hist->Write();
   
   file_out.ls();
   file_out.Close();

}

//
// Main function
//
//void ana_PFStudy_MET_eta(TString rootfile="../../HGCalTreeMaker/test/ttbar_10_4_D30_pt25.root",TString outfile="pfstudy_histograms.root",int maxevents=-1)
void ana_PFStudy_MET_eta(std::string rootfile="./ttbar_10_4_D30_pt25.root",TString outfile="PFD30_histos.root",int maxevents=-1)
{
  // edit 
  bool test_file = false; // if testing setup with single file (will have to edit below for file choice)
  std::string geoType = "D30" ; // "D30" for D30 geo, "D28" for D28

  std::vector<std::string> inputFiles;
  if (!test_file)
    {
      inputFiles = GetInputFiles(geoType);
     
    }
  else inputFiles.push_back(rootfile);

  PFCheckRun(inputFiles, outfile, maxevents, 0);
}

//
// --- Aux ---
//

//
// Book 1D histograms
//
void book1D(TList *v_hist, std::string name, int n, double min, double max)
{
  TH1D *htemp = new TH1D(name.c_str(), name.c_str(), n, min, max);
  v_hist->Add(htemp);
}
//
// Book 1D profile histograms
//
void book1DProf(TList *v_hist, std::string name, int n, double min, double max, double ymin, double ymax, Option_t *option="")
{
  TProfile *htemp = new TProfile(name.c_str(), name.c_str(), n, min, max, ymin, ymax, option);
  v_hist->Add(htemp);
}
//
// Book 2D profile histograms 
//
void book2D(TList *v_hist, std::string name, int xn, double xmin, double xmax, int yn, double ymin, double ymax) 
{
  TH2D *htemp = new TH2D(name.c_str(), name.c_str(), xn, xmin, xmax, yn, ymin, ymax);
  v_hist->Add(htemp);
}
//
// Book histograms
//
void bookHistograms(TList *v_hist)
{

  Char_t histo[100];
  std::string strtmp;
  
  //
  // Booking histograms
  // 
  sprintf(histo, "PFTask_hcalFrac1Zero_vs_pt");
  book1DProf(v_hist, histo, 50., -1, 4., -1., 2.);
  sprintf(histo, "PFTask_hcalFrac2Zero_vs_pt");
  book1DProf(v_hist, histo, 50., -1, 4., -1., 2.);
  sprintf(histo, "PFTask_hcalFrac3Zero_vs_pt");
  book1DProf(v_hist, histo, 50., -1, 4., -1., 2.);
  sprintf(histo, "PFTask_hcalFrac4Zero_vs_pt");
  book1DProf(v_hist, histo, 50., -1, 4., -1., 2.);
  sprintf(histo, "PFTask_hcalFrac5Zero_vs_pt");
  book1DProf(v_hist, histo, 50., -1, 4., -1., 2.);
  sprintf(histo, "PFTask_hcalFrac6Zero_vs_pt");
  book1DProf(v_hist, histo, 50., -1, 4., -1., 2.);
  sprintf(histo, "PFTask_hcalFrac7Zero_vs_pt");
  book1DProf(v_hist, histo, 50., -1, 4., -1., 2.);
  sprintf(histo, "PFTask_hcalFracAllZero_vs_pt");
  book1DProf(v_hist, histo, 50., -1, 4., -1., 2.);

  sprintf(histo, "PFTask_PdgId_vs_pt");              // added by Bryan
  book2D(    v_hist, histo, 10., 0.5, 10.5, 100., 0., 10.);       // added by Bryan
  relabel2D( v_hist, histo); // added by Bryan

  ////
  //// Bryan's Studies 
  ////
  
  sprintf(histo, "PFTask_PFEta_All");
  book1D(v_hist, histo, 24, -6., 6.);
  sprintf(histo, "PFTask_PFEta_pt_All");
  book1D(v_hist, histo, 24, -6., 6.);
  
  sprintf(histo, "PFTask_PFEta_ChargedHadron");
  book1D(v_hist, histo, 24, -6., 6.);
  sprintf(histo, "PFTask_PFEta_pt_ChargedHadron");
  book1D(v_hist, histo, 24, -6., 6.);

  sprintf(histo, "PFTask_PFEta_NeutralHadron");
  book1D(v_hist, histo, 24, -6., 6.);
  sprintf(histo, "PFTask_PFEta_pt_NeutralHadron");
  book1D(v_hist, histo, 24, -6., 6.);
  
  sprintf(histo, "PFTask_PFEta_Electron");
  book1D(v_hist, histo, 24, -6., 6.);
  sprintf(histo, "PFTask_PFEta_pt_Electron");
  book1D(v_hist, histo, 24, -6., 6.);

  sprintf(histo, "PFTask_PFEta_Muon");
  book1D(v_hist, histo, 24, -6., 6.);
  sprintf(histo, "PFTask_PFEta_pt_Muon");
  book1D(v_hist, histo, 24, -6., 6.);

  sprintf(histo, "PFTask_PFEta_Photon");
  book1D(v_hist, histo, 24, -6., 6.);
  sprintf(histo, "PFTask_PFEta_pt_Photon");
  book1D(v_hist, histo, 24, -6., 6.);

  sprintf(histo, "PFTask_PFEta_HFPhoton");
  book1D(v_hist, histo, 24, -6., 6.);
  sprintf(histo, "PFTask_PFEta_pt_HFPhoton");
  book1D(v_hist, histo, 24, -6., 6.);

  sprintf(histo, "PFTask_PFEta_HFHadron");
  book1D(v_hist, histo, 24, -6., 6.);
  sprintf(histo, "PFTask_PFEta_pt_HFHadron");
  book1D(v_hist, histo, 24, -6., 6.);

  ///
  /// MET
  ///
  
  sprintf(histo, "PFTask_MET");
  book1D(v_hist, histo, 300, 0., 300.);
  
  sprintf(histo, "PFParMET");
  book1D(v_hist, histo, 300, 0., 300.);

  sprintf(histo, "PFMET");
  book1D(v_hist, histo, 300, 0., 300.);
  
  //
  // Charged hadrons
  //
  
  sprintf(histo, "PFTask_ChargedHadron_TrackPtRatio_PtAbove5");
  book1D(v_hist, histo, 70., -0.2, 1.2);
  sprintf(histo, "PFTask_ChargedHadron_TrackPtRatio_Pt1To5");
  book1D(v_hist, histo, 70., -0.2, 1.2);
  sprintf(histo, "PFTask_ChargedHadron_TrackPtRatio_PtBelow1");
  book1D(v_hist, histo, 70., -0.2, 1.2);
  
  sprintf(histo,"PFTask_Profile_ChargedHadron_Endcap_PtAbove5");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);
  sprintf(histo,"PFTask_Profile_ChargedHadron_Endcap_Pt1To5");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);
  sprintf(histo,"PFTask_Profile_ChargedHadron_Endcap_PtBelow1");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);

  // PF candidate Pt not 
  sprintf(histo,"PFTask_Profile_ChargedHadron_Endcap_PtAbove5_Special");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);
  sprintf(histo,"PFTask_Profile_ChargedHadron_Endcap_Pt1To5_Special");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);
  sprintf(histo,"PFTask_Profile_ChargedHadron_Endcap_PtBelow1_Special");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);

  //
  // PF electrons
  //
  
  sprintf(histo, "PFTask_PFElectron_TrackPtRatio_PtAbove5");
  book1D(v_hist, histo, 70., -0.2, 1.2);
  sprintf(histo, "PFTask_PFElectron_TrackPtRatio_Pt1To5");
  book1D(v_hist, histo, 70., -0.2, 1.2);
  sprintf(histo, "PFTask_PFElectron_TrackPtRatio_PtBelow1");
  book1D(v_hist, histo, 70., -0.2, 1.2);
  
  sprintf(histo,"PFTask_Profile_PFElectron_Endcap_PtAbove5");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);
  sprintf(histo,"PFTask_Profile_PFElectron_Endcap_Pt1To5");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);
  sprintf(histo,"PFTask_Profile_PFElectron_Endcap_PtBelow1");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);

  //
  // PF photons
  //
  
  sprintf(histo,"PFTask_Profile_PFPhoton_Endcap_PtAbove5");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);
  sprintf(histo,"PFTask_Profile_PFPhoton_Endcap_Pt1To5");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);
  sprintf(histo,"PFTask_Profile_PFPhoton_Endcap_PtBelow1");
  book1DProf(v_hist, histo, 11, -3.5, 7.5, -1., 2., "S");
  relabelProfA(v_hist, histo);

  //
  // Neutral hadrons
  //
  
  sprintf(histo,"PFTask_hcalProfile_NeutralHadron_Endcap_PtAbove5");
  book1DProf(v_hist, histo, 7, 0.5,7.5, -1., 2., "S");
  sprintf(histo,"PFTask_hcalProfile_NeutralHadron_Endcap_Pt1To5");
  book1DProf(v_hist, histo, 7, 0.5,7.5, -1., 2., "S");
  sprintf(histo,"PFTask_hcalProfile_NeutralHadron_Endcap_PtBelow1");
  book1DProf(v_hist, histo, 7, 0.5,7.5, -1., 2., "S");
  
}
//
// relabel 1DProf histograms
//
void relabelProfA(TList *v_hist, std::string name)
{
  TProfile* htemp = (TProfile*) v_hist->FindObject(name.c_str());
  htemp->GetXaxis()->SetBinLabel(1,"Track");
  htemp->GetXaxis()->SetBinLabel(2,"ECAL");
  htemp->GetXaxis()->SetBinLabel(3,"HCAL");
  htemp->GetXaxis()->SetBinLabel(5,"HCAL d1");
  htemp->GetXaxis()->SetBinLabel(6,"HCAL d2");
  htemp->GetXaxis()->SetBinLabel(7,"HCAL d3");
  htemp->GetXaxis()->SetBinLabel(8,"HCAL d4");
  htemp->GetXaxis()->SetBinLabel(9,"HCAL d5");
  htemp->GetXaxis()->SetBinLabel(10,"HCAL d6");
  htemp->GetXaxis()->SetBinLabel(11,"HCAL d7");
}

void relabel2D(TList *v_hist, std::string name) // added by Bryan 
{
  TH2D* htemp = (TH2D*) v_hist->FindObject(name.c_str());
  htemp->GetXaxis()->SetBinLabel(1,"Charged Had");
  htemp->GetXaxis()->SetBinLabel(2,"Electron");
  htemp->GetXaxis()->SetBinLabel(4,"Photon");
  htemp->GetXaxis()->SetBinLabel(5,"Nuetral Had");
}
//
// Fill 1D histograms
//
void fill1D(TList *v_hist, std::string name, double value)
{
  TH1F* htemp = (TH1F*) v_hist->FindObject(name.c_str());
  htemp->Fill(value);
}
void fill1D(TList *v_hist, std::string name, double value, double value2)
{
  TH1F* htemp = (TH1F*) v_hist->FindObject(name.c_str());
  htemp->Fill(value,value2);
}
//
// Fill 1D Profile histograms
//
void fill1DProf(TList *v_hist, std::string name, double value, double valuey)
{
  TProfile* htemp = (TProfile*) v_hist->FindObject(name.c_str());
  htemp->Fill(value,valuey);
}
// 
// Fill 2D histograms
//
void fill2D(TList *v_hist, std::string name, double valuex, double valuey)
{
  TH2D* h_temp = (TH2D*) v_hist->FindObject(name.c_str());
  h_temp->Fill(valuex,valuey);
}
