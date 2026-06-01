#include "TROOT.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TStyle.h"

#include "RooRealVar.h"
#include "RooFormulaVar.h"
#include "RooStats/SPlot.h"
#include "RooDataSet.h"
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
#include <RooGlobalFunc.h>
#include <RooProdPdf.h>
#include <RtypesCore.h>
#include <TGraph.h>
#include <cstdio>

#define PSI2S_MASS_MIN 3.6
#define PSI2S_MASS_MAX 3.8
#define JPSI_MASS_MIN 2.9469
#define JPSI_MASS_MAX 3.2469

struct FitResult2D {
    double significance;
    double n_sig;
    double n_sig_err;
    double n_bkg_psi2s;
    double n_bkg_psi2s_err;
    double n_bkg_jpsi;
    double n_bkg_jpsi_err;
    double n_bkg_both;
    double n_bkg_both_err;
    double n_combinatorial;
    double purity;
    double psi2s_sigma_val;
    double psi2s_sigma_err;
    double jpsi_sigma_val;
    double jpsi_sigma_err;
    Long64_t entries;
    int iterations;
    int status;
};

// 2D 拟合函数：对 Psi(2S) 和 J/psi 进行二维拟合
FitResult2D Fit2D(RooDataSet *data, bool isDraw = false)
{
    // 定义拟合变量
    RooRealVar Psi2S_mass("Psi2S_mass", "Psi(2S) mass", PSI2S_MASS_MIN, PSI2S_MASS_MAX);
    RooRealVar Jpsi2_mass("Jpsi2_mass", "J/psi mass", JPSI_MASS_MIN, JPSI_MASS_MAX);

    // ==================== 1. Psi(2S) Crystal Ball 参数 ====================
    // CB(M): Crystal Ball function for Psi(2S)
    RooRealVar psi2s_mean("psi2s_mean", "Psi(2S) mean", 3.686, 3.66, 3.72);
    RooRealVar psi2s_sigma("psi2s_sigma", "Psi(2S) sigma", 0.006, 0.0, 0.05);
    RooRealVar psi2s_alpha("psi2s_alpha", "Psi(2S) alpha", 4.3, 0.1, 5.0);
    RooRealVar psi2s_n("psi2s_n", "Psi(2S) n", 0.006, -40, 135);
    RooCBShape cb_psi2s("cb_psi2s", "Crystal Ball Psi(2S)", Psi2S_mass, psi2s_mean, psi2s_sigma, psi2s_alpha, psi2s_n);

    // ==================== 2. J/psi Crystal Ball 参数 ====================
    // CB(M): Crystal Ball function for J/psi
    RooRealVar jpsi_mean("jpsi_mean", "J/psi mean", 3.097, 3.05, 3.15);
    RooRealVar jpsi_sigma("jpsi_sigma", "J/psi sigma", 0.03, 0.0, 0.08);
    RooRealVar jpsi_alpha("jpsi_alpha", "J/psi alpha", 1.35, 0.1, 5.0);
    RooRealVar jpsi_n("jpsi_n", "J/psi n", 130, -40, 200);
    RooCBShape cb_jpsi("cb_jpsi", "Crystal Ball J/psi", Jpsi2_mass, jpsi_mean, jpsi_sigma, jpsi_alpha, jpsi_n);

    // ==================== 3. 背景多项式参数 ====================
    // Poly1(M): Second-order Chebyshev polynomial (用于 J/psi 背景)
    RooRealVar poly1_c0("poly1_c0", "Poly1 c0", -0.2, -10.0, 10.0);
    RooRealVar poly1_c1("poly1_c1", "Poly1 c1", -0.44, -10.0, 10.0);
    RooRealVar poly1_c2("poly1_c2", "Poly1 c2", 0.021, -10.0, 10.0);
    RooChebychev poly1_jpsi("poly1_jpsi", "Poly1 for J/psi", Jpsi2_mass, RooArgList(poly1_c0, poly1_c1, poly1_c2));

    // Poly2(M): Third-order Chebyshev polynomial (用于 Psi(2S) 背景)
    RooRealVar poly2_c0("poly2_c0", "Poly2 c0", 0.06, -10, 10.0);
    RooRealVar poly2_c1("poly2_c1", "Poly2 c1", -0.055, -10.0, 10.0);
    RooRealVar poly2_c2("poly2_c2", "Poly2 c2", 0.012, -10.0, 10.0);
    RooRealVar poly2_c3("poly2_c3", "Poly2 c3", 0.0025, -10.0, 10.0);
    RooChebychev poly2_psi2s("poly2_psi2s", "Poly2 for Psi(2S)", Psi2S_mass, RooArgList(poly2_c0, poly2_c1, poly2_c2, poly2_c3));

    // ==================== 4. 2D PDF 分量 ====================
    // PDF = N_psi2sJpsi * [CB(psi2s) * CB(jpsi)]                        - 信号
    //     + N_psi2sbkg * [CB(psi2s) * Poly1(jpsi)]                      - Psi(2S)信号 + J/psi背景
    //     + N_bkgJpsi * [Poly2(psi2s) * CB(jpsi)]                        - Psi(2S)背景 + J/psi信号
    //     + N_bkgBkg  * [Poly2(psi2s) * Poly1(jpsi)]                    - 双背景
    
    // 分量1: CB(psi2s) * CB(jpsi) - 纯信号
    RooProdPdf sig_pdf("sig_pdf", "Signal PDF (CB*CB)", RooArgSet(cb_psi2s, cb_jpsi));

    // 分量2: CB(psi2s) * Poly1(jpsi) - Psi(2S)信号, J/psi背景
    RooProdPdf bkg_psi2s_pdf("bkg_psi2s_pdf", "Psi2S sig + Jpsi bkg", RooArgSet(cb_psi2s, poly1_jpsi));

    // 分量3: Poly2(psi2s) * CB(jpsi) - Psi(2S)背景, J/psi信号
    RooProdPdf bkg_jpsi_pdf("bkg_jpsi_pdf", "Psi2S bkg + Jpsi sig", RooArgSet(poly2_psi2s, cb_jpsi));

    // 分量4: Poly2(psi2s) * Poly1(jpsi) - 双背景
    RooProdPdf bkg_both_pdf("bkg_both_pdf", "Both background", RooArgSet(poly2_psi2s, poly1_jpsi));

    // ==================== 5. 事件数参数 ====================
    RooRealVar n_sig("n_sig", "N_psi2sJpsi", 100, 0, 100000);
    RooRealVar n_bkg_psi2s("n_bkg_psi2s", "N_psi2sBkg", 1000, 0, 1000000);
    RooRealVar n_bkg_jpsi("n_bkg_jpsi", "N_BkgJpsi", 1000, 0, 1000000);
    RooRealVar n_bkg_both("n_bkg_both", "N_BkgBkg", 100000, 0, 10000000);

    // ==================== 6. 总 PDF ====================
    RooAddPdf total_pdf("total_pdf", "Total 2D PDF",
                        RooArgList(sig_pdf, bkg_psi2s_pdf, bkg_jpsi_pdf, bkg_both_pdf),
                        RooArgList(n_sig, n_bkg_psi2s, n_bkg_jpsi, n_bkg_both));

    // ==================== 7. 执行拟合 ====================
    double Data_edm;
    int Data_status, Data_covQual;
    RooFitResult *fitRes = nullptr;
    unsigned int ii = 0;

    while (true) {
        if (fitRes) delete fitRes;
        
        fitRes = total_pdf.fitTo(*data, 
                                 RooFit::Hesse(kTRUE), 
                                 RooFit::NumCPU(8), 
                                 RooFit::Save(kTRUE), 
                                 RooFit::Minos(kFALSE), 
                                 RooFit:: PrintLevel(0), 
                                 RooFit::Warnings(kFALSE), 
                                 RooFit::PrintEvalErrors(-1));
        
        Data_edm = fitRes->edm();
        Data_status = fitRes->status();
        Data_covQual = fitRes->covQual();

        if (isDraw) {
            std::cout << "[Fit2D] Iteration " << ii << ": EDM=" << Data_edm 
                      << ", Status=" << Data_status << ", CovQual=" << Data_covQual << std::endl;
        }

        ii++;

        if ((Data_edm < 0.01 && Data_status == 0 && Data_covQual == 3) || ii >= 10)
            break;
    }


    // ==================== 8. 计算结果 ====================
    double nsig_total = n_sig.getVal();
    double nsig_total_err = n_sig.getError();
    double nbkg_psi2s_val = n_bkg_psi2s.getVal();
    double nbkg_psi2s_err = n_bkg_psi2s.getError();
    double nbkg_jpsi_val = n_bkg_jpsi.getVal();
    double nbkg_jpsi_err = n_bkg_jpsi.getError();
    double nbkg_both_val = n_bkg_both.getVal();
    double nbkg_both_err = n_bkg_both.getError();
    double n_combinatorial = nbkg_psi2s_val + nbkg_jpsi_val + nbkg_both_val;

    n_sig.setVal(627);
    n_bkg_psi2s.setVal(155);
    n_bkg_jpsi.setVal(1463);
    n_bkg_both.setVal(629);
    
    // 计算纯度 = N_signal / (N_signal + N_bkg_psi2s + N_bkg_jpsi)
    double purity = nsig_total / (nsig_total + n_combinatorial) * 100.0;
    
    // 获取sigma值
    double psi2s_sigma_val = psi2s_sigma.getVal();
    double psi2s_sigma_err = psi2s_sigma.getError();
    double jpsi_sigma_val = jpsi_sigma.getVal();
    double jpsi_sigma_err = jpsi_sigma.getError();
    
    // 定义信号区域 (mass ± 3σ)
    double psi2s_low = psi2s_mean.getVal() - 3 * psi2s_sigma_val;
    double psi2s_high = psi2s_mean.getVal() + 3 * psi2s_sigma_val;
    double jpsi_low = jpsi_mean.getVal() - 3 * jpsi_sigma_val;
    double jpsi_high = jpsi_mean.getVal() + 3 * jpsi_sigma_val;
    
    // 计算信号区域内的事件数
    // 使用直接积分方式，而不是命名范围
    RooAbsReal* sig_integral = sig_pdf.createIntegral(RooArgSet(Psi2S_mass, Jpsi2_mass));
    RooAbsReal* bkg_psi2s_integral = bkg_psi2s_pdf.createIntegral(RooArgSet(Psi2S_mass, Jpsi2_mass));
    RooAbsReal* bkg_jpsi_integral = bkg_jpsi_pdf.createIntegral(RooArgSet(Psi2S_mass, Jpsi2_mass));
    RooAbsReal* bkg_both_integral = bkg_both_pdf.createIntegral(RooArgSet(Psi2S_mass, Jpsi2_mass));

    // 计算信号区域内的积分
    Psi2S_mass.setRange("psi2s_peak", psi2s_low, psi2s_high);
    Jpsi2_mass.setRange("jpsi_peak", jpsi_low, jpsi_high);
    
    // 创建信号区域的积分（使用乘积积分）
    RooAbsReal* sig_integral_peak = sig_pdf.createIntegral(RooArgSet(Psi2S_mass), RooArgSet(Jpsi2_mass), "psi2s_peak")
                                          ->createIntegral(RooArgSet(Jpsi2_mass), "jpsi_peak");
    RooAbsReal* bkg_psi2s_integral_peak = bkg_psi2s_pdf.createIntegral(RooArgSet(Psi2S_mass), RooArgSet(Jpsi2_mass), "psi2s_peak")
                                                          ->createIntegral(RooArgSet(Jpsi2_mass), "jpsi_peak");
    RooAbsReal* bkg_jpsi_integral_peak = bkg_jpsi_pdf.createIntegral(RooArgSet(Psi2S_mass), RooArgSet(Jpsi2_mass), "psi2s_peak")
                                                        ->createIntegral(RooArgSet(Jpsi2_mass), "jpsi_peak");
    RooAbsReal* bkg_both_integral_peak = bkg_both_pdf.createIntegral(RooArgSet(Psi2S_mass), RooArgSet(Jpsi2_mass), "psi2s_peak")
                                                        ->createIntegral(RooArgSet(Jpsi2_mass), "jpsi_peak");

    double sig_fraction = sig_integral_peak->getVal() / sig_integral->getVal();
    double bkg_psi2s_fraction = bkg_psi2s_integral_peak->getVal() / bkg_psi2s_integral->getVal();
    double bkg_jpsi_fraction = bkg_jpsi_integral_peak->getVal() / bkg_jpsi_integral->getVal();
    double bkg_both_fraction = bkg_both_integral_peak->getVal() / bkg_both_integral->getVal();

    double nsig_signal = nsig_total * sig_fraction;
    double nbkg_signal = nbkg_psi2s_val * bkg_psi2s_fraction + 
                         nbkg_jpsi_val * bkg_jpsi_fraction + 
                         nbkg_both_val * bkg_both_fraction;

    double significance = nsig_signal / sqrt(nsig_signal + nbkg_signal);
    
    // 如果拟合不成功，返回 -1
    if (!(Data_edm < 0.01 && Data_status == 0 && Data_covQual == 3)) {
        significance = -1.0;
    }

    // 打印拟合结果表格
    if (isDraw) {
        std::cout << "\n======================================" << std::endl;
        std::cout << "           2D Fit Results" << std::endl;
        std::cout << "======================================" << std::endl;
        std::cout << "Signal region: Psi(2S) [" << psi2s_low << ", " << psi2s_high << "] GeV" << std::endl;
        std::cout << "               J/psi   [" << jpsi_low << ", " << jpsi_high << "] GeV" << std::endl;
        std::cout << "--------------------------------------" << std::endl;
        printf("N_psi2sJpsi     = %6.0f ± %4.0f\n", nsig_total, nsig_total_err);
        printf("N_psi2sBkg      = %6.0f ± %4.0f\n", nbkg_psi2s_val, nbkg_psi2s_err);
        printf("N_BkgJpsi       = %6.0f ± %4.0f\n", nbkg_jpsi_val, nbkg_jpsi_err);
        printf("N_BkgBkg        = %6.0f ± %4.0f\n", nbkg_both_val, nbkg_both_err);
        printf("N_combinatorial = %6.0f\n", n_combinatorial);
        printf("Purity          = %6.1f%%\n", purity);
        printf("--------------------------------------\n");
        printf("sigma_Psi(2S)   = %.6f ± %.6f GeV\n", psi2s_sigma_val, psi2s_sigma_err);
        printf("sigma_J/psi     = %.6f ± %.6f GeV\n", jpsi_sigma_val, jpsi_sigma_err);
        printf("--------------------------------------\n");
        printf("Significance    = %.2f\n", significance);
        printf("======================================\n\n");
    }

    delete sig_integral;
    delete bkg_psi2s_integral;
    delete bkg_jpsi_integral;
    delete bkg_both_integral;
    delete sig_integral_peak;
    delete bkg_psi2s_integral_peak;
    delete bkg_jpsi_integral_peak;
    delete bkg_both_integral_peak;
    delete fitRes;

    // ==================== 9. 绘制结果（如果需要）====================
    if (isDraw) {
        // 设置绘图风格
        gStyle->SetOptStat(0);
        gStyle->SetOptTitle(0);
        
        TCanvas* c = new TCanvas("c", "2D Fit Projections", 2000, 800);
        c->Divide(2, 1);
        
        // 创建非负版本的背景多项式用于绘图
        RooFormulaVar poly1_jpsi_pos("poly1_jpsi_pos", "Poly1 positive", "@0>0 ? @0 : 1e-10", RooArgList(poly1_jpsi));
        RooFormulaVar poly2_psi2s_pos("poly2_psi2s_pos", "Poly2 positive", "@0>0 ? @0 : 1e-10", RooArgList(poly2_psi2s));
        
        // 创建非负版本的背景 PDF
        RooProdPdf bkg_psi2s_pdf_pos("bkg_psi2s_pdf_pos", "Psi2S sig + Jpsi bkg pos", RooArgSet(cb_psi2s, poly1_jpsi_pos));
        RooProdPdf bkg_jpsi_pdf_pos("bkg_jpsi_pdf_pos", "Psi2S bkg + Jpsi sig pos", RooArgSet(poly2_psi2s_pos, cb_jpsi));
        RooProdPdf bkg_both_pdf_pos("bkg_both_pdf_pos", "Both background pos", RooArgSet(poly2_psi2s_pos, poly1_jpsi_pos));
        
        // 创建非负版本的总 PDF
        RooAddPdf total_pdf_pos("total_pdf_pos", "Total 2D PDF positive",
                                RooArgList(sig_pdf, bkg_psi2s_pdf_pos, bkg_jpsi_pdf_pos, bkg_both_pdf_pos),
                                RooArgList(n_sig, n_bkg_psi2s, n_bkg_jpsi, n_bkg_both));
        
        // 绘制 Psi(2S) 投影
        c->cd(1);
        RooPlot* psi2s_frame = Psi2S_mass.frame(RooFit::Title("M(#mu^{+}#mu^{-}#pi^{+}#pi^{-})"));
        data->plotOn(psi2s_frame, RooFit::MarkerColor(kBlack), RooFit::MarkerStyle(20));
        
        // 绘制各个分量（使用非负版本）
        total_pdf_pos.plotOn(psi2s_frame, RooFit::LineColor(kBlue), RooFit::LineWidth(2));
        total_pdf_pos.plotOn(psi2s_frame, RooFit::Components("sig_pdf"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
        total_pdf_pos.plotOn(psi2s_frame, RooFit::Components("bkg_psi2s_pdf_pos"), RooFit::LineColor(kCyan), RooFit::LineStyle(kDashed));
        total_pdf_pos.plotOn(psi2s_frame, RooFit::Components("bkg_jpsi_pdf_pos"), RooFit::LineColor(kGreen), RooFit::LineStyle(kDashed));
        total_pdf_pos.plotOn(psi2s_frame, RooFit::Components("bkg_both_pdf_pos"), RooFit::LineColor(kMagenta), RooFit::LineStyle(kDashed));

        // 添加图例
        TLegend* leg1 = new TLegend(0.6, 0.65, 0.89, 0.89);
        leg1->SetBorderSize(0);
        leg1->AddEntry(psi2s_frame->findObject("h_data"), "Data", "P");
        leg1->AddEntry(psi2s_frame->findObject("total_pdf_pos_Int[Jpsi2_mass]_Norm[Jpsi2_mass,Psi2S_mass]"), "Total Fit", "L");
        leg1->AddEntry(psi2s_frame->findObject("total_pdf_pos_Int[Jpsi2_mass]_Norm[Jpsi2_mass,Psi2S_mass]_Comp[sig_pdf]"), "#psi(2S)J/#psi", "L");
        leg1->AddEntry(psi2s_frame->findObject("total_pdf_pos_Int[Jpsi2_mass]_Norm[Jpsi2_mass,Psi2S_mass]_Comp[bkg_psi2s_pdf_pos]"), "#psi(2S)Bkg", "L");
        leg1->AddEntry(psi2s_frame->findObject("total_pdf_pos_Int[Jpsi2_mass]_Norm[Jpsi2_mass,Psi2S_mass]_Comp[bkg_jpsi_pdf_pos]"), "BkgJ/#psi", "L");
        leg1->AddEntry(psi2s_frame->findObject("total_pdf_pos_Int[Jpsi2_mass]_Norm[Jpsi2_mass,Psi2S_mass]_Comp[bkg_both_pdf_pos]"), "BkgBkg", "L");
        
        psi2s_frame->SetXTitle("M(#mu^{+}#mu^{-}#pi^{+}#pi^{-}) [GeV]");
        psi2s_frame->SetYTitle("Events / 0.004 GeV");
        psi2s_frame->Draw();
        leg1->DrawClone();
        
        // 绘制 J/psi 投影
        c->cd(2);
        RooPlot* jpsi_frame = Jpsi2_mass.frame(RooFit::Title("M(#mu^{+}#mu^{-})"));
        data->plotOn(jpsi_frame, RooFit::MarkerColor(kBlack), RooFit::MarkerStyle(20));
        
        // 绘制各个分量（使用非负版本）
        total_pdf_pos.plotOn(jpsi_frame, RooFit::LineColor(kBlue), RooFit::LineWidth(2));
        total_pdf_pos.plotOn(jpsi_frame, RooFit::Components("sig_pdf"), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));
        total_pdf_pos.plotOn(jpsi_frame, RooFit::Components("bkg_psi2s_pdf_pos"), RooFit::LineColor(kCyan), RooFit::LineStyle(kDashed));
        total_pdf_pos.plotOn(jpsi_frame, RooFit::Components("bkg_jpsi_pdf_pos"), RooFit::LineColor(kGreen), RooFit::LineStyle(kDashed));
        total_pdf_pos.plotOn(jpsi_frame, RooFit::Components("bkg_both_pdf_pos"), RooFit::LineColor(kMagenta), RooFit::LineStyle(kDashed));

        // 添加图例
        TLegend* leg2 = new TLegend(0.6, 0.65, 0.89, 0.89);
        leg2->SetBorderSize(0);
        leg2->AddEntry(jpsi_frame->findObject("h_data"), "Data", "P");
        leg2->AddEntry(jpsi_frame->findObject("total_pdf_pos_Int[Psi2S_mass]_Norm[Jpsi2_mass,Psi2S_mass]"), "Total Fit", "L");
        leg2->AddEntry(jpsi_frame->findObject("total_pdf_pos_Int[Psi2S_mass]_Norm[Jpsi2_mass,Psi2S_mass]_Comp[sig_pdf]"), "#psi(2S)J/#psi", "L");
        leg2->AddEntry(jpsi_frame->findObject("total_pdf_pos_Int[Psi2S_mass]_Norm[Jpsi2_mass,Psi2S_mass]_Comp[bkg_psi2s_pdf_pos]"), "#psi(2S)Bkg", "L");    
        leg2->AddEntry(jpsi_frame->findObject("total_pdf_pos_Int[Psi2S_mass]_Norm[Jpsi2_mass,Psi2S_mass]_Comp[bkg_jpsi_pdf_pos]"), "BkgJ/#psi", "L");
        leg2->AddEntry(jpsi_frame->findObject("total_pdf_pos_Int[Psi2S_mass]_Norm[Jpsi2_mass,Psi2S_mass]_Comp[bkg_both_pdf_pos]"), "BkgBkg", "L");
        
        jpsi_frame->SetXTitle("M(#mu^{+}#mu^{-}) [GeV]");
        jpsi_frame->SetYTitle("Events / 0.01 GeV");
        jpsi_frame->Draw();
        leg2->DrawClone();
        
        c->SaveAs("fit_result_2d_proj.png");
        c->SaveAs("fit_result_2d_proj.pdf");
        
        delete leg1;
        delete leg2;
        delete psi2s_frame;
        delete jpsi_frame;
        delete c;
    }
    if (isDraw) {
        std::cout << "[Fit2D] Final: Iterations=" << ii << ", EDM=" << Data_edm 
                  << ", Status=" << Data_status << ", CovQual=" << Data_covQual << std::endl;
    }

    // ==================== 10. 返回结果 ====================
    FitResult2D result;
    result.significance = significance;
    result.n_sig = nsig_total;
    result.n_sig_err = nsig_total_err;
    result.n_bkg_psi2s = nbkg_psi2s_val;
    result.n_bkg_psi2s_err = nbkg_psi2s_err;
    result.n_bkg_jpsi = nbkg_jpsi_val;
    result.n_bkg_jpsi_err = nbkg_jpsi_err;
    result.n_bkg_both = nbkg_both_val;
    result.n_bkg_both_err = nbkg_both_err;
    result.n_combinatorial = n_combinatorial;
    result.purity = purity;
    result.psi2s_sigma_val = psi2s_sigma_val;
    result.psi2s_sigma_err = psi2s_sigma_err;
    result.jpsi_sigma_val = jpsi_sigma_val;
    result.jpsi_sigma_err = jpsi_sigma_err;
    result.entries = data->sumEntries();
    result.iterations = ii;
    result.status = (Data_edm < 0.01 && Data_status == 0 && Data_covQual == 3) ? 0 : -1;

    return result;
}

// 主运行函数
void Fit2D(TString path = "roots/", bool isDraw = true)
{
    // 使用 pre.o 中的 cut 条件
    //std::string cut = "mu1_pt>2 && mu2_pt>2 && mu3_pt>2 && mu4_pt>2 && pi1_pt>0.5 && pi2_pt>0.5 && dR_Psi2S_pi1 < 0.35 && dR_Psi2S_pi2 < 0.35 && Psi2S_VtxProb > 0.001"; //&& Psi2S_mass < 3.8 && Psi2S_mass > 3.6 && Jpsi1_mass < 3.4 && Jpsi1_mass > 2.8 && Jpsi2_mass < 3.4 && Jpsi2_mass > 2.8";
    std::string cut = "mu1_pt>2 && mu2_pt>2 && mu3_pt>2 && mu4_pt>2 && pi1_pt>0.5 && pi2_pt>0.5 && dR_Psi2S_pi1 < 0.35 && dR_Psi2S_pi2 < 0.35 && Psi2S_VtxProb > 0.001 && Jpsi1_VtxProb > 0.01 && Jpsi2_VtxProb > 0.01 && X_PJ_mass < 16 && nLooseMuons >= 4 && Jpsi2_hasJConstraintFit == 1 ";//&& Psi2S_mass < 3.8 && Psi2S_mass > 3.6 && Jpsi1_mass < 3.4 && Jpsi1_mass > 2.8 && Jpsi2_mass < 3.4 && Jpsi2_mass > 2.8";

    std::cout << "[Fit2D] Loading tree from: " << path << std::endl;
    std::cout << "[Fit2D] Using cut: " << cut << std::endl;
    
    // 获取树
    TTree *SourceTree = GetTree<TTree>(path, cut);
    if (!SourceTree || !SourceTree->GetEntries()) {
        if (SourceTree) delete SourceTree;
        throw std::runtime_error("Error: Fail to read data tree or tree is empty");
    }
    
    std::cout << "[Fit2D] Tree entries: " << SourceTree->GetEntries() << std::endl;
    
    // 定义 RooFit 变量
    RooRealVar Psi2S_mass("Psi2S_mass", "M(#mu#mu#pi#pi)", PSI2S_MASS_MIN, PSI2S_MASS_MAX);
    RooRealVar Jpsi2_mass("Jpsi2_mass", "M(#mu#mu) mass", JPSI_MASS_MIN, JPSI_MASS_MAX);
    
    // 构建数据集
    RooDataSet *data = new RooDataSet("data", "2D dataset for Psi(2S) and J/psi", 
                                      RooArgSet(Psi2S_mass, Jpsi2_mass), 
                                      RooFit::Import(*SourceTree));
    
    std::cout << "[Fit2D] Dataset entries: " << data->sumEntries() << std::endl;
    
    // 执行 2D 拟合
    FitResult2D result = Fit2D(data, isDraw);
    
    // 输出结果摘要
    std::cout << "\n[Fit2D] Fit Summary:" << std::endl;
    std::cout << "Status: " << (result.status == 0 ? "SUCCESS" : "FAILED") << std::endl;
    std::cout << "Iterations: " << result.iterations << std::endl;
    std::cout << "Entries: " << result.entries << std::endl;
    std::cout << "Significance: " << result.significance << std::endl;
    std::cout << "Purity: " << result.purity << "%" << std::endl;
    
    // 清理
    delete data;
    delete SourceTree;
}