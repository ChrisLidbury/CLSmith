#! python
#! /usr/bin/python

import sys
import os
import argparse
from processTimeout import WorkerThread

# seed_min  = 1
# seed_max  = 1000
# seed_step = 1

# cl_launcher       = "." + os.sep + "cl_launcher"
# cl_filename       = "CLProg.c"
# cl_platform_idx   = 0
# cl_device_idx     = 0

# output  = "Result.csv"
# gen_exe = "." + os.sep + "CLSmith"

parser = argparse.ArgumentParser("Run generator on a range of seeds.")

parser.add_argument('-cl_launcher', default = "." + os.sep + "cl_launcher")
parser.add_argument('-cl_platform_idx', default = 0, type = int)
parser.add_argument('-cl_device_idx', default = 0, type = int)

parser.add_argument('-path', default = "CLSmithTests/")
parser.add_argument('-output', default = "Result.csv")

parser.add_argument('flags', nargs='*')

args = parser.parse_args()

""" ProcessTimeout testing 
tst = WorkerThread(1, "sleep 2")
result = tst.start()
print(result)
"""

if os.path.isfile(args.output):
    print("Overwriting file %s." % args.output)
#"""
output = open(args.output, 'w')

if not os.path.exists(args.path):
  print("Given path %s does not exist!" % (args.path))
  exit(1);

file_list = os.listdir(args.path)
file_index = 0
for curr_file in os.listdir(args.path):
  file_index += 1
  print("Executing kernel %s (%d/%d)..." % (curr_file, file_index, len(file_list)))
  
  file_path = args.path + os.sep + curr_file
  cmd = "%s -f %s -p %d -d %d" % (args.cl_launcher, file_path, args.cl_platform_idx, args.cl_device_idx)
  run_prog = WorkerThread(150, cmd)
  run_prog_res = run_prog.start()
  
  if not run_prog_res[1] == 0:
      output.write("run_error: %s\n" % (run_prog_res[0]))
      continue
  elif run_prog_res[0] == "Process timeout":
      output.write("timeout\n")
      continue
  
  run_prog_out = run_prog_res[0]
  run_prog_out = run_prog_out.split(',')
  run_prog_out = filter(None, set(run_prog_out))
  
  if len(run_prog_out) > 1:
      output.write("Different.\n")
  else:
      output.write(run_prog_out[0] + "\n")
#"""