#!/usr/lib/python

import subprocess
import sys

filename = sys.argv[1]
flags=""

csvfile = open(filename, 'r')
contents = csvfile.read().splitlines()
contents = [s.strip() for s in (filter(None, contents))]

program_names = []

for result in contents:
  if result.startswith("RESULTS FOR"):
    seed = [p for p in result.split(" ") if "." in p]
    seed = seed[0].split(".")[0]
    program_names.append(seed)
      
for p in program_names:
  print p
  subprocess.Popen("./CLSmith_Altera -s %s -o %s --fake_divergence --group_divergence %s" % (p, p, flags), shell=True)
