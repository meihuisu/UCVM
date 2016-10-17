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

// Application driver to fill in octants of central CA velocity model
// to create spatially averaged model.

#include "cencalvm/average/Averager.h" // USES VMCreator

#include <stdlib.h> // USES exit()
#include <unistd.h> // USES getopt()
#include <stdio.h> // USES EOF
#include <assert.h> // USES assert()

#include <iostream> // USES std::cerr
#include <stdexcept> // USES std::exception

// ----------------------------------------------------------------------
void
usage(void)
{ // usage
  std::cerr
    << "usage: avgcencalvm [-h] -i inFile -o outFile\n"
    << "  -i inFile     Etree database to average.\n"
    << "  -o outFile    Averaged Etree database.\n"
    << "  -h            Display usage and exit.\n"
    << "\n";
  exit(1);
} // usage

// ----------------------------------------------------------------------
void
parseArgs(std::string* pFilenameIn,
	  std::string* pFilenameOut,
	  int argc,
	  char** argv)
{ // parseArgs
  assert(0 != pFilenameIn);
  assert(0 != pFilenameOut);

  extern char* optarg;

  int nparsed = 1;
  *pFilenameIn = "";
  *pFilenameOut = "";
  int c = EOF;
  while ( (c = getopt(argc, argv, "hi:o:") ) != EOF) {
    switch (c)
      { // switch
	case 'i' : // process -i option
	  *pFilenameIn = optarg;
	  nparsed += 2;
	  break;
	case 'o' : // process -o option
	  *pFilenameOut = optarg;
	  nparsed += 2;
	  break;
	case 'h' : // process -h option
	  nparsed += 1;
	  usage();
	  exit(0);
	  break;
	default :
	  usage();
	} // switch
    } // while
  if (nparsed != argc || 
      0 == pFilenameIn->length() ||
      0 == pFilenameOut->length())
    usage();
} // parseArgs

// ----------------------------------------------------------------------
int
main(int argc,
     char* argv[])
{ // main
  std::string filenameIn = "";
  std::string filenameOut = "";
  
  parseArgs(&filenameIn, &filenameOut, argc, argv);

  try {
    cencalvm::average::Averager averager;

    averager.filenameIn(filenameIn.c_str());
    averager.filenameOut(filenameOut.c_str());
    averager.average();
  } catch (const std::exception& err) {
    std::cerr << err.what();
    return 1;
  } catch (...) {
    return 1;
  } // catch

  return 0;
} // main

// version
// $Id: avgcencalvm.cc 5020 2010-01-19 23:55:29Z brad $

// End of file 
