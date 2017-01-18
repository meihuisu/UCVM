Linux
=====

Fedora 25
---------

In order to install UCVM on Fedora, a few required packages must be installed first.

sudo yum install freetype-devel
sudo yum install python3-devel make automake gcc gcc-c++ gcc-gfortran
sudo yum install redhat-rpm-config
sudo yum install subverison
sudo yum install hdf5 hdf5-devel

To also add MPI support:

sudo yum install openmpi openmpi-devel

Ubuntu 16.04

sudo apt install git
sudo apt install libfreetype6-dev
apt install python3-dev
apt install libhdf5-serial-dev

