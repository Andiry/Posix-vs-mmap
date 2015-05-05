#!/usr/bin/python

import re
import sys
import os
import time

def main():
	if len(sys.argv) < 2:
		print "paramemter error: #num"
		sys.exit(0)
	os.system("mkdir /mnt/ramdisk/dir")
	dir = "/mnt/ramdisk/dir/log000"
	num = int(sys.argv[1])

	print "%f" % time.time()
	for i in range(num):
		filename = dir + str(i)
		os.system("touch " + filename)
	print "%f" % time.time()
	print "done."
	return

main()
