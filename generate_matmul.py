#!/usr/bin/python

import re
import sys
import os

def main():
  if len(sys.argv) < 2:
    print "paramemter error"
    sys.exit(0)
  filename = sys.argv[1]
  print "processing", filename, "..."
  sum1 = 0
  count = 0

  fd = open(filename, 'r')
  lines = fd.readlines()
  for line in lines:
    words = line.strip().split(',')
    if len(words) != 3:
      continue
    count += 1
    config = 'MatMul(m=' + words[0].replace(' ', '')\
             + ',n=' + words[1].replace(' ','')\
             + ',k=' + words[2].replace(' ','')\
             + ',a_tran=false,b_tran=false,type=DT_FLOAT)'
    print(config)
  print("%d records" % count)

if __name__ == '__main__':
  main()
