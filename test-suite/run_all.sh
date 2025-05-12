#!/bin/bash
set -e

OWN_DIR=`dirname $0`



# for file in $OWN_DIR/user/templates/*$1*.h
# do
#     echo "================================="
#     echo $file
#     # python3 analyze-spec-window-result.py $file
# done

first=1

while read file; do

    # We skip the randomization tests
    if [[ $file == *"/randomization/"* ]]; then
        continue
    fi

    template=`echo $file | grep -o '[^/]*\.h' | sed 's/\.h//g'`
    printf '>%.0s' {1..40} && printf '<%.0s' {1..40} && printf '\n'
    echo "||" $template "#1"
    printf '>%.0s' {1..40} && printf '<%.0s' {1..40} && printf '\n'

    # Lets only print mitigation info the first test
    if [ $first -eq 0 ]; then
        EXTRA=-DSUPSRESS_MITIGATION_INFO
    fi
    first=0

    ARCH=$ARCH TEST=$template EXTRA=$EXTRA ${OWN_DIR}/run.sh

    printf '>%.0s' {1..40} && printf '<%.0s' {1..40} && printf '\n'
    echo "||" $template "#2"
    printf '>%.0s' {1..40} && printf '<%.0s' {1..40} && printf '\n'
    ARCH=$ARCH TEST=$template EXTRA=$EXTRA ${OWN_DIR}/run.sh

done <<< "$(find user/templates/*$1* -name '*.h' | sort)"
