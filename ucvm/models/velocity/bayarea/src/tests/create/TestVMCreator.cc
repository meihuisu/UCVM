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

#include "TestVMCreator.h" // Implementation of class methods

#include "cencalvm/create/VMCreator.h" // USES VMCreator
#include "cencalvm/storage/Geometry.h" // USES GeomCenCA
#include "cencalvm/storage/GeomCenCA.h" // USES GeomCenCA

extern "C" {
#include "etree.h"
}

#include <stdexcept> // USES std::runtime_error
#include <sstream> // USES std::ostringstream

// ----------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( cencalvm::create::TestVMCreator );

// ----------------------------------------------------------------------
#include "data/TestVMCreator.dat"

// ----------------------------------------------------------------------
// Test constructor
void
cencalvm::create::TestVMCreator::testConstructor(void)
{ // testConstructor
  VMCreator creator;
} // testConstructor

// ----------------------------------------------------------------------
// Test openDB()
void
cencalvm::create::TestVMCreator::testOpenDB(void)
{ // testopenDB
  const char* filename = _FILENAMETMP;
  const char* description = "Hello";
  const int cacheSize = 2;

  VMCreator creator;
  creator.quiet(true);
  creator.openDB(filename, cacheSize, description);
  CPPUNIT_ASSERT_EQUAL(std::string(filename), creator._filename);
  CPPUNIT_ASSERT(0 != creator._pDB);
} // testOpenDB

// ----------------------------------------------------------------------
// Test closeDB()
void
cencalvm::create::TestVMCreator::testCloseDB(void)
{ // testCloseDB
  const char* filename = _FILENAMETMP;
  const char* description = "Hello";
  const int cacheSize = 2;

  VMCreator creator;
  creator.quiet(true);
  creator.openDB(filename, cacheSize, description);
  creator.closeDB();
  CPPUNIT_ASSERT(0 == creator._pDB);
} // testCloseDB

// ----------------------------------------------------------------------
// Test packDB()
void
cencalvm::create::TestVMCreator::testPackDB(void)
{ // testPackDB
  const char* filenamePacked = _FILENAMEIN;
  const char* filenameUnpacked = _FILENAMETMP;
  const int cacheSize = 2;
  const char* description = "Hello";

  VMCreator creator;
  creator.quiet(true);

  creator.openDB(filenameUnpacked, cacheSize, description);

  storage::GeomCenCA geometry;

  const double p = 409612.5;
  const double q = 204812.5;
  const double r = 1587187.5;
  const etree_tick_t level = 3;
  const double res = geometry.edgeLen(level);
  const etree_tick_t tickLen = 0x80000000 >> level;
  etree_addr_t addr;
  addr.x = tickLen*int(p / res);
  addr.y = tickLen*int(q / res);
  addr.z = tickLen*int(r / res);
  addr.level = level;
  double lon = 0;
  double lat = 0;
  double elev = 0;
  geometry.addrToLonLatElev(&lon, &lat, &elev, &addr);
  creator.insert(_PAYLOAD, lon, lat, elev, res, &geometry);
  creator.closeDB();
  creator.packDB(filenamePacked, filenameUnpacked, cacheSize);

  etree_t* db = etree_open(filenamePacked, O_RDONLY, 0, 0, 0);
  if (0 == db) {
    std::ostringstream msg;
    msg << "Could not open etree database '" << filenamePacked << "'.";
    throw std::runtime_error(msg.str());
  } // if

  etree_addr_t resaddr;
  cencalvm::storage::PayloadStruct payload;
  if (0 != etree_search(db, addr, &resaddr, "*", &payload))
    throw std::runtime_error(etree_strerror(etree_errno(db)));
  CPPUNIT_ASSERT_EQUAL(addr.x, resaddr.x);
  CPPUNIT_ASSERT_EQUAL(addr.y, resaddr.y);
  CPPUNIT_ASSERT_EQUAL(addr.z, resaddr.z);
  CPPUNIT_ASSERT_EQUAL(addr.level, resaddr.level);

  const double tolerance = 1.0e-06;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _PAYLOAD.Vp/payload.Vp, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _PAYLOAD.Vs/payload.Vs, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _PAYLOAD.Density/payload.Density, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _PAYLOAD.Qp/payload.Qp, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _PAYLOAD.Qs/payload.Qs, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, 
			       _PAYLOAD.DepthFreeSurf/payload.DepthFreeSurf,
			       tolerance);
  CPPUNIT_ASSERT_EQUAL(_PAYLOAD.FaultBlock, payload.FaultBlock);
  CPPUNIT_ASSERT_EQUAL(_PAYLOAD.Zone, payload.Zone);
} // testPackDB

