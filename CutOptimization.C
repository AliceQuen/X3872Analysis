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
#include "RooProduct.h"
#include "RooCBShape.h"
#include "RooFitResult.h"

#include "X3872Utils.h"
#include <TGraph.h>
#include <cstddef>
#include <cstdio>

#define PSI2S_MASS_MIN 3.6
#define PSI2S_MASS_MAX 3.76

using namespace RooFit;
using namespace RooStats;

struct Work
{
    double cut;
    RooDataSet *data;
    Work() : cut(0.0), data(nullptr) {}
    Work(double c, RooDataSet *d) : cut(c), data(d) {}
};

Result Fit(Work work, bool isDraw = false)
{
    RooDataSet *data = work.data;
    RooRealVar Psi2S_mass("Psi2S_mass", "Psi2S_mass", PSI2S_MASS_MIN, PSI2S_MASS_MAX);

    RooRealVar mean("meanCB1_psi2s", "meanCB1_psi2s", 3.68619e+00, 3.68, 3.69);
    RooRealVar sigma1 = RooRealVar("sigma1", "sigma1", 6.27649e-03, 0.0, 0.06);
    RooRealVar sigma2 = RooRealVar("sigma2", "sigma2", 4.99147e-02, 0.0, 0.06);

    RooRealVar frac("frac", "frac", 0.5);
    RooGaussian sigPdf1 = RooGaussian("sigPdf1", "sigPdf1", Psi2S_mass, mean, sigma1);
    RooGaussian sigPdf2 = RooGaussian("sigPdf2", "sigPdf2", Psi2S_mass, mean, sigma2);
    RooAddPdf sigPdf("sigPdf", "sigPdf", RooArgList(sigPdf1, sigPdf2), frac);

    RooRealVar c1_bkg("c1_bkg", "c1_bkg", 2.24023e-01, -0.8, 0.8);
    RooRealVar c2_bkg("c2_bkg", "c2_bkg", -1.64660e-02, -0.6, 0.6);
    RooRealVar c3_bkg("c3_bkg", "c3_bkg", 1.18262e-02, -0.07, 0.07);
    RooChebychev bkgPdf("bkgPdf", "bkgPdf", Psi2S_mass, RooArgList(c1_bkg, c2_bkg, c3_bkg));

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
        delete fitRes;
        if ((Data_edm < 0.01 && Data_status == 0 && Data_covQual == 3) || ii >= 10)
            break;
    }
    if (isDraw)
    {
        std::cout << "at last we have " << ii << " " << Data_edm << " " << Data_status << " " << Data_covQual << std::endl;
    }
    Psi2S_mass.setRange("massPeak", 3.68, 3.69);

    double fraclimitbkg = bkgPdf.createIntegral(RooArgSet(Psi2S_mass), Range("massPeak"))->getVal() / bkgPdf.createIntegral(RooArgSet(Psi2S_mass))->getVal();
    double fraclimitsig = sigPdf.createIntegral(RooArgSet(Psi2S_mass), Range("massPeak"))->getVal() / sigPdf.createIntegral(RooArgSet(Psi2S_mass))->getVal();

    double nbkgTotal = n_bkg.getVal();
    double nsigTotal = n_sig.getVal();

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
        TCanvas c;
        frame->Draw();
        c.SaveAs("fit_result.png");
    }

    Result r = {work.cut, m, data->sumEntries(), ii};
    return r;
}

