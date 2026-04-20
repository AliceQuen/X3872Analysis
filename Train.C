// Code for Training
#include "X3872Utils.h"
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Config.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"

void Train(TString sig_path, TString bkg_path, TString output_dir = "/home/storage0/users/junkaiqin/TMVA-Cut-Optimization/modeltest/", bool batch = false)
{
	TMVA::Tools::Instance();
	TMVA::Config::Instance().fIONames.fWeightFileDirPrefix = output_dir;

	TChain *SigTree = GetTree<TChain>(sig_path, "");
	TChain *BkgTree = GetTree<TChain>(bkg_path, "");

	std::cout << "sig " << SigTree->GetEntries() << " bkg " << BkgTree->GetEntries() << std::endl;

	std::string job_name = "cutopt";
	TString output_file = TString(output_dir) + "result/cutopt.root";
	TFile *OutputFile = TFile::Open(output_file, "RECREATE");
	TMVA::Factory *Factory = new TMVA::Factory(job_name.c_str(), OutputFile,
	"!V:!Silent:Color:DrawProgressBar:Transformations=I:AnalysisType=Classification");
	TMVA::DataLoader *DataLoader = new TMVA::DataLoader("dataset");

	std::vector<std::string> excludedVars = {"Psi2S_mass"};
	std::vector<std::string> variables;

	std::string varListFile = "varlist.txt";
	bool varListExists = false;

	std::ifstream inFile(varListFile);
	if (inFile.good()) {
		varListExists = true;
		std::cout << std::endl << "[INFO] Found existing variable list file: " << varListFile << std::endl;
		variables = ReadVariableList(varListFile, excludedVars);
		std::cout << "[INFO] Contents of " << varListFile << ":" << std::endl;
		std::cout << "----------------------------------------" << std::endl;
		for (const auto &var : variables) {
			std::cout << "  " << var << std::endl;
		}
		std::cout << "----------------------------------------" << std::endl;
		std::cout << "[INFO] Total variables read: " << variables.size() << std::endl;

		if (!batch) {
			char userChoice = 'n';
			std::cout << std::endl << "[QUESTION] Do you want to use the variables from this file? (y/n): ";
			std::cin >> userChoice;

			if (userChoice != 'y' && userChoice != 'Y') {
				std::cout << std::endl << "[INFO] User chose not to use existing variable list" << std::endl;
				std::cout << "Please select an option:" << std::endl;
				std::cout << "  1. Regenerate variable list from Tree (overwrites existing varlist.txt)" << std::endl;
				std::cout << "  2. Manually edit the existing varlist.txt file and continue" << std::endl;

				int option = 0;
				std::cout << std::endl << "Enter your choice (1 or 2): ";
				std::cin >> option;

				if (option == 1) {
					std::cout << std::endl << "[INFO] Regenerating variable list from Tree..." << std::endl;
					variables = GetVariableListFromTree(SigTree, excludedVars);
					std::cout << "[INFO] Found " << variables.size() << " variables:" << std::endl;
					for (const auto &var : variables) {
						std::cout << "  - " << var << std::endl;
					}

					std::ofstream outVarList(varListFile);
					if (!outVarList.is_open()) {
						std::cerr << "[ERROR] Cannot write to varlist.txt" << std::endl;
						delete SigTree;
						delete BkgTree;
						delete Factory;
						delete DataLoader;
						delete OutputFile;
						return;
					}
					for (const auto &var : variables) {
						outVarList << var << std::endl;
					}
					outVarList.close();
					std::cout << "[INFO] Successfully regenerated varlist.txt with " << variables.size() << " variables" << std::endl;
				}
				else if (option == 2) {
					std::cout << std::endl << "[INFO] Please manually edit varlist.txt and run again" << std::endl;
					std::cout << "[INFO] Current variables will be used for training" << std::endl;
					if (variables.empty()) {
						std::cerr << "[ERROR] No variables in varlist.txt after manual editing" << std::endl;
						delete SigTree;
						delete BkgTree;
						delete Factory;
						delete DataLoader;
						delete OutputFile;
						return;
					}
				}
				else {
					std::cerr << "[ERROR] Invalid option. Please enter 1 or 2" << std::endl;
					delete SigTree;
					delete BkgTree;
					delete Factory;
					delete DataLoader;
					delete OutputFile;
					return;
				}
			}
			else {
				std::cout << "[INFO] Using existing variable list from varlist.txt" << std::endl;
			}
		}
	}
	else {
		std::cout << std::endl << "[INFO] varlist.txt not found, generating from Tree..." << std::endl;
		variables = GetVariableListFromTree(SigTree, excludedVars);
		std::cout << "[INFO] Found " << variables.size() << " variables:" << std::endl;
		for (const auto &var : variables) {
			std::cout << "  - " << var << std::endl;
		}

		std::ofstream outVarList(varListFile);
		if (!outVarList.is_open()) {
			std::cerr << "[ERROR] Cannot create varlist.txt" << std::endl;
			delete SigTree;
			delete BkgTree;
			delete Factory;
			delete DataLoader;
			delete OutputFile;
			return;
		}
		for (const auto &var : variables) {
			outVarList << var << std::endl;
		}
		outVarList.close();
		std::cout << "[INFO] Successfully generated varlist.txt with " << variables.size() << " variables" << std::endl;
	}

	if (variables.empty()) {
		std::cerr << "[ERROR] No variables to process (only Psi2S_mass found or file empty)" << std::endl;
		delete SigTree;
		delete BkgTree;
		delete Factory;
		delete DataLoader;
		delete OutputFile;
		return;
	}

	for (const auto &var : variables) {
		DataLoader->AddVariable(var, 'D');
	}

	DataLoader->AddSpectator("TMVA_Mark", 'F');
	DataLoader->AddSignalTree(SigTree, 1);
	DataLoader->AddBackgroundTree(BkgTree, 1);
	DataLoader->PrepareTrainingAndTestTree("", "", "nTrain_Signal=100000:nTest_Signal=1000:nTrain_Background=100000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V");

    TString layoutString ("Layout=TANH|128,TANH|128,TANH|128,LINEAR");
    TString trainingStrategyString = ("TrainingStrategy=LearningRate=1e-3,Momentum=0.9,"
                                        "ConvergenceSteps=40,BatchSize=200,TestRepetitions=1,"
                                        "WeightDecay=1e-4,Regularization=None,"
                                        "DropConfig=0.0+0.5+0.5+0.5");

    TString dnnOptions ("!H:V:ErrorStrategy=CROSSENTROPY:VarTransform=N:"
                          "WeightInitialization=XAVIERUNIFORM");
    dnnOptions.Append (":"); dnnOptions.Append (layoutString);
    dnnOptions.Append (":"); dnnOptions.Append (trainingStrategyString);
	TString cpuOptions = dnnOptions + ":Architecture=CPU:IgnoreNegWeightsInTraining=True:VarTransform=N";
	Factory->BookMethod(DataLoader, TMVA::Types::kDL, "DNN", cpuOptions);

	Factory->TrainAllMethods();
	Factory->TestAllMethods();
	Factory->EvaluateAllMethods();
	OutputFile->Close();

	if (!gROOT->IsBatch()) {
		TMVA::TMVAGui(output_file);
	}

	delete SigTree;
	delete BkgTree;
	delete Factory;
	delete DataLoader;
	delete OutputFile;

	// After training, type TMVA::TMVAGui("TMVACutOpt.root") in root cmd line to see the results without training.
}
