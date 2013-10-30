#!/usr/bin/python
#coding=utf-8

import time
import os
import sys

def do_work(XIP, filesize, filename):
	i = 0
	while i < 10:
		os.system('./write_to_ram %s %s %s %s' %(XIP, '0', filesize, filename))
		os.system('./mmap_to_ram %s %s %s %s' %(XIP, '0', filesize, filename))
#		print XIP + '0' + filename
		i += 1
	i = 0
	while i < 10:
		os.system('./run_nvp ./write_to_ram %s %s %s %s' %(XIP, '1', filesize, filename))
		os.system('./run_nvp ./mmap_to_ram %s %s %s %s' %(XIP, '1', filesize, filename))
#		print XIP + '1' + filename
		i += 1

def main():
	date = time.strftime("%Y-%m-%d_%X", time.localtime(time.time()))
	filename = "./results/results_" + date + ".csv"
	print filename
	f = open(filename, 'w')
	f.write("Type,XIP,request_size,file_size,count,time (ns),Bandwidth (GB/s)\n")
	file.close(f)

	file_sizes = ['4194304', '134217728', '4294967296']
	for filesize in file_sizes:
		os.system('sh clean.sh')
		time.sleep(1)
		print "Mount without XIP..."
		os.system('sh test.sh')
		time.sleep(1)
		print "Performing test..."
		do_work('0', filesize, filename)
		time.sleep(1)

		os.system('sh clean.sh')
		time.sleep(1)
		print "Mount with XIP..."
		os.system('sh test_xip.sh')
		time.sleep(1)
		print "Performing test..."
		do_work('1', filesize, filename)
		time.sleep(1)

main()
