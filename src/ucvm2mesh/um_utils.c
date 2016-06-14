#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "um_utils.h"


/* Little endian or big endian system */
int isLittleEndian()
{
  int num = 1;
  if(*(char *)&num == 1) {
    printf("Little-Endian system detected\n");
    return(1);
  } else {
    printf("Big-Endian system detected\n");
    return(0);
  }
}


/* Check if file exists */
int fileExists(const char *file)
{
  struct stat st;

  if (stat(file, &st) == 0) {
    return(1);
  } else {
    return(0);
  }
}


/* Delete file */
int deleteFile(const char *file)
{
  if (fileExists(file)) {
    unlink(file);
  }

  return(0);
}


/* Copy file */
int copyFile(const char *src, const char *dest)
{
  char cmd[512];
  int retval;

  /* Perform copy */
  sprintf(cmd, "cp -r %s %s", src, dest);
  retval = system(cmd);
  if (WEXITSTATUS(retval) != 0) {
    return(1);
  }

  return(0);
}

