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
filestring = ""
while i < len(files):
    #print(files[i])
    
    filestring += files[i] + " "
    
    '''print(r'~/Developer/BagelCheckers/my_checker ' + files[i] + r' -I$HOME/Developer/bada120 -I$HOME/Developer/clng/nobs/inc -I$HOME/Developer/boost145')'''
    
    '''status = subprocess.call(r'~/Developer/BagelCheckers/my_checker ' + files[i] + r' -I$HOME/Developer/bada120 -I$HOME/Developer/clng/nobs/inc -I$HOME/Developer/boost145', shell=True)'''
    
    i += 1

'''print(r'~/Developer/BagelCheckers/my_checker ' + filestring + "$" + r' -I$HOME/Developer/bada120 -I$HOME/Developer/clng/nobs/inc -I$HOME/Developer/boost145')'''

status = subprocess.call(r'~/Developer/BagelCheckers/my_checker ' + filestring + "$" + r' -I$HOME/Developer/bada120 -I$HOME/Developer/clng/nobs/inc -I$HOME/Developer/boost145', shell=True)
print('Complete')
