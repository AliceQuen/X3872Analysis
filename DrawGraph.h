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
#define SIDE_BAND_DOWN_R 2.71
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
ROOT::TThreadedObjectUtils::MergeFunctionType<double> MergeFloats = [](std::shared_ptr<double> target, std::vector<std::shared_ptr<double>> &objs)
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
std::shared_ptr<TH1F> DrawGraph(TString path, TString variable, TString title, float min, float max, unsigned int bin = 100, std::function<bool(float)> selection = [](double)
                                                                                                                             { return true; })
{
    auto start = std::chrono::high_resolution_clock::now();
    TChain SourceTree("SourceTree", "");
    SourceTree.Add(path + "/*.root");
    unsigned int entries = SourceTree.GetEntries();
    if (entries == 0)
    {
        std::cerr << "No ROOT files found in the specified path: " << path << std::endl;
        return nullptr;
    }
    int nthreads = std::thread::hardware_concurrency();
    void EnableThreadSafety();
    ROOT::EnableImplicitMT(nthreads);
    ROOT::TTreeProcessorMT tp(SourceTree);
    float max_x = max;
    float min_x = min;

    // Automatically determine histogram varible range, [mu - 3*sigma, mu + 3*sigma], if min == max
    if (min == max)
    {
        ROOT::TThreadedObject<double> sum;
        ROOT::TThreadedObject<double> sum2;

        // Calculate the sum x and x^2 in parallel, x = variable, then calculate mean and sigma
        // This calcualtion introduce some float precision error, but it is acceptable for histogram range estimation

        auto calculateSum = [&SourceTree, &selection, &sum, &sum2, &variable](TTreeReader &reader)
        {
            TTreeReaderValue<float> var(reader, variable);
            TTreeReaderValue<float> Psi2s_mass(reader, "Psi2S_mass");
            double local_sum = 0.0;
            double local_sum2 = 0.0;
            while (reader.Next())
            {
                if (selection(*Psi2s_mass))
                {
                    local_sum += *var;
                    local_sum2 += (*var) * (*var);
                }
            }
            *(sum.Get()).get() = local_sum;
            *(sum2.Get()).get() = local_sum2;
        };
        tp.Process(calculateSum);
        float mean = *(sum.Merge(MergeFloats)).get() / entries;
        float sigma = sqrt(*(sum2.Merge(MergeFloats)).get() / entries - mean * mean);
        max_x = mean + 3 * sigma;
        min_x = mean - 3 * sigma;
    }
    ROOT::TThreadedObject<TH1F> ptHist(variable, title, bin, min_x, max_x);
    auto fillHistogram = [&SourceTree, &ptHist, &selection, variable](TTreeReader &reader)
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
    tp.Process(fillHistogram);
    auto Hist = ptHist.Merge();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken: " << elapsed.count() << " seconds" << std::endl;
    return Hist;
}
void DrawSideBand(TString path, TString variable, TString title, float min, float max, unsigned int bin = 100)
{
    auto hist_side = DrawGraph(path, variable, title, min, max, bin, SideBandSelection);
    auto hist_peak = DrawGraph(path, variable, title, min, max, bin, PeakSelection);
    double scale = (PEAK_UP - PEAK_DOWN) / ((SIDE_BAND_UP_L - SIDE_BAND_DOWN_L) + (SIDE_BAND_UP_R - SIDE_BAND_DOWN_R));
    hist_peak->Add(hist_peak.get(), hist_side.get(), 1, -scale);
    hist_peak->Scale(1.0 / hist_peak->Integral());
    hist_side->Scale(1.0 / hist_side->Integral());
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