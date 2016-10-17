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

#include "AvgEngine.h" // implementation of class methods

#include "cencalvm/storage/Payload.h" // USES PayloadStruct
#include "cencalvm/storage/Geometry.h" // USES Geometry

extern "C" {
#include "etree.h"
}

#include <stdexcept> // USES std::runtime_error
#include <sstream> // USES std::ostringstream
#include <assert.h> // USES assert()
#include <iostream> // USES std::cout

// ----------------------------------------------------------------------
const etree_tick_t cencalvm::average::AvgEngine::_LEFTMOSTONE =
  ~(~((etree_tick_t)0) >> 1);

// ----------------------------------------------------------------------
// Default constructor
cencalvm::average::AvgEngine::AvgEngine(etree_t* dbOut,
					etree_t* dbIn) :
  _dbAvg(dbOut),
  _dbIn(dbIn),
  _pPendingOctants(0),
  _pendingSize(0),
  _pendingCursor(-1)
{ // constructor
  const int pendingSize = ETREE_MAXLEVEL + 1;
  _pPendingOctants = new OctantPendingStruct[pendingSize];
  for (int i=0; i < pendingSize; ++i) {
    _pPendingOctants[i].data.pSum = new storage::PayloadStruct;
    _pPendingOctants[i].data.numChildren = 0;
    _pPendingOctants[i].pAddr = new etree_addr_t;
    _pPendingOctants[i].pAddr->x = 0;
    _pPendingOctants[i].pAddr->y = 0;
    _pPendingOctants[i].pAddr->z = 0;
    _pPendingOctants[i].pAddr->t = 0;
    _pPendingOctants[i].pAddr->level = 0;
    _pPendingOctants[i].processedChildren = 0x00;
    _pPendingOctants[i].isValid = false;
  } // if
  _pendingSize = pendingSize;
  
  _octantCounter.input = 0;
  _octantCounter.output = 0;
  _octantCounter.interior = 0;
  _octantCounter.inc_x = 0;
  _octantCounter.inc_y = 0;
  _octantCounter.inc_z = 0;
  _octantCounter.inc_xy = 0;
  _octantCounter.inc_yz = 0;
  _octantCounter.inc_xz = 0;
  _octantCounter.inc_xyz = 0;
  _octantCounter.inc_invalid = 0;  
} // constructor
  
// ----------------------------------------------------------------------
// Default destructor.
cencalvm::average::AvgEngine::~AvgEngine(void)
{ // destructor
  if (0 != _pPendingOctants)
    for (int i=0; i < _pendingSize; ++i) {
      delete _pPendingOctants[i].data.pSum; _pPendingOctants[i].data.pSum = 0;
      delete _pPendingOctants[i].pAddr; _pPendingOctants[i].pAddr = 0;
    } // for
  delete[] _pPendingOctants; _pPendingOctants = 0;
  _pendingSize = 0;
} // destructor

// ----------------------------------------------------------------------
// Fill in octants with averages of their children
void
cencalvm::average::AvgEngine::fillOctants(void)
{ // fillOctants
  assert(0 != _dbIn);
  assert(0 != _dbAvg);

  etree_addr_t cursor;
  cursor.x = 0;
  cursor.y = 0;
  cursor.z = 0;
  cursor.t = 0;
  cursor.level = ETREE_MAXLEVEL;

  int eof = etree_initcursor(_dbIn, cursor);
  if (eof)
    throw std::runtime_error("Error occurred while initializing etree cursor.");

  int err = etree_beginappend(_dbAvg, 1.0);
  if (0 != err)
    throw std::runtime_error("Error occurred while trying to initiate appending "
			     "of etree.");

  storage::PayloadStruct payload;
  while (!eof) {
    eof = etree_getcursor(_dbIn, &cursor, 0, &payload);
    if (eof)
      throw std::runtime_error("Error occurred while trying to get payload at "
			       "current cursor position.");
    _averageOctant(&cursor, payload);
    eof = etree_advcursor(_dbIn);
  } // while

  _finishProcessing();

  err = etree_endappend(_dbAvg);
  if (0 != err)
    throw std::runtime_error("Error occurred while trying to terminate appending "
			     "of etree.");
} // fillOctants

