#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "test_defs.h"
#include "unittest_suite_lib.h"


int main (int argc, char *argv[])
{
  char *xmldir;

  if (argc == 2) {  
    xmldir = argv[1];
  } else {
    xmldir = NULL;
  }

  /* Run test suites */
  suite_lib(xmldir);

  return 0;
}
