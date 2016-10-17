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

// Application driver to generate etree database of central CA
// velocity model.

#include "cencalvm/create/GridIngester.h" // USES GridIngester
#include "cencalvm/storage/Geometry.h" // USES Geometry
#include "cencalvm/storage/GeomCenCA.h" // USES GeomCenCA

#include <stdlib.h> // USES exit()
#include <unistd.h> // USES getopt()
#include <stdio.h> // USES EOF

#include <iostream> // USES std::cerr
#include <stdexcept> // USES std::exception
#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
void
usage(void)
{ // usage
  std::cerr
    << "usage: gencencalvm [-h] -i paramFile -o outFile -t tmpFile [-l logFile]\n"
    << "  -i paramFile  Parameter file with list of grid input files\n"
    << "  -o outFile    Etree database file created.\n"
    << "  -t tmpFile    Name of scratch file used in database construction.\n"
    << "  -h            Display usage and exit.\n"
    << "  -l logFile    Log file for warnings about data.\n"
    << "\n"
    << "Parameter file is list of grid input files, one per line.\n";
  exit(1);
} // usage

// ----------------------------------------------------------------------
void
parseArgs(std::string* pFilenameParams,
	  std::string* pFilenameOut,
	  std::string* pFilenameTmp,
	  int* pCacheSize,
	  int argc,
	  char** argv)
{ // parseArgs
  assert(0 != pFilenameParams);
  assert(0 != pFilenameOut);
  assert(0 != pFilenameTmp);
  assert(0 != pCacheSize);

  extern char* optarg;

  int nparsed = 1;
  *pFilenameParams = "";
  *pFilenameOut = "";
  *pFilenameTmp = "";
  int c = EOF;
  while ( (c = getopt(argc, argv, "c:hi:o:t:") ) != EOF) {
    switch (c)
      { // switch
      case 'c' : // process -c option
	*pCacheSize = atoi(optarg);
	nparsed += 2;
	break;
      case 'h' : // process -h option
	nparsed += 1;
	usage();
	exit(0);
	break;
      case 'i' : // process -i option
	*pFilenameParams = optarg;
	nparsed += 2;
	break;
      case 'o' : // process -o option
	*pFilenameOut = optarg;
	nparsed += 2;
	break;
      case 't' : // process -t option
	*pFilenameTmp = optarg;
	nparsed += 2;
	break;
      default :
	usage();
      } // switch
    } // while
  if (nparsed != argc || 
      0 == pFilenameParams->length() ||
      0 == pFilenameOut->length() ||
      0 == pFilenameTmp->length())
    usage();
} // parseArgs

// ----------------------------------------------------------------------
int
main(int argc,
     char* argv[])
{ // main
  std::string filenameParams = "";
  std::string filenameOut = "";
  std::string filenameTmp = "";
  std::string filenameLog = "";
  int cacheSize = 512;
  const char* description = 
    "U.S. Geological Survey\n"
    "Thomas Brocher, Robert Jachens, Carl Wentworth, Russel Graymer, "
    "Robert Simpson, Brad Aagaard";
  
  parseArgs(&filenameParams, &filenameOut, &filenameTmp, &cacheSize,
	    argc, argv);

  try {
    cencalvm::create::GridIngester db;
    
    cencalvm::storage::GeomCenCA geom;

    db.geometry(&geom);
    db.filenameParams(filenameParams.c_str());
    db.filenameOut(filenameOut.c_str());
    db.filenameTmp(filenameTmp.c_str());
    db.cacheSize(cacheSize);
    db.description(description);
    db.run();
  } catch (const std::exception& err) {
    std::cerr << err.what();
    return 1;
  } catch (...) {
    return 1;
  } // catch

  return 0;
} // main

// version
// $Id: gencencalvm.cc 5020 2010-01-19 23:55:29Z brad $

// End of file 
