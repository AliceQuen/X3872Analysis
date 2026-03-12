# This is a helper class for the cut optimization process. It provides methods to calculate the optimal cut value for a given set of data.
# Arguments:
# 1. mode: submit --> Submit jobs to condor, result --> Get the result of the cut optimization process
import os
import argparse
# Class for a cut point
class CutPoint:
    def __init__(self, variable, cut_value, merit, efficiency):
        self.variable = variable
        self.cut_value = cut_value
        self.merit = merit
        self.merit = merit
    def __str__(self):
        return f"{self.variable} {self.cut_value} {self.signal_efficiency} {self.background_efficiency}"
    def __add__(self, other):
        if self.variable != other.variable:
            raise ValueError("Cannot add cut points with different variables")
        return CutPoint(self.variable + '/' + other.variable, self.cut_value, self.merit + other.merit, self.efficiency * other.efficiency)
def conmbine_cuts(cuts1, cuts2):
    combined_cuts = []
    for cut1 in cuts1:
        for cut2 in cuts2:
            if cut1.variable == cut2.variable:
                continue
            combined_cut = cut1 + cut2
            if combined_cut.efficiency >= 0.6:
                combined_cuts.append(combined_cut)
    return combined_cuts
prasor = argparse.ArgumentParser(description='Submit cut optimization jobs or collect results')
prasor.add_argument('mode', type=str, help='Mode of operation, either submit or result')

args = prasor.parse_args()
mode = args.mode
# Change to your settings here
prifix = '/home/storage0/users/junkaiqin/X3872Analysis'
jobs = ['2016', '2017', '2018']
variables = ['mu1_Pt', 'mu2_Pt', 'mu1_Pz', 'mu2_Pz', 'mu1_Eta', 'mu2_Eta', 'mu1_mu2_DeltaR', 'mu1_Iso', 'mu2_Iso', 'n_Loose_mu', 'n_Tight_mu',
    'n_Soft_mu', 'n_Medium_mu', 'Pi1_Pt', 'Pi2_Pt', 'Pi1_Pz', 'Pi2_Pz', 'Pi1_Eta', 'Pi2_Eta', 'Pi1_mu1_DeltaR', 'Pi2_mu1_DeltaR', 'Pi1_mu2_DeltaR',
    'Pi2_mu2_DeltaR', 'Pi_Pi_DeltaR', 'Jpsi1_Pt', 'Jpsi1_Pz', 'Jpsi1_Eta', 'Jpsi1_massErr', 'Jpsi1_VtxProb', 'Jpsi1_mu1_DeltaR', 'Jpsi1_mu2_DeltaR',
    'Jpsi1_Pi1_DeltaR', 'Jpsi1_Pi2_DeltaR', 'Psi2S_Eta', 'Psi2S_massErr', 'Psi2S_VtxProb', 'Psi2S_mu1_DeltaR', 'Psi2S_mu2_DeltaR',
    'Psi2S_Pi1_DeltaR', 'Psi2S_Pi2_DeltaR', 'Psi2S_Jpsi1_DeltaR']
if mode == 'submit':
    # Recreate cut_optimization_results.txt
    open(f'{prifix}/cut_optimization_results.txt', 'w').close()
    arguments = []
    for job in jobs:
        for variable in variables:
            job.rsplit('/')
            arguments.append(f'\t{prifix}/output/DATA/{job} {variable}\n')
    with open(f'condor_cutopt.sub', 'r') as f:
        lines = f.readlines()
    idx = lines.index('    LIST\n')
    lines[idx:idx] = arguments
    idx = lines.index('    LIST\n')
    lines.pop(idx)
    with open(f'do_cutopt.sub', 'w') as f:
        for line in lines:
            f.write(line)
elif mode == 'result':
    with open(f'{prifix}/cut_optimization_results.txt', 'r') as f:
        lines = f.readlines()
    for job in jobs:
        print(f"\033[31m Results for {job}:\033[0m ")
        cuts = []
        # Find all cuts of the job
        for variable in variables:
            for line in lines:
                if line.startswith(f'prefix/{job} {variable}'):
                    if line.strip().split()[4] < 0.6 and line.strip().split()[3] <= 0:
                        continue
                    cuts.append(CutPoint(variable, *line.strip().split()[2:]))
                    break
        # Combine cuts
        conmbine_cuts = cuts
        for i in [1, 2, 3, 4]:
            best_cuts = conmbine_cuts
            combined_cuts = conmbine_cuts(conmbine_cuts, cuts)
            if len(combined_cuts_1) == 0:
                break
        sorted_cuts = sorted(combined_cuts, key=lambda x: x.merit, reverse=True)
        for i in best_cuts:
            print(i)
else:
    print(f"Mode {mode} is not valid. Exiting.")
    exit(1)