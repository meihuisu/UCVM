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

#include "TestGeomCenCA.h" // Implementation of class methods

#include "cencalvm/storage/Geometry.h" // USES GeomCenCA
#include "cencalvm/storage/GeomCenCA.h" // USES GeomCenCA

extern "C" {
#include "etree.h"
}

// ----------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( cencalvm::storage::TestGeomCenCA );

// ----------------------------------------------------------------------
// Test constructor
void
cencalvm::storage::TestGeomCenCA::testConstructor(void)
{ // testConstructor
  GeomCenCA geom;
} // testConstructor

// ----------------------------------------------------------------------
// Test clone()
void
cencalvm::storage::TestGeomCenCA::testClone(void)
{ // testClone
  GeomCenCA geom;
  Geometry* pGeom = geom.clone();
  CPPUNIT_ASSERT(0 != pGeom);
  delete pGeom; pGeom = 0;
} // testClone

// ----------------------------------------------------------------------
// Test lonLatElevToAddr()
void 
cencalvm::storage::TestGeomCenCA::testLonLatElevToAddr(void)
{ // testLonLatElevToAddr
  GeomCenCA geom;

  const double lon = -123.8584929;
  const double lat = 38.424179;
  const double elev = 0.0;
  const double p = GeomCenCA::_BUFFERNW+0.1;
  const double q = GeomCenCA::_BUFFERSW+0.1;
  const double r = GeomCenCA::_ROOTLEN - GeomCenCA::_MAXELEV;

  const int numLevels = 6;
  for (int iLevel=0; iLevel < numLevels; ++iLevel) {
    const double res = GeomCenCA::_ROOTLEN / ((etree_tick_t) 1 << iLevel);
    const etree_tick_t tickLen = 0x80000000 >> iLevel;

    etree_tick_t x = tickLen*int(p / res);
    etree_tick_t y = tickLen*int(q / res);
    etree_tick_t z = tickLen*int(r / res);
    
    etree_addr_t addr;
    addr.level = iLevel;
    geom.lonLatElevToAddr(&addr, lon, lat, elev);

    CPPUNIT_ASSERT_EQUAL(x, addr.x);
    CPPUNIT_ASSERT_EQUAL(y, addr.y);
    CPPUNIT_ASSERT_EQUAL(z, addr.z);
  } // for
} // testLonLatElevToAddr

// ----------------------------------------------------------------------
// Test addrToLonLatElev()
void 
cencalvm::storage::TestGeomCenCA::testAddrToLonLatElev(void)
{ // testAddrToLonLatElev
  GeomCenCA geom;

  const int level = 6;
  const double res = GeomCenCA::_ROOTLEN / ((etree_tick_t) 1 << level);
  const etree_tick_t tickLen = 0x80000000 >> level;

  const double lonE = -123.653262;
  const double latE = 38.400998;
  const double elevE = 0.5*res/GeomCenCA::_VERTEXAG;

  const double p = GeomCenCA::_BUFFERNW + 0.5*res;
  const double q = GeomCenCA::_BUFFERSW + 0.5*res;
  const double r = GeomCenCA::_ROOTLEN +
    (0.5*res / GeomCenCA::_VERTEXAG - GeomCenCA::_MAXELEV) * GeomCenCA::_VERTEXAG;

  etree_addr_t addr;
  addr.level = level;
  addr.x = tickLen*int(p / res);
  addr.y = tickLen*int(q / res);
  addr.z = tickLen*int(r / res);
    
  double lon = 0;
  double lat = 0;
  double elev = 0;
  geom.addrToLonLatElev(&lon, &lat, &elev, &addr);
  
  const double tolerance = 1.0e-6;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, lon/lonE, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, lat/latE, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, elev/elevE, tolerance);

  // double check by going lon/lat/elev back to addr
  etree_addr_t addrCheck;
  addrCheck.level = level;
  geom.lonLatElevToAddr(&addrCheck, lon, lat, elev);
  CPPUNIT_ASSERT_EQUAL(addr.x, addrCheck.x);
  CPPUNIT_ASSERT_EQUAL(addr.y, addrCheck.y);
  CPPUNIT_ASSERT_EQUAL(addr.z, addrCheck.z);
} // testAddrToLonLatElev

// ----------------------------------------------------------------------
// Test edgeLen()
void 
cencalvm::storage::TestGeomCenCA::testEdgeLen(void)
{ // testEdgeLen
  GeomCenCA geom;

  const double rootLen = GeomCenCA::_ROOTLEN;
  const int numLevels = 19;
  double len = rootLen;
  for (int iLevel=0; iLevel < numLevels; ++iLevel, len /= 2.0)
    CPPUNIT_ASSERT_EQUAL(len, geom.edgeLen(iLevel));
} // testEdgeLen

// ----------------------------------------------------------------------
// Test level()
void 
cencalvm::storage::TestGeomCenCA::testLevel(void)
{ // testLevel
  GeomCenCA geom;

  const double rootLen = GeomCenCA::_ROOTLEN;
  const int numLevels = 19;
  double len = rootLen;
  for (int iLevel=0; iLevel < numLevels; ++iLevel, len /= 2.0) {
    CPPUNIT_ASSERT_EQUAL(iLevel, geom.level(len));
  } // for
} // testLevel


// ----------------------------------------------------------------------
// Test metadata()
void
cencalvm::storage::TestGeomCenCA::testMetadata(void)
{ // testMetadata
  GeomCenCA geom;
  const char* metadata = geom.metadata();
  CPPUNIT_ASSERT(0 == strcmp(metadata, GeomCenCA::_METADATA));
} // testMetadata

// ----------------------------------------------------------------------
// Test projector()
void
cencalvm::storage::TestGeomCenCA::testProjector(void)
{ // testProjector
  GeomCenCA geom;
  const Projector* pProj = geom.projector();
  CPPUNIT_ASSERT(0 != pProj);
} // testProjector

// version
// $Id: TestGeomCenCA.cc 2064 2006-01-02 19:34:49Z brad $

// End of file 
