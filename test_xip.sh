#!/bin/sh

echo "Make ext2 on /dev/ram1.."
sudo mke2fs -b 4096 /dev/memuram0
sudo chmod a+rw /dev/memuram0

mkdir /mnt/ramdisk

sudo mount -t ext2 -o xip /dev/memuram0 /mnt/ramdisk
sudo chmod a+rw /mnt/ramdisk

echo "Mount to /mnt/ramdisk."
