#!/bin/sh

echo "Make ext2 on /dev/ram1.."
mkfs.ext2 -b 4096 /dev/ram0

mkdir /mnt/ramdisk

mount -t ext2 -o xip /dev/ram0 /mnt/ramdisk

echo "Mount to /mnt/ramdisk."
