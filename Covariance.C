#include "X3872Utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "TString.h"
#include "TCanvas.h"
#include "TPaletteAxis.h"
#include "TGaxis.h"

void Covariance(TString dataPath = "roots/2017_merged.root")
{
    std::string varListFile = "varlist.txt";
    std::vector<std::string> variables;

    std::ifstream inFile(varListFile);
    if (!inFile.good()) {
        std::cerr << "[ERROR] varlist.txt not found!" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        if (!line.empty()) {
            variables.push_back(line);
        }
    }
    inFile.close();

    if (variables.empty()) {
        std::cerr << "[ERROR] No variables found in varlist.txt" << std::endl;
        return;
    }

    std::cout << "[INFO] Loaded " << variables.size() << " variables" << std::endl;

    ROOT::TTreeProcessorMT *tp = GetTree<ROOT::TTreeProcessorMT>(dataPath, "");
    if (!tp) {
        std::cerr << "[ERROR] Failed to get TTreeProcessorMT" << std::endl;
        return;
    }
    std::cout << "[INFO] TreeProcessorMT created" << std::endl;

    int nVars = variables.size();

    TH2F* corrMatrix = new TH2F("corrMatrix", "Correlation Matrix", 
                                 nVars, 0, nVars, nVars, 0, nVars);

    TGraph* corrWithMass = new TGraph(nVars);

    for (int i = 0; i < nVars; ++i) {
        corrMatrix->GetXaxis()->SetBinLabel(i + 1, variables[i].c_str());
        corrMatrix->GetYaxis()->SetBinLabel(i + 1, variables[i].c_str());
    }

    for (int i = 0; i < nVars; ++i) {
        for (int j = i; j < nVars; ++j) {
            auto cov = Statistics(tp, variables[i].c_str(), variables[j].c_str());
            double corr = cov.correlation;
            corrMatrix->SetBinContent(i + 1, j + 1, corr);
            if (i != j) {
                corrMatrix->SetBinContent(j + 1, i + 1, corr);
            }
        }

        auto covMass = Statistics(tp, variables[i].c_str(), "Psi2S_mass");
        corrWithMass->SetPoint(i, i + 0.5, covMass.correlation);
    }

    TCanvas* c1 = new TCanvas("c1", "Correlation Matrix", 800, 800);
    c1->SetGrid();
    c1->SetBottomMargin(0.2);

    
    corrMatrix->SetStats(0);
    corrMatrix->GetXaxis()->LabelsOption("v");
    corrMatrix->GetZaxis()->SetRangeUser(-1, 1);
    corrMatrix->GetXaxis()->SetLabelSize(0.018);
    corrMatrix->GetXaxis()->SetLabelOffset(0.015);
    corrMatrix->GetYaxis()->SetLabelSize(0.018);
    corrMatrix->Draw("COLZ");
    
    TPaletteAxis *palette = (TPaletteAxis*)corrMatrix->GetListOfFunctions()->FindObject("palette");
    if (palette) {
        palette->SetX2NDC(0.92);
        palette->Draw();
    }
    
    c1->SaveAs("correlation_matrix.pdf");

    TCanvas* c2 = new TCanvas("c2", "Correlation with Psi2S_mass", 1000, 600);
    
    c2->SetGrid();
    c2->SetBottomMargin(0.25);
    
    TH1F* frame = new TH1F("frame", "Correlation with Psi2S_mass", nVars, -0.5, nVars - 0.5);
    frame->SetStats(0);
    frame->GetXaxis()->SetTitle("Variables");
    frame->GetXaxis()->SetLabelSize(0.018);
    frame->GetXaxis()->SetLabelOffset(0.01);
    for (int i = 0; i < nVars; ++i) {
        frame->GetXaxis()->SetBinLabel(i + 1, variables[i].c_str());
    }
    frame->SetTitleOffset(2);
    frame->GetXaxis()->LabelsOption("v");
    frame->GetYaxis()->SetRangeUser(-1, 1);
    frame->GetYaxis()->SetTitle("Correlation");
    frame->Draw();

    
    corrWithMass->SetMarkerStyle(20);
    corrWithMass->GetXaxis()->LabelsOption("v");
    corrWithMass->SetMarkerColor(kBlue);
    corrWithMass->SetMarkerSize(1.2);
    corrWithMass->SetLineColor(kBlue);
    corrWithMass->SetLineWidth(3);
    corrWithMass->Draw("P SAME");
    
    c2->SaveAs("correlation_with_mass.pdf");
    delete frame;

    delete tp;
    delete corrMatrix;
    delete corrWithMass;
    delete c1;
    delete c2;

    std::cout << "[INFO] Correlation analysis completed!" << std::endl;
    std::cout << "[INFO] Output files: correlation_matrix.pdf, correlation_with_mass.pdf" << std::endl;
}
