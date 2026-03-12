#include <vector>
#include <iostream>
#include <string>
#include <dirent.h>
#include <functional>
#include <c++/4.8.2/thread>

#include "TString.h"
#include "TChain.h"
#include "TCanvas.h"
#include <ROOT/TThreadedObject.hxx>
#include <ROOT/TTreeProcessorMT.hxx>
#include "TH1F.h"
#include "TROOT.h"

#define PEAK_UP 3.69
#define PEAK_DOWN 3.68
#define SIDE_BAND_UP_L 3.67
#define SIDE_BAND_DOWN_L 3.65
#define SIDE_BAND_UP_R 3.728
#define SIDE_BAND_DOWN_R 3.71
#define X3872_UP 3.88
#define X3872_DOWN 3.869

std::function<bool(float)> PeakSelection = [](float mass)
{
    return (mass > PEAK_DOWN && mass < PEAK_UP);
};
std::function<bool(float)> SideBandSelection = [](float mass)
{
    return ((mass > SIDE_BAND_DOWN_L && mass < SIDE_BAND_UP_L) || (mass > SIDE_BAND_DOWN_R && mass < SIDE_BAND_UP_R));
};
std::function<bool(float)> X3872Selection = [](float mass)
{
    return (mass > X3872_DOWN && mass < X3872_UP);
};
ROOT::TThreadedObjectUtils::MergeFunctionType<double> MergeDoubles = [](std::shared_ptr<double> target, std::vector<std::shared_ptr<double>> &objs)
{
    double sum = 0.0;
    for (auto &obj : objs)
    {
        if (obj && obj != target)
        {
            // if (obj) {
            *target += *obj;
        }
    }
};
// Tree and its multi-thread reader
using tree_pair = std::pair<ROOT::TTreeProcessorMT *, TChain *>;
tree_pair GetTree(TString path)
{

    TChain *SourceTree = new TChain("SourceTree", "");
    SourceTree->Add(path + "/*.root");
    unsigned int entries = SourceTree->GetEntries();
    if (entries == 0)
        throw std::runtime_error("No ROOT files found in the specified path: ");
    int nthreads = std::thread::hardware_concurrency();
    void EnableThreadSafety();
    ROOT::EnableImplicitMT(nthreads);
    return std::make_pair(new ROOT::TTreeProcessorMT(*SourceTree), SourceTree);
}
std::pair<float, float> AutoRange(tree_pair pair, TString variable, std::function<bool(float)> selection = [](double)
                                                                    { return true; })
{
    ROOT::TThreadedObject<double> sum;
    ROOT::TThreadedObject<double> sum2;
    ROOT::TThreadedObject<double> min;
    ROOT::TThreadedObject<double> max;
    // Calculate the sum x and x^2 in parallel, x = variable, then calculate mean and sigma
    // This calcualtion introduce some float precision error, but it is acceptable for histogram range estimation
    // Automatically determine histogram varible range, [mu - 3*sigma, mu + 3*sigma] intersection [min_data, max_data], if min == max
    unsigned int entries = pair.second->GetEntries();
    auto calculateSum = [&selection, &sum, &sum2, &min, &max, &variable](TTreeReader &reader)
    {
        TTreeReaderValue<float> var(reader, variable);
        TTreeReaderValue<float> Psi2s_mass(reader, "Psi2S_mass");
        double local_sum = 0.0;
        double local_sum2 = 0.0;
        double local_min = 0.0;
        double local_max = 0.0;
        while (reader.Next())
        {
            if (selection(*Psi2s_mass))
            {
                local_sum += *var;
                local_sum2 += (*var) * (*var);
                if (*var > local_max)
                    local_max = *var;
                if (*var < local_min)
                    local_min = *var;
            }
        }
        *(sum.Get()).get() = local_sum;
        *(sum2.Get()).get() = local_sum2;
        *(max.Get()).get() = local_max;
        *(min.Get()).get() = local_min;
    };
    pair.first->Process(calculateSum);
    double mean = *(sum.Merge(MergeDoubles)).get() / entries;
    double sigma = sqrt(*(sum2.Merge(MergeDoubles)).get() / entries - mean * mean);
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
    std::cout << min_ << " " << max_ << std::endl;
    return std::make_pair(std::max(mean - 3 * sigma, min_), std::min(mean + 3 * sigma, max_));
};
std::shared_ptr<TH1F> DrawGraph(tree_pair pair, TString variable, TString title, float min, float max, unsigned int bin = 100, std::function<bool(float)> selection = [](double)
                                                                                                                               { return true; })
{
    auto start = std::chrono::high_resolution_clock::now();
    float max_x = max;
    float min_x = min;
    ROOT::TTreeProcessorMT *tp = pair.first;
    // Automatically determine histogram varible range, [mu - 3*sigma, mu + 3*sigma], if min == max
    if (min == max)
    {
        auto range = AutoRange(pair, variable, selection);
        min_x = range.first;
        max_x = range.second;
    }
    ROOT::TThreadedObject<TH1F> ptHist(variable, title, bin, min_x, max_x);
    auto fillHistogram = [&ptHist, &selection, variable](TTreeReader &reader)
    {
        TTreeReaderValue<float> var(reader, variable);
        TTreeReaderValue<float> Psi2s_mass(reader, "Psi2S_mass");
        auto Hist = ptHist.Get();
        while (reader.Next())
        {
            if (selection(*Psi2s_mass))
            {
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
std::shared_ptr<TH1F> DrawGraph(TString path, TString variable, TString title, float min, float max, unsigned int bin = 100, std::function<bool(float)> selection = [](double)
                                                                                                                             { return true; })
{
    auto pair = GetTree(path);
    auto result = DrawGraph(pair, variable, title, min, max, bin, selection);
    delete pair.first;
    return result;
}
std::pair<std::shared_ptr<TH1F>, std::shared_ptr<TH1F>> DrawSideBand(tree_pair pair, TString variable, TString title, float min, float max, unsigned int bin = 100)
{
    float min_x = min;
    float max_x = max;
    if (min == max)
    {
        auto range = AutoRange(pair, variable);
        min_x = range.first;
        max_x = range.second;
    }
    auto hist_side = DrawGraph(pair, variable, title, min_x, max_x, bin, SideBandSelection);
    auto hist_peak = DrawGraph(pair, variable, title, min_x, max_x, bin, PeakSelection);
    double scale = (PEAK_UP - PEAK_DOWN) / ((SIDE_BAND_UP_L - SIDE_BAND_DOWN_L) + (SIDE_BAND_UP_R - SIDE_BAND_DOWN_R));
    hist_peak->Add(hist_peak.get(), hist_side.get(), 1, -scale);
    for (unsigned int i = 0; i < bin; i++)
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
std::pair<std::shared_ptr<TH1F>, std::shared_ptr<TH1F>> DrawSideBand(TString path, TString variable, TString title, float min, float max, unsigned int bin = 100)
{
    auto pair = GetTree(path);
    auto result = DrawSideBand(pair, variable, title, min, max, bin);
    delete pair.first;
    return result;
}
void DrawSideBandAsPdf(TString path, TString variable, TString title, float min, float max, unsigned int bin = 100)
{
    auto hists = DrawSideBand(path, variable, title, min, max, bin);
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
// Simply draw a variable and save it as a pdf
void DrawAsPdf(TString path, TString variable, TString title, float min, float max, unsigned int bin = 100)
{
    auto g = DrawGraph(path, variable, title, min, max, bin);
    TCanvas *c1 = new TCanvas("c1", variable, 800, 600);
    g->Draw("HIST");
    c1->SaveAs(variable + ".pdf");
}