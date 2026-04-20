# X3872Analysis

Physics analysis code for the X(3872) particle in the J/ψ ππ decay channel. This project performs distribution plotting, cut optimization, and significance calculation using the CMS Run II data.

## Project Overview

This analysis project contains three main components:

1. **Distribution Plotting** - Plots kinematic distributions of various physical observables from reconstructed Monte Carlo or data
2. **Cut Optimization** - Systematically optimizes selection cuts to maximize the significance of the X(3872) signal
3. **Significance Calculation** - Perverts a likelihood fit to calculate the statistical significance of the observed signal

## Architecture

```
X3872Analysis/
├── X3872Utils.h           # Centralized utility functions (common helper functions)
├── MergeRootFiles.C       # Merge multiple ROOT files into a single ntuple
├── DistributionPlotting.C # Distribution plotting component
├── CutOptimization.C      # Cut optimization component
├── SignificanceCalculation.C # Significance calculation component
├── FindBestCut.C          # Find optimal cut combinations after optimization
├── CondorHelper.py        # Helper script for HTCondor job submission
└── README.md              # This file
```

## Component Description

### 1. Distribution Plotting Module

**File**: [DistributionPlotting.C](DistributionPlotting.C)

**Key Functions**:
- `PlotDistributions()` - Main entry point for plotting all distributions
- Uses multi-threaded parallel processing with ROOT's TTreeProcessorMT
- Automatically determines variable ranges using 3σ range estimation
- Generates PDF output files for each distribution

**Plotted Distributions**:
- J/ψ candidate mass and pT distributions
- Di-muon and di-pion angular correlations
- Ψ(2S) mass distributions (for sideband subtraction)
- X(6900) invariant mass distribution

### 2. Cut Optimization Module

**File**: [CutOptimization.C](CutOptimization.C)

**Key Functions**:
- `CutOpt(TString path, TString variable)` - Main entry point for cut optimization
- `Fit(Work work, bool isDraw)` - Fits the invariant mass spectrum with signal + background PDF
- `FindBestCut(std::vector<Result> &result)` - Applies smoothing and finds the optimal cut

**Optimization Method**:
- For each variable, scans 100 different cut values
- Performs a binned fit with RooFit for each cut
- Signal PDF: Double Gaussian
- Background PDF: Third-order Chebychev polynomial
- Figure of merit: S/√(S+B) in the signal region
- Uses Savitzky-Golay smoothing on the significance profile to find the maximum

**Output**: Writes results to `cut_optimization_results.txt` with format:
```
path,cut_expression,merit,efficiency
```

### 3. Significance Calculation Module

**File**: [SignificanceCalculation.C](SignificanceCalculation.C)

**Key Functions**:
- `Significance(TString path)` - Main entry point for significance calculation
- `Fit(RooDataSet *data, bool isBkg, unsigned int Nnorm)` - Performs maximum likelihood fit

**Calculation Method**:
- Fits the data twice:
  - Background-only hypothesis (H₀)
  - Signal + background hypothesis (H₁)
- Uses likelihood ratio test: -2Δln(L) is asymptotically χ² distributed
- Converts p-value to significance using RooStats utilities

**Output**: Prints significance in standard deviations to stdout and creates a plot of the fit.

## Centralized Utility Library

**File**: [X3872Utils.h](X3872Utils.h)

All auxiliary functions are collected in this header file for maximum reusability:

### File System Utilities
- `ExpandPath(const std::string &path)` - Expands ~ to home directory
- `FindDirectoriesWithRootFiles(const std::string &directory, std::vector<std::string> &directories)` - Recursively finds directories containing ROOT files

### ROOT Parallel Processing Helpers
- `GetTree(TChain &tree)` - Initializes multi-threaded tree processor
- `GetTree(TString path)` - Creates chain and initializes tree processor

### Statistical Utilities
- `Statics(ROOT::TTreeProcessorMT *tp, TString variable, std::function<bool(float)> selection)` - Calculates statistics (mean, sigma, min, max)
- `AutoRange(...)` - Automatically determines histogram range as [μ-3σ, μ+3σ]
- `Median`, `MedianAbsoluteDiff`, `SavitzkyGolay` - Smoothing utilities

### Histogram Drawing Functions
- `DrawGraph(...)` - Fills histogram in parallel with selection
- `DrawSideBand(...)` - Does sideband subtraction for background
- `DrawAsPdf(...)` - Draws histogram and saves as PDF

