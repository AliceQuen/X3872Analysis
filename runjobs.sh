#!/bin/bash
# Prepare root environment
which root > /dev/null 2>&1 || source /cvmfs/sft.cern.ch/lcg/releases/LCG_100/ROOT/v6.24.00/x86_64-centos7-clang11-opt/ROOT-env.sh
which root > /dev/null 2>&1 || echo "ROOT not found. Please source the ROOT environment setup script." 
myntuple=$(find . -type f -name "*.h")
myntuple=$(basename $myntuple)
myntuple=${myntuple%.h}
sed -i -e '1,$s/myntuple/'"$myntuple"'/g' runjobs.C
root -e ".L runjobs.C" -e ".x runjobs(\"$1\", \"$2\", \"$3\")"