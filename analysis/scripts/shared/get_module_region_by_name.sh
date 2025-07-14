#!/bin/bash
set -e

BINARY=$1
SEGMENT_START=$2
MODULES_NAMES_FILE=$3


SEGMENT_SIZE=`readelf -l $BINARY | grep $SEGMENT_START -A1 | tail -n 1 | awk '{print $1'}`

# >&2 echo "BINARY" $BINARY
# >&2 echo "SEGMENT START" $SEGMENT_START
# >&2 echo "SEGMENT SIZE " $SEGMENT_SIZE


SEGMENT_END=`printf "0x%x\n" $(("$SEGMENT_START" + "$SEGMENT_SIZE"))`

SEGMENT_NAME=`grep $SEGMENT_START modules_text.txt -B 1 | head -n 1 | awk -F'/' '{print $4}'`

echo "$SEGMENT_NAME;$SEGMENT_START;$SEGMENT_END"



