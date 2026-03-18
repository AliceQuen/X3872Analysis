#include "Rtypes.h"
#include "TChain.h"
#include <string>
#include <iostream>
#include "DataP.h"
void runjobs(TString inputpath, TString outputdir, TString outname)
{
	TChain *chain = new TChain("mkcands/X_data", "");
	chain->Add(inputpath);
	if (!chain->GetEntries())
		throw std::runtime_error("No entires read from the files!");
	DataP a(chain);
	TString out = outputdir + "/" + outname + ".root";
	a.Loop(out);
	// Delete everything to save memory
	a.~DataP();
	delete chain;
}
