#!/bin/bash

# Process options
FLAGS=""

# Pass along any arguments to grd_query
while getopts 'd:b:ev' OPTION
do
  if [ -z "$OPTARG" ]; then
      FLAGS="${FLAGS} -$OPTION"
  else
      FLAGS="${FLAGS} -$OPTION $OPTARG"
  fi
done
shift $(($OPTIND - 1))


if [ $# -lt 2 ]; then
	printf "Usage: %s: [options] <infile> <outfile>\n" $(basename $0) >&2    
    	exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
IN_FILE=$1
OUT_FILE=$2

echo "${SCRIPT_DIR}/grd_query ${FLAGS} < ${IN_FILE} > ${OUT_FILE}" >> run.log
${SCRIPT_DIR}/grd_query ${FLAGS} < ${IN_FILE} > ${OUT_FILE}
if [ $? -ne 0 ]; then
    exit 1
fi

exit 0
