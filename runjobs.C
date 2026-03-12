#include "Rtypes.h"
#include "TChain.h"
#include <string>
#include <iostream>
#include "myntuple.h"
void runjobs(TString inputpath, TString outputdir, TString outname)
{
	TChain *chain = new TChain("mkcands/X_data", "");
	chain->Add(inputpath);
	myntuple a(chain);
	TString out = outputdir + "/" + outname + ".root";
	a.Loop(out);
    // Delete everything to save memory
	a.~myntuple();
	delete chain;
}
