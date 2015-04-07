#!/usr/bin/python

import re
import sys
import os

def main():
	if len(sys.argv) < 3:
		print "paramemter error: #num #size"
		sys.exit(0)
	os.system("mkdir /mnt/ramdisk/")
	dir = "/mnt/ramdisk/log000"
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
