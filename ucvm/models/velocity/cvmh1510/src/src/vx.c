/** VX - A simple program to extract velocity values from
    a voxet. VX accepts Geographic Coordinates or UTM Zone 11 coordinates.
01/2011: PES: Minor formatting changes to output to make it consistent across all cases
06/2009: AP: higher precision for output coordinates, coor[] becomes double; replaced GTL in Salton T.
03/2009: AP: changed density scaling to Nafe-Drake
08/2008: AP: area enlarged to Terashake box, vs from models everywhere, no more tt file creation
02/2008: AP: does not ignore the uppermost CM layer anymore to define depths between 15 and 15.5km
11/2007: AP: vs CM voxet, vs from Brocher (2005) in LR and HR, included vs floor, made HR consistent with LR
9/2007: AP: elevation voxets, flag voxets, gcc4 compatible
6/2007: AP: modified to also output cell centers, improved output formatting, made consistent with gocad query
**/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "voxet.h"
#include "proj.h"
#include "cproj.h"
#include "params.h"
#include "coor_para.h"

void gctp ();

int GetLine(FILE *, char *);
static boolean fend (FILE *);
int GetVec(FILE *,char *,float *);
int GetDim(FILE *,char *,int *);
int GetPropName(FILE *, char *, int, char *);
static int GetPropSize(FILE *, char *, int, int *);
static int GetPropVal(FILE *, char *, int, float *);
int LoadVolume (char *, int, char *);
int IsBigendian();

static char *hrbuffer,*lrbuffer,*cmbuffer,*tobuffer,*mobuffer,*babuffer,*mtopbuffer;
static char *lrtbuffer,*cmtbuffer,*cmvsbuffer,*hrtbuffer,*lrvsbuffer,*hrvsbuffer;
char cbuffer;
char lbuffer[80];
char infile [80];
FILE *ifi, *ofi;
struct axis lr_a, mr_a, hr_a, cm_a, node, to_a, mo_a, ba_a;
struct flags f;
struct property p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13;

int voxbytepos(int *, int* ,int);

