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

#include "VMCreator.h" // implementation of class methods

#include "cencalvm/storage/Payload.h" // USES SCHEMA
#include "cencalvm/storage/Geometry.h" // USES Geometry

extern "C" {
#include "etree.h"
}

#include <iostream> // USES std::cout

#include <time.h> // USES strftime(), gettimeofday(), localtime()
#include <unistd.h> // USES gethostname()
#include <stdlib.h> // USES free()

#include <stdexcept> // USES std::runtime_error
#include <sstream> // USES std::ostringstream
#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
// Constructor
cencalvm::create::VMCreator::VMCreator(void) :
  _filename(""),
  _pDB(0),
  _quiet(false)
{ // constructor
} // constructor

// ----------------------------------------------------------------------
// Destructor
cencalvm::create::VMCreator::~VMCreator(void)
{ // destructor
  if (0 != _pDB) {
    etree_close(_pDB); _pDB = 0;
  } // if
} // destructor

// ----------------------------------------------------------------------
// Open a new database
void
cencalvm::create::VMCreator::openDB(const char* filename,
				    const int cacheSize,
				    const char* description)
{ // openDB
  assert(0 != filename);
  assert(0 != description);

  if (!_quiet)
    std::cout << "Opening new etree database '" << filename << "'."
	      << std::endl;

  const int numDims = 3;
  const int payloadSize = sizeof(storage::PayloadStruct);
  _pDB = etree_open(filename, 
		    O_CREAT | O_TRUNC | O_RDWR,
		    cacheSize, payloadSize, numDims);
  if (0 == _pDB)
    throw std::runtime_error("Could not create etree database.");
  
  // Register schema
  if (0 != etree_registerschema(_pDB, storage::Payload::SCHEMA))
    throw std::runtime_error(etree_strerror(etree_errno(_pDB)));
 
  // Create and then register metadata 
  const int maxLen = 128;
  char hostname[maxLen];
  gethostname(hostname, maxLen);
  time_t rawTime = time(0);
  const char* datetime = ctime(&rawTime);
  std::ostringstream metainfo;
  metainfo
    << description << "\n"
    << "created on: " << datetime
    << "host: "  << hostname;
  if (0 != etree_setappmeta(_pDB, metainfo.str().c_str()))
    throw std::runtime_error(etree_strerror(etree_errno(_pDB)));

  _filename = filename;
  
  if (!_quiet)
    std::cout << "Finished preparing database." << std::endl;
} // openDB

// ----------------------------------------------------------------------
// Close the database
void
cencalvm::create::VMCreator::closeDB(void)
{ // openDB
  if (!_quiet)
    std::cout << "Closing database '" << _filename << "'." << std::endl;

  if (0 != _pDB) {
    etree_close(_pDB); _pDB = 0;
  } // if
} // closeDB

// ----------------------------------------------------------------------
// Create packed etree database from unpacked etree database
void
cencalvm::create::VMCreator::packDB(const char* filenamePacked,
				    const char* filenameUnpacked,
				    const int cacheSize) const
{ // packDB
  if (!_quiet)
    std::cout 
      << "Creating packed etree database '" << filenamePacked
      << "' from unpacked etree database '" << filenameUnpacked
      << "'." << std::endl;

  const int numDims = 3;
  const int payloadSize = sizeof(cencalvm::storage::PayloadStruct);
  etree_t* unpackeddb = etree_open(filenameUnpacked, O_RDONLY,
				   cacheSize, payloadSize, numDims);
  if (0 == unpackeddb)
    throw std::runtime_error("Could not open unpacked etree database.");
  
  etree_t* packeddb = etree_open(filenamePacked, O_CREAT | O_TRUNC | O_RDWR,
				 cacheSize, payloadSize, numDims);
  if (0 == packeddb)
    throw std::runtime_error("Could not open packed etree database.");
  
  if (0 != etree_registerschema(packeddb, cencalvm::storage::Payload::SCHEMA))
    throw std::runtime_error(etree_strerror(etree_errno(packeddb)));
  
  char* appmeta = 0;
  if (0 != (appmeta = etree_getappmeta(unpackeddb)))
    if (0 != etree_setappmeta(packeddb, appmeta))
      throw std::runtime_error(etree_strerror(etree_errno(packeddb)));
  free(appmeta);
  
  etree_addr_t addr;
  addr.x = 0;
  addr.y = 0;
  addr.z = 0;
  addr.level = 0;
  if (0 != etree_initcursor(unpackeddb, addr))
    throw std::runtime_error(etree_strerror(etree_errno(unpackeddb)));
  
  if (0 != etree_beginappend(packeddb, 1))
    throw std::runtime_error(etree_strerror(etree_errno(packeddb)));
  
  do {
    cencalvm::storage::PayloadStruct payload;
    
    if (0 != etree_getcursor(unpackeddb, &addr, "*", &payload))
      throw std::runtime_error(etree_strerror(etree_errno(unpackeddb)));
    
    if (0 != etree_append(packeddb, addr, &payload))
      throw std::runtime_error(etree_strerror(etree_errno(packeddb)));
  } while (0 == etree_advcursor(unpackeddb));
  
  if (0 != etree_endappend(packeddb))
    throw std::runtime_error(etree_strerror(etree_errno(packeddb)));
  
  if (0 != etree_close(unpackeddb))
    throw std::runtime_error(etree_strerror(etree_errno(unpackeddb)));
  
  if (0 != etree_close(packeddb))
    throw std::runtime_error(etree_strerror(etree_errno(packeddb)));
  
  if (!_quiet)
    std::cout << "Done packing etree database." << std::endl;
} // packDB
  
// ----------------------------------------------------------------------
// Insert data into database.
void
cencalvm::create::VMCreator::insert(const storage::PayloadStruct& payload,
				    const double lon,
				    const double lat,
				    const double elev,
				    const double resHoriz,
				    storage::Geometry* pGeom)
{ // insert
  assert(0 != pGeom);
  assert(0 != _pDB);

  etree_addr_t addr;
  addr.level = pGeom->level(resHoriz);
  addr.type = ETREE_LEAF;
  pGeom->lonLatElevToAddr(&addr, lon, lat, elev);

  if (0 != etree_insert(_pDB, addr, &payload))
    throw std::runtime_error(etree_strerror(etree_errno(_pDB)));
} // insert

// version
// $Id: VMCreator.cc 4189 2008-09-12 00:10:44Z brad $

// End of file 
