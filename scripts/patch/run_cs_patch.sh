#!/bin/bash

cd ../src/patch

./patchmodel -f ../../conf/aftershock/cencal_p1.conf
./patchmodel -f ../../conf/aftershock/cencal_p2.conf
./patchmodel -f ../../conf/aftershock/cencal_p3.conf
./patchmodel -f ../../conf/aftershock/cencal_p4.conf
./patchmodel -f ../../conf/aftershock/cvmh_p1.conf
./patchmodel -f ../../conf/aftershock/cvmh_p2.conf
./patchmodel -f ../../conf/aftershock/cvmh_p3.conf
./patchmodel -f ../../conf/aftershock/cvmh_p4.conf

