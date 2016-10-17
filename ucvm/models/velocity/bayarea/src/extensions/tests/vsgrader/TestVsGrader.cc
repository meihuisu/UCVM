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

#include "TestVsGrader.h" // Implementation of class methods

#include "VsGrader.h" // USES VsGrader
#include "cencalvm/storage/Payload.h" // USES PayloadStruct
#include "cencalvm/storage/Geometry.h" // USES Geometry
#include "cencalvm/storage/GeomCenCA.h" // USES GeomCenCA

extern "C" {
#include "etree.h"
}

// ----------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( cencalvm::vsgrader::TestVsGrader );

// ----------------------------------------------------------------------
#include "data/TestVsGrader.dat"

// ----------------------------------------------------------------------
// Setup
void
cencalvm::vsgrader::TestVsGrader::setUp(void)
{ // setUp
  _pGeom = new storage::GeomCenCA;
} // setUp

// ----------------------------------------------------------------------
// Tear down
void
cencalvm::vsgrader::TestVsGrader::tearDown(void)
{ // tearDown
  delete _pGeom; _pGeom = 0;
} // testDown

// ----------------------------------------------------------------------
// Test constructor
void
cencalvm::vsgrader::TestVsGrader::testConstructor(void)
{ // testConstructor
  VsGrader grader;
} // testConstructor

// ----------------------------------------------------------------------
// Test filenameParams()
void
cencalvm::vsgrader::TestVsGrader::testFilenameParams(void)
{ // testFilenameParams
  const char* filename = "abc";

  VsGrader grader;
  grader.filenameParams(filename);
  CPPUNIT_ASSERT_EQUAL(std::string(filename), grader._filenameParams);
} // testFilenameParams

// ----------------------------------------------------------------------
// Test filenameIn()
void
cencalvm::vsgrader::TestVsGrader::testFilenameIn(void)
{ // testFilenameIn
  const char* filename = "xyz";

  VsGrader grader;
  grader.filenameIn(filename);
  CPPUNIT_ASSERT_EQUAL(std::string(filename), grader._filenameIn);
} // testFilenameIn

// ----------------------------------------------------------------------
// Test filenameOut()
void
cencalvm::vsgrader::TestVsGrader::testFilenameOut(void)
{ // testFilenameOut
  const char* filename = "xyz";

  VsGrader grader;
  grader.filenameOut(filename);
  CPPUNIT_ASSERT_EQUAL(std::string(filename), grader._filenameOut);
} // testFilenameOut

// ----------------------------------------------------------------------
// Test filenameTmp()
void
cencalvm::vsgrader::TestVsGrader::testFilenameTmp(void)
{ // testFilenameTmp
  const char* filename = "lmn";

  VsGrader grader;
  grader.filenameTmp(filename);
  CPPUNIT_ASSERT_EQUAL(std::string(filename), grader._filenameTmp);
} // testFilenameTmp

// ----------------------------------------------------------------------
// Test cacheSize()
void
cencalvm::vsgrader::TestVsGrader::testCacheSize(void)
{ // testCacheSize
  const int cacheSize = 34;

  VsGrader grader;
  grader.cacheSize(cacheSize);
  CPPUNIT_ASSERT_EQUAL(cacheSize, grader._cacheSize);
} // testCacheSize

// ----------------------------------------------------------------------
// Test quiet()
void
cencalvm::vsgrader::TestVsGrader::testQuiet(void)
{ // testQuiet
  VsGrader grader;
  CPPUNIT_ASSERT(!grader._quiet); // default is not quiet
  grader.quiet(true);
  CPPUNIT_ASSERT(grader._quiet);
  grader.quiet(false);
  CPPUNIT_ASSERT(!grader._quiet);
} // testQuiet

// ----------------------------------------------------------------------
// Test readParams()
void
cencalvm::vsgrader::TestVsGrader::testReadParams(void)
{ // testReadParams
  VsGrader grader;
  grader.quiet(true);
  grader.filenameParams(_FILENAMEPARAMS);
  grader._readParams();

  const double tolerance = 1.0e-06;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, grader._swcornerLon/_SWCORNERLON, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, grader._swcornerLat/_SWCORNERLAT, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, grader._swcornerElev/_SWCORNERELEV, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, grader._domainLen/_DOMAINLEN, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, grader._domainWidth/_DOMAINWIDTH, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, grader._domainHt/_DOMAINHT, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, grader._resVert/_RESVERT, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, grader._gradientMaxVs/_VSGRADIENTMAX, 
			       tolerance);
} // testReadParams

