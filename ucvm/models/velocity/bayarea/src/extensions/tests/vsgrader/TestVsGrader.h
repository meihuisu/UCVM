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

/** @file tests/TestVsGrader.h
 *
 * @brief C++ TestVsGrader object
 *
 * C++ unit testing for TestVsGrader.
 */

#if !defined(cencalvm_storage_testvsgrader_h)
#define cencalvm_storage_testvsgrader_h

#include <cppunit/extensions/HelperMacros.h>

namespace cencalvm {
  namespace vsgrader {
    class TestVsGrader;
  } // vsgrader
  namespace storage {
    class Geometry; // HOLDSA Geometry
  } // storage
} // cencalvm

/// C++ unit testing for VsGrader
class cencalvm::vsgrader::TestVsGrader : public CppUnit::TestFixture
{ // class TestVsGrader

  // CPPUNIT TEST SUITE /////////////////////////////////////////////////
  CPPUNIT_TEST_SUITE( TestVsGrader );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testFilenameParams );
  CPPUNIT_TEST( testFilenameIn );
  CPPUNIT_TEST( testFilenameOut );
  CPPUNIT_TEST( testFilenameTmp );
  CPPUNIT_TEST( testCacheSize );
  CPPUNIT_TEST( testQuiet );
  CPPUNIT_TEST( testReadParams );
  CPPUNIT_TEST( testInitialize );
  CPPUNIT_TEST( testIndexToElev );
  CPPUNIT_TEST( testIndexToLonLat );
  CPPUNIT_TEST( testPullData );
  CPPUNIT_TEST( testPushPull );
  CPPUNIT_TEST( testLimitDiff );
  CPPUNIT_TEST_SUITE_END();

  // PUBLIC METHODS /////////////////////////////////////////////////////
public :

  /// Setup
  void setUp(void);

  /// Tear down
  void tearDown(void);

  /// Test constructor
  void testConstructor(void);

  /// Test filenameParams()
  void testFilenameParams(void);

  /// Test filenameIn()
  void testFilenameIn(void);

  /// Test filenameOut()
  void testFilenameOut(void);

  /// Test filenameTmp()
  void testFilenameTmp(void);

  /// Test cacheSize()
  void testCacheSize(void);

  /// Test quiet()
  void testQuiet(void);

  /// Test readParams()
  void testReadParams(void);

  /// Test initialize()
  void testInitialize(void);

  /// Test indexToElev()
  void testIndexToElev(void);

  /// Test indexToLonLat()
  void testIndexToLonLat(void);

  /// Test pullData()
  void testPullData(void);

  /// Test push() and pull()
  void testPushPull(void);

  /// Test limitDiff()
  void testLimitDiff(void);

  // PRIVATE METHODS ////////////////////////////////////////////////////
private :

  /// Create test database
  void _createDB(void) const;

  // PRIVATE MEMBERS ////////////////////////////////////////////////////
private :

  storage::Geometry* _pGeom; ///< Pointer to velocity model geometry

  static const double _SWCORNERLON; ///< Longitude for top SW corner of domain
  static const double _SWCORNERLAT; ///< Latitude for top SW corner of domain
  static const double _SWCORNERELEV; ///< Elevation of top SW corner of domain
  static const double _AZIMUTHLEN; ///< Azimuth for length direction
  static const double _DOMAINLEN; ///< Length of domain
  static const double _DOMAINWIDTH; ///< Width of domain
  static const double _DOMAINHT; ///< Height of domain
  static const double _RESVERT; ///< Vertical resolution
  static const double _VSGRADIENTMAX; ///< Maximum allowable gradient in Vs

  static const double _SWCORNERX; ///< Projected x coordinate of SW top corner
  static const double _SWCORNERY; ///< Projected y coordinate of SW top corner

  static const double _DBVS; ///< Vs value in test database

  static const char* _FILENAMEPARAMS; ///< Name of sample parameter file
  static const char* _FILENAMEDB; ///< Name of test database

}; // class TestVsGrader

#endif // cencalvm_vsgrader_testvsgrader

// version
// $Id: TestVsGrader.h 2067 2006-01-02 20:38:17Z brad $

// End of file 