int main (int argc, char *argv[])
{
  int NCells,i,j;
//int u,v,w;
union zahl h,l,m;
float fl,z;
//double ic[2],oc[2];
double coor[3];
float gridcoor[3];
//float sina,cosa;
//float hyp,akat;
float step[3];
//float start[3];
//float dist;
int gcoor[3];
int Number,n;
double SP[2],SPUTM[2];
//char file1[20]="tt";
//char line[120];
char res[4]="nr\0";
char filename[CMLEN];

char MODEL_DIR[CMLEN] = ".";

char LR_PAR[CMLEN]="CVM_LR.vo";
FILE *LALR;

char HR_PAR[CMLEN]="CVM_HR.vo";
FILE *LAHR;

//char MR_PAR[CMLEN]="HUSCV.vo";
//FILE *LAMR;

char CM_PAR[CMLEN]="CVM_CM.vo";
FILE *LACM;

char TO_PAR[CMLEN]="interfaces.vo";
FILE *LATO;

/*
char MO_PAR[CMLEN]="moho.vo";
FILE *LAMO;

char BA_PAR[CMLEN]="base.vo";
FILE *LABA;
*/

if(argv[1])
if(!strcmp(argv[1], "-h"))
{
printf("     vx6.2 - Copyright M. Peter Suess 2006, A. Plesch 2009\n");
printf("Harvard University, University of Tuebingen\n");
printf("Extract velocities from a simple GOCAD voxet\n");
printf("     usage: vx < file.in\n");
printf("vx accepts geographic coordinates and \n");
printf("UTM Zone 11, NAD27, coordinates in X Y Z columns.\n");
printf("Output is:\n");
printf("X Y Z utmX utmY elevX elevY topo mtop base moho hr/lr/cm cellX cellY cellZ tg vp vs rho\n");
exit(0);
}

/**** First we load the LowRes File****/

sprintf(filename, "%s/%s", MODEL_DIR, LR_PAR);
LALR = fopen(filename, "r");
if (LALR == NULL) {
fprintf(stderr, "Failed to open model file %s\n", filename);
exit(1);
}
GetVec(LALR,"AXIS_O",lr_a.O);
GetVec(LALR,"AXIS_U",lr_a.U);
GetVec(LALR,"AXIS_V",lr_a.V);
GetVec(LALR,"AXIS_W",lr_a.W);
GetVec(LALR,"AXIS_MIN",lr_a.MIN);
GetVec(LALR,"AXIS_MAX",lr_a.MAX);
GetDim(LALR,"AXIS_N",lr_a.N);
//printf("bb %6.2f %6.2f %6.2f ",lr_a.U[0],lr_a.V[1],lr_a.W[2]);
//printf("aa %d %d %d ",lr_a.N[0],lr_a.N[1],lr_a.N[2]);

/** AP: AXIS_MIN and AXIS_MAX are currently not used and need to be 0
   0 0 and 1 1 1, respectively, in the .vo file. The AXIS_UVW would
   need to be adjusted accordingly in the .vo file.
**/

NCells=lr_a.N[0]*lr_a.N[1]*lr_a.N[2];

sprintf(p0.NAME,"vint");
GetPropName(LALR,"PROP_FILE",1,p0.FN);
GetPropSize(LALR,"PROP_ESIZE",1,&p0.ESIZE);
GetPropVal(LALR,"PROP_NO_DATA_VALUE",1,&p0.NO_DATA_VALUE);

//printf("lr esize: %d", p0.ESIZE);
//printf("filename lr: %s\n", p0.FN);

lrbuffer=(char *)malloc(NCells*p0.ESIZE);
sprintf(filename, "%s/%s", MODEL_DIR, p0.FN);
LoadVolume(filename,p0.ESIZE,lrbuffer);

// and the tags

 sprintf(p7.NAME,"tag");
 GetPropName(LALR,"PROP_FILE",2,p7.FN);
 GetPropSize(LALR,"PROP_ESIZE",2,&p7.ESIZE);
 GetPropVal(LALR,"PROP_NO_DATA_VALUE",2,&p7.NO_DATA_VALUE);

// printf("lrt name: %s\n", p7.FN);

 lrtbuffer=(char *)malloc(NCells*p7.ESIZE);
 sprintf(filename, "%s/%s", MODEL_DIR, p7.FN);
 LoadVolume(filename,p7.ESIZE,lrtbuffer);

// and vs

 sprintf(p11.NAME,"vs");
 GetPropName(LALR,"PROP_FILE",3,p11.FN);
 GetPropSize(LALR,"PROP_ESIZE",3,&p11.ESIZE);
 GetPropVal(LALR,"PROP_NO_DATA_VALUE",3,&p11.NO_DATA_VALUE);

//printf("lrt name: %s\n", p11.FN);

 lrvsbuffer=(char *)malloc(NCells*p11.ESIZE);
 sprintf(filename, "%s/%s", MODEL_DIR, p11.FN);
 LoadVolume(filename,p11.ESIZE,lrvsbuffer);

/**** Now we load the HighRes File *****/
sprintf(filename, "%s/%s", MODEL_DIR, HR_PAR);
LAHR = fopen(filename, "r");

GetVec(LAHR,"AXIS_O",hr_a.O);
GetVec(LAHR,"AXIS_U",hr_a.U);
GetVec(LAHR,"AXIS_V",hr_a.V);
GetVec(LAHR,"AXIS_W",hr_a.W);
GetVec(LAHR,"AXIS_MIN",hr_a.MIN);
GetVec(LAHR,"AXIS_MAX",hr_a.MAX);
GetDim(LAHR,"AXIS_N ",hr_a.N);
//printf("%d %d %d ",hr_a.N[0],hr_a.N[1],hr_a.N[2]);

NCells=hr_a.N[0]*hr_a.N[1]*hr_a.N[2];

sprintf(p1.NAME,"vint");
GetPropName(LAHR,"PROP_FILE",1,p2.FN);
GetPropSize(LAHR,"PROP_ESIZE",1,&p2.ESIZE);
GetPropVal(LAHR,"PROP_NO_DATA_VALUE",1,&p2.NO_DATA_VALUE);

//printf("hr esize: %d", p2.ESIZE);

hrbuffer=(char *)malloc(NCells*p2.ESIZE);
sprintf(filename, "%s/%s", MODEL_DIR, p2.FN);
LoadVolume(filename,p2.ESIZE,hrbuffer);

// and the tags

 sprintf(p9.NAME,"tag");
 GetPropName(LAHR,"PROP_FILE",2,p9.FN);
 GetPropSize(LAHR,"PROP_ESIZE",2,&p9.ESIZE);
 GetPropVal(LAHR,"PROP_NO_DATA_VALUE",2,&p9.NO_DATA_VALUE);

 //printf("hrt name: %s\n", p9.FN);

 hrtbuffer=(char *)malloc(NCells*p9.ESIZE);
 sprintf(filename, "%s/%s", MODEL_DIR, p9.FN);
 LoadVolume(filename,p9.ESIZE,hrtbuffer);

// and vs

 sprintf(p12.NAME,"vs");
 GetPropName(LAHR,"PROP_FILE",3,p12.FN);
 GetPropSize(LAHR,"PROP_ESIZE",3,&p12.ESIZE);
 GetPropVal(LAHR,"PROP_NO_DATA_VALUE",3,&p12.NO_DATA_VALUE);

// printf("hrt name: %s\n", p12.FN);

 hrvsbuffer=(char *)malloc(NCells*p12.ESIZE);
 sprintf(filename, "%s/%s", MODEL_DIR, p12.FN);
 LoadVolume(filename,p12.ESIZE,hrvsbuffer);
 
/**** Now we load the CrustMantle File *****/
sprintf(filename, "%s/%s", MODEL_DIR, CM_PAR);
LACM = fopen(filename, "r");

GetVec(LACM,"AXIS_O",cm_a.O);
GetVec(LACM,"AXIS_U",cm_a.U);
GetVec(LACM,"AXIS_V",cm_a.V);
GetVec(LACM,"AXIS_W",cm_a.W);
GetVec(LACM,"AXIS_MIN",cm_a.MIN);
GetVec(LACM,"AXIS_MAX",cm_a.MAX);
GetDim(LACM,"AXIS_N ",cm_a.N);
//printf("%d %d %d ",cm_a.N[0],cm_a.N[1],cm_a.N[2]);

NCells=cm_a.N[0]*cm_a.N[1]*cm_a.N[2];

sprintf(p3.NAME,"cvp");
GetPropName(LACM,"PROP_FILE",1,p3.FN);
GetPropSize(LACM,"PROP_ESIZE",1,&p3.ESIZE);
GetPropVal(LACM,"PROP_NO_DATA_VALUE",1,&p3.NO_DATA_VALUE);

//printf("cm esize: %d", p3.ESIZE);

cmbuffer=(char *)malloc(NCells*p3.ESIZE);
sprintf(filename, "%s/%s", MODEL_DIR, p3.FN);
LoadVolume (filename, p3.ESIZE, cmbuffer);

// and the flags

 sprintf(p8.NAME,"tag");
 GetPropName(LACM,"PROP_FILE",2,p8.FN);
 GetPropSize(LACM,"PROP_ESIZE",2,&p8.ESIZE);
 GetPropVal(LACM,"PROP_NO_DATA_VALUE",2,&p8.NO_DATA_VALUE);

 cmtbuffer=(char *)malloc(NCells*p8.ESIZE);
 sprintf(filename, "%s/%s", MODEL_DIR, p8.FN);
 LoadVolume (filename, p8.ESIZE, cmtbuffer);

// and vs

 sprintf(p10.NAME,"cvs");
 GetPropName(LACM,"PROP_FILE",3,p10.FN);
 GetPropSize(LACM,"PROP_ESIZE",3,&p10.ESIZE);
 GetPropVal(LACM,"PROP_NO_DATA_VALUE",3,&p10.NO_DATA_VALUE);

 cmvsbuffer=(char *)malloc(NCells*p10.ESIZE);
 sprintf(filename, "%s/%s", MODEL_DIR, p10.FN);
 LoadVolume (filename, p10.ESIZE, cmvsbuffer);

/**** Now we load the topo, moho, base, model top File *****/
sprintf(filename, "%s/%s", MODEL_DIR, TO_PAR);
LATO = fopen(filename, "r");

GetVec(LATO,"AXIS_O",to_a.O);
GetVec(LATO,"AXIS_U",to_a.U);
GetVec(LATO,"AXIS_V",to_a.V);
GetVec(LATO,"AXIS_W",to_a.W);
GetVec(LATO,"AXIS_MIN",to_a.MIN);
GetVec(LATO,"AXIS_MAX",to_a.MAX);
GetDim(LATO,"AXIS_N ",to_a.N);
//printf("%d %d %d ",to_a.N[0],to_a.N[1],to_a.N[2]);

NCells=to_a.N[0]*to_a.N[1]*to_a.N[2];

// topo

sprintf(p4.NAME,"topo_dem");
GetPropName(LATO,"PROP_FILE",1,p4.FN);
GetPropSize(LATO,"PROP_ESIZE",1,&p4.ESIZE);
GetPropVal(LATO,"PROP_NO_DATA_VALUE",1,&p4.NO_DATA_VALUE);

//printf("to esize: %d", p4.ESIZE);

tobuffer=(char *)malloc(NCells*p4.ESIZE);
sprintf(filename, "%s/%s", MODEL_DIR, p4.FN);
LoadVolume (filename, p4.ESIZE, tobuffer);

// moho

sprintf(p5.NAME,"moho");
GetPropName(LATO,"PROP_FILE",3,p5.FN);
GetPropSize(LATO,"PROP_ESIZE",3,&p5.ESIZE);
GetPropVal(LATO,"PROP_NO_DATA_VALUE",3,&p5.NO_DATA_VALUE);

//printf("mo esize: %d", p5.ESIZE);

mobuffer=(char *)malloc(NCells*p5.ESIZE);
sprintf(filename, "%s/%s", MODEL_DIR, p5.FN);
LoadVolume (filename, p5.ESIZE, mobuffer);

// basement

sprintf(p6.NAME,"base");
GetPropName(LATO,"PROP_FILE",2,p6.FN);
GetPropSize(LATO,"PROP_ESIZE",2,&p6.ESIZE);
GetPropVal(LATO,"PROP_NO_DATA_VALUE",2,&p6.NO_DATA_VALUE);

//printf("ba esize: %d", p6.ESIZE);

babuffer=(char *)malloc(NCells*p6.ESIZE);
sprintf(filename, "%s/%s", MODEL_DIR, p6.FN);
LoadVolume (filename, p6.ESIZE, babuffer);

// top elevation of model

sprintf(p13.NAME,"modeltop");
GetPropName(LATO,"PROP_FILE",4,p13.FN);
GetPropSize(LATO,"PROP_ESIZE",4,&p13.ESIZE);
GetPropVal(LATO,"PROP_NO_DATA_VALUE",4,&p13.NO_DATA_VALUE);

//printf("ba esize: %d", p6.ESIZE);

mtopbuffer=(char *)malloc(NCells*p13.ESIZE);
sprintf(filename, "%s/%s", MODEL_DIR, p13.FN);
LoadVolume (filename, p13.ESIZE, mtopbuffer);

/* now let's start with searching .... */

i=0;
while (fread(&cbuffer,sizeof(char),1,stdin)==1)
{lbuffer[i]=cbuffer;
i++;
if(cbuffer=='\n')
{sscanf(lbuffer,"%lf %lf %lf",&coor[0],&coor[1],&coor[2]);
if(coor[1]<10000000) printf("%14.6f %15.6f %9.2f ",coor[0],coor[1],coor[2]);

/* In case we got anything like degrees */

if ((coor[0]<360.)&&(fabs(coor[1])<90))
//if (coor[0]<360.)
{
Number=1;

for(n=0;n>15;n++) inparm[n]=0;

SP[0]=coor[0];
SP[1]=coor[1];

gctp(SP,&insys,&inzone,inparm,&inunit,&indatum,&ipr,efile,&jpr,efile,
		SPUTM,&outsys,&outzone,inparm,&outunit,&outdatum,
		file27, file83,&iflg);

coor[0]=SPUTM[0];
coor[1]=SPUTM[1];

}

/* Now we have UTM Zone 11 */

if (coor[1]<10000000) /*** Prevent all to obvious bad coordinates from being displayed */
{

  /* AP: Let's provide the computed UTM coordinates as well */
  printf("%10.2f %11.2f ", coor[0], coor[1]);

  // we start with the elevations; the voxet does not have a vertical dimension

  step[0]=to_a.U[0]/(to_a.N[0]-1);
  step[1]=to_a.V[1]/(to_a.N[1]-1);
  //step[2]=lr_a.W[2]/(lr_a.N[2]-1);

  gcoor[0]=round((coor[0]-to_a.O[0])/step[0]);
  gcoor[1]=round((coor[1]-to_a.O[1])/step[1]);
  gcoor[2]=0;
  //gcoor[2]=round((coor[2]-lr_a.O[2])/step[2]);
  //gcoor[1]=round((coor[1]-lr_a.O[1])/(lr_a.V[1]/(lr_a.N[1]-1)));
  //gcoor[2]=round((coor[2]-lr_a.O[2])/(lr_a.W[2]/(lr_a.N[2]-1)));

  //check if inside
  if(gcoor[0]>=0&&gcoor[1]>=0&&
     gcoor[0]<to_a.N[0]&&gcoor[1]<to_a.N[1])

    {
      gridcoor[0]= to_a.O[0]+gcoor[0]*step[0];
      gridcoor[1]= to_a.O[1]+gcoor[1]*step[1];
      j=voxbytepos(gcoor,to_a.N,p4.ESIZE);
      //printf("%d %d %d \n",j,gcoor[0],gcoor[1]);
      l.c[0]=tobuffer[j];
      l.c[1]=tobuffer[j+1];
      l.c[2]=tobuffer[j+2];
      l.c[3]=tobuffer[j+3];
      printf("%10.2f %11.2f ", gridcoor[0], gridcoor[1]);
      printf("%9.2f ", l.z);
      l.c[0]=mtopbuffer[j];
      l.c[1]=mtopbuffer[j+1];
      l.c[2]=mtopbuffer[j+2];
      l.c[3]=mtopbuffer[j+3];
      printf("%9.2f ", l.z);
      l.c[0]=babuffer[j];
      l.c[1]=babuffer[j+1];
      l.c[2]=babuffer[j+2];
      l.c[3]=babuffer[j+3];
      printf("%9.2f ", l.z);
      l.c[0]=mobuffer[j];
      l.c[1]=mobuffer[j+1];
      l.c[2]=mobuffer[j+2];
      l.c[3]=mobuffer[j+3];
      printf("%9.2f ", l.z);
    }
  else
      printf("%10.2f %11.2f %9.2f %9.2f %9.2f %9.2f ", p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE);

  /* AP: this calculates the cell numbers from the coordinates and the grid spacing. The
     -1 is necessary to do the counting correctly. The rounding is necessary because the
     data are cell centered, eg. they are valid half a cell width away from the data point */

  step[0]=lr_a.U[0]/(lr_a.N[0]-1);
  step[1]=lr_a.V[1]/(lr_a.N[1]-1);
  step[2]=lr_a.W[2]/(lr_a.N[2]-1);

  gcoor[0]=round((coor[0]-lr_a.O[0])/step[0]);
  gcoor[1]=round((coor[1]-lr_a.O[1])/step[1]);
  gcoor[2]=round((coor[2]-lr_a.O[2])/step[2]);
  //gcoor[1]=round((coor[1]-lr_a.O[1])/(lr_a.V[1]/(lr_a.N[1]-1)));
  //gcoor[2]=round((coor[2]-lr_a.O[2])/(lr_a.W[2]/(lr_a.N[2]-1)));

  if(gcoor[0]>=0&&gcoor[1]>=0&&gcoor[2]>=0&&
     gcoor[0]<lr_a.N[0]&&gcoor[1]<lr_a.N[1]&&gcoor[2]<lr_a.N[2])
    {
      //printf("diff: %f, ratio: %f", coor[0]-lr_a.O[0], lr_a.U[0]/(lr_a.N[0]-1));
      /* AP: And here are the cell centers*/
      //printf("lr %10.2f %11.2f %9.2f ", lr_a.O[0]+gcoor[0]*step[0], lr_a.O[1]+gcoor[1]*step[1], lr_a.O[2]+gcoor[2]*step[2]);
      gridcoor[0]= lr_a.O[0]+gcoor[0]*step[0];
      gridcoor[1]= lr_a.O[1]+gcoor[1]*step[1];
      gridcoor[2]= lr_a.O[2]+gcoor[2]*step[2];
      strcpy(res, "lr");
      j=voxbytepos(gcoor,lr_a.N,p0.ESIZE);
      //printf("%d",j);
      //vp
      l.c[0]=lrbuffer[j];
      l.c[1]=lrbuffer[j+1];
      l.c[2]=lrbuffer[j+2];
      l.c[3]=lrbuffer[j+3];
      //tag
      h.c[0]=lrtbuffer[j];
      h.c[1]=lrtbuffer[j+1];
      h.c[2]=lrtbuffer[j+2];
      h.c[3]=lrtbuffer[j+3];
      //vs
      m.c[0]=lrvsbuffer[j];
      m.c[1]=lrvsbuffer[j+1];
      m.c[2]=lrvsbuffer[j+2];
      m.c[3]=lrvsbuffer[j+3];

      step[0]=hr_a.U[0]/(hr_a.N[0]-1);
      step[1]=hr_a.V[1]/(hr_a.N[1]-1);
      step[2]=hr_a.W[2]/(hr_a.N[2]-1);

      gcoor[0]=round((coor[0]-hr_a.O[0])/step[0]);
      gcoor[1]=round((coor[1]-hr_a.O[1])/step[1]);
      gcoor[2]=round((coor[2]-hr_a.O[2])/step[2]);

      if(gcoor[0]>=0&&gcoor[1]>=0&&gcoor[2]>=0&&
	 gcoor[0]<hr_a.N[0]&&gcoor[1]<hr_a.N[1]&&gcoor[2]<hr_a.N[2])
	{
	  /* AP: And here are the cell centers*/
	  //printf("hr %10.2f %11.2f %9.2f ", hr_a.O[0]+gcoor[0]*step[0], hr_a.O[1]+gcoor[1]*step[1], hr_a.O[2]+gcoor[2]*step[2]);
	  //printf("hr %d %d %d ",gcoor[0], gcoor[1], gcoor[2]);
	  gridcoor[0]= hr_a.O[0]+gcoor[0]*step[0];
	  gridcoor[1]= hr_a.O[1]+gcoor[1]*step[1];
	  gridcoor[2]= hr_a.O[2]+gcoor[2]*step[2];
	  strcpy(res, "hr");
	  j=voxbytepos(gcoor,hr_a.N,p2.ESIZE);
	  //vp
	  l.c[0]=hrbuffer[j];
	  l.c[1]=hrbuffer[j+1];
	  l.c[2]=hrbuffer[j+2];
	  l.c[3]=hrbuffer[j+3];
	  //tag
	  h.c[0]=hrtbuffer[j];
	  h.c[1]=hrtbuffer[j+1];
	  h.c[2]=hrtbuffer[j+2];
	  h.c[3]=hrtbuffer[j+3];
	  //vs
	  m.c[0]=hrvsbuffer[j];
	  m.c[1]=hrvsbuffer[j+1];
	  m.c[2]=hrvsbuffer[j+2];
	  m.c[3]=hrvsbuffer[j+3];
	}

      printf("%s %10.2f %11.2f %9.2f ", res, gridcoor[0], gridcoor[1], gridcoor[2]);
      printf("%9.2f %9.2f %9.2f ", h.z, l.z, m.z);

/* vs is now done all in the grid
      if(l.z!=1480) //
	{
// Peters relation 
// A linear increase of vp/vs  up to 8500 m 
//
//	  if(coor[2]>-8500)
//	    printf("%9.2f ",l.z/(2-(0.27*coor[2]/-8500)));
//	  else
//	    printf("%9.2f ",l.z/1.73);
//
// Brocher, 2005 
	  if(l.z<4250)
// mudline
	    if(l.z>1500) printf("%9.2f ",(l.z - 1360)/1.16);
	    else printf("%9.2f ",(1500-1360)/1.16);
	  else
// Brocher regression fit
	    printf("%9.2f ",(785.8-1.2344*l.z+794.9*pow(l.z/1000,2)-123.8*pow(l.z/1000,3)+6.4*pow(l.z/1000,4)));
	}
      else
	printf("%9.2f ", p0.NO_DATA_VALUE ); // no vs in water
*/
      /*** Density should be at least 1000 ***/
      if(l.z!=1480)
	{
	  if(l.z>744.) {
	    //	    rho (g/cm^3) = 1.6612Vp - 0.4721Vp^2 + 0.0671Vp^3 - 0.0043Vp^4 + 0.000106Vp^5
	    fl = l.z/1000.0;
	  //	    printf("%9.2f\n", 1000*(fl*(1.6612 + fl*(-0.4721 + fl*(0.0671 + fl*(-0.0043 + fl*0.000106))))));
	    printf("%9.2f\n", 1000.0*(fl*(1.6612 + fl*(-0.4721 + fl*(0.0671 + fl*(-0.0043 + fl*0.000106))))));
	  } else
	    printf("%9.2f\n", 1000.0);
	}
      else
	printf("%9.2f\n", 1000.0);
      //printf("1000.00\n");
      goto alldone;
    }
  //**** lower crust and mantle voxet *****//
  step[0]=cm_a.U[0]/(cm_a.N[0]-1);
  step[1]=cm_a.V[1]/(cm_a.N[1]-1);
  step[2]=cm_a.W[2]/(cm_a.N[2]-1);

  gcoor[0]=round((coor[0]-cm_a.O[0])/step[0]);
  gcoor[1]=round((coor[1]-cm_a.O[1])/step[1]);
  gcoor[2]=round((coor[2]-cm_a.O[2])/step[2]);

  /** AP: check if inside CM voxet; the uppermost layer of CM overlaps 
      with the lowermost of LR, may need to be ignored but is not.
  **/
  if(gcoor[0]>=0&&gcoor[1]>=0&&gcoor[2]>=0&&
     gcoor[0]<cm_a.N[0]&&gcoor[1]<cm_a.N[1]&&gcoor[2]<(cm_a.N[2]))
    {
      gridcoor[0]= cm_a.O[0]+gcoor[0]*step[0];
      gridcoor[1]= cm_a.O[1]+gcoor[1]*step[1];
      gridcoor[2]= cm_a.O[2]+gcoor[2]*step[2];
      //strcpy(res, "cm");
      j=voxbytepos(gcoor,cm_a.N,p3.ESIZE);
      //vp
      l.c[0]=cmbuffer[j];
      l.c[1]=cmbuffer[j+1];
      l.c[2]=cmbuffer[j+2];
      l.c[3]=cmbuffer[j+3];
      //tag
      h.c[0]=cmtbuffer[j];
      h.c[1]=cmtbuffer[j+1];
      h.c[2]=cmtbuffer[j+2];
      h.c[3]=cmtbuffer[j+3];

      printf("%s %10.2f %11.2f %9.2f ", "cm", gridcoor[0], gridcoor[1], gridcoor[2]);
      printf("%9.2f %9.2f ", h.z, l.z);
      //vs
      h.c[0]=cmvsbuffer[j];
      h.c[1]=cmvsbuffer[j+1];
      h.c[2]=cmvsbuffer[j+2];
      h.c[3]=cmvsbuffer[j+3];

      //printf("%9.2f ",l.z/1.73);
      printf("%9.2f ",h.z);
      //rho
      //printf("%9.2f\n",l.z/3+1280);
      fl = l.z/1000;
      printf("%9.2f\n",1000*(fl*(1.6612 + fl*(-0.4721 + fl*(0.0671 + fl*(-0.0043 + fl*0.000106))))));
    }
  else
    /* AP: in case outside data area */
    printf("nr %10.2f %11.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n", p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE);
  z=lr_a.O[0]-lr_a.U[0];
  sprintf(&lbuffer[0],"\n");
  i=0;
}
/* AP: in case somewhere completely unreasonable */
else
printf("%10.2f %11.2f nr %10.2f %11.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n", p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE, p0.NO_DATA_VALUE);
alldone:
sprintf(&lbuffer[0],"\n");
i=0;

}
}    

}


