#!/usr/bin/python

import re
import sys
import os
import time

def main():
	if len(sys.argv) < 3:
		print "paramemter error: #FS #num"
		sys.exit(0)
	os.system("rm -rf /mnt/ramdisk/dir")
	fs = sys.argv[1]
	num = int(sys.argv[2])
	count = 0

	while count < num:
		print "Making snapshot " + str(count) + "."
		if fs == "NOVA":
			os.system("echo 1 > /proc/fs/NOVA/pmem0m/create_snapshot")
		elif fs == "btrfs":
			os.system("btrfs subvolume snapshot /mnt/ramdisk /mnt/ramdisk/snapshot-%d" % count)
		elif fs == "nilfs2":
			os.system("mkcp -s")
		count = count + 1
		time.sleep(1)

	print "done."
	return

main()
