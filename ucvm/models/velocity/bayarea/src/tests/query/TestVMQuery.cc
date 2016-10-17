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

#include "TestVMQuery.h" // Implementation of class methods

#include "cencalvm/query/VMQuery.h" // USES VMQuery
#include "cencalvm/average/Averager.h" // USES Averager
#include "cencalvm/storage/Geometry.h" // USES Geometry
#include "cencalvm/storage/ErrorHandler.h" // USES ErrorHandler
#include "cencalvm/storage/GeomCenCA.h" // USES GeomCenCA

extern "C" {
#include "etree.h"
}

#include <iostream> // USES std::cerr
#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( cencalvm::query::TestVMQuery );

// ----------------------------------------------------------------------
#include "data/TestVMQuery.dat"

// ----------------------------------------------------------------------
// Setup TestVMQuery
void
cencalvm::query::TestVMQuery::setUp(void)
{ // setUp
  _pGeom = new storage::GeomCenCA;
} // setUp

// ----------------------------------------------------------------------
// Tear down TestVMQuery
void
cencalvm::query::TestVMQuery::tearDown(void)
{ // tearDown
  delete _pGeom; _pGeom = 0;
} // tearDown

// ----------------------------------------------------------------------
// Test constructor
void
cencalvm::query::TestVMQuery::testConstructor(void)
{ // testConstructor
  VMQuery query;
} // testConstructor

// ----------------------------------------------------------------------
// Test filename()
void
cencalvm::query::TestVMQuery::testFilename(void)
{ // testFilename
  VMQuery query;
  query.filename(_DBFILENAME);
  CPPUNIT_ASSERT(0 == strcmp(_DBFILENAME, query._filename.c_str()));
} // testFilename

// ----------------------------------------------------------------------
// Test open(), close()
void
cencalvm::query::TestVMQuery::testOpenClose(void)
{ // testOpenClose
  _createDB();  

  VMQuery query;
  query.filename(_DBFILENAME);
  query.open();
  CPPUNIT_ASSERT(0 != query._db);
  query.close();
  CPPUNIT_ASSERT(0 == query._db);
} // testOpenClose

// ----------------------------------------------------------------------
// Test queryType()
void
cencalvm::query::TestVMQuery::testQueryType(void)
{ // testQueryType
  VMQuery query;

  // default should be max
  CPPUNIT_ASSERT(&cencalvm::query::VMQuery::_queryMax == query._queryFn);

  cencalvm::query::VMQuery::QueryEnum queryType = 
    cencalvm::query::VMQuery::FIXEDRES;
  query.queryType(queryType);
  CPPUNIT_ASSERT(&cencalvm::query::VMQuery::_queryFixed == query._queryFn);

  queryType = cencalvm::query::VMQuery::WAVERES;
  query.queryType(queryType);
  CPPUNIT_ASSERT(&cencalvm::query::VMQuery::_queryWave == query._queryFn);

  queryType = cencalvm::query::VMQuery::MAXRES;
  query.queryType(queryType);
  CPPUNIT_ASSERT(&cencalvm::query::VMQuery::_queryMax == query._queryFn);
} // testQueryType

// ----------------------------------------------------------------------
// Test queryVals()
void
cencalvm::query::TestVMQuery::testQueryVals(void)
{ // testQueryVals
  VMQuery query;

  // default should be all values
  const int defaultNumVals = 9;
  CPPUNIT_ASSERT(0 != query._pQueryVals);
  for (int i=0; i < defaultNumVals; ++i)
    CPPUNIT_ASSERT_EQUAL(i, query._pQueryVals[i]);


  { // Set A
    const int numVals = 3;
    const char* pNames[] = { "Vp", "Zone", "Qs" };
    const int pVals[] = { 0, 7, 4 };
    query.queryVals(pNames, numVals);
    CPPUNIT_ASSERT_EQUAL(numVals, query._querySize);
    for (int i=0; i < numVals; ++i)
      CPPUNIT_ASSERT_EQUAL(pVals[i], query._pQueryVals[i]);
  } // Set A

  { // Set B
    const int numVals = 3;
    const char* pNames[] = { "Vp", "Qs", "Elevation" };
    const int pVals[] = { 0, 4, 8 };
    query.queryVals(pNames, numVals);
    CPPUNIT_ASSERT_EQUAL(numVals, query._querySize);
    for (int i=0; i < numVals; ++i)
      CPPUNIT_ASSERT_EQUAL(pVals[i], query._pQueryVals[i]);
  } // Set B
} // testQueryVals

