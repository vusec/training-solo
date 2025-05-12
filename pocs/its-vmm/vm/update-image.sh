#!/bin/bash

if [[ $EUID -ne 0 ]]; then
   echo "Please run as root"
   exit 1
fi

sudo mkdir -p /mnt/chroot
sudo mount -o loop bookworm.img /mnt/chroot

cp -r ../../common ./chroot/root/
mkdir -p ./chroot/root/its-vmm

cp -r ../kernel ./chroot/root/its-vmm
cp -r ../user ./chroot/root/its-vmm
cp -r ../*.sh ./chroot/root/its-vmm


cp -a chroot/. /mnt/chroot/.
sudo umount /mnt/chroot
