import subprocess
import sys
import glob
import difflib
import csv

files = glob.glob("*.csv")
sample_name = "sample.csv"
output_name = "diff_out.html"

index = 0
contents = dict()

for filename in files:
    if filename == sample_name:
        continue
    csvfile = open(filename, 'r')
    contents[filename] = csvfile.read().splitlines()
    index += 1
    
no_lines = max(len(file) for file in contents.values())
ignored = {name:file for name, file in contents.items() if not len(file) is no_lines}
contents = {name:file for name, file in contents.items() if not name in ignored}

sample = dict()

for line_idx in range(no_lines):
    vote = dict()
    for file in contents.values():
        if file[line_idx] == "run_error":
            continue
        if file[line_idx] in vote:
            vote[file[line_idx]] += 1
        else:
            vote[file[line_idx]] = 1
    if not vote:
        sample[line_idx] = "run_error"
    else:
        vote = {v:k for k,v in vote.items()}
        sample[line_idx] = vote[max(vote.keys(), key=int)]
    
samplefile = open(sample_name, 'w')    
for v in sample.values():
    samplefile.write(v + "\n")
   
diff = dict()
file_idx = -1
for name, file in contents.items():
    line_idx = 0
    for line in file:
        if not line == sample[line_idx]:
            if not name in diff:
                diff[name] = [line_idx]
            else: 
                diff[name].append(line_idx)
        line_idx += 1
    if not name in diff:
        diff[name] = "same"
        
output = open(output_name, 'w')

output.write("<table>\n")
output.write("<tr><th>Line</th><th>Sample</th>")
for name in contents.keys():
    output.write("<th>" + name + "</th>")
output.write("</tr>\n")
for line_idx in range(no_lines):
    output.write("<tr><td>" + str(line_idx + 1) + "</td>")
    output.write("<td>" + sample[line_idx] + "</td>")
    for file in contents.values():
        color = ""
        if not file[line_idx] == sample[line_idx]:
            color = " style=\"background-color:red;\""
        output.write("<td" + color + ">" + file[line_idx] + "</td>")
    output.write("</tr>\n")
    line_idx += 1
output.write("</table>\n")