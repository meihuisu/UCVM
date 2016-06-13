##
#   Handles the download and installation of UCVM models. Each model is downloaded
#   from the web and added to the model directory of the plugin. This creates
#   a standardized model location which can be used for query purposes.

import os
import site
import sys
from subprocess import call

from urllib.request import urlopen
from ucvm.api.strings import get_string

# Download a file with the progress indicator.
def downloadWithProgress(url, folderTo, description):
    u = urlopen(url)
    download_size = int(u.getheader("Content-Length"))

    print("\n" + description)
    sys.stdout.write("[                    ]\r")
    sys.stdout.flush()
    
    call(["mkdir", "-p", folderTo])
    f = open(folderTo + "/" + url.split('/')[-1], 'wb')

    file_size_dl = 0
    block_sz = 8192
    while True:
        buffer = u.read(block_sz)
        if not buffer:
            break
        file_size_dl += len(buffer)
        f.write(buffer)
        
        p = float(file_size_dl) / download_size
        
        numEquals = int(p / 0.05)
        if p / 0.05 >= 19.5:
            numEquals = 20
        
        sys.stdout.write("[")
        for i in range(0, numEquals):
            sys.stdout.write("=")
        for i in range(0, 20 - numEquals):
            sys.stdout.write(" ")
        sys.stdout.write("]  %d%%\r" % (p * 100))
        sys.stdout.flush()
    
    f.close()
    
    sys.stdout.write("\n")
    sys.stdout.flush()

def install_model(model):
    #version = pkg_resources.require("UCVM")[0].version
    model_location = site.USER_BASE + "/lib/python/site-packages/UCVM-16.9.0-py3.5.egg/ucvm/model"
    
    # We need to download the model.
    downloadWithProgress(get_string(8) + "/" + model + ".tar.gz", model_location + "/" + model, "Downloading " + model)
    os.chdir(model_location + "/" + model)
    call(["gunzip", model + ".tar.gz"])
    call(["tar", "xvf", (model + ".tar.gz").replace(".gz", ""), "--strip", "2"])
    call(["rm", (model + ".tar.gz").replace(".gz", "")])
    call(["aclocal"])
    call(["automake", "--add-missing", "--force-missing"])
    call(["autoconf"])
    call(["mkdir", "-p", model_location + "/" + model + "/installed"])
    call(["./configure", "--prefix=" + model_location + "/" + model + "/installed"])
    call(["make", "clean"])
    call(["make"])
    call(["make", "install"])
    return