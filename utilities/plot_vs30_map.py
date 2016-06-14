#!/usr/bin/env python

##
#  @file plot_vs30_slice.py
#  @brief Plots a Vs30 slice using command-line parameters.
#  @author David Gill - SCEC <davidgil@usc.edu>
#  @version 14.7.0
#
#  Plots a Vs30 slice given a set of command-line parameters.

from pycvm import Vs30Slice, UCVM, VERSION, UCVM_CVMS, Point
import getopt, sys

## Prints usage of this utility.
def usage():
    print "Generates a Vs30 map or text file given two bounding latitude and longitude "
    print "co-ordinates, the CVM to plot, and a couple of other settings."
    print "\nValid arguments:"
    print "\t-b, --bottomleft: bottom-left latitude, longitude (e.g. 34,-118)"
    print "\t-u, --upperright: upper-right latitude, longitude (e.g. 35,-117)"
    print "\t-s, --spacing: grid spacing in degrees (typically 0.01)"
    print "\t-c, --cvm: one of the installed community velocity models"
    print "\t-a, --scale: color scale, either 's' for smooth or 'd' for discretized, without quotes"
    print "UCVM %s\n" % VERSION


## Makes sure the response is a number.
def ask_number(question):
    temp_val = None
    
    while temp_val is None:
        temp_val = raw_input(question)
        try:
            float(temp_val)
            return float(temp_val)
        except ValueError:
            print temp_val + " is not a number. Please enter a number."
            temp_val = None
    
## Gets the options and assigns them to the correct variables.
def get_user_opts(options):
    
    short_opt_string = ""
    long_opts = []
    opts_left = []
    ret_val = {}
    
    for key, value in options.iteritems():
        short_opt_string = short_opt_string + key.split(",")[0] + ":"
        long_opts.append(key.split(",")[1])
        opts_left.append(key.split(",")[0])

    try:
        opts, args = getopt.getopt(sys.argv[1:], short_opt_string, long_opts)
    except getopt.GetoptError as err:
        print str(err)   
        exit(1)
    
    for o, a in opts:
        for key, value in options.iteritems():
            if o == "-" + key.split(",")[0] or o == "--" + key.split(",")[1]:
                opts_left.remove(key.split(",")[0])
                if "," in value:
                    ret_val[value.split(",")[0]] = a.split(",")[0]
                    ret_val[value.split(",")[1]] = a.split(",")[1]
                else:
                    ret_val[value] = a
    
    if len(opts_left) == 0 or len(opts_left) == len(options):
        return ret_val
    else:
        return "bad"

# Create a new UCVM object.
u = UCVM()

ret_val = get_user_opts({"b,bottomleft":"lat1,lon1", "u,upperright":"lat2,lon2", \
                         "s,spacing":"spacing", \
                         "c,cvm":"cvm_selected", "a,scale": "color"})

if ret_val == "bad":
    usage()
    exit(1)
elif len(ret_val) > 0:
    print "Using parameters:\n"
    for key, value in ret_val.iteritems():
        print key + " = " + value
        try:
            float(value)
            exec("%s = float(%s)" % (key, value))
        except StandardError, e:
            exec("%s = '%s'" % (key, value))
else:      
    print ""
    print "Vs30  - UCVM %s" % VERSION
    print ""
    print "This utility helps you either plot a Vs30 basin depth map or save the data in a"
    print "text file that you can then later parse."
    print ""
    print "In order to create the plot, you must first specify the region."
    print ""

    lon1 = ask_number("Please enter the bottom-left longitude from which the Vs30 values should come: ")
    lat1 = ask_number("Next, enter the bottom-left latitude from which the Vs30 values should come: ")
    lon2 = ask_number("Enter the top-right longitude where the Vs30 values should end: ")
    lat2 = ask_number("Enter the top-right latitude where the Vs30 values should end: ")

    # Check to see that this is a valid box.
    if lon1 > lon2 or lat1 > lat2:
        print "Error: (%.2f, %.2f) to (%.2f, %.2f) is not a valid box. Please re-run this script" % (lon1, lat1, lon2, lat2)
        print "and specify a valid region. The first point should be the lower-left corner, the"
        print "second point should be the upper-right corner."
        exit(1)
    
    spacing = -1

    while spacing <= 0:
        spacing = ask_number("Which grid spacing (in decimal degree units) would you like (usually, this is 0.01): ")
    
        if spacing <= 0:
            print "Error: grid spacing must be a positive number."
        
    print ""

    # Ask which CVMs to use.
    print "From which CVM would you like this data to come:"

    counter = 1
    corresponding_cvm = []

    for cvm in u.models:
        cvmtoprint = cvm
        if cvm in UCVM_CVMS:
            cvmtoprint = UCVM_CVMS[cvm]
        corresponding_cvm.append(cvm)
        print "\t%d) %s" % (counter, cvmtoprint)
        counter += 1
    
        cvm_selected = -1

    while cvm_selected < 0 or cvm_selected > counter:
        cvm_selected = int(ask_number("\nSelect the CVM: ")) - 1
    
        if cvm_selected < 0 or cvm_selected > counter:
            print "Error: the number you selected must be between 1 and %d" % counter

    cvm_selected = corresponding_cvm[cvm_selected]

# Now that we have all the requisite data, we can actually make the plot now.
print ""
print "Retrieving data. Please wait..."

# Generate the horizontal slice.
v = Vs30Slice(Point(lon1, lat2, 0), Point(lon2, lat1, 0), spacing, cvm_selected)
v.plot(color_scale="s")