Blue Waters (HPC)
=================

module load bwpy
module load bwpy-mpi
module load cray-hdf5-parallel
module swap PrgEnv-cray PrgEnv-gnu

export CC="cc"

pyvenv-3.5 ucvm-17.1.0 --system-site-packages
