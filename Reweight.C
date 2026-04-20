#include "X3872Utils.h"
#define MU1_PT_MIN 20
#define MU1_PT_MAX 30
#define MU2_PT_MIN 20
#define MU2_PT_MAX 30

void Reweight(TString data_path, TString mc_path, TString out_path_mc = "mc_weighted.root", TString out_path_data = "data_weighted.root")
{
    auto sig = DrawSideBand2D(data_path, "mu1_pt", "mu2_pt", "mu1_pt vs mu2_pt", MU1_PT_MIN, MU1_PT_MAX, MU2_PT_MIN, MU2_PT_MAX, "", 100, 100).first;
    auto mc = DrawGraph2D(mc_path, "mu1_pt", "mu2_pt", "mu1_pt vs mu2_pt", MU1_PT_MIN, MU1_PT_MAX, MU2_PT_MIN, MU2_PT_MAX, 100, 100);
    
    // Reweight MC with multi-threading
    int nthreads = std::thread::hardware_concurrency();
    ROOT::EnableImplicitMT(nthreads);
    auto chain = GetTree<TChain>(mc_path, "");
    ROOT::TTreeProcessorMT tp(*chain);
    
    // Each thread processes its own portion, creates a local tree with new branch
    ROOT::TThreadedObject<std::shared_ptr<TTree>> local_trees;
    
    auto setweight = [&local_trees, &sig, &mc](TTreeReader &reader)
    {
        TTree *input_tree = reader.GetTree();
        auto local_tree = local_trees.Get();
        *local_tree = std::shared_ptr<TTree>(input_tree->CloneTree(0));
    
        // add new branch
        double weight;
        (*local_tree)->Branch("weight", &weight, "weight/D");
    
        TTreeReaderValue<double> mu1_pt(reader, "mu1_pt");
        TTreeReaderValue<double> mu2_pt(reader, "mu2_pt");

        while (reader.Next()) {
            int bin_x = sig->GetXaxis()->FindBin(*mu1_pt);
            int bin_y = sig->GetYaxis()->FindBin(*mu2_pt);
            double data_content = sig->GetBinContent(bin_x, bin_y);
            double mc_content = mc->GetBinContent(bin_x, bin_y);
        
            if (mc_content > 0 && data_content > 0) {
                weight = data_content / mc_content;
            } else {
                weight = 1.0;
            }
            (*local_tree)->Fill();
        }
    };
    
    tp.Process(setweight);
    
    // Merge all local trees from different threads
    TFile *out_file = new TFile(out_path_mc, "RECREATE");
    TTree *merged_tree = chain->GetTree()->CloneTree(0);
    double weight_mc;
    merged_tree->Branch("weight", &weight_mc, "weight/D");
    
    for (unsigned int i = 0; i < local_trees.GetNSlots(); i++) {
        auto *tree_ptr = local_trees.GetAtSlotRaw(i);
        if (tree_ptr && *tree_ptr) {
            TTree *local_tree = tree_ptr->get();
            if (local_tree && local_tree->GetEntries() > 0) {
                TBranch *br_weight_local = local_tree->GetBranch("weight");
                double weight_local;
                br_weight_local->SetAddress(&weight_local);
                
                for (Long64_t j = 0; j < local_tree->GetEntries(); j++) {
                    local_tree->GetEntry(j);
                    weight_mc = weight_local;
                    merged_tree->Fill();
                }
            }
        }
    }
    
    merged_tree->Write("SourceTree");
    out_file->Close();
    delete chain;
    delete out_file;
    
    // Reweight Data (add weight branch with weight = 1.0) using multi-threading
    int nthreads_data = std::thread::hardware_concurrency();
    ROOT::EnableImplicitMT(nthreads_data);
    auto data_chain = GetTree<TChain>(data_path, "");
    ROOT::TTreeProcessorMT tp_data(*data_chain);
    
    ROOT::TThreadedObject<std::shared_ptr<TTree>> data_local_trees;
    
    auto setweight_data = [&data_local_trees](TTreeReader &reader)
    {
        TTree *input_tree = reader.GetTree();
        auto local_tree = data_local_trees.Get();
        *local_tree = std::shared_ptr<TTree>(input_tree->CloneTree(0));
        
        double weight = 1.0;
        (*local_tree)->Branch("weight", &weight, "weight/D");
    
        while (reader.Next()) {
            (*local_tree)->Fill();
        }
    };
    
    tp_data.Process(setweight_data);
    
    // Merge all local trees
    TFile *data_file = new TFile(out_path_data, "RECREATE");
    TTree *merged_data = data_chain->GetTree()->CloneTree(0);
    double weight_data;
    merged_data->Branch("weight", &weight_data, "weight/D");
    
    for (unsigned int i = 0; i < data_local_trees.GetNSlots(); i++) {
        auto *tree_ptr = data_local_trees.GetAtSlotRaw(i);
        if (tree_ptr && *tree_ptr) {
            TTree *local_tree = tree_ptr->get();
            if (local_tree && local_tree->GetEntries() > 0) {
                TBranch *br_weight_local = local_tree->GetBranch("weight");
                double weight_local;
                br_weight_local->SetAddress(&weight_local);
                
                for (Long64_t j = 0; j < local_tree->GetEntries(); j++) {
                    local_tree->GetEntry(j);
                    weight_data = weight_local;
                    merged_data->Fill();
                }
            }
        }
    }
    
    merged_data->Write("SourceTree");
    data_file->Close();

    std::cout << "Done! MC entries: " << merged_tree->GetEntries() << std::endl;
    std::cout << "Data entries: " << merged_data->GetEntries() << std::endl;
    std::cout << "MC output: " << out_path_mc << std::endl;
    std::cout << "Data output: " << out_path_data << std::endl;
    
    delete data_file;
    delete data_chain;
}