/********************************************************/

int voxbytepos(int *ic,int *gs,int esize)

{
int pos;

pos=(gs[0]*gs[1]*(ic[2])+gs[0]*(ic[1])+ic[0])*esize;

return pos;
}

/*** Gets a line without knowing where it writes the info, so
be careful assinging enought space, else ... ***/

int GetLine(FILE *input, char *p)
{
int c;
while (c=fgetc(input), (c!='\n'))
*p++ = c;

if(c!=EOF) *p++ =c;
*p++ = '\0';
return c;
}

/** A very convient function to find the very end ... **/

static boolean fend (FILE *fp)
{
int ch = fgetc (fp);
if (ch == EOF)
return True;
else
{
ungetc (ch, fp);
return False;
}
}

int GetVec(FILE *fp,char *search,float *vec)
{
char LineBuffer[120];
//float tf[3];
//float test;
//int erg;

while (!fend(fp))
{
GetLine(fp,&LineBuffer[0]);
if(strstr(LineBuffer,search))
{
  //printf("%s \n",&LineBuffer[0]+strlen(search)+1);
sscanf(&LineBuffer[0]+strlen(search)+1,"%f %f %f",&vec[0],&vec[1],&vec[2]);
//printf("%f %f %f \n",vec[0],vec[1],vec[2]);
}

}
rewind(fp);
return 1;
}

