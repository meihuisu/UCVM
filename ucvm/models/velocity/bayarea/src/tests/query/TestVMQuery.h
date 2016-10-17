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

/** @file tests/TestVMQuery.h
 *
 * @brief C++ TestVMQuery object
 *
 * C++ unit testing for TestVMQuery.
 */

#if !defined(cencalvm_storage_testvmquery_h)
#define cencalvm_storage_testvmquery_h

#include "cencalvm/storage/etreefwd.h"
#include "cencalvm/storage/Payload.h"

#include <cppunit/extensions/HelperMacros.h>

namespace cencalvm {
  namespace query {
    class TestVMQuery;
  } // query
  namespace storage {
    class Geometry; // HOLDSA Geometry
  } // storage
} // cencalvm

/// C++ unit testing for VMQuery
class cencalvm::query::TestVMQuery : public CppUnit::TestFixture
{ // class TestVMQuery

  // CPPUNIT TEST SUITE /////////////////////////////////////////////////
  CPPUNIT_TEST_SUITE( TestVMQuery );

  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testFilename );
  CPPUNIT_TEST( testOpenClose );
  CPPUNIT_TEST( testQueryType );
  CPPUNIT_TEST( testQueryVals );
  CPPUNIT_TEST( testCacheSize );
  CPPUNIT_TEST( testSquash );
  CPPUNIT_TEST( testQueryMax );
  CPPUNIT_TEST( testQueryFixed );
  CPPUNIT_TEST( testQueryWave );
  CPPUNIT_TEST( testQuerySquash );
  CPPUNIT_TEST( testErrorHandler );
  CPPUNIT_TEST( testCacheSizeExt );
  CPPUNIT_TEST( testFilenameExt );
  CPPUNIT_TEST( testQueryMaxExt );

  CPPUNIT_TEST_SUITE_END();

  // PUBLIC METHODS /////////////////////////////////////////////////////
public :

  /// Setup TestVMQuery
  void setUp(void);

  /// Tear down TestVMQuery
  void tearDown(void);

  /// Test constructor
  void testConstructor(void);

  /// Test filename()
  void testFilename(void);

  /// Test open(), close()
  void testOpenClose(void);

  /// Test queryType()
  void testQueryType(void);

  /// Test queryVals()
  void testQueryVals(void);

  /// Test cacheSize()
  void testCacheSize(void);

  /// Test squash()
  void testSquash(void);

  /// Test query() with max query
  void testQueryMax(void);

  /// Test query() with fixed query
  void testQueryFixed(void);

  /// Test query() for wave query
  void testQueryWave(void);

  /// Test query() with squashing.
  void testQuerySquash(void);

  /// Test errorHandler()
  void testErrorHandler(void);

  /// Test filenameExt()
  void testFilenameExt(void);

  /// Test cacheSizeExt()
  void testCacheSizeExt(void);

  /// Test query() with max query and extended model
  void testQueryMaxExt(void);

  // PRIVATE METHODS ////////////////////////////////////////////////////
private :

  /// Create etree database.
  void _createDB(void) const;

  /// Create etree database for extended region.
  void _createDBExt(void) const;

  /** Get lon/lat/elev of octants in database.
   *
   * @param ppCoords Pointer to array of coordinates
   */
  void _dbLonLatElev(double** ppCoords) const;

  /** Get lon/lat/elev of octants in extended database.
   *
   * @param ppCoords Pointer to array of coordinates
   */
  void _dbLonLatElevExt(double** ppCoords) const;

  // PRIVATE MEMBERS ////////////////////////////////////////////////////
private :

  storage::Geometry* _pGeom; ///< Pointer to velocity model geometry

  static const double _OCTVALS[]; ///< Octant values in database
  static const double _RELPAY[]; ///< Relative values of payload
  static const int _COORDS[]; ///< Coordinates of octants in database
  static const char* _DBFILENAME; ///< Filename of output etree database
  static const int _NUMOCTANTS; ///< Number of octants
  static const int _NUMOCTANTSLEAF; ///< Number of octants for input

  static const double _OCTVALSEXT[]; ///< Octant values for extended database
  static const double _RELPAYEXT[]; ///< Relative values of extended payload
  static const int _COORDSEXT[]; ///< Coordinates of octants in database
  static const char* _DBFILENAMEEXT; ///< Filename of output etree database
  static const int _NUMOCTANTSEXT; ///< Number of octants
  static const int _NUMOCTANTSLEAFEXT; ///< Number of octants for input

}; // class TestVMQuery

#endif // cencalvm_query_testvmquery


// End of file 
