#include <vector>
#include <iostream>
#include <string>
#include <dirent.h>
#include <functional>
#include <c++/4.8.2/thread>

#include "TString.h"
#include "TChain.h"
#include "TCanvas.h"
#include <ROOT/TThreadExecutor.hxx>
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
Result Fit(Work work)
{
    // Set signal and background PDF and paramiters
    // signal: double gaussian mixing
    // background: 2th order Chebychev for Jpsi and 3th order Chebychev for Psi2s

    // signal config
    RooDataSet *data = work.data;
    RooRealVar Psi2S_mass("Psi2S_mass", "Psi2S_mass", 3.66, 3.71);

    RooRealVar mean("meanCB1_psi2s", "meanCB1_psi2s", 3.68619e+00, 3.68, 3.69);
    RooRealVar sigma1 = RooRealVar("sigma1", "sigma1", 0.0025, 0.001, 0.009);
    RooRealVar sigma2 = RooRealVar("sigma2", "sigma2", 0.0025, 0.001, 0.009);

    RooRealVar frac("frac", "frac", 0.5);
    RooGaussian sigPdf1 = RooGaussian("sigPdf1", "sigPdf1", Psi2S_mass, mean, sigma1);
    RooGaussian sigPdf2 = RooGaussian("sigPdf2", "sigPdf2", Psi2S_mass, mean, sigma2);
    RooAddPdf sigPdf("sigPdf", "sigPdf", RooArgList(sigPdf1, sigPdf2), frac);

    // background config
    RooRealVar c1_bkg("c1_bkg", "c1_bkg", 1.35819e-01, -0.8, 0.8);
    RooRealVar c2_bkg("c2_bkg", "c2_bkg", -4.91313e-02, -0.6, 0.6);
    RooRealVar c3_bkg("c3_bkg", "c3_bkg", 5.91313e-02, -0.07, 0.07);
    RooChebychev bkgPdf("bkg", "bkg", Psi2S_mass, RooArgList(c1_bkg, c2_bkg, c3_bkg));

    // Fraction of psi2s and bkg, also total event numbers of each
    RooRealVar n_sig("n_sig", "n_sig", 0, 0, 100000);
    RooRealVar n_bkg("n_bkg", "n_bkg", 1000, 0, 100000);

    RooAddPdf totalPdf("totalPdf", "totalPdf", RooArgList(sigPdf, bkgPdf), RooArgList(n_sig, n_bkg));

    double tem = 1.35819e-01;
    double Data_edm;
    int Data_status, Data_covQual;
    RooFitResult *fitRes;
    unsigned int ii = 0;
    while (true)
    {
        c1_bkg.setVal(tem);
        // fitRes = totalPdf.fitTo(*data, Hesse(kTRUE), Strategy(2), NumCPU(8), Save(kTRUE), Minos(kFALSE), PrintLevel(-1), Warnings(kFALSE), PrintEvalErrors(-1));
        fitRes = totalPdf.fitTo(*data, Hesse(kTRUE), BatchMode("cpu"), NumCPU(8), Save(kTRUE), Minos(kFALSE), PrintLevel(-1), Warnings(kFALSE), PrintEvalErrors(-1));
        Data_edm = fitRes->edm();
        Data_status = fitRes->status();
        Data_covQual = fitRes->covQual();
        ii = ii + 1;
        if ((Data_edm < 0.01 && Data_status == 0 && Data_covQual == 3) || ii >= 100)
        {
            break;
        }
        tem += 0.0001;
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
    {
        m = -1;
    }
    Result r = {work.cut, m, data->sumEntries()};
    delete data, fitRes;
    return r;
}
template<class get>
double SavitzkyGolay(unsigned int n, unsigned int i, unsigned int k, get p)
{
	if (k % 2 == 0 || k >= n || k == 1) return p(i);
	int k1 = (k - 1) / 2; 
	if (i < k1 || i > n - k1 - 1) return p(i);
	TGraph g;
	for (int j = 0; j < k; j++){
		g.SetPoint(j, j - k1, p(i - k1 + j));
	}
	g.Fit("pol3", "Q");
	auto f = g.GetFunction("pol3");
	return f->Eval(0);
}
void FindBestCut(std::vector<Result> &result)
{
   	auto it = std::remove_if(result.begin(), result.end(), [](Result r) { return (r.merit <= 0.8 || r.merit > 1e4); });
	result.erase(it, result.end());
	std::sort(result.begin(), result.end(), [](Result a, Result b) { return a.cut < b.cut; });
    std::vector<double> merit;
    unsigned int n = result.size();
    for (auto o = 0; o < n; o++) merit.push_back(SavitzkyGolay(n, o, 9, [&result](unsigned int t) -> double { return result.at(t).merit; }));
    auto maxIt = std::max_element(merit.begin(), merit.end());
    unsigned int index = std::distance(merit.begin(), maxIt);
    std::cout << "Best cut: " << result.at(index).cut << " with merit: " << result.at(index).merit << std::endl;
}
void CutOpt(TString path, TString variable)
{
    // Load ROOT files and set up multithreading
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
    ROOT::TTreeProcessorMT tp(SourceTree);
    // Step 1: Find the range of the variable
    ROOT::TThreadedObject<std::vector<float>> varValues(variable);
    auto collectValues = [&SourceTree, &varValues, variable](TTreeReader &reader)
    {
        TTreeReaderValue<float> var(reader, variable);
        auto values = varValues.Get();
        while (reader.Next())
        {
            values->push_back(*var);
        }
    };
    tp.Process(collectValues);
    auto values = varValues.Merge();
    if (values->empty())
    {
        std::cerr << "No entries found for variable: " << variable << std::endl;
        return;
    }
    std::sort(values->begin(), values->end());
    float min = values->front();
    float max = values->back();
    TH1F hist(variable, variable, 200, min, max);
    for (const auto &val : *values)
        hist.Fill(val);
    float smallEntry = values->size() * 0.001; // 0.1% of total entries
    bool findingMin = true;
    for (size_t i = 0; i < 200; i++)
    {
        if (hist.GetBinContent(i) > smallEntry && findingMin)
        {
            min = hist.GetBinLowEdge(i);
            findingMin = false;
        }
        if (hist.GetBinContent(i) < smallEntry && !findingMin)
        {
            max = hist.GetBinLowEdge(i + 1);
            break;
        }
    }
    std::cout << "Variable range: [" << min << ", " << max << "]" << std::endl;
    // Step 2: Fitting
    RooRealVar Psi2S_mass("Psi2S_mass", "M(#mu#mu#pi#pi)-M(#mu#mu)+3.0969 GeV", 3.66, 3.71);
    RooRealVar roovar(variable, variable, min, max);
    RooArgSet variables;
    variables.add(Psi2S_mass);
    variables.add(roovar);
    RooDataSet *data = new RooDataSet("data", "Psi2S_mass", variables, RooFit::Import(SourceTree));
    double total = values->size();
    ROOT::TThreadExecutor pool;
    // Varying maximum
    std::vector<Work> works;
    double x = 10; // number of points to test on each side
    double d = (max - min) / x;
    for (auto i = 1; i <= x; i++)
    {
        roovar.setRange(min, min + d * i);
        RooDataSet *data_ = new RooDataSet("Psi2S_mass", "Psi2S_mass", variables, RooFit::Import(*data));
        works.push_back({min + d * i, data_});
    }
    auto results_max = pool.Map([](Work work) -> Result { return Fit(work); }, works);
    std::cout << "Finished maximum cut optimization." << std::endl;
    FindBestCut(results_max);
    // Varying minimum
    for (auto i = 1; i <= x; i++)
    {
        roovar.setRange(max - d * i, max);
        RooDataSet *data_ = new RooDataSet("Psi2S_mass", "Psi2S_mass", variables, RooFit::Import(*data));
        works.push_back({max - d * i, data_});
    }
    auto results_min = pool.Map([](Work work) -> Result { return Fit(work); }, works);
    std::cout << "Finished minimum cut optimization." << std::endl;
    FindBestCut(results_min);
}