int GetDim(FILE *fp,char *search,int *vec)
{
char LineBuffer[120];
//float tf[3];
//float test;
//int erg;

while (!fend(fp))
{
GetLine(fp,&LineBuffer[0]);
if(strstr(LineBuffer,search))
{
//printf("%s \n",&LineBuffer[0]+strlen(search));
sscanf(&LineBuffer[0]+strlen(search),"%d %d %d",&vec[0],&vec[1],&vec[2]);
//printf("%f %f %f",vec[0],vec[1],vec[2]);
}

}
rewind(fp);
return 1;
}

int GetPropName(FILE *fp,char *search, int PNumber, char *name)
{
char LineBuffer[120];
//float tf[3];
//float test;
int erg;

while (!fend(fp))
{
GetLine(fp,&LineBuffer[0]);
if(strstr(LineBuffer,search))
{
sscanf(&LineBuffer[0]+strlen(search)+1,"%d" ,&erg);
if(erg==PNumber)
sscanf(&LineBuffer[0]+strlen(search)+1,"%d %s" ,&erg,name);
}

}
rewind(fp);
return 1;
}

static int GetPropSize (FILE *fp,char *search, int PNumber, int *size)
{
char LineBuffer[120];
//float tf[3];
//float test;
int erg;

while (!fend(fp))
{
GetLine(fp,&LineBuffer[0]);
if(strstr(LineBuffer,search))
{
sscanf(&LineBuffer[0]+strlen(search)+1,"%d" ,&erg);
if(erg==PNumber)
{
sscanf(&LineBuffer[0]+strlen(search)+1,"%d %d" ,&erg,size);
}
}

}
rewind(fp);
return 1;
}

