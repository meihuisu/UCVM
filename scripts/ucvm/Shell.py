#!/usr/bin/env python


# Imports
import sys
import subprocess


class Shell:
    cmd = None
    retcode = None
    output = None

    def __init__(self, cmd):
        self.cmd = cmd
        self.retcode = 0
        self.stdout = None
        self.stderr = None

    def __runCommand(self, cmd, sin, sout, serr):
        try:
            self.p = subprocess.Popen(cmd, stdin = sin, \
                                     stdout=sout, \
                                     stderr=serr)
            self.stdout, self.stderr = self.p.communicate()
            self.retcode = self.p.returncode
            #self.output = output.splitlines()
        except:
            print sys.exc_info()
            print "Failed: " + str(cmd)
            self.stdout = None
            self.stderr = None
            self.retcode = None
            return 1

        return 0


    def runCommand(self):
        return self.__runCommand(self.cmd, None, \
                                     subprocess.PIPE, \
                                     subprocess.PIPE)


    #def runCommandWithPipes(self, sin, sout):
    #    return self.__runCommand(self.cmd, sin, sout, \
    #                                 subprocess.STDOUT)


    def getReturnCode(self):
        return self.retcode


    def getOutput(self):
        return (self.stdout + self.stderr)

    def getStdout(self):
        return self.stdout

    def getStderr(self):
        return self.stderr
    



