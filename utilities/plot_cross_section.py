#!/usr/bin/env python

##
#  @file plot_cross_section.py
#  @brief Plots a cross section using command-line parameters.
#  @author David Gill - SCEC <davidgil@usc.edu>
#  @version 14.7.0
#
#  Plots a cross section given a set of command-line parameters.

from pycvm import CrossSection, UCVM, VERSION, UCVM_CVMS, Point
import getopt, sys

## Prints usage statement.
def usage():
    print "Plots a cross-section given two latitude, longitude points, a depth,"
    print "the CVM to plot, and a couple of other settings."
    print "\nValid arguments:"
    print "\t-s, --starting: starting depth for cross-section (meters)"
    print "\t-e, --ending: ending depth for cross-section (meters)"
    print "\t-h, --horizontal: horizontal spacing for cross-section (meters)"
    print "\t-v, --vertical: vertical spacing for cross-section (meters)"
    print "\t-d, --datatype: either 'vs', 'vp', 'rho', or 'poisson', without quotation marks"
    print "\t-c, --cvm: one of the installed CVMs"
    print "\t-a, --scale: color scale, either 's' for smooth or 'd' for discretized, without quotes"
    print "\t-o, --origin: origin latitude, longitude from which to start plot (e.g. 34,-118)"
    print "\t-f, --final: destination latitude, longitude to end plot (e.g. 35,-117)"
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

ret_val = get_user_opts({"s,starting":"starting_depth", "e,ending":"ending_depth", "b,bottomleft":"lat1,lon1", "u,upperright":"lat2,lon2", \
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
    print "Plot Cross-Section - UCVM %s" % VERSION
    print ""
    print "This utility helps you plot a cross-section across the earth for one of the CVMs"
    print "that you installed with UCVM."
    print ""
    print "In order to create the plot, you must first specify the starting point."
    print ""

    lon1 = ask_number("Please enter the origin longitude from which the plot should start: ")
    lat1 = ask_number("Next, enter the origin latitude from which the plot should start: ")
    lon2 = ask_number("Enter the destination longitude where the plot should end: ")
    lat2 = ask_number("Enter the destination latitude where the plot should end: ")

    starting_depth = -1
    print ""

    while starting_depth < 0:
        starting_depth = ask_number("Please enter the depth, in meters, at which you would like \n" + \
                                    "this cross-section to start: ")
        if starting_depth < 0:
            print "Error: the depth must be a positive number."

    ending_depth = -1
    while ending_depth < 0:
        ending_depth = ask_number("Please enter the depth, in meters, at which you would like \n" + \
                                  "this cross-section to end: ")
        if ending_depth < 0:
            print "Error: the depth must be a positive number."

    if ending_depth <= starting_depth:
        print "Error: the bottom, ending depth must be greater than the starting depth."
 
    horizontal_spacing = -1
    print ""

    while horizontal_spacing < 0:
        horizontal_spacing = ask_number("Please enter the horizontal spacing, in meters, for the plot: ")
        if horizontal_spacing < 0:
            print "Error: the spacing must be a positive number."

    vertical_spacing = -1
    while vertical_spacing < 0:
        vertical_spacing = ask_number("Please enter the vertical spacing, in meters, for the plot: ")
        if vertical_spacing < 0:
            print "Error: the spacing must be a positive number."   

    print ""

    data_type = ""

    while data_type != "vs" and data_type != "vp" and data_type != "rho" and data_type != "poisson":
        data_type = raw_input("What would you like to plot (either vp, vs, rho, or poisson): ")
        data_type = data_type.lower().strip()
    
        if data_type != "vs" and data_type != "vp" and data_type != "rho" and data_type != "poisson":
            print "Error: you must select either 'vp', 'vs', 'rho', 'poisson' (without quotation marks)."

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

# Now we have all the information so we can actually plot the data.
print ""
print "Retrieving data. Please wait..."

# Generate the horizontal slice.
d = CrossSection(Point(lon1, lat1, starting_depth), Point(lon2, lat2, starting_depth), \
                 ending_depth, horizontal_spacing, vertical_spacing, cvm_selected)
d.plot(data_type)