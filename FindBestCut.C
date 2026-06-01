#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include "TLegend.h"
#include "CutOptimization.C"

struct CutResult {
    std::string filename;
    std::string cut;
    double merit;
    double eff;
    CutResult(const std::string& f, const std::string& c, double m, double e)
        : filename(f), cut(c), merit(m), eff(e) {}
};

const int N_TOP_DEVIATION = 10;

std::vector<CutResult> LoadCutResults(const std::string& filename) {
    std::vector<CutResult> results;
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    int line_num = 0;
    while (std::getline(infile, line)) {
        line_num++;
        if (line.empty()) continue;

        std::vector<std::string> tokens = split(line, ',');
        if (tokens.size() != 4) {
            throw std::runtime_error("Line " + std::to_string(line_num) + ": expected 4 fields, got " + std::to_string(tokens.size()));
        }

        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i].empty()) {
                throw std::runtime_error("Line " + std::to_string(line_num) + ": field " + std::to_string(i) + " is empty");
            }
        }

        results.emplace_back(tokens[0], tokens[1], std::stod(tokens[2]), std::stod(tokens[3]));
    }

    infile.close();
    return results;
}

std::map<std::string, std::vector<CutResult>> GroupByFilename(const std::vector<CutResult>& results) {
    std::map<std::string, std::vector<CutResult>> grouped;
    for (const auto& cr : results) {
        grouped[cr.filename].push_back(cr);
    }
    return grouped;
}

std::vector<std::pair<double, std::string>> FindTopDeviations(const std::vector<CutResult>& group) {
    if (group.size() < N_TOP_DEVIATION) {
        std::cerr << "[WARNING] Group " << group[0].filename << " has less than " << N_TOP_DEVIATION << " points, skipping fitting" << std::endl;
        return {};
    }


    TGraph* gr = new TGraph(group.size());
    for (size_t i = 0; i < group.size(); ++i) {
        gr->SetPoint(i, group[i].eff, group[i].merit);
    }

    TF1* f = new TF1("linear", "pol1", 0, 1);
    gr->Fit(f, "Q");

    double p0 = f->GetParameter(0);
    double p1 = f->GetParameter(1);

    std::vector<std::pair<double, std::string>> deviations;
    for (const auto& cr : group) {
        double expected = p0 + p1 * cr.eff;
        double deviation = cr.merit - expected;
        deviations.push_back({deviation, cr.cut});
    }

    std::sort(deviations.begin(), deviations.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    if (deviations.size() > static_cast<size_t>(N_TOP_DEVIATION)) {
        deviations.resize(N_TOP_DEVIATION);
    }

    TCanvas* c = new TCanvas("c", "Merit vs Efficiency", 800, 600);
    c->SetGrid();
    
    gr->SetTitle("");
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kBlue);
    gr->SetMarkerSize(1.2);
    gr->SetLineColor(kBlue);
    gr->Draw("AP");
    
    gr->GetXaxis()->SetTitle("Efficiency");
    gr->GetXaxis()->SetTitleSize(0.045);
    gr->GetXaxis()->SetLabelSize(0.04);
    gr->GetXaxis()->SetRangeUser(0, 1);
    
    gr->GetYaxis()->SetTitle("Merit");
    gr->GetYaxis()->SetTitleSize(0.045);
    gr->GetYaxis()->SetLabelSize(0.04);
    
    f->SetLineColor(kRed);
    f->SetLineWidth(2);
    f->SetLineStyle(kDashed);
    f->Draw("same");
    
    TLegend* legend = new TLegend(0.65, 0.75, 0.9, 0.9);
    legend->AddEntry(gr, "Data points", "p");
    legend->AddEntry(f, Form("Fit: y = %.2f + %.2fx", p0, p1), "l");
    legend->Draw();
    
    std::string filename = group[0].filename;
    size_t lastSlash = filename.find_last_of("/");
    if (lastSlash != std::string::npos) {
        filename = filename.substr(lastSlash + 1);
    }
    size_t dotPos = filename.find_last_of(".");
    if (dotPos != std::string::npos) {
        filename = filename.substr(0, dotPos);
    }
    filename += "_merit_vs_eff.pdf";
    c->SaveAs(filename.c_str());
    
    //delete legend;
    delete c;
    delete gr;
    delete f;

    return deviations;
}

void FindBestCut() {
    std::string inputfile = "cut_optimization_results.txt";
    auto results = LoadCutResults(inputfile);
    std::cout << "[INFO] Loaded " << results.size() << " cut results" << std::endl;

    auto grouped = GroupByFilename(results);
    std::cout << "[INFO] Grouped into " << grouped.size() << " files" << std::endl;

    std::map<std::string, std::vector<std::string>> outputGroups;
    std::ofstream outfile("best_cut.o");

    for (const auto& [filename, cuts] : grouped) {
        std::cout << "\n[INFO] Processing: " << filename << " (" << cuts.size() << " cuts)" << std::endl;
        auto topDeviations = FindTopDeviations(cuts);

        if (topDeviations.empty()) continue;

        std::cout << "[INFO] Top " << N_TOP_DEVIATION << " deviations for " << filename << ":" << std::endl;
        for (const auto& [dev, cut] : topDeviations) {
            std::cout << "  deviation: " << dev << ", cut: " << cut << std::endl;
        }

        outputGroups[filename] = std::vector<std::string>();
        for (const auto& [dev, cut] : topDeviations) {
            outputGroups[filename].push_back(cut);
        }
    }

    std::cout << "\n========== FINAL RESULTS ==========" << std::endl;
    for (const auto& [filename, cuts] : outputGroups) {
        std::cout << "\n" << filename << ":" << std::endl;
        
        TString combinedCut = "";
        for (size_t i = 0; i < cuts.size(); ++i) {
            if (i > 0) combinedCut += " && ";
            combinedCut += cuts[i].c_str();
        }
        
        DrawAsPdf(filename.c_str(), "Psi2S_mass", "#psi(2S) Mass Distribution", 3.3, 4, combinedCut, 100);
        
        std::string pdfName = filename;
        size_t lastSlash = pdfName.find_last_of("/");
        if (lastSlash != std::string::npos) {
            pdfName = pdfName.substr(lastSlash + 1);
        }
        size_t dotPos = pdfName.find_last_of(".");
        if (dotPos != std::string::npos) {
            pdfName = pdfName.substr(0, dotPos);
        }
        pdfName += "_Psi2S_mass.pdf";
        std::rename("Psi2S_mass.pdf", pdfName.c_str());
        
        TString combinedCut_Jpsi = combinedCut + " && Jpsi2_hasJConstraintFit == 1";
        DrawAsPdf(filename.c_str(), "X_PJ_mass", "X(#psi+J/#psi) Mass Distribution", 6, 9, combinedCut_Jpsi, 100);
        pdfName = filename;
        lastSlash = pdfName.find_last_of("/");
        if (lastSlash != std::string::npos) {
            pdfName = pdfName.substr(lastSlash + 1);
        }
        dotPos = pdfName.find_last_of(".");
        if (dotPos != std::string::npos) {
            pdfName = pdfName.substr(0, dotPos);
        }
        pdfName += "_X_PJ_mass.pdf";
        std::rename("X_PJ_mass.pdf", pdfName.c_str());
        std::cout << "\033[32m" << combinedCut << "\033[0m" << std::endl;
        outfile << filename << "," << combinedCut << std::endl;
    }
    outfile.close();
}