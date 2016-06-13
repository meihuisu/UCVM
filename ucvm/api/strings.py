##
#   Defines the list of strings and IDs that are used within UCVM. This prevents
#   problems with something being changed in one place but not the other.

import pkg_resources

STRINGS = {
    1: "Welcome to the UCVM [version] installation. The code will now install.\nWhen that is complete, you " +
       "will be asked which models you would like to\ndownload. Finally, this script can help configure " +
       "paths to make launching\nUCVM significantly easier.",
    2: "Welcome to UCVM [version]!",
    3: "\n----------------------------------------------------------------------\n",
    4: "UCVM supports multiple 3D velocity models. Please type 'y' or 'yes' to\nthe models you would like this " +
       "installation to include. Please note\nthat if you change your mind and would like additional models, you\n" +
       "can always add models later.",
    5: "Thank you for installing UCVM [version]. Please run the tests by executing\n\"ucvm --test\". " +
       "If you want to query a model, execute\n\"ucvm --query --model [model name]\" and provide a latitude, " +
       "longitude,\nand depth.\n\nTo launch the web interface which makes things significantly easier,\n" +
       "execute \"ucvm --server\".\n",
    6: "\nUCVM [version]:\n\nThe following commands are supported by UCVM:\n\n\t-q, --query\n",
    7: "\nInstall CVM-S4? Type 'y' or 'yes' to install: ",
    8: "http://hypocenter.usc.edu/research/ucvm/[version]/models",
    "query_usage": "Querying UCVM requires you to specify a list of points comprised of longitude, latitude, " +
                   "and depth. These points should be provided in WGS84 format. Use Ctrl + D to end the input.",
    "param_model": "Model parameter not provided."
}

def get_string(id, custom_dict = None):
    str = STRINGS[id]
    if custom_dict == None: 
        version = pkg_resources.require("UCVM")[0].version
        str = str.replace("[version]", version)
    else:
        str = str.replace("[version]", custom_dict["version"])
    return str