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

#include "VsGrader.h" // implementation of class methods

#include "cencalvm/storage/Payload.h" // USES SCHEMA
#include "cencalvm/storage/Geometry.h" // USES Geometry
#include "cencalvm/storage/GeomCenCA.h" // USES GeomCenCA
#include "cencalvm/storage/Projector.h" // USES Projector
#include "cencalvm/create/VMCreator.h" // USES VMCreator

extern "C" {
#include "etree.h"
}

#include <fstream> // USES std::ifstream
#include <strings.h> // USES strcasecmp()

#include <stdexcept> // USES std::runtime_error
#include <sstream> // USES std::ostringstream
#include <assert.h> // USES assert()
#include <iostream> // USES std::cout

// ----------------------------------------------------------------------
const double cencalvm::vsgrader::VsGrader::_AZIMUTHLEN = -36.3620 * M_PI/180.0;
const double cencalvm::vsgrader::VsGrader::_NODATAVAL = 1.0e+10;
const int16_t cencalvm::vsgrader::VsGrader::_NODATABLOCK = 999;
const int16_t cencalvm::vsgrader::VsGrader::_NODATAZONE = 999;

// ----------------------------------------------------------------------
// Constructor
cencalvm::vsgrader::VsGrader::VsGrader(void) :
  _gradientMaxVs(0),
  _minVs(0),
  _swcornerLon(0),
  _swcornerLat(0),
  _swcornerElev(0),
  _domainLen(0),
  _domainWidth(0),
  _domainHt(0),
  _resVert(1.0),
  _swcornerX(0),
  _swcornerY(0),
  _queryRes(0.0),
  _filenameIn(""),
  _filenameOut(""),
  _filenameTmp(""),
  _filenameParams(""),
  _cacheSize(128),
  _queryType(query::VMQuery::MAXRES),
  _pGeom(new cencalvm::storage::GeomCenCA),
  _quiet(false)
{ // constructor
} // constructor

// ----------------------------------------------------------------------
// Destructor
cencalvm::vsgrader::VsGrader::~VsGrader(void)
{ // destructor
  delete _pGeom; _pGeom = 0;
} // destructor

// ----------------------------------------------------------------------
// Set velocity model geometry.
void
cencalvm::vsgrader::VsGrader::geometry(const storage::Geometry* pGeometry)
{ // geometry
  delete _pGeom;
  _pGeom = (0 != pGeometry) ? pGeometry->clone() : 0;
} // geometry

// ----------------------------------------------------------------------
// Create Etree database with maximum gradient.
void
cencalvm::vsgrader::VsGrader::run(void)
{ // run
  _readParams();
  _initialize();
  _extract();
  _pack();
  _grade();
} // run