// ----------------------------------------------------------------------
// Print octant counting information to stream.
void
cencalvm::average::AvgEngine::printOctantInfo(void) const
{ // printOctantInfo
  std::cout
    << "Summary of etree averaging\n"
    << "Number of octants\n"
    << "  IN: " << _octantCounter.input << "\n"
    << "  OUT total: " << _octantCounter.output
    << ", interior: " << _octantCounter.interior << "\n"
    << "Incomplete octants\n"
    << "  x: " << _octantCounter.inc_x << "\n"
    << "  y: " << _octantCounter.inc_y << "\n"
    << "  z: " << _octantCounter.inc_z << "\n"
    << "  xy: " << _octantCounter.inc_xy << "\n"
    << "  yz: " << _octantCounter.inc_yz << "\n"
    << "  xz: " << _octantCounter.inc_xz << "\n"
    << "  xyz: " << _octantCounter.inc_xyz << "\n"
    << "  invalid: " << _octantCounter.inc_invalid << "\n"
    << std::endl;
} // printOctantInfo

// ----------------------------------------------------------------------
// Do average processing on octant.
void
cencalvm::average::AvgEngine::_averageOctant(etree_addr_t* pAddr,
					     const storage::PayloadStruct& payload)
{ // _averageOctant
  assert(0 != pAddr);
  assert(0 != _dbAvg);
  assert(0 != _pPendingOctants);

  // if at root, we are done
  if (pAddr->level <= 0)
    return;

  int pendingLevel = _findParent(pAddr);
  assert(pendingLevel == pAddr->level-1);
  assert(pendingLevel >= 0 && pendingLevel == pAddr->level-1);
  assert(pendingLevel <=_pendingCursor);

  int err = etree_append(_dbAvg, *pAddr, &payload);
  if (0 != err)
    throw std::runtime_error("Error occurred while trying to append octant "
			     "to etree.");
  ++_octantCounter.input;
  ++_octantCounter.output;

  _addToParent(&_pPendingOctants[pendingLevel], pAddr, payload);
  
  // if completed processing of octant, update higher levels
  const unsigned char INC_FULL = 0xFF;
  while(pendingLevel >= 0 &&
	INC_FULL == _pPendingOctants[pendingLevel].processedChildren) {
    assert(_pPendingOctants[pendingLevel].isValid);
    assert(pendingLevel == _pendingCursor);
    _processOctant(pendingLevel);
    --pendingLevel;
  } // while

  assert(pendingLevel <=_pendingCursor);  
} // _averageOctant

