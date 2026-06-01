#include "X3872Utils.h"
#include <TString.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1F.h>

void Draw_(TString dataPath = "roots/", TString mcPath = "proots/", TString varListFile = "varlist.txt", TString output_dir = "plots")
{
    // 读取变量列表
    std::vector<std::string> variables;
    std::ifstream varFile(varListFile.Data());
    if (!varFile.is_open()) {
        std::cerr << "[ERROR] Cannot open variable list file: " << varListFile << std::endl;
        return;
    }

    std::string varName;
    while (std::getline(varFile, varName)) {
        if (!varName.empty()) {
            variables.push_back(varName);
        }
    }
    varFile.close();

    std::cout << "[INFO] Loaded " << variables.size() << " variables" << std::endl;

    // 获取数据的 TTreeProcessorMT（用于 sideband 分析）
    std::cout << "[INFO] Loading data from: " << dataPath << std::endl;
    TString cut_peak = TString(PeakSelectionCut);
    TString cut_side = TString(SideBandSelectionCut);
    
    ROOT::TTreeProcessorMT *tp_data_peak = GetTree<ROOT::TTreeProcessorMT>(dataPath, cut_peak, "peak.root");
    ROOT::TTreeProcessorMT *tp_data_side = GetTree<ROOT::TTreeProcessorMT>(dataPath, cut_side, "side.root");
    if (tp_data_peak == nullptr || tp_data_side == nullptr) {
        std::cerr << "[ERROR] Failed to load data trees" << std::endl;
        return;
    }

    // 获取 MC 的 TTreeProcessorMT
    std::cout << "[INFO] Loading MC from: " << mcPath << std::endl;
    ROOT::TTreeProcessorMT *tp_mc = GetTree<ROOT::TTreeProcessorMT>(mcPath, "", "mc.root");
    if (tp_mc == nullptr) {
        std::cerr << "[ERROR] Failed to load MC tree" << std::endl;
        return;
    }
    // 为每个变量绘制分布
    for (const auto& var : variables) {
        std::cout << "[INFO] Processing variable: " << var << std::endl;

        // 使用相同的 TTreeProcessorMT 指针来绘制，避免重复调用 GetTree
        auto hists = DrawSideBand(tp_data_peak, tp_data_side, var.c_str(), var.c_str(), 0, 0);
        auto hist_sig = hists.first;    // 信号分布 (peak - scaled side)
        auto hist_bkg = hists.second;   // 背景分布 (side)
        
        // 获取 MC 分布
        auto hist_mc = DrawGraph(tp_mc, var.c_str(), var.c_str(), 0, 0);

        // 归一化到相同积分
        hist_sig->Scale(1.0 / hist_sig->Integral());
        hist_bkg->Scale(1.0 / hist_bkg->Integral());
        hist_mc->Scale(1.0 / hist_mc->Integral());

        // 设置样式
        hist_sig->SetLineColor(kRed);
        hist_sig->SetLineWidth(2);
        hist_sig->SetLineStyle(1);
        hist_sig->SetStats(kFALSE);

        hist_bkg->SetLineColor(kBlue);
        hist_bkg->SetLineWidth(2);
        hist_bkg->SetLineStyle(2);
        hist_bkg->SetStats(kFALSE);

        hist_mc->SetLineColor(kGreen);
        hist_mc->SetLineWidth(2);
        hist_mc->SetLineStyle(3);
        hist_mc->SetStats(kFALSE);

        // 使用新函数设置 Y 轴范围，确保所有图都能完整显示
        SetYAxisRangeForAll(hist_sig.get(), hist_bkg.get(), hist_mc.get());

        // 创建画布和图例
        TCanvas *canvas = new TCanvas(("canvas_" + var).c_str(), var.c_str(), 800, 600);
        canvas->SetGrid();

        // 绘制直方图
        hist_sig->Draw("HIST");
        hist_bkg->Draw("HIST SAME");
        hist_mc->Draw("HIST SAME");

        // 设置标题和坐标轴
        hist_sig->SetTitle(("Distribution of " + var).c_str());
        hist_sig->GetXaxis()->SetTitle(var.c_str());
        hist_sig->GetYaxis()->SetTitle("Normalized Events");

        // 创建图例
        TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9);
        legend->AddEntry(hist_sig.get(), "Signal (Data)", "l");
        legend->AddEntry(hist_bkg.get(), "Background (Data)", "l");
        legend->AddEntry(hist_mc.get(), "MC", "l");
        legend->Draw();

        // 保存图片
        std::cout << "[INFO] Saving plots for: " << var << std::endl;
        TString varStr(var.c_str());
        TString plot_path_pdf = output_dir + "/" + varStr + "_dist.pdf";
        canvas->SaveAs(plot_path_pdf.Data());

        // 清理
        delete canvas;
        delete legend;

        std::cout << "[INFO] Saved plots for: " << var << std::endl;
    }

    // 清理资源
    delete tp_data_peak;
    delete tp_data_side;
    delete tp_mc;
    
    std::remove("peak.root");
    std::remove("side.root");
    std::remove("mc.root");

    std::cout << "[INFO] All plots generated successfully!" << std::endl;
}

void Draw()
{
    Draw_("roots/", "proots/", "varlist.txt", "plots");
}