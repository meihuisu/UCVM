#!/bin/sh

PREFIX_DIR=$1

# Model install paths
CENCAL_DIR=/home/scec-00/patrices/opt/aftershock/cencalvm-0.6.6
CVMH_DIR=/home/scec-00/patrices/opt/aftershock/cvmh-11.9.0
CVMS_DIR=/home/scec-00/patrices/opt/aftershock/cvms
CVMSI_DIR=/home/scec-00/patrices/opt/aftershock/cvmsi
CVMNCI_DIR=/home/scec-00/patrices/opt/aftershock/cvmnci
WFCVM_DIR=/home/scec-00/patrices/opt/aftershock/wfcvm
CVMLT_DIR=/home/scec-00/patrices/opt/aftershock/cvmlt
CMRG_DIR=/home/scec-00/patrices/opt/aftershock/cvm-cmrg
TAPE_DIR=/home/scec-00/patrices/opt/aftershock/cvm-tape

# Etree files
CENCAL_MODEL_DIR=/home/scec-00/patrices/opt/etree/USGSBayAreaVM-08.3.0.etree
CENCAL_EXTMODEL_DIR=/home/scec-00/patrices/opt/etree/USGSBayAreaVMExt-08.3.0.etree

# Etree and Proj.4 install paths
ETREE_DIR=/home/scec-00/patrices/opt/aftershock/euclid3-1.2
PROJ4_DIR=/home/scec-00/patrices/opt/aftershock/proj-4.7.0
NETCDF_DIR=/home/scec-00/patrices/opt/aftershock/netcdf-4.1.1

# Model install paths
#CENCAL_DIR=/home/rcf-104/patrices/opt/cencalvm-0.6.6
#CVMH_DIR=/home/rcf-104/patrices/opt/cvmh-11.9.0
#CVMS_DIR=/home/rcf-104/patrices/opt/cvms
#CVMSI_DIR=/home/rcf-104/patrices/opt/cvmsi
#CVMNCI_DIR=/home/rcf-104/patrices/opt/cvmnci
#WFCVM_DIR=/home/rcf-104/patrices/opt/wfcvm
#CVMLT_DIR=/home/rcf-104/patrices/opt/cvmlt
#CMRG_DIR=/home/rcf-104/patrices/opt/cvm-cmrg
#TAPE_DIR=/home/rcf-104/patrices/opt/cvm-tape

# Etree files
#CENCAL_MODEL_DIR=/home/rcf-104/patrices/opt/etree/USGSBayAreaVM-08.3.0.etree
#CENCAL_EXTMODEL_DIR=/home/rcf-104/patrices/opt/etree/USGSBayAreaVMExt-08.3.0.etree

# Etree and Proj.4 install paths
#ETREE_DIR=/home/rcf-104/patrices/opt/euclid3-1.2
#PROJ4_DIR=/home/rcf-104/patrices/opt/proj-4.7.0
#NETCDF_DIR=/home/rcf-104/patrices/opt/netcdf-4.1.1

# Model install paths
#CENCAL_DIR=/lustre/scratch/patricks/opt/cencalvm-0.6.6-iobuf
#CVMH_DIR=/lustre/scratch/patricks/opt/cvmh-11.9.0
#CVMS_DIR=/lustre/scratch/patricks/opt/cvms
#CVMSI_DIR=/lustre/scratch/patricks/opt/cvmsi
#CVMNCI_DIR=/lustre/scratch/patricks/opt/cvmnci
#WFCVM_DIR=/lustre/scratch/patricks/opt/wfcvm
#CVMLT_DIR=/lustre/scratch/patricks/opt/cvmlt
#CMRG_DIR=/lustre/scratch/patricks/opt/cvm-cmrg
#TAPE_DIR=/lustre/scratch/patricks/opt/cvm-tape

# Etree files
#CENCAL_MODEL_DIR=/lustre/scratch/patricks/opt/etree/USGSBayAreaVM-08.3.0.etree
#CENCAL_EXTMODEL_DIR=/lustre/scratch/patricks/opt/etree/USGSBayAreaVMExt-08.3.0.etree

# Etree and Proj.4 install paths
#ETREE_DIR=/lustre/scratch/patricks/opt/euclid3-1.2-iobuf
#PROJ4_DIR=/lustre/scratch/patricks/opt/proj-4.7.0