// ----------------------------------------------------------------------
// Read parameters from file.
void
cencalvm::vsgrader::VsGrader::_readParams(void)
{ // _readParams
  // Set default values
  _gradientMaxVs = 0.0;
  _minVs = 0.0;
  _swcornerLon = 0.0;
  _swcornerLat = 0.0;
  _swcornerElev = 0.0;
  _domainLen = 0.0;
  _domainWidth = 0.0;
  _domainHt = 0.0;
  _resVert = 0.0;

  if (!_quiet)
    std::cout << "Reading parameters from '" << _filenameParams << "'..." 
	      << std::endl;

  std::ifstream filein(_filenameParams.c_str());
  if (!filein.is_open() || !filein.good()) {
    std::ostringstream msg;
    msg << "Could not open file '" << _filenameParams
	<< "' to read VsGrader parameters.";
    throw std::runtime_error(msg.str());
  } // if

  std::string name;
  filein >> name;

  if (0 == strcasecmp(name.c_str(), "VsGrader")) {
    std::string token;
    const int maxIgnore = 128;
    filein.ignore(maxIgnore, '{');

    filein >> token;
    while(filein.good() && token != "}") {
      if (0 == strcasecmp(token.c_str(), "swcorner-lon")) {
	filein.ignore(maxIgnore, '=');
	filein >> _swcornerLon;
      } else if (0 == strcasecmp(token.c_str(), "swcorner-lat")) {
	filein.ignore(maxIgnore, '=');
	filein >> _swcornerLat;
      } else if (0 == strcasecmp(token.c_str(), "swcorner-elev")) {
	filein.ignore(maxIgnore, '=');
	filein >> _swcornerElev;
      } else if (0 == strcasecmp(token.c_str(), "domain-length")) {
	filein.ignore(maxIgnore, '=');
	filein >> _domainLen;
      } else if (0 == strcasecmp(token.c_str(), "domain-width")) {
	filein.ignore(maxIgnore, '=');
	filein >> _domainWidth;
      } else if (0 == strcasecmp(token.c_str(), "domain-height")) {
	filein.ignore(maxIgnore, '=');
	filein >> _domainHt;
      } else if (0 == strcasecmp(token.c_str(), "resolution-vert")) {
	filein.ignore(maxIgnore, '=');
	filein >> _resVert;
      } else if (0 == strcasecmp(token.c_str(), "vs-gradient-max")) {
	filein.ignore(maxIgnore, '=');
	filein >> _gradientMaxVs;
      } else if (0 == strcasecmp(token.c_str(), "vs-min")) {
	filein.ignore(maxIgnore, '=');
	filein >> _minVs;
      } else if (0 == strcasecmp(token.c_str(), "cache-size")) {
	filein.ignore(maxIgnore, '=');
	int value;
	filein >> value;
	if (value > 0)
	  _cacheSize = value;
      } else if (0 == strcasecmp(token.c_str(), "query-resolution")) {	
	filein.ignore(maxIgnore, '=');
	double value;
	filein >> value;
	if (value >= 0)
	  _queryRes = value;
      } else if (0 == strcasecmp(token.c_str(), "query-type")) {
	filein.ignore(maxIgnore, '=');
	std::string value;
	filein >> value;
	if (0 == strcasecmp("maxres", value.c_str()))
	  _queryType = query::VMQuery::MAXRES;
	else if (0 == strcasecmp("fixedres" , value.c_str()))
	  _queryType = query::VMQuery::FIXEDRES;
	else if (0 == strcasecmp("waveres", value.c_str()))
	  _queryType = query::VMQuery::WAVERES;
	else {
	  std::ostringstream msg;
	  msg << "Could not parse query type '" << value
	      << "' into a known type of query.";
	  throw std::runtime_error(msg.str());
	} // else
      } else {
	std::ostringstream msg;
	msg << "Could not parse '" << token << "' into a parameter setting.";
	throw std::runtime_error(msg.str());
      } // else
      
      filein >> token;
    } // while
    if (!filein.good())
      throw std::runtime_error("I/O error while parsing settings.");

    std::ostringstream msg;
    bool ok = true;
    if (0 == _gradientMaxVs) {
      ok = false;
      msg << "Parameter file must specify 'vs-gradient-max'.\n";
    } // if
    if (0 == _minVs) {
      ok = false;
      msg << "Parameter file must specify 'vs-min'.\n";
    } // if
    if (0 == _swcornerLon) {
      ok = false;
      msg << "Parameter file must specify 'swcorner-lon'.\n";
    } // if
    if (0 == _swcornerLat) {
      ok = false;
      msg << "Parameter file must specify 'swcorner-lat'.\n";
    } // if
    if (0 == _swcornerElev) {
      ok = false;
      msg << "Parameter file must specify 'swcorner-elev'.\n";
    } // if
    if (0 == _domainLen) {
      ok = false;
      msg << "Parameter file must specify 'domain-len'.\n";
    } // if
    if (0 == _domainWidth) {
      ok = false;
      msg << "Parameter file must specify 'domain-width'.\n";
    } // if
    if (0 == _domainHt) {
      ok = false;
      msg << "Parameter file must specify 'domain-height'.";
    } // if
    if (0 == _resVert) {
      ok = false;
      msg << "Parameter file must specify 'resolution-vert'.";
    } // if
    if (query::VMQuery::MAXRES != _queryType && _queryRes <= 0) {
      ok = false;
      msg << "Parameter file must specify 'query-res' when 'query-type' "
	  << "is not 'maxres'.";
    } // if
    if (!ok)
      throw std::runtime_error(msg.str());      
  } else {
    std::ostringstream msg;
    msg << "Could not parse '" << name << "' into 'VsGrader'.";
    throw std::runtime_error(msg.str());
  } // else

  filein.close();
} // _readParams

// ----------------------------------------------------------------------
// Initialize.
void
cencalvm::vsgrader::VsGrader::_initialize(void)
{ // _initialize
  assert(0 != _pGeom);

  if (!_quiet)
    std::cout << "Initializing grader..." << std::endl;

  const storage::Projector* pProj = _pGeom->projector();
  assert(0 != pProj);
  pProj->project(&_swcornerX, &_swcornerY, _swcornerLon, _swcornerLat);
} // _initialize

