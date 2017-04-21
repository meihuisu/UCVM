#!/bin/bash
#PBS -l nodes=1:ppn=1
#PBS -l walltime=02:00:00

source /usr/usc/python/3.5.2/setup.sh
source /usr/usc/openmpi/default/setup.sh

PATH_TO_UCVM=/staging/path/to/your/ucvm-venv

source $PATH_TO_UCVM/bin/activate

LIB_PATH=$PATH_TO_UCVM/lib/python3.5/site-packages/ucvm-17.2.0-py3.5.egg/ucvm/libraries
for d in $(find $LIB_PATH -mindepth 1 -maxdepth 1 -type d); do
        LD_LIBRARY_PATH="$d/lib:$LD_LIBRARY_PATH"
done

MODEL_PATH=$PATH_TO_UCVM/lib/python3.5/site-packages/ucvm-17.2.0-py3.5.egg/ucvm/models
echo $(find $MODEL_PATH -mindepth 1 -maxdepth 1 -type d)
for d in $(find $MODEL_PATH -mindepth 1 -maxdepth 1 -type d); do
        LD_LIBRARY_PATH="$d/lib:$LD_LIBRARY_PATH"
done

NP=`wc -l < $PBS_NODEFILE`
echo "Running on $NP processors: "`cat $PBS_NODEFILE`

cd ..
mpirun -n 1 python3 release.py

echo "Done " `date`
exit 0