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

#include "TestAverager.h" // Implementation of class methods

#include "cencalvm/average/Averager.h" // USES Averager

#include "cencalvm/storage/Payload.h" // USES Payload

extern "C" {
#include "etree.h" // USES etree
}

#include <iostream> // USES std::cerr

// ----------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( cencalvm::average::TestAverager );

// ----------------------------------------------------------------------
#include "data/TestAverager.dat"

// ----------------------------------------------------------------------
// Test constructor
void
cencalvm::average::TestAverager::testConstructor(void)
{ // testConstructor
  Averager averager;
} // testConstructor

// ----------------------------------------------------------------------
// Test fillOctants()
void
cencalvm::average::TestAverager::testFillOctants(void)
{ // testFillOctants

  _createDB();

  Averager averager;
  averager.filenameIn(_DBFILENAMEIN);
  averager.filenameOut(_DBFILENAMEOUT);
  averager.quiet(true);
  averager.average();

  etree_t* db = etree_open(_DBFILENAMEOUT, O_RDONLY, 0, 0, 0);
  CPPUNIT_ASSERT(0 != db);

  const int numOctants = _NUMOCTANTSIN;
  for (int iOctant=0; iOctant < numOctants; ++iOctant) {
    
    etree_addr_t addr;
    addr.level = _LEVELS[iOctant];
    addr.type = ETREE_LEAF;

    const etree_tick_t tickLen = 0x80000000 >> addr.level;
    const int numCoords = 3;
    addr.x = tickLen * _COORDS[numCoords*iOctant  ];
    addr.y = tickLen * _COORDS[numCoords*iOctant+1];
    addr.z = tickLen * _COORDS[numCoords*iOctant+2];

    cencalvm::storage::PayloadStruct payload;
    etree_addr_t resAddr;
    int err = etree_search(db, addr, &resAddr, "*", &payload);
    CPPUNIT_ASSERT(0 == err);

    const double tolerance = 1.0e-06;
    const double val = _OCTVALS[iOctant];
    int i=0;
    double valE = _RELPAY[i++]*val;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payload.Vp/valE, tolerance);
    valE = _RELPAY[i++]*val;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payload.Vs/valE, tolerance);
    valE = _RELPAY[i++]*val;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payload.Density/valE, tolerance);
    valE = _RELPAY[i++]*val;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payload.Qp/valE, tolerance);
    valE = _RELPAY[i++]*val;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payload.Qs/valE, tolerance);
    valE = _RELPAY[i++]*val;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payload.DepthFreeSurf/valE, tolerance);
    if (iOctant < _NUMOCTANTSIN) {
      valE = _RELPAY[i++];
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payload.FaultBlock/valE, tolerance);
      valE = _RELPAY[i++];
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payload.Zone/valE, tolerance);
    } else {
      valE = cencalvm::storage::Payload::INTERIORBLOCK;
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payload.FaultBlock/valE, tolerance);
      valE = cencalvm::storage::Payload::INTERIORZONE;
      CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, payload.Zone/valE, tolerance);
    } // if/else
  } // for

  int err = etree_close(db);
  CPPUNIT_ASSERT(0 == err);
} // testFillOctants

// ----------------------------------------------------------------------
// Create etree with desired number of octants.
void
cencalvm::average::TestAverager::_createDB(void) const
{ // _createDB
  etree_t* db = etree_open(_DBFILENAMEIN, O_CREAT|O_RDWR|O_TRUNC, 0, 0, 3);
  CPPUNIT_ASSERT(0 != db);

  int err = etree_registerschema(db, cencalvm::storage::Payload::SCHEMA);
  CPPUNIT_ASSERT(0 == err);

  const int numOctants = _NUMOCTANTSIN;
  for (int iOctant=0; iOctant < numOctants; ++iOctant) {
    
    etree_addr_t addr;
    addr.level = _LEVELS[iOctant];
    addr.type = ETREE_LEAF;

    const etree_tick_t tickLen = 0x80000000 >> addr.level;
    const int numCoords = 3;
    addr.x = tickLen * _COORDS[numCoords*iOctant  ];
    addr.y = tickLen * _COORDS[numCoords*iOctant+1];
    addr.z = tickLen * _COORDS[numCoords*iOctant+2];

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
} // _createDB

// version
// $Id: TestAverager.cc 2064 2006-01-02 19:34:49Z brad $

// End of file 
