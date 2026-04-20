#ifndef X3872UTILS_H
#define X3872UTILS_H

#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <functional>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <cmath>
#include <algorithm>

#include "TString.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TGraph.h"
#include "TF1.h"
#include <ROOT/TProcessExecutor.hxx>
#include <ROOT/TThreadedObject.hxx>
#include <ROOT/TTreeProcessorMT.hxx>
#include "TH1F.h"
#include "TH2F.h"
#include "TROOT.h"

#define PEAK_UP 3.69
#define PEAK_DOWN 3.68
#define SIDE_BAND_UP_L 3.67
#define SIDE_BAND_DOWN_L 3.65
#define SIDE_BAND_UP_R 3.728
#define SIDE_BAND_DOWN_R 3.71
#define X3872_UP 3.88
#define X3872_DOWN 3.869

#define MAX_FILES_THRESHOLD 10000

#define DEBUG 1

struct Average
{
    double min;
    double max;
    double mean;
    double sigma;
    unsigned int n;
};

ROOT::TThreadedObjectUtils::MergeFunctionType<double> MergeDoubles = [](std::shared_ptr<double> target, std::vector<std::shared_ptr<double>> &objs)
{
    double sum = 0.0;
    for (auto &obj : objs)
    {
        if (obj && obj != target)
        {
            *target += *obj;
        }
    }
};

ROOT::TThreadedObjectUtils::MergeFunctionType<unsigned int> MergeInts = [](std::shared_ptr<unsigned int> target, std::vector<std::shared_ptr<unsigned int>> &objs)
{
    unsigned int sum = 0;
    for (auto &obj : objs)
    {
        if (obj && obj != target)
        {
            *target += *obj;
        }
    }
};

const char* PeakSelectionCut = "3.68 < Psi2s_mass < 3.69";
const char* SideBandSelectionCut = "(Psi2s_mass > 3.65 && Psi2s_mass < 3.67) || (Psi2s_mass > 3.71 && Psi2s_mass < 3.728)";
const char* X3872SelectionCut = "3.869 < Psi2s_mass < 3.88";

std::vector<std::string> split(const std::string &str, const char delimiter = ',')
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}
std::string ExpandPath(const std::string &path)
{
    if (path.empty())
        return path;

    if (path[0] == '~')
    {
        std::string home = getenv("HOME");
        if (home.empty())
            return path;
        return home + path.substr(1);
    }
    return path;
}

void FindDirectoriesWithRootFiles(const std::string &directory, std::vector<std::string> &directories)
{
    std::string expandedDir = ExpandPath(directory);
    DIR *dir = opendir(expandedDir.c_str());
    if (!dir)
    {
        std::cerr << "Warning: Cannot open directory: " << expandedDir << std::endl;
        return;
    }

    bool hasRootFiles = false;
    struct dirent *entry;

    while ((entry = readdir(dir)) != nullptr)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        if (entry->d_type != DT_DIR && name.size() >= 5 &&
            name.substr(name.size() - 5) == ".root")
        {
            hasRootFiles = true;
            break;
        }
    }
    rewinddir(dir);

    if (hasRootFiles)
    {
        #if DEBUG == 1
        std::cout << "[DEBUG] Found directory with ROOT files: " << expandedDir << std::endl;
        #endif
        directories.push_back(expandedDir);
    }

    while ((entry = readdir(dir)) != nullptr)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        std::string fullPath = expandedDir + "/" + name;
        if (entry->d_type == DT_DIR)
        {
            #if DEBUG == 1
            std::cout << "[DEBUG] Searching subdirectory: " << fullPath << std::endl;
            #endif
            FindDirectoriesWithRootFiles(fullPath, directories);
        }
    }
    closedir(dir);
}

struct CutCondition
{
    std::string variable;
    double value;
    bool is_greater; // true for 'var > value', false for 'var < value'
};

