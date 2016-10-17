// -*- C++ -*-
//
// ----------------------------------------------------------------------
//
//                           Brad T. Aagaard
//                        U.S. Geological Survey
//
// {LicenseText}
//
// ----------------------------------------------------------------------
//

#include "TestErrorHandler.h" // Implementation of class methods

#include "cencalvm/storage/ErrorHandler.h" // USES ErrorHandler

#include <fstream> // USES std::ofstream

// ----------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( cencalvm::storage::TestErrorHandler );

// ----------------------------------------------------------------------
const char* cencalvm::storage::TestErrorHandler::_LOGFILENAME = "data/test.log";

// ----------------------------------------------------------------------
// Test constructor
void
cencalvm::storage::TestErrorHandler::testConstructor(void)
{ // testConstructor
  ErrorHandler h;

  CPPUNIT_ASSERT_EQUAL(std::string(""), h._message);
  CPPUNIT_ASSERT_EQUAL(std::string(ErrorHandler::_NULLFILE), h._logFilename);
  CPPUNIT_ASSERT(0 != h._pLogFile);
  CPPUNIT_ASSERT_EQUAL(ErrorHandler::OK, h._status);
} // testConstructor

// ----------------------------------------------------------------------
// Test logFilename()
void 
cencalvm::storage::TestErrorHandler::testLogFilename(void)
{ // testLogFilename
  ErrorHandler handler;

  handler.logFilename(_LOGFILENAME);
  CPPUNIT_ASSERT_EQUAL(handler._logFilename, std::string(_LOGFILENAME));
  CPPUNIT_ASSERT(handler._pLogFile->is_open());
} // testLogFilename

// ----------------------------------------------------------------------
// Test loggingOn()
void 
cencalvm::storage::TestErrorHandler::testLoggingOn(void)
{ // testLoggingOn
  ErrorHandler handler;

  handler.loggingOn(true);
  CPPUNIT_ASSERT(handler._pLogFile->is_open());
  
  handler.loggingOn(false);
  handler.logFilename(_LOGFILENAME);
  handler.loggingOn(true);
  CPPUNIT_ASSERT(handler._pLogFile->is_open());
  handler.loggingOn(false);
} // testLoggingOn

// ----------------------------------------------------------------------
// Test resetStatus()
void 
cencalvm::storage::TestErrorHandler::testResetStatus(void)
{ // testResetStatus
  ErrorHandler handler;

  handler._status = ErrorHandler::ERROR;
  handler._message = "Error";
  handler.resetStatus();
  CPPUNIT_ASSERT_EQUAL(ErrorHandler::OK, handler._status);
  CPPUNIT_ASSERT_EQUAL(std::string(""), handler._message);
} // testResetStatus

// ----------------------------------------------------------------------
// Test status()
void 
cencalvm::storage::TestErrorHandler::testStatus(void)
{ // testStatus
  ErrorHandler handler;

  CPPUNIT_ASSERT_EQUAL(ErrorHandler::OK, handler.status());
  handler._status = ErrorHandler::ERROR;
  CPPUNIT_ASSERT_EQUAL(ErrorHandler::ERROR, handler.status());
} // testStatus

// ----------------------------------------------------------------------
// Test message()
void 
cencalvm::storage::TestErrorHandler::testMessage(void)
{ // testMessage
  ErrorHandler handler;

  const char* message = "Error message.";

  CPPUNIT_ASSERT(0 == strcmp("", handler.message()));
  handler._message = message;
  CPPUNIT_ASSERT(0 == strcmp(message, handler.message()));
} // testMessage

// ----------------------------------------------------------------------
// Test error()
void 
cencalvm::storage::TestErrorHandler::testError(void)
{ // testError
  ErrorHandler handler;

  const char* message = "Error message.";

  handler.error(message);

  CPPUNIT_ASSERT_EQUAL(ErrorHandler::ERROR, handler._status);
  CPPUNIT_ASSERT_EQUAL(std::string(message), handler._message);
} // testError

// ----------------------------------------------------------------------
// Test warning()
void 
cencalvm::storage::TestErrorHandler::testWarning(void)
{ // testWarning
  ErrorHandler handler;

  const char* message = "Warning message.";

  handler.warning(message);

  CPPUNIT_ASSERT_EQUAL(ErrorHandler::WARNING, handler._status);
  CPPUNIT_ASSERT_EQUAL(std::string(message), handler._message);
} // testWarning

// ----------------------------------------------------------------------
// Test log()
void 
cencalvm::storage::TestErrorHandler::testLog(void)
{ // testResetStatus
  ErrorHandler handler;

  const char* noA = "This stuff should go into a black hole.\n";
  const char* noB = "This stuff should also go into a black hole.\n";
  const char* yesA = "This stuff should go into the log file.\n";
  const char* yesB = "This stuff should also go into the log file.\n";

  handler.log(noA);

  handler.logFilename(_LOGFILENAME);
  handler.log(yesA);

  handler.loggingOn(false);
  handler.log(noB);

  handler.loggingOn(true);
  handler.log(yesB);

  handler.loggingOn(false);

  std::ifstream fileIn(_LOGFILENAME);
  std::string line;

  std::getline(fileIn, line, '\n');
  line += "\n";
  CPPUNIT_ASSERT_EQUAL(std::string(yesA), line);

  std::getline(fileIn, line, '\n'); 
  line += "\n";
  CPPUNIT_ASSERT_EQUAL(std::string(yesB), line);

  fileIn.close();
} // testResetStatus

// version
// $Id: TestErrorHandler.cc 2007 2005-10-10 17:25:27Z brad $

// End of file 
