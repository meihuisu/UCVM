#!/bin/sh


# Enabled models
HOST_OPTIONS="--enable-model-cvmh --enable-model-cvms"


# Run configuration script
cd ../..
./configure --prefix=${UCVM_INSTALL_DIR} ${HOST_OPTIONS} --with-etree-include-path="${ETREE_DIR}/libsrc" --with-etree-lib-path="${ETREE_DIR}/libsrc" --with-proj4-include-path="${PROJ4_DIR}/include" --with-proj4-lib-path="${PROJ4_DIR}/lib" --with-cvmh-include-path="${CVMH_DIR}/include" --with-cvmh-lib-path="${CVMH_DIR}/lib" --with-gctpc-lib-path="${CVMH_DIR}/lib" --with-cvms-include-path="${CVMS_DIR}/include" --with-cvms-lib-path="${CVMS_DIR}/lib" --with-cvmh-model-path="${CVMH_DIR}/model" --with-cvms-model-path="${CVMS_DIR}/src"
