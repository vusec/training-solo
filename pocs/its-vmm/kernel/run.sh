#!/bin/bash
set -e

if [[ $EUID -ne 0 ]]; then
   echo "Please run as root"
   exit 1
fi

make

rmmod execute_caller_vm_module || true
dmesg -C
insmod execute_caller_vm_module.ko
dmesg
