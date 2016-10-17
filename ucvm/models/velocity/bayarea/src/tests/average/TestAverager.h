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

/** @file tests/TestAverager.h
 *
 * @brief C++ TestAverager object
 *
 * C++ unit testing for TestAverager.
 */

#if !defined(cencalvm_average_testaverager_h)
#define cencalvm_average_testaverage_h

#include <cppunit/extensions/HelperMacros.h>

namespace cencalvm {
  namespace average {
    class TestAverager;
  } // query
} // cencalvm

/// C++ unit testing for Averager
class cencalvm::average::TestAverager : public CppUnit::TestFixture
{ // class TestAverager

  // CPPUNIT TEST SUITE /////////////////////////////////////////////////
  CPPUNIT_TEST_SUITE( TestAverager );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testFillOctants );
  CPPUNIT_TEST_SUITE_END();

  // PUBLIC METHODS /////////////////////////////////////////////////////
public :

  /// Test constructor
  void testConstructor(void);

  /// Test fillOctants()
  void testFillOctants(void);

  // PRIVATE MEMBERS ////////////////////////////////////////////////////
private :

  /// Create etree database.
  void _createDB(void) const;

  // PRIVATE METHODS ////////////////////////////////////////////////////
private :

  static const double _OCTVALS[]; ///< Octant values in database
  static const double _RELPAY[]; ///< Relateive values of payload
  static const int _LEVELS[]; ///< Levels of octants in database
  static const int _COORDS[]; ///< Coordinates of octants in database
  static const char* _DBFILENAMEIN; ///< Filename of input etree database
  static const char* _DBFILENAMEOUT; ///< Filename of output etree database
  static const int _NUMOCTANTS; ///< Number of octants
  static const int _NUMOCTANTSIN; ///< Number of octants for input

}; // class TestAverager

#endif // cencalvm_average_testaverager

// version
// $Id: TestAverager.h 2010 2005-10-11 18:27:42Z brad $

// End of file 