std::vector<CutCondition> ParseCuts(const std::string &cuts_string)
{
    std::vector<CutCondition> conditions;
    if (cuts_string.empty())
        return conditions;
    
    auto cut_tokens = split(cuts_string);
    
    for (const auto &cut_str : cut_tokens)
    {
        CutCondition cond;
        std::vector<std::string> parts;
        
        if (cut_str.find('<') != std::string::npos)
            parts = split(cut_str, '<');
        else if (cut_str.find('>') != std::string::npos)
            parts = split(cut_str, '>');
        else
            throw std::runtime_error("No comparison operator found in cut: " + cut_str);
        
        if (parts.size() == 2)
        {
            if (cut_str.find('<') != std::string::npos)
            {
                if (!parts[0].empty() && isdigit(parts[0][0]))
                {
                    cond.value = std::stod(parts[0]);
                    cond.variable = parts[1];
                    cond.is_greater = false;
                }
                else
                {
                    cond.value = std::stod(parts[1]);
                    cond.variable = parts[0];
                    cond.is_greater = false;
                }
            }
            else
            {
                if (!parts[0].empty() && isdigit(parts[0][0]))
                {
                    cond.value = std::stod(parts[0]);
                    cond.variable = parts[1];
                    cond.is_greater = true;
                }
                else
                {
                    cond.value = std::stod(parts[1]);
                    cond.variable = parts[0];
                    cond.is_greater = true;
                }
            }
            conditions.push_back(cond);
        }
        else if (parts.size() == 3)
        {
            if (!parts[0].empty() && isdigit(parts[0][0]) && !parts[2].empty() && isdigit(parts[2][0]))
            {
                double min_val = std::stod(parts[0]);
                double max_val = std::stod(parts[2]);
                std::string var = parts[1];
                conditions.push_back({var, min_val, true});
                conditions.push_back({var, max_val, false});
            }
            else
            {
                throw std::runtime_error("Invalid range format: " + cut_str);
            }
        }
        else
        {
            throw std::runtime_error("Invalid cut format: " + cut_str);
        }
    }
    
    return conditions;
}

bool PassCuts(TTreeReader &reader, const std::vector<CutCondition> &conditions)
{
    for (const auto &cond : conditions)
    {
        TTreeReaderValue<float> var(reader, cond.variable.c_str());
        if (cond.is_greater)
        {
            if (!(*var > cond.value))
                return false;
        }
        else
        {
            if (!(*var < cond.value))
                return false;
        }
    }
    return true;
}

ROOT::TTreeProcessorMT *GetTree(TChain &tree)
{
    int nthreads = std::thread::hardware_concurrency();
    std::cout << "a4 " << nthreads << std::endl;
    ROOT::EnableImplicitMT(nthreads);
    return new ROOT::TTreeProcessorMT(tree);
}

template <typename ResultType>
ResultType *GetTree(TString path, std::string cuts = "");