// ----------------------------------------------------------------------
// Add contribution of octant to parent.
void
cencalvm::average::AvgEngine::_addToParent(OctantPendingStruct* pPendingParent,
					   etree_addr_t* pAddrChild,
			const storage::PayloadStruct& childPayload)
{ // _addToParent
  assert(0 != pPendingParent);
  assert(0 != pAddrChild);
  assert(pAddrChild->level-1 == pPendingParent->pAddr->level);

  unsigned char childBit = _childOctantBit(pAddrChild);
  if (pPendingParent->processedChildren & childBit) {
    char bufA[ETREE_MAXBUF];
    char bufB[ETREE_MAXBUF];
    std::ostringstream msg;
    msg
      << "Consistency check for parent/child failed while trying to add\n"
      << "child " << etree_straddr(_dbAvg, bufA, *pAddrChild)
      << " to\n"
      << "parent " << etree_straddr(_dbAvg, bufB, *pPendingParent->pAddr)
      << "\n"
      << "pendingCursor: " << _pendingCursor
      << "\n";
    throw std::runtime_error(msg.str());
  } // if

  pPendingParent->processedChildren |= childBit;

  // We allow averaging of both water and solid material properties,
  // but we don't permit mixing the two in the averaging. During the
  // averaging of the octants, a child with solid properties takes
  // precendence over children with water properties, so a parent
  // octant will have solid properties if any of its children do.
  if (childPayload.Vs > 0.0) { // if not water
    // check to see if another child added water material properties
    if (pPendingParent->data.pSum->Vs > 0.0) {
      // normal solid properties
      ++pPendingParent->data.numChildren;
      pPendingParent->data.pSum->Vp += childPayload.Vp;
      pPendingParent->data.pSum->Vs += childPayload.Vs;
      pPendingParent->data.pSum->Density += childPayload.Density;
      pPendingParent->data.pSum->Qp += childPayload.Qp;
      pPendingParent->data.pSum->Qs += childPayload.Qs;
      pPendingParent->data.pSum->DepthFreeSurf += childPayload.DepthFreeSurf;
    } else {
      // replace average values with our values (force solid properties)
      pPendingParent->data.numChildren = 1;
      pPendingParent->data.pSum->Vp = childPayload.Vp;
      pPendingParent->data.pSum->Vs = childPayload.Vs;
      pPendingParent->data.pSum->Density = childPayload.Density;
      pPendingParent->data.pSum->Qp = childPayload.Qp;
      pPendingParent->data.pSum->Qs = childPayload.Qs;
      pPendingParent->data.pSum->DepthFreeSurf = childPayload.DepthFreeSurf;
    } // else
  } else if (childPayload.Vp > 0.0) {
    // if water and children with solid material properties have not
    // added their contributions, then add water contributions
    if (pPendingParent->data.pSum->Vs <= 0) {
      ++pPendingParent->data.numChildren;
      pPendingParent->data.pSum->Vp += childPayload.Vp;
      pPendingParent->data.pSum->Vs += childPayload.Vs;
      pPendingParent->data.pSum->Density += childPayload.Density;
      pPendingParent->data.pSum->Qp += childPayload.Qp;
      pPendingParent->data.pSum->Qs += childPayload.Qs;
      pPendingParent->data.pSum->DepthFreeSurf += childPayload.DepthFreeSurf;
    } // if
  } // else
} // _addToParent
		    
// ----------------------------------------------------------------------
// Update pending octant.
void 
cencalvm::average::AvgEngine::_updateOctant(etree_addr_t* pAddr,
		       const storage::PayloadStruct& childPayload)
{ // _updateOctant
  assert(0 != pAddr);
  assert(0 != _dbAvg);

  int err = etree_update(_dbAvg, *pAddr, &childPayload);
  if (0 != err)
    throw std::runtime_error("Could not update octant in etree database.");
} // _updateOctant

