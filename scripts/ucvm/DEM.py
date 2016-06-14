#!/bin/env python

# Basic modules
import os
import sys
import array
import shutil

# Patrick's modules
from Shell import *


class DEM:
    def __init__(self, workdir, executable, neddir, bathdir = None):
        self.workdir = workdir
        self.executable = executable
        self.neddir = neddir
	self.bathdir = bathdir
        return;

    def run(self, inputfile, outputfile):

        inputpath = os.path.abspath(inputfile)
        outputpath = os.path.abspath(outputfile)
        
        # Save the start directory
        startdir = os.getcwd()

        # Change dir to the GRD codes
        print "Changing dir to %s" % (self.workdir)
        os.chdir(self.workdir)

        # Execute GRD codes
        if (self.bathdir != None):
            cmd = ['./%s' % (self.executable), '-e', '-d', self.neddir,\
                       '-b', self.bathdir, \
                       inputpath, outputpath,]
        else:
            cmd = ['./%s' % (self.executable), '-e', '-d', self.neddir,\
                       inputpath, outputpath,]
        print "Executing cmd: %s" % (str(cmd))
        shell = Shell(cmd)
        shell.runCommand()
        retcode = shell.getReturnCode()
        output = shell.getOutput()

        # Change dir to start dir
        print "Changing dir back to %s" % (startdir)
        os.chdir(startdir)
        if (output != None):
            output = output.splitlines()
        if (retcode != 0):
            print "Failed to generate GRD outputfile"
            print "Output: %s" % (str(output))
            return 1

        return 0

