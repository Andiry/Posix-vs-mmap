#!/usr/bin/python

import re
import sys
import os
import time

def main():
	time1 = time.time()
	os.system("cp ~/Downloads/linux-4.10.6.tar.xz /mnt/ramdisk/")
	os.chdir("/mnt/ramdisk")
	os.system("tar xf linux-4.10.6.tar.xz")
	os.system("cp ~/Downloads/4.10-config linux-4.10.6/.config")
	os.chdir("/mnt/ramdisk/linux-4.10.6")
	time2 = time.time()


	time3 = time.time()

	os.system("make -j8")

	time4 = time.time()

	print "Prepare time: %f" % (float(time2) - float(time1))
	print "Make time: %f" % (float(time4) - float(time3))
	print "done."
	return

main()
