#!/usr/bin/python

import string
import sys

vals = sys.argv[1].split(',')
vals.pop()
total = 0
for val in vals:
  total += int(val, 16)
print hex(total)