// ----------------------------------------------------------------------
// Extract database.
void
cencalvm::vsgrader::VsGrader::_extract(void) const
{ // _extract
  assert(0 != _pGeom);

  if (!_quiet)
    std::cout << "Extracting data from '" << _filenameIn << "'..."
	      << std::endl;

  query::VMQuery dbOrig;
  dbOrig.queryType(_queryType);
  dbOrig.queryRes(_queryRes);
  dbOrig.cacheSize(_cacheSize);
  dbOrig.filename(_filenameIn.c_str());
  dbOrig.open();

  std::ostringstream description;
  description << _pGeom->metadata() << "\n"
	      << "Gradient in Vs limited to " << _gradientMaxVs << ".";
  create::VMCreator dbNew;
  dbNew.openDB(_filenameTmp.c_str(),
	       _cacheSize,
	       description.str().c_str());
  
  const double resHoriz = _resVert * _pGeom->vertExag();
  const int numLen = int(1 + _domainLen / resHoriz);
  const int numWidth = int(1 + _domainWidth / resHoriz);
  const int numHt = int(1 + _domainHt / _resVert);

  cencalvm::storage::PayloadStruct payload;
  const int numVals = 8;
  const char* valNames[] = { "Vp", "Vs", "Density", "Qp", "Qs", 
			     "DepthFreeSurf", "FaultBlock", "Zone" };
  dbOrig.queryVals(valNames, numVals);
  double* pVals = (numVals > 0) ? new double[numVals] : 0;
  for (int iLen=0; iLen < numLen; ++iLen) {
    for (int iWidth=0; iWidth < numWidth; ++iWidth) {
      double lon = 0;
      double lat = 0;
      _indexToLonLat(&lon, &lat, iLen, iWidth);
      for (int iHt=0; iHt < numHt; ++iHt) {
	const double elev = _indexToElev(iHt);
	dbOrig.query(&pVals, numVals, lon, lat, elev);
	payload.Vp = pVals[0];
	payload.Vs = pVals[1];
	payload.Density = pVals[2];
	payload.Qp = pVals[3];
	payload.Qs = pVals[4];
	payload.DepthFreeSurf = pVals[5];
	payload.FaultBlock = int16_t(pVals[6]);
	payload.Zone = int16_t(pVals[7]);
	if (payload.Vs < 0) {
	  payload.Vp = _NODATAVAL;
	  payload.Vs = _NODATAVAL;
	  payload.Density = _NODATAVAL;
	  payload.Qp = _NODATAVAL;
	  payload.Qs = _NODATAVAL;
	  payload.DepthFreeSurf = _NODATAVAL;
	  payload.FaultBlock = _NODATABLOCK;
	  payload.Zone = _NODATAZONE;
	} else if (payload.Vs < _minVs)
	  payload.Vs = _minVs;

	dbNew.insert(payload, lon, lat, elev, resHoriz, _pGeom);
      } // for
    } // for
  } // for
  delete[] pVals; pVals = 0;
  
  dbNew.closeDB();
  dbOrig.close();

  if (!_quiet)
    std::cout << "Done extracting." << std::endl;
} // _extract

// ----------------------------------------------------------------------
// Pack database.
void
cencalvm::vsgrader::VsGrader::_pack(void) const
{ // _pack
  cencalvm::create::VMCreator creator;
  creator.quiet(_quiet);
  creator.packDB(_filenameOut.c_str(), _filenameTmp.c_str(), _cacheSize);
} // _pack

