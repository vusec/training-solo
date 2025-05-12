#!/bin/bash
set -e

TEST="${TEST:=default}"

cd user

make -B ARCH=$ARCH TEST=$TEST

taskset -c 0 ./main || (dmesg && dmesg -C)
