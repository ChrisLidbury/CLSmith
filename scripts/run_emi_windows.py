import subprocess
import os

platform = "0"
device = "0"
optimizations = False

probabilities = [ 0, 33, 67, 100]

for x in probabilities:
  for y in probabilities:
    for z in probabilities:
      testsdir = "basic_viar_emi_leaf_" + str(x) + "_compound_" + str(y) + "_lift_" + str(z)
      if os.path.isdir("emi/" + testsdir):
        subprocess.call(["python", "-u", "cl_get_and_test.py", "-cl_platform_idx", platform, "-cl_device_idx", device, "-path", "emi/" + testsdir, "-timeout", "60", "-output", "emi_results/" + testsdir + ".csv"] + ([] if not optimizations else [ "--", "---disable_opts"]))

