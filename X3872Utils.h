#ifndef X3872UTILS_H
#define X3872UTILS_H

#include <TFile.h>
#include <cstddef>
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
#include <regex>

#include "TString.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TTreeReader.h"
#include "TLorentzVector.h"
#include "TTreeReaderValue.h"
#include "TGraph.h"
#include "TF1.h"
#include <ROOT/TProcessExecutor.hxx>
#include <ROOT/TThreadedObject.hxx>
#include <ROOT/TTreeProcessorMT.hxx>
#include <memory>
#include <utility>

// 计算多个 histogram 的 Y 轴范围，确保所有图都能完整显示
// margin: 边距比例，默认为5%
template<typename... Hists>
std::pair<double, double> GetYAxisRange(double margin, Hists*... hists) {
    std::vector<double> max_values;
    std::vector<double> min_values;
    
    // 使用折叠表达式遍历所有 histogram
    (void)std::initializer_list<int>{(max_values.push_back(hists->GetMaximum()), 
                                       min_values.push_back(hists->GetMinimum()), 0)...};
    
    if (max_values.empty() || min_values.empty()) {
        return {0.0, 1.0};
    }
    
    double max_y = *std::max_element(max_values.begin(), max_values.end());
    double min_y = *std::min_element(min_values.begin(), min_values.end());
    
    // 计算边距
    double range = max_y - min_y;
    double margin_amount = range * margin;
    
    return {min_y - margin_amount, max_y + margin_amount};
}

// 重载版本，默认边距为5%
template<typename... Hists>
std::pair<double, double> GetYAxisRange(Hists*... hists) {
    return GetYAxisRange(0.05, hists...);
}

// 计算并设置多个 histogram 的 Y 轴范围
template<typename... Hists>
void SetYAxisRangeForAll(double margin, Hists*... hists) {
    auto range = GetYAxisRange(margin, hists...);
    
    // 使用折叠表达式设置所有 histogram 的 Y 轴范围
    (void)std::initializer_list<int>{(hists->GetYaxis()->SetRangeUser(range.first, range.second), 0)...};
}

// 重载版本，默认边距为5%
template<typename... Hists>
void SetYAxisRangeForAll(Hists*... hists) {
    SetYAxisRangeForAll(0.05, hists...);
}

// 计算多个 2D histogram 的 Z 轴范围
template<typename... Hists>
std::pair<double, double> GetZAxisRange(double margin, Hists*... hists) {
    std::vector<double> max_values;
    std::vector<double> min_values;
    
    (void)std::initializer_list<int>{(max_values.push_back(hists->GetMaximum()),
                                       min_values.push_back(hists->GetMinimum()), 0)...};
    
    if (max_values.empty() || min_values.empty()) {
        return {0.0, 1.0};
    }
    
    double max_z = *std::max_element(max_values.begin(), max_values.end());
    double min_z = *std::min_element(min_values.begin(), min_values.end());
    
    double range = max_z - min_z;
    double margin_amount = range * margin;
    
    return {min_z - margin_amount, max_z + margin_amount};
}

// 重载版本，默认边距为5%
template<typename... Hists>
std::pair<double, double> GetZAxisRange(Hists*... hists) {
    return GetZAxisRange(0.05, hists...);
}

// 计算并设置多个 2D histogram 的 Z 轴范围
template<typename... Hists>
void SetZAxisRangeForAll(double margin, Hists*... hists) {
    auto range = GetZAxisRange(margin, hists...);
    
    (void)std::initializer_list<int>{(hists->GetZaxis()->SetRangeUser(range.first, range.second), 0)...};
}

// 重载版本，默认边距为5%
template<typename... Hists>
void SetZAxisRangeForAll(Hists*... hists) {
    SetZAxisRangeForAll(0.05, hists...);
}

#include "TH1F.h"
#include "TH2F.h"
#include "TROOT.h"

#define _STR(x) #x
#define TOSTRING(x) _STR(x)

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