template <>
TChain *GetTree<TChain>(TString path, std::string cuts)
{
    TChain *SourceTree = new TChain("SourceTree", "");
    std::string pathStr = std::string(path.Data());
    
    struct stat pathStat;
    if (stat(pathStr.c_str(), &pathStat) != 0) {
        delete SourceTree;
        throw std::runtime_error("Cannot access path: " + pathStr);
    }
    
    if (S_ISREG(pathStat.st_mode)) {
        if (pathStr.size() >= 5 && pathStr.substr(pathStr.size() - 5) == ".root") {
            SourceTree->Add(path);
            std::cout << "[INFO] Added single ROOT file: " << pathStr << std::endl;
        } else {
            delete SourceTree;
            throw std::runtime_error("File is not a .root file: " + pathStr);
        }
    } else if (S_ISDIR(pathStat.st_mode)) {
        std::vector<std::string> directories;
        FindDirectoriesWithRootFiles(pathStr, directories);
        for (const auto &dir : directories) {
            std::string pattern = dir + "/*.root";
            int added = SourceTree->Add(pattern.c_str());
            if (added == 0) {
                std::cerr << "[WARNING] No files matched pattern: " << pattern << std::endl;
            }
        }
        std::cout << "[INFO] Searched " << directories.size() << " directories with ROOT files" << std::endl;
    } else {
        delete SourceTree;
        throw std::runtime_error("Path is neither a regular file nor a directory: " + pathStr);
    }
    
    int nFiles = SourceTree->GetNtrees();
    unsigned int entries = SourceTree->GetEntries();
    std::cout << "[INFO] Total files: " << nFiles << "  Total entries before cuts: " << entries << std::endl;
    
    if (nFiles > MAX_FILES_THRESHOLD) {
        std::cerr << "[WARNING] Number of files (" << nFiles << ") exceeds recommended threshold of 10,000. Performance may be affected." << std::endl;
        std::cerr << "[HINT] Consider merging files with MergeRootFiles.C first." << std::endl;
    }
    
    if (entries == 0) {
        delete SourceTree;
        throw std::runtime_error("No entries found in the specified path: " + pathStr + ". Check if the TTree name 'SourceTree' exists in the files.");
    }
    
    // Apply cuts if specified
    if (!cuts.empty()) {
        auto conditions = ParseCuts(cuts);
        if (!conditions.empty()) {
            int nthreads = std::thread::hardware_concurrency();
            ROOT::EnableImplicitMT(nthreads);
            ROOT::TTreeProcessorMT tp(*SourceTree);
            
            // Each thread processes its own portion, creates a local filtered tree
            ROOT::TThreadedObject<std::shared_ptr<TTree>> local_trees;
            
            auto filter_fun = [&local_trees, &conditions](TTreeReader &reader)
            {
                TTree *input_tree = reader.GetTree();
                auto local_tree = local_trees.Get();
                *local_tree = std::shared_ptr<TTree>(input_tree->CloneTree(0));
                
                while (reader.Next()) {
                    if (PassCuts(reader, conditions)) {
                        (*local_tree)->Fill();
                    }
                }
            };
            
            tp.Process(filter_fun);
            
            // Create final tree and merge all local trees
            TTree *final_tree = SourceTree->CloneTree(0);
            
            // Merge each thread's filtered tree into final tree
            for (unsigned int i = 0; i < local_trees.GetNSlots(); i++) {
                auto *tree_ptr = local_trees.GetAtSlotRaw(i);
                if (tree_ptr && *tree_ptr) {
                    TTree *local_tree = tree_ptr->get();
                    if (local_tree && local_tree->GetEntries() > 0) {
                        // Copy all entries from local tree to final tree
                        for (Long64_t j = 0; j < local_tree->GetEntries(); j++) {
                            local_tree->GetEntry(j);
                            final_tree->Fill();
                        }
                    }
                }
            }
            
            delete SourceTree;
            
            // Write merged tree to temp file and read back as TChain
            TFile *temp_file = TFile::Open("temp_filtered.root", "RECREATE");
            final_tree->Write("SourceTree");
            temp_file->Close();
            delete temp_file;
            
            TChain *filtered_tree = new TChain("SourceTree", "");
            filtered_tree->Add("temp_filtered.root");
            
            entries = filtered_tree->GetEntries();
            std::cout << "[INFO] After cuts: " << entries << " entries remaining" << std::endl;
            
            return filtered_tree;
        }
    }
    
    return SourceTree;
}
template <>
ROOT::TTreeProcessorMT *GetTree<ROOT::TTreeProcessorMT>(TString path, std::string cuts)
{
    return GetTree(*GetTree<TChain>(path, cuts));
}
Average Statics(ROOT::TTreeProcessorMT *tp, TString variable)
{
    ROOT::TThreadedObject<double> sum;
    ROOT::TThreadedObject<double> sum2;
    ROOT::TThreadedObject<double> min;
    ROOT::TThreadedObject<double> max;
    ROOT::TThreadedObject<unsigned int> n;

    auto calculateSum = [&sum, &sum2, &min, &max, &n, &variable](TTreeReader &reader)
    {
        TTreeReaderValue<float> var(reader, variable);
        unsigned int local_n = 0;
        double local_sum = 0.0;
        double local_sum2 = 0.0;
        double local_min = 0.0;
        double local_max = 0.0;
        while (reader.Next())
        {
            local_sum += *var;
            local_sum2 += (*var) * (*var);
            local_n++;
            if (*var > local_max)
                local_max = *var;
            if (*var < local_min)
                local_min = *var;
        }
        *(sum.Get()).get() = local_sum;
        *(sum2.Get()).get() = local_sum2;
        *(max.Get()).get() = local_max;
        *(min.Get()).get() = local_min;
        *(n.Get()).get() = local_n;
    };
    tp->Process(calculateSum);
    unsigned int n_merge = *(n.Merge(MergeInts)).get();
    double mean = *(sum.Merge(MergeDoubles)).get() / n_merge;
    double sigma = sqrt(*(sum2.Merge(MergeDoubles)).get() / n_merge - mean * mean);
    double max_ = 0.0;
    double min_ = 0.0;
    for (unsigned int i = 0; i < max.GetNSlots(); i++)
    {
        double *i_max = max.GetAtSlotRaw(i);
        double *i_min = min.GetAtSlotRaw(i);
        if (i_max)
        {
            if (*i_max > max_)
                max_ = *i_max;
        }
        if (i_min)
        {
            if (*i_min < min_)
                min_ = *i_min;
        }
    }
    Average result = {min_, max_, mean, sigma, n_merge};
    return result;
}