// ----------------------------------------------------------------------
// Limit gradient in database.
void
cencalvm::vsgrader::VsGrader::_grade(void) const
{ // _grade
  assert(0 != _pGeom);

  if (!_quiet)
    std::cout << "Starting grading of database." << std::endl;

  // Open database for reading/writing
  const int cacheSize = 512;
  const int numDims = 3;
  const int payloadSize = sizeof(cencalvm::storage::PayloadStruct);
  etree_t* db = etree_open(_filenameOut.c_str(), O_RDWR,
			       cacheSize, payloadSize, numDims);
  if (0 == db)
    throw std::runtime_error("Could not open etree database.");

  const double resHoriz = _resVert * _pGeom->vertExag();
  const int numLen = int(1 + _domainLen / resHoriz);
  const int numWidth = int(1 + _domainWidth / resHoriz);
  const int numHt = int(1 + _domainHt / _resVert);
  
  int dataLen = 0;
  cencalvm::storage::PayloadStruct* pData = 0;
  bool* pIsChanged = 0;

  // Limit gradient in vertical direction
  if (!_quiet)
    std::cout << "Grading database in vertical direction." << std::endl;
  dataLen = (numHt > 0) ? numHt : 0;
  pData = (dataLen > 0) ? new cencalvm::storage::PayloadStruct[dataLen] : 0;
  pIsChanged = (dataLen > 0) ? new bool[dataLen] : 0;
  for (int iLen=0; iLen < numLen; ++iLen)
    for (int iWidth=0; iWidth < numWidth; ++iWidth) {
      double lon = 0;
      double lat = 0;
      _indexToLonLat(&lon, &lat, iLen, iWidth);
      for (int iHt=0; iHt < numHt; ++iHt) {
	const double elev = _indexToElev(iHt);
	_pullData(&pData[iHt], lon, lat, elev, db);
      } // for
      const double maxdiff = _gradientMaxVs * _resVert;
      _limitDiff(&pData, &pIsChanged, dataLen, maxdiff);
      
      for (int iHt=0; iHt < numHt; ++iHt)
	if (pIsChanged[iHt]) {
	  const double elev = _indexToElev(iHt);
	  _pushData(db, pData[iHt], lon, lat, elev);
	} // for
    } // for
  delete[] pData; pData = 0;
  delete[] pIsChanged; pIsChanged = 0;
  dataLen = 0;

  // Limit gradient in length direction
  if (!_quiet)
    std::cout << "Grading database in length direction." << std::endl;
  dataLen = (numLen > 0) ? numLen : 0;
  pData = (dataLen > 0) ? new cencalvm::storage::PayloadStruct[dataLen] : 0;
  pIsChanged = (dataLen > 0) ? new bool[dataLen] : 0;
  for (int iHt=0; iHt < numHt; ++iHt) {
    const double elev = _indexToElev(iHt);
    double lon = 0;
    double lat = 0;
    for (int iWidth=0; iWidth < numWidth; ++iWidth) {
      for (int iLen=0; iLen < numLen; ++iLen) {
	_indexToLonLat(&lon, &lat, iLen, iWidth);
	_pullData(&pData[iLen], lon, lat, elev, db);
      } // for
      const double maxdiff = _gradientMaxVs * resHoriz;
      _limitDiff(&pData, &pIsChanged, dataLen, maxdiff);
      
      for (int iLen=0; iLen < numLen; ++iLen)
	if (pIsChanged[iLen]) {
	  _indexToLonLat(&lon, &lat, iLen, iWidth);
	  _pushData(db, pData[iLen], lon, lat, elev);
	} // for
    } // for
  } // for
  delete[] pData; pData = 0;
  delete[] pIsChanged; pIsChanged = 0;
  dataLen = 0;
      
  // Limit gradient in width direction
  if (!_quiet)
    std::cout << "Grading database in width direction." << std::endl;
  dataLen = (numWidth > 0) ? numWidth : 0;
  pData = (dataLen > 0) ? new cencalvm::storage::PayloadStruct[dataLen] : 0;
  pIsChanged = (dataLen > 0) ? new bool[dataLen] : 0;
  for (int iHt=0; iHt < numHt; ++iHt) {
    const double elev = _indexToElev(iHt);
    double lon = 0;
    double lat = 0;
    for (int iLen=0; iLen < numLen; ++iLen) {
      for (int iWidth=0; iWidth < numWidth; ++iWidth) {
	_indexToLonLat(&lon, &lat, iLen, iWidth);
	_pullData(&pData[iWidth], lon, lat, elev, db);
      } // for
      const double maxdiff = _gradientMaxVs * resHoriz;
      _limitDiff(&pData, &pIsChanged, dataLen, maxdiff);
      
      for (int iWidth=0; iWidth < numWidth; ++iWidth)
	if (pIsChanged[iWidth]) {
	  _indexToLonLat(&lon, &lat, iLen, iWidth);
	  _pushData(db, pData[iWidth], lon, lat, elev);
	} // for
    } // for
  } // for
  delete[] pData; pData = 0;
  delete[] pIsChanged; pIsChanged = 0;
  dataLen = 0;

  // close database
  if (0 != etree_close(db))
    throw std::runtime_error(etree_strerror(etree_errno(db)));

  if (!_quiet)
    std::cout << "Done grading database." << std::endl;
} // _grade

