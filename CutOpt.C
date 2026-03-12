#include <vector>
#include <iostream>
#include <string>
#include <dirent.h>
#include <functional>
#include <fstream>
#include <c++/4.8.2/thread>

#include "TString.h"
#include "TChain.h"
#include "TCanvas.h"
#include <ROOT/TProcessExecutor.hxx>
#include <ROOT/TThreadedObject.hxx>
#include <ROOT/TTreeProcessorMT.hxx>

#include "TH1F.h"
#include "TROOT.h"

#include "RooRealVar.h"
#include "RooStats/SPlot.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooBreitWigner.h"
#include "RooChebychev.h"
#include "RooAddPdf.h"
#include "RooAddition.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include "RooProdPdf.h"
#include "RooProduct.h"
#include "RooCBShape.h"
#include "TSystem.h"
#include "RooFitResult.h"
#include "DrawGraph.h"

using namespace RooFit;
using namespace RooStats;

struct Work
{
    double cut;
    RooDataSet *data;
};
struct Result
{
    double cut;
    double merit;
    double n;
};
ROOT::TThreadedObjectUtils::MergeFunctionType<std::vector<float>> MergeVectors = [](std::shared_ptr<std::vector<float>> target, std::vector<std::shared_ptr<std::vector<float>>> &objs)
{
    size_t total = 0;
    for (const auto &obj : objs)
    {
        if (obj && obj != target)
            total += obj->size();
    }
    target->reserve(total);
    for (const auto &obj : objs)
    {
        if (obj && obj != target)
            target->insert(target->end(), obj->begin(), obj->end());
    }
};
Result Fit(Work work, bool isDraw = false)
{
    // Set signal and background PDF and paramiters
    // signal: double gaussian mixing
    // background: 2th order Chebychev for Jpsi and 3th order Chebychev for Psi2s

    // signal config
    RooDataSet *data = work.data;
    RooRealVar Psi2S_mass("Psi2S_mass", "Psi2S_mass", 3.64, 3.73);

    RooRealVar mean("meanCB1_psi2s", "meanCB1_psi2s", 3.68619e+00, 3.68, 3.69);
    RooRealVar sigma1 = RooRealVar("sigma1", "sigma1", 6.27649e-03, 0.0, 0.06);
    RooRealVar sigma2 = RooRealVar("sigma2", "sigma2", 4.99147e-02, 0.0, 0.06);

    RooRealVar frac("frac", "frac", 0.5);
    RooGaussian sigPdf1 = RooGaussian("sigPdf1", "sigPdf1", Psi2S_mass, mean, sigma1);
    RooGaussian sigPdf2 = RooGaussian("sigPdf2", "sigPdf2", Psi2S_mass, mean, sigma2);
    RooAddPdf sigPdf("sigPdf", "sigPdf", RooArgList(sigPdf1, sigPdf2), frac);

    // background config
    RooRealVar c1_bkg("c1_bkg", "c1_bkg", 2.24023e-01, -0.8, 0.8);
    RooRealVar c2_bkg("c2_bkg", "c2_bkg", -1.64660e-02, -0.6, 0.6);
    RooRealVar c3_bkg("c3_bkg", "c3_bkg", 1.18262e-02, -0.07, 0.07);
    RooChebychev bkgPdf("bkg", "bkg", Psi2S_mass, RooArgList(c1_bkg, c2_bkg, c3_bkg));

    // Fraction of psi2s and bkg, also total event numbers of each
    RooRealVar n_sig("n_sig", "n_sig", 1, 0, 1000000);
    RooRealVar n_bkg("n_bkg", "n_bkg", 1000, 0, 1000000);

    RooAddPdf totalPdf("totalPdf", "totalPdf", RooArgList(sigPdf, bkgPdf), RooArgList(n_sig, n_bkg));
    double Data_edm;
    int Data_status, Data_covQual;
    RooFitResult *fitRes;
    unsigned int ii = 0;
    while (true)
    {

        fitRes = totalPdf.fitTo(*data, Hesse(kTRUE), NumCPU(8), Save(kTRUE), Minos(kFALSE), PrintLevel(-1), Warnings(kFALSE), PrintEvalErrors(-1));
        Data_edm = fitRes->edm();
        Data_status = fitRes->status();
        Data_covQual = fitRes->covQual();
        ii = ii + 1;
        if ((Data_edm < 0.01 && Data_status == 0 && Data_covQual == 3) || ii >= 20)
            break;
        delete fitRes;
    }
    Psi2S_mass.setRange("massPeak", 3.68, 3.69);

    double fraclimitbkg = bkgPdf.createIntegral(RooArgSet(Psi2S_mass), Range("massPeak"))->getVal() / bkgPdf.createIntegral(RooArgSet(Psi2S_mass))->getVal();
    double fraclimitsig = sigPdf.createIntegral(RooArgSet(Psi2S_mass), Range("massPeak"))->getVal() / sigPdf.createIntegral(RooArgSet(Psi2S_mass))->getVal();

    // getntotal
    double nbkgTotal = n_bkg.getVal();
    double nsigTotal = n_sig.getVal();

    // getnlimit
    double nbkglimit = nbkgTotal * fraclimitbkg;
    double nsiglimit = nsigTotal * fraclimitsig;

    double m = nsiglimit / sqrt(nsiglimit + nbkglimit);
    if (!(Data_edm < 0.01 && Data_status == 0 && Data_covQual == 3))
        m = -1;
    if (isDraw)
    {
        RooPlot *frame = Psi2S_mass.frame();
        data->plotOn(frame);
        totalPdf.plotOn(frame, LineColor(kBlue));
        // bkgPdf.plotOn(frame, LineColor(kRed), LineStyle(kDashed));
        // sigPdf.plotOn(frame, LineColor(kGreen), LineStyle(kDashed));
        TCanvas c;
        frame->Draw();
        c.SaveAs("fit_result.png");
    }

    Result r = {work.cut, m, data->sumEntries()};
    delete data, fitRes;
    return r;
}
template <class get>
double SavitzkyGolay(unsigned int n, unsigned int i, unsigned int k, get p)
{
    if (k % 2 == 0 || k >= n || k == 1)
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
Result &FindBestCut(std::vector<Result> &result)
{
    auto it = std::remove_if(result.begin(), result.end(), [](Result r)
                             { return (r.merit <= 0.8 || r.merit > 1e4); });
    result.erase(it, result.end());
    std::sort(result.begin(), result.end(), [](Result a, Result b)
              { return a.cut < b.cut; });
    std::vector<double> merit;
    unsigned int n = result.size();
    for (auto o = 0; o < n; o++)
        merit.push_back(SavitzkyGolay(n, o, 9, [&result](unsigned int t) -> double
                                      { return result.at(t).merit; }));
    auto maxIt = std::max_element(merit.begin(), merit.end());
    unsigned int index = std::distance(merit.begin(), maxIt);
    return result.at(index);
}
void TestFit(TString path, TString variable, double min, double max)
{
    TChain SourceTree("SourceTree", "");
    SourceTree.Add(path + "/*.root");
    RooRealVar Psi2S_mass("Psi2S_mass", "M(#mu#mu#pi#pi)-M(#mu#mu)+3.0969 GeV", 3.64, 3.73);
    RooRealVar roovar(variable, variable, min, max);
    RooArgSet variables;
    variables.add(Psi2S_mass);
    variables.add(roovar);
    RooDataSet *data = new RooDataSet("data", "Psi2S_mass", variables, RooFit::Import(SourceTree));
    Work work = {min + (max - min) / 2, data};
    Fit(work, true);
}
void CutOpt(TString path, TString variable)
{
    // Load ROOT files and set up multithreading
    auto start = std::chrono::high_resolution_clock::now();
    auto pair = GetTree(path);
    // Step 1: Find the range of the variable and determine whether to optimize minimum or maximum cut
    // Automatically determine histogram varible range, [mu - 3*sigma, mu + 3*sigma], if min == max
    auto range = AutoRange(pair, variable);
    double min = range.first;
    double max = range.second;
    auto hists = DrawSideBand(pair, variable, variable, min, max);
    // True change minimum, False change upper limit
    bool min_or_max = (hists.first->GetMean() - hists.second->GetMean() > 0);
    // Step 2: Fitting
    RooRealVar Psi2S_mass("Psi2S_mass", "M(#mu#mu#pi#pi)-M(#mu#mu)+3.0969 GeV", 3.64, 3.73);
    RooRealVar roovar(variable, variable, min, max);
    RooArgSet variables;
    variables.add(Psi2S_mass);
    variables.add(roovar);
    RooDataSet *data = new RooDataSet("data", "Psi2S_mass", variables, RooFit::Import(*pair.second));
    ROOT::TProcessExecutor pool;
    // Varying maximum
    std::vector<Work> works;
    double x = 10.0; // number of points to test on each side
    double d = (max - min) / x;
    if (min_or_max)
    {
        // Varying minimum
        for (auto i = 1; i <= x; i++)
        {
            roovar.setRange(max - d * i, max);
            std::string name = "Psi2S_mass_min_" + std::to_string(i);
            RooDataSet *data_ = new RooDataSet(name.c_str(), "Psi2S_mass", variables, RooFit::Import(*data));
            works.push_back({max - d * i, data_});
        }
    }
    else
    {
        // Varying maximum
        for (auto i = 1; i <= x; i++)
        {
            roovar.setRange(min, min + d * i);
            std::string name = "Psi2S_mass_min_" + std::to_string(i);
            RooDataSet *data_ = new RooDataSet(name.c_str(), "Psi2S_mass", variables, RooFit::Import(*data));
            works.push_back({min + d * i, data_});
        }
    }
    auto results = pool.Map([](Work work) -> Result
                            { return Fit(work); }, works);
    auto report = FindBestCut(results);
    works.clear();
    // Step 3: Output
    std::ofstream out;
    std::cout << results.size() << std::endl;
    out.open("/home/storage0/users/junkaiqin/X3872Analysis/cut_optimization_results.txt", std::ios::out | std::ios::app);
    // out << "path,variable,cut,merit,efficiency" << std::endl;
    std::string cut;
    std::string v = std::string(variable);
    if (min_or_max)
    {
        cut = std::to_string(report.cut) + std::string("<") + v + std::string("<") + std::to_string(max);
    }
    else
    {
        cut = std::to_string(min) + std::string("<") + v + std::string("<") + std::to_string(report.cut);
    }
    out << path << "," << cut << "," << report.merit << "," << report.n / pair.second->GetEntries() << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Total time taken: " << elapsed.count() << " seconds" << std::endl;
}