struct CovarianceResult
{
    double mean_x;
    double mean_y;
    double sigma_x;
    double sigma_y;
    double covariance;
    double correlation;
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

#define PeakSelectionCut "(" TOSTRING(PEAK_DOWN) " < Psi2S_mass && Psi2S_mass < " TOSTRING(PEAK_UP) ")"
#define SideBandSelectionCut "((" TOSTRING(SIDE_BAND_DOWN_L) " < Psi2S_mass && Psi2S_mass < " TOSTRING(SIDE_BAND_UP_L) ") || (" TOSTRING(SIDE_BAND_DOWN_R) " < Psi2S_mass && Psi2S_mass < " TOSTRING(SIDE_BAND_UP_R) "))"
#define X3872SelectionCut "(" TOSTRING(X3872_DOWN) " < Psi2S_mass && Psi2S_mass < " TOSTRING(X3872_UP) ")"

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


ROOT::TTreeProcessorMT *GetTree(TTree &tree)
{
    int nthreads = std::thread::hardware_concurrency();
    ROOT::EnableImplicitMT(nthreads);
    return new ROOT::TTreeProcessorMT(tree);
}

template <typename ResultType>
ResultType *GetTree(TString path, TString cut = "", TString outfile = "");
        
template <>
TTree *GetTree<TTree>(TString path, TString cut, TString outfile)
{
    if (!outfile.IsNull()) {
        std::string outStr = std::string(outfile.Data());
        if (outStr.size() < 5 || outStr.substr(outStr.size() - 5) != ".root") {
            throw std::runtime_error("GetTree: output file must end with .root extension: " + outStr);
        }
    }
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
    if (!cut.IsNull()) {
        //auto start_time = std::chrono::high_resolution_clock::now();
        auto filtered_tree = SourceTree->CopyTree(cut.Data());
        if (filtered_tree == nullptr){
            delete SourceTree;
            throw std::runtime_error("No entries found after cuts: " + cut);
        }
        entries = filtered_tree->GetEntries();
        TString temp_outfile = "temp_filtered.root";
        if (!outfile.IsNull()){
            temp_outfile = outfile;
        }
        TFile file(temp_outfile, "recreate");
        filtered_tree->Write();
        file.Close();
        //auto end_time = std::chrono::high_resolution_clock::now();
        //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        //std::cerr << "[INFO] CopyTree completed in " << duration.count() << " ms" << std::endl;
        std::cerr << "[INFO] Total entries after cuts: " << filtered_tree->GetEntries() << std::endl;
        delete SourceTree;
        delete filtered_tree;
        return GetTree<TTree>(temp_outfile);
    }
    if (!outfile.IsNull()){
        SourceTree->Merge(outfile);
        delete SourceTree;
        return GetTree<TTree>(outfile);
    }
    return SourceTree;
}
template <>
ROOT::TTreeProcessorMT *GetTree<ROOT::TTreeProcessorMT>(TString path, TString cut, TString outfile)
{
    return GetTree(*GetTree<TTree>(path, cut, outfile));
}
Average Statistics(ROOT::TTreeProcessorMT *tp, TString variable)
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
            if (std::isnan(*var))
                continue;
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

CovarianceResult Statistics(ROOT::TTreeProcessorMT *tp, TString variable_x, TString variable_y)
{
    ROOT::TThreadedObject<double> sum_x;
    ROOT::TThreadedObject<double> sum_y;
    ROOT::TThreadedObject<double> sum_xy;
    ROOT::TThreadedObject<double> sum_x2;
    ROOT::TThreadedObject<double> sum_y2;
    ROOT::TThreadedObject<unsigned int> n;

    auto calculateStats = [&sum_x, &sum_y, &sum_xy, &sum_x2, &sum_y2, &n, &variable_x, &variable_y](TTreeReader &reader)
    {
        TTreeReaderValue<float> var_x(reader, variable_x);
        TTreeReaderValue<float> var_y(reader, variable_y);
        unsigned int local_n = 0;
        double local_sum_x = 0.0;
        double local_sum_y = 0.0;
        double local_sum_xy = 0.0;
        double local_sum_x2 = 0.0;
        double local_sum_y2 = 0.0;
        while (reader.Next())
        {
            if (std::isnan(*var_x) || std::isnan(*var_y))
                continue;
            local_sum_x += *var_x;
            local_sum_y += *var_y;
            local_sum_xy += *var_x * *var_y;
            local_sum_x2 += *var_x * *var_x;
            local_sum_y2 += *var_y * *var_y;
            local_n++;
        }
        *(sum_x.Get()).get() = local_sum_x;
        *(sum_y.Get()).get() = local_sum_y;
        *(sum_xy.Get()).get() = local_sum_xy;
        *(sum_x2.Get()).get() = local_sum_x2;
        *(sum_y2.Get()).get() = local_sum_y2;
        *(n.Get()).get() = local_n;
    };
    tp->Process(calculateStats);

    unsigned int n_merge = *(n.Merge(MergeInts)).get();
    double mean_x = *(sum_x.Merge(MergeDoubles)).get() / n_merge;
    double mean_y = *(sum_y.Merge(MergeDoubles)).get() / n_merge;
    double sigma_x = sqrt(*(sum_x2.Merge(MergeDoubles)).get() / n_merge - mean_x * mean_x);
    double sigma_y = sqrt(*(sum_y2.Merge(MergeDoubles)).get() / n_merge - mean_y * mean_y);
    double covariance = *(sum_xy.Merge(MergeDoubles)).get() / n_merge - mean_x * mean_y;
    double correlation = covariance / (sigma_x * sigma_y);

    CovarianceResult result = {mean_x, mean_y, sigma_x, sigma_y, covariance, correlation, n_merge};
    return result;
}

std::pair<double, double> AutoRange(ROOT::TTreeProcessorMT *tp, TString variable)
{
    auto st = Statistics(tp, variable);
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
    auto fillHistogram = [&ptHist, &variable, &min_x, &max_x](TTreeReader &reader)
    {
        TTreeReaderValue<float> var(reader, variable);
        auto Hist = ptHist.Get();
        while (reader.Next())
        {
            if (*var >= min_x && *var <= max_x){
                Hist->Fill(*var);
            }
        }
    };
    tp->Process(fillHistogram);
    auto Hist = ptHist.Merge();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken: " << elapsed.count() << " seconds" << std::endl;
    return Hist;
}

std::shared_ptr<TH1F> DrawGraph(TString path, TString variable, TString title, float min, float max, TString cut = "", unsigned int bin = 100)
{
    ROOT::TTreeProcessorMT *tp = GetTree<ROOT::TTreeProcessorMT>(path, cut);
    auto result = DrawGraph(tp, variable, title, min, max, bin);
    delete tp;
    if (!cut.IsNull()) {
        std::remove("temp_filtered.root");
    }
    return result;
}

std::pair<std::shared_ptr<TH1F>, std::shared_ptr<TH1F>> DrawSideBand(ROOT::TTreeProcessorMT *tp_peak, ROOT::TTreeProcessorMT *tp_side, TString variable, TString title, float min, float max, unsigned int bin = 100)
{
    double min_x = min;
    double max_x = max;
    if (min == max)
    {
        auto range_peak = AutoRange(tp_peak, variable);
        min_x = range_peak.first;
        max_x = range_peak.second;
        auto range_side = AutoRange(tp_side, variable);
        min_x = std::min(min_x, range_side.first);
        max_x = std::max(max_x, range_side.second);
    }
    auto hist_peak = DrawGraph(tp_peak, variable, title, min_x, max_x, bin);
    auto hist_side = DrawGraph(tp_side, variable, title, min_x, max_x, bin);
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

std::pair<std::shared_ptr<TH1F>, std::shared_ptr<TH1F>> DrawSideBand(TString path, TString variable, TString title, float min, float max, TString cut = "", unsigned int bin = 100)
{
    TString cut_side = cut + (cut.IsNull() ? "" : " && ") + "(" + TString(SideBandSelectionCut) + ")";
    TString cut_peak = cut + (cut.IsNull() ? "" : " && ") + "(" + TString(PeakSelectionCut) + ")";
    ROOT::TTreeProcessorMT *tp_peak = GetTree<ROOT::TTreeProcessorMT>(path, cut_peak,"peak.root");
    ROOT::TTreeProcessorMT *tp_side = GetTree<ROOT::TTreeProcessorMT>(path, cut_side,"side.root");
    auto result = DrawSideBand(tp_peak, tp_side, variable, title, min, max, bin);
    delete tp_peak;
    delete tp_side;
    std::remove("peak.root");
    std::remove("side.root");
    return result;
}

void DrawSideBandAsPdf(TString path, TString variable, TString title, float min, float max, TString cut = "", unsigned int bin = 100, TString outputPath = "")
{
    auto hists = DrawSideBand(path, variable, title, min, max, cut, bin);
    auto hist_peak = hists.first;
    auto hist_side = hists.second;
    // 使用新函数设置 Y 轴范围，确保所有图都能完整显示
    SetYAxisRangeForAll(hist_peak.get(), hist_side.get());
    hist_peak->SetLineColor(kRed);
    hist_side->SetLineColor(kBlue);
    TCanvas *c1 = new TCanvas("c1", "Side Band", 800, 600);
    hist_peak->Draw("HIST");
    hist_side->Draw("HIST SAME");
    TString savePath = outputPath.IsNull() ? variable + ".pdf" : outputPath + "/" + variable + ".pdf";
    c1->SaveAs(savePath);
}

void DrawAsPdf(TString path, TString variable, TString title, float min, float max, TString cut = "", unsigned int bin = 100, TString outputPath = "")
{
    auto g = DrawGraph(path, variable, title, min, max, cut, bin);
    TCanvas *c1 = new TCanvas("c1", variable, 800, 600);
    g->SetStats(kFALSE);
    g->Draw("HIST");
    TString savePath = outputPath.IsNull() ? variable + ".pdf" : outputPath + "/" + variable + ".pdf";
    c1->SaveAs(savePath);
}

std::shared_ptr<TH2F> DrawGraph(ROOT::TTreeProcessorMT *tp, TString x_variable, TString y_variable, TString title, 
                                float x_min, float x_max, float y_min, float y_max, 
                                unsigned int x_bin = 100, unsigned int y_bin = 100)
{
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
    auto fillHistogram = [&ptHist, &x_variable, &y_variable, &min_x, &max_x, &min_y, &max_y](TTreeReader &reader)
    {
        TTreeReaderValue<float> x_var(reader, x_variable);
        TTreeReaderValue<float> y_var(reader, y_variable);
        auto Hist = ptHist.Get();
        while (reader.Next())
        {
            if (*x_var >= min_x && *x_var <= max_x && *y_var >= min_y && *y_var <= max_y){
                Hist->Fill(*x_var, *y_var);
            }
        }
    };
    tp->Process(fillHistogram);
    auto Hist = ptHist.Merge();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken: " << elapsed.count() << " seconds" << std::endl;
    return Hist;
}

std::shared_ptr<TH2F> DrawGraph(TString path, TString x_variable, TString y_variable, TString title, 
                                float x_min, float x_max, float y_min, float y_max, TString cut = "",
                                unsigned int x_bin = 100, unsigned int y_bin = 100)
{
    ROOT::TTreeProcessorMT *tp = GetTree<ROOT::TTreeProcessorMT>(path, cut);
    auto result = DrawGraph(tp, x_variable, y_variable, title, x_min, x_max, y_min, y_max, x_bin, y_bin);
    delete tp;
    if (!cut.IsNull()) {
        std::remove("temp_filtered.root");
    }
    return result;
}

std::pair<std::shared_ptr<TH2F>, std::shared_ptr<TH2F>> DrawSideBand(ROOT::TTreeProcessorMT *tp_peak, ROOT::TTreeProcessorMT *tp_side,
                                                                       TString x_variable, TString y_variable, TString title,
                                                                       float x_min, float x_max, float y_min, float y_max,
                                                                       unsigned int x_bin = 100, unsigned int y_bin = 100)
{
    double max_x = x_max;
    double min_x = x_min;
    double max_y = y_max;
    double min_y = y_min;

    if (x_min == x_max)
    {
        auto range_x_peak = AutoRange(tp_peak, x_variable);
        min_x = range_x_peak.first;
        max_x = range_x_peak.second;
        auto range_x_side = AutoRange(tp_side, x_variable);
        min_x = std::min(min_x, range_x_side.first);
        max_x = std::max(max_x, range_x_side.second);
    }
    if (y_min == y_max)
    {
        auto range_y_peak = AutoRange(tp_peak, y_variable);
        min_y = range_y_peak.first;
        max_y = range_y_peak.second;
        auto range_y_side = AutoRange(tp_side, y_variable);
        min_y = std::min(min_y, range_y_side.first);
        max_y = std::max(max_y, range_y_side.second);
    }
    auto hist_peak = DrawGraph(tp_peak, x_variable, y_variable, title, min_x, max_x, min_y, max_y, x_bin, y_bin);
    auto hist_side = DrawGraph(tp_side, x_variable, y_variable, title, min_x, max_x, min_y, max_y, x_bin, y_bin);
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

std::pair<std::shared_ptr<TH2F>, std::shared_ptr<TH2F>> DrawSideBand(TString path,
                                                                       TString x_variable, TString y_variable, TString title,
                                                                       float x_min, float x_max, float y_min, float y_max,
                                                                       TString cut = "",
                                                                       unsigned int x_bin = 100, unsigned int y_bin = 100)
{
    TString cut_peak = cut + (cut.IsNull() ? "" : " && ") + "(" + TString(PeakSelectionCut) + ")";
    TString cut_side = cut + (cut.IsNull() ? "" : " && ") + "(" + TString(SideBandSelectionCut) + ")";
    ROOT::TTreeProcessorMT *tp_peak = GetTree<ROOT::TTreeProcessorMT>(path, cut_peak,"peak.root");
    ROOT::TTreeProcessorMT *tp_side = GetTree<ROOT::TTreeProcessorMT>(path, cut_side,"side.root");
    auto result = DrawSideBand(tp_peak, tp_side, x_variable, y_variable, title, x_min, x_max, y_min, y_max, x_bin, y_bin);
    delete tp_peak;
    delete tp_side;
    std::remove("peak.root");
    std::remove("side.root");
    return result;
}

void DrawSideBandAsPdf(TString path, TString x_variable, TString y_variable, TString title, 
                         float x_min, float x_max, float y_min, float y_max, 
                         std::string cuts = "", 
                         unsigned int x_bin = 100, unsigned int y_bin = 100, TString outputPath = "")
{
    auto hists = DrawSideBand(path, x_variable, y_variable, title, x_min, x_max, y_min, y_max, cuts, x_bin, y_bin);
    auto hist_peak = hists.first;
    auto hist_side = hists.second;
    // 使用新函数设置 Z 轴范围，确保所有图都能完整显示
    SetZAxisRangeForAll(hist_peak.get(), hist_side.get());
    TCanvas *c1 = new TCanvas("c1", "Side Band 2D", 800, 600);
    hist_peak->SetMarkerColor(kRed);
    hist_peak->SetStats(kFALSE);
    hist_peak->Draw("COLZ");
    TString savePath1 = outputPath.IsNull() ? x_variable + "_" + y_variable + "_peak.pdf" : outputPath + "/" + x_variable + "_" + y_variable + "_peak.pdf";
    c1->SaveAs(savePath1);
    TCanvas *c2 = new TCanvas("c2", "Side Band 2D", 800, 600);
    hist_side->SetMarkerColor(kBlue);
    hist_side->SetStats(kFALSE);
    hist_side->Draw("COLZ, same");
    TString savePath2 = outputPath.IsNull() ? x_variable + "_" + y_variable + "_side.pdf" : outputPath + "/" + x_variable + "_" + y_variable + "_side.pdf";
    c2->SaveAs(savePath2);
}

void DrawAsPdf(TString path, TString x_variable, TString y_variable, TString title, 
                 float x_min, float x_max, float y_min, float y_max, TString cut = "",
                 unsigned int x_bin = 100, unsigned int y_bin = 100, TString outputPath = "")
{
    auto g = DrawGraph(path, x_variable, y_variable, title, x_min, x_max, y_min, y_max, cut, x_bin, y_bin);
    TCanvas *c1 = new TCanvas("c1", x_variable + "_" + y_variable, 800, 600);
    g->SetStats(kFALSE);
    g->Draw("HIST");
    g->Draw("COLZ");
    TString savePath = outputPath.IsNull() ? x_variable + "_" + y_variable + ".pdf" : outputPath + "/" + x_variable + "_" + y_variable + ".pdf";
    c1->SaveAs(savePath);
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
struct Result
{
    double cut;
    double merit;
    double n;
    // this "fit" is now useless, consider removing it
    unsigned int fit;
};
Result &FindBestCut(std::vector<Result> &result, double initial_merit, unsigned int k_smooth = 21)
{
    auto it = std::remove_if(result.begin(), result.end(), [](Result r)
                             { return (r.merit <= 0.8 || r.merit > 160); });
    result.erase(it, result.end());
    std::sort(result.begin(), result.end(), [](Result a, Result b)
              { return a.cut < b.cut; });
    bool isLarger = false;
    for (auto &r : result){
        if (r.merit > initial_merit){
            isLarger = true;
            break;
        }
    }
    if (!isLarger){
        static Result r = {0, -1, 0, 0};
        return r;
    }
    std::vector<double> merit;
    unsigned int n = result.size();
    if (!n){
        throw std::runtime_error("FindBestCut: No valid results found");
    }

    for (auto o = 0; o < n; o++)
        merit.push_back(MedianAbsoluteDiff(n, o, k_smooth, [&result](unsigned int t) -> double
                                           { return result.at(t).merit; }));
    for (auto o = 0; o < n; o++)
        merit.at(o) = (SavitzkyGolay(n, o, k_smooth, [&merit](unsigned int t) -> double
                                     { return merit.at(t); }));
    auto maxIt = std::max_element(merit.begin(), merit.end());
    unsigned int index = std::distance(merit.begin(), maxIt);
    for (auto o = 0; o < n; o++)
        result.at(o).merit = merit.at(o);
    return result.at(index);
}

inline bool IsExcluded(const std::string &varName, const std::vector<std::string> &excludedVars)
{
    for (const auto &excludedPattern : excludedVars) {
        if (varName == excludedPattern) {
            return true;
        }
        try {
            std::regex pattern(excludedPattern);
            if (std::regex_match(varName, pattern)) {
                return true;
            }
        } catch (const std::regex_error &) {
        }
    }
    return false;
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
        if (!line.empty() && !IsExcluded(line, excludedVars)) {
            variables.push_back(line);
        }
    }
    inFile.close();
    return variables;
}

inline std::vector<std::string> GetVariableListFromTree(TTree *tree, const std::vector<std::string> &excludedVars)
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
        if (!IsExcluded(branchName, excludedVars)) {
            variables.push_back(branchName);
        }
    }
    
    return variables;
}

#endif
