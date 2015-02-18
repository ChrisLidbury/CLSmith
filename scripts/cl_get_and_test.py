#!/usr/bin/python

import atexit
import sys
import os
import argparse
from processTimeout import WorkerThread
import tempfile
import zipfile
import shutil

# nb: extractall is unsafe if you pass in archives from untrusted sources
def unzip(path, fname):
  oldpath = os.getcwd()
  os.chdir(path)
  zf = zipfile.ZipFile(fname, 'r')
  zf.extractall()
  os.chdir(oldpath)

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
parser.add_argument('-zipfile', type=argparse.FileType('r'), default=None, help="Zipfile containing tests to run")
parser.add_argument('-output', default = "Result.csv")
parser.add_argument('-timeout', default = 150, type = int)

parser.add_argument('-resume', type=argparse.FileType('r'), default=None, help="Do not run tests that appear in an existing results file")

parser.add_argument('flags', nargs='*')

args = parser.parse_args()

if os.path.isfile(args.output):
    print("Overwriting file %s." % args.output)

if os.path.dirname(args.output) and not os.path.isdir(os.path.dirname(args.output)):
    print("Creating directory %s." % os.path.dirname(args.output))
    os.makedirs(os.path.dirname(args.output))

output = open(args.output, 'w')

if args.zipfile:
  print("Creating temporary directory [%s]" % args.path)
  args.path = tempfile.mkdtemp(dir=os.getcwd())
  def exit_handler():
    print("Removing temporary directory [%s]" % args.path)
    shutil.rmtree(args.path)
  atexit.register(exit_handler)
  with open(os.path.join(args.path, 'args.txt'), 'w') as f:
    d = args.__dict__
    for x in d:
      f.write(x + ":" + str(d[x]) + "\n")
  unzip(args.path, args.zipfile)

if not os.path.exists(args.path):
  print("Given path %s does not exist!" % (args.path))
  exit(1)

already_processed = []
if args.resume:
  for l in args.resume:
    if "RESULTS FOR" in l:
      already_processed.append(l.split()[-2])

full_file_list = os.listdir(args.path)
file_list = sorted([f for f in os.listdir(args.path) if not f.endswith(".args")])
file_index = 0
for curr_file in file_list:
  file_index += 1

  if curr_file in already_processed:
      print("Skipping kernel %s (%d/%d)..." % (curr_file, file_index, len(file_list)))
      continue

  check_args = os.path.splitext(curr_file)[0] + ".args"
  if not check_args in full_file_list:
    check_args = ""
  else:
    check_args = args.path + pathSeparator + check_args

  lines = ""
  if sys.platform != "win32":
    lines = " (" + os.popen("wc -l " + args.path + pathSeparator + curr_file).readline().split()[0] + ")"
  output.write("RESULTS FOR " + curr_file + lines + "\n")
  output.flush()
  print("Executing kernel %s (%d/%d)..." % (curr_file, file_index, len(file_list)))
  sys.stdout.flush()

  file_path = args.path + pathSeparator + curr_file
  cmd = "%s -f %s -p %d -d %d" % (args.cl_launcher, file_path, args.cl_platform_idx, args.cl_device_idx)
  if (args.device_name_contains):
      cmd += " -n " + args.device_name_contains
  if (check_args):
      cmd += " -a " + check_args
  if (args.flags):
      cmd += " " + " ".join(args.flags)
  run_prog = WorkerThread(args.timeout, cmd)
  run_prog_res = run_prog.start()

  run_prog_out = run_prog_res[0].decode('unicode_escape')
  run_prog_out = '\n'.join(filter(lambda x: (not "PLUGIN" in x), run_prog_out.split("\n")))

  if "not found in device name" in run_prog_out or "No matching platform or device found" in run_prog_out:
      print("Mismatch in device name (aborting all further runs)")
      print(run_prog_out)
      try:
          os.remove(args.output)
      except IOError:
          pass
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