// ----------------------------------------------------------------------
// Test initialize()
void
cencalvm::vsgrader::TestVsGrader::testInitialize(void)
{ // testInitialize
  VsGrader grader;
  grader.quiet(true);
  grader._swcornerLon = _SWCORNERLON;
  grader._swcornerLat = _SWCORNERLAT;
  grader._initialize();

  const double tolerance = 1.0e-06;
  const double x = grader._swcornerX;
  const double y = grader._swcornerY;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(_SWCORNERX, x, _SWCORNERX*tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(_SWCORNERY, y, _SWCORNERY*tolerance);
} // testInitialize

// ----------------------------------------------------------------------
// Test indexToElev()
void
cencalvm::vsgrader::TestVsGrader::testIndexToElev(void)
{ // testIndexToElev
  VsGrader grader;

  const int index = 5;
  grader._resVert = _RESVERT;
  grader._swcornerElev = _SWCORNERELEV;
  const double elevE = _SWCORNERELEV - _RESVERT * index;
  const double elev = grader._indexToElev(index);
  const double tolerance = 1.0e-6;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(elevE, elev, fabs(elevE)*tolerance);
} // testIndexToElev

// ----------------------------------------------------------------------
// Test indexToLonLat()
void
cencalvm::vsgrader::TestVsGrader::testIndexToLonLat(void)
{ // testIndexToLonLat
  CPPUNIT_ASSERT(0 != _pGeom);

  const int iLen = 23;
  const int iWidth = 75;
  const double resHoriz = _RESVERT * _pGeom->vertExag();
  const double lonE = -122.292303;
  const double latE = 35.526138;

  VsGrader grader;
  grader._swcornerLon = _SWCORNERLON;
  grader._swcornerLat = _SWCORNERLAT;
  grader._swcornerX = _SWCORNERX;
  grader._swcornerY = _SWCORNERY;
  grader._resVert = _RESVERT;

  double lon = 0;
  double lat = 0;
  grader._indexToLonLat(&lon, &lat, iLen, iWidth);

  const double tolerance = 1.0e-06;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(lonE, lon, fabs(lonE)*tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(latE, lat, fabs(latE)*tolerance);
} // testindexToLonLat

// ----------------------------------------------------------------------
// Test pullData()
void
cencalvm::vsgrader::TestVsGrader::testPullData(void)
{ // testPullData
  _createDB();

  etree_t* db = etree_open(_FILENAMEDB, O_RDONLY, 0, 0, 3);
  CPPUNIT_ASSERT(0 != db);
  
  VsGrader grader;
  grader._resVert = _RESVERT;
  cencalvm::storage::PayloadStruct payload;
  grader._pullData(&payload, _SWCORNERLON, _SWCORNERLAT, _SWCORNERELEV, db);

  const double tolerance = 1.0e-06;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(_DBVS, payload.Vs, _DBVS*tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, payload.Vp, 0.1*tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2, payload.Density, 0.2*tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3, payload.Qp, 0.3*tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.4, payload.Qs, 0.4*tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, payload.DepthFreeSurf, 0.5*tolerance);
  CPPUNIT_ASSERT_EQUAL(int16_t(6), payload.FaultBlock);
  CPPUNIT_ASSERT_EQUAL(int16_t(7), payload.Zone);

  int err = etree_close(db);
  CPPUNIT_ASSERT(0 == err);
} // testPullData

// ----------------------------------------------------------------------
// Test pushData()
void
cencalvm::vsgrader::TestVsGrader::testPushPull(void)
{ // testPushPull
  _createDB();

  etree_t* db = etree_open(_FILENAMEDB, O_RDWR, 0, 0, 3);
  CPPUNIT_ASSERT(0 != db);
  
  VsGrader grader;
  cencalvm::storage::PayloadStruct payload;
  payload.Vs = _DBVS+1.0;
  payload.Vp = -0.1;
  payload.Density = -0.2;
  payload.Qp = -0.3;
  payload.Qs = -0.4;
  payload.DepthFreeSurf = -0.5;
  payload.FaultBlock = -6;
  payload.Zone = -7;
  grader._resVert = _RESVERT;
  grader._pushData(db, payload, _SWCORNERLON, _SWCORNERLAT, _SWCORNERELEV);
  
  cencalvm::storage::PayloadStruct payloadN;
  grader._pullData(&payloadN, _SWCORNERLON, _SWCORNERLAT, _SWCORNERELEV, db);

  const double tolerance = 1.0e-06;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payloadN.Vs/payload.Vs, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payloadN.Vp/payload.Vp, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payloadN.Density/payload.Density, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payloadN.Qp/payload.Qp, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payloadN.Qs/payload.Qs, tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payloadN.DepthFreeSurf/payload.DepthFreeSurf, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payloadN.FaultBlock/payload.FaultBlock, 
			       tolerance);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payloadN.Zone/payload.Zone, tolerance);

  int err = etree_close(db);
  CPPUNIT_ASSERT(0 == err);
} // testPushPull

