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

extern "C" {
#include "cvmerror.h"
}

#include "cencalvm/query/VMQuery.h" // USES VMQuery
#include "cencalvm/storage/ErrorHandler.h" // USES ErrorHandler

#include <stdexcept> // USES std::exception
#include <iostream> // USES std::cerr

// ----------------------------------------------------------------------
static const int VMERR_OK = 0;
static const int VMERR_ERROR = 1;

// ----------------------------------------------------------------------
// Set the filename for logging and enable logging.
int
cencalvm_error_logFilename(void* handle,
			   char* filename)
{ // logFilename
  if (0 == handle) {
    std::cerr << "Null handle for error handler in call to logFilename()."
	      << std::endl;
    return VMERR_ERROR;
  } // if

  cencalvm::storage::ErrorHandler* pHandler = 
    (cencalvm::storage::ErrorHandler*) handle;
  pHandler->logFilename(filename);

  return VMERR_OK;
} // logFilename

// ----------------------------------------------------------------------
// Turn logging on or off.
int
cencalvm_error_loggingOn(void* handle,
			 int turnOn)
{ // loggingOn
  if (0 == handle) {
    std::cerr << "Null handle for error handler in call to loggingOn()."
	      << std::endl;
    return VMERR_ERROR;
  } // if

  cencalvm::storage::ErrorHandler* pHandler = 
    (cencalvm::storage::ErrorHandler*) handle;
  pHandler->loggingOn(turnOn);

  return VMERR_OK;
} // loggingOn

// ----------------------------------------------------------------------
// Reset error status and clear any error message.
int
cencalvm_error_resetStatus(void* handle)
{ // resetStatus
  if (0 == handle) {
    std::cerr << "Null handle for error handler in call to resetStatus()."
	      << std::endl;
    return VMERR_ERROR;
  } // if

  cencalvm::storage::ErrorHandler* pHandler = 
    (cencalvm::storage::ErrorHandler*) handle;
  pHandler->resetStatus();

  return VMERR_OK;
} // resetStatus
  
// ----------------------------------------------------------------------
// Get status of error handler.
int
cencalvm_error_status(void* handle)
{ // status
  if (0 == handle) {
    std::cerr << "Null handle for error handler in call to status()."
	      << std::endl;
    return cencalvm::storage::ErrorHandler::ERROR;
  } // if

  cencalvm::storage::ErrorHandler* pHandler = 
    (cencalvm::storage::ErrorHandler*) handle;
  return pHandler->status();
} // status
  
// ----------------------------------------------------------------------
// Get warning/error message.
char*
cencalvm_error_message(void* handle)
{ // message
  if (0 == handle) {
    std::cerr << "Null handle for error handler in call to message()."
	      << std::endl;
    return 0;
  } // if

  cencalvm::storage::ErrorHandler* pHandler = 
    (cencalvm::storage::ErrorHandler*) handle;
  return (char*) pHandler->message();
} // message

// version
// $Id: cvmerror.cc 3059 2007-02-22 05:33:12Z brad $

// End of file
