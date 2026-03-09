# This script generates a condor job list for the given directory and type
# Arguments:
#   1. in_directory: Path to dataset directory
#   2. out_directory: Path to output directory
#   3. type: Type of dataset, either DATA or MC
#   4. error_dir: Path to error directory
#   5. myntuple: Name of the MakeClass marco file
# Optional arguments:
#   --dry_run: Do not submit jobs, only generate job lists and submit files
#   --cat: Find job error if specified

# The condor submit file templete is condor.sub, which should be in the same directory as this script.
# The condor submit file should include the following line to use the generated job list:
#       transfer_input_files	= runjobs.C, MYNTUPLE.C, MYNTUPLE.h
#       error                   = ERROR/NAME_$(Process).err
#       queue arguments from (
#   LIST
#)
# Pay attenion to the parentheses and the position of the error line in the submit file. It should follow the format above to work properly.
# Example usage:
#   python CondorListHelper.py condor_job_defination.json
import json
import os
import argparse
import re

prasor = argparse.ArgumentParser(description='Generate and submit condor job from directory and type')
prasor.add_argument('in_directory', type=str, help='Path to dataset directory')
prasor.add_argument('out_directory', type=str, help='Path to output directory')
prasor.add_argument('job_type', type=str, help='Type of dataset, either DATA or MC')
prasor.add_argument('error_dir', type=str, help='Path to error directory')
prasor.add_argument('myntuple', type=str, help='Name of the MakeClass marco file')
prasor.add_argument('--dry_run', action='store_true', help='Do not submit jobs, only generate job lists and submit files')
prasor.add_argument('--cat', action='store_true', help='Find job error if specified')
args = prasor.parse_args()

in_directory = args.in_directory
out_directory = os.path.abspath(args.out_directory)
err = args.error_dir.rstrip('/')
myntuple = args.myntuple.strip('.C')
myntuple = myntuple.strip('.h')
template = 'condor.sub'
job_type = args.job_type
if args.cat:
    if not os.path.exists(f'err/{job_type}'):
        print(f"Error directory {err} does not exist. Exiting.")
        exit(1)
    else:
        print(f"Errors for {name}:\n-----------------------------")
        for file in os.listdir(err):
            os.system(f'echo $(cat {err}/{job_type}/{file})')
        print('-----------------------------')
    exit(0)
if not os.path.exists(template):
    print(f"Condor submit file template {template} does not exist. Exiting.")
    exit(1)
print('Recreating error directory...')
os.system(f"mkdir -p {err}/{job_type}")
# Generate condor submit file
output = list()
#    if not os.path.exists(directory):
#        print(f"Directory {directory} does not exist. Skipping {job_type}.")
#        continue
if not job_type in ['MC', 'DATA']:
    print(f"Type {job_type} is not valid. Skipping...")
    exit(1)
print(f"\033[31m Generating condor job with directory {in_directory} and type {job_type}.\033[0m ")

# Generate condor job list
if job_type == 'DATA':
    os.system(f"find {in_directory} -type d -links 2 > {job_type}.list")
elif job_type == 'MC':
    os.system(f"find {in_directory} -type f -name '*.root' > {job_type}.list")
with open(f"{job_type}.list") as f:
    lines = f.readlines()
if len(lines) == 0:
    print(f"No files found in {in_directory}. Skipping...")
    os.remove(f"{job_type}.list")
    exit(1)
years = set()
for line in lines:
    line = line.strip()
    job_dir = line
    line = line.split('/')
    if job_type == 'DATA':
        name = line[-3]
        num = line[-1]
        job_dir = f"{job_dir}/*.root"
    elif job_type == 'MC':
        name = line[-4]
        num = re.match(r'.*_([0-9]*).root', line[-1]).group(1)
    # old data only
    name = name.replace(r'_MINIAOD', '')
    year = re.search(r'20[0-9]{2}', name).group(0)
    years.add(year)
    output.append(f'\t{job_dir} {out_directory}/{job_type}/{year} {name}_{num}\n')
# Make sure the output directory exists
if os.path.exists(f'{out_directory}/{job_type}'):
    print(f"Output directory {out_directory}/{job_type} already exists. Do you want remove it? [y/n]")
    ans = input('>>>')
    if ans.lower() == 'n':
        exit(1)
    elif ans.lower() != 'y':
        print("Invalid input. Exiting.")
        exit(1)
os.system(f"rm -rf {out_directory}/{job_type}")
for year in years:
    os.system(f"mkdir -p {out_directory}/{job_type}/{year}")
os.remove(f"{job_type}.list")
os.system(f"cp {template} condor_{job_type}.sub")
os.system(f"sed -i -e 's:ERROR:{err}/{job_type}:g' -e 's/MYNTUPLE/{myntuple}/g' condor_{job_type}.sub") 
with open(f'condor_{job_type}.sub', 'r') as f:
    lines = f.readlines()
idx = lines.index('    LIST\n')
lines[idx:idx] = output
idx = lines.index('    LIST\n')
lines.pop(idx)
with open(f'condor_{job_type}.sub', 'w') as f:
    for line in lines:
        f.write(line)
if not args.dry_run:
    print(f"Submitting condor job for {job_type}...")
    os.system(f"condor_submit condor_{job_type}.sub")
else:
    print(f"Dry run: condor_submit condor_{job_type}.sub\n These file will be used for submission:\n-----------------------------\n")
    os.system(f"cat condor_{job_type}.sub")
    print(f"\n-----------------------------\n")
    os.system(f"condor_submit condor_{job_type}.sub --dry-run dry_run_{job_type}.log")
        