// ----------------------------------------------------------------------
// Test limitDiff()
void
cencalvm::vsgrader::TestVsGrader::testLimitDiff(void)
{ // testLimitDiff

  const int numLocs = 5;
  static const double vsVals[] = { 3.4, 7.6, 5.4, 8.7, 1.4 };
  const double maxdiff = _VSGRADIENTMAX;
  static const double vsValsE[] = { 3.4, 4.5, 3.6, 2.5, 1.4 };
  static const double scale[] = { 1.0, 0.1, 2.3, 5.6, 1.2, 0.8, 0.5 };
  static const bool changed[] = { false, true, true, true, false };

  cencalvm::storage::PayloadStruct* pData = 
    new cencalvm::storage::PayloadStruct[numLocs];
  bool* pIsChanged = new bool[numLocs];
  for (int iLoc=0; iLoc < numLocs; ++iLoc) {
    const double vs = vsVals[iLoc];
    pData[iLoc].Vs = vs;
    pData[iLoc].Vp = vs*scale[0];
    pData[iLoc].Density = vs*scale[1];
    pData[iLoc].Qp = vs*scale[2];
    pData[iLoc].Qs = vs*scale[3];
    pData[iLoc].DepthFreeSurf = vs*scale[4];
    pData[iLoc].FaultBlock = int16_t(vs*scale[5]);
    pData[iLoc].Zone = int16_t(vs*scale[6]);
  } // for

  VsGrader grader;
  grader._limitDiff(&pData, &pIsChanged, numLocs, maxdiff);

  const double tolerance = 1.0e-06;
  for (int iLoc=0; iLoc < numLocs; ++iLoc) {
    const double vs = vsValsE[iLoc];
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pData[iLoc].Vs/vs, tolerance);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pData[iLoc].Vp/(vs*scale[0]), tolerance);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pData[iLoc].Density/(vs*scale[1]), 
				 tolerance);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pData[iLoc].Qp/(vs*scale[2]), tolerance);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pData[iLoc].Qs/(vs*scale[3]), tolerance);

    const double vsOrig = vsVals[iLoc];
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, 
				 pData[iLoc].DepthFreeSurf/(vsOrig*scale[4]),
				 tolerance);
    CPPUNIT_ASSERT_EQUAL(int16_t(vsOrig*scale[5]), pData[iLoc].FaultBlock);
    CPPUNIT_ASSERT_EQUAL(int16_t(vsOrig*scale[6]), pData[iLoc].Zone);
    CPPUNIT_ASSERT_EQUAL(changed[iLoc], pIsChanged[iLoc]);
  } // for
  delete[] pData; pData = 0;
  delete[] pIsChanged; pIsChanged = 0;
} // testLimitDiff

// ----------------------------------------------------------------------
// Create etree to test push/pull.
void
cencalvm::vsgrader::TestVsGrader::_createDB(void) const
{ // _createDB
  CPPUNIT_ASSERT(0 != _pGeom);

  etree_t* db = etree_open(_FILENAMEDB, O_CREAT|O_RDWR|O_TRUNC, 0, 0, 3);
  CPPUNIT_ASSERT(0 != db);

  int err = etree_registerschema(db, cencalvm::storage::Payload::SCHEMA);
  CPPUNIT_ASSERT(0 == err);

  etree_addr_t addr;
  addr.type = ETREE_LEAF;
  const double vertExag = _pGeom->vertExag();
  addr.level = _pGeom->level(_RESVERT*_pGeom->vertExag());
  _pGeom->lonLatElevToAddr(&addr, _SWCORNERLON, _SWCORNERLAT, _SWCORNERELEV);
  
  cencalvm::storage::PayloadStruct payload;
  payload.Vp = 0.1;
  payload.Vs = _DBVS;
  payload.Density = 0.2;
  payload.Qp = 0.3;
  payload.Qs = 0.4;
  payload.DepthFreeSurf = 0.5;
  payload.FaultBlock = 6;
  payload.Zone = 7;

  err = etree_insert(db, addr, &payload);
  CPPUNIT_ASSERT(0 == err);
  
  err = etree_close(db);
  CPPUNIT_ASSERT(0 == err);
} // _createDB

// version
// $Id: TestVsGrader.cc 3064 2007-02-22 17:10:52Z brad $

// End of file 