// ----------------------------------------------------------------------
// Process pending octant.
void
cencalvm::average::AvgEngine::_processOctant(const int pendingLevel)
{ // _processOctant
  assert(0 != _pPendingOctants);

  static const unsigned char X0_Y0_Z0 = 0x01;
  static const unsigned char X1_Y0_Z0 = 0x02;
  static const unsigned char X0_Y1_Z0 = 0x04;
  static const unsigned char X1_Y1_Z0 = 0x08;
  static const unsigned char X0_Y0_Z1 = 0x10;
  static const unsigned char X1_Y0_Z1 = 0x20;
  static const unsigned char X0_Y1_Z1 = 0x40;
  //static const unsigned char X1_Y1_Z1 = 0x80;
  static const unsigned char INC_FULL = 0xFF;
  static const unsigned char INC_X = X0_Y0_Z0 | X0_Y1_Z0 | X0_Y0_Z1 | X0_Y1_Z1;
  static const unsigned char INC_Y = X0_Y0_Z0 | X1_Y0_Z0 | X0_Y0_Z1 | X1_Y0_Z1;
  static const unsigned char INC_Z = X0_Y0_Z0 | X1_Y0_Z0 | X0_Y1_Z0 | X1_Y1_Z0;
  static const unsigned char INC_XY = X0_Y0_Z0 | X0_Y0_Z1;
  static const unsigned char INC_YZ = X0_Y0_Z0 | X1_Y0_Z0;
  static const unsigned char INC_XZ = X0_Y0_Z0 | X0_Y1_Z0;
  static const unsigned char INC_XYZ = X0_Y0_Z0;

  assert(0 <= pendingLevel && pendingLevel < _pendingSize);
  OctantPendingStruct& pendingOctant = _pPendingOctants[pendingLevel];

  assert(pendingOctant.isValid);
  switch(pendingOctant.processedChildren)
    { // switch
    case INC_FULL :
      break;
    case INC_X :
      ++_octantCounter.inc_x;
      break;
    case INC_Y :
      ++_octantCounter.inc_y;
      break;
    case INC_Z :
      ++_octantCounter.inc_z;
      break;
    case INC_XY :
      ++_octantCounter.inc_xy;
      break;
    case INC_YZ :
      ++_octantCounter.inc_yz;
      break;
    case INC_XZ :
      ++_octantCounter.inc_xz;
      break;
    case INC_XYZ :
      ++_octantCounter.inc_xyz;
      break;
    default :
      ++_octantCounter.inc_invalid;
    } // switch

  storage::PayloadStruct payload;
  const int numChildren = pendingOctant.data.numChildren;
  if (numChildren > 0) {
    payload.Vp = pendingOctant.data.pSum->Vp / numChildren;
    payload.Vs = pendingOctant.data.pSum->Vs / numChildren;
    payload.Density = pendingOctant.data.pSum->Density / numChildren;
    payload.Qp = pendingOctant.data.pSum->Qp / numChildren;
    payload.Qs = pendingOctant.data.pSum->Qs / numChildren;
    payload.DepthFreeSurf = 
      pendingOctant.data.pSum->DepthFreeSurf / numChildren;
  } else {
    // If there are no contributing children, then set payload
    // values to 'NODATA' values

    payload.Vp = storage::Payload::NODATAVAL;
    payload.Vs = storage::Payload::NODATAVAL;
    payload.Density = storage::Payload::NODATAVAL;
    payload.Qp = storage::Payload::NODATAVAL;
    payload.Qs = storage::Payload::NODATAVAL;
    payload.DepthFreeSurf = storage::Payload::NODATAVAL;
  } // if/else
  payload.FaultBlock = storage::Payload::INTERIORBLOCK;
  payload.Zone = storage::Payload::INTERIORZONE;

  _updateOctant(pendingOctant.pAddr, payload);
  if (pendingLevel > 0) {
    assert(_pPendingOctants[pendingLevel-1].pAddr->level == 
	   pendingOctant.pAddr->level-1);
    _addToParent(&_pPendingOctants[pendingLevel-1], pendingOctant.pAddr,
		 payload);
  } // if

  // Flag octant as processed
  pendingOctant.isValid = false;
  assert(pendingLevel ==_pendingCursor);
  _pendingCursor = pendingLevel - 1;
} // _processOctant

// ----------------------------------------------------------------------
// Find parent octant.
int
cencalvm::average::AvgEngine::_findParent(etree_addr_t* pAddr)
{ // _findParent
  assert(0 != _pPendingOctants);

  assert(0 != pAddr);
  assert(pAddr->level >= 0);
  assert(pAddr->level <= _pendingSize);

  if (0 == pAddr->level)
    return -1;

  // Process pending octants that are not parents of the current
  // octant.  These octants will be octants that have all
  // contributions from their children. If they are not "INC_FULL",
  // then they are associated with branches that are not "square".

  // Any pending octants at a higher level than the current octant
  // cannot be ancestors
  const int levelCur = pAddr->level;
  for (int pendingLevel=_pendingCursor; 
       pendingLevel >= levelCur;
       --pendingLevel) {
    _processOctant(pendingLevel);
  } // for
    
  // Find lowest level of valid pending octant
  int pendingLevel = levelCur - 1;
  while (pendingLevel >= 0 && !_pPendingOctants[pendingLevel].isValid)
    --pendingLevel;

  etree_addr_t ancestorCur; // address of octant that IS ancestor of current
  while(pendingLevel >= 0 && _pPendingOctants[pendingLevel].isValid) {
    storage::Geometry::findAncestor(&ancestorCur, 
					      *pAddr, pendingLevel);

    // If the octant at the current level is fully processed or it is
    // not a parent of the current octant, then finish processing it.
    const unsigned char INC_FULL = 0xFF;
    if (!_sameAddr(&ancestorCur, _pPendingOctants[pendingLevel].pAddr) ||
	INC_FULL == _pPendingOctants[pendingLevel].processedChildren) {
      assert(pendingLevel == _pendingCursor);
      _processOctant(pendingLevel);
      --pendingLevel;
    } // if
    else
      break;
  } // while
  
  // Create pending ancestors for current octant
  for (++pendingLevel; pendingLevel < pAddr->level; ++pendingLevel) {
    storage::Geometry::findAncestor(&ancestorCur, 
					      *pAddr, pendingLevel); 
    _createOctant(&ancestorCur);
  } // for

  return pendingLevel-1;
} // _findParent

