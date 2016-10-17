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

// Application to extract depth (in meters) of specified vs (in m/s)
// at list of points.

#include "cencalvm/query/VMQuery.h" // USES VMQuery
#include "cencalvm/storage/ErrorHandler.h" // USES ErrorHandler

#include <stdlib.h> // USES exit()
#include <stdio.h> // USES EOF
#include <unistd.h> // USES getopt()

#include <fstream> // USES std::ifstream, std::ofstream
#include <iomanip> // USES setw(), setiosflags(), resetiosflags()

#include <iostream> // USES std::cerr
#include <sstream> // USES std::ostringstream
#include <cassert> // USES assert()
#include <stdexcept> // USES std::runtime_error()

#include <string> // USES std::string
#include <cmath> // USES log(), ceil()

// ----------------------------------------------------------------------
// Dump usage to stderr.
void
usage(void)
{ // usage
  std::cerr
    << "usage: cencalvmisosurface [-h] -v vs -i fileIn -o fileOut -d dbfile\n"
    << "       [-e dbextfile] [-c cacheSize] [-s squashLimit] [-z elevMin]\n"
    << "\n"
    << "  -v vs         Value of shear wave speed for isosurface\n"
    << "  -s squashLim  Turn on squashing of topography and set limit\n"
    << "  -i fileIn     File containing list of locations: 'lon lat'.\n"
    << "  -o fileOut    Output file with locations and elev of isosurface.\n"
    << "  -d dbfile     Etree database file to query.\n"
    << "  -e dbextfile  Etree extended database file to query.\n"
    << "  -c cacheSize  Size of cache in MB to use in query\n"
    << "  -s squashLim  Turn on squashing of topography and set limit\n"
    << "  -z elevMin    Minimum elevation to consider for isosurface\n"
    << "  -h            Display usage and exit.\n";
} // usage

// ----------------------------------------------------------------------
// Parse command line arguments.
void
parseArgs(double* vs,
	  std::string* filenameIn,
	  std::string* filenameOut,
	  std::string* filenameDB,
	  std::string* filenameDBExt,
	  int* cacheSize,
	  double* squashLimit,
	  double* elevMin,
	  int argc,
	  char** argv)
{ // parseArgs
  assert(0 != vs);
  assert(0 != filenameIn);
  assert(0 != filenameOut);
  assert(0 != filenameDB);
  assert(0 != filenameDBExt);
  assert(0 != cacheSize);
  assert(0 != squashLimit);
  assert(0 != elevMin);

  extern char* optarg;

  int nparsed = 1;
  *vs = 0.0;
  *filenameIn = "";
  *filenameOut = "";
  *filenameDB = "";
  *filenameDBExt = "";
  int c = EOF;
  while ( (c = getopt(argc, argv, "c:d:e:hi:o:s:v:z:") ) != EOF) {
    switch (c)
      { // switch
      case 'c' : // process -c option
	*cacheSize = atoi(optarg);
	nparsed += 2;
	break;
      case 'd' : // process -d option
	*filenameDB = optarg;
	nparsed += 2;
	break;
      case 'e' : // process -e option
	*filenameDBExt = optarg;
	nparsed += 2;
	break;
      case 'h' : // process -h option
	nparsed += 1;
	usage();
	exit(0);
	break;
      case 'i' : // process -i option
	*filenameIn = optarg;
	nparsed += 2;
	break;
      case 'o' : // process -o option
	*filenameOut = optarg;
	nparsed += 2;
	break;
      case 's': // process -s option
	*squashLimit = atof(optarg);
	nparsed += 2;
	break;
      case 'v': // process -v option
	*vs = atof(optarg);
	nparsed += 2;
	break;
      case 'z': // process -z option
	*elevMin = atof(optarg);
	nparsed += 2;
	break;
      default :
	usage();
	exit(1);
      } // switch
  } // while
  if (nparsed != argc || 
      0 == *vs ||
      0 == filenameIn->length() ||
      0 == filenameOut->length() ||
      0 == filenameDB->length()) {
    usage();
    exit(1);
  } // if
  
} // parseArgs

