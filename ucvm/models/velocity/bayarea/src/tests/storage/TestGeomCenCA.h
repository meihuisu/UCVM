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

/** @file tests/TestGeomCenCA.h
 *
 * @brief C++ TestGeomCenCA object
 *
 * C++ unit testing for TestGeomCenCA.
 */

#if !defined(cencalvm_storage_testgeomcenca_h)
#define cencalvm_storage_testgeomcenca_h

#include <cppunit/extensions/HelperMacros.h>

namespace cencalvm {
  namespace storage {
    class TestGeomCenCA;
  } // storage
} // cencalvm

/// C++ unit testing for Geometry
class cencalvm::storage::TestGeomCenCA : public CppUnit::TestFixture
{ // class TestGeomCenCA

  // CPPUNIT TEST SUITE /////////////////////////////////////////////////
  CPPUNIT_TEST_SUITE( TestGeomCenCA );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testClone );
  CPPUNIT_TEST( testLonLatElevToAddr );
  CPPUNIT_TEST( testAddrToLonLatElev );
  CPPUNIT_TEST( testEdgeLen );
  CPPUNIT_TEST( testLevel );
  CPPUNIT_TEST( testMetadata );
  CPPUNIT_TEST( testProjector );
  CPPUNIT_TEST_SUITE_END();

  // PUBLIC METHODS /////////////////////////////////////////////////////
public :

  /// Test constructor
  void testConstructor(void);

  /// Test clone()
  void testClone(void);

  /// Test lonLatElevToAddr()
  void testLonLatElevToAddr(void);

  /// Test addrToLonLatElev()
  void testAddrToLonLatElev(void);

  /// Test edgeLen()
  void testEdgeLen(void);

  /// Test level()
  void testLevel(void);

  /// Test metadata()
  void testMetadata(void);

  /// Test projector()
  void testProjector(void);

}; // class TestGeomCenCA

#endif // cencalvm_storage_testgeomcenca

// version
// $Id: TestGeomCenCA.h 2064 2006-01-02 19:34:49Z brad $

// End of file 
