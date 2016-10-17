// -*- C++ -*-
//
// ----------------------------------------------------------------------
//
//                           Brad T. Aagaard
//                        U.S. Geological Survey
//
// <LicenseText>
//
// ----------------------------------------------------------------------
//

#include <portinfo>

#include "spatialdata/spatialdb/SpatialDB.hh" // ISA SpatialDB object
#include "CenCalVMDB.h" // Implementation of class methods

#include "cencalvm/query/VMQuery.h" // USES VMQuery
#include "spatialdata/geocoords/CoordSys.hh" // USES CoordSys
#include "spatialdata/geocoords/CSGeo.hh" // USES CoordSys
#include "spatialdata/geocoords/Converter.hh" // USES Converter
#include "cencalvm/storage/ErrorHandler.h" // USES ErrorHandler
#include "cencalvm/storage/Payload.h" // USES Payload::NODATAVAL

#include <strings.h> // USES strcasecmp()
#include <stdexcept> // USES std::runtime_error
#include <math.h> // USES pow()
#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
/// Default constructor
cencalvm::extensions::cencalvmdb::CenCalVMDB::CenCalVMDB(void) :
  _minVs(0),
  _vsVal(-1),
  _vpVal(-1),
  _densityVal(-1),
  _pQuery(new cencalvm::query::VMQuery),
  _pCS(new spatialdata::geocoords::CSGeo)
{ // constructor
  _pCS->ellipsoid("WGS84");
  _pCS->datumHoriz("WGS84");
  _pCS->datumVert("mean sea level");
  _pCS->isGeocentric(false);
  _pCS->toMeters(1.0);
  _pCS->initialize();
} // constructor

// ----------------------------------------------------------------------
/// Default destructor
cencalvm::extensions::cencalvmdb::CenCalVMDB::~CenCalVMDB(void)
{ // destructor
  delete _pQuery; _pQuery = 0;
  delete _pCS; _pCS = 0;
} // destructor

// ----------------------------------------------------------------------
// Open the database and prepare for querying.
void
cencalvm::extensions::cencalvmdb::CenCalVMDB::open(void)
{ // open
  assert(0 != _pQuery);

  _pQuery->open();
  if (storage::ErrorHandler::ERROR == _pQuery->errorHandler()->status())
    throw std::runtime_error(_pQuery->errorHandler()->message());
} // open

// ----------------------------------------------------------------------
// Close the database.
void
cencalvm::extensions::cencalvmdb::CenCalVMDB::close(void)
{ // close
  assert(0 != _pQuery);

  _pQuery->close(); 
  if (storage::ErrorHandler::ERROR == _pQuery->errorHandler()->status())
    throw std::runtime_error(_pQuery->errorHandler()->message());
} // close

// ----------------------------------------------------------------------
// Set query type.
void
cencalvm::extensions::cencalvmdb::CenCalVMDB::queryType(const query::VMQuery::QueryEnum queryType)
{ // queryType
  assert(0 != _pQuery);

  _pQuery->queryType(queryType);
  if (storage::ErrorHandler::ERROR == _pQuery->errorHandler()->status())
    throw std::runtime_error(_pQuery->errorHandler()->message());
} // queryType

// ----------------------------------------------------------------------
// Set values to be returned by queries.
void
cencalvm::extensions::cencalvmdb::CenCalVMDB::queryVals(const char* const* names,
							const int numVals)
{ // queryVals
  assert(0 != _pQuery);

  _pQuery->queryVals(names, numVals);
  if (storage::ErrorHandler::ERROR == _pQuery->errorHandler()->status())
    throw std::runtime_error(_pQuery->errorHandler()->message());

  _vsVal = -1;
  _vpVal = -1;
  _densityVal = -1;
  for (int i=0; i < numVals; ++i) {
    if (0 == strcasecmp(names[i], "Vs"))
      _vsVal = i;
    if (0 == strcasecmp(names[i], "Vp"))
      _vpVal = i;
    if (0 == strcasecmp(names[i], "density"))
      _densityVal = i;
  } // for
} // queryVals

// ----------------------------------------------------------------------
// Query the database.
int
cencalvm::extensions::cencalvmdb::CenCalVMDB::query(double* pVals,
					const int numVals,
					const double* coords,
					const int numDims,
			      const spatialdata::geocoords::CoordSys* pCSQuery)
{ // query
  assert(0 != _pQuery);

  double buffer[3];
  assert(3 == numDims);

  const int numLocs = 1;
  for (int i=0; i < numDims; ++i)
    buffer[i] = coords[i];
  spatialdata::geocoords::Converter::convert(buffer, numLocs, numDims,
					     _pCS, pCSQuery);
  
  /** :KLUDGE:
   * Prevent elevations from being deeper than 45.0 km.
   */
  if (buffer[2] < -44.95e+3)
    buffer[2] = -44.95e+3;

  _pQuery->query(&pVals, numVals, buffer[0], buffer[1], buffer[2]);
  cencalvm::storage::ErrorHandler* pErrHandler = _pQuery->errorHandler();
  if (storage::ErrorHandler::ERROR == pErrHandler->status())
    throw std::runtime_error(pErrHandler->message());

  if (_vsVal >= 0) {
    double* pVs = &pVals[_vsVal];
    int iter = 1;
    while (*pVs < 0.0) {
      const int maxIter = 8;
      const double elevDiff = 25.0;
      pErrHandler->resetStatus();
      const double newElev = buffer[2] - pow(2,iter-1)*elevDiff;
      _pQuery->query(&pVals, numVals, buffer[0], buffer[1], newElev);
      if (iter < maxIter)
	++iter;
      else
	break;
    } // while
    if (0.0 < *pVs && *pVs < _minVs)
      *pVs = _minVs;
  } // if
  if (_vpVal >= 0) {
    // Enforce minimum Vp
    double* pVp = &pVals[_vpVal];
    const double minVp = 1360.0 + 1.16 * _minVs;
    if (*pVp > 0 && *pVp < minVp)
      *pVp = minVp;
  } // if    
  if (_densityVal >= 0) {
    double* pDensity = &pVals[_densityVal];
    const double minDensity = 2000.0;
    // If not water enforce minimum density
    if (*pDensity > 1000.0 && *pDensity < minDensity)
      *pDensity = minDensity;
  } // if    


  int err = 0;
  if (cencalvm::storage::ErrorHandler::WARNING == pErrHandler->status()) {
    err = 1;
    pErrHandler->resetStatus();
  } // if

  return err;
} // query


// End of file 