// ----------------------------------------------------------------------
int
initialize(cencalvm::query::VMQuery* query,
	   const char* filenameDB,
	   const char* filenameDBExt,
	   const int cacheSize,
	   const double squashLimit,
	   const bool squashOn)
{ // initialize
  assert(0 != query);

  // Get handle to error handler
  cencalvm::storage::ErrorHandler* errHandler = query->errorHandler();
  if (0 == errHandler) {
    std::cerr << "Could not get handle to error handler.\n";
    return 1;
  } // if
    
  // Set database filename
  query->filename(filenameDB);
  if (cencalvm::storage::ErrorHandler::OK != errHandler->status()) {
    std::cerr << errHandler->message();
    return 1;
  } // if

  // Set cache size
  query->cacheSize(cacheSize);
  if (cencalvm::storage::ErrorHandler::OK != errHandler->status()) {
    std::cerr << errHandler->message();
    return 1;
  } // if

  // Set extended database filename if given
  if ("" != filenameDBExt) {
    query->filenameExt(filenameDBExt);
    if (cencalvm::storage::ErrorHandler::OK != errHandler->status()) {
      std::cerr << errHandler->message();
      return 1;
    } // if
    // Set cache size
    query->cacheSizeExt(cacheSize);
    if (cencalvm::storage::ErrorHandler::OK != errHandler->status()) {
      std::cerr << errHandler->message();
      return 1;
    } // if
  } // if

  // Turn on squashing if requested
  if (squashOn) {
    query->squash(true, squashLimit);
    if (cencalvm::storage::ErrorHandler::OK != errHandler->status()) {
      std::cerr << errHandler->message();
      return 1;
    } // if
  } // if    

  // Open database for querying
  query->open();
  if (cencalvm::storage::ErrorHandler::OK != errHandler->status()) {
    std::cerr << errHandler->message();
    return 1;
  } // if

  // Set query type to maximum resolution
  query->queryType(cencalvm::query::VMQuery::MAXRES);

  return 0;
} // initialize

// ----------------------------------------------------------------------
double
queryElev(cencalvm::query::VMQuery* query,
	  const double lon,
	  const double lat)
{ // queryElev
  assert(0 != query);

  // Get handle to error handler
  cencalvm::storage::ErrorHandler* errHandler = query->errorHandler();
  if (0 == errHandler) {
    std::cerr << "Could not get handle to error handler.\n";
    return 1;
  } // if
    
  const int numVals = 1;
  const char* elevName[] = { "Elevation" };
  query->queryVals(elevName, numVals);
  if (cencalvm::storage::ErrorHandler::OK != errHandler->status()) {
    std::cerr << errHandler->message();
    throw std::runtime_error("Could not get elevation.");
  } // if

  double* vals = (numVals > 0) ? new double[numVals] : 0;
  assert(0 != vals);

  // Query for elevation at location
  query->query(&vals, numVals, lon, lat, -5.0e+3);
  double elev = vals[0];
  
  // Correct elevation for stair-stepping grid
  const char* valNames[] = { "Vs" };
  query->queryVals(valNames, numVals);

  const int niters = 10;
  const double dx = -12.5;
  for (int iter=0; iter < niters; ++iter) {
    const double elevQ = elev + iter*dx;
    query->query(&vals, numVals, lon, lat, elevQ);
    const double vs = vals[0];
    if (vs > 0.0) {
      elev = elevQ;
      break;
    } // if
  } // for

  delete[] vals; vals = 0;

  return elev;
} // queryElev

// ----------------------------------------------------------------------
double
searchVs(cencalvm::query::VMQuery* query,
	 const double vsTarget,
	 const double lon,
	 const double lat,
	 const double elevLimitUpper,
	 const double elevLimitLower)
{ // searchVs
  assert(0 != query);

  const int niters = 
    int(1 + ceil(log( (elevLimitUpper - elevLimitLower) / 25.0) / log(2.0)));

  const int numVals = 1;
  double* vals = (numVals > 0) ? new double[numVals] : 0;
  assert(0 != vals);

  double elevUpper = elevLimitUpper;
  double elevLower = elevLimitLower;
  double elevVs = elevLower;

  // Set values to be returned in queries
  const char* valNames[] = { "Vs" };
  query->queryVals(valNames, numVals);

  for (int iter=0; iter < niters; ++iter) {
    query->query(&vals, numVals, lon, lat, elevUpper);
    const double vsU = vals[0];

    query->query(&vals, numVals, lon, lat, elevLower);
    const double vsL = vals[0];

    const double elevMiddle = 0.5 * (elevUpper + elevLower);
    query->query(&vals, numVals, lon, lat, elevMiddle);
    const double vsM = vals[0];
  
    if (vsTarget > vsU && vsTarget <= vsM) {
      elevLower = elevMiddle;
      elevVs = elevLower;
    } else if (vsTarget > vsM && vsTarget <= vsL) {
      elevUpper = elevMiddle;
      elevVs = elevLower;
    } else if (vsTarget <= vsU) {
      elevVs = elevUpper;
      break;
    } else if (vsL < 0.0) {
      elevVs = 9999.0;
      break;
    } else {
      std::cerr
	<< "Invalid search range!\n"
	<< "Target vs: " << vsTarget << "\n"
	<< "Location: (" << lon << ", " << lat << ")\n"
	<< "Upper Vs(z=" << elevUpper << "): " << vsU << "\n"
	<< "Lower Vs(z=" << elevLower << "): " << vsL << "\n"
	<< "Middle Vs(z=" << elevMiddle << "): " << vsM
	<< std::endl;
      exit(1);
    } // else
  } // for
  
  return elevVs;
} // searchVs
	  