std::pair<double, double> AutoRange(ROOT::TTreeProcessorMT *tp, TString variable)
{
    auto st = Statics(tp, variable);
    return std::make_pair(std::max(st.mean - 3 * st.sigma, st.min), std::min(st.mean + 3 * st.sigma, st.max));
}

std::shared_ptr<TH1F> DrawGraph(ROOT::TTreeProcessorMT *tp, TString variable, TString title, float min, float max, unsigned int bin = 100)
{
    auto start = std::chrono::high_resolution_clock::now();
    float max_x = max;
    float min_x = min;

    if (min == max)
    {
        auto range = AutoRange(tp, variable);
        min_x = range.first;
        max_x = range.second;
    }

    ROOT::TThreadedObject<TH1F> ptHist(variable, title, bin, min_x, max_x);
    auto fillHistogram = [&ptHist, &variable](TTreeReader &reader)
    {
        TTreeReaderValue<float> var(reader, variable);
        auto Hist = ptHist.Get();
        while (reader.Next())
        {
            Hist->Fill(*var);
        }
    };
    tp->Process(fillHistogram);
    auto Hist = ptHist.Merge();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken: " << elapsed.count() << " seconds" << std::endl;
    return Hist;
}

std::shared_ptr<TH1F> DrawGraph(TString path, TString variable, TString title, float min, float max, unsigned int bin = 100, std::string cuts = "")
{
    ROOT::TTreeProcessorMT *tp = GetTree<ROOT::TTreeProcessorMT>(path, cuts);
    auto result = DrawGraph(tp, variable, title, min, max, bin);
    delete tp;
    return result;
}

std::pair<std::shared_ptr<TH1F>, std::shared_ptr<TH1F>> DrawSideBand(ROOT::TTreeProcessorMT *tp, TString variable, TString title, float min, float max, unsigned int bin = 100)
{
    float min_x = min;
    float max_x = max;
    if (min == max)
    {
        auto range = AutoRange(tp, variable);
        min_x = range.first;
        max_x = range.second;
    }
    auto hist_side = DrawGraph(tp, variable, title, min_x, max_x, bin);
    auto hist_peak = DrawGraph(tp, variable, title, min_x, max_x, bin);
    double scale = (PEAK_UP - PEAK_DOWN) / ((SIDE_BAND_UP_L - SIDE_BAND_DOWN_L) + (SIDE_BAND_UP_R - SIDE_BAND_DOWN_R));
    hist_peak->Add(hist_peak.get(), hist_side.get(), 1, -scale);
    for (unsigned int i = 0; i <= bin; i++)
    {
        if (hist_peak->GetBinContent(i) < 0)
            hist_peak->SetBinContent(i, 0);
        if (hist_side->GetBinContent(i) < 0)
            hist_side->SetBinContent(i, 0);
    }
    hist_peak->Scale(1.0 / hist_peak->Integral());
    hist_side->Scale(1.0 / hist_side->Integral());
    return std::make_pair(hist_peak, hist_side);
}

