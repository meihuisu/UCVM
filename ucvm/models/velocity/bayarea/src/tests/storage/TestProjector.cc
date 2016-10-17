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

#include "TestProjector.h" // Implementation of class methods

#include "cencalvm/storage/Projector.h" // USES Projector

// ----------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( cencalvm::storage::TestProjector );

// ----------------------------------------------------------------------
#include "data/TestProjector.dat"

// ----------------------------------------------------------------------
// Test constructor
void
cencalvm::storage::TestProjector::testConstructor(void)
{ // testConstructor
  Projector proj;
} // testConstructor

// ----------------------------------------------------------------------
// Test project()
void
cencalvm::storage::TestProjector::testProject(void)
{ // testProject
  Projector proj;

  const int numLocs = _NUMLOCS;
  for (int iLoc=0; iLoc < numLocs; ++iLoc) {
    double x = 0;
    double y = 0;
    const double* pLonLat = &_LONLAT[2*iLoc];
    proj.project(&x, &y, pLonLat[0], pLonLat[1]);
    const double* pXY = &_XY[2*iLoc];
    const double tolerance = 1.0e-6;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, x/pXY[0], tolerance);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, y/pXY[1], tolerance);
  } // for
} // testProject

// ----------------------------------------------------------------------
// Test invProject()
void
cencalvm::storage::TestProjector::testInvProject(void)
{ // testInvProject
  Projector proj;

  const int numLocs = _NUMLOCS;
  for (int iLoc=0; iLoc < numLocs; ++iLoc) {
    double lon = 0;
    double lat = 0;
    const double* pXY = &_XY[2*iLoc];
    proj.invProject(&lon, &lat, pXY[0], pXY[1]);
    const double* pLonLat = &_LONLAT[2*iLoc];
    const double tolerance = 1.0e-6;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, lon/pLonLat[0], tolerance);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, lat/pLonLat[1], tolerance);
  } // for
} // testInvProject

// version
// $Id: TestProjector.cc 2064 2006-01-02 19:34:49Z brad $

// End of file 
