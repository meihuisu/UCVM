#!/usr/bin/env python

import inspect
import os
import math
import array
import sys
from subprocess import call, Popen, PIPE, STDOUT

def test_vs30_query(dir):
    # Basic vs30 test.
    curdir = os.getcwd()
    os.chdir(dir)
    
    proc = Popen(["./bin/vs30_query", "-f", "./conf/ucvm.conf", "-m", "bbp1d", \
                  "-i", "0.1"], stdout=PIPE, stdin=PIPE, stderr=STDOUT)
    output = proc.communicate(input="-118 34\n-117 35")[0]
    
    os.chdir(curdir)
    
    expected_output =  " -118.0000    34.0000    843.189\n -117.0000    35.0000    843.189\n"
    
    if not output == expected_output:
        print "Error: Vs30 expected output not equal to actual output.\n"
        print "Expected output:\n%s\n\nActual output:\n%s\n" % (expected_output, output)
        return 1
    
    return 0

def test_ssh_generate(dir):
    # Basic small-scale heterogeneities validation.
    proc = Popen([dir + "/bin/ssh_generate", "-u", "0.1", "-d", "20", "-l", "50", \
                  "-s", "5", "-a", "100", "-b", "100", "-c", "100", "-m", "ssh.out"], \
                  stdout=PIPE, stdin=PIPE, stderr=STDOUT)    
    output = proc.communicate()
    
    f = open("./ssh.out", "rb")
    generatedfloats = array.array("f")
    generatedfloats.fromfile(f, 100 * 100 * 100)
    f.close()
    
    os.remove("./ssh.out")
    
    f = open("./validate.ssh", "rb")
    validfloats = array.array("f")
    validfloats.fromfile(f, 100 * 100 * 100)
    f.close()
    
    for i in xrange(0, len(generatedfloats)):
        if not generatedfloats[i] == validfloats[i]:
            return 1
    
    return 0

# Main loop.
if __name__ == "__main__":
    func_list = [x for x in dir(__import__(inspect.getmodulename(__file__))) if "test_" in x]

    passed = 1

    for func in func_list:
        print "Runnning test %s" % func
    
        if eval("%s('%s')" % (func, sys.argv[1])) == 0:
            print "[SUCCESS]"
        else:
            passed = 0
            print "[FAIL]"
    
    if not passed:
        print "Some tests failed. Please try re-compiling UCVM. If that doesn't work"
        print "please contact software@scec.org."