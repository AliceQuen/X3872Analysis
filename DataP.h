//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Feb  9 21:19:51 2026 by ROOT version 6.24/06
// from TTree X_data/X(3872) Data
// found on file: mymultilep_1.root
//////////////////////////////////////////////////////////

#ifndef _DataP_h
#define _DataP_h 1

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"
using namespace std;

class DataP {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   vector<unsigned int> *TrigRes;
   vector<string>  *TrigNames;
   vector<unsigned int> *L1TrigRes;
   vector<string>  *MatchJpsiTrigNames;
   UInt_t          evtNum;
   UInt_t          runNum;
   UInt_t          lumiNum;
   UInt_t          nGoodPrimVtx;
   Float_t         priVtxX;
   Float_t         priVtxY;
   Float_t         priVtxZ;
   Float_t         priVtxXErr;
   Float_t         priVtxYErr;
   Float_t         priVtxZErr;
   Float_t         priVtxChiNorm;
   Float_t         priVtxChi;
   Float_t         priVtxCL;
   vector<float>   *priVtxXCorrY;
   vector<float>   *priVtxXCorrZ;
   vector<float>   *priVtxYCorrZ;
   UInt_t          nMu;
   vector<float>   *muPx;
   vector<float>   *muPy;
   vector<float>   *muPz;
   vector<float>   *muD0BS;
   vector<float>   *muD0EBS;
   vector<float>   *muD3dBS;
   vector<float>   *muD3dEBS;
   vector<float>   *muD0PV;
   vector<float>   *muD0EPV;
   vector<float>   *muDzPV;
   vector<float>   *muDzEPV;
   vector<float>   *muCharge;
   vector<float>   *muTrackIso;
   vector<int>     *muIsPatLooseMuon;
   vector<int>     *muIsPatTightMuon;
   vector<int>     *muIsPatSoftMuon;
   vector<int>     *muIsPatMediumMuon;
   vector<int>     *muJpsiFilterRes;
   vector<float>   *X_mu1Id;
   vector<float>   *X_mu2Id;
   vector<float>   *X_mu3Id;
   vector<float>   *X_mu4Id;
   vector<float>   *X_mass;
   vector<float>   *X_VtxProb;
   vector<float>   *X_Chi2;
   vector<float>   *X_ndof;
   vector<float>   *X_px;
   vector<float>   *X_py;
   vector<float>   *X_pz;
   vector<float>   *X_massErr;
   vector<float>   *X_JPiPi_mass;
   vector<float>   *X_JPiPi_VtxProb;
   vector<float>   *X_JPiPi_Chi2;
   vector<float>   *X_JPiPi_ndof;
   vector<float>   *X_JPiPi_px;
   vector<float>   *X_JPiPi_py;
   vector<float>   *X_JPiPi_pz;
   vector<float>   *X_JPiPi_massErr;
   vector<float>   *X_Jpsi1_mass;
   vector<float>   *X_Jpsi1_VtxProb;
   vector<float>   *X_Jpsi1_Chi2;
   vector<float>   *X_Jpsi1_ndof;
   vector<float>   *X_Jpsi1_px;
   vector<float>   *X_Jpsi1_py;
   vector<float>   *X_Jpsi1_pz;
   vector<float>   *X_Jpsi1_massErr;
   vector<float>   *X_Jpsi2_mass;
   vector<float>   *X_Jpsi2_VtxProb;
   vector<float>   *X_Jpsi2_Chi2;
   vector<float>   *X_Jpsi2_ndof;
   vector<float>   *X_Jpsi2_px;
   vector<float>   *X_Jpsi2_py;
   vector<float>   *X_Jpsi2_pz;
   vector<float>   *X_Jpsi2_massErr;
   vector<float>   *X_JPiPi_Pi1Id;
   vector<float>   *X_JPiPi_Pi2Id;
   vector<float>   *X_JPiPi_Pi1px;
   vector<float>   *X_JPiPi_Pi1py;
   vector<float>   *X_JPiPi_Pi1pz;
   vector<float>   *X_JPiPi_Pi2px;
   vector<float>   *X_JPiPi_Pi2py;
   vector<float>   *X_JPiPi_Pi2pz;
   vector<float>   *cs_X_Jpsi1_mass;
   vector<float>   *cs_X_Jpsi1_VtxProb;
   vector<float>   *cs_X_Jpsi1_Chi2;
   vector<float>   *cs_X_Jpsi1_ndof;
   vector<float>   *cs_X_Jpsi1_px;
   vector<float>   *cs_X_Jpsi1_py;
   vector<float>   *cs_X_Jpsi1_pz;
   vector<float>   *cs_X_Jpsi1_massErr;
   vector<float>   *cs_X_Jpsi2_mass;
   vector<float>   *cs_X_Jpsi2_VtxProb;
   vector<float>   *cs_X_Jpsi2_Chi2;
   vector<float>   *cs_X_Jpsi2_ndof;
   vector<float>   *cs_X_Jpsi2_px;
   vector<float>   *cs_X_Jpsi2_py;
   vector<float>   *cs_X_Jpsi2_pz;
   vector<float>   *cs_X_Jpsi2_massErr;
   vector<float>   *cs_X_JPiPi_mass;
   vector<float>   *cs_X_JPiPi_VtxProb;
   vector<float>   *cs_X_JPiPi_Chi2;
   vector<float>   *cs_X_JPiPi_ndof;
   vector<float>   *cs_X_JPiPi_px ;
   vector<float>   *cs_X_JPiPi_py;
   vector<float>   *cs_X_JPiPi_pz;
   vector<float>   *cs_X_JPiPi_massErr;
   vector<float>   *cs_X_mass_Psi2S;
   vector<float>   *cs_X_VtxProb_Psi2S;
   vector<float>   *cs_X_Chi2_Psi2S;
   vector<float>   *cs_X_ndof_Psi2S;
   vector<float>   *cs_X_px_Psi2S;
   vector<float>   *cs_X_py_Psi2S;
   vector<float>   *cs_X_pz_Psi2S;
   vector<float>   *cs_X_massErr_Psi2S;
   vector<float>   *cs_X_JPiPi_mass_Psi2S;
   vector<float>   *cs_X_JPiPi_VtxProb_Psi2S;
   vector<float>   *cs_X_JPiPi_Chi2_Psi2S;
   vector<float>   *cs_X_JPiPi_ndof_Psi2S;
   vector<float>   *cs_X_JPiPi_px_Psi2S;
   vector<float>   *cs_X_JPiPi_py_Psi2S;
   vector<float>   *cs_X_JPiPi_pz_Psi2S;
   vector<float>   *cs_X_JPiPi_massErr_Psi2S;
   vector<float>   *cs_X_mass_X3872;
   vector<float>   *cs_X_VtxProb_X3872;
   vector<float>   *cs_X_Chi2_X3872;
   vector<float>   *cs_X_ndof_X3872;
   vector<float>   *cs_X_px_X3872;
   vector<float>   *cs_X_py_X3872;
   vector<float>   *cs_X_pz_X3872;
   vector<float>   *cs_X_massErr_X3872;
   vector<float>   *cs_X_JPiPi_mass_X3872;
   vector<float>   *cs_X_JPiPi_VtxProb_X3872;
   vector<float>   *cs_X_JPiPi_Chi2_X3872;
   vector<float>   *cs_X_JPiPi_ndof_X3872;
   vector<float>   *cs_X_JPiPi_px_X3872;
   vector<float>   *cs_X_JPiPi_py_X3872;
   vector<float>   *cs_X_JPiPi_pz_X3872;
   vector<float>   *cs_X_JPiPi_massErr_X3872;

