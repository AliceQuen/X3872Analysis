#include "X3872Utils.h"
#include <ROOT/TProcessExecutor.hxx>
#include <ROOT/TThreadExecutor.hxx>
#include <ROOT/TTreeProcessorMT.hxx>
#include <TAttFill.h>
#include <TFile.h>
#include <TObject.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>
#include <TBranch.h>
#include <TLeaf.h>
#include <TObjArray.h>
#include <TChain.h>
#include <TTreeFormula.h>
#include <map>
#include <string>
#include <iostream>
#include <atomic>
#include <vector>
#include <sstream>

std::atomic<int> file_counter(0);

void AddBranch_(TString datafile = "roots/2016_merged.root", 
               TString outputfile = "output/added_branches.root",
               TString newBranchName = "",
               TString newBranchType = "Float_t",
               TString newBranchFormula = "")
{
    TString datafile_local = datafile;
    TString outputfile_local = outputfile;
    std::vector<std::string> temp_files;
    
    auto workItem = [&temp_files, &newBranchName, &newBranchType, &newBranchFormula](TTreeReader &reader) {
        auto local_tree = reader.GetTree();
        
        // 创建临时文件名
        int file_id = file_counter.fetch_add(1);
        std::stringstream ss;
        ss << "output/temp_" << file_id << ".root";
        std::string temp_file = ss.str();
        temp_files.push_back(temp_file);
        
        TFile *temp_output = new TFile(temp_file.c_str(), "RECREATE");
        TTree *target_tree = new TTree("SourceTree", "SourceTree");
        
        std::map<std::string, std::pair<void*, std::string>> var_map;
        std::map<std::string, void*> reader_map;
        
        TObjArray *branches = local_tree->GetListOfBranches();
        if (!branches) {
            std::cerr << "[ERROR] Cannot get list of branches from tree" << std::endl;
            return;
        }
        
        std::cout << "[INFO] Total branches in source tree: " << branches->GetEntries() << std::endl;
        
        for (Int_t i = 0; i < branches->GetEntries(); ++i) {
            TBranch *branch = (TBranch*)branches->At(i);
            if (!branch) continue;
            
            const char *branch_name = branch->GetName();
            
            TObjArray *leaves = branch->GetListOfLeaves();
            if (!leaves || leaves->GetEntries() == 0) continue;
            
            TLeaf *leaf = (TLeaf*)leaves->At(0);
            if (!leaf) continue;
            
            const char *type_name = leaf->GetTypeName();
            std::string type_str(type_name);
            
            if (type_str == "Float_t") {
                float *var = new float;
                *var = 0.0f;
                target_tree->Branch(branch_name, var, (TString(branch_name) + "/F").Data());
                var_map[branch_name] = {var, "Float_t"};
                reader_map[branch_name] = new TTreeReaderValue<float>(reader, branch_name);
            } else if (type_str == "Double_t") {
                double *var = new double;
                *var = 0.0;
                target_tree->Branch(branch_name, var, (TString(branch_name) + "/D").Data());
                var_map[branch_name] = {var, "Double_t"};
                reader_map[branch_name] = new TTreeReaderValue<double>(reader, branch_name);
            } else if (type_str == "Int_t") {
                int *var = new int;
                *var = 0;
                target_tree->Branch(branch_name, var, (TString(branch_name) + "/I").Data());
                var_map[branch_name] = {var, "Int_t"};
                reader_map[branch_name] = new TTreeReaderValue<int>(reader, branch_name);
            } else if (type_str == "UInt_t") {
                unsigned int *var = new unsigned int;
                *var = 0;
                target_tree->Branch(branch_name, var, (TString(branch_name) + "/i").Data());
                var_map[branch_name] = {var, "UInt_t"};
                reader_map[branch_name] = new TTreeReaderValue<unsigned int>(reader, branch_name);
            } else if (type_str == "Long64_t") {
                Long64_t *var = new Long64_t;
                *var = 0;
                target_tree->Branch(branch_name, var, (TString(branch_name) + "/L").Data());
                var_map[branch_name] = {var, "Long64_t"};
                reader_map[branch_name] = new TTreeReaderValue<Long64_t>(reader, branch_name);
            } else if (type_str == "Bool_t") {
                bool *var = new bool;
                *var = false;
                target_tree->Branch(branch_name, var, (TString(branch_name) + "/O").Data());
                var_map[branch_name] = {var, "Bool_t"};
                reader_map[branch_name] = new TTreeReaderValue<bool>(reader, branch_name);
            } else {
                std::cerr << "[WARNING] Unsupported type " << type_name << " for branch " << branch_name << std::endl;
            }
        }
        
        // 添加新的计算 branch
        void *new_var = nullptr;
        TTreeFormula *formula = nullptr;
        bool add_new_branch = (newBranchName.Length() > 0 && newBranchFormula.Length() > 0);
        
        if (add_new_branch) {
            std::cout << "[INFO] Adding new branch: " << newBranchName 
                      << " (type: " << newBranchType << ", formula: " << newBranchFormula << ")" << std::endl;
            
            // 创建新 branch 的变量
            if (newBranchType == "Float_t") {
                new_var = new float;
                *(float*)new_var = 0.0f;
                target_tree->Branch(newBranchName.Data(), new_var, (newBranchName + "/F").Data());
            } else if (newBranchType == "Double_t") {
                new_var = new double;
                *(double*)new_var = 0.0;
                target_tree->Branch(newBranchName.Data(), new_var, (newBranchName + "/D").Data());
            } else if (newBranchType == "Int_t") {
                new_var = new int;
                *(int*)new_var = 0;
                target_tree->Branch(newBranchName.Data(), new_var, (newBranchName + "/I").Data());
            } else if (newBranchType == "Long64_t") {
                new_var = new Long64_t;
                *(Long64_t*)new_var = 0;
                target_tree->Branch(newBranchName.Data(), new_var, (newBranchName + "/L").Data());
            } else if (newBranchType == "Bool_t") {
                new_var = new bool;
                *(bool*)new_var = false;
                target_tree->Branch(newBranchName.Data(), new_var, (newBranchName + "/O").Data());
            } else {
                std::cerr << "[WARNING] Unsupported type " << newBranchType << " for new branch " << newBranchName << std::endl;
                add_new_branch = false;
            }
            
            // 创建 TTreeFormula 来解析表达式
            if (add_new_branch) {
                formula = new TTreeFormula("formula", newBranchFormula.Data(), local_tree);
                if (!formula->GetNdim()) {
                    std::cerr << "[ERROR] Invalid formula: " << newBranchFormula << std::endl;
                    add_new_branch = false;
                }
            }
        }
        add_new_branch = add_new_branch && !newBranchName.IsNull() && !newBranchFormula.IsNull();
        std::cout << "[INFO] Created " << var_map.size() << " branches in target tree" << std::endl;
        
        Long64_t entry_count = 0;
        while (reader.Next()) {
            //if (entry_count > 100) break;
            for (auto &var_pair : var_map) {
                const std::string &branch_name = var_pair.first;
                void *var_ptr = var_pair.second.first;
                const std::string &type_str = var_pair.second.second;
                void *reader_ptr = reader_map[branch_name];
                
                if (type_str == "Float_t") {
                    *(float*)var_ptr = **(TTreeReaderValue<float>*)reader_ptr;
                } else if (type_str == "Double_t") {
                    *(double*)var_ptr = **(TTreeReaderValue<double>*)reader_ptr;
                } else if (type_str == "Int_t") {
                    *(int*)var_ptr = **(TTreeReaderValue<int>*)reader_ptr;
                } else if (type_str == "UInt_t") {
                    *(unsigned int*)var_ptr = **(TTreeReaderValue<unsigned int>*)reader_ptr;
                } else if (type_str == "Long64_t") {
                    *(Long64_t*)var_ptr = **(TTreeReaderValue<Long64_t>*)reader_ptr;
                } else if (type_str == "Bool_t") {
                    *(bool*)var_ptr = **(TTreeReaderValue<bool>*)reader_ptr;
                }
            }
            
            // 计算并填充新 branch
            if (add_new_branch && formula) {
                formula->UpdateFormulaLeaves();
                double value = formula->EvalInstance();
                
                if (newBranchType == "Float_t") {
                    *(float*)new_var = (float)value;
                } else if (newBranchType == "Double_t") {
                    *(double*)new_var = value;
                } else if (newBranchType == "Int_t") {
                    *(int*)new_var = (int)value;
                } else if (newBranchType == "Long64_t") {
                    *(Long64_t*)new_var = (Long64_t)value;
                } else if (newBranchType == "Bool_t") {
                    *(bool*)new_var = (value != 0);
                }
            }
            
            target_tree->Fill();
            entry_count++;
        }
        
        std::cout << "[INFO] Filled " << entry_count << " entries in target tree" << std::endl;
        
        // 写入临时文件
        target_tree->Write();
        temp_output->Close();
        delete temp_output;
        
        // 清理内存
        for (auto &var_pair : var_map) {
            void *var_ptr = var_pair.second.first;
            const std::string &type_str = var_pair.second.second;
            
            if (type_str == "Float_t") {
                delete (float*)var_ptr;
            } else if (type_str == "Double_t") {
                delete (double*)var_ptr;
            } else if (type_str == "Int_t") {
                delete (int*)var_ptr;
            } else if (type_str == "UInt_t") {
                delete (unsigned int*)var_ptr;
            } else if (type_str == "Long64_t") {
                delete (Long64_t*)var_ptr;
            } else if (type_str == "Bool_t") {
                delete (bool*)var_ptr;
            }
        }
        
        // 正确删除所有 TTreeReaderValue 对象
        for (auto &reader_pair : reader_map) {
            const std::string &branch_name = reader_pair.first;
            void *reader_ptr = reader_pair.second;
            std::string type_str = var_map[branch_name].second;
            
            if (type_str == "Float_t") {
                delete (TTreeReaderValue<float>*)reader_ptr;
            } else if (type_str == "Double_t") {
                delete (TTreeReaderValue<double>*)reader_ptr;
            } else if (type_str == "Int_t") {
                delete (TTreeReaderValue<int>*)reader_ptr;
            } else if (type_str == "UInt_t") {
                delete (TTreeReaderValue<unsigned int>*)reader_ptr;
            } else if (type_str == "Long64_t") {
                delete (TTreeReaderValue<Long64_t>*)reader_ptr;
            } else if (type_str == "Bool_t") {
                delete (TTreeReaderValue<bool>*)reader_ptr;
            }
        }
        
        // 清理新 branch 的资源
        if (add_new_branch) {
            if (newBranchType == "Float_t") {
                delete (float*)new_var;
            } else if (newBranchType == "Double_t") {
                delete (double*)new_var;
            } else if (newBranchType == "Int_t") {
                delete (int*)new_var;
            } else if (newBranchType == "Long64_t") {
                delete (Long64_t*)new_var;
            } else if (newBranchType == "Bool_t") {
                delete (bool*)new_var;
            }
            delete formula;
        }
    };
    
    auto *tp = GetTree<ROOT::TTreeProcessorMT>(datafile_local, "");
    tp->Process(workItem);
    
    // 使用 TChain::Merge() 合并临时文件（参考 MergeRootFiles.C 的模式）
    std::cout << "[INFO] Merging " << temp_files.size() << " temporary files..." << std::endl;
    
    TChain chain("SourceTree");
    for (const auto &temp_file : temp_files) {
        chain.Add(temp_file.c_str());
    }
    
    chain.SetName("SourceTree");
    chain.Merge(outputfile.Data());
    
    std::cout << "[INFO] Total entries in merged tree: " << chain.GetEntries() << std::endl;
    
    // 清理临时文件
    for (const auto &temp_file : temp_files) {
        std::remove(temp_file.c_str());
    }
    
    std::cout << "[INFO] AddBranch completed successfully!" << std::endl;
}

// 默认调用（无新 branch）
void AddBranch() {
    AddBranch_("roots/2016_merged.root", "roots1/2016_merged.root", "Psi2S_mass_diff", "Float_t", "fabs(Psi2S_mass - 3.686)");
    AddBranch_("roots/2017_merged.root", "roots1/2017_merged.root", "Psi2S_mass_diff", "Float_t", "fabs(Psi2S_mass - 3.686)");
    AddBranch_("roots/2018_merged.root", "roots1/2018_merged.root", "Psi2S_mass_diff", "Float_t", "fabs(Psi2S_mass - 3.686)");
}