static int GetPropVal (FILE *fp,char *search, int PNumber, float *val)
{
char LineBuffer[120];
//float tf[3];
//float test;
int erg;

while (!fend(fp))
{
GetLine(fp,&LineBuffer[0]);
if(strstr(LineBuffer,search))
{
sscanf(&LineBuffer[0]+strlen(search)+1,"%d" ,&erg);
if(erg==PNumber)
{
sscanf(&LineBuffer[0]+strlen(search)+1,"%d %f" ,&erg,val);
}
}

}
rewind(fp);
return 1;
}


int LoadVolume (char *FN, int ESIZE, char *buffer)
{ 
FILE *ifi;
int i,j;
union zahl l,h;

ifi=fopen(FN,"r");
if (ifi == NULL) {
fprintf(stderr, "Failed to open model file %s\n", FN);
exit(1);
}
i=0;
j=0;
while(fread(h.c,ESIZE,1,ifi)==1)
{
l.c[3]=h.c[0];
l.c[2]=h.c[1];
l.c[1]=h.c[2];
l.c[0]=h.c[3];
if (IsBigendian()) {
l.c[0]=h.c[0];
l.c[1]=h.c[1];
l.c[2]=h.c[2];
l.c[3]=h.c[3];
}

for(i=0;i<4;i++){
buffer[j]=l.c[i];
j++;
}
}
fclose (ifi);
return(0);
}

/* Determine system endian */
int IsBigendian()
{
  int num = 1;
  if(*(char *)&num == 1) {
    return 0;
  } else {
    return 1;
  }
}