// ----------------------------------------------------------------------
// Compute longitude and latitude given length and width
void
cencalvm::vsgrader::VsGrader::_indexToLonLat(double* pLon,
					     double* pLat,
					     const int indexL,
					     const int indexW) const
{ // _indexToLonLat
  assert(0 != _pGeom);

  const double azR = _AZIMUTHLEN;
  const double resHoriz = _resVert * _pGeom->vertExag();
  const double l = indexL * resHoriz;
  const double w = indexW * resHoriz;
  const double x = _swcornerX + l * sin(azR) + w * cos(azR);
  const double y = _swcornerY + l * cos(azR) - w * sin(azR);

  const storage::Projector* pProj = _pGeom->projector();
  assert(0 != pProj);  
  pProj->invProject(pLon, pLat, x, y);
} // _indexToLonLat

// ----------------------------------------------------------------------
// Get data from database. Payload is returned through the parameters.
void
cencalvm::vsgrader::VsGrader::_pullData(cencalvm::storage::PayloadStruct* pPayload,
					const double lon,
					const double lat,
					const double elev,
					etree_t* pDB) const
{ // _pullData
  assert(0 != pPayload);
  assert(0 != pDB);
  assert(0 != _pGeom);

  etree_addr_t addr;
  addr.type = ETREE_LEAF;

  const double resHoriz = _resVert * _pGeom->vertExag();
  addr.level = _pGeom->level(resHoriz);

  _pGeom->lonLatElevToAddr(&addr, lon, lat, elev);
  
  etree_addr_t resAddr;
  int err = etree_search(pDB, addr, &resAddr, "*", pPayload);
  if (0 != err)
    throw std::runtime_error("Could not find octant in database.");
} // _pullData

// ----------------------------------------------------------------------
// Update data in database.
void
cencalvm::vsgrader::VsGrader::_pushData(etree_t* pDB,
			      const cencalvm::storage::PayloadStruct& payload,
					const double lon,
					const double lat,
					const double elev) const
{ // _pushData
  assert(0 != pDB);
  assert(0 != _pGeom);

  etree_addr_t addr;
  addr.type = ETREE_LEAF;

  const double resHoriz = _resVert * _pGeom->vertExag();
  addr.level = _pGeom->level(resHoriz);

  _pGeom->lonLatElevToAddr(&addr, lon, lat, elev);
  int err = etree_update(pDB, addr, &payload);
  if (0 != err)
    throw std::runtime_error("Could not update octant in database.");
} // _pushData

// ----------------------------------------------------------------------
// Limit maximum difference in Vs. Other material properties are
void
cencalvm::vsgrader::VsGrader::_limitDiff(cencalvm::storage::PayloadStruct** ppData,
					 bool** ppIsChanged,
					 const int dataLen,
					 const double maxdiff) const
{ // _limitDiff
  assert(0 != ppData);
  assert(0 != ppIsChanged);

  for (int i=0; i < dataLen; ++i)
    (*ppIsChanged)[i] = false;

  for (int i=1; i < dataLen; ++i)
    if ((*ppData)[i].Vs - (*ppData)[i-1].Vs > maxdiff) {
      const double vs = (*ppData)[i-1].Vs + maxdiff;
      cencalvm::storage::PayloadStruct* pPayload = (*ppData)+i;
      const double scale = vs / pPayload->Vs;
      pPayload->Vp *= scale;
      pPayload->Vs = vs;
      pPayload->Density *= scale;
      pPayload->Qp *= scale;
      pPayload->Qs *= scale;
      (*ppIsChanged)[i] = true;
    } // if
  for (int i=dataLen-2; i >= 0; --i)
    if ((*ppData)[i].Vs - (*ppData)[i+1].Vs > maxdiff) {
      const double vs = (*ppData)[i+1].Vs + maxdiff;
      cencalvm::storage::PayloadStruct* pPayload = (*ppData)+i;
      const double scale = vs / pPayload->Vs;
      pPayload->Vp *= scale;
      pPayload->Vs = vs;
      pPayload->Density *= scale;
      pPayload->Qp *= scale;
      pPayload->Qs *= scale;
      (*ppIsChanged)[i] = true;
    } // if
} // _limitDiff

// version
// $Id: VsGrader.cc 4189 2008-09-12 00:10:44Z brad $

// End of file 
