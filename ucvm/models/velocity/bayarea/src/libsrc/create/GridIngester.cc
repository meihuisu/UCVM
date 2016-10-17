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

#include "GridIngester.h" // implementation of class methods

#include "VMCreator.h" // USES VMCreator
#include "cencalvm/storage/Payload.h" // USES PayloadStruct
#include "cencalvm/storage/Geometry.h" // USES Geometry

extern "C" {
#include "etree.h"
}

#include <vector> // USES std::vector
#include <math.h> // USES fabs()

#include <fstream> // USES std::ifstream
#include <iostream> // USES std::cout
#include <stdexcept> // USES std::runtime_error
#include <sstream> // USES std::ostringstream
#include <iomanip> // USES std::resetiosflags(), std::setprecision()
#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
// Constructor
cencalvm::create::GridIngester::GridIngester(void) :
  _filenameParams(""),
  _filenameOut(""),
  _filenameTmp(""),
  _cacheSize(64),
  _description(""),
  _pGeom(0),
  _quiet(false)
{ // constructor
} // constructor

// ----------------------------------------------------------------------
// Destructor
cencalvm::create::GridIngester::~GridIngester(void)
{ // destructor
  delete _pGeom; _pGeom = 0;
} // destructor

// ----------------------------------------------------------------------
// Set velocity model geometry
void
cencalvm::create::GridIngester::geometry(const storage::Geometry* pGeom)
{ // geometry
  delete _pGeom; _pGeom = (0 != pGeom) ? _pGeom = pGeom->clone() : 0;
} // geometry

// ----------------------------------------------------------------------
// Create and then pack the velocity model database.
void
cencalvm::create::GridIngester::run(void) const
{ // run
  assert(std::string("") != _filenameParams);
  assert(std::string("") != _filenameTmp);
  assert(std::string("") != _filenameOut);

  VMCreator creator;
  creator.quiet(_quiet);

  std::string* pGridFilenames = 0;
  int numGrids = 0;
  _readParams(&pGridFilenames, &numGrids);
  
  creator.openDB(_filenameTmp.c_str(),
		 _cacheSize,
		 _description.c_str());

  for (int iGrid=0; iGrid < numGrids; ++iGrid)
    _addGrid(&creator, pGridFilenames[iGrid].c_str());

  creator.closeDB();

  creator.packDB(_filenameOut.c_str(), _filenameTmp.c_str(),
		 _cacheSize);
} // run

// ----------------------------------------------------------------------
// Read parameter file.
void
cencalvm::create::GridIngester::_readParams(std::string** ppGridFilenames,
					    int* pNumGrids) const
{ // _readParams
  assert(0 != ppGridFilenames);
  assert(0 != pNumGrids);
  
  std::ifstream fin(_filenameParams.c_str());
  if (!fin.is_open()) {
    std::ostringstream msg;
    msg << "Could not open parameter file '" << _filenameParams
	<< "' for reading.";
    throw std::runtime_error(msg.str());
  } // if

  int numGrids = 0;
  int capacity = 16;
  std::vector<std::string> gridFilenames;
  gridFilenames.resize(capacity);
  while (!fin.eof() && fin.good()) {
    if (numGrids == capacity) {
      capacity *= 2;
      gridFilenames.resize(capacity);
    } // if
    assert(numGrids < capacity);
    std::string filename;
    fin >> filename;
    if (fin.good())
      gridFilenames[numGrids++] = filename;
  } // while

  fin.close();

  if (!_quiet)
    std::cout << "Starting processing of " << numGrids << " grid files."
	      << std::endl;

  delete[] *ppGridFilenames; *ppGridFilenames = new std::string[numGrids];
  for (int iGrid=0; iGrid < numGrids; ++iGrid)
    (*ppGridFilenames)[iGrid] = gridFilenames[iGrid];
  *pNumGrids = numGrids;
} // _readParams

