#!/usr/bin/env python

##
#  @file plot_horizontal_slice.py
#  @brief Plots a horizontal slice using command-line parameters.
#  @author David Gill - SCEC <davidgil@usc.edu>
#  @version 14.7.0
#
#  Plots a horizontal slice given a set of command-line parameters.

from pycvm import HorizontalSlice, UCVM, VERSION, UCVM_CVMS, Point
import getopt, sys

## Prints usage of this utility.
def usage():
    print "Plots a horizontal slice given two bounding latitude and longitude co-ordinates,"
    print "the CVM to plot, and a couple of other settings."
    print "\nValid arguments:"
    print "\t-b, --bottomleft: bottom-left latitude, longitude (e.g. 34,-118)"
    print "\t-u, --upperright: upper-right latitude, longitude (e.g. 35,-117)"
    print "\t-s, --spacing: grid spacing in degrees (typically 0.01)"
    print "\t-e, --depth: depth for horizontal slice in meters (e.g. 1000)"
    print "\t-d, --datatype: either 'vs', 'vp', 'rho', or 'poisson', without quotation marks"
    print "\t-c, --cvm: one of the installed velocity models"
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

ret_val = get_user_opts({"b,bottomleft":"lat1,lon1", "u,upperright":"lat2,lon2", \
                         "s,spacing":"spacing", "e,depth":"depth", \
                         "d,datatype":"data_type", "c,cvm":"cvm_selected", "a,scale": "color"})

# Create a new UCVM object.
u = UCVM()

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
    print "Plot Horizontal Slice - UCVM %s" % VERSION
    print ""
    print "This utility helps you plot a horizontal slice across the earth for one of the CVMs"
    print "that you installed with UCVM."
    print ""
    print "In order to create the plot, you must first specify the region."
    print ""

    lon1 = ask_number("Please enter the bottom-left longitude from which the plot should start: ")
    lat1 = ask_number("Next, enter the bottom-left latitude from which the plot should start: ")
    lon2 = ask_number("Enter the top-right longitude where the plot should end: ")
    lat2 = ask_number("Enter the top-right latitude where the plot should end: ")

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

    depth = -1
    print ""

    while depth < 0:
        depth = ask_number("Please enter the depth, in meters, at which you would like this plot: ")
        if depth < 0:
            print "Error: the depth must be a positive number."

    print ""

    data_type = ""

    while data_type != "vs" and data_type != "vp" and data_type != "density" and data_type != "poisson":
        data_type = raw_input("What would you like to plot (either vp, vs, density, or poisson): ")
        data_type = data_type.lower().strip()
    
        if data_type != "vs" and data_type != "vp" and data_type != "density" and data_type != "poisson":
            print "Error: you must select either 'vp', 'vs', 'density', 'poisson' (without quotation marks)."

    # Ask which CVMs to use.
    print "\nFrom which CVM would you like this data to come:"

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

    # We will offer two color options. Discretized or smooth. But, we'll only offer red-blue for now.
    color = ""

    while color != "s" and color != "d" and data_type != "poisson":
        print ""
        color = raw_input("Finally, would you like a descritized or smooth color scale\n(enter 'd' for discrete, 's' for smooth): ")
        color = color.strip()
    
        if color != "s" and color != "d":
            print "Please enter 'd' (without quotation marks) for a discrete color bar and 's' (without quotation"
            print "marks) for a smooth color scale."

# Now that we have all the requisite data, we can actually make the plot now.
print ""
print "Retrieving data. Please wait..."

# Generate the horizontal slice.
h = HorizontalSlice(Point(lon1, lat2, depth), Point(lon2, lat1, depth), spacing, cvm_selected)
h.plot(data_type, color_scale=color)