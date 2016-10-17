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

#include "TestGeometry.h" // Implementation of class methods

#include "cencalvm/storage/Geometry.h" // USES Geometry

extern "C" {
#include "etree.h"
}

// ----------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( cencalvm::storage::TestGeometry );

// ----------------------------------------------------------------------
// Test findAncestor()
void 
cencalvm::storage::TestGeometry::testFindAncestor(void)
{ // testFindAncestor

  const int numTests = 4;
  const int pLevelsChild[] = { 2, 3, 4, 4 };
  const int pAddrChild[] = { 0, 0, 0,
			     1, 0, 2,
			     53, 88, 19,
			     53, 88, 19 };
  const int pAddrParent[] = { 0, 0, 0,
			      0, 0, 1,
			      26, 44, 9,
                              13, 22, 4 };
  const int pLevelsParent[] = { 1, 2, 3, 2 };

  for (int iTest=0, i=0; iTest < numTests; ++iTest, i+=3) {
    etree_addr_t child;
    child.level = pLevelsChild[iTest];
    etree_tick_t tickLen = 0x80000000 >> child.level;
    child.x = pAddrChild[i  ]*tickLen;
    child.y = pAddrChild[i+1]*tickLen;
    child.z = pAddrChild[i+2]*tickLen;

    etree_addr_t parent;
    etree_tick_t parentLevel = pLevelsParent[iTest];
    tickLen = 0x80000000 >> parentLevel;
    etree_tick_t x = pAddrParent[i  ]*tickLen;
    etree_tick_t  y = pAddrParent[i+1]*tickLen;
    etree_tick_t  z = pAddrParent[i+2]*tickLen;
    Geometry::findAncestor(&parent, child, parentLevel);
    CPPUNIT_ASSERT_EQUAL(x, parent.x);
    CPPUNIT_ASSERT_EQUAL(y, parent.y);
    CPPUNIT_ASSERT_EQUAL(z, parent.z);
    CPPUNIT_ASSERT_EQUAL((int) parentLevel, parent.level);
  } // for
} // testFindAncestor

// version
// $Id: TestGeometry.cc 2064 2006-01-02 19:34:49Z brad $

// End of file 