std::pair<std::shared_ptr<TH1F>, std::shared_ptr<TH1F>> DrawSideBand(TString path, TString variable, TString title, float min, float max, std::string cuts = "", unsigned int bin = 100)
{
    std::string full_cuts = std::string(cuts) + (cuts.empty() ? "" : ", ") + "(" + std::string(PeakSelectionCut) + ") || (" + std::string(SideBandSelectionCut) + ")";
    ROOT::TTreeProcessorMT *tp = GetTree<ROOT::TTreeProcessorMT>(path, full_cuts);
    auto result = DrawSideBand(tp, variable, title, min, max, bin);
    delete tp;
    return result;
}

void DrawSideBandAsPdf(TString path, TString variable, TString title, float min, float max, std::string cuts = "", unsigned int bin = 100)
{
    auto hists = DrawSideBand(path, variable, title, min, max, cuts, bin);
    auto hist_peak = hists.first;
    auto hist_side = hists.second;
    float max_y = std::max(hist_peak->GetMaximum(), hist_side->GetMaximum());
    float min_y = std::min(hist_peak->GetMinimum(), hist_side->GetMinimum());
    hist_peak->GetYaxis()->SetRangeUser(min_y, max_y + 0.005);
    hist_side->GetYaxis()->SetRangeUser(min_y, max_y + 0.005);
    hist_peak->SetLineColor(kRed);
    hist_side->SetLineColor(kBlue);
    TCanvas *c1 = new TCanvas("c1", "Side Band", 800, 600);
    hist_peak->Draw("HIST");
    hist_side->Draw("HIST SAME");
    c1->SaveAs(variable + ".pdf");
}

void DrawAsPdf(TString path, TString variable, TString title, float min, float max, unsigned int bin = 100, std::string cuts = "")
{
    auto g = DrawGraph(path, variable, title, min, max, bin, cuts);
    TCanvas *c1 = new TCanvas("c1", variable, 800, 600);
    g->Draw("HIST");
    c1->SaveAs(variable + ".pdf");
}

std::shared_ptr<TH2F> DrawGraph2D(ROOT::TTreeProcessorMT *tp, TString x_variable, TString y_variable, TString title, 
                                  float x_min, float x_max, float y_min, float y_max, 
                                  unsigned int x_bin = 100, unsigned int y_bin = 100)
{
    std::cout << "a1" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    float max_x = x_max;
    float min_x = x_min;
    float max_y = y_max;
    float min_y = y_min;

    if (x_min == x_max)
    {
        auto range_x = AutoRange(tp, x_variable);
        min_x = range_x.first;
        max_x = range_x.second;
    }
    if (y_min == y_max)
    {
        auto range_y = AutoRange(tp, y_variable);
        min_y = range_y.first;
        max_y = range_y.second;
    }
    ROOT::TThreadedObject<TH2F> ptHist(x_variable + "_" + y_variable, title, x_bin, min_x, max_x, y_bin, min_y, max_y);
    std::cout << "a2" << std::endl;
    auto fillHistogram = [&ptHist, &x_variable, &y_variable](TTreeReader &reader)
    {
        std::cout << "fs" << std::endl;
        TTreeReaderValue<float> x_var(reader, x_variable);
        TTreeReaderValue<float> y_var(reader, y_variable);
        auto Hist = ptHist.Get();
        while (reader.Next())
        {
            Hist->Fill(*x_var, *y_var);
        }
        std::cout << "fill" << std::endl;
    };
    tp->Process(fillHistogram);
    std::cout << "a3" << std::endl;
    auto Hist = ptHist.Merge();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken: " << elapsed.count() << " seconds" << std::endl;
    return Hist;
}

std::shared_ptr<TH2F> DrawGraph2D(TString path, TString x_variable, TString y_variable, TString title, 
                                  float x_min, float x_max, float y_min, float y_max, 
                                  unsigned int x_bin = 100, unsigned int y_bin = 100, std::string cuts = "")
{
    ROOT::TTreeProcessorMT *tp = GetTree<ROOT::TTreeProcessorMT>(path, cuts);
    auto result = DrawGraph2D(tp, x_variable, y_variable, title, x_min, x_max, y_min, y_max, x_bin, y_bin);
    delete tp;
    return result;
}