// ----------------------------------------------------------------------
// Test cacheSize()
void
cencalvm::query::TestVMQuery::testCacheSize(void)
{ // testCacheSize
  VMQuery query;

  // default should be 128
  const int defaultSize = 128;
  CPPUNIT_ASSERT_EQUAL(defaultSize, query._cacheSize);

  const int cacheSize = 523;
  query.cacheSize(cacheSize);
  CPPUNIT_ASSERT_EQUAL(cacheSize, query._cacheSize);
} // testCacheSize

// ----------------------------------------------------------------------
// Test squash()
void
cencalvm::query::TestVMQuery::testSquash(void)
{ // testSquash
  const double tolerance = 0.01;

  VMQuery query;

  // default should be false
  CPPUNIT_ASSERT(false == query._squashTopo);

  // default should be -2000.0
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-2000.0, query._squashLimit, tolerance);

  bool value = true;
  query.squash(value);
  CPPUNIT_ASSERT_EQUAL(value, query._squashTopo);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-2000.0, query._squashLimit, tolerance);

  value = false;
  query.squash(value);
  CPPUNIT_ASSERT_EQUAL(value, query._squashTopo);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-2000.0, query._squashLimit, tolerance);

  value = true;
  query.squash(value, -3000.0);
  CPPUNIT_ASSERT_EQUAL(value, query._squashTopo);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-3000.0, query._squashLimit, tolerance);
} // testSquash

// ----------------------------------------------------------------------
// Test query() with max query
void 
cencalvm::query::TestVMQuery::testQueryMax(void)
{ // testQuerymax
  _createDB();

  VMQuery query;
  query.filename(_DBFILENAME);
  query.queryType(cencalvm::query::VMQuery::MAXRES);
  query.open();

  const cencalvm::storage::ErrorHandler* pHandler = query.errorHandler();

  const int numVals = 9;
  double* pVals = (numVals > 0) ? new double[numVals] : 0;

  double* pLonLatElev = 0;
  _dbLonLatElev(&pLonLatElev);
  const int numLocs = _NUMOCTANTSLEAF;
  for (int iLoc=0, i=0; iLoc < numLocs; ++iLoc, i+=3) {
    query.query(&pVals, numVals, 
		pLonLatElev[i  ], pLonLatElev[i+1], pLonLatElev[i+2]);
    
    const double tolerance = 1.0e-06;
    const double val = _OCTVALS[iLoc];
    for (int iVal=0; iVal < 6; ++iVal) {
      const double  valE = _RELPAY[iVal]*val;
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);
    } // for

    int iVal = 6; // Block
    double valE = (iLoc < _NUMOCTANTSLEAF) ? 
      _RELPAY[iVal] :
      cencalvm::storage::Payload::INTERIORBLOCK;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);

    iVal = 7; // Zone
    valE = (iLoc < _NUMOCTANTSLEAF) ? 
      _RELPAY[iVal] :
      cencalvm::storage::Payload::INTERIORZONE;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);

    iVal = 8; // Elevation
    valE = pLonLatElev[i+2] + pVals[5];
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);
  } // for

  query.close();

  CPPUNIT_ASSERT(cencalvm::storage::ErrorHandler::OK == pHandler->status());

  delete[] pLonLatElev; pLonLatElev = 0;
} // testQueryMax

