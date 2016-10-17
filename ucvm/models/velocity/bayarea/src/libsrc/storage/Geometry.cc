// -*- C++ -*-
//
// ======================================================================
//
//                           Brad T. Aagaard
//                        U.S. Geological Survey
//
// {LicenseText}
//
// ======================================================================
//

#include "Geometry.h" // implementation of class methods

extern "C" {
#include "etree.h"
}

#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
const etree_tick_t cencalvm::storage::Geometry::_LEFTMOSTONE =
  ~(~((etree_tick_t)0) >> 1);

// ----------------------------------------------------------------------
cencalvm::storage::Geometry::Geometry(void)
{ // constructor
} // constructor

// ----------------------------------------------------------------------
cencalvm::storage::Geometry::~Geometry(void)
{ // destructor
} // destructor

// ----------------------------------------------------------------------
// Compute address of ancestor at specified level of given octant.
void
cencalvm::storage::Geometry::findAncestor(etree_addr_t* pAncestorAddr,
					  const etree_addr_t& childAddr,
					  const int ancestorLevel)
{ // findAncestor
  assert(0 != pAncestorAddr);
  assert(0 <= ancestorLevel && ancestorLevel < childAddr.level);

  etree_tick_t mask = ((signed) _LEFTMOSTONE) >> ancestorLevel;
  pAncestorAddr->x = childAddr.x & mask;
  pAncestorAddr->y = childAddr.y & mask;
  pAncestorAddr->z = childAddr.z & mask;
  pAncestorAddr->t = 0;
  pAncestorAddr->level = ancestorLevel;
  pAncestorAddr->type  = ETREE_INTERIOR;
} // findParent

// version
// $Id: Geometry.cc 2064 2006-01-02 19:34:49Z brad $

// End of file 
