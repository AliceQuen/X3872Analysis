// This is script for loading hists in condorOutput
// and connect them

#include "TH1F.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TList.h"
#include <vector>
#include <iostream>
#include <string>
#include <dirent.h>
#include <functional>

// type 1:cut max, type 0:cut min
struct Extremum
{
	double min;
	double max;
	double ext;
};
Extremum FindExtremum(std::vector<double> &vec, int i)
{
	unsigned int n = vec.size();
	double l = vec.at(i);
	double m = vec.at(i);
	double r = vec.at(i);
	int lj = i;
	int rj = i;
	bool skip = false;
	unsigned int ls = 0;
	unsigned int rs = 0;
	do
	{
		if (!skip) l = vec.at(lj);
		lj = lj + 1;
		if (lj >= n) break;
		if (vec.at(lj) > l && vec.at(lj) - l > 0.1) break;
	/*	if (vec.at(lj) > l && vec.at(lj) - l > 0.1)
		{
			if (skip) break;
			else
			{
				ls++;
				skip = true;	
			}
		}else
		{
			skip = false;
		}*/
	}while (true);
	// if (lj - 2 >= 0) lj = lj - 2;
	lj = lj - 1;
	skip =  false;
	do
	{
		if (!skip) r = vec.at(rj);
		rj = rj - 1;
		if (rj < 0) break;
		if (vec.at(rj) > r && vec.at(rj) - r > 0.1) break;
	/*	if (vec.at(rj) > r && vec.at(rj) - r > 0.1)
		{
			if (skip) break;
			else
			{
				rs++;
				skip = true;	
			}
		}else
		{
			skip = false;
		}*/
	}while (true);
	rj = rj + 1;
	// if (rj + 2 < n) rj = rj + 2;
	Extremum result;
//	if (abs(lj - i) - ls + 1 > 2 && abs(rj - i) - rs + 1 > 2)
	
	double max_ = vec.at(rj);
	double min_ = vec.at(lj); 
	for (int j = rj; j <= lj; j++)
	{
		if (vec.at(j) > max_) max_ = vec.at(j);
		if (vec.at(j) < min_) min_ = vec.at(j);
	}
	if (abs(lj - i) > 1 && abs(rj - i) > 1 && fabs(max_ - min_) > 1 && fabs(max_ - vec.at(rj)) > 0.5 && fabs(max_ - vec.at(lj)) > 0.5)
	{
		result.min = rj;
		result.max = lj;
		result.ext = m;
	}else
	{
		result.min = -1;
		result.max = -1;
		result.ext = m;
	}
	return result;
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
template<class get>
double kMovingAverage(unsigned int n, unsigned int i, unsigned int k, get p)
{
	if (k % 2 == 0 || k >= n || k == 1) return p(i);
	unsigned int k1 = (k - 1) / 2; 
	unsigned int t = 0;
	double mid;
	std::vector<double> m;
	for (unsigned int j = 0; j <= k1; j++)
	{
		m.push_back(p(i - j));
		if (!(i - j)) break;	
	}
	for (unsigned int j = 0; j <= k1; j++)
	{
		if (j != 0) m.push_back(p(i + j));
		if ((i + j) == n - 1) break;	
	}
	std::sort(m.begin(), m.end());
	mid = m.at(k1);
	std::vector<double> ad;
	for (auto mi : m) ad.push_back(fabs(mi - mid));
	std::sort(ad.begin(), ad.end());
	double mad = ad.at(k1);
	for (auto &mi : m)
	{
		if (mi < mid - 5 * mad) mi = mid;
		if (mi > mid + 5 * mad) mi = mid;
	}
	return std::accumulate(m.begin(), m.end(), 0.0) / m.size();
}
int FindPeak()
{
	auto start = std::chrono::high_resolution_clock::now();
//	TChain SourceTree("SourceTree", "");
//	SourceTree.Add("/home/storage0/users/junkaiqin/Ntuple-Processor/Output/MakeSourceTree_CutOpt_23_v1_Output/*.root");

	std::ifstream landscape("result.o");
	std::string line;
	std::istringstream iss(line);
	std::string token;
	unsigned int i;
	std::vector<std::pair<double, double>> points;
	std::getline(landscape, line);	
	while (std::getline(landscape, line))
	{
			
		iss.str(line);
		iss.clear();
		i = 0;
		double cut, merit;
		while (std::getline(iss, token, ' '))
		{
			switch (i)
			{
				case 2:
					cut = std::stod(token);
					break;
				case 3:
					merit = std::stod(token);
					break;
			}
			i++;
		}		
		points.push_back({cut, merit});
	}
	unsigned int n = points.size();
	std::sort(points.begin(), points.end());
	std::vector<double> meritk;
	for (auto o = 0; o < n; o++) meritk.push_back(SavitzkyGolay(n, o, 9, [&points](unsigned int t) -> double { return points.at(t).second; }));
	for (auto o = 0; o < n; o++) std::cout << points.at(o).first << " " << meritk.at(o) << std::endl;
	
	std::vector<int> id(n);
	std::iota(id.begin(), id.end(), 0);
	ROOT::TThreadExecutor pool;
	std::cout << "pool size "<< pool.GetPoolSize() << std::endl;
	auto extremums = pool.Map([&meritk](unsigned int i) { return FindExtremum(meritk, i); }, id);
	auto it = std::remove_if(extremums.begin(), extremums.end(), [](Extremum r) -> bool { return (r.min < 0 || r.max < 0); });
	extremums.erase(it, extremums.end());
	bool re_x = false;
	bool re_y = false;
	for (auto x = extremums.begin(); x != extremums.end();)
	{
		re_x = false;
		for(auto y = std::next(x); y != extremums.end();)
		{
			re_y = false;	
			if ((*x).min == (*y).min && (*x).max == (*y).max)
			{
				if ((*x).ext > (*y).ext)
				{
					extremums.erase(y);
					re_y = true;
				}else
				{
					extremums.erase(x);
					re_x = true;
					break;
				}	
			}
			if (!re_y) y++;
		}
		if (!re_x) x++;
	}
	std::sort(extremums.begin(), extremums.end(), [](Extremum a, Extremum b) { return a.ext > b.ext; });
	TCanvas c("c", "c");
	TGraph g(n);
	for (i = 0; i < n; i++ ) g.SetPoint(i, points.at(i).first, meritk.at(i));
	g.SetLineColor(kBlue);
	g.SetLineWidth(4);
	TMultiGraph *mg = new TMultiGraph();
	mg->Add(&g);
	for (auto x : extremums)
	{
		std::cout << "min " << points.at(x.min).first << " max " << points.at(x.max).first << " ext " << x.ext << std::endl;
		TGraph *g_ = new TGraph();
		g_->Sort();
		g_->SetLineColor(2);
        	g_->SetLineWidth(1504);
        	//g_->SetFillStyle(3005);
        	//g_->SetFillColor(9);
		for (auto j = x.min; j <= x.max; j++){
			std::cout << points.at(j).first << " " << meritk.at(j) << std::endl;
			 g_->AddPoint(points.at(j).first, meritk.at(j));
		}
		mg->Add(g_);
	}
	mg->Draw("A");
	c.SaveAs("p.png");
	
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> ms = end - start;
	std::cout << "Time used " << ms.count() << " ms" << std::endl;
	return 0;
}
