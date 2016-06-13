##
#   Parses the template and performs all the necessary replacements
#   based on which version of the UCVM package is installed.

import pkg_resources

def perform_replacements(str):
    version = pkg_resources.require("UCVM")[0].version
    str = str.replace("[version]", version)
    return str