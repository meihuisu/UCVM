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

#include "ErrorHandler.h" // implementation of class methods

#include <fstream> // USES std::ofstream

#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
const char* cencalvm::storage::ErrorHandler::_NULLFILE = "/dev/null";

// ----------------------------------------------------------------------
cencalvm::storage::ErrorHandler::ErrorHandler(void) :
  _message(""),
  _logFilename(_NULLFILE),
  _pLogFile(new std::ofstream(_NULLFILE)),
  _status(OK)
{ // constructor
} // constructor

// ----------------------------------------------------------------------
cencalvm::storage::ErrorHandler::~ErrorHandler(void)
{ // destructor
  delete _pLogFile; _pLogFile = 0;
} // destructor

// ----------------------------------------------------------------------
// Set filename for logging.
void
cencalvm::storage::ErrorHandler::logFilename(const char* filename)
{ // logFilename
  assert(0 != _pLogFile);

  _logFilename = filename;

  if (_pLogFile->is_open())
    _pLogFile->close();
  _pLogFile->clear();
  _pLogFile->open(_logFilename.c_str(), std::ios::out|std::ios::trunc);
} // logFilename

// ----------------------------------------------------------------------
// Turn logging on/off.
void
cencalvm::storage::ErrorHandler::loggingOn(const bool turnOn)
{ // loggingOn
  assert(0 != _pLogFile);

  _pLogFile->close();
  _pLogFile->clear();
  if (turnOn && _logFilename.length() > 0)
    _pLogFile->open(_logFilename.c_str(), std::ios::out|std::ios::app);
  else
    _pLogFile->open(_NULLFILE, std::ios::out);
} // loggingOn

// ----------------------------------------------------------------------
// Write message to log file.
void
cencalvm::storage::ErrorHandler::log(const char* msg)
{ // log
  assert(0 != _pLogFile);
  (*_pLogFile) << msg;
} // log


// End of file 
