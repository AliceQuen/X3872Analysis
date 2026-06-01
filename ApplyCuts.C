#include "X3872Utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include "TChain.h"

void ApplyCuts()
{
    std::string cutFile = "best_cut.o";
    std::ifstream infile(cutFile);
    if (!infile.is_open()) {
        std::cerr << "[ERROR] Cannot open file: " << cutFile << std::endl;
        return;
    }

    std::map<std::string, std::string> cutMap;
    std::vector<std::string> filenames;
    std::string line;
    int line_num = 0;

    while (std::getline(infile, line)) {
        line_num++;
        if (line.empty()) continue;

        size_t commaPos = line.find(',');
        if (commaPos == std::string::npos) {
            std::cerr << "[WARNING] Line " << line_num << ": invalid format, skipping" << std::endl;
            continue;
        }

        std::string filename = line.substr(0, commaPos);
        std::string cut = line.substr(commaPos + 1);

        cutMap[filename] = cut;
        filenames.push_back(filename);
    }
    infile.close();

    if (filenames.empty()) {
        std::cerr << "[ERROR] No cuts found in " << cutFile << std::endl;
        return;
    }

    std::cout << "\n========== Available Files ==========" << std::endl;
    for (size_t i = 0; i < filenames.size(); ++i) {
        std::cout << "[" << (i + 1) << "] " << filenames[i] << std::endl;
    }
    std::cout << "\nEnter the numbers of files to merge (separated by spaces, e.g., 1 2 3): ";

    std::string input;
    std::getline(std::cin, input);

    std::vector<int> selections;
    std::stringstream ss(input);
    int num;
    while (ss >> num) {
        if (num >= 1 && num <= static_cast<int>(filenames.size())) {
            selections.push_back(num - 1);
        } else {
            std::cerr << "[WARNING] Invalid selection: " << num << ", ignoring" << std::endl;
        }
    }

    if (selections.empty()) {
        std::cerr << "[ERROR] No valid selections made" << std::endl;
        return;
    }

    std::sort(selections.begin(), selections.end());
    selections.erase(std::unique(selections.begin(), selections.end()), selections.end());

    std::cout << "\nSelected files:" << std::endl;
    for (int idx : selections) {
        std::cout << "  - " << filenames[idx] << std::endl;
    }

    std::vector<std::string> tempFiles;

    for (int idx : selections) {
        std::string filename = filenames[idx];
        std::string cut = cutMap[filename];

        std::cout << "\nProcessing: " << filename << std::endl;
        std::cout << "Applying cut: " << cut << std::endl;

        std::string tempFileName = "temp_" + std::to_string(idx) + ".root";
        TTree* tree = GetTree<TTree>(filename.c_str(), cut, tempFileName.c_str());
        delete tree;
        tempFiles.push_back(tempFileName);
        std::cout << "  Saved to temp file: " << tempFileName << std::endl;
    }

    std::string outputFile = "merged_output.root";
    std::remove(outputFile.c_str());

    TChain* chain = new TChain("SourceTree");
    for (const auto& tempFile : tempFiles) {
        chain->Add(tempFile.c_str());
    }

    std::cout << "\nTotal entries in merged chain: " << chain->GetEntries() << std::endl;

    chain->Merge(outputFile.c_str());
    std::cout << "\nMerged tree saved to: " << outputFile << std::endl;

    for (const auto& tempFile : tempFiles) {
        std::remove(tempFile.c_str());
        std::cout << "Cleaned up temp file: " << tempFile << std::endl;
    }

    delete chain;
}
