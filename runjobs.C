#include "Rtypes.h"
#include "TChain.h"
#include <string>
#include <iostream>
#include "myntuple.h"
void runjobs(TString inputpath, TString outputdir, TString outname)
{
	std::cout << "Input Path: " << inputpath << std::endl;
	std::cout << "Output Directory: " << outputdir << std::endl;
	std::cout << "Output Name: " << outname << std::endl;
	TChain *chain = new TChain("mkcands/X_data", "");
	chain->Add(inputpath);
	myntuple a(chain);
	TString out = outputdir + "/" + outname + ".root";
	std::cout << "Output File: " << out << std::endl;
	a.Loop(out);
    // Delete everything to save memory
	a.~myntuple();
	delete chain;
}
