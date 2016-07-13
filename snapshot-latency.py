#!/usr/bin/python

import re
import sys
import os
import time

def main():
	if len(sys.argv) < 2:
		print "paramemter error: #FS"
		sys.exit(0)
	os.system("rm -rf /mnt/ramdisk/dir")
	fs = sys.argv[1]
	time1 = time.time()

	print "%f" % time1
	if fs == "NOVA":
		os.system("echo 1 > /proc/fs/NOVA/pmem0m/create_snapshot")
	elif fs == "btrfs":
		os.system("btrfs subvolume snapshot /mnt/ramdisk /mnt/ramdisk/snapshot")
	elif fs == "nilfs2":
		os.system("mkcp -s")
	time2 = time.time()

	print "%f" % time2
	print "Time: %f" % (float(time2) - float(time1))
	print "done."
	return

main()
