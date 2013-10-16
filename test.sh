#!/bin/sh

echo "Make ext4 on /dev/ram1.."
mkfs.ext4 /dev/ram1

mkdir /mnt/ramdisk

mount -t ext4 /dev/ram1 /mnt/ramdisk

echo "Mount to /mnt/ramdisk."