// ----------------------------------------------------------------------
// Test query() with fixed query
void 
cencalvm::query::TestVMQuery::testQueryFixed(void)
{ // testQueryFixed
  assert(0 != _pGeom);

  _createDB();

  VMQuery query;
  query.filename(_DBFILENAME);
  query.queryType(cencalvm::query::VMQuery::FIXEDRES);
  query.open();

  cencalvm::storage::ErrorHandler* pHandler = query.errorHandler();
//   pHandler->logFilename("fixed.log");

  const int numVals = 9;
  double* pVals = (numVals > 0) ? new double[numVals] : 0;

  double* pLonLatElev = 0;
  _dbLonLatElev(&pLonLatElev);

  const int pOctIndices[] = { 3, 13, 17, 10, 12, 14, 15, 18 };
  const int numLocs = 8;
  for (int iLoc=0; iLoc < numLocs; ++iLoc) {
    const int iOctant = pOctIndices[iLoc];

    const int numOctCoords = 4;
    const int level = _COORDS[numOctCoords*iOctant+3];
    const double res = _pGeom->edgeLen(level) / _pGeom->vertExag();
    
    query.queryRes(res);
    query.query(&pVals, numVals, 
		pLonLatElev[3*iOctant  ], 
		pLonLatElev[3*iOctant+1],
		pLonLatElev[3*iOctant+2]);
    if (cencalvm::storage::ErrorHandler::OK != pHandler->status())
      std::cerr << pHandler->message() << std::endl;
    
    const double tolerance = 1.0e-06;
    const double val = _OCTVALS[iOctant];
    for (int iVal=0; iVal < 6; ++iVal) {
      const double  valE = _RELPAY[iVal]*val;
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);
    } // for

    int iVal = 6; // Block
    double valE = (iOctant < _NUMOCTANTSLEAF) ? 
      _RELPAY[iVal] :
      cencalvm::storage::Payload::INTERIORBLOCK;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);

    iVal = 7; // Zone
    valE = (iOctant < _NUMOCTANTSLEAF) ? 
      _RELPAY[iVal] :
      cencalvm::storage::Payload::INTERIORZONE;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);

    iVal = 8; // Elevation (same result as maxres query)
    etree_addr_t addr;
    valE = query._queryElev(&addr,
			    pLonLatElev[3*iOctant  ], 
			    pLonLatElev[3*iOctant+1],
			    pLonLatElev[3*iOctant+2]);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);
  } // for

  query.close();

  CPPUNIT_ASSERT(cencalvm::storage::ErrorHandler::OK == pHandler->status());

  delete[] pLonLatElev; pLonLatElev = 0;
} // testQueryFixed

// ----------------------------------------------------------------------
// Test query() with wave query
void 
cencalvm::query::TestVMQuery::testQueryWave(void)
{ // testQueryWave
  _createDB();

  VMQuery query;
  query.filename(_DBFILENAME);
  query.queryType(cencalvm::query::VMQuery::WAVERES);
  query.open();
  cencalvm::storage::ErrorHandler* pHandler = query.errorHandler();
  //  pHandler->logFilename("wave.log");
  
  const int numVals = 9;
  double* pVals = (numVals > 0) ? new double[numVals] : 0;

  double* pLonLatElev = 0;
  _dbLonLatElev(&pLonLatElev);

  const int numLocs = 3;
  const int pOctIndices[] = { 1, 7, 11 };
  const double periodMin[] = { 800.0, 1.0, 4000.0 };
  const double octValsWave[] = { 3.575, 1.1, 3.8875 };
  const bool isResLeaf[] = { false, true, false };


  for (int iLoc=0; iLoc < numLocs; ++iLoc) {
    const int iOctant = pOctIndices[iLoc];
    const double res = periodMin[iLoc];
    
    query.queryRes(res);
    query.query(&pVals, numVals, 
		pLonLatElev[3*iOctant  ], 
		pLonLatElev[3*iOctant+1],
		pLonLatElev[3*iOctant+2]);
    if (cencalvm::storage::ErrorHandler::OK != pHandler->status())
      std::cerr << pHandler->message() << std::endl;
    
    const double tolerance = 1.0e-06;
    const double val = octValsWave[iLoc];
    for (int iVal=0; iVal < 6; ++iVal) {
      const double  valE = _RELPAY[iVal]*val;
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);
    } // for

    int iVal = 6; // Block
    double valE = isResLeaf[iLoc] ? 
      _RELPAY[iVal] : cencalvm::storage::Payload::INTERIORBLOCK;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);

    iVal = 7; // Zone
    valE = isResLeaf[iLoc] ?
      _RELPAY[iVal] : cencalvm::storage::Payload::INTERIORZONE;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);

    iVal = 8; // Elevation (same result as maxres query)
    etree_addr_t addr;
    valE = query._queryElev(&addr,
			    pLonLatElev[3*iOctant  ], 
			    pLonLatElev[3*iOctant+1],
			    pLonLatElev[3*iOctant+2]);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);
  } // for

  query.close();

  CPPUNIT_ASSERT(cencalvm::storage::ErrorHandler::OK == pHandler->status());

  delete[] pLonLatElev; pLonLatElev = 0;
} // testQueryWave

