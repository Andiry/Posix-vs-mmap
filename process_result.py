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
		words = line.strip().split()
		if len(words) < 5:
			continue
		if words[2] == 'us' and words[4] == 'vfs_write();':
			if float(words[1]) < 5:
				sum1 += float(words[1])
				count += 1
	print "count:", count, "latency:", sum1 / count, "us"


main()
