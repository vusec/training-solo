#!/bin/bash
set -e

#  cat kprobe_hist | awk '{print $3}' | tr -d , | sort | uniq -c | sort -nr

DIR=`dirname $0`
BINARY=$1
KPROBE_FILE=$2

# echo $DIR

# cat $KPROBE_FILE | while read line; do
#     address=`echo $line | awk '{print $3}' | tr -d ,`
#     echo $address

#     ${DIR}/output_symbol_for_list.sh ../images/vmlinux-dbg

#     exit0
# done
sorted_list=`cat $KPROBE_FILE | awk '{print $3}' | tr -d , | sort | uniq -c | sort -nr`

while IFS= read -r line; do
    count=`echo $line | awk '{print $1}'`
    address=`echo $line | awk '{print $2}'`

    echo $line | ${DIR}/output_symbol_for_list.sh ../images/vmlinux-dbg


    targets_list=`grep $address $KPROBE_FILE | awk '{print $5}' | tr -d , | sort`

    while IFS= read -r target; do
        symbol=`${DIR}/get_symbol_for_address.sh ../images/vmlinux-dbg $target`

        if [ "$symbol" != "" ]; then

            echo "|-" $target $symbol
        fi

    done <<< "$targets_list"
done <<< "$sorted_list"
