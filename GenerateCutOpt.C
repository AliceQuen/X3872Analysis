#include "X3872Utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

bool AskUserAndHandleVariables(std::vector<std::string>& variables, const std::string& varListFile, TTree* SourceTree, const std::vector<std::string>& excludedVars)
{
    char userChoice = 'n';
    std::cout << std::endl << "[QUESTION] Do you want to use the variables from this file? (y/n): ";
    std::cin >> userChoice;

    if (userChoice != 'y' && userChoice != 'Y') {
        std::cout << std::endl << "[INFO] User chose not to use existing variable list" << std::endl;
        std::cout << "Please select an option:" << std::endl;
        std::cout << "  1. Regenerate variable list from SourceTree (overwrites existing varlist.txt)" << std::endl;
        std::cout << "  2. Manually edit the existing varlist.txt file and continue" << std::endl;

        int option = 0;
        std::cout << std::endl << "Enter your choice (1 or 2): ";
        std::cin >> option;

        if (option == 1) {
            std::cout << std::endl << "[INFO] Regenerating variable list from SourceTree..." << std::endl;
            variables = GetVariableListFromTree(SourceTree, excludedVars);
            std::cout << "[INFO] Found " << variables.size() << " variables:" << std::endl;
            for (const auto &var : variables) {
                std::cout << "  - " << var << std::endl;
            }

            std::ofstream outVarList(varListFile);
            if (!outVarList.is_open()) {
                std::cerr << "[ERROR] Cannot write to varlist.txt" << std::endl;
                return false;
            }
            for (const auto &var : variables) {
                outVarList << var << std::endl;
            }
            outVarList.close();
            std::cout << "[INFO] Successfully regenerated varlist.txt with " << variables.size() << " variables" << std::endl;
        }
        else if (option == 2) {
            std::cout << std::endl << "[INFO] Please manually edit varlist.txt and run again" << std::endl;
            std::cout << "[INFO] Current variables will be used for condor script generation" << std::endl;
            if (variables.empty()) {
                std::cerr << "[ERROR] No variables in varlist.txt after manual editing" << std::endl;
                return false;
            }
        }
        else {
            std::cerr << "[ERROR] Invalid option. Please enter 1 or 2" << std::endl;
            return false;
        }
    }
    else {
        std::cout << "[INFO] Using existing variable list from varlist.txt" << std::endl;
    }
    return true;
}

