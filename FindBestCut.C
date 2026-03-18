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

#define PSI2S_MASS_MIN 3.6
#define PSI2S_MASS_MAX 3.76
#define NUM_CUT_ITEMS 3

std::vector<std::string> split(const std::string &str, const char delimiter = ',')
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}
class CutPoint
{
public:
    std::string year;
    std::set<std::string> cuts;
    double merit;
    double efficiency;

public:
    CutPoint(std::string line, bool from_file = true);
    CutPoint(std::string year, std::set<std::string> cuts_, double merit, double efficiency) : year(year), merit(merit), efficiency(efficiency)
    {
        cuts.insert(cuts_.begin(), cuts_.end());
    }
    CutPoint operator+(const CutPoint &other) const;
    static std::vector<CutPoint> combine_points(std::vector<CutPoint> cuts1, std::vector<CutPoint> cuts2);
    void PrintCuts();
    RooDataSet *CuttedData(TChain &tree);
};
CutPoint::CutPoint(std::string line, bool from_file = true)
{
    if (from_file)
    {
        // read from a result file
        // line format: path, cut, merit, efficiency
        // path format: prefix/year
        // cut format: min<variable<max
        auto tokens = split(line);
        merit = std::stod(tokens[2]);
        efficiency = std::stod(tokens[3]);
        year = split(tokens[0], '/').back();
        cuts.insert(tokens[1]);
    }
    else
    {
        // read from a string to generate cut points with cuts only
        // line format: cut1, cut2, ... cutN
        // cut format: number < var or number > var
        merit = -1;
        efficiency = -1;
        year = "unknown";
        auto tokens = split(line);
        for (const auto &c : tokens)
            cuts.insert(c);
    }
}
CutPoint CutPoint::operator+(const CutPoint &other) const
{
    if (year != other.year)
        throw std::runtime_error("Trying to combine cuts from different years!");
    auto new_cuts(cuts);
    new_cuts.insert(other.cuts.begin(), other.cuts.end());
    return CutPoint(year, new_cuts, merit + other.merit, efficiency * other.efficiency);
}
std::vector<CutPoint> CutPoint::combine_points(std::vector<CutPoint> cuts1, std::vector<CutPoint> cuts2)
{
    std::vector<CutPoint> result;
    for (auto &cut1 : cuts1)
    {
        for (auto &cut2 : cuts2)
        {
            if (cut1.cuts == cut2.cuts)
                continue;
            CutPoint combined = cut1 + cut2;
            if (combined.efficiency >= 0.1)
                result.push_back(combined);
        }
    }
    return result;
}

void CutPoint::PrintCuts()
{
    for (const auto &cut : cuts)
        std::cout << "Cut: " << cut << "\n";
    std::cout << " Merit: " << merit << " Eff: " << efficiency << std::endl;
}

RooDataSet *CutPoint::CuttedData(TChain &tree)
{
    RooRealVar Psi2S_mass("Psi2S_mass", "M(#mu#mu#pi#pi)-M(#mu#mu)+3.0969 GeV", PSI2S_MASS_MIN, PSI2S_MASS_MAX);
    // prase varibales and cuts
    RooArgSet variables;
    variables.addClone(Psi2S_mass);
    std::set<std::string> vars;
    TString final_cut = "true";
    for (const auto &cut : cuts)
    {
        TString variable;
        auto cut_parse = split(cut, '<');
        if (cut_parse.size() == 1)
            cut_parse = split(cut, '>');
        if (cut_parse.size() == 1)
            throw std::runtime_error("no cuts parsed");
        vars.insert(cut_parse[1]);
        final_cut += TString(" && " + cut);
    }
    for (const auto &var : vars)
    {
        RooRealVar roovar(var.c_str(), var.c_str(), -std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
        variables.addClone(roovar);
    }
    RooDataSet *data = new RooDataSet("data", "Psi2S_mass", variables, RooFit::Cut(final_cut), RooFit::Import(tree));
    return data;
}

double SumFront(const std::vector<CutPoint> &cuts, unsigned int n)
{
    double result = 0;
    for (size_t i = 0; i < n; i++)
    {
        result += cuts.at(i).merit;
    }
    return result;
}
double SumBack(const std::vector<CutPoint> &cuts, unsigned int n)
{
    double result = 0;
    unsigned int size = cuts.size() - 1;
    for (size_t i = 0; i < n; i++)
    {
        result += cuts.at(size - i).merit;
    }
    return result;
}

void FindBestCut()
{
    // Read from cut_optimization_results.txt, generate cutpoints
    std::ifstream in("/home/storage0/users/junkaiqin/X3872Analysis/cut_optimization_results.txt");
    std::string line;
    std::vector<CutPoint> cutPoints;
    while (std::getline(in, line))
    {
        cutPoints.emplace_back(line);
    }
    // Group by year
    std::map<std::string, std::vector<CutPoint>> cutPointsByYear;
    for (const auto &cp : cutPoints)
    {
        cutPointsByYear[cp.year].push_back(cp);
    }
    // For each year, find the best cut combination
    for (auto &entry : cutPointsByYear)
    {
        auto &year = entry.first;
        auto &points = entry.second;
        auto cuts = std::vector<CutPoint>(points);
        auto &bestcuts = cuts;
        auto it = std::remove_if(cuts.begin(), cuts.end(), [](const CutPoint &c)
                                 { return c.efficiency < 0.1; });
        cuts.erase(it, cuts.end());
        if (!cuts.size())
            bestcuts = points;
        if (!cuts.size())
            std::cout << "no cuts\n";
        std::sort(cuts.begin(), cuts.end(), [](const CutPoint &c1, const CutPoint &c2)
                  { return c1.efficiency > c2.efficiency; });
        unsigned int N = 0;
        double eff = 1;
        double bestEff = 1;
        for (size_t i = 0; i < cuts.size(); i++)
        {
            eff *= cuts.at(i).efficiency;
            std::cout << "this " << cuts.at(i).efficiency << " total " << eff << std::endl;
            if (eff < 0.1 && N == 0)
            {
                N = i + 1;
                bestEff = eff;
            }
        }
        std::cout << "most eff with " << bestEff << " size " << N << std::endl;
        std::cout << "cuts size " << cuts.size() << std::endl;
        std::sort(cuts.begin(), cuts.end(), [](const CutPoint &c1, const CutPoint &c2)
                  { return c1.merit > c2.merit; });
        for (size_t i = 1; i <= N; i++)
        {
            std::cout << "N " << i << " with max meirt " << SumFront(cuts, i) << " min merit " << SumBack(cuts, i) << std::endl;
        }

        /*auto combined = cuts;
        for (size_t i = 0; i < NUM_CUT_ITEMS; i++)
        {
            bestcuts = combined;
            combined = CutPoint::combine_points(combined, cuts);
            if (!combined.size())
            {
                std::cout << "no cuts\n";
                break;
            }
        }
        std::cout << "best cut for " << year << " is " << std::endl;
        for (auto &p : bestcuts)
        {
            p.print_cuts();
            std::cout << "//////////////////////////\n";
        }*/
    }
}