### Selection Functions
- `PeakSelection` - Selects events in the Ψ(2S) signal region (3.68 - 3.69 GeV)
- `SideBandSelection` - Selects events in the sidebands (3.65-3.67 GeV and 3.71-3.728 GeV)
- `X3872Selection` - Selects events in the X(3872) region (3.869 - 3.88 GeV)

## Dependencies

- **ROOT** (with parallel extensions): ≥ 6.00
- **RooFit** and **RooStats**: Included with ROOT
- **C++ compiler**: Supports C++11 or later
- **Python** (optional): For Condor job submission helper

## Compilation

Each component can be compiled separately with ROOT:

```bash
# Compile and run merging
root -l -b -q MergeRootFiles.C

# Compile and run distribution plotting
root -l -b -q DistributionPlotting.C

# Compile and run cut optimization
root -l -b -q "CutOptimization.C+(\"/path/to/data\", \"variable_name\")"

# Compile and run significance calculation
root -l -b -q "SignificanceCalculation.C+(\"/path/to/data\")"

# Compile and find best cuts
root -l -b -q FindBestCut.C
```

Or compile with ACLiC:

```bash
root -l -b -q 'gSystem->Load("X3872Utils.h"); .L DistributionPlotting.C+'
```

## Usage Instructions

### Step 1: Merge Input Files

Before analysis, merge multiple input ROOT files:

```cpp
root -l -b -q MergeRootFiles.C
```

This creates `merged.root` containing a TTree named `SourceTree` with all entries.

### Step 2: Plot Distributions

To plot all kinematic distributions:

```cpp
root -l -b -q RunPlotDistributions.C
```

This generates multiple PDF files, one for each distribution.

### Step 3: Optimize Cuts

For cut optimization on a batch system with HTCondor:

1. Configure jobs in `CutOptHelper.py`:
   - Set `prefix` to your working directory
   - Add the years/datasets to process
   - Add the variables to optimize

2. Submit jobs:
```bash
python CutOptHelper.py submit
condor_submit do_cutopt.sub
```

3. Collect results:
```bash
python CutOptHelper.py result
```

Alternatively, run interactively for a single variable:
```cpp
root -l -b -q "CutOpt(\"/path/to/data\", \"mu1_Pt\")"
```

### Step 4: Find Best Cut Combinations

After all optimizations complete:

```cpp
root -l -b -q FindBestCut.C
```

This reads the results from `cut_optimization_results.txt` and outputs the best combinations.

### Step 5: Calculate Significance

With optimized cuts applied, calculate the final significance:

```cpp
root -l -b -q "Significance(\"/path/to/data\")"
```

This outputs the significance in standard deviations and creates a plot of the fit.

## Input Data Format

The code expects a ROOT TTree (`SourceTree`) containing the following branches:

- `Jpsi1_mass`, `Jpsi1_pt`, `Jpsi1_eta`, etc. - J/ψ 1 kinematics
- `Jpsi2_mass`, `Jpsi2_pt`, `Jpsi2_eta`, etc. - J/ψ 2 kinematics
- `Psi2S_mass` - Invariant mass of the Ψ(2S) candidate
- `Psi2S_pt`, `Psi2S_eta`, etc. - Ψ(2S) kinematics
- `mu1_Pt`, `mu1_Eta`, etc. - Muon kinematics
- `Pi1_Pt`, `Pi1_Eta`, etc. - Pion kinematics
- Various ΔR observables between objects

## Mass Window Definitions

| Region | Mass Range (GeV) |
|--------|------------------|
| Ψ(2S) Peak | 3.68 - 3.69 |
| Ψ(2S) Left Sideband | 3.65 - 3.67 |
| Ψ(2S) Right Sideband | 3.71 - 3.728 |
| X(3872) | 3.869 - 3.88 |

## Parallel Processing

The code uses ROOT's implicit multi-threading:
- TTreeProcessorMT for histogram filling
- TProcessExecutor for parallel fitting in cut optimization
- Number of threads automatically detected from hardware

## Author

[@Alice Quen](https://github.com/AliceQuen) - Project Lead
- A Physics Student at Tsinhua University
- Contact Me: [junkai.qin@cern.ch](mailto:junkai.qin@cern.ch), [qjk21@mails.tsinghua.edu.cn](mailto:qjk21@mails.tsinghua.edu.cn)

## License

MIT License
