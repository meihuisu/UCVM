#ifndef VX_COOR_PARAMS_H
#define VX_COOR_PARAMS_H


/**** Coordinate Trans parameters*/
double incoor[2];
double outcoor[2];

long insys = 0;
long inzone =0;
double inparm[15];
long inunit = 4;
long indatum= 0;


long ipr = 5;
//long jpr = 1;
/* suppress output of projection parameters */
long jpr = 5;

long outsys = 1;
long outzone= 11;
double outparm[15];
long outunit = 2;
long outdatum =  0;

long iflg;              /* error flag for inverse conversion of C version */
char efile[CMLEN]="errfile";      /* name of error file     */

char pfile[CMLEN]="pfile";
char file27[CMLEN]="proj27";
char file83[CMLEN]="file83";

#endif
