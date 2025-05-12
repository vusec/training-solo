#!/bin/bash
set -e

OWN_DIR=`dirname $0`

echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

cd ${OWN_DIR}/kernel
make clean
./run.sh

cd ../../../common/kmmap/kernel
make clean
./run.sh

modprobe msr
