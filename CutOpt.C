#include <vector>
#include <iostream>
#include <limits>
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
#include "FindBestCut.C"

#define PSI2S_MASS_MIN 3.6
#define PSI2S_MASS_MAX 3.76

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
    unsigned int fit;
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
    RooDataSet *data = work.data;
    RooRealVar Psi2S_mass("Psi2S_mass", "Psi2S_mass", PSI2S_MASS_MIN, PSI2S_MASS_MAX);

    // signal config

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
    RooChebychev bkgPdf("bkgPdf", "bkgPdf", Psi2S_mass, RooArgList(c1_bkg, c2_bkg, c3_bkg));

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
        if (isDraw)
        {
            std::cout << ii << " " << Data_edm << " " << Data_status << " " << Data_covQual << std::endl;
        }
        ii = ii + 1;
        if ((Data_edm < 0.01 && Data_status == 0 && Data_covQual == 3) || ii >= 10)
            break;
        delete fitRes;
    }
    if (isDraw)
    {
        std::cout << "at last we have " << ii << " " << Data_edm << " " << Data_status << " " << Data_covQual << std::endl;
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

    Result r = {work.cut, m, data->sumEntries(), ii};
    delete data, fitRes;
    return r;
}
inline double Midian(std::vector<double> &nums)
{
    std::sort(nums.begin(), nums.end());
    auto size = nums.size();
    if (!size)
        throw std::runtime_error("Trying to find midian for an empty vector");
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
double MidianAbsoluteDiff(unsigned int n, unsigned int i, unsigned int k, get p)
{
    // return the origin value if can't do the averaging
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
    double mid = Midian(m);
    std::vector<double> ad;
    for (auto mi : m)
        ad.push_back(fabs(mi - mid));
    std::sort(ad.begin(), ad.end());
    double mad = Midian(ad);
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
Result &FindBestCut(std::vector<Result> &result)
{
    auto it = std::remove_if(result.begin(), result.end(), [](Result r)
                             { return (r.merit <= 0.8 || r.merit > 160); });
    result.erase(it, result.end());
    std::sort(result.begin(), result.end(), [](Result a, Result b)
              { return a.cut < b.cut; });
    std::vector<double> merit;
    unsigned int n = result.size();
    for (auto o = 0; o < n; o++)
        merit.push_back(MidianAbsoluteDiff(n, o, 9, [&result](unsigned int t) -> double
                                           { return result.at(t).merit; }));
    for (auto o = 0; o < n; o++)
        merit.at(o) = (SavitzkyGolay(n, o, 9, [&merit](unsigned int t) -> double
                                     { return merit.at(t); }));
    auto maxIt = std::max_element(merit.begin(), merit.end());
    unsigned int index = std::distance(merit.begin(), maxIt);
    for (auto o = 0; o < n; o++)
        result.at(o).merit = merit.at(o);
    return result.at(index);
}
void TestFit(TString path, std::string cut)
{
    TChain SourceTree("SourceTree", "");
    SourceTree.Add(path + "/*.root");

    auto start = std::chrono::high_resolution_clock::now();
    CutPoint c(cut, false);
    auto data = c.CuttedData(SourceTree);
    Work work = {2, data};
    auto result = Fit(work, true);
    std::cout << "Merit: " << result.merit << " Efficiency: " << result.n / SourceTree.GetEntries() << " Fitted for " << result.fit << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "We are testint Total time taken: " << elapsed.count() << " seconds" << std::endl;
}
void CutOpt(TString path, TString variable)
{
    // Load ROOT files and set up multithreading
    auto start = std::chrono::high_resolution_clock::now();
    TChain SourceTree("SourceTree", "");
    SourceTree.Add(path + "/*.root");
    unsigned int entries = SourceTree.GetEntries();
    if (entries == 0)
        throw std::runtime_error("No ROOT files found in the specified path: ");
    auto tp = GetTree(SourceTree);
    // Step 1: Find the range of the variable and determine whether to optimize minimum or maximum cut
    // Automatically determine histogram varible range, [mu - 3*sigma, mu + 3*sigma], if min == max
    auto range = AutoRange(tp, variable, [](float a)
                           { return (PSI2S_MASS_MIN <= a && a <= PSI2S_MASS_MAX); });
    double min = range.first;
    double max = range.second;
    auto hs_side_band = DrawSideBand(tp, variable, variable, min, max);
    auto h_psi2s_mass = DrawGraph(tp, variable, variable, PSI2S_MASS_MIN, PSI2S_MASS_MAX, 500, [](float a)
                                  { return (PSI2S_MASS_MIN <= a && a <= PSI2S_MASS_MAX); });
    auto total = h_psi2s_mass->GetEntries();
    // True change minimum, False change upper limit
    bool min_or_max = (hs_side_band.first->GetMean() - hs_side_band.second->GetMean() > 0);

    // Step 2: Fitting
    RooRealVar Psi2S_mass("Psi2S_mass", "M(#mu#mu#pi#pi)-M(#mu#mu)+3.0969 GeV", PSI2S_MASS_MIN, PSI2S_MASS_MAX);
    RooRealVar roovar(variable, variable, -std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    RooArgSet variables;
    variables.add(Psi2S_mass);
    variables.add(roovar);
    RooDataSet *data = new RooDataSet("data", "Psi2S_mass", variables, RooFit::Import(SourceTree));
    ROOT::TProcessExecutor pool;
    // Varying maximum
    std::vector<Work> works;
    double x = 100; // number of points to test on each side
    double d = (max - min) / x;
    if (min_or_max)
    {
        // Varying minimum
        for (auto i = 1; i <= x; i++)
        {
            // roovar.setRange(max - d * i, max);
            TString cut = variable + ">" + TString(std::to_string(max - d * i));
            std::string name = "Psi2S_mass_min_" + std::to_string(i);
            RooDataSet *data_ = new RooDataSet(name.c_str(), "Psi2S_mass", variables, RooFit::Cut(cut), RooFit::Import(*data));
            works.push_back({max - d * i, data_});
        }
    }
    else
    {
        // Varying maximum
        for (auto i = 1; i <= x; i++)
        {
            // roovar.setRange(min, min + d * i);
            TString cut = variable + "<" + TString(std::to_string(min + d * i));
            std::string name = "Psi2S_mass_min_" + std::to_string(i);
            RooDataSet *data_ = new RooDataSet(name.c_str(), "Psi2S_mass", variables, RooFit::Cut(cut), RooFit::Import(*data));
            works.push_back({min + d * i, data_});
        }
    }
    auto results = pool.Map([](Work work) -> Result
                            { return Fit(work); }, works);

    auto report = FindBestCut(results);
    works.clear();
    // Step 3: Output
    std::ofstream out;

    out.open("/home/storage0/users/junkaiqin/X3872Analysis/cut_optimization_results.txt", std::ios::out | std::ios::app);
    std::string cut;
    if (min_or_max)
    {
        cut = TString(std::to_string(report.cut)) + "<" + variable;
    }
    else
    {
        cut = TString(std::to_string(report.cut)) + ">" + variable;
    }
    out << path << "," << cut << "," << report.merit << "," << report.n / total << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Total time taken: " << elapsed.count() << " seconds" << std::endl;
}