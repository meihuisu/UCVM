// -*- C++ -*-
//
// ======================================================================
//
//                           Brad T. Aagaard
//                        U.S. Geological Survey
//
// {LicenseText}
//
// ======================================================================
//

#include <portinfo>

#include <sys/types.h> // USES size_t

#include "f77vmerror.h"

extern "C" {
#include "cvmerror.h"
}

#include <sstream> // USES std::istringstream
#include <string.h> // USE strncpy
#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
// Set the filename for logging and enable logging.
void
cencalvm_error_logfilename_f(size_t* handleAddr,
			     const char* filename,
			     int* err,
			     const int len)
{ // logFilename
  assert(0 != err);

  std::istringstream sin(filename);
  std::string cfilename;
  sin >> cfilename;
  *err = cencalvm_error_logFilename((void*) *handleAddr, 
				    (char*) cfilename.c_str());
} // logFilename

// ----------------------------------------------------------------------
// Turn logging on or off.
void
cencalvm_error_loggingon_f(size_t* handleAddr,
			   int* turnOn,
			   int* err)
{ // loggingOn
  assert(0 != err);

  *err = cencalvm_error_loggingOn((void*) *handleAddr, *turnOn);
} // loggingOn
  
// ----------------------------------------------------------------------
// Reset error status and clear any error message.
void
cencalvm_error_resetstatus_f(size_t* handleAddr,
			     int* err)
{ // resetStatus
  assert(0 != err);

  *err = cencalvm_error_resetStatus((void*) *handleAddr);
} // resetStatus

// ----------------------------------------------------------------------
// Get status of error handler.
void
cencalvm_error_status_f(size_t* handleAddr,
			int* err)
{ // status
  assert(0 != err);

  *err = cencalvm_error_status((void*) *handleAddr);
} // status

#include <iostream>
// ----------------------------------------------------------------------
// Get warning/error message.
void
cencalvm_error_message_f(size_t* handleAddr,
			 char* message,
			 int* err,
			 const int len)
{ // message
  assert(0 != err);
  assert(0 != message);
  assert(len > 0);

  *err = 0;

  char* cmessage = cencalvm_error_message((void*) *handleAddr);
  if (0 == cmessage)
    *err = 1;

  strncpy(message, cmessage, len);
} // message

// version
// $Id: f77vmerror.cc 3059 2007-02-22 05:33:12Z brad $

// End of file
