# This script is used to faciltate the filling of the tree
# It also generates variable lists condor job list for cut optimization
import os
import re
filename = '4mu2pi'
with open(f'{filename}.C', 'r') as file:
    lines = file.readlines()
begin = False
varibales = list()
var = list()
for i in lines:
    m = i.strip()
    if m == '// BRANCH':
        idx = lines.index(i)
    if m == '//_BRANCH':
        idy = lines.index(i)
    if m == '//_PHYSICS':
        break
    if begin:
        j = m.split(' ')[0]
        match = re.match(r'^//.*', j.strip())
        if not match:
            var.append(j)
            varibales.append(f'\t\tfloat {j};\n\t\tvar.push_back(&{j});\n\t\tSourceTree->Branch("{j}", &{j});\n')
    if m == '// PHYSICS':
        begin = True
lines = lines[:idx+1] + varibales + lines[idy:]
os.remove(f'{filename}.C')
with open(f'{filename}.C', 'w') as file:
    for i in lines:
        file.write(i)
with open(f'{filename}.list', 'w') as file:
    for i in var:
        file.write(f'{i}\n')