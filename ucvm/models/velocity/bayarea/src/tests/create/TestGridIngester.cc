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

#include "TestGridIngester.h" // Implementation of class methods

#include "cencalvm/create/GridIngester.h" // USES GridIngester
#include "cencalvm/storage/Geometry.h" // USES Geometry
#include "cencalvm/storage/GeomCenCA.h" // USES GeomCenCA

extern "C" {
#include "etree.h"
}

#include <stdexcept> // USES std::runtime_error
#include <sstream> // USES std::ostringstream

// ----------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( cencalvm::create::TestGridIngester );

// ----------------------------------------------------------------------
#include "data/TestGridIngester.dat"

// ----------------------------------------------------------------------
// Test constructor
void
cencalvm::create::TestGridIngester::testConstructor(void)
{ // testConstructor
  GridIngester ingester;
} // testConstructor

// ----------------------------------------------------------------------
// Test filenameParams()
void
cencalvm::create::TestGridIngester::testFilenameParams(void)
{ // testFilenameParams
  const char* filename = "abc";

  GridIngester ingester;
  ingester.filenameParams(filename);
  CPPUNIT_ASSERT_EQUAL(std::string(filename), ingester._filenameParams);
} // testFilenameParams

// ----------------------------------------------------------------------
// Test filenameOut()
void
cencalvm::create::TestGridIngester::testFilenameOut(void)
{ // testFilenameOut
  const char* filename = "xyz";

  GridIngester ingester;
  ingester.filenameOut(filename);
  CPPUNIT_ASSERT_EQUAL(std::string(filename), ingester._filenameOut);
} // testFilenameOut

// ----------------------------------------------------------------------
// Test filenameTmp()
void
cencalvm::create::TestGridIngester::testFilenameTmp(void)
{ // testFilenameTmp
  const char* filename = "lmn";

  GridIngester ingester;
  ingester.filenameTmp(filename);
  CPPUNIT_ASSERT_EQUAL(std::string(filename), ingester._filenameTmp);
} // testFilenameTmp

// ----------------------------------------------------------------------
// Test cacheSize()
void
cencalvm::create::TestGridIngester::testCacheSize(void)
{ // testCacheSize
  const int cacheSize = 342;

  GridIngester ingester;
  ingester.cacheSize(cacheSize);
  CPPUNIT_ASSERT_EQUAL(cacheSize, ingester._cacheSize);
} // testCacheSize

// ----------------------------------------------------------------------
// Test description()
void
cencalvm::create::TestGridIngester::testDescription(void)
{ // testDescription
  const char* description = "Hello world!";

  GridIngester ingester;
  ingester.description(description);
  CPPUNIT_ASSERT_EQUAL(std::string(description), ingester._description);
} // testDescription

// ----------------------------------------------------------------------
// Test geometry()
void
cencalvm::create::TestGridIngester::testGeometry(void)
{ // testGeometry
  storage::GeomCenCA geom;

  GridIngester ingester;
  ingester.geometry(&geom);
} // testGeometry

// ----------------------------------------------------------------------
// Test run()
void 
cencalvm::create::TestGridIngester::testRun(void)
{ // testRun
  const char* filenameParams = "data/paramfile.txt";
  const char* filenameOut = "data/one.etree";
  const char* filenameTmp = "data/tmp.etree";
  storage::GeomCenCA geometry;

  GridIngester ingester;
  ingester.geometry(&geometry);
  ingester.filenameParams(filenameParams);
  ingester.filenameOut(filenameOut);
  ingester.filenameTmp(filenameTmp);
  ingester.quiet(true);
  ingester.run();

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

  etree_t* db = etree_open(filenameOut, O_RDONLY, 0, 0, 0);
  if (0 == db) {
    std::ostringstream msg;
    msg << "Could not open etree database '" << filenameOut << "'.";
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
} // testRun

// ----------------------------------------------------------------------
// Test quiet()
void
cencalvm::create::TestGridIngester::testQuiet(void)
{ // testQuiet
  GridIngester ingester;
  CPPUNIT_ASSERT(!ingester._quiet); // default is not quiet
  ingester.quiet(true);
  CPPUNIT_ASSERT(ingester._quiet);
  ingester.quiet(false);
  CPPUNIT_ASSERT(!ingester._quiet);
} // testQuiet

// version
// $Id: TestGridIngester.cc 2064 2006-01-02 19:34:49Z brad $

// End of file 
