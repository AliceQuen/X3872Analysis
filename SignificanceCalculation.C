#include <limits.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "TMath.h"
#include "X3872Utils.h"
#include "RooRealVar.h"
#include "RooStats/SPlot.h"
#include "RooStats/RooStatsUtils.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooBreitWigner.h"
#include "RooChebychev.h"
#include "RooAddPdf.h"
#include "RooAddition.h"
#include "RooExtendPdf.h"
#include "RooPlot.h"
#include "RooFitResult.h"
#include "RooProdPdf.h"
#include "RooProduct.h"
#include "RooCBShape.h"
#include "TSystem.h"
#include "RooFitResult.h"

#include "TCanvas.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TChain.h"

#include "TGraph.h"

#define RANGE_MIN 3.60
#define RANGE_MAX 3.76
#define MUMUPIPI_M 3.686e+00

using namespace std;
using namespace RooFit;
using namespace RooStats;

double Fit(RooDataSet *data, bool isBkg, unsigned int Nnorm)
{
	RooRealVar Psi2S_mass("Psi2S_mass", "M(#mu#mu#pi#pi)-M(#mu#mu)+3.0969 GeV", RANGE_MIN, RANGE_MAX);
	Psi2S_mass.setRange("fullRange", RANGE_MIN, RANGE_MAX);

	RooRealVar mean = RooRealVar("mean", "mean", MUMUPIPI_M, RANGE_MIN, RANGE_MAX);
	RooRealVar sigma1 = RooRealVar("sigma1", "sigma1", 0.0025, 0.001, 0.009);
	RooRealVar sigma2 = RooRealVar("sigma2", "sigma2", 0.0025, 0.001, 0.009);

	RooRealVar frac("frac", "frac", 0.5);
	RooGaussian sigPdf1 = RooGaussian("sigPdf1", "sigPdf1", Psi2S_mass, mean, sigma1);
	RooGaussian sigPdf2 = RooGaussian("sigPdf2", "sigPdf2", Psi2S_mass, mean, sigma2);
	RooAddPdf sigPdf("sigPdf", "sigPdf", RooArgList(sigPdf1, sigPdf2), frac);
	RooRealVar a0 = RooRealVar("a0", "a0", 10.0, 0.0, 3000.0);
	RooRealVar a1 = RooRealVar("a1", "a1", -0.10, -3000, 3000);
	RooChebychev chbPdf = RooChebychev("bkg", "bkg", Psi2S_mass, RooArgList(a0, a1));
	RooRealVar norm("norm", "norm", Nnorm);

	RooExtendPdf bkgPdf("bkgPdf", "bkgPdf", chbPdf, norm);
	RooRealVar nsig = RooRealVar("nsig", "nsig", 1, 0, 1e6);
	RooRealVar nbkg = RooRealVar("nbkg", "nbkg", 1000, 0, 1e6);
	RooAddPdf totalPdf = RooAddPdf("totalPdf", "totalPdf", RooArgList(sigPdf, bkgPdf), RooArgList(nsig, nbkg));

	double Data_edm;
	int Data_status, Data_covQual;
	RooFitResult *fitRes;
	unsigned int i = 0;

	RooPlot *MyFrame = Psi2S_mass.frame();
	TLegend *legend0 = new TLegend(.65, .1, .95, .35);

	while (true)
	{
		if (isBkg)
		{
			fitRes = bkgPdf.fitTo(*data, Hesse(kTRUE), Strategy(2), NumCPU(8), Save(kTRUE), Minos(kFALSE), PrintLevel(-1), Warnings(-1), PrintEvalErrors(-1));
		}
		else
		{
			fitRes = totalPdf.fitTo(*data, Hesse(kTRUE), Strategy(2), NumCPU(8), Save(kTRUE), Minos(kFALSE), PrintLevel(-1), Warnings(-1), PrintEvalErrors(-1));
		}
		Data_edm = fitRes->edm();
		Data_status = fitRes->status();
		Data_covQual = fitRes->covQual();
		i = i + 1;
		if ((Data_edm < 0.01 && Data_status == 0 && Data_covQual == 3) || i >= 100)
		{
			break;
		}
	}

	double NLL = 0;
	if (isBkg)
	{
		NLL = bkgPdf.createNLL(*data)->getVal();
		return NLL;
	}
	else
	{
		data->plotOn(MyFrame);
		totalPdf.plotOn(MyFrame, LineColor(kRed));
		legend0->AddEntry(MyFrame->findObject("data"), "RunII 2018 Data", "L");
		legend0->AddEntry(MyFrame->findObject("totalPdf"), "Fit Result", "L");
		TCanvas c;
		MyFrame->Draw();
		std::cout << i << " " << Data_edm << " " << Data_status << " " << Data_covQual << std::endl;
		c.SaveAs("significance.pdf");
		NLL = totalPdf.createNLL(*data)->getVal();
		return NLL;
	}
}

void Significance(TString path, std::string cut="")
{
	TChain *SourceTree = GetTree<TChain>(path, cut);
	if (!SourceTree->GetEntries()) {
		delete SourceTree;
		throw std::runtime_error("Error: Fail to read data tree");
	}
	RooRealVar Psi2S_mass("Psi2S_mass", "M(#mu#mu#pi#pi)-M(#mu#mu)+3.0969 GeV", RANGE_MIN, RANGE_MAX);
	Psi2S_mass.setRange("fullRange", RANGE_MIN, RANGE_MAX);
	RooDataSet *data = new RooDataSet("data", "#psi(2S)(#mu#mu#pi#pi) Ivariate Mass", RooArgSet(Psi2S_mass), RooFit::Import(*SourceTree));
	unsigned int Nnorm = data->sumEntries();
	double NLL_H0 = Fit(data, true, Nnorm);
	double NLL_H1 = Fit(data, false, Nnorm);
	unsigned int ndf = 4;
	double pvalue = TMath::Prob(2 * fabs(NLL_H0 - NLL_H1), ndf);
	std::cout << "p-value: " << pvalue << "  Significance (sigma): " << RooStats::PValueToSignificance(pvalue / 2.0) << "  NLL_H0: " << NLL_H0 << "  NLL_H1: " << NLL_H1 << std::endl;
	delete SourceTree;
}
