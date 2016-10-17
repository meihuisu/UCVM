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

/** @file tests/TestProjector.h
 *
 * @brief C++ TestProjector object
 *
 * C++ unit testing for TestProjector.
 */

#if !defined(cencalvm_storage_testprojector_h)
#define cencalvm_storage_testprojector_h

#include <cppunit/extensions/HelperMacros.h>

namespace cencalvm {
  namespace storage {
    class TestProjector;
  } // storage
} // cencalvm

/// C++ unit testing for Projector
class cencalvm::storage::TestProjector : public CppUnit::TestFixture
{ // class TestProjector

  // CPPUNIT TEST SUITE /////////////////////////////////////////////////
  CPPUNIT_TEST_SUITE( TestProjector );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testProject );
  CPPUNIT_TEST( testInvProject );
  CPPUNIT_TEST_SUITE_END();

  // PUBLIC METHODS /////////////////////////////////////////////////////
public :

  /// Test constructor
  void testConstructor(void);

  /// Test project()
  void testProject(void);

  /// Test invProject()
  void testInvProject(void);

  // PRIVATE METHODS ////////////////////////////////////////////////////
private :

  static const double _XY[]; ///< Array of projected locations
  static const double _LONLAT[]; ///< Array of geographic locations
  static const int _NUMLOCS; ///< Number of locations

}; // class TestProjector

#endif // cencalvm_storage_testprojector

// version
// $Id: TestProjector.h 2011 2005-10-11 18:28:21Z brad $

// End of file 
