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

/** @file tests/TestGridIngester.h
 *
 * @brief C++ TestGridIngester object
 *
 * C++ unit testing for TestGridIngester.
 */

#if !defined(cencalvm_storage_testgridingester_h)
#define cencalvm_storage_testgridingester_h

#include "cencalvm/storage/etreefwd.h"
#include "cencalvm/storage/Payload.h"

#include <cppunit/extensions/HelperMacros.h>

namespace cencalvm {
  namespace create {
    class TestGridIngester;
  } // create
} // cencalvm

/// C++ unit testing for GridIngester
class cencalvm::create::TestGridIngester : public CppUnit::TestFixture
{ // class TestGridIngester

  // CPPUNIT TEST SUITE /////////////////////////////////////////////////
  CPPUNIT_TEST_SUITE( TestGridIngester );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testFilenameParams );
  CPPUNIT_TEST( testFilenameOut );
  CPPUNIT_TEST( testFilenameTmp );
  CPPUNIT_TEST( testCacheSize );
  CPPUNIT_TEST( testDescription );
  CPPUNIT_TEST( testGeometry );
  CPPUNIT_TEST( testRun );
  CPPUNIT_TEST( testQuiet );
  CPPUNIT_TEST_SUITE_END();

  // PUBLIC METHODS /////////////////////////////////////////////////////
public :

  /// Test constructor
  void testConstructor(void);

  /// Test filenameParams()
  void testFilenameParams(void);

  /// Test filenameOut()
  void testFilenameOut(void);

  /// Test filenameTmp()
  void testFilenameTmp(void);

  /// Test cacheSize()
  void testCacheSize(void);

  /// Test description()
  void testDescription(void);

  /// Test geometry()
  void testGeometry(void);

  /// Test run()
  void testRun(void);

  /// Test quiet()
  void testQuiet(void);

  // PRIVATE METHODS ////////////////////////////////////////////////////
private :

  static const cencalvm::storage::PayloadStruct _PAYLOAD; ///< Payload in db
  static const double _LONLATELEV[]; ///< Lon/Lat/Elev of test location
  static const etree_tick_t _ADDRX; ///< Address x tick of test location
  static const etree_tick_t _ADDRY; ///< Address y tick of test location
  static const etree_tick_t _ADDRZ; ///< Address z tick of test location
  static const etree_tick_t _ADDRLEVEL; ///< Address level of test location

}; // class TestGridIngester

#endif // cencalvm_create_testgridingester

// version
// $Id: TestGridIngester.h 2064 2006-01-02 19:34:49Z brad $

// End of file 
