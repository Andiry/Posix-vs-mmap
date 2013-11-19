#!/bin/sh

modprobe pmfs

sleep 1

mount -t pmfs -o physaddr=0x100000000,init=2G none /mnt/ramdisk
