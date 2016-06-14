#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include "test_defs.h"
#include "accepttest_suite_grid.h"


int main (int argc, char *argv[])
{

  char *xmldir;

  if (argc == 2) {  
    xmldir = argv[1];
  } else {
    xmldir = NULL;
  }

  /* Run test suites */
  suite_grid(xmldir);

  return 0;
}
