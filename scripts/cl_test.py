#! python
#! /usr/bin/python

import sys
import os
import argparse
from processTimeout import Command

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

parser.add_argument('-seed_min', default = 1, type = int)
parser.add_argument('-seed_max', default = 2, type = int)
parser.add_argument('-seed_step', default = 1, type = int)

parser.add_argument('-cl_launcher', default = "." + os.sep + "cl_launcher")
parser.add_argument('-cl_filename', default = "CLProg.c")
parser.add_argument('-cl_platform_idx', default = 0, type = int)
parser.add_argument('-cl_device_idx', default = 0, type = int)

parser.add_argument('-output', default = "Result.csv")
parser.add_argument('-gen_exe', default = "." + os.sep + "CLSmith")

parser.add_argument('flags', nargs='*')

args = parser.parse_args()

if os.path.isfile(args.output):
    print("Overwriting file %s." % args.output)
   
output = open(args.output, 'w')
for seed in range(args.seed_min, args.seed_max, args.seed_step):
    if os.path.isfile(args.cl_filename):
        os.remove(args.cl_filename)
    gen_prog_string = "%s --seed %d" % (args.gen_exe, seed)
    if args.flags:
      gen_prog_string += " " + " ".join(args.flags)
    gen_prog = Command(gen_prog_string)
    gen_prog_res = gen_prog.run(timeout = 30)
    if not os.path.isfile(args.cl_filename) or not gen_prog_res[0] == 0:
        output.write("gen_error")
        continue
    run_prog = Command("%s %s %d %d" % (args.cl_launcher, args.cl_filename, args.cl_platform_idx, args.cl_device_idx)) 
    run_prog_res = run_prog.run(timeout = 150)
    if not run_prog_res[0] == 0:
        output.write("run_error")
        continue
    
    run_prog_out = run_prog_res[1].split('\n')[1]
    run_prog_out = run_prog_out.split(',')
    run_prog_out = set(run_prog_out)
    
    if len(run_prog_out) > 1:
        output.write("Different.\n")
    else:
        output.write(run_prog_out[0] + "\n")
