#!/bin/bash
set -e

BINARY=$1
SEGMENT_START=$2
SEGMENT_END=$3

objdump -D $BINARY --start-address=$SEGMENT_START --stop-address=$SEGMENT_END