// ----------------------------------------------------------------------
// main
int
main(int argc,
     char* argv[])
{ // main
  double vsTarget = 0.0;
  std::string filenameIn = "";
  std::string filenameOut = "";
  std::string filenameDB = "";
  std::string filenameDBExt = "";
  double queryRes = 0.0;
  int cacheSize = 128;
  const double squashDefault = 1.0e+06;
  double squashLimit = squashDefault;
  double elevMin = -45.0e+03;
  
  // Parse command line arguments
  parseArgs(&vsTarget, &filenameIn, &filenameOut, &filenameDB, &filenameDBExt,
	    &cacheSize, &squashLimit, &elevMin,
	    argc, argv);

  // Create query
  cencalvm::query::VMQuery query;

  const bool squashOn = squashLimit != squashDefault;
  initialize(&query, filenameDB.c_str(), filenameDBExt.c_str(), 
	     cacheSize, squashLimit, squashOn);

  // Open input file to read locations
  std::ifstream fileIn(filenameIn.c_str());
  if (!fileIn.is_open()) {
    std::cerr << "Could not open file '" << filenameIn
	      << "' to read query locations.\n";
    return 1;
  } // if
  
  // Open output file to accept data
  std::ofstream fileOut(filenameOut.c_str());
  if (!fileOut.is_open()) {
    std::cerr << "Could not open file '" << filenameOut
	      << "' to receive query data.\n";
    return 1;
  } // if

  const int numVals = 1; // Vs
  // Create array to hold values returned in queries
  double* vals = (numVals > 0) ? new double[numVals] : 0;

  // Read location from input file
  double lon = 0.0;
  double lat = 0.0;
  fileIn >> lon >> lat;

  // Get handle to error handler
  cencalvm::storage::ErrorHandler* errHandler = query.errorHandler();
  if (0 == errHandler) {
    std::cerr << "Could not get handle to error handler.\n";
    return 1;
  } // if
    
  // Continue operating on locations until end of file, reading fails,
  // or writing fails
  while (!fileIn.eof() && fileIn.good() && fileOut.good()) {

    const double elevTopo = queryElev(&query, lon, lat);
    double elevUpper = (squashOn) ? 0.0 : elevTopo;
    double elevLower = (squashOn) ? -elevTopo+elevMin : elevMin;
    const double elevVs = searchVs(&query, vsTarget, 
				   lon, lat, elevUpper, elevLower);

    // If query generated a warning or error, dump message to std::cerr
    if (cencalvm::storage::ErrorHandler::OK != errHandler->status()) {
      std::cerr << errHandler->message();
      // If query generated an error, then bail out, otherwise reset status
      if (cencalvm::storage::ErrorHandler::ERROR == errHandler->status())
	return 1;
      errHandler->resetStatus();
    } // if

    const double distIsosurf = (elevVs != 9999.0) ? elevTopo - elevVs : -9999.0;

    // Write values returned by query to output file
    fileOut
      << std::resetiosflags(std::ios::scientific)
      << std::setiosflags(std::ios::fixed)
      << std::setprecision(5)
      << std::setw(10) << lon
      << std::setw(9) << lat
      << std::setprecision(1) << std::setw(9) << distIsosurf
      << "\n";
    
    // Read in next location from input file
    fileIn >> lon >> lat;
  } // while
  
  // Close database
  query.close();

  // Close input and output files
  fileIn.close();
  fileOut.close();

  // If an error was generated, write error message and bail out
  if (cencalvm::storage::ErrorHandler::OK != errHandler->status()) {
    std::cerr << errHandler->message();
    return 1;
  } // if

  return 0;
} // main


// End of file 
