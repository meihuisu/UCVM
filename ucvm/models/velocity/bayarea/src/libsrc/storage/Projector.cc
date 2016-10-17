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

#include "Projector.h" // implementation of class methods

#include "ErrorHandler.h" // HOLDSA ErrorHandler

extern "C" {
#include "proj_api.h" // USES PROJ4
};

#include <stdexcept> // USES std::runtime_error
#include <sstream> // USES std::ostringstream
#include <iomanip> // USES setw(), setiosflags(), resetiosflags()
#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
// @warning These values must be compatible with the 3-D geologic model!!!
const char* cencalvm::storage::Projector::_PROJECTION = "tmerc";
const double cencalvm::storage::Projector::_MERIDIAN = -123.0;
const double cencalvm::storage::Projector::_LAT = 35.0;
const double cencalvm::storage::Projector::_SCALE = 0.9996;
const char* cencalvm::storage::Projector::_DATUM = "NAD83";
const char* cencalvm::storage::Projector::_UNITS = "m";

// ----------------------------------------------------------------------
cencalvm::storage::Projector::Projector(void) :
  _pProj(0)
{ // constructor
  std::ostringstream args;
  args
    << "+proj=" << _PROJECTION
    << " +lon_0=" << _MERIDIAN
    << " +lat_0=" << _LAT
    << " +k=" << _SCALE
    << " +datum=" << _DATUM
    << " +units=" << _UNITS;
  
  pj_free(_pProj);
  _pProj = pj_init_plus(args.str().c_str());
  if (0 == _pProj) {
    std::ostringstream msg;
    msg << "Error while initializing projection:\n"
	<< "  " << pj_strerrno(pj_errno) << "\n"
	<< "Projection parameters:\n"
	<< "  " << args;
    throw std::runtime_error(msg.str());
  } // if
} // constructor

// ----------------------------------------------------------------------
cencalvm::storage::Projector::~Projector(void)
{ // destructor
  pj_free(_pProj);
} // destructor

// ----------------------------------------------------------------------
void
cencalvm::storage::Projector::project(double* pX,
				      double* pY,
				      const double lon,
				      const double lat) const
{ // project
  assert(0 != pX);
  assert(0 != pY);
  assert(0 != _pProj);

  const double degToRad = M_PI / 180.0;

  projUV lonlat;
  lonlat.u = lon * degToRad;
  lonlat.v = lat * degToRad;
  projUV xy = pj_fwd(lonlat, _pProj);
  if (HUGE_VAL == xy.u) {
    std::ostringstream msg;
    msg << "Error while projecting location.\n"
	<< "  " << pj_strerrno(pj_errno) << "\n"
	<< "  longitude: " << lon << "\n"
	<< "  latitude: " << lat << "\n";
    throw std::runtime_error(msg.str());
  } // if
  *pX = xy.u;
  *pY = xy.v;
} // project

// ----------------------------------------------------------------------
// Apply inverse projection.
void
cencalvm::storage::Projector::invProject(double* pLon,
					 double* pLat,
					 const double x,
					 const double y) const
{ // invProject
  assert(0 != pLon);
  assert(0 != pLat);
  assert(0 != _pProj);

  const double radToDeg = 180.0 / M_PI;

  projUV xy;
  xy.u = x;
  xy.v = y;
  projUV lonlat = pj_inv(xy, _pProj);
  if (HUGE_VAL == lonlat.u) {
    std::ostringstream msg;
    msg << "Error while projecting location.\n"
	<< "  " << pj_strerrno(pj_errno) << "\n"
	<< "  x: " << x << "\n"
	<< "  y: " << y << "\n";
    throw std::runtime_error(msg.str());
  } // if
  *pLon = lonlat.u * radToDeg;
  *pLat = lonlat.v * radToDeg;
} // invProject

// version
// $Id: Projector.cc 2062 2005-12-23 02:43:23Z brad $

// End of file 
