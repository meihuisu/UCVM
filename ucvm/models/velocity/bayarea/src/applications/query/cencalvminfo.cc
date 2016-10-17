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

// Application to dump database metadata and schema to stdout

extern "C" {
#include "etree.h"
}

#include <iostream> // USES std::cout, std::cerr
#include <stdlib.h> // USES exit()
#include <assert.h> // USES assert()

// ----------------------------------------------------------------------
void
usage(void)
{ // usage
  std::cerr << "usage: cencalvminfo dbfile\n";
  exit(1);
} // usage

// ----------------------------------------------------------------------
void
parseArgs(std::string* pFilename,
	  int argc,
	  char** argv)
{ // parseArgs
  assert(0 != pFilename);

  if (argc != 2)
    usage();
  
  *pFilename = argv[1];
} // parseArgs

// ----------------------------------------------------------------------
int
main(int argc,
     char* argv[])
{ // main
  std::string filename = "";
  
  parseArgs(&filename, argc, argv);

  etree_t* db = etree_open(filename.c_str(), O_RDONLY, 0, 0, 0);
  if (0 == db) {
    std::cerr << "Could not open etree database '" << filename << "'.";
    return -1;
  } // if

  char* appmeta = etree_getappmeta(db);
  if (0 != appmeta) {
    std::cout << "Database metadata:\n" << appmeta << "\n\n";
    free(appmeta);
  } else {
    std::cerr << "Could not retrieve database metadata.";
    return -1;
  } // else

  char* schema = etree_getschema(db);
  if (0 != schema) {
    std::cout << "Database schema:\n" << schema << "\n\n";
    free(schema);
  } else {
    std::cerr << "Could not retrieve database schema.";
    return -1;
  } // else

  uint64_t size = etree_gettotalcount(db);
  std::cout << "Number of octants: " << size << std::endl;

  if (0 != etree_close(db)) {
    std::cerr << etree_strerror(etree_errno(db));
    return -1;
  } // if

  return 0;
} // main



// version
// $Id: cencalvminfo.cc 1988 2005-08-11 03:05:32Z brad $

// End of file 
