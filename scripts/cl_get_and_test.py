#! python
#! /usr/bin/python

import sys
import os
import argparse
from processTimeout import WorkerThread

clLauncherExecutable = "cl_launcher"
pathSeparator = os.sep
if sys.platform == "win32":
  clLauncherExecutable += ".exe"
  pathSeparator += os.sep

parser = argparse.ArgumentParser("Run generator on a range of given programs.")

parser.add_argument('-cl_launcher', default = "." + pathSeparator + clLauncherExecutable)
parser.add_argument('-cl_platform_idx', default = 0, type = int)
parser.add_argument('-cl_device_idx', default = 0, type = int)
parser.add_argument('-device_name_contains', default = "")

parser.add_argument('-path', default = "CLSmithTests/")
parser.add_argument('-output', default = "Result.csv")
parser.add_argument('-timeout', default = 150, type = int)

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
  output.write("RESULTS FOR " + curr_file + "\n")  
  file_index += 1
  print("Executing kernel %s (%d/%d)..." % (curr_file, file_index, len(file_list)))
  
  file_path = args.path + pathSeparator + curr_file
  cmd = "%s -f %s -p %d -d %d -n %s" % (args.cl_launcher, file_path, args.cl_platform_idx, args.cl_device_idx, args.device_name_contains)
  run_prog = WorkerThread(args.timeout, cmd)
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
  
  for result in run_prog_out:
      output.write(result + ", ")
      output.flush()
  output.write("\n")
#"""
