/*Once complete the modification, write file and quit vim*/
/*Maybe you still want to modify it*/
/*This is Data.C generated automatically*/
#define DataP_cxx
#include "DataP.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <cmath>
#include <set>
#include <vector>
#include <exception>
#include "TLorentzVector.h"
// Physics constants
#define MU_MASS 0.10565837
#define PI_MASS 0.139570
#define JPSI_MASS 3.096900
#define PSI2S_MASS 3.68610
using namespace std;
void DataP::Loop(TString outputname)
{
        //   In a ROOT session, you can do:
        //      root> .L Data.C
        //      root> Data t
        //      root> t.GetEntry(12); // Fill t data members with entry number 12
        //      root> t.Show();       // Show values of entry 12
        //      root> t.Show(16);     // Read and show values of entry 16
        //      root> t.Loop();       // Loop on all entries
        //

        //     This is the loop skeleton where:
        //    jentry is the global entry number in the chain
        //    ientry is the entry number in the current Tree
        //  Note that the argument to GetEntry must be:
        //    jentry for TChain::GetEntry
        //    ientry for TTree::GetEntry and TBranch::GetEntry
        //
        //       To read only selected branches, Insert statements like:
        // METHOD1:
        //    fChain->SetBranchStatus("*",0);  // disable all branches
        //    fChain->SetBranchStatus("branchname",1);  // activate branchname
        // METHOD2: replace line
        //    fChain->GetEntry(jentry);       //read all branches
        // by  b_branchname->GetEntry(ientry); //read only this branch
        if (fChain == 0) return;

        Long64_t nentries = fChain->GetEntriesFast();

        Long64_t nbytes = 0, nb = 0;
        std::vector<unsigned int> final_candidates;
        std::set<unsigned int> check_mu;

        // Prepare for the tree and variables
        TFile *Data_out = new TFile(outputname, "RECREATE");
        TTree *SourceTree = new TTree("SourceTree", "SourceTree");
        // Define branches
		std::vector<float*> vars;
        // BRANCH
		float TMVA_Mark;
		vars.push_back(&TMVA_Mark);
		SourceTree->Branch("TMVA_Mark", &TMVA_Mark);
		float Psi2S_mass;
		vars.push_back(&Psi2S_mass);
		SourceTree->Branch("Psi2S_mass", &Psi2S_mass);
		float Jpsi2_mass;
		vars.push_back(&Jpsi2_mass);
		SourceTree->Branch("Jpsi2_mass", &Jpsi2_mass);
		float X_mass_Fit;
		vars.push_back(&X_mass_Fit);
		SourceTree->Branch("X_mass_Fit", &X_mass_Fit);
		float X_mass_P4;
		vars.push_back(&X_mass_P4);
		SourceTree->Branch("X_mass_P4", &X_mass_P4);
		float X_VtxProb_;
		vars.push_back(&X_VtxProb_);
		SourceTree->Branch("X_VtxProb", &X_VtxProb_);
		float mu1_Pt;
		vars.push_back(&mu1_Pt);
		SourceTree->Branch("mu1_Pt", &mu1_Pt);
		float mu2_Pt;
		vars.push_back(&mu2_Pt);
		SourceTree->Branch("mu2_Pt", &mu2_Pt);
		float mu1_Pz;
		vars.push_back(&mu1_Pz);
		SourceTree->Branch("mu1_Pz", &mu1_Pz);
		float mu2_Pz;
		vars.push_back(&mu2_Pz);
		SourceTree->Branch("mu2_Pz", &mu2_Pz);
		float mu1_Eta;
		vars.push_back(&mu1_Eta);
		SourceTree->Branch("mu1_Eta", &mu1_Eta);
		float mu2_Eta;
		vars.push_back(&mu2_Eta);
		SourceTree->Branch("mu2_Eta", &mu2_Eta);
		float mu1_mu2_DeltaR;
		vars.push_back(&mu1_mu2_DeltaR);
		SourceTree->Branch("mu1_mu2_DeltaR", &mu1_mu2_DeltaR);
		float mu1_Iso;
		vars.push_back(&mu1_Iso);
		SourceTree->Branch("mu1_Iso", &mu1_Iso);
		float mu2_Iso;
		vars.push_back(&mu2_Iso);
		SourceTree->Branch("mu2_Iso", &mu2_Iso);
		float n_Loose_mu;
		vars.push_back(&n_Loose_mu);
		SourceTree->Branch("n_Loose_mu", &n_Loose_mu);
		float n_Tight_mu;
		vars.push_back(&n_Tight_mu);
		SourceTree->Branch("n_Tight_mu", &n_Tight_mu);
		float n_Soft_mu;
		vars.push_back(&n_Soft_mu);
		SourceTree->Branch("n_Soft_mu", &n_Soft_mu);
		float n_Medium_mu;
		vars.push_back(&n_Medium_mu);
		SourceTree->Branch("n_Medium_mu", &n_Medium_mu);
		float Pi1_Pt;
		vars.push_back(&Pi1_Pt);
		SourceTree->Branch("Pi1_Pt", &Pi1_Pt);
		float Pi2_Pt;
		vars.push_back(&Pi2_Pt);
		SourceTree->Branch("Pi2_Pt", &Pi2_Pt);
		float Pi1_Pz;
		vars.push_back(&Pi1_Pz);
		SourceTree->Branch("Pi1_Pz", &Pi1_Pz);
		float Pi2_Pz;
		vars.push_back(&Pi2_Pz);
		SourceTree->Branch("Pi2_Pz", &Pi2_Pz);
		float Pi1_Eta;
		vars.push_back(&Pi1_Eta);
		SourceTree->Branch("Pi1_Eta", &Pi1_Eta);
		float Pi2_Eta;
		vars.push_back(&Pi2_Eta);
		SourceTree->Branch("Pi2_Eta", &Pi2_Eta);
		float Pi1_mu1_DeltaR;
		vars.push_back(&Pi1_mu1_DeltaR);
		SourceTree->Branch("Pi1_mu1_DeltaR", &Pi1_mu1_DeltaR);
		float Pi2_mu1_DeltaR;
		vars.push_back(&Pi2_mu1_DeltaR);
		SourceTree->Branch("Pi2_mu1_DeltaR", &Pi2_mu1_DeltaR);
		float Pi1_mu2_DeltaR;
		vars.push_back(&Pi1_mu2_DeltaR);
		SourceTree->Branch("Pi1_mu2_DeltaR", &Pi1_mu2_DeltaR);
		float Pi2_mu2_DeltaR;
		vars.push_back(&Pi2_mu2_DeltaR);
		SourceTree->Branch("Pi2_mu2_DeltaR", &Pi2_mu2_DeltaR);
		float Pi_Pi_DeltaR;
		vars.push_back(&Pi_Pi_DeltaR);
		SourceTree->Branch("Pi_Pi_DeltaR", &Pi_Pi_DeltaR);
		float Jpsi1_Pt;
		vars.push_back(&Jpsi1_Pt);
		SourceTree->Branch("Jpsi1_Pt", &Jpsi1_Pt);
		float Jpsi1_Pz;
		vars.push_back(&Jpsi1_Pz);
		SourceTree->Branch("Jpsi1_Pz", &Jpsi1_Pz);
		float Jpsi1_Eta;
		vars.push_back(&Jpsi1_Eta);
		SourceTree->Branch("Jpsi1_Eta", &Jpsi1_Eta);
		float Jpsi1_massErr;
		vars.push_back(&Jpsi1_massErr);
		SourceTree->Branch("Jpsi1_massErr", &Jpsi1_massErr);
		float Jpsi1_VtxProb;
		vars.push_back(&Jpsi1_VtxProb);
		SourceTree->Branch("Jpsi1_VtxProb", &Jpsi1_VtxProb);
		float Jpsi1_mu1_DeltaR;
		vars.push_back(&Jpsi1_mu1_DeltaR);
		SourceTree->Branch("Jpsi1_mu1_DeltaR", &Jpsi1_mu1_DeltaR);
		float Jpsi1_mu2_DeltaR;
		vars.push_back(&Jpsi1_mu2_DeltaR);
		SourceTree->Branch("Jpsi1_mu2_DeltaR", &Jpsi1_mu2_DeltaR);
		float Jpsi1_Pi1_DeltaR;
		vars.push_back(&Jpsi1_Pi1_DeltaR);
		SourceTree->Branch("Jpsi1_Pi1_DeltaR", &Jpsi1_Pi1_DeltaR);
		float Jpsi1_Pi2_DeltaR;
		vars.push_back(&Jpsi1_Pi2_DeltaR);
		SourceTree->Branch("Jpsi1_Pi2_DeltaR", &Jpsi1_Pi2_DeltaR);
                float Psi2S_Pt;
                vars.push_back(&Psi2S_Pt);
                SourceTree->Branch("Psi2S_Pt", &Psi2S_Pt);
                float Psi2S_Pz;
                vars.push_back(&Psi2S_Pz);
                SourceTree->Branch("Psi2S_Pz", &Psi2S_Pz);
		float Psi2S_Eta;
		vars.push_back(&Psi2S_Eta);
		SourceTree->Branch("Psi2S_Eta", &Psi2S_Eta);
		float Psi2S_massErr;
		vars.push_back(&Psi2S_massErr);
		SourceTree->Branch("Psi2S_massErr", &Psi2S_massErr);
		float Psi2S_VtxProb;
		vars.push_back(&Psi2S_VtxProb);
		SourceTree->Branch("Psi2S_VtxProb", &Psi2S_VtxProb);
		float Psi2S_mu1_DeltaR;
		vars.push_back(&Psi2S_mu1_DeltaR);
		SourceTree->Branch("Psi2S_mu1_DeltaR", &Psi2S_mu1_DeltaR);
		float Psi2S_mu2_DeltaR;
		vars.push_back(&Psi2S_mu2_DeltaR);
		SourceTree->Branch("Psi2S_mu2_DeltaR", &Psi2S_mu2_DeltaR);
		float Psi2S_Pi1_DeltaR;
		vars.push_back(&Psi2S_Pi1_DeltaR);
		SourceTree->Branch("Psi2S_Pi1_DeltaR", &Psi2S_Pi1_DeltaR);
		float Psi2S_Pi2_DeltaR;
		vars.push_back(&Psi2S_Pi2_DeltaR);
		SourceTree->Branch("Psi2S_Pi2_DeltaR", &Psi2S_Pi2_DeltaR);
		float Psi2S_Jpsi1_DeltaR;
		vars.push_back(&Psi2S_Jpsi1_DeltaR);
		SourceTree->Branch("Psi2S_Jpsi1_DeltaR", &Psi2S_Jpsi1_DeltaR);
        //_BRANCH

        // Loop over entries
        for (Long64_t jentry = 0; jentry < nentries; jentry++)
        {
                Long64_t ientry = LoadTree(jentry);
                if (ientry < 0) break;
                nb = fChain->GetEntry(jentry);
                nbytes += nb;
                if (!MatchJpsiTrigNames->size()) continue;
                // Multi-candidate handling
                final_candidates.clear();
                unsigned int X_JPiPi_size = X_JPiPi_mass->size();
                for (unsigned int m = 0; m < X_JPiPi_size; m++) final_candidates.push_back(m);
                bool re_i = false;
                bool re_j = false;
                for (std::vector<unsigned int>::iterator i = final_candidates.begin(); i != final_candidates.end();)
                {
                        re_i = false;
                        for (std::vector<unsigned int>::iterator j = std::next(i); j != final_candidates.end();)
                        {
                                re_j = false;
                                check_mu.clear();
                                check_mu.insert((*X_mu1Id)[*i]);
                                check_mu.insert((*X_mu2Id)[*i]);
                                check_mu.insert((*X_mu3Id)[*i]);
                                check_mu.insert((*X_mu4Id)[*i]);

                                check_mu.insert((*X_mu1Id)[*j]);
                                check_mu.insert((*X_mu2Id)[*j]);
                                check_mu.insert((*X_mu3Id)[*j]);
                                check_mu.insert((*X_mu4Id)[*j]);

                                if (check_mu.size() == 4)
                                {
                                        if (!(((*X_mu1Id)[*i] == (*X_mu1Id)[*j]) && ((*X_mu2Id)[*i] == (*X_mu2Id)[*j]) && ((*X_mu3Id)[*i] == (*X_mu3Id)[*j]) && ((*X_mu4Id)[*i] == (*X_mu4Id)[*j])) &&
                                                !(((*X_mu1Id)[*i] == (*X_mu3Id)[*j]) && ((*X_mu2Id)[*i] == (*X_mu4Id)[*j]) && ((*X_mu3Id)[*i] == (*X_mu1Id)[*j]) && ((*X_mu4Id)[*i] == (*X_mu2Id)[*j])))
                                        {
                                                double Chi2_i = pow(((*X_Jpsi1_mass)[*i] - JPSI_MASS) / (*X_Jpsi1_massErr)[*i], 2) + pow(((*X_Jpsi2_mass)[*i] - JPSI_MASS) / (*X_Jpsi2_massErr)[*i], 2);
                                                double Chi2_j = pow(((*X_Jpsi1_mass)[*j] - JPSI_MASS) / (*X_Jpsi1_massErr)[*j], 2) + pow(((*X_Jpsi2_mass)[*j] - JPSI_MASS) / (*X_Jpsi2_massErr)[*j], 2);
                                                if (Chi2_i < Chi2_j)
                                                {
                                                        j = final_candidates.erase(j);
                                                        re_j = true;
                                                }
                                                else
                                                {
                                                        i = final_candidates.erase(i);
                                                        re_i = true;
                                                        break;
                                                }
                                        }
                                }
                                if (!re_j) j++;
                        }
                        if (!re_i) i++;
                }
                // if (Cut(ientry) < 0) continue;
                // Filter Macthing
                for (auto k = final_candidates.begin(); k != final_candidates.end(); k++)
                {
                        unsigned int l = *k;
                        unsigned int filter = 0;
                        if ((*muJpsiFilterRes)[(*X_mu1Id)[l]]) filter++;
                        if ((*muJpsiFilterRes)[(*X_mu2Id)[l]]) filter++;
                        if ((*muJpsiFilterRes)[(*X_mu3Id)[l]]) filter++;
                        if ((*muJpsiFilterRes)[(*X_mu4Id)[l]]) filter++;
                        if (filter < 2) continue;
                        // Fill Tree
                        // Prepare Lorentz Vectors
                        TLorentzVector mu1_P4, mu2_P4, mu3_P4, mu4_P4, Pi1_P4, Pi2_P4;
                        TLorentzVector Psi2S_P4, Jpsi1_P4, Jpsi2_P4, X_P4;
                        mu1_P4.SetXYZM((*muPx)[(*X_mu3Id)[l]], (*muPy)[(*X_mu1Id)[l]], (*muPz)[(*X_mu1Id)[l]], MU_MASS);
                        mu2_P4.SetXYZM((*muPx)[(*X_mu2Id)[l]], (*muPy)[(*X_mu2Id)[l]], (*muPz)[(*X_mu2Id)[l]], MU_MASS);
                        mu3_P4.SetXYZM((*muPx)[(*X_mu3Id)[l]], (*muPy)[(*X_mu3Id)[l]], (*muPz)[(*X_mu3Id)[l]], MU_MASS);
                        mu4_P4.SetXYZM((*muPx)[(*X_mu4Id)[l]], (*muPy)[(*X_mu4Id)[l]], (*muPz)[(*X_mu4Id)[l]], MU_MASS);
                        Pi1_P4.SetXYZM((*X_JPiPi_Pi1px)[l], (*X_JPiPi_Pi1py)[l], (*X_JPiPi_Pi1pz)[l], PI_MASS);
                		Pi2_P4.SetXYZM((*X_JPiPi_Pi2px)[l], (*X_JPiPi_Pi2py)[l], (*X_JPiPi_Pi2pz)[l], PI_MASS);
                        Psi2S_P4.SetXYZM((*X_JPiPi_px)[l], (*X_JPiPi_py)[l], (*X_JPiPi_pz)[l], (*X_JPiPi_mass)[l]);
                		Jpsi1_P4.SetXYZM((*X_Jpsi1_px)[l], (*X_Jpsi1_py)[l], (*X_Jpsi1_pz)[l], (*X_Jpsi1_mass)[l]);
                		Jpsi2_P4.SetXYZM((*X_Jpsi2_px)[l], (*X_Jpsi2_py)[l], (*X_Jpsi2_pz)[l], (*X_Jpsi2_mass)[l]);
                        // Pre-Cut
                        if (
                                std::isfinite((*X_JPiPi_VtxProb)[l]) && 
                                (*muIsPatSoftMuon)[(*X_mu1Id)[l]] + (*muIsPatSoftMuon)[(*X_mu2Id)[l]] + (*muIsPatSoftMuon)[(*X_mu3Id)[l]] + (*muIsPatSoftMuon)[(*X_mu4Id)[l]] >= 2 &&
                                (*muCharge)[(*X_mu1Id)[l]] + (*muCharge)[(*X_mu2Id)[l]] == 0 && (*muCharge)[(*X_mu3Id)[l]] + (*muCharge)[(*X_mu4Id)[l]]
                        ) continue;
                        // Physics Calculations
                        // PHYSICS
                        TMVA_Mark = 1;
                        Psi2S_mass = (Jpsi1_P4 + Pi1_P4 + Pi2_P4).M() - Jpsi1_P4.M() + JPSI_MASS;
                        Jpsi2_mass = Jpsi2_P4.M();
                        X_mass_Fit = (*X_mass)[l];
                        X_mass_P4 = (Psi2S_P4 + Jpsi2_P4).M() - Jpsi2_P4.M() + JPSI_MASS;
                        X_VtxProb_ = (*X_VtxProb)[l];
                        mu1_Pt = mu1_P4.Pt();
                        mu2_Pt = mu2_P4.Pt();
                        mu1_Pz = mu1_P4.Pz();
                        mu2_Pz = mu2_P4.Pz();
                        mu1_Eta = std::abs(mu1_P4.Eta());
                        mu2_Eta = std::abs(mu2_P4.Eta());
                        mu1_mu2_DeltaR = mu1_P4.DeltaR(mu2_P4);
                        mu1_Iso = (*muTrackIso)[(*X_mu1Id)[l]];
                        mu2_Iso = (*muTrackIso)[(*X_mu2Id)[l]];
                        n_Loose_mu = (*muIsPatLooseMuon)[(*X_mu1Id)[l]] + (*muIsPatLooseMuon)[(*X_mu2Id)[l]] + (*muIsPatLooseMuon)[(*X_mu3Id)[l]] + (*muIsPatLooseMuon)[(*X_mu4Id)[l]];
                        n_Tight_mu = (*muIsPatTightMuon)[(*X_mu1Id)[l]] + (*muIsPatTightMuon)[(*X_mu2Id)[l]] + (*muIsPatTightMuon)[(*X_mu3Id)[l]] + (*muIsPatTightMuon)[(*X_mu4Id)[l]];
                        n_Soft_mu = (*muIsPatSoftMuon)[(*X_mu1Id)[l]] + (*muIsPatSoftMuon)[(*X_mu2Id)[l]] + (*muIsPatSoftMuon)[(*X_mu3Id)[l]] + (*muIsPatSoftMuon)[(*X_mu4Id)[l]];
                        n_Medium_mu = (*muIsPatMediumMuon)[(*X_mu1Id)[l]] + (*muIsPatMediumMuon)[(*X_mu2Id)[l]] + (*muIsPatMediumMuon)[(*X_mu3Id)[l]] + (*muIsPatMediumMuon)[(*X_mu4Id)[l]];
                        Pi1_Pt = Pi1_P4.Pt();
                        Pi2_Pt = Pi2_P4.Pt();
                        Pi1_Pz = Pi1_P4.Pz();
                        Pi2_Pz = Pi2_P4.Pz();
                        Pi1_Eta = std::abs(Pi1_P4.Eta());
                        Pi2_Eta = std::abs(Pi2_P4.Eta());
                        Pi1_mu1_DeltaR = Pi1_P4.DeltaR(mu1_P4);
                        Pi2_mu1_DeltaR = Pi2_P4.DeltaR(mu1_P4);
                        Pi1_mu2_DeltaR = Pi1_P4.DeltaR(mu2_P4);
                        Pi2_mu2_DeltaR = Pi2_P4.DeltaR(mu2_P4);
                        Pi_Pi_DeltaR = Pi1_P4.DeltaR(Pi2_P4);
                        Jpsi1_Pt = Jpsi1_P4.Pt();
                        Jpsi1_Pz = Jpsi1_P4.Pz();
                        Jpsi1_Eta = std::abs(Jpsi1_P4.Eta());
                        Jpsi1_massErr = (*X_Jpsi1_massErr)[l];
                        Jpsi1_VtxProb = (*X_Jpsi1_VtxProb)[l];
                        Jpsi1_mu1_DeltaR = Jpsi1_P4.DeltaR(mu1_P4);
                        Jpsi1_mu2_DeltaR = Jpsi1_P4.DeltaR(mu2_P4);
                        Jpsi1_Pi1_DeltaR = Jpsi1_P4.DeltaR(Pi1_P4);
                        Jpsi1_Pi2_DeltaR = Jpsi1_P4.DeltaR(Pi2_P4);
                        Psi2S_Pt = Psi2S_P4.Pt();
                        Psi2S_Pz = Psi2S_P4.Pz();
                        Psi2S_Eta = std::abs(Psi2S_P4.Eta());
                        Psi2S_massErr = (*X_JPiPi_massErr)[l];
                        Psi2S_VtxProb = (*X_JPiPi_VtxProb)[l];
                        Psi2S_mu1_DeltaR = Psi2S_P4.DeltaR(mu1_P4);
                        Psi2S_mu2_DeltaR = Psi2S_P4.DeltaR(mu2_P4);
                        Psi2S_Pi1_DeltaR = Psi2S_P4.DeltaR(Pi1_P4);
                        Psi2S_Pi2_DeltaR = Psi2S_P4.DeltaR(Pi2_P4);
						Psi2S_Jpsi1_DeltaR = Psi2S_P4.DeltaR(Jpsi1_P4);
                        //_PHYSICS
                        bool finite = true;
                        for (auto k : vars) if (!std::isfinite(*k)) finite = false;
                        if (!finite) continue;
                        SourceTree->Fill();
                }
        }
        Data_out->Write();
}