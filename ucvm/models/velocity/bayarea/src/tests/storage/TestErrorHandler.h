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

/** @file tests/TestErrorHandler.h
 *
 * @brief C++ TestErrorHandler object
 *
 * C++ unit testing for TestErrorHandler.
 */

#if !defined(cencalvm_storage_testerrorhandler_h)
#define cencalvm_storage_testerrorhandler_h

#include <cppunit/extensions/HelperMacros.h>

namespace cencalvm {
  namespace storage {
    class TestErrorHandler;
  } // storage
} // cencalvm

/// C++ unit testing for ErrorHandler
class cencalvm::storage::TestErrorHandler : public CppUnit::TestFixture
{ // class TestErrorHandler

  // CPPUNIT TEST SUITE /////////////////////////////////////////////////
  CPPUNIT_TEST_SUITE( TestErrorHandler );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testLogFilename );
  CPPUNIT_TEST( testLoggingOn );
  CPPUNIT_TEST( testResetStatus );
  CPPUNIT_TEST( testStatus );
  CPPUNIT_TEST( testMessage );
  CPPUNIT_TEST( testError );
  CPPUNIT_TEST( testWarning );
  CPPUNIT_TEST( testLog );
  CPPUNIT_TEST_SUITE_END();

  // PUBLIC METHODS /////////////////////////////////////////////////////
public :

  /// Test constructor
  void testConstructor(void);

  /// Test logFilename()
  void testLogFilename(void);

  /// Test loggingOn()
  void testLoggingOn(void);

  /// Test resetStatus()
  void testResetStatus(void);

  /// Test status()
  void testStatus(void);

  /// Test message()
  void testMessage(void);

  /// Test error()
  void testError(void);

  /// Test warning()
  void testWarning(void);

  /// Test log()
  void testLog(void);

  // PRIVATE METHODS ////////////////////////////////////////////////////
private :

  static const char* _LOGFILENAME; ///< Name of test log file

}; // class TestErrorHandler

#endif // cencalvm_storage_testerrorhandler

// version
// $Id: TestErrorHandler.h 2011 2005-10-11 18:28:21Z brad $

// End of file 
