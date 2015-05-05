#!/usr/bin/python

import re
import sys
import os
import time

def main():
	if len(sys.argv) < 3:
		print "paramemter error: #num #size"
		sys.exit(0)
	os.system("mkdir /mnt/ramdisk/dir")
	dir = "/mnt/ramdisk/dir/log000"
	num = int(sys.argv[1])
	size = int(sys.argv[2])

	print "%f" % time.time()
	for i in range(num):
		filename = dir + str(i)
		fd1 = open(filename, 'w')
		if size > 0:
			fd1.write(size * 'a')
		fd1.close()
	print "%f" % time.time()
	print "done."
	return

main()
