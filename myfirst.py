import os
import sys
import re
import subprocess

print(os.getcwd())
rootdir = os.path.join(os.getcwd(), 'src')

files = []

for root, dirs, filenames in os.walk(rootdir):    
    if '.svn' in dirs:
        dirs.remove('.svn')

    for filename in filenames:
        if re.search(".cpp", filename):
            files.append(os.path.join(root, filename))

print(len(files))

i = 0
while i < len(files):
    '''print(files[i])'''
    status = subprocess.call(r'/Developer/BagelCheckers/my_checker ' + files[i] + ' -I/Developer/badainclude -I/Developer/mp/src -I/Developer/boost145', shell=True)

    i += 1

print('Complete')