// ----------------------------------------------------------------------
// Add gridded data to database.
void
cencalvm::create::GridIngester::_addGrid(VMCreator* pCreator,
					 const char* filename) const
{ // addGrid
  assert(0 != pCreator);
  assert(0 != _pGeom);

  std::ifstream fin(filename);
  if (!fin.is_open()) {
    std::ostringstream msg;
    msg << "Could not open grid file '" << filename
	<< "' for reading.";
    throw std::runtime_error(msg.str());
  } // if

  if (!_quiet)
    std::cout
      << "Beginning processing of '" << filename << "'..." << std::endl;
  
  int numAdded = 0;
  int numIgnored = 0;
  try {
    double resHoriz = 0.0;
    double resVert = 0.0;
    int numX = 0;
    int numY = 0;
    int numZ = 0;
    int numTotal = 0;
    fin >> resHoriz >> resVert >> numX >> numY >> numZ >> numTotal;
    if (0.0 == resHoriz ||
	0.0 == resVert) {
      std::ostringstream msg;
      msg << "Could not read horizontal and vertical resolution in '"
	  << filename << "'.";
      std::runtime_error(msg.str());
    } // if
    // convert resHoriz and resVert from km to m
    resHoriz *= 1.0e+3;
    resVert *= 1.0e+3;
  
    const double tolerance = 1.0e-6;
    const double vertExag = resHoriz / resVert;
    const double vertExagE = _pGeom->vertExag();
    if (fabs(1.0 - vertExag/vertExagE) > tolerance) {
      std::ostringstream msg;
      msg << "Vertical exaggeration of " << vertExag << " in '" << filename
	  << "' does not match velocity model vertical exaggeration of "
	  << vertExagE << ".";
      throw std::runtime_error(msg.str());
    } // if

    const etree_tick_t level = _pGeom->level(resHoriz);
    const double edgeLen = _pGeom->edgeLen(level);
    if ( fabs(1.0 - resHoriz/edgeLen) > tolerance) {
      std::ostringstream msg;
      msg << "Horizontal resolution of " << resHoriz << " in '" << filename
	  << "' does not fit resolution of nearest level in database of "
	  << edgeLen << ".";
      throw std::runtime_error(msg.str());
    } // if    

    for (int i=0; i < numTotal; ++i) {
      double lon = 0.0;
      double lat = 0.0;
      double elev = 0.0;
      int volID = 0;
      storage::PayloadStruct payload;
      fin
	>> lon
	>> lat
	>> elev
	>> payload.Vp
	>> payload.Vs
	>> payload.Density
	>> payload.Qp
	>> payload.Qs
	>> payload.DepthFreeSurf
	>> payload.FaultBlock
	>> payload.Zone
	>> volID;
      if (!fin.good())
	throw std::runtime_error("Couldn't parse line.");
      if (payload.FaultBlock != cencalvm::storage::Payload::NODATABLOCK &&
	  payload.Zone != cencalvm::storage::Payload::NODATAZONE) {
	// convert elev and depth from km to m
	if (elev != cencalvm::storage::Payload::NODATAVAL)
	  elev *= 1.0e+3;
	if (payload.DepthFreeSurf != cencalvm::storage::Payload::NODATAVAL)
	  payload.DepthFreeSurf *= 1.0e+3;
      
	// convert Vp & Vs from km/s to m/s
	if (payload.Vp != cencalvm::storage::Payload::NODATAVAL)
	  payload.Vp *= 1.0e+3;
	if (payload.Vs != cencalvm::storage::Payload::NODATAVAL)
	  payload.Vs *= 1.0e+3;
      
	// convert Density from g/cm^3 to kg/m^3
	if (payload.Density != cencalvm::storage::Payload::NODATAVAL)
	  payload.Density *= 1.0e+3;
    
	pCreator->insert(payload, lon, lat, elev, resHoriz, _pGeom);
	numAdded++;
      } else {
	// convert elev and depth from km to m
	if (elev != cencalvm::storage::Payload::NODATAVAL)
	  elev *= 1.0e+3;
	if (payload.DepthFreeSurf != cencalvm::storage::Payload::NODATAVAL)
	  payload.DepthFreeSurf *= 1.0e+3;

	if (payload.FaultBlock != cencalvm::storage::Payload::NODATABLOCK) {
	  if (!_quiet)
	    std::cerr 
	      << std::resetiosflags(std::ios::fixed)
	      << std::setiosflags(std::ios::scientific)
	      << std::setprecision(6)
	      << lon << ", " << lat << ", " << elev << ", No fault block\n";
	  numIgnored++;
	} else {
	  if (!_quiet)
	    std::cerr
	      << std::resetiosflags(std::ios::fixed)
	      << std::setiosflags(std::ios::scientific)
	      << std::setprecision(6)
	      << lon << ", " << lat << ", " << elev << ", Ignoring\n";
	  numIgnored++;
	} // if/else
      
      } // if/else
    } // for

    fin.close();
  } catch (const std::exception& err) {
    std::cerr
      << "Caught error while reading grid from '" << filename << "'.\n"
      << "Successfully added " << numAdded << " points and ignored "
      << numIgnored << " others.\n"
      << "Error message: " << err.what();
    throw std::runtime_error(err.what());
  } // if

  if (!_quiet)
    std::cout << "Done procesing '" << filename << "'."
	      << "  # points added: " << numAdded
	      << ",  # points ignored: " << numIgnored
	      << std::endl;
} // addGrid

// version
// $Id: GridIngester.cc 2066 2006-01-02 20:22:28Z brad $

// End of file