// ----------------------------------------------------------------------
// Complete processing
void
cencalvm::average::AvgEngine::_finishProcessing(void)
{ // _finishProcessing
  assert(0 != _pPendingOctants);

  // Complete processing of the octants, begin at the bottom of the
  // tree and work towards the root
  for (int pendingLevel = _pendingSize-1; pendingLevel >= 0; --pendingLevel) {
    if (_pPendingOctants[pendingLevel].isValid) {
      assert(pendingLevel == _pendingCursor);
      _processOctant(pendingLevel);
    } // if
  } // for
} // _finishProcessing
  
// ----------------------------------------------------------------------
// Create a new octant in the average database and append it to the
void
cencalvm::average::AvgEngine::_createOctant(etree_addr_t* pAddr)
{ // _createOctant
  assert(0 != pAddr);
  assert(0 != _pPendingOctants);
  assert(0 != _dbAvg);

  const int pendingLevel = pAddr->level;
  assert(!_pPendingOctants[pendingLevel].isValid);
  assert(pendingLevel == _pendingCursor+1);  

  storage::PayloadStruct payload;
  int err = etree_append(_dbAvg, *pAddr, &payload);
  if (0 != err)
    throw std::runtime_error("Error occurred while appending new octant to etree.");

  *_pPendingOctants[pendingLevel].pAddr = *pAddr;
  _pPendingOctants[pendingLevel].processedChildren = 0x00;
  _pPendingOctants[pendingLevel].isValid = true;
  _pPendingOctants[pendingLevel].data.numChildren = 0;
  _pPendingOctants[pendingLevel].data.pSum->Vp = 0;
  _pPendingOctants[pendingLevel].data.pSum->Vs = 0;
  _pPendingOctants[pendingLevel].data.pSum->Density = 0;
  _pPendingOctants[pendingLevel].data.pSum->Qp = 0;
  _pPendingOctants[pendingLevel].data.pSum->Qs = 0;
  _pPendingOctants[pendingLevel].data.pSum->DepthFreeSurf = 0;
  _pPendingOctants[pendingLevel].data.pSum->FaultBlock = 
    storage::Payload::INTERIORBLOCK;
  _pPendingOctants[pendingLevel].data.pSum->Zone = 
    storage::Payload::INTERIORZONE;
  _pendingCursor = pendingLevel;

  ++_octantCounter.output;
  ++_octantCounter.interior;
} // _createOctant

// ----------------------------------------------------------------------
// Get bit associated with child's location relative to parent.
unsigned char
cencalvm::average::AvgEngine::_childOctantBit(etree_addr_t* pAddr) const
{ // _childOctantBit
  assert(0 != pAddr);

  const etree_tick_t mask = _LEFTMOSTONE >> pAddr->level;
  unsigned int shifts = (mask & pAddr->z) ? 4 : 0;
  shifts += (mask & pAddr->y) ? 2 : 0;
  shifts += (mask & pAddr->x) ? 1 : 0;

  return (1 << shifts);
} // _childOctantBit

// ----------------------------------------------------------------------
// Are octant addresses the same.
bool
cencalvm::average::AvgEngine::_sameAddr(etree_addr_t* pA,
					etree_addr_t* pB) const
{ // _sameAddr
  assert(0 != pA);
  assert(0 != pB);

  return (pA->x == pB->x &&
	  pA->y == pB->y &&
	  pA->z == pB->z &&
	  pA->t == pB->t &&
	  pA->level == pB->level);
} // _sameAddr
  
// version
// $Id: AvgEngine.cc 2064 2006-01-02 19:34:49Z brad $

// End of file 
