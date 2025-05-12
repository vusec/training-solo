#!/bin/bash
set -e

if [[ $EUID -ne 0 ]]; then
   echo "Please run as root"
   exit 1
fi

make

rmmod map_region_module || true
dmesg -C
insmod map_region_module.ko
dmesg
