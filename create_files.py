#!/usr/bin/python

import re
import sys
import os

def main():
	if len(sys.argv) < 3:
		print "paramemter error"
		sys.exit(0)
	os.system("mkdir /mnt/ramdisk/footprint100000")
	dir = "/mnt/ramdisk/footprint100000/log.00000"
	num = int(sys.argv[1])
	size = int(sys.argv[2])
	
	for i in range(num):
		filename = dir + str(i)
		fd1 = open(filename, 'w')
		fd1.write(size * 'a')
		fd1.close()
	print "done."
	return

main()
