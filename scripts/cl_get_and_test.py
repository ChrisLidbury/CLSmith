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
parser.add_argument('-debug', dest = 'debug', action = 'store_true')
parser.add_argument('-disable_opts', dest = 'disable_opts', action = 'store_true')

parser.add_argument('flags', nargs='*')

parser.add_argument('-resume', type=argparse.FileType('r'), default=None, help="Do not run tests that appear in an existing results file")

parser.set_defaults(debug = False, disable_opts = False)

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

already_processed = []
if args.resume:
  for l in args.resume:
    if "RESULTS FOR" in l:
      already_processed.append(l.split()[-1])

file_list = os.listdir(args.path)
file_index = 0
dirlist = sorted(os.listdir(args.path))
for curr_file in dirlist:
  file_index += 1

  if curr_file in already_processed:
      print("Skipping kernel %s (%d/%d)..." % (curr_file, file_index, len(file_list)))
      continue

  output.write("RESULTS FOR " + curr_file + "\n")  
  output.flush()
  print("Executing kernel %s (%d/%d)..." % (curr_file, file_index, len(file_list)))
  sys.stdout.flush()
  
  file_path = args.path + pathSeparator + curr_file
  cmd = "%s -f %s -p %d -d %d" % (args.cl_launcher, file_path, args.cl_platform_idx, args.cl_device_idx)
  if (args.device_name_contains):
      cmd += " -n " + args.device_name_contains
  if (args.disable_opts):
      cmd += " ---disable_opts"
  if (args.debug):
      cmd += " ---debug"
  run_prog = WorkerThread(args.timeout, cmd)
  run_prog_res = run_prog.start()

  run_prog_out = '\n'.join(filter(lambda x: (not "PLUGIN" in x), run_prog_res[0].split("\n")))

  if "not found in device name" in run_prog_out:
      print("Mismatch in device name (aborting all further runs)")
      sys.exit(1)
  
  if not run_prog_res[1] == 0:
      output.write("run_error: %s\n" % (run_prog_out))
      output.flush()
      continue
  elif run_prog_out == "Process timeout":
      output.write("timeout\n")
      output.flush()
      continue

  run_prog_out = run_prog_out.split(',')
  run_prog_out = filter(None, set(run_prog_out))
  
  for result in run_prog_out:
      output.write(result + ", ")
      output.flush()
  output.write("\n")
#"""