void CutOpt(TString path, TString variable, double x = 500, double max_ = 0, double min_ = 0, bool min_or_max_ = true, unsigned int k_smooth = 21, bool isDraw = false)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::string full_cuts = "std::isfinite(Psi2S_VtxProb) && Psi2S_massErr >= 0 && Psi2S_mass > " + std::to_string(PSI2S_MASS_MIN) + " && Psi2S_mass < " + std::to_string(PSI2S_MASS_MAX);
    TTree *SourceTree = GetTree<TTree>(path, full_cuts);
    unsigned int entries = SourceTree->GetEntries();
    auto total = entries;
    if (entries == 0) {
        delete SourceTree;  
        throw std::runtime_error("No ROOT files found in the specified path: ");
    }
    ROOT::TTreeProcessorMT *tp = GetTree(*SourceTree);
    auto range = AutoRange(tp, variable);
    double min = min_;
    double max = max_;
    if (min_ == 0 && max_ == 0)
    {
        min = range.first;
        max = range.second;
    }
   
    bool min_or_max = min_or_max_;
    if (min_ == 0 && max_ == 0)
    {
        auto hs_side_band = DrawSideBand(path, variable, variable, min, max);
        min_or_max = (hs_side_band.first->GetMean() - hs_side_band.second->GetMean() > 0);
    }
    delete tp;

    RooRealVar Psi2S_mass("Psi2S_mass", "M(#mu#mu#pi#pi)-M(#mu#mu)+3.0969 GeV", PSI2S_MASS_MIN, PSI2S_MASS_MAX);
    RooRealVar roovar(variable, variable, -std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    RooArgSet variables;
    variables.add(Psi2S_mass);
    variables.add(roovar);
    RooDataSet *data = new RooDataSet("data", "Psi2S_mass", variables, RooFit::Import(*SourceTree));
    Work work;
    work.cut = 0.0;
    work.data = data;
    auto r = Fit(work);
    ROOT::TProcessExecutor pool;
    std::vector<Work> works;
    double d = (max - min) / x;

    if (min_or_max)
    {
        for (auto i = 1; i <= x; i++)
        {
            TString cut = variable + ">" + TString(std::to_string(max - d * i));
            std::string name = "Psi2S_mass_min_" + std::to_string(i);
            RooDataSet *data_ = new RooDataSet(name.c_str(), "Psi2S_mass", variables, RooFit::Cut(cut), RooFit::Import(*data));
            works.emplace_back(max - d * i, data_);
        }
    }
    else
    {
        for (auto i = 1; i <= x; i++)
        {
            TString cut = variable + "<" + TString(std::to_string(min + d * i));
            std::string name = "Psi2S_mass_min_" + std::to_string(i);
            RooDataSet *data_ = new RooDataSet(name.c_str(), "Psi2S_mass", variables, RooFit::Cut(cut), RooFit::Import(*data));
            works.emplace_back(min + d * i, data_);
        }
    }
    delete data;
    auto results = pool.Map([](Work work) -> Result
                            { return Fit(work); }, works);
    auto report = FindBestCut(results, r.merit, k_smooth);
    if (isDraw)
    {
        std::cout << "[INFO] isDraw is set to true" << std::endl;
        TGraph* gr = new TGraph(results.size());
        for (size_t i = 0; i < results.size(); ++i) {
            gr->SetPoint(i, results[i].cut, results[i].merit);
        }
        TCanvas c;
        gr->Draw();
        c.SaveAs("cut_optimization.pdf");
        c.Close();
        delete gr;
    }
    delete SourceTree;
    for (auto &work : works) {
        delete work.data;
    }
    works.clear();
    if (report.merit < 0)
        return;
    
    std::string cut;
    if (min_or_max)
    {
        cut = TString(std::to_string(report.cut)) + "<" + variable;
    }
    else
    {
        cut = TString(std::to_string(report.cut)) + ">" + variable;
    }
    if (!isDraw)
    {
        std::ofstream out;
        out.open("/home/storage29/users/junkaiqin/X3872Analysis/cut_optimization_results.txt", std::ios::out | std::ios::app);
        out << path << "," << cut << "," << report.merit << "," << report.n / total << std::endl;
    }else{
        std::cout << path << "," << cut << "," << report.merit << "," << report.n / total << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Total time taken: " << elapsed.count() << " seconds" << std::endl;
}
