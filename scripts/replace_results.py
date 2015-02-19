# This script takes two files, original.csv and replacements.csv.  The idea is that replacements.csv should
# contain a number of test results, such that for each test original.csv has an existing result.  The script
# prints csv that is identical to original.csv, except that whenever original.csv and replacements.csv both
# have a result for a given test, the result from replacements.csv is printed.
#
# The use case for this is that it can be useful to rerun a number of tests sometimes.  For example, with
# Intel OCL compilers we have found there is an issue with the parentheses limit being exceeded; we may
# wish to re-run the relevant tests in those cases


import sys

def read_results(infile):
  with open(infile) as f:
    lines = f.readlines()
  lines_index = 0
  result = []
  while lines_index < len(lines):
    if "RESULTS FOR" in lines[lines_index]:
      testname = lines[lines_index]
    testresult = ""
    lines_index = lines_index + 1
    while lines_index < len(lines) and (not ("RESULTS FOR" in lines[lines_index])):
      testresult += lines[lines_index]
      lines_index = lines_index + 1
    result += [ [ testname, testresult ] ]
  return result

if not (len(sys.argv) == 3):
  print "Usage: " + sys.argv[0] + " original-file replacements-file"
  exit(1)

original_results = read_results(sys.argv[1])
replacement_results = read_results(sys.argv[2])

for result in original_results:
  found = False
  for replacement in replacement_results:
    if result[0] == replacement[0]:
      sys.stdout.write(replacement[0])
      sys.stdout.write(replacement[1])
      found = True
      break
  if not found:
    sys.stdout.write(result[0])
    sys.stdout.write(result[1])

