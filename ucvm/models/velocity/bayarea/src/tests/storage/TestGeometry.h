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

/** @file tests/TestGeometry.h
 *
 * @brief C++ TestGeometry object
 *
 * C++ unit testing for TestGeometry.
 */

#if !defined(cencalvm_storage_testgeometry_h)
#define cencalvm_storage_testgeometry_h

#include <cppunit/extensions/HelperMacros.h>

namespace cencalvm {
  namespace storage {
    class TestGeometry;
  } // storage
} // cencalvm

/// C++ unit testing for Geometry
class cencalvm::storage::TestGeometry : public CppUnit::TestFixture
{ // class TestGeometry

  // CPPUNIT TEST SUITE /////////////////////////////////////////////////
  CPPUNIT_TEST_SUITE( TestGeometry );
  CPPUNIT_TEST( testFindAncestor );
  CPPUNIT_TEST_SUITE_END();

  // PUBLIC METHODS /////////////////////////////////////////////////////
public :

  /// Test findAncestor()
  void testFindAncestor(void);
  
}; // class TestGeometry

#endif // cencalvm_storage_testgeometry

// version
// $Id: TestGeometry.h 2064 2006-01-02 19:34:49Z brad $

// End of file 
