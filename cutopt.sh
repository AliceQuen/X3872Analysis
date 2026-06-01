#!/bin/bash
# Prepare root environment
which root > /dev/null 2>&1 || source /cvmfs/sft.cern.ch/lcg/views/LCG_108/x86_64-el9-gcc14-opt/setup.sh
which root > /dev/null 2>&1 || (echo "ROOT not found. Please source the ROOT environment setup script." >&2 && exit 1)
(root -e ".L CutOptimization.C" -e "CutOpt(\"$1\", \"$2\")" && echo "CutOpt success." >&2) || (echo "CutOpt failed." >&2 && exit 1)