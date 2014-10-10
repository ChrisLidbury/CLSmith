# TODO ignore fully coherent rows

import subprocess
import sys
import glob
import difflib
import csv
import collections
import os

if len(sys.argv) > 1:
  if not os.path.isdir(sys.argv[1]):
    print("Could not find given directory %s!" % (sys.argv[1]))
    exit(1)
  print("Using %s as target directory." % (sys.argv[1]))
  os.chdir(sys.argv[1])

files = glob.glob("*.csv")
sample_file_name = "sample_results.csv"
output_file_name = "diff_out.html"

index = 0
contents = dict()
results = dict()

for filename in files:
    if filename == sample_file_name:
      continue
    csvfile = open(filename, 'r')
    platform_name = ' '.join(filename.split('.')[0].split('_'))
    contents[platform_name] = csvfile.read().splitlines()
    contents[platform_name] = [s.strip() for s in (filter(None, contents[platform_name]))]
    results[platform_name] = dict()

sample = dict()
vote = dict()
lines = dict()

for platform_name, full_results in contents.items():
  for result in full_results:
    if result.startswith("RESULTS FOR"):
      program_name = result.replace("RESULTS FOR", "").strip()
      if "(" in program_name and program_name not in lines:
        no_lines = program_name.split("(")[1].split(")")[0]
        program_name = program_name.split("(")[0].strip()
        lines[program_name] = no_lines
    else:
      results[platform_name][program_name] = filter(None, result.split(','))
      temp_results = []
      for r in results[platform_name][program_name]:
        r = r.strip()
        if r.startswith("timeout") or r.startswith("run_error") or r.startswith("0x"):
          temp_results.append(r)
        else:
          temp_results.append("0x" + r)
      results[platform_name][program_name] = temp_results
      if program_name in vote.keys():
        if result in vote[program_name].keys():
          vote[program_name][result] = vote[program_name][result] + 1
        else:
          vote[program_name][result] = 1
      else:
        vote[program_name] = {result:1}
        sample[program_name] = []
    
for program_name in vote.keys():
  curr_max = -1
  curr_res = []
  for result, votes in vote[program_name].items():
    if result.startswith("run_error") or result.startswith("timeout"):
      continue
    if votes > curr_max and votes > 2:
      curr_max = votes
      curr_res = [result]
    elif votes == curr_max:
      curr_res.append(result)
  if len(curr_res) != 1:
    curr_res = "Inconclusive"
  else:
    curr_res = curr_res[0]
  sample[program_name] = filter(None, curr_res.split(','))
  temp_sample = []
  for r in sample[program_name]:
    r = r.strip()
    if r.startswith("timeout") or r.startswith("run_error") or r.startswith("0x"):
      temp_sample.append(r)
    else:
      temp_sample.append("0x" + r)
  sample[program_name] = temp_sample
  
sample = collections.OrderedDict(sorted(sample.items()))
contents = collections.OrderedDict(sorted(contents.items()))
results = collections.OrderedDict(sorted(results.items()))

samplefile = open(sample_file_name, 'w')    
for program_name, result in sample.items():
    samplefile.write("SAMPLE RESULT FOR %s\n%s\n" % (program_name, result))
samplefile.close() 
        
output = open(output_file_name, 'w')

cell_width = 200
max_count = 20

output.write("""
<!doctype html>
  <head>
    <title>Compiler testing results</title>
    <style type="text/css">
  
    .fixed-table-container {
      height: 100%;
      position: absolute; /* could be absolute or relative */
      padding-top: 75px; /* height of header */
    }
  
    .fixed-table-container-inner {
      overflow-x: hidden;
      overflow-y: auto;
      height: 100%;
    }
  
    .header-background {
      background-color: #D5ECFF;
      height: 75px; /* height of header */
      position: absolute;
      top: 0;
      right: 0;
      left: 0;
    }
    
    .th-inner {
      position: absolute;
      top: 0;
      text-align: center;
      vertical-align: middle;
      width: 200px;
      height: 75px;
    }
    
    body {
      overflow-y: hidden;
    }
    
    table {
      table-layout: fixed;
      word-wrap: break-word;
      height: 100%;
      overflow-x: hidden;
      overflow-y: auto;\n""")
output.write("      width: " + `(len(contents) + 2) * cell_width` + "px;")
output.write("      padding-bottom: " + `max_count * 30` + "px;")
output.write("""
     padding-right: 35px;
    }
    
    td {
      text-align: center;\n""")
output.write("      width: " + `cell_width` + "px;")
output.write("""
    }
    
    </style>
    
    </head>
    <body>
  """)

output.write("<div class=\"fixed-table-container\">\n")
output.write("<div class=\"header-background\"></div>\n")
output.write("<div class=\"fixed-table-container-inner\">\n")
output.write("<table>\n")
output.write("<tr><th><div class=\"th-inner\">Program</div></th><th><div class=\"th-inner\">Sample</div></th>\n")
for platform_name in contents.keys():
    output.write("<th><div class=\"th-inner\">" + platform_name + "</div></th>\n")
output.write("</tr>\n")
for program_name in sample.keys():
    output.write("<tr><td align=\"center\">" + program_name)
    if program_name in lines:
      output.write("</br>Lines: " + lines[program_name])
    output.write("</td>")
    color = ""
    if sample[program_name] == ["Inconclusive"]:
      color = " style=\"background-color:yellow;\""
    output.write("<td " + color + ">")
    curr_count = 0
    for result in sample[program_name]:
      if (curr_count > max_count):
        output.write("<br/>...")
        break
      output.write(result + "<br/>")
      curr_count += 1
    output.write("</td>")
    for platform_name in results.keys():
        color = ""
        if not program_name in results[platform_name].keys():
            results[platform_name][program_name] = ["N/A"]
        if not sample[program_name] == ["Inconclusive"]:
          if not set(results[platform_name][program_name]) == set(sample[program_name]):
            color = " style=\"background-color:red;\""
        if results[platform_name][program_name][0].startswith("run_error"):
          color = " style=\"background-color:blue;\""
        output.write("<td " + color + ">")
        curr_count = 0
        for result in results[platform_name][program_name]:
          if (curr_count > max_count):
            output.write("<br/>...")
            break
          output.write(result + "<br/>")
          curr_count += 1
        output.write("</td>")
    output.write("</tr>\n")
output.write("</table>\n")
output.write("</div>\n")
output.write("</div>\n")

output.write("""
   </body>
   </html>
   """)
output.close()

os.remove(sample_file_name)