   // List of branches
   TBranch        *b_TrigRes;   //!
   TBranch        *b_TrigNames;   //!
   TBranch        *b_L1TrigRes;   //!
   TBranch        *b_MatchJpsiTrigNames;   //!
   TBranch        *b_evtNum;   //!
   TBranch        *b_runNum;   //!
   TBranch        *b_lumiNum;   //!
   TBranch        *b_nGoodPrimVtx;   //!
   TBranch        *b_priVtxX;   //!
   TBranch        *b_priVtxY;   //!
   TBranch        *b_priVtxZ;   //!
   TBranch        *b_priVtxXErr;   //!
   TBranch        *b_priVtxYErr;   //!
   TBranch        *b_priVtxZErr;   //!
   TBranch        *b_priVtxChiNorm;   //!
   TBranch        *b_priVtxChi;   //!
   TBranch        *b_priVtxCL;   //!
   TBranch        *b_priVtxXCorrY;   //!
   TBranch        *b_priVtxXCorrZ;   //!
   TBranch        *b_priVtxYCorrZ;   //!
   TBranch        *b_nMu;   //!
   TBranch        *b_muPx;   //!
   TBranch        *b_muPy;   //!
   TBranch        *b_muPz;   //!
   TBranch        *b_muD0BS;   //!
   TBranch        *b_muD0EBS;   //!
   TBranch        *b_muD3dBS;   //!
   TBranch        *b_muD3dEBS;   //!
   TBranch        *b_muD0PV;   //!
   TBranch        *b_muD0EPV;   //!
   TBranch        *b_muDzPV;   //!
   TBranch        *b_muDzEPV;   //!
   TBranch        *b_muCharge;   //!
   TBranch        *b_muTrackIso;   //!
   TBranch        *b_muIsPatLooseMuon;   //!
   TBranch        *b_muIsPatTightMuon;   //!
   TBranch        *b_muIsPatSoftMuon;   //!
   TBranch        *b_muIsPatMediumMuon;   //!
   TBranch        *b_muJpsiFilterRes;   //!
   TBranch        *b_X_mu1Id;   //!
   TBranch        *b_X_mu2Id;   //!
   TBranch        *b_X_mu3Id;   //!
   TBranch        *b_X_mu4Id;   //!
   TBranch        *b_X_mass;   //!
   TBranch        *b_X_VtxProb;   //!
   TBranch        *b_X_Chi2;   //!
   TBranch        *b_X_ndof;   //!
   TBranch        *b_X_px;   //!
   TBranch        *b_X_py;   //!
   TBranch        *b_X_pz;   //!
   TBranch        *b_X_massErr;   //!
   TBranch        *b_X_JPiPi_mass;   //!
   TBranch        *b_X_JPiPi_VtxProb;   //!
   TBranch        *b_X_JPiPi_Chi2;   //!
   TBranch        *b_X_JPiPi_ndof;   //!
   TBranch        *b_X_JPiPi_px;   //!
   TBranch        *b_X_JPiPi_py;   //!
   TBranch        *b_X_JPiPi_pz;   //!
   TBranch        *b_X_JPiPi_massErr;   //!
   TBranch        *b_X_Jpsi1_mass;   //!
   TBranch        *b_X_Jpsi1_VtxProb;   //!
   TBranch        *b_X_Jpsi1_Chi2;   //!
   TBranch        *b_X_Jpsi1_ndof;   //!
   TBranch        *b_X_Jpsi1_px;   //!
   TBranch        *b_X_Jpsi1_py;   //!
   TBranch        *b_X_Jpsi1_pz;   //!
   TBranch        *b_X_Jpsi1_massErr;   //!
   TBranch        *b_X_Jpsi2_mass;   //!
   TBranch        *b_X_Jpsi2_VtxProb;   //!
   TBranch        *b_X_Jpsi2_Chi2;   //!
   TBranch        *b_X_Jpsi2_ndof;   //!
   TBranch        *b_X_Jpsi2_px;   //!
   TBranch        *b_X_Jpsi2_py;   //!
   TBranch        *b_X_Jpsi2_pz;   //!
   TBranch        *b_X_Jpsi2_massErr;   //!
   TBranch        *b_X_JPiPi_Pi1Id;   //!
   TBranch        *b_X_JPiPi_Pi2Id;   //!
   TBranch        *b_X_JPiPi_Pi1px;   //!
   TBranch        *b_X_JPiPi_Pi1py;   //!
   TBranch        *b_X_JPiPi_Pi1pz;   //!
   TBranch        *b_X_JPiPi_Pi2px;   //!
   TBranch        *b_X_JPiPi_Pi2py;   //!
   TBranch        *b_X_JPiPi_Pi2pz;   //!
   TBranch        *b_cs_X_Jpsi1_mass;   //!
   TBranch        *b_cs_X_Jpsi1_VtxProb;   //!
   TBranch        *b_cs_X_Jpsi1_Chi2;   //!
   TBranch        *b_cs_X_Jpsi1_ndof;   //!
   TBranch        *b_cs_X_Jpsi1_px;   //!
   TBranch        *b_cs_X_Jpsi1_py;   //!
   TBranch        *b_cs_X_Jpsi1_pz;   //!
   TBranch        *b_cs_X_Jpsi1_massErr;   //!
   TBranch        *b_cs_X_Jpsi2_mass;   //!
   TBranch        *b_cs_X_Jpsi2_VtxProb;   //!
   TBranch        *b_cs_X_Jpsi2_Chi2;   //!
   TBranch        *b_cs_X_Jpsi2_ndof;   //!
   TBranch        *b_cs_X_Jpsi2_px;   //!
   TBranch        *b_cs_X_Jpsi2_py;   //!
   TBranch        *b_cs_X_Jpsi2_pz;   //!
   TBranch        *b_cs_X_Jpsi2_massErr;   //!
   TBranch        *b_cs_X_JPiPi_mass;   //!
   TBranch        *b_cs_X_JPiPi_VtxProb;   //!
   TBranch        *b_cs_X_JPiPi_Chi2;   //!
   TBranch        *b_cs_X_JPiPi_ndof;   //!
   TBranch        *b_cs_X_JPiPi_px ;   //!
   TBranch        *b_cs_X_JPiPi_py;   //!
   TBranch        *b_cs_X_JPiPi_pz;   //!
   TBranch        *b_cs_X_JPiPi_massErr;   //!
   TBranch        *b_cs_X_mass_Psi2S;   //!
   TBranch        *b_cs_X_VtxProb_Psi2S;   //!
   TBranch        *b_cs_X_Chi2_Psi2S;   //!
   TBranch        *b_cs_X_ndof_Psi2S;   //!
   TBranch        *b_cs_X_px_Psi2S;   //!
   TBranch        *b_cs_X_py_Psi2S;   //!
   TBranch        *b_cs_X_pz_Psi2S;   //!
   TBranch        *b_cs_X_massErr_Psi2S;   //!
   TBranch        *b_cs_X_JPiPi_mass_Psi2S;   //!
   TBranch        *b_cs_X_JPiPi_VtxProb_Psi2S;   //!
   TBranch        *b_cs_X_JPiPi_Chi2_Psi2S;   //!
   TBranch        *b_cs_X_JPiPi_ndof_Psi2S;   //!
   TBranch        *b_cs_X_JPiPi_px_Psi2S;   //!
   TBranch        *b_cs_X_JPiPi_py_Psi2S;   //!
   TBranch        *b_cs_X_JPiPi_pz_Psi2S;   //!
   TBranch        *b_cs_X_JPiPi_massErr_Psi2S;   //!
   TBranch        *b_cs_X_mass_X3872;   //!
   TBranch        *b_cs_X_VtxProb_X3872;   //!
   TBranch        *b_cs_X_Chi2_X3872;   //!
   TBranch        *b_cs_X_ndof_X3872;   //!
   TBranch        *b_cs_X_px_X3872;   //!
   TBranch        *b_cs_X_py_X3872;   //!
   TBranch        *b_cs_X_pz_X3872;   //!
   TBranch        *b_cs_X_massErr_X3872;   //!
   TBranch        *b_cs_X_JPiPi_mass_X3872;   //!
   TBranch        *b_cs_X_JPiPi_VtxProb_X3872;   //!
   TBranch        *b_cs_X_JPiPi_Chi2_X3872;   //!
   TBranch        *b_cs_X_JPiPi_ndof_X3872;   //!
   TBranch        *b_cs_X_JPiPi_px_X3872;   //!
   TBranch        *b_cs_X_JPiPi_py_X3872;   //!
   TBranch        *b_cs_X_JPiPi_pz_X3872;   //!
   TBranch        *b_cs_X_JPiPi_massErr_X3872;   //!