std::pair<std::shared_ptr<TH2F>, std::shared_ptr<TH2F>> DrawSideBand2D(ROOT::TTreeProcessorMT *tp, 
                                                                       TString x_variable, TString y_variable, TString title, 
                                                                       float x_min, float x_max, float y_min, float y_max, 
                                                                       unsigned int x_bin = 100, unsigned int y_bin = 100)
{
    float max_x = x_max;
    float min_x = x_min;
    float max_y = y_max;
    float min_y = y_min;
    
    if (x_min == x_max)
    {
        auto range_x = AutoRange(tp, x_variable);
        min_x = range_x.first;
        max_x = range_x.second;
    }
    if (y_min == y_max)
    {
        auto range_y = AutoRange(tp, y_variable);
        min_y = range_y.first;
        max_y = range_y.second;
    }
    
    auto hist_side = DrawGraph2D(tp, x_variable, y_variable, title, min_x, max_x, min_y, max_y, x_bin, y_bin);
    auto hist_peak = DrawGraph2D(tp, x_variable, y_variable, title, min_x, max_x, min_y, max_y, x_bin, y_bin);
    double scale = (PEAK_UP - PEAK_DOWN) / ((SIDE_BAND_UP_L - SIDE_BAND_DOWN_L) + (SIDE_BAND_UP_R - SIDE_BAND_DOWN_R));
    hist_peak->Add(hist_peak.get(), hist_side.get(), 1, -scale);
    for (unsigned int i = 0; i <= x_bin; i++)
    {
        for (unsigned int j = 0; j <= y_bin; j++)
        {
            if (hist_peak->GetBinContent(i, j) < 0)
                hist_peak->SetBinContent(i, j, 0);
            if (hist_side->GetBinContent(i, j) < 0)
                hist_side->SetBinContent(i, j, 0);
        }
    }
    hist_peak->Scale(1.0 / hist_peak->Integral());
    hist_side->Scale(1.0 / hist_side->Integral());
    return std::make_pair(hist_peak, hist_side);
}

std::pair<std::shared_ptr<TH2F>, std::shared_ptr<TH2F>> DrawSideBand2D(TString path, 
                                                                       TString x_variable, TString y_variable, TString title, 
                                                                       float x_min, float x_max, float y_min, float y_max, 
                                                                       std::string cuts = "", 
                                                                       unsigned int x_bin = 100, unsigned int y_bin = 100)
{
    std::string full_cuts = std::string(cuts) + (cuts.empty() ? "" : ", ") + "(" + std::string(PeakSelectionCut) + ") || (" + std::string(SideBandSelectionCut) + ")";
    ROOT::TTreeProcessorMT *tp = GetTree<ROOT::TTreeProcessorMT>(path, full_cuts);
    auto result = DrawSideBand2D(tp, x_variable, y_variable, title, x_min, x_max, y_min, y_max, x_bin, y_bin);
    delete tp;
    return result;
}

void DrawSideBand2DAsPdf(TString path, TString x_variable, TString y_variable, TString title, 
                         float x_min, float x_max, float y_min, float y_max, 
                         std::string cuts = "", 
                         unsigned int x_bin = 100, unsigned int y_bin = 100)
{
    auto hists = DrawSideBand2D(path, x_variable, y_variable, title, x_min, x_max, y_min, y_max, cuts, x_bin, y_bin);
    auto hist_peak = hists.first;
    auto hist_side = hists.second;
    float max_z = std::max(hist_peak->GetMaximum(), hist_side->GetMaximum());
    float min_z = std::min(hist_peak->GetMinimum(), hist_side->GetMinimum());
    hist_peak->GetZaxis()->SetRangeUser(min_z, max_z + 0.005);
    hist_side->GetZaxis()->SetRangeUser(min_z, max_z + 0.005);
    TCanvas *c1 = new TCanvas("c1", "Side Band 2D", 800, 600);
    hist_peak->SetMarkerColor(kRed);
    hist_peak->Draw("COLZ");
    c1->SaveAs(x_variable + "_" + y_variable + "_peak.pdf");
    TCanvas *c2 = new TCanvas("c2", "Side Band 2D", 800, 600);
    hist_side->SetMarkerColor(kBlue);
    hist_side->Draw("COLZ");
    c2->SaveAs(x_variable + "_" + y_variable + "_side.pdf");
}

