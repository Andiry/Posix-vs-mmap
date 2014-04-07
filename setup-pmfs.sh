#!/bin/sh

umount /mnt/ramdisk
rmmod pmfs
modprobe pmfs

sleep 1

mount -t pmfs -o physaddr=0x100000000,init=2G none /mnt/ramdisk

sleep 1
#cp test1 /mnt/ramdisk/
#dd if=/dev/zero of=/mnt/ramdisk/test1 bs=1M count=1024 oflag=direct
