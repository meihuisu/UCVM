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

#include "Geometry.h" // ISA Geometry
#include "GeomCenCA.h" // implementation of class methods

#include "Projector.h"
#include "ErrorHandler.h"

extern "C" {
#include "etree.h"
}

#include <stdexcept> // USES std::runtime_error
#include <sstream> // USES std::ostringstream
#include <iomanip> // USES setw(), setiosflags(), resetiosflags()
#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
// @warning If you change these values, you MUST regenerate the Etree
// database!!!!
const double cencalvm::storage::GeomCenCA::_ROOTLEN = 1638400.0;
const double cencalvm::storage::GeomCenCA::_AZ = 53.638;
const double cencalvm::storage::GeomCenCA::_VERTEXAG = 4.0;
const double cencalvm::storage::GeomCenCA::_PROJXNW = -74937.817;
const double cencalvm::storage::GeomCenCA::_PROJYNW = 380186.124;
const double cencalvm::storage::GeomCenCA::_BUFFERNW = 409600.0;
const double cencalvm::storage::GeomCenCA::_BUFFERSW = 204800.0;
const double cencalvm::storage::GeomCenCA::_MAXELEV = 12800.0;
const char* cencalvm::storage::GeomCenCA::_METADATA =
  "Central CA velocity model";

// ----------------------------------------------------------------------
cencalvm::storage::GeomCenCA::GeomCenCA(void) :
  _projXNWRoot(_PROJXNW 
	       - _BUFFERNW*cos(_AZ*M_PI/180.0)
	       - _BUFFERSW*sin(_AZ*M_PI/180.0)),
  _projYNWRoot(_PROJYNW
	       + _BUFFERNW*sin(_AZ*M_PI/180.0)
	       - _BUFFERSW*cos(_AZ*M_PI/180.0)),
  _pProj(new Projector)
{ // constructor
} // constructor

// ----------------------------------------------------------------------
cencalvm::storage::GeomCenCA::~GeomCenCA(void)
{ // destructor
  delete _pProj; _pProj = 0;
} // destructor

// ----------------------------------------------------------------------
// Clone geometry.
cencalvm::storage::Geometry*
cencalvm::storage::GeomCenCA::clone(void) const
{ // clone
  return new GeomCenCA();
} // clone

// ----------------------------------------------------------------------
// Map global coordinates xyz to etree address. 
void
cencalvm::storage::GeomCenCA::lonLatElevToAddr(etree_addr_t* pAddr,
					      const double lon,
					      const double lat,
					      const double elev)
{ // lonLatElevToAddr
  assert(0 != _pProj);
  assert(0 != pAddr);
  assert(0 <= pAddr->level && 32 > pAddr->level);

  // Convert lon, lat, elev to projected coordinates
  double x = 0;
  double y = 0;
  _pProj->project(&x, &y, lon, lat);
  
  // convert projected coordinates to coordinates along root octant
  const double azR = _AZ * M_PI / 180.0;
  const double p = 
    (x - _projXNWRoot) * cos(azR) - (y - _projYNWRoot) * sin(azR);
  const double q = 
    (x - _projXNWRoot) * sin(azR) + (y - _projYNWRoot) * cos(azR);
  const double r = (elev-_MAXELEV)*_VERTEXAG + _ROOTLEN;

  if (p < 0.0 || p > _ROOTLEN ||
      q < 0.0 || q > _ROOTLEN ||
      r < 0.0 || r > _ROOTLEN) {
    std::ostringstream msg;
    msg
      << std::resetiosflags(std::ios::fixed)
      << std::setiosflags(std::ios::scientific)
      << std::setprecision(6)
      << "Location (" << lon << ", " << lat << ", " << elev << ")\n"
      << "not in domain. Coordinates relative to root octant are:\n"
      << "  p: " << p/_ROOTLEN << "\n"
      << "  q: " << q/_ROOTLEN << "\n"
      << "  r: " << r/_ROOTLEN << "\n";
    throw std::runtime_error(msg.str());
  } // if

  const double res = _ROOTLEN / ((etree_tick_t) 1 << pAddr->level);
  const etree_tick_t tickLen = 0x80000000 >> pAddr->level;

  pAddr->x = tickLen*etree_tick_t(p / res);
  pAddr->y = tickLen*etree_tick_t(q / res);
  pAddr->z = tickLen*etree_tick_t(r / res);
  pAddr->t = 0;
} // lonLatElevToAddr
  
// ----------------------------------------------------------------------
// Get global coordinates of octant centroid.
void
cencalvm::storage::GeomCenCA::addrToLonLatElev(double* pLon,
					      double* pLat,
					      double* pElev,
					      etree_addr_t* pAddr)
{ // addrToLonLatElev
  assert(0 != _pProj);
  assert(0 != pAddr);
  assert(0 <= pAddr->level && 32 > pAddr->level);

  // find coordinates in rotated projected space
  const double res = _ROOTLEN / ((etree_tick_t) 1 << pAddr->level);
  const etree_tick_t tickLen = 0x80000000 >> pAddr->level;
  const double p = (pAddr->x / double(tickLen) + 0.5) * res;
  const double q = (pAddr->y / double(tickLen) + 0.5) * res;
  const double r = (pAddr->z / double(tickLen) + 0.5) * res;

  // find projected coordinates
  const double azR = _AZ * M_PI / 180.0;
  const double x = _projXNWRoot + p * cos(azR) + q * sin(azR);
  const double y = _projYNWRoot - p * sin(azR) + q * cos(azR);
  *pElev = (r - _ROOTLEN) / _VERTEXAG + _MAXELEV;

  // unproject to get geographic coordinates
  _pProj->invProject(pLon, pLat, x, y);
} // addrToLonLatElev
  

// End of file 
