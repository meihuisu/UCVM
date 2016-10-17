#!/bin/bash

if [ $# -lt 2 ]; then
	printf "Usage: %s: <infile> <outfile>\n" $(basename $0) >&2    
        exit 1
fi


SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
IN_FILE=$1
OUT_FILE=$2

${SCRIPT_DIR}/vx < ${IN_FILE} > ${OUT_FILE}
if [ $? -ne 0 ]; then
    exit 1
fi

exit 0