// ----------------------------------------------------------------------
// Test insert()
void 
cencalvm::create::TestVMCreator::testInsert(void)
{ // testInsert
  const char* filename = _FILENAMETMP;
  const char* description = "Hello";
  const int cacheSize = 2;

  VMCreator creator;
  creator.quiet(true);

  creator.openDB(filename, cacheSize, description);

  storage::GeomCenCA geometry;

  const double p = 409612.5;
  const double q = 204812.5;
  const double r = 1587187.5;
  const etree_tick_t level = 3;
  const double res = geometry.edgeLen(level);
  const etree_tick_t tickLen = 0x80000000 >> level;
  etree_addr_t addr;
  addr.x = tickLen*int(p / res);
  addr.y = tickLen*int(q / res);
  addr.z = tickLen*int(r / res);
  addr.level = level;
  double lon = 0;
  double lat = 0;
  double elev = 0;
  geometry.addrToLonLatElev(&lon, &lat, &elev, &addr);
  creator.insert(_PAYLOAD, lon, lat, elev, res, &geometry);
  creator.closeDB();

  etree_t* db = etree_open(filename, O_RDONLY, 0, 0, 0);
  if (0 == db) {
    std::ostringstream msg;
    msg << "Could not open etree database '" << filename << "'.";
    throw std::runtime_error(msg.str());
  } // if

  etree_addr_t resaddr;
  cencalvm::storage::PayloadStruct payload;
  if (0 != etree_search(db, addr, &resaddr, "*", &payload))
    throw std::runtime_error(etree_strerror(etree_errno(db)));
  CPPUNIT_ASSERT_EQUAL(addr.x, resaddr.x);
  CPPUNIT_ASSERT_EQUAL(addr.y, resaddr.y);
  CPPUNIT_ASSERT_EQUAL(addr.z, resaddr.z);
  CPPUNIT_ASSERT_EQUAL(addr.level, resaddr.level);

  const double tolerance = 1.0e-06;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _PAYLOAD.Vp/payload.Vp, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _PAYLOAD.Vs/payload.Vs, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _PAYLOAD.Density/payload.Density, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _PAYLOAD.Qp/payload.Qp, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _PAYLOAD.Qs/payload.Qs, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, 
			       _PAYLOAD.DepthFreeSurf/payload.DepthFreeSurf,
			       tolerance);
  CPPUNIT_ASSERT_EQUAL(_PAYLOAD.FaultBlock, payload.FaultBlock);
  CPPUNIT_ASSERT_EQUAL(_PAYLOAD.Zone, payload.Zone);
} // testInsert

// ----------------------------------------------------------------------
// Test quiet()
void
cencalvm::create::TestVMCreator::testQuiet(void)
{ // testQuiet
  VMCreator creator;
  CPPUNIT_ASSERT(!creator._quiet); // default is not quiet
  creator.quiet(true);
  CPPUNIT_ASSERT(creator._quiet);
  creator.quiet(false);
  CPPUNIT_ASSERT(!creator._quiet);
} // testQuiet

// version
// $Id: TestVMCreator.cc 2064 2006-01-02 19:34:49Z brad $

// End of file 