// ----------------------------------------------------------------------
// Test query() with max query and squashing.
void 
cencalvm::query::TestVMQuery::testQuerySquash(void)
{ // testQueryMaxSquash
  _createDB();

  VMQuery query;
  query.filename(_DBFILENAME);
  query.queryType(cencalvm::query::VMQuery::MAXRES);
  query.squash(true);
  query.open();

  CPPUNIT_ASSERT_EQUAL(true, query._squashTopo);

  const cencalvm::storage::ErrorHandler* pHandler = query.errorHandler();

  const int numVals = 1;
  const char* queryVals[] = { "Elevation" };
  double* pVals = (numVals > 0) ? new double[numVals] : 0;

  query.queryVals(queryVals, numVals);

  double* pLonLatElev = 0;
  _dbLonLatElev(&pLonLatElev);
  const int numLocs = _NUMOCTANTSLEAF;
  for (int iLoc=0, i=0; iLoc < numLocs; ++iLoc, i+=3) {
    const double val = _OCTVALS[iLoc];
    const double depth = _RELPAY[5]*val;
    const double elevQ = (pLonLatElev[i+2] < -2000.0) ? 
      pLonLatElev[i+2] :
      -depth;
    query.query(&pVals, numVals, 
		pLonLatElev[i  ], pLonLatElev[i+1], elevQ);

    const double tolerance = 1.0e-06;
    const double elevE = pLonLatElev[i+2] + depth;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[0]/elevE, tolerance);
  } // for

  query.close();

  CPPUNIT_ASSERT(cencalvm::storage::ErrorHandler::OK == pHandler->status());

  delete[] pLonLatElev; pLonLatElev = 0;
} // testQueryMax

// ----------------------------------------------------------------------
// Test errorHandler()
void
cencalvm::query::TestVMQuery::testErrorHandler(void)
{ // testErrorHandler
  VMQuery query;
  const cencalvm::storage::ErrorHandler* pHandler = query.errorHandler();
  CPPUNIT_ASSERT(0 != pHandler);
  CPPUNIT_ASSERT_EQUAL(cencalvm::storage::ErrorHandler::OK,
		       pHandler->status());
} // testErrorHandler

// ----------------------------------------------------------------------
// Test filenameExt()
void
cencalvm::query::TestVMQuery::testFilenameExt(void)
{ // testFilenameExt
  VMQuery query;
  query.filenameExt(_DBFILENAME);
  CPPUNIT_ASSERT(0 == strcmp(_DBFILENAME, query._filenameExt.c_str()));
} // testFilenameExt

// ----------------------------------------------------------------------
// Test cacheSize()
void
cencalvm::query::TestVMQuery::testCacheSizeExt(void)
{ // testCacheSizeExt
  VMQuery query;

  // default should be 128
  const int defaultSize = 128;
  CPPUNIT_ASSERT_EQUAL(defaultSize, query._cacheSizeExt);

  const int cacheSize = 523;
  query.cacheSizeExt(cacheSize);
  CPPUNIT_ASSERT_EQUAL(cacheSize, query._cacheSizeExt);
} // testCacheSizeExt

