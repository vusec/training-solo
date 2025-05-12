#!/bin/bash
set -e

if [[ $EUID -ne 0 ]]; then
   echo "Please run as root"
   exit 1
fi

echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

make

rmmod its_vmm_host_module || true
dmesg -C
taskset -c 0 insmod its_vmm_host_module.ko
dmesg
