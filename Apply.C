// Codes for Application
#include "X3872Utils.h"
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <chrono>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TPRegexp.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TChain.h"
#include "TStopwatch.h"

#include "TMVA/Tools.h"
#include "TMVA/Reader.h"

#define VERBOSE false

void Apply(TString datafile, TString histinfo, TString weightfile)
{

	TMVA::Tools::Instance();
	TMVA::Reader *reader = new TMVA::Reader("!Color:!Silent:!V");
	TChain *SourceTree = GetTree<TChain>(datafile, "");

	TString weightsfile = weightfile;

	TString filename = datafile;
	auto filenames = filename.Tokenize("/");
	if (!filenames->Last())  throw std::runtime_error("Error: Invalid data file name format");
	filename = ((TObjString *)(filenames->Last()))->GetString();
	auto job_names = weightsfile.Tokenize("_");
	auto nj = job_names->GetEntries();
	std::cout << nj << "\n";
	if (nj < 3) throw std::runtime_error("Error: Invalid weights file name format");
	TString id = ((TObjString *)(job_names->At(nj - 2)))->GetString();
	TString job_name;
	for (unsigned int m = 0; m < nj - 2; m++)
	{
		job_name = job_name + ((TObjString *)(job_names->At(m)))->GetString();
		if (m != nj - 3) job_name = job_name + "_";
	}

	TString outfile = "/home/storage0/users/junkaiqin/TMVA-Cut-Optimization/modeltest/applied/" + job_name + "/data_" + id + "/" + filename;
	TFile *results = TFile::Open(outfile, "RECREATE");
	if (!results || !results->IsOpen()) throw std::runtime_error("Error: Unable to open output file");
	if (SourceTree->GetEntries() < 0 || !SourceTree) throw std::runtime_error("Error: Unable to open SourceTree");

	std::vector<std::string> excludedVars = {"Psi2S_mass"};
	std::vector<std::string> variables;

	std::ifstream ifs(histinfo);
	if (!ifs.is_open()) throw std::runtime_error("Error: Unable to open variables file: " + std::string(histinfo.Data()));

	variables = ReadVariableList(histinfo.Data(), excludedVars);
	std::cout << std::endl << "[INFO] Reading variables from: " << histinfo << std::endl;
	std::cout << "[INFO] Contents of " << histinfo << ":" << std::endl;
	std::cout << "----------------------------------------" << std::endl;
	for (const auto &var : variables) {
		std::cout << "  " << var << std::endl;
	}
	std::cout << "----------------------------------------" << std::endl;
	std::cout << "[INFO] Total variables read: " << variables.size() << std::endl;

	float MVAMark = 0;

	std::map<std::string, float*> var_map;
	for (const auto &var : variables) {
		var_map[var] = new float;
		reader->AddVariable(var, var_map[var]);
		SourceTree->SetBranchAddress(var.c_str(), var_map[var]);
	}


	reader->BookMVA("MVA", weightsfile);

	bool disableMT = false;
	TTree *merged_tree = nullptr;
	if (disableMT) {
		TTree *input_tree = SourceTree;
		merged_tree = input_tree->CloneTree(0);
		merged_tree->Branch("TMVA_Mark", &MVAMark);

		for (Long64_t i = 0; i < input_tree->GetEntries(); i++) {
			input_tree->GetEntry(i);
			MVAMark = reader->EvaluateMVA("MVA");
			merged_tree->Fill();
		}
	} else {
		int nthreads = std::thread::hardware_concurrency();
		ROOT::EnableImplicitMT(nthreads);
		ROOT::TTreeProcessorMT tp(*SourceTree);

		ROOT::TThreadedObject<std::shared_ptr<TTree>> local_trees;

		auto apply_mva = [&local_trees, &variables, &weightsfile, &MVAMark](TTreeReader &r)
		{
			TTree *input_tree = r.GetTree();
			auto local_tree = local_trees.Get();
			*local_tree = std::shared_ptr<TTree>(input_tree->CloneTree(0));
			(*local_tree)->Branch("TMVA_Mark", &MVAMark);

			TMVA::Reader *local_reader = new TMVA::Reader("!Color:!Silent:!V");
			std::map<std::string, float*> local_var_map;
			for (const auto &var : variables) {
				local_var_map[var] = new float;
				local_reader->AddVariable(var, local_var_map[var]);
				input_tree->SetBranchAddress(var.c_str(), local_var_map[var]);
			}
			local_reader->BookMVA("MVA", weightsfile);

			while (r.Next()) {
				MVAMark = local_reader->EvaluateMVA("MVA");
				(*local_tree)->Fill();
			}

			for (auto &p : local_var_map) {
				delete p.second;
			}
			delete local_reader;
		};

		tp.Process(apply_mva);
		float MVAMark = 0;

		merged_tree = SourceTree->CloneTree(0);
		merged_tree->Branch("TMVA_Mark", &MVAMark);

		for (unsigned int i = 0; i < local_trees.GetNSlots(); i++) {
			auto *tree_ptr = local_trees.GetAtSlotRaw(i);
			if (tree_ptr && *tree_ptr) {
				TTree *local_tree = tree_ptr->get();
				if (local_tree && local_tree->GetEntries() > 0) {
					TBranch *br_mva_local = local_tree->GetBranch("TMVA_Mark");
					float mva_local;
					br_mva_local->SetAddress(&mva_local);

					for (Long64_t j = 0; j < local_tree->GetEntries(); j++) {
						local_tree->GetEntry(j);
						MVAMark = mva_local;
						merged_tree->Fill();
					}
				}
			}
		}
	}

	results->cd();
	merged_tree->Write();
	results->Close();
	delete reader;
	delete SourceTree;
	delete results;
}