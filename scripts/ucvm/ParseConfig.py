#!/usr/bin/env python

# Basic modules
import os
import sys


# Parse configuration
class ParseConfig:
    def __init__(self, meta):
        self.valid = False
        self.meta = meta
        self.dict = {}

        for line in self.meta:
            line = line.lstrip()
            if ((len(line) > 0) and (line[0] != '#')):
                tokens = line.split("=", 1)
                if (len(tokens) == 2):
                    key = tokens[0].strip()
                    value = tokens[1].strip()
                    self.dict[key] = value

        self.valid = True

    def isValid(self):
        return self.valid

    def cleanup(self):
        return

    def showDict(self):
        print "Parsed from config:"
        for key in self.dict.keys():
            print "\t%s = %s" % (key, self.dict[key])
        return(0)


    def getDict(self):
        return(self.dict)

