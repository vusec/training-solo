#!/bin/bash
set -e

#  cat kprobe_hist | awk '{print $3}' | tr -d , | sort | uniq -c | sort -nr

BOOTLIN_SEARCH_URL="https://elixir.bootlin.com/linux/v6.8.8/A/ident/"
BOOTLIN_FILE_URL="https://elixir.bootlin.com/linux/v6.8.8/source"
BINARY=$1

while IFS=$'\n' read -r line; do
    count=`echo $line | awk '{print $1}'`
    address=`echo $line | awk '{print $2}'`

    SYMBOL_INFO=`addr2line -e $BINARY -p -f $address`
    SYMBOL_INFO=`tr '\n' ' ' <<< $SYMBOL_INFO`
    SYMBOL_INFO=`sed -e 's/ at / /' <<< $SYMBOL_INFO`
    SYMBOL_INFO=`sed -e 's/build\/linux-hwe-6.8-Wyme5e\/linux-hwe-6.8-6.8.0\///' <<< $SYMBOL_INFO`
    SYMBOL_INFO=`sed -e 's/build\/linux-jgGCUS\/linux-6.8.0\///' <<< $SYMBOL_INFO`

    FUNCTION_NAME=`awk '{print $1}' <<< $SYMBOL_INFO`
    FILE_SUFFIX=`awk '{print $2}' <<< $SYMBOL_INFO`
    FILE_SUFFIX=`sed -e 's/:/#L/' <<< $FILE_SUFFIX`
    echo "$line $SYMBOL_INFO ${BOOTLIN_FILE_URL}${FILE_SUFFIX} ${BOOTLIN_SEARCH_URL}${FUNCTION_NAME}"
done
