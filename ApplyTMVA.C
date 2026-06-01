// Codes for Application
#include "X3872Utils.h"
#include <ROOT/TThreadExecutor.hxx>
#include <ROOT/TThreadedObject.hxx>
#include <ROOT/TTreeProcessorMT.hxx>
#include <RtypesCore.h>
#include <TCollection.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TROOT.h"
#include "TChain.h"

#include "TMVA/Tools.h"
#include "TMVA/Reader.h"

#define VERBOSE false

void ApplyTMVA()
{
	TString inputfile, varlist, weightfile;
	inputfile = "roots";
	varlist = "varlist.txt";
	weightfile = "output/dataset/weights/cutopt_DNN.weights.xml";
	TMVA::Tools::Instance();

	TFile output_file("output/DNN_result.root", "RECREATE");
	if (!output_file.IsOpen()) throw std::runtime_error("Error: Unable to open output file: " + std::string("output/DNN_result.root"));

	auto *tp = GetTree<ROOT::TTreeProcessorMT>(inputfile, "");

	TString weightsfile = weightfile;

	std::vector<std::string> excludedVars = {"Psi2S_mass"};
	std::vector<std::string> variables;

	std::ifstream ifs(varlist);
	if (!ifs.is_open()) throw std::runtime_error("Error: Unable to open variables file: " + std::string(varlist.Data()));

	variables = ReadVariableList(varlist.Data(), excludedVars);
	std::cout << std::endl << "[INFO] Reading variables from: " << varlist << std::endl;
	std::cout << "[INFO] Contents of " << varlist << ":" << std::endl;
	std::cout << "----------------------------------------" << std::endl;
	for (const auto &var : variables) {
		std::cout << "  " << var << std::endl;
	}
	std::cout << "----------------------------------------" << std::endl;
	std::cout << "[INFO] Total variables read: " << variables.size() << std::endl;

	ROOT::TThreadedObject<TTree> trees("SourceTree", "SourceTree");

	auto workItem = [&trees, &variables, &weightsfile](TTreeReader &reader) {
		auto target_tree = trees.Get();
		float *MVAMark = new float;
		float *Psi2S_mass = new float;
		float *Jpsi1_mass = new float;
		float *Jpsi2_mass = new float;
		float *X_PJ_mass = new float;
		
		*MVAMark = 0.0;
		*Jpsi1_mass = 0.0;
		*Jpsi2_mass = 0.0;
		*X_PJ_mass = 0.0;
		*Psi2S_mass = 0.0;
		target_tree->Branch("MVAMark", MVAMark, "MVAMark/F");
		target_tree->Branch("Jpsi1_mass", Jpsi1_mass, "Jpsi1_mass/F");
		target_tree->Branch("Jpsi2_mass", Jpsi2_mass, "Jpsi2_mass/F");
		target_tree->Branch("X_PJ_mass", X_PJ_mass, "X_PJ_mass/F");
		target_tree->Branch("Psi2S_mass", Psi2S_mass, "Psi2S_mass/F");

		auto TMVA_reader = new TMVA::Reader("!Color:!Silent:!V");

		auto local_tree = reader.GetTree();
		std::map<std::string, std::pair<float*, TTreeReaderValue<float>*>> var_map;
		TTreeReaderValue<float> Psi2S_mass_value(reader, "Psi2S_mass");
		TTreeReaderValue<float> Jpsi1_mass_value(reader, "Jpsi1_mass");
		TTreeReaderValue<float> Jpsi2_mass_value(reader, "Jpsi2_mass");
		TTreeReaderValue<float> X_PJ_mass_value(reader, "X_PJ_mass");

		for (const auto &var : variables) {
			var_map[var].first = new float;
			var_map[var].second = new TTreeReaderValue<float>(reader, var.c_str());
			TMVA_reader->AddVariable(var.c_str(), var_map[var].first);
		}
		local_tree->SetBranchAddress("Psi2S_mass", Psi2S_mass);
		
		TMVA_reader->BookMVA("MVA", weightsfile);

		while (reader.Next()) {
			*Psi2S_mass = *Psi2S_mass_value;
			*Jpsi1_mass = *Jpsi1_mass_value;
			*Jpsi2_mass = *Jpsi2_mass_value;
			*X_PJ_mass = *X_PJ_mass_value;

			for (const auto &var : variables) {
				*var_map[var].first = *(*var_map[var].second);
			}
			*MVAMark = TMVA_reader->EvaluateMVA("MVA");
			target_tree->Fill();
		}
		for (const auto &var : variables) {
			delete var_map[var].first;
			delete var_map[var].second;
		}
		delete MVAMark;
		delete Psi2S_mass;
		delete Jpsi1_mass;
		delete Jpsi2_mass;
		delete X_PJ_mass;
	};
	tp->Process(workItem);

	auto SourceTree = trees.Merge()->CloneTree();
	std::cout << "[INFO] Total entries in SourceTree: " << SourceTree->GetEntries() << std::endl;
	SourceTree->Write();
	output_file.Close();
	DrawAsPdf("output/DNN_result.root", "Psi2S_mass", "Psi2S_mass MVA > 0.6", 3.4, 4, "MVAMark > 0.6");
	DrawAsPdf("output/DNN_result.root", "Psi2S_mass", "Psi2S_mass MVA > 0.6", 3.85, 3.9, "MVAMark > 0.6", 100, "x3872");
	DrawAsPdf("output/DNN_result.root", "X_PJ_mass", "X_mass MVA > 0.6", 6.5, 9, "MVAMark > 0.6");

}