// ----------------------------------------------------------------------
// Test query() with max query for extended model
void 
cencalvm::query::TestVMQuery::testQueryMaxExt(void)
{ // testQueryMaxExt
  _createDB();
  _createDBExt();

  VMQuery query;
  query.filename(_DBFILENAME);
  query.filenameExt(_DBFILENAMEEXT);
  query.queryType(cencalvm::query::VMQuery::MAXRES);
  query.open();

  const cencalvm::storage::ErrorHandler* pHandler = query.errorHandler();

  const int numVals = 9;
  double* pVals = (numVals > 0) ? new double[numVals] : 0;

  double* pLonLatElev = 0;
  _dbLonLatElevExt(&pLonLatElev);

  const int pOctIndices[] = { 2, 0 };
  const int numLocs = 2;
  for (int iLoc=0; iLoc < numLocs; ++iLoc) {
    const int iOctant = pOctIndices[iLoc];

    query.query(&pVals, numVals, 
		pLonLatElev[3*iOctant  ], 
		pLonLatElev[3*iOctant+1],
		pLonLatElev[3*iOctant+2]);
    if (cencalvm::storage::ErrorHandler::OK != pHandler->status())
      std::cerr << pHandler->message() << std::endl;
    
    const double tolerance = 1.0e-06;
    const double val = _OCTVALSEXT[iOctant];
    for (int iVal=0; iVal < 6; ++iVal) {
      const double  valE = _RELPAYEXT[iVal]*val;
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);
    } // for
    int iVal = 6; // Block
    double valE = (iOctant < _NUMOCTANTSLEAFEXT) ? 
      _RELPAY[iVal] :
      cencalvm::storage::Payload::INTERIORBLOCK;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);
    iVal = 7; // Zone
    valE = (iOctant < _NUMOCTANTSLEAFEXT) ? 
      _RELPAY[iVal] :
      cencalvm::storage::Payload::INTERIORZONE;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pVals[iVal]/valE, tolerance);
  } // for

  query.close();

  CPPUNIT_ASSERT(cencalvm::storage::ErrorHandler::OK == pHandler->status());

  delete[] pLonLatElev; pLonLatElev = 0;
} // testQueryMaxExt

// ----------------------------------------------------------------------
// Create etree with desired number of octants.
void
cencalvm::query::TestVMQuery::_createDB(void) const
{ // _createDB
  const char* filenameTmp = "data/leaf.etree";

  etree_t* db = etree_open(filenameTmp, O_CREAT|O_RDWR|O_TRUNC, 0, 0, 3);
  CPPUNIT_ASSERT(0 != db);

  int err = etree_registerschema(db, cencalvm::storage::Payload::SCHEMA);
  CPPUNIT_ASSERT(0 == err);

  const int numOctants = _NUMOCTANTSLEAF;
  const int numCoords = 4;
  for (int iOctant=0; iOctant < numOctants; ++iOctant) {
    
    etree_addr_t addr;
    addr.level = _COORDS[numCoords*iOctant+3];
    addr.type = ETREE_LEAF;

    const etree_tick_t tickLen = 0x80000000 >> addr.level;
    addr.x = tickLen * _COORDS[numCoords*iOctant  ];
    addr.y = tickLen * _COORDS[numCoords*iOctant+1];
    addr.z = tickLen * _COORDS[numCoords*iOctant+2];
    addr.t = 0;

    const double val = _OCTVALS[iOctant];
    cencalvm::storage::PayloadStruct payload;
    int i=0;
    payload.Vp = _RELPAY[i++]*val;
    payload.Vs = _RELPAY[i++]*val;
    payload.Density = _RELPAY[i++]*val;
    payload.Qp = _RELPAY[i++]*val;
    payload.Qs = _RELPAY[i++]*val;
    payload.DepthFreeSurf = _RELPAY[i++]*val;
    payload.FaultBlock = int(_RELPAY[i++]);
    payload.Zone = int(_RELPAY[i++]);

    err = etree_insert(db, addr, &payload);
    CPPUNIT_ASSERT(0 == err);
  } // for

  err = etree_close(db);
  CPPUNIT_ASSERT(0 == err);

  cencalvm::average::Averager averager;
  averager.filenameIn(filenameTmp);
  averager.filenameOut(_DBFILENAME);
  averager.quiet(true);
  averager.average();  
} // _createDB

// ----------------------------------------------------------------------
// Create etree with desired number of octants.
void
cencalvm::query::TestVMQuery::_createDBExt(void) const
{ // _createDBExt
  const char* filenameTmp = "data/leafext.etree";

  etree_t* db = etree_open(filenameTmp, O_CREAT|O_RDWR|O_TRUNC, 0, 0, 3);
  CPPUNIT_ASSERT(0 != db);

  int err = etree_registerschema(db, cencalvm::storage::Payload::SCHEMA);
  CPPUNIT_ASSERT(0 == err);

  const int numOctants = _NUMOCTANTSLEAFEXT;
  const int numCoords = 4;
  for (int iOctant=0; iOctant < numOctants; ++iOctant) {
    
    etree_addr_t addr;
    addr.level = _COORDSEXT[numCoords*iOctant+3];
    addr.type = ETREE_LEAF;

    const etree_tick_t tickLen = 0x80000000 >> addr.level;
    addr.x = tickLen * _COORDSEXT[numCoords*iOctant  ];
    addr.y = tickLen * _COORDSEXT[numCoords*iOctant+1];
    addr.z = tickLen * _COORDSEXT[numCoords*iOctant+2];
    addr.t = 0;

    const double val = _OCTVALSEXT[iOctant];
    cencalvm::storage::PayloadStruct payload;
    int i=0;
    payload.Vp = _RELPAYEXT[i++]*val;
    payload.Vs = _RELPAYEXT[i++]*val;
    payload.Density = _RELPAYEXT[i++]*val;
    payload.Qp = _RELPAYEXT[i++]*val;
    payload.Qs = _RELPAYEXT[i++]*val;
    payload.DepthFreeSurf = _RELPAYEXT[i++]*val;
    payload.FaultBlock = int(_RELPAYEXT[i++]);
    payload.Zone = int(_RELPAYEXT[i++]);

    err = etree_insert(db, addr, &payload);
    CPPUNIT_ASSERT(0 == err);
  } // for

  err = etree_close(db);
  CPPUNIT_ASSERT(0 == err);

  cencalvm::average::Averager averager;
  averager.filenameIn(filenameTmp);
  averager.filenameOut(_DBFILENAMEEXT);
  averager.quiet(true);
  averager.average();  
} // _createDBExt

// ----------------------------------------------------------------------
// Get lon/lat/elev of octants in database.
void
cencalvm::query::TestVMQuery::_dbLonLatElev(double** ppCoords) const
{ // _dbLonLatElev
  assert(0 != ppCoords);
  assert(0 != _pGeom);

  const int numCoords = 3;
  const int numOctants = _NUMOCTANTS;
  delete[] *ppCoords; *ppCoords = new double[numCoords*numOctants];

  const int numOctCoords = 4;
  for (int iOctant=0, i=0; iOctant < numOctants; ++iOctant) {
    etree_addr_t addr;
    addr.level = _COORDS[numOctCoords*iOctant+3];
    addr.type = ETREE_LEAF;

    const etree_tick_t tickLen = 0x80000000 >> addr.level;
    addr.x = tickLen * _COORDS[numOctCoords*iOctant  ];
    addr.y = tickLen * _COORDS[numOctCoords*iOctant+1];
    addr.z = tickLen * _COORDS[numOctCoords*iOctant+2];

    double lon = 0;
    double lat = 0;
    double elev = 0;
    _pGeom->addrToLonLatElev(&lon, &lat, &elev, &addr);

    (*ppCoords)[i++] = lon;
    (*ppCoords)[i++] = lat;
    (*ppCoords)[i++] = elev;    
  } // for
} // _dbLonLatElev

// ----------------------------------------------------------------------
// Get lon/lat/elev of octants in database.
void
cencalvm::query::TestVMQuery::_dbLonLatElevExt(double** ppCoords) const
{ // _dbLonLatElevExt
  assert(0 != ppCoords);
  assert(0 != _pGeom);

  const int numCoords = 3;
  const int numOctants = _NUMOCTANTSEXT;
  delete[] *ppCoords; *ppCoords = new double[numCoords*numOctants];

  const int numOctCoords = 4;
  for (int iOctant=0, i=0; iOctant < numOctants; ++iOctant) {
    etree_addr_t addr;
    addr.level = _COORDSEXT[numOctCoords*iOctant+3];
    addr.type = ETREE_LEAF;

    const etree_tick_t tickLen = 0x80000000 >> addr.level;
    addr.x = tickLen * _COORDSEXT[numOctCoords*iOctant  ];
    addr.y = tickLen * _COORDSEXT[numOctCoords*iOctant+1];
    addr.z = tickLen * _COORDSEXT[numOctCoords*iOctant+2];

    double lon = 0;
    double lat = 0;
    double elev = 0;
    _pGeom->addrToLonLatElev(&lon, &lat, &elev, &addr);

    (*ppCoords)[i++] = lon;
    (*ppCoords)[i++] = lat;
    (*ppCoords)[i++] = elev;    
  } // for
} // _dbLonLatElevExt


// End of file 
