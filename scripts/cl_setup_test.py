#!/usr/bin/python

""" Script that copies over all the files required in order to run CLSmith
for random program generation and execution. It takes one argument, a path
relative to $HOME, where all the files should be copied. The script must be
executed from within the "scripts" folder of CLSmith """

import sys
import os
import shutil

srcCLSmith = ".." + os.sep + "src" + os.sep + "CLSmith" + os.sep
runtime      = ".." + os.sep + "runtime" + os.sep

clLauncherExecutable = "cl_launcher"
if sys.platform == "win32":
  clLauncherExecutable += ".exe"

makeGen = not (len(sys.argv) == 3 and sys.argv[2] == "--no-gen")

if  len(sys.argv) == 1:
    print("Expected one arg (directory).")
    sys.exit()
elif makeGen and not os.path.exists(srcCLSmith + "CLSmith"):
    print("Build generator first")
    sys.exit()
elif not os.path.exists(srcCLSmith + clLauncherExecutable):
    print("Build launcher first.")
    sys.exit()
elif not os.path.exists(runtime + "CLSmith.h"):
    print("Cannot find CLSmith.h in " + runtime)
    sys.exit()
elif not os.path.exists(runtime + "cl_safe_math_macros.h"):
    print("Cannot find cl_safe_math_macros.h in " + runtime)
    sys.exit()
elif not os.path.exists(runtime + "safe_math_macros.h"):
    print("Cannot find safe_math_macros.h in " + runtime)
    sys.exit()
elif not os.path.exists("cl_get_and_test.py"):
    print("Cannot find cl_get_and_test.py")
    sys.exit()
elif not os.path.exists("processTimeout.py"):
    print("Cannot find processTimeout.py")
    sys.exit()


dir = os.path.expanduser("~") + os.sep + sys.argv[1]

if os.path.exists(dir):
    overwrt = raw_input("Test directory already exists. Type 'y' to overwrite: ")
    if (overwrt == 'y'):
      print("Deleting existing directory %s." % (dir))
      shutil.rmtree(dir)
    else:
      print("Aborting test setup.")
      sys.exit()

os.mkdir(dir)
if makeGen:
  shutil.copy(srcCLSmith + "CLSmith", dir)
shutil.copy(srcCLSmith + clLauncherExecutable, dir)
shutil.copy(runtime + "CLSmith.h", dir)
shutil.copy(runtime + "cl_safe_math_macros.h", dir)
shutil.copy(runtime + "safe_math_macros.h", dir)
shutil.copy("cl_get_and_test.py", dir)
shutil.copy("processTimeout.py", dir)

