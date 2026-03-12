#!/bin/shell
# Prepare root environment
which root > /dev/null 2>&1 || source /cvmfs/sft.cern.ch/lcg/releases/LCG_100/ROOT/v6.24.00/x86_64-centos7-clang11-opt/ROOT-env.sh
which root > /dev/null 2>&1 || error "ROOT not found. Please source the ROOT environment setup script." >&2 && exit 1
which root
echo $1
echo $2
#root -e ".L CutOpt.C" -e "CutOpt(\"$1\", \"$2\")"