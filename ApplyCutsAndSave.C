#include "X3872Utils.h"
#include <string>

void ApplyCutsAndSave(std::string inputPath, std::string cuts = "")
{
    if (cuts.empty())
    {
        std::cout << "Error: Please provide cuts as second argument!" << std::endl;
        std::cout << "Usage: ApplyCutsAndSave(\"input.root\", \"pt > 1.0, abs(eta) < 1.5\")" << std::endl;
        return;
    }
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::cout << "============================================" << std::endl;
    std::cout << "Applying cuts to: " << inputPath << std::endl;
    std::cout << "Cuts: " << cuts << std::endl;
    std::cout << "============================================" << std::endl;
    
    TChain *tree = GetTree<TChain>(inputPath.c_str(), cuts);
    
    size_t lastDot = inputPath.find_last_of('.');
    std::string outputPath;
    
    if (lastDot != std::string::npos && inputPath.substr(lastDot) == ".root") {
        outputPath = inputPath.substr(0, lastDot) + "_cut.root";
    } else {
        outputPath = inputPath + "_cut.root";
    }
    
    std::cout << "Writing output to: " << outputPath << std::endl;
    
    TFile *outFile = TFile::Open(outputPath.c_str(), "RECREATE");
    tree->SetName("SourceTree");
    tree->Write();
    outFile->Close();
    
    delete tree;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
    
    std::cout << "============================================" << std::endl;
    std::cout << "Finished!" << std::endl;
    std::cout << "Output file: " << outputPath << std::endl;
    std::cout << "Total time taken: " << duration.count() << " s" << std::endl;
    std::cout << "============================================" << std::endl;
}