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

/** @file tests/TestVMCreator.h
 *
 * @brief C++ TestVMCreator object
 *
 * C++ unit testing for TestVMCreator.
 */

#if !defined(cencalvm_storage_testvmcreator_h)
#define cencalvm_storage_testvmcreator_h

#include "cencalvm/storage/etreefwd.h"
#include "cencalvm/storage/Payload.h"

#include <cppunit/extensions/HelperMacros.h>

namespace cencalvm {
  namespace create {
    class TestVMCreator;
  } // create
} // cencalvm

/// C++ unit testing for VMCreator
class cencalvm::create::TestVMCreator : public CppUnit::TestFixture
{ // class TestVMCreator

  // CPPUNIT TEST SUITE /////////////////////////////////////////////////
  CPPUNIT_TEST_SUITE( TestVMCreator );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testOpenDB );
  CPPUNIT_TEST( testCloseDB );
  CPPUNIT_TEST( testPackDB );
  CPPUNIT_TEST( testInsert );
  CPPUNIT_TEST( testQuiet );
  CPPUNIT_TEST_SUITE_END();

  // PUBLIC METHODS /////////////////////////////////////////////////////
public :

  /// Test constructor
  void testConstructor(void);

  /// Test openDB()
  void testOpenDB(void);

  /// Test closeDB()
  void testCloseDB(void);

  /// Test packDB()
  void testPackDB(void);

  /// Test insert()
  void testInsert(void);

  /// Test quiet()
  void testQuiet(void);

  // PRIVATE METHODS ////////////////////////////////////////////////////
private :

  static const char* _FILENAMETMP; ///< Name of temporary db file
  static const char* _FILENAMEIN; ///< Name of existing db test file

  static const cencalvm::storage::PayloadStruct _PAYLOAD; ///< Payload in db
  static const double _LONLATELEV[]; ///< Lon/Lat/Elev of test location
  static const etree_tick_t _ADDRX; ///< Address x tick of test location
  static const etree_tick_t _ADDRY; ///< Address y tick of test location
  static const etree_tick_t _ADDRZ; ///< Address z tick of test location
  static const etree_tick_t _ADDRLEVEL; ///< Address level of test location

}; // class TestVMCreator

#endif // cencalvm_create_testvmcreator

// version
// $Id: TestVMCreator.h 2064 2006-01-02 19:34:49Z brad $

// End of file 