void Draw2DAsPdf(TString path, TString x_variable, TString y_variable, TString title, 
                 float x_min, float x_max, float y_min, float y_max, 
                 unsigned int x_bin = 100, unsigned int y_bin = 100, std::string cuts = "")
{
    auto g = DrawGraph2D(path, x_variable, y_variable, title, x_min, x_max, y_min, y_max, x_bin, y_bin, cuts);
    TCanvas *c1 = new TCanvas("c1", x_variable + "_" + y_variable, 800, 600);
    g->Draw("COLZ");
    c1->SaveAs(x_variable + "_" + y_variable + ".pdf");
}

inline double Median(std::vector<double> &nums)
{
    std::sort(nums.begin(), nums.end());
    auto size = nums.size();
    if (!size)
        throw std::runtime_error("Trying to find median for an empty vector");
    if (size % 2 != 0)
    {
        return nums.at(size / 2);
    }
    else
    {
        return (nums.at(size / 2 - 1) + nums.at(size / 2)) / 2.0;
    }
}

template <class get>
double MedianAbsoluteDiff(unsigned int n, unsigned int i, unsigned int k, get p)
{
    if (k % 2 == 0 || k >= n || k == 1)
        return p(i);

    int k1 = (k - 1) / 2;
    std::vector<double> m;
    for (unsigned int j = 0; j <= k1; j++)
    {
        m.push_back(p(i - j));
        if (!(i - j))
            break;
    }
    for (unsigned int j = 0; j <= k1; j++)
    {
        if (j != 0)
            m.push_back(p(i + j));
        if ((i + j) == n - 1)
            break;
    }
    double mid = Median(m);
    std::vector<double> ad;
    for (auto mi : m)
        ad.push_back(fabs(mi - mid));
    std::sort(ad.begin(), ad.end());
    double mad = Median(ad);
    double x = p(i);
    if (x < mid - 4 * mad || x > mid + 4 * mad)
        return mid;
    else
        return x;
}

template <class get>
double SavitzkyGolay(unsigned int n, unsigned int i, unsigned int k, get p)
{
    if (k % 2 == 0 || k >= n || k < 5)
        return p(i);
    int k1 = (k - 1) / 2;
    if (i < k1 || i > n - k1 - 1)
        return p(i);
    TGraph g;
    for (int j = 0; j < k; j++)
    {
        g.SetPoint(j, j - k1, p(i - k1 + j));
    }
    g.Fit("pol3", "Q");
    auto f = g.GetFunction("pol3");
    return f->Eval(0);
}

inline std::vector<std::string> ReadVariableList(const std::string &filename, const std::vector<std::string> &excludedVars)
{
    std::vector<std::string> variables;
    std::ifstream inFile(filename);
    if (!inFile.good()) {
        throw std::runtime_error("Cannot open variable list file: " + filename);
    }
    
    std::string line;
    while (std::getline(inFile, line)) {
        if (!line.empty()) {
            bool excluded = false;
            for (const auto &excludedVar : excludedVars) {
                if (line == excludedVar) {
                    excluded = true;
                    break;
                }
            }
            if (!excluded) {
                variables.push_back(line);
            }
        }
    }
    inFile.close();
    
    return variables;
}

inline std::vector<std::string> GetVariableListFromTree(TChain *tree, const std::vector<std::string> &excludedVars)
{
    std::vector<std::string> variables;
    TObjArray *branches = tree->GetListOfBranches();
    if (!branches) {
        throw std::runtime_error("Cannot get branch list from tree");
    }
    
    int nBranches = branches->GetEntries();
    for (int i = 0; i < nBranches; i++) {
        TBranch *branch = (TBranch*)branches->At(i);
        std::string branchName = branch->GetName();
        bool excluded = false;
        for (const auto &excludedVar : excludedVars) {
            if (branchName == excludedVar) {
                excluded = true;
                break;
            }
        }
        if (!excluded) {
            variables.push_back(branchName);
        }
    }
    
    return variables;
}

#endif
