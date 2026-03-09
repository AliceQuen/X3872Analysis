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

#define PEAK_UP 3.092
#define PEAK_DOWN 2.992
#define SIDE_BAND_UP_L 3.192
#define SIDE_BAND_DOWN_L 2.892
#define SIDE_BAND_UP_R 3.392
#define SIDE_BAND_DOWN_R 2.792
#define X3872_UP 3.872
#define X3872_DOWN 3.772

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
std::shared_ptr<TH1F> DrawGraph(TString path, TString variable, TString title, float min, float max, std::function<bool(float)> selection = [](double)
                                                                                                     { return true; })
{
    auto start = std::chrono::high_resolution_clock::now();
    TChain SourceTree("SourceTree", "");
    SourceTree.Add(path + "/*.root");
    if (SourceTree.GetEntries() == 0)
    {
        std::cerr << "No ROOT files found in the specified path: " << path << std::endl;
        return;
    }
    int nthreads = std::thread::hardware_concurrency();
    void EnableThreadSafety();
    ROOT::EnableImplicitMT(nthreads);
    ROOT::TThreadedObject<TH1F> ptHist(variable, title, 100, min, max);
    ROOT::TTreeProcessorMT tp(SourceTree);
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
void DrawSideBand(TString path, TString variable, TString title, float min, float max)
{
    auto hist_side = DrawGraph(path, variable, title, min, max, SideBandSelection);
    auto hist_peak = DrawGraph(path, variable, title, min, max, PeakSelection);
    double scale = (PEAK_UP - PEAK_DOWN) / ((SIDE_BAND_UP_L - SIDE_BAND_DOWN_L) + (SIDE_BAND_UP_R - SIDE_BAND_DOWN_R));
    hist_side->Add(hist_side.get(), hist_peak.get(), 1, -scale);
    hist_peak->Scale(1.0 / hist_peak->Integral());
    hist_side->Scale(1.0 / hist_side->Integral());
    double max = std::max(hist_peak->GetMaximum(), hist_side->GetMaximum());
    double min = std::min(hist_peak->GetMinimum(), hist_side->GetMinimum());
    hist_peak->GetYaxis()->SetRangeUser(min, max + 0.005);
    hist_side->GetYaxis()->SetRangeUser(min, max + 0.005);
    hist_peak->SetLineColor(kRed);
    hist_side->SetLineColor(kBlue);
    TCanvas *c1 = new TCanvas("c1", "Side Band", 800, 600);
    hist_peak->Draw("HIST");
    hist_side->Draw("HIST SAME");
    c1->SaveAs(variable + ".pdf");
}