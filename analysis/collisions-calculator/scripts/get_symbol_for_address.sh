#!/bin/bash
set -e

BINARY=$1
ADDRESS=$2
BOOTLIN_SEARCH_URL="https://elixir.bootlin.com/linux/v6.8.8/A/ident/"
BOOTLIN_FILE_URL="https://elixir.bootlin.com/linux/v6.8.8/source"


SYMBOL_INFO=`addr2line -i -p -e $BINARY -f $ADDRESS`
SYMBOL_INFO=`sed -e 's/build\/linux-hwe-6.8-Wyme5e\/linux-hwe-6.8-6.8.0\///' <<< $SYMBOL_INFO`
SYMBOL_INFO=`sed -e 's/build\/linux-jgGCUS\/linux-6.8.0\///' <<< $SYMBOL_INFO`
SYMBOL_INFO=`sed -e 's/ at / /' <<< $SYMBOL_INFO`

FUNCTION_NAME=`awk '{print $1}' <<< $SYMBOL_INFO`
FILE_SUFFIX=`awk '{print $2}' <<< $SYMBOL_INFO`
FILE_SUFFIX=`sed -e 's/:/#L/' <<< $FILE_SUFFIX`

if [ "$SYMBOL_INFO" != "?? ??:0" ]; then
    echo $ADDRESS `tr '\n' ' ' <<< $SYMBOL_INFO`
    # "${BOOTLIN_FILE_URL}${FILE_SUFFIX} ${BOOTLIN_SEARCH_URL}${FUNCTION_NAME}"
fi

# echo `tr '\n' ' ' <<< $SYMBOL_INFO` "${BOOTLIN_URL}${FUNCTION_NAME}"