void GenerateCutOpt(TString dataPath, TString outputScript = "condor_cutopt.sub")
{
    // excluded variables
    // exclude Psi2S_mass
    const std::vector<std::string> excludedVars = {"Psi2S_mass", "X_.*", "Jpsi2_.*", ".*charge"};
    // prefix for condor submission script, that is the path to the analysis directory
    std::string prefix = "/home/storage29/users/junkaiqin/X3872Analysis";
    // to edit the condor script content, see "condor submission script content" in this file

    std::cout << "==============================================" << std::endl;
    std::cout << "GenerateCutOpt: Generating cut optimization jobs" << std::endl;
    std::cout << "Data path: " << dataPath << std::endl;
    std::cout << "Output script: " << outputScript << std::endl;
    std::cout << "==============================================" << std::endl << std::endl;

    TTree *SourceTree = nullptr;
    try {
        SourceTree = GetTree<TTree>(dataPath);
    } catch (const std::exception &e) {
        std::cerr << "[ERROR] Failed to get TTree: " << e.what() << std::endl;
        return;
    }

    if (!SourceTree) {
        std::cerr << "[ERROR] SourceTree is null" << std::endl;
        return;
    }

    unsigned int entries = SourceTree->GetEntries();
    if (entries == 0) {
        std::cerr << "[ERROR] No entries found in SourceTree" << std::endl;
        delete SourceTree;
        return;
    }

    std::cout << "[INFO] Successfully loaded SourceTree" << std::endl;
    std::cout << "[INFO] Total entries: " << entries << std::endl;

    std::string varListFile = "varlist.txt";
    std::vector<std::string> variables;
    bool varListExists = false;

    std::ifstream inFile(varListFile);
    if (inFile.good()) {
        varListExists = true;
        std::cout << std::endl << "[INFO] Found existing variable list file: " << varListFile << std::endl;
        std::cout << "[INFO] Contents of " << varListFile << ":" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::string line;
        while (std::getline(inFile, line)) {
            if (!line.empty()) {
                variables.push_back(line);
                std::cout << "  " << line << std::endl;
            }
        }
        inFile.close();
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "[INFO] Total variables read: " << variables.size() << std::endl;

        if (!AskUserAndHandleVariables(variables, varListFile, SourceTree, excludedVars)) {
            delete SourceTree;
            return;
        }
    }
    else {
        std::cout << std::endl << "[INFO] varlist.txt not found, generating from SourceTree..." << std::endl;
        variables = GetVariableListFromTree(SourceTree, excludedVars);
        std::cout << "[INFO] Found " << variables.size() << " variables:" << std::endl;
        for (const auto &var : variables) {
            std::cout << "  - " << var << std::endl;
        }

        std::ofstream outVarList(varListFile);
        if (!outVarList.is_open()) {
            std::cerr << "[ERROR] Cannot create varlist.txt" << std::endl;
            delete SourceTree;
            return;
        }
        for (const auto &var : variables) {
            outVarList << var << std::endl;
        }
        outVarList.close();
        std::cout << "[INFO] Successfully generated varlist.txt with " << variables.size() << " variables in SourceTree" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "[INFO] Total variables: " << variables.size() << std::endl;

        if (!AskUserAndHandleVariables(variables, varListFile, SourceTree, excludedVars)) {
            delete SourceTree;
            return;
        }
    }

    if (variables.empty()) {
        std::cerr << "[ERROR] No variables to process (only Psi2S_mass found or file empty)" << std::endl;
        delete SourceTree;
        return;
    }

    std::cout << std::endl << "[INFO] Generating condor submission script..." << std::endl;

    std::vector<std::string> jobs = {"roots/2016_merged.root", "roots/2017_merged.root", "roots/2018_merged.root"};


    std::ofstream outFile;
    outFile.open(outputScript.Data());
    if (!outFile.is_open()) {
        std::cerr << "[ERROR] Cannot open output file: " << outputScript << std::endl;
        delete SourceTree;
        return;
    }
    // condor submission script content
    outFile << "executable          =" << prefix << "/cutopt.sh" << std::endl;
    outFile << "output              = " << prefix << "/output/cutopt/$(ClusterId)_$(Process).out" << std::endl;
    outFile << "error               = " << prefix << "/error/cutopt/$(ClusterId)_$(Process).err" << std::endl;
    outFile << "log                 = " << prefix << "/cutopt.log" << std::endl;
    outFile << "request_cpus        = 48" << std::endl;
    outFile << "request_memory      = 1024 MB" << std::endl;
    outFile << "request_disk        = 256 MB" << std::endl;
    outFile << "rank                = TotalCpus" << std::endl;
    outFile << "queue arguments from (" << std::endl;

    for (const auto &job : jobs) {
        for (const auto &var : variables) {
            outFile << "\t" << prefix << "/" << job << " " << var << std::endl;
        }
    }
    outFile << ")" << std::endl;
    outFile.close();

    std::cout << "[INFO] Successfully generated condor submission script" << std::endl;
    std::cout << "[INFO] Output file: " << outputScript << std::endl;
    std::cout << "[INFO] Total jobs: " << jobs.size() * variables.size() << std::endl;
    std::cout << "[INFO] Number of jobs: " << jobs.size() << " years × " << variables.size() << " variables" << std::endl;

    std::ofstream resultFile;
    std::string resultPath = prefix + "/cut_optimization_results.txt";
    resultFile.open(resultPath.c_str(), std::ios::out | std::ios::trunc);
    if (!resultFile.is_open()) {
        std::cerr << "[WARNING] Cannot create result file: " << resultPath << std::endl;
    } else {
        resultFile.close();
        std::cout << "[INFO] Cleared result file: " << resultPath << std::endl;
    }

    delete SourceTree;

    std::cout << std::endl << "==============================================" << std::endl;
    std::cout << "Generation completed successfully!" << std::endl;
    std::cout << "==============================================" << std::endl;
}
