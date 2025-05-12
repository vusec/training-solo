#!/bin/bash
set -e

TEST="${TEST:=default}"

OWN_DIR=`dirname $0`
cd ${OWN_DIR}/user

make -B ARCH=$ARCH TEST=$TEST

taskset -c 0 ./main || (dmesg && dmesg -C)
