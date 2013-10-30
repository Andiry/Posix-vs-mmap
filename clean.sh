#!/bin/sh

echo Umount ramdisk and remove module...
sudo umount /mnt/ramdisk
sudo rmmod memudisk

sleep 1

RD_SIZE=$[46*1024*1024]
echo Mount memudisk with $RD_SIZE KB...
QUIET=/dev/null

pushd ${BEE3HOME}/Tools/KernelModules/Ramdisk/ >$QUIET
sudo insmod memudisk.ko rd_nr=1 rd_size=$RD_SIZE
popd > $QUIET
sleep 1

sudo chmod a+rw /dev/memuram0
echo Fill the memudisk...
dd if=/dev/zero of=/dev/memuram0 bs=1M count=47104 oflag=direct
sleep 1

