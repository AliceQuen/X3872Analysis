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

#define NUM_CUT_ITEMS 3

#include "CutOpt.C"
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
    CutPoint(std::string line);
    CutPoint(std::string year, std::set<std::string> cuts_, double merit, double efficiency) : year(year), merit(merit), efficiency(efficiency)
    {
        cuts.insert(cuts_.begin(), cuts_.end());
    }
    CutPoint operator+(const CutPoint &other) const;
    static std::vector<CutPoint> combine_points(std::vector<CutPoint> cuts1, std::vector<CutPoint> cuts2);
    void print_cuts();
};
CutPoint::CutPoint(std::string line)
{
    // line format: path, cut, merit, efficiency
    // path format: prefix/year
    // cut format: min<variable<max
    auto tokens = split(line);
    merit = std::stod(tokens[2]);
    efficiency = std::stod(tokens[3]);
    year = split(tokens[0], '/').back();
    cuts.insert(tokens[1]);
}
CutPoint CutPoint::operator+(const CutPoint &other) const
{
    if (year != other.year)
        throw std::runtime_error("Trying to combine cuts from different years!");
    return CutPoint(year, other.cuts, merit + other.merit, efficiency * other.efficiency);
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
            if (combined.efficiency >=1e3)
                result.push_back(combined);
        }
    }
    return result;
}

void CutPoint::print_cuts()
{
    for (const auto &cut : cuts)
    {
        std::cout << cut << std::endl;
    }
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
        auto it = std::remove_if(cuts.begin(), cuts.end(), [](CutPoint r)
                                 { return r.efficiency < 1e3; });
        cuts.erase(it, cuts.end());
        if (!cuts.size()) bestcuts = points;
        for (size_t i = 0; i < NUM_CUT_ITEMS; i++)
        {
            auto combined = cuts;
            bestcuts = combined;
            auto combined = CutPoint::combine_points(combined, cuts);
            if (!combined.size())
                break; 
        }
        std::cout << "best cut for " << year << " is " <<std::endl;
        for (auto &p : bestcuts)
            p.print_cuts();
    }
}
