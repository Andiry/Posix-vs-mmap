#!/usr/bin/python

import re
import sys
import os

def main():
  if len(sys.argv) < 2:
    print("paramemter error")
    sys.exit(0)
  filename = sys.argv[1]
  print("processing", filename, "...")
  local_count = 0
  count = 0

  fd = open(filename, 'r')
  lines = fd.readlines()
  for line in lines:
    if "BeamSearchStepOp" in line:
      local_count += 1
    elif "finishes" in line:
      print(local_count)
      local_count = 0
      count += 1
  print("%d records" % count)

if __name__ == '__main__':
  main()
