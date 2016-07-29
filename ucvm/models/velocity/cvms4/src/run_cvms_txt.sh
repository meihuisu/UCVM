#!/bin/bash

IN_FILE=$1
OUT_FILE=$2

./cvms_txt < ${IN_FILE} > ${OUT_FILE}
if [ $? -ne 0 ]; then
    exit 1
fi

exit 0
