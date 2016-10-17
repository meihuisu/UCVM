// -*- C++ -*-
//
// ----------------------------------------------------------------------
//
// Brad T. Aagaard, U.S. Geological Survey
//
// ----------------------------------------------------------------------
//

// Define the cencalvm SWIG interface module.

// Set module name
%module cencalvm

// Header files for module C++ code.
%{
#include "CenCalVMDB.h"
%}

// Convert standard C++ exceptions to Python exceptions.
%include "exception.i"
%exception {
  try {
    $action
  } catch (const std::exception& err) {
    SWIG_exception(SWIG_RuntimeError, err.what());
  } // try/catch
} // exception

%include "typemaps.i"
%include "include/chararray.i"

// Numpy interface stuff
%{
#define SWIG_FILE_WITH_INIT
%}
%include "include/numpy.i"
%init %{
import_array();
%}


// Interface files.
%include "spatialdb/SpatialDBObj.i"
%include "CenCalVMDB.i"


// End of file
