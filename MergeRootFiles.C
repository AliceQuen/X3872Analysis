#include "X3872Utils.h"
#include <map>
#include <cctype>

void MergeRootFiles() {
    auto startTime = std::chrono::high_resolution_clock::now();
    std::string dataPath = "~/runs/JpsiX3872_Result/";
    
    std::cout << "Starting to merge root files from: " << dataPath << std::endl;
    std::vector<std::string> directories;
    FindDirectoriesWithRootFiles(dataPath, directories);
    ROOT::TProcessExecutor pool;
    auto mergeDir = [&](const std::string &dirPath) {
        TChain chain("mkcands/X_data");
        std::string mergedFiles = dirPath + "/*.root";
        chain.Add(mergedFiles.c_str());
        std::string mergedFilePath = dirPath ;
        chain.SetName("SourceTree");
        mergedFilePath += "/merged.root";
        chain.Merge(mergedFilePath.c_str());
        return true;
    };
    for (const auto &dirPath : directories) {
        std::string existingMergedFile = dirPath + "/merged.root";
        std::remove(existingMergedFile.c_str());
    }
    #if DEBUG == 1
    std::cout << "[INFO] Finishend searching files. Start merging "<< std::endl;
    #endif
    pool.Map(mergeDir, directories);
    #if DEBUG == 1
    std::cout << "[INFO] Finishend merging files "<< std::endl;
    #endif
    std::map<std::string, std::vector<std::string>> yearToDirectories;
    for (const auto &dirPath : directories) {

        std::string year = "unknown";
        for (size_t i = 0; i + 3 < dirPath.size(); i++) {
            if (dirPath[i] == '2' && dirPath[i+1] == '0' && 
                isdigit(dirPath[i+2]) && isdigit(dirPath[i+3])) {
                year = dirPath.substr(i, 4);
                break;
            }
        }
        yearToDirectories[year].push_back(dirPath);
    }
    auto mergeYear = [&](std::pair<std::string, std::vector<std::string>> entry) {
        auto &year = entry.first;
        auto &dirs = entry.second;
        TChain chain("SourceTree");
        for (const auto &dirPath : dirs) {
            std::string existingMergedFile = dirPath + "/merged.root";
            chain.Add(existingMergedFile.c_str());
        }
        std::string outputFile = year + "_merged.root";
        std::remove(outputFile.c_str());
        chain.SetName("SourceTree");
        chain.Merge(outputFile.c_str());
        std::cout << "Merged " << dirs.size() << " directories for year " << year << " into " << outputFile << std::endl;
        return true;
    };
    std::vector<std::pair<std::string, std::vector<std::string>>> yearDirVector;
    for (auto &entry : yearToDirectories) {
        yearDirVector.push_back(entry);
    }
    #if DEBUG == 1
    std::cout << "[INFO] Starting parallel merging by year" << std::endl;
    #endif
    pool.Map(mergeYear, yearDirVector);
    #if DEBUG == 1
    std::cout << "[INFO] Finished parallel merging by year" << std::endl;
    #endif
    auto cleanupMerged = [&](const std::string &dirPath) {
        std::string existingMergedFile = dirPath + "/merged.root";
        std::remove(existingMergedFile.c_str());
        return true;
    };
    #if DEBUG == 1
    std::cout << "[INFO] Starting parallel cleanup of intermediate merged files" << std::endl;
    #endif
    pool.Map(cleanupMerged, directories);
    #if DEBUG == 1
    std::cout << "[INFO] Finished parallel cleanup" << std::endl;
    #endif
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
    std::cout << "total time taken: " << duration.count() << " s" << std::endl;
}