# Recommended compilers/flags
# Kraken/Jaguar
#CC=cc
#HOST_OPTIONS="--enable-static --enable-iobuf"
#HOST_CFLAGS="-D_UCVM_MODEL_CVMH_11_2_0"
#HOST_CFLAGS=""
#HOST_LDFLAGS=""
#
# Ranger
#CC=mpicc
#HOST_OPTIONS="--enable-static"
#HOST_CFLAGS="-D_UCVM_MODEL_CVMH_11_2_0"
#HOST_CFLAGS=""
#HOST_LDFLAGS=""
#
# HPCC
#CC=mpicc
#HOST_OPTIONS=""
#HOST_OPTIONS="--enable-netcdf --with-netcdf-include-path=${NETCDF_DIR}/include --with-netcdf-lib-path=${NETCDF_DIR}/lib"
#HOST_CFLAGS="-D_UCVM_MODEL_CVMH_11_2_0"
#HOST_CFLAGS=""
#HOST_LDFLAGS=""
#
# Aftershock
CC=gcc
#HOST_OPTIONS="--enable-static --enable-netcdf --with-netcdf-include-path=${NETCDF_DIR}/include --with-netcdf-lib-path=${NETCDF_DIR}/lib"
HOST_OPTIONS="--enable-netcdf --with-netcdf-include-path=${NETCDF_DIR}/include --with-netcdf-lib-path=${NETCDF_DIR}/lib"
#HOST_CFLAGS="-D_UCVM_MODEL_CVMH_11_2_0"
HOST_CFLAGS=""
HOST_LDFLAGS=""


# Enable all models
HOST_OPTIONS="${HOST_OPTIONS} --enable-model-cvmh --enable-model-cvms --enable-model-cencal --enable-model-cvmsi --enable-model-cvmnci --enable-model-wfcvm --enable-model-cvmlt --enable-model-cmrg --enable-model-tape"


# Run configuration script
cd ../..
./configure --prefix=$1 ${HOST_OPTIONS} CFLAGS="${HOST_CFLAGS}" LDFLAGS="${HOST_LDFLAGS}" CC=${CC} --with-etree-include-path=${ETREE_DIR}/libsrc --with-etree-lib-path=${ETREE_DIR}/libsrc --with-proj4-include-path=${PROJ4_DIR}/include --with-proj4-lib-path=${PROJ4_DIR}/lib --with-cencal-include-path=${CENCAL_DIR}/include/cencalvm/query --with-cencal-lib-path=${CENCAL_DIR}/lib --with-cvmh-include-path=${CVMH_DIR}/include --with-cvmh-lib-path=${CVMH_DIR}/lib --with-gctpc-lib-path=${CVMH_DIR}/gctpc/lib --with-cvms-include-path=${CVMS_DIR}/include --with-cvms-lib-path=${CVMS_DIR}/lib --with-cvmsi-include-path=${CVMSI_DIR}/include --with-cvmsi-lib-path=${CVMSI_DIR}/lib --with-cvmnci-include-path=${CVMNCI_DIR}/include --with-cvmnci-lib-path=${CVMNCI_DIR}/lib --with-wfcvm-include-path=${WFCVM_DIR}/include --with-wfcvm-lib-path=${WFCVM_DIR}/lib --with-cvmlt-include-path=${CVMLT_DIR}/include --with-cvmlt-lib-path=${CVMLT_DIR}/lib --with-cmrg-include-path=${CMRG_DIR}/include --with-tape-include-path=${TAPE_DIR}/include --with-cmrg-lib-path=${CMRG_DIR}/lib --with-tape-lib-path=${TAPE_DIR}/lib --with-cencal-model-path=${CENCAL_MODEL_DIR} --with-cencal-extmodel-path=${CENCAL_EXTMODEL_DIR} --with-cvmh-model-path=${CVMH_DIR}/model --with-cvms-model-path=${CVMS_DIR}/src --with-cvmsi-model-path=${CVMSI_DIR}/model/i8 --with-cvmnci-model-path=${CVMNCI_DIR}/model/i2 --with-wfcvm-model-path=${WFCVM_DIR}/src --with-cvmlt-model-path=${CVMLT_DIR}/model --with-cmrg-model-path=${CMRG_DIR}/model/cmrg.conf --with-tape-model-path=${TAPE_DIR}/model/m16

