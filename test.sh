#!/bin/sh

echo "Make ext2 on /dev/ram1.."
mkfs.ext2  /dev/ram0

mkdir /mnt/ramdisk

mount -t ext2  /dev/ram0 /mnt/ramdisk

echo "Mount to /mnt/ramdisk."