   DataP(TTree *tree=0);
   virtual ~DataP();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(TString outputname);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef DataP_cxx
DataP::DataP(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("mymultilep_1.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("mymultilep_1.root");
      }
      TDirectory * dir = (TDirectory*)f->Get("mymultilep_1.root:/mkcands");
      dir->GetObject("X_data",tree);

   }
   Init(tree);
}

DataP::~DataP()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t DataP::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t DataP::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void DataP::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   TrigRes = 0;
   TrigNames = 0;
   L1TrigRes = 0;
   MatchJpsiTrigNames = 0;
   priVtxXCorrY = 0;
   priVtxXCorrZ = 0;
   priVtxYCorrZ = 0;
   muPx = 0;
   muPy = 0;
   muPz = 0;
   muD0BS = 0;
   muD0EBS = 0;
   muD3dBS = 0;
   muD3dEBS = 0;
   muD0PV = 0;
   muD0EPV = 0;
   muDzPV = 0;
   muDzEPV = 0;
   muCharge = 0;
   muTrackIso = 0;
   muIsPatLooseMuon = 0;
   muIsPatTightMuon = 0;
   muIsPatSoftMuon = 0;
   muIsPatMediumMuon = 0;
   muJpsiFilterRes = 0;
   X_mu1Id = 0;
   X_mu2Id = 0;
   X_mu3Id = 0;
   X_mu4Id = 0;
   X_mass = 0;
   X_VtxProb = 0;
   X_Chi2 = 0;
   X_ndof = 0;
   X_px = 0;
   X_py = 0;
   X_pz = 0;
   X_massErr = 0;
   X_JPiPi_mass = 0;
   X_JPiPi_VtxProb = 0;
   X_JPiPi_Chi2 = 0;
   X_JPiPi_ndof = 0;
   X_JPiPi_px = 0;
   X_JPiPi_py = 0;
   X_JPiPi_pz = 0;
   X_JPiPi_massErr = 0;
   X_Jpsi1_mass = 0;
   X_Jpsi1_VtxProb = 0;
   X_Jpsi1_Chi2 = 0;
   X_Jpsi1_ndof = 0;
   X_Jpsi1_px = 0;
   X_Jpsi1_py = 0;
   X_Jpsi1_pz = 0;
   X_Jpsi1_massErr = 0;
   X_Jpsi2_mass = 0;
   X_Jpsi2_VtxProb = 0;
   X_Jpsi2_Chi2 = 0;
   X_Jpsi2_ndof = 0;
   X_Jpsi2_px = 0;
   X_Jpsi2_py = 0;
   X_Jpsi2_pz = 0;
   X_Jpsi2_massErr = 0;
   X_JPiPi_Pi1Id = 0;
   X_JPiPi_Pi2Id = 0;
   X_JPiPi_Pi1px = 0;
   X_JPiPi_Pi1py = 0;
   X_JPiPi_Pi1pz = 0;
   X_JPiPi_Pi2px = 0;
   X_JPiPi_Pi2py = 0;
   X_JPiPi_Pi2pz = 0;
   cs_X_Jpsi1_mass = 0;
   cs_X_Jpsi1_VtxProb = 0;
   cs_X_Jpsi1_Chi2 = 0;
   cs_X_Jpsi1_ndof = 0;
   cs_X_Jpsi1_px = 0;
   cs_X_Jpsi1_py = 0;
   cs_X_Jpsi1_pz = 0;
   cs_X_Jpsi1_massErr = 0;
   cs_X_Jpsi2_mass = 0;
   cs_X_Jpsi2_VtxProb = 0;
   cs_X_Jpsi2_Chi2 = 0;
   cs_X_Jpsi2_ndof = 0;
   cs_X_Jpsi2_px = 0;
   cs_X_Jpsi2_py = 0;
   cs_X_Jpsi2_pz = 0;
   cs_X_Jpsi2_massErr = 0;
   cs_X_JPiPi_mass = 0;
   cs_X_JPiPi_VtxProb = 0;
   cs_X_JPiPi_Chi2 = 0;
   cs_X_JPiPi_ndof = 0;
   cs_X_JPiPi_px  = 0;
   cs_X_JPiPi_py = 0;
   cs_X_JPiPi_pz = 0;
   cs_X_JPiPi_massErr = 0;
   cs_X_mass_Psi2S = 0;
   cs_X_VtxProb_Psi2S = 0;
   cs_X_Chi2_Psi2S = 0;
   cs_X_ndof_Psi2S = 0;
   cs_X_px_Psi2S = 0;
   cs_X_py_Psi2S = 0;
   cs_X_pz_Psi2S = 0;
   cs_X_massErr_Psi2S = 0;
   cs_X_JPiPi_mass_Psi2S = 0;
   cs_X_JPiPi_VtxProb_Psi2S = 0;
   cs_X_JPiPi_Chi2_Psi2S = 0;
   cs_X_JPiPi_ndof_Psi2S = 0;
   cs_X_JPiPi_px_Psi2S = 0;
   cs_X_JPiPi_py_Psi2S = 0;
   cs_X_JPiPi_pz_Psi2S = 0;
   cs_X_JPiPi_massErr_Psi2S = 0;
   cs_X_mass_X3872 = 0;
   cs_X_VtxProb_X3872 = 0;
   cs_X_Chi2_X3872 = 0;
   cs_X_ndof_X3872 = 0;
   cs_X_px_X3872 = 0;
   cs_X_py_X3872 = 0;
   cs_X_pz_X3872 = 0;
   cs_X_massErr_X3872 = 0;
   cs_X_JPiPi_mass_X3872 = 0;
   cs_X_JPiPi_VtxProb_X3872 = 0;
   cs_X_JPiPi_Chi2_X3872 = 0;
   cs_X_JPiPi_ndof_X3872 = 0;
   cs_X_JPiPi_px_X3872 = 0;
   cs_X_JPiPi_py_X3872 = 0;
   cs_X_JPiPi_pz_X3872 = 0;
   cs_X_JPiPi_massErr_X3872 = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("TrigRes", &TrigRes, &b_TrigRes);
   fChain->SetBranchAddress("TrigNames", &TrigNames, &b_TrigNames);
   fChain->SetBranchAddress("L1TrigRes", &L1TrigRes, &b_L1TrigRes);
   fChain->SetBranchAddress("MatchJpsiTrigNames", &MatchJpsiTrigNames, &b_MatchJpsiTrigNames);
   fChain->SetBranchAddress("evtNum", &evtNum, &b_evtNum);
   fChain->SetBranchAddress("runNum", &runNum, &b_runNum);
   fChain->SetBranchAddress("lumiNum", &lumiNum, &b_lumiNum);
   fChain->SetBranchAddress("nGoodPrimVtx", &nGoodPrimVtx, &b_nGoodPrimVtx);
   fChain->SetBranchAddress("priVtxX", &priVtxX, &b_priVtxX);
   fChain->SetBranchAddress("priVtxY", &priVtxY, &b_priVtxY);
   fChain->SetBranchAddress("priVtxZ", &priVtxZ, &b_priVtxZ);
   fChain->SetBranchAddress("priVtxXErr", &priVtxXErr, &b_priVtxXErr);
   fChain->SetBranchAddress("priVtxYErr", &priVtxYErr, &b_priVtxYErr);
   fChain->SetBranchAddress("priVtxZErr", &priVtxZErr, &b_priVtxZErr);
   fChain->SetBranchAddress("priVtxChiNorm", &priVtxChiNorm, &b_priVtxChiNorm);
   fChain->SetBranchAddress("priVtxChi", &priVtxChi, &b_priVtxChi);
   fChain->SetBranchAddress("priVtxCL", &priVtxCL, &b_priVtxCL);
   fChain->SetBranchAddress("priVtxXCorrY", &priVtxXCorrY, &b_priVtxXCorrY);
   fChain->SetBranchAddress("priVtxXCorrZ", &priVtxXCorrZ, &b_priVtxXCorrZ);
   fChain->SetBranchAddress("priVtxYCorrZ", &priVtxYCorrZ, &b_priVtxYCorrZ);
   fChain->SetBranchAddress("nMu", &nMu, &b_nMu);
   fChain->SetBranchAddress("muPx", &muPx, &b_muPx);
   fChain->SetBranchAddress("muPy", &muPy, &b_muPy);
   fChain->SetBranchAddress("muPz", &muPz, &b_muPz);
   fChain->SetBranchAddress("muD0BS", &muD0BS, &b_muD0BS);
   fChain->SetBranchAddress("muD0EBS", &muD0EBS, &b_muD0EBS);
   fChain->SetBranchAddress("muD3dBS", &muD3dBS, &b_muD3dBS);
   fChain->SetBranchAddress("muD3dEBS", &muD3dEBS, &b_muD3dEBS);
   fChain->SetBranchAddress("muD0PV", &muD0PV, &b_muD0PV);
   fChain->SetBranchAddress("muD0EPV", &muD0EPV, &b_muD0EPV);
   fChain->SetBranchAddress("muDzPV", &muDzPV, &b_muDzPV);
   fChain->SetBranchAddress("muDzEPV", &muDzEPV, &b_muDzEPV);
   fChain->SetBranchAddress("muCharge", &muCharge, &b_muCharge);
   fChain->SetBranchAddress("muTrackIso", &muTrackIso, &b_muTrackIso);
   fChain->SetBranchAddress("muIsPatLooseMuon", &muIsPatLooseMuon, &b_muIsPatLooseMuon);
   fChain->SetBranchAddress("muIsPatTightMuon", &muIsPatTightMuon, &b_muIsPatTightMuon);
   fChain->SetBranchAddress("muIsPatSoftMuon", &muIsPatSoftMuon, &b_muIsPatSoftMuon);
   fChain->SetBranchAddress("muIsPatMediumMuon", &muIsPatMediumMuon, &b_muIsPatMediumMuon);
   fChain->SetBranchAddress("muJpsiFilterRes", &muJpsiFilterRes, &b_muJpsiFilterRes);
   fChain->SetBranchAddress("X_mu1Id", &X_mu1Id, &b_X_mu1Id);
   fChain->SetBranchAddress("X_mu2Id", &X_mu2Id, &b_X_mu2Id);
   fChain->SetBranchAddress("X_mu3Id", &X_mu3Id, &b_X_mu3Id);
   fChain->SetBranchAddress("X_mu4Id", &X_mu4Id, &b_X_mu4Id);
   fChain->SetBranchAddress("X_mass", &X_mass, &b_X_mass);
   fChain->SetBranchAddress("X_VtxProb", &X_VtxProb, &b_X_VtxProb);
   fChain->SetBranchAddress("X_Chi2", &X_Chi2, &b_X_Chi2);
   fChain->SetBranchAddress("X_ndof", &X_ndof, &b_X_ndof);
   fChain->SetBranchAddress("X_px", &X_px, &b_X_px);
   fChain->SetBranchAddress("X_py", &X_py, &b_X_py);
   fChain->SetBranchAddress("X_pz", &X_pz, &b_X_pz);
   fChain->SetBranchAddress("X_massErr", &X_massErr, &b_X_massErr);
   fChain->SetBranchAddress("X_JPiPi_mass", &X_JPiPi_mass, &b_X_JPiPi_mass);
   fChain->SetBranchAddress("X_JPiPi_VtxProb", &X_JPiPi_VtxProb, &b_X_JPiPi_VtxProb);
   fChain->SetBranchAddress("X_JPiPi_Chi2", &X_JPiPi_Chi2, &b_X_JPiPi_Chi2);
   fChain->SetBranchAddress("X_JPiPi_ndof", &X_JPiPi_ndof, &b_X_JPiPi_ndof);
   fChain->SetBranchAddress("X_JPiPi_px", &X_JPiPi_px, &b_X_JPiPi_px);
   fChain->SetBranchAddress("X_JPiPi_py", &X_JPiPi_py, &b_X_JPiPi_py);
   fChain->SetBranchAddress("X_JPiPi_pz", &X_JPiPi_pz, &b_X_JPiPi_pz);
   fChain->SetBranchAddress("X_JPiPi_massErr", &X_JPiPi_massErr, &b_X_JPiPi_massErr);
   fChain->SetBranchAddress("X_Jpsi1_mass", &X_Jpsi1_mass, &b_X_Jpsi1_mass);
   fChain->SetBranchAddress("X_Jpsi1_VtxProb", &X_Jpsi1_VtxProb, &b_X_Jpsi1_VtxProb);
   fChain->SetBranchAddress("X_Jpsi1_Chi2", &X_Jpsi1_Chi2, &b_X_Jpsi1_Chi2);
   fChain->SetBranchAddress("X_Jpsi1_ndof", &X_Jpsi1_ndof, &b_X_Jpsi1_ndof);
   fChain->SetBranchAddress("X_Jpsi1_px", &X_Jpsi1_px, &b_X_Jpsi1_px);
   fChain->SetBranchAddress("X_Jpsi1_py", &X_Jpsi1_py, &b_X_Jpsi1_py);
   fChain->SetBranchAddress("X_Jpsi1_pz", &X_Jpsi1_pz, &b_X_Jpsi1_pz);
   fChain->SetBranchAddress("X_Jpsi1_massErr", &X_Jpsi1_massErr, &b_X_Jpsi1_massErr);
   fChain->SetBranchAddress("X_Jpsi2_mass", &X_Jpsi2_mass, &b_X_Jpsi2_mass);
   fChain->SetBranchAddress("X_Jpsi2_VtxProb", &X_Jpsi2_VtxProb, &b_X_Jpsi2_VtxProb);
   fChain->SetBranchAddress("X_Jpsi2_Chi2", &X_Jpsi2_Chi2, &b_X_Jpsi2_Chi2);
   fChain->SetBranchAddress("X_Jpsi2_ndof", &X_Jpsi2_ndof, &b_X_Jpsi2_ndof);
   fChain->SetBranchAddress("X_Jpsi2_px", &X_Jpsi2_px, &b_X_Jpsi2_px);
   fChain->SetBranchAddress("X_Jpsi2_py", &X_Jpsi2_py, &b_X_Jpsi2_py);
   fChain->SetBranchAddress("X_Jpsi2_pz", &X_Jpsi2_pz, &b_X_Jpsi2_pz);
   fChain->SetBranchAddress("X_Jpsi2_massErr", &X_Jpsi2_massErr, &b_X_Jpsi2_massErr);
   fChain->SetBranchAddress("X_JPiPi_Pi1Id", &X_JPiPi_Pi1Id, &b_X_JPiPi_Pi1Id);
   fChain->SetBranchAddress("X_JPiPi_Pi2Id", &X_JPiPi_Pi2Id, &b_X_JPiPi_Pi2Id);
   fChain->SetBranchAddress("X_JPiPi_Pi1px", &X_JPiPi_Pi1px, &b_X_JPiPi_Pi1px);
   fChain->SetBranchAddress("X_JPiPi_Pi1py", &X_JPiPi_Pi1py, &b_X_JPiPi_Pi1py);
   fChain->SetBranchAddress("X_JPiPi_Pi1pz", &X_JPiPi_Pi1pz, &b_X_JPiPi_Pi1pz);
   fChain->SetBranchAddress("X_JPiPi_Pi2px", &X_JPiPi_Pi2px, &b_X_JPiPi_Pi2px);
   fChain->SetBranchAddress("X_JPiPi_Pi2py", &X_JPiPi_Pi2py, &b_X_JPiPi_Pi2py);
   fChain->SetBranchAddress("X_JPiPi_Pi2pz", &X_JPiPi_Pi2pz, &b_X_JPiPi_Pi2pz);
   fChain->SetBranchAddress("cs_X_Jpsi1_mass", &cs_X_Jpsi1_mass, &b_cs_X_Jpsi1_mass);
   fChain->SetBranchAddress("cs_X_Jpsi1_VtxProb", &cs_X_Jpsi1_VtxProb, &b_cs_X_Jpsi1_VtxProb);
   fChain->SetBranchAddress("cs_X_Jpsi1_Chi2", &cs_X_Jpsi1_Chi2, &b_cs_X_Jpsi1_Chi2);
   fChain->SetBranchAddress("cs_X_Jpsi1_ndof", &cs_X_Jpsi1_ndof, &b_cs_X_Jpsi1_ndof);
   fChain->SetBranchAddress("cs_X_Jpsi1_px", &cs_X_Jpsi1_px, &b_cs_X_Jpsi1_px);
   fChain->SetBranchAddress("cs_X_Jpsi1_py", &cs_X_Jpsi1_py, &b_cs_X_Jpsi1_py);
   fChain->SetBranchAddress("cs_X_Jpsi1_pz", &cs_X_Jpsi1_pz, &b_cs_X_Jpsi1_pz);
   fChain->SetBranchAddress("cs_X_Jpsi1_massErr", &cs_X_Jpsi1_massErr, &b_cs_X_Jpsi1_massErr);
   fChain->SetBranchAddress("cs_X_Jpsi2_mass", &cs_X_Jpsi2_mass, &b_cs_X_Jpsi2_mass);
   fChain->SetBranchAddress("cs_X_Jpsi2_VtxProb", &cs_X_Jpsi2_VtxProb, &b_cs_X_Jpsi2_VtxProb);
   fChain->SetBranchAddress("cs_X_Jpsi2_Chi2", &cs_X_Jpsi2_Chi2, &b_cs_X_Jpsi2_Chi2);
   fChain->SetBranchAddress("cs_X_Jpsi2_ndof", &cs_X_Jpsi2_ndof, &b_cs_X_Jpsi2_ndof);
   fChain->SetBranchAddress("cs_X_Jpsi2_px", &cs_X_Jpsi2_px, &b_cs_X_Jpsi2_px);
   fChain->SetBranchAddress("cs_X_Jpsi2_py", &cs_X_Jpsi2_py, &b_cs_X_Jpsi2_py);
   fChain->SetBranchAddress("cs_X_Jpsi2_pz", &cs_X_Jpsi2_pz, &b_cs_X_Jpsi2_pz);
   fChain->SetBranchAddress("cs_X_Jpsi2_massErr", &cs_X_Jpsi2_massErr, &b_cs_X_Jpsi2_massErr);
   fChain->SetBranchAddress("cs_X_JPiPi_mass", &cs_X_JPiPi_mass, &b_cs_X_JPiPi_mass);
   fChain->SetBranchAddress("cs_X_JPiPi_VtxProb", &cs_X_JPiPi_VtxProb, &b_cs_X_JPiPi_VtxProb);
   fChain->SetBranchAddress("cs_X_JPiPi_Chi2", &cs_X_JPiPi_Chi2, &b_cs_X_JPiPi_Chi2);
   fChain->SetBranchAddress("cs_X_JPiPi_ndof", &cs_X_JPiPi_ndof, &b_cs_X_JPiPi_ndof);
   fChain->SetBranchAddress("cs_X_JPiPi_px ", &cs_X_JPiPi_px , &b_cs_X_JPiPi_px );
   fChain->SetBranchAddress("cs_X_JPiPi_py", &cs_X_JPiPi_py, &b_cs_X_JPiPi_py);
   fChain->SetBranchAddress("cs_X_JPiPi_pz", &cs_X_JPiPi_pz, &b_cs_X_JPiPi_pz);
   fChain->SetBranchAddress("cs_X_JPiPi_massErr", &cs_X_JPiPi_massErr, &b_cs_X_JPiPi_massErr);
   fChain->SetBranchAddress("cs_X_mass_Psi2S", &cs_X_mass_Psi2S, &b_cs_X_mass_Psi2S);
   fChain->SetBranchAddress("cs_X_VtxProb_Psi2S", &cs_X_VtxProb_Psi2S, &b_cs_X_VtxProb_Psi2S);
   fChain->SetBranchAddress("cs_X_Chi2_Psi2S", &cs_X_Chi2_Psi2S, &b_cs_X_Chi2_Psi2S);
   fChain->SetBranchAddress("cs_X_ndof_Psi2S", &cs_X_ndof_Psi2S, &b_cs_X_ndof_Psi2S);
   fChain->SetBranchAddress("cs_X_px_Psi2S", &cs_X_px_Psi2S, &b_cs_X_px_Psi2S);
   fChain->SetBranchAddress("cs_X_py_Psi2S", &cs_X_py_Psi2S, &b_cs_X_py_Psi2S);
   fChain->SetBranchAddress("cs_X_pz_Psi2S", &cs_X_pz_Psi2S, &b_cs_X_pz_Psi2S);
   fChain->SetBranchAddress("cs_X_massErr_Psi2S", &cs_X_massErr_Psi2S, &b_cs_X_massErr_Psi2S);
   fChain->SetBranchAddress("cs_X_JPiPi_mass_Psi2S", &cs_X_JPiPi_mass_Psi2S, &b_cs_X_JPiPi_mass_Psi2S);
   fChain->SetBranchAddress("cs_X_JPiPi_VtxProb_Psi2S", &cs_X_JPiPi_VtxProb_Psi2S, &b_cs_X_JPiPi_VtxProb_Psi2S);
   fChain->SetBranchAddress("cs_X_JPiPi_Chi2_Psi2S", &cs_X_JPiPi_Chi2_Psi2S, &b_cs_X_JPiPi_Chi2_Psi2S);
   fChain->SetBranchAddress("cs_X_JPiPi_ndof_Psi2S", &cs_X_JPiPi_ndof_Psi2S, &b_cs_X_JPiPi_ndof_Psi2S);
   fChain->SetBranchAddress("cs_X_JPiPi_px_Psi2S", &cs_X_JPiPi_px_Psi2S, &b_cs_X_JPiPi_px_Psi2S);
   fChain->SetBranchAddress("cs_X_JPiPi_py_Psi2S", &cs_X_JPiPi_py_Psi2S, &b_cs_X_JPiPi_py_Psi2S);
   fChain->SetBranchAddress("cs_X_JPiPi_pz_Psi2S", &cs_X_JPiPi_pz_Psi2S, &b_cs_X_JPiPi_pz_Psi2S);
   fChain->SetBranchAddress("cs_X_JPiPi_massErr_Psi2S", &cs_X_JPiPi_massErr_Psi2S, &b_cs_X_JPiPi_massErr_Psi2S);
   fChain->SetBranchAddress("cs_X_mass_X3872", &cs_X_mass_X3872, &b_cs_X_mass_X3872);
   fChain->SetBranchAddress("cs_X_VtxProb_X3872", &cs_X_VtxProb_X3872, &b_cs_X_VtxProb_X3872);
   fChain->SetBranchAddress("cs_X_Chi2_X3872", &cs_X_Chi2_X3872, &b_cs_X_Chi2_X3872);
   fChain->SetBranchAddress("cs_X_ndof_X3872", &cs_X_ndof_X3872, &b_cs_X_ndof_X3872);
   fChain->SetBranchAddress("cs_X_px_X3872", &cs_X_px_X3872, &b_cs_X_px_X3872);
   fChain->SetBranchAddress("cs_X_py_X3872", &cs_X_py_X3872, &b_cs_X_py_X3872);
   fChain->SetBranchAddress("cs_X_pz_X3872", &cs_X_pz_X3872, &b_cs_X_pz_X3872);
   fChain->SetBranchAddress("cs_X_massErr_X3872", &cs_X_massErr_X3872, &b_cs_X_massErr_X3872);
   fChain->SetBranchAddress("cs_X_JPiPi_mass_X3872", &cs_X_JPiPi_mass_X3872, &b_cs_X_JPiPi_mass_X3872);
   fChain->SetBranchAddress("cs_X_JPiPi_VtxProb_X3872", &cs_X_JPiPi_VtxProb_X3872, &b_cs_X_JPiPi_VtxProb_X3872);
   fChain->SetBranchAddress("cs_X_JPiPi_Chi2_X3872", &cs_X_JPiPi_Chi2_X3872, &b_cs_X_JPiPi_Chi2_X3872);
   fChain->SetBranchAddress("cs_X_JPiPi_ndof_X3872", &cs_X_JPiPi_ndof_X3872, &b_cs_X_JPiPi_ndof_X3872);
   fChain->SetBranchAddress("cs_X_JPiPi_px_X3872", &cs_X_JPiPi_px_X3872, &b_cs_X_JPiPi_px_X3872);
   fChain->SetBranchAddress("cs_X_JPiPi_py_X3872", &cs_X_JPiPi_py_X3872, &b_cs_X_JPiPi_py_X3872);
   fChain->SetBranchAddress("cs_X_JPiPi_pz_X3872", &cs_X_JPiPi_pz_X3872, &b_cs_X_JPiPi_pz_X3872);
   fChain->SetBranchAddress("cs_X_JPiPi_massErr_X3872", &cs_X_JPiPi_massErr_X3872, &b_cs_X_JPiPi_massErr_X3872);
   Notify();
}

Bool_t DataP::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void DataP::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t DataP::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef DataP_cxx
