/** vx_sub.c - Query interface to GoCAD voxet volumes and GTL. Supports
    queries for material properties and topography. Accepts Geographic Coordinates 
    or UTM Zone 11 coordinates.

01/2010: PES: Derived from original VX interface, vx.c. 
              Added Vs30 Derived GTL, 1D background, smoothing
07/2011: PES: Extracted io into separate module from vx_sub.c
**/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "params.h"
#include "coor_para.h"
#include "voxet.h"
#include "proj.h"
#include "cproj.h"
#include "scec1d.h"
#include "vs30_gtl.h"
#include "utils.h"
#include "vx_io.h"
#include "vx_sub.h"

/* Smoothing parameters for SCEC 1D */
#define SCEC_SMOOTH_DIST 50.0 // km

/* Topography filtering DEPRECATED */
#define ELEV_EPSILON 0.01
#define MAX_ITER_ELEV 4


/* Function declarations */
void gctp();
int voxbytepos(int *, int* ,int);
double calc_rho(float vp, vx_src_t data_src);

/* User-defined background model function pointer */
int (*callback_bkg)(vx_entry_t *entry, vx_request_t req_type) = NULL;

/* Model state variables */
static int is_setup = False;
vx_zmode_t vx_zmode = VX_ZMODE_ELEV;
int vx_use_gtl = True;
struct axis lr_a, mr_a, hr_a, cm_a, to_a;
struct property p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13;
float step_to[3], step_lr[3], step_hr[3], step_cm[3];

/* Model buffers */
static char *hrbuffer = NULL;
static char *lrbuffer = NULL;
static char *cmbuffer = NULL;
static char *tobuffer = NULL;
static char *mobuffer = NULL;
static char *babuffer = NULL;
static char *mtopbuffer = NULL;
static char *lrtbuffer = NULL;
static char *cmtbuffer = NULL;
static char *cmvsbuffer = NULL;
static char *hrtbuffer = NULL;
static char *lrvsbuffer = NULL;
static char *hrvsbuffer = NULL;

/* Data source labels */
char *VX_SRC_NAMES[7] = {"nr", "hr", "lr", "cm", "to", "bk", "gt"};


/* Setup function to be called prior to querying points */
int vx_setup(const char *data_dir)
{
  int NCells;
  int n;
  char gtlpath[CMLEN];

  /* zero-out inparm for gctpc */
  for(n=0;n>15;n++) inparm[n]=0;

  /* Initialize buffer pointers to NULL */
  hrbuffer = lrbuffer = cmbuffer = NULL;
  tobuffer = mobuffer = babuffer = mtopbuffer = NULL;
  lrtbuffer = cmtbuffer = hrtbuffer = NULL;
  cmvsbuffer = lrvsbuffer = hrvsbuffer = NULL;

  sprintf(gtlpath, "%s/%s", data_dir, DEFAULT_GTL_FILE);

  char LR_PAR[CMLEN];
  sprintf(LR_PAR, "%s/CVM_LR.vo", data_dir);
  
  char HR_PAR[CMLEN];
  sprintf(HR_PAR, "%s/CVM_HR.vo", data_dir);
  
  char CM_PAR[CMLEN];
  sprintf(CM_PAR, "%s/CVM_CM.vo", data_dir);

  char TO_PAR[CMLEN];
  sprintf(TO_PAR, "%s/interfaces.vo", data_dir);


  /**** First we load the LowRes File****/
  if (vx_io_init(LR_PAR) != 0) {
    fprintf(stderr, "Failed to load LR param file %s. Check that the model path is correct.\n", LR_PAR);
    return(1);
  }
  vx_io_getvec("AXIS_O",lr_a.O);
  vx_io_getvec("AXIS_U",lr_a.U);
  vx_io_getvec("AXIS_V",lr_a.V);
  vx_io_getvec("AXIS_W",lr_a.W);
  vx_io_getvec("AXIS_MIN",lr_a.MIN);
  vx_io_getvec("AXIS_MAX",lr_a.MAX);
  vx_io_getdim("AXIS_N",lr_a.N);

  /** AP: AXIS_MIN and AXIS_MAX are currently not used and need to be 0
      0 0 and 1 1 1, respectively, in the .vo file. The AXIS_UVW would
      need to be adjusted accordingly in the .vo file.
  **/

  NCells=lr_a.N[0]*lr_a.N[1]*lr_a.N[2];
  sprintf(p0.NAME,"vint");
  vx_io_getpropname("PROP_FILE",1,p0.FN);
  vx_io_getpropsize("PROP_ESIZE",1,&p0.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",1,&p0.NO_DATA_VALUE);

  lrbuffer=(char *)malloc(NCells*p0.ESIZE);
  if (lrbuffer == NULL) {
    fprintf(stderr, "Failed to allocate LR Vp buffer\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p0.FN,
		       p0.ESIZE,NCells,lrbuffer) != 0) {
    fprintf(stderr, "Failed to load LR Vp volume\n");
    return(1);
  }

  // and the tags
  sprintf(p7.NAME,"tag");
  vx_io_getpropname("PROP_FILE",2,p7.FN);
  vx_io_getpropsize("PROP_ESIZE",2,&p7.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",2,&p7.NO_DATA_VALUE);

  lrtbuffer=(char *)malloc(NCells*p7.ESIZE);
  if (lrtbuffer == NULL) {
    fprintf(stderr, "Failed to allocate LR tag buffer\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p7.FN,
		       p7.ESIZE,NCells,lrtbuffer) != 0) {
    fprintf(stderr, "Failed to load LR tag volume\n");
    return(1);
  }

  // and vs
  sprintf(p11.NAME,"vs");
  vx_io_getpropname("PROP_FILE",3,p11.FN);
  vx_io_getpropsize("PROP_ESIZE",3,&p11.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",3,&p11.NO_DATA_VALUE);

  lrvsbuffer=(char *)malloc(NCells*p11.ESIZE);
  if (lrvsbuffer == NULL) {
    fprintf(stderr, "Failed to allocate LR Vs buffer\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p11.FN,
		       p11.ESIZE,NCells,lrvsbuffer) != 0) {
    fprintf(stderr, "Failed to load LR Vs volume\n");
    return(1);
  }
  vx_io_finalize();

  /**** Now we load the HighRes File *****/
  if (vx_io_init(HR_PAR) != 0) {
    fprintf(stderr, "Failed to load HR param file %s\n", HR_PAR);
    return(1);
  }

  vx_io_getvec("AXIS_O",hr_a.O);
  vx_io_getvec("AXIS_U",hr_a.U);
  vx_io_getvec("AXIS_V",hr_a.V);
  vx_io_getvec("AXIS_W",hr_a.W);
  vx_io_getvec("AXIS_MIN",hr_a.MIN);
  vx_io_getvec("AXIS_MAX",hr_a.MAX);
  vx_io_getdim("AXIS_N ",hr_a.N);

  NCells=hr_a.N[0]*hr_a.N[1]*hr_a.N[2];

  sprintf(p1.NAME,"vint");
  vx_io_getpropname("PROP_FILE",1,p2.FN);
  vx_io_getpropsize("PROP_ESIZE",1,&p2.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",1,&p2.NO_DATA_VALUE);

  hrbuffer=(char *)malloc(NCells*p2.ESIZE);
  if (hrbuffer == NULL) {
    fprintf(stderr, "Failed to allocate HR Vp file\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p2.FN,
		       p2.ESIZE,NCells,hrbuffer) != 0) {
    fprintf(stderr, "Failed to load HR Vp volume\n");
    return(1);
  }

  // and the tags
  sprintf(p9.NAME,"tag");
  vx_io_getpropname("PROP_FILE",2,p9.FN);
  vx_io_getpropsize("PROP_ESIZE",2,&p9.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",2,&p9.NO_DATA_VALUE);

  hrtbuffer=(char *)malloc(NCells*p9.ESIZE);
  if (hrtbuffer == NULL) {
    fprintf(stderr, "Failed to allocate HR tag file\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p9.FN,
		       p9.ESIZE,NCells,hrtbuffer) != 0) {
    fprintf(stderr, "Failed to load HR tag volume\n");
    return(1);
  }

  // and vs
  sprintf(p12.NAME,"vs");
  vx_io_getpropname("PROP_FILE",3,p12.FN);
  vx_io_getpropsize("PROP_ESIZE",3,&p12.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",3,&p12.NO_DATA_VALUE);

  hrvsbuffer=(char *)malloc(NCells*p12.ESIZE);
  if (hrvsbuffer == NULL) {
    fprintf(stderr, "Failed to allocate HR Vs file\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p12.FN,
		       p12.ESIZE,NCells,hrvsbuffer) != 0) {
    fprintf(stderr, "Failed to load HR Vs volume\n");
    return(1);
  }

  vx_io_finalize();

  /**** Now we load the CrustMantle File *****/
  if (vx_io_init(CM_PAR) != 0) {
    fprintf(stderr, "Failed to load CM param file %s\n", CM_PAR);
    return(1);
  }

  vx_io_getvec("AXIS_O",cm_a.O);
  vx_io_getvec("AXIS_U",cm_a.U);
  vx_io_getvec("AXIS_V",cm_a.V);
  vx_io_getvec("AXIS_W",cm_a.W);
  vx_io_getvec("AXIS_MIN",cm_a.MIN);
  vx_io_getvec("AXIS_MAX",cm_a.MAX);
  vx_io_getdim("AXIS_N ",cm_a.N);

  NCells=cm_a.N[0]*cm_a.N[1]*cm_a.N[2];
  sprintf(p3.NAME,"cvp");
  vx_io_getpropname("PROP_FILE",1,p3.FN);
  vx_io_getpropsize("PROP_ESIZE",1,&p3.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",1,&p3.NO_DATA_VALUE);

  cmbuffer=(char *)malloc(NCells*p3.ESIZE);
  if (cmbuffer == NULL) {
    fprintf(stderr, "Failed to allocate CM Vp buffer\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p3.FN, 
		       p3.ESIZE, NCells, cmbuffer) != 0) {
    fprintf(stderr, "Failed to load CM Vp volume\n");
    return(1);
  }

  // and the flags
  sprintf(p8.NAME,"tag");
  vx_io_getpropname("PROP_FILE",2,p8.FN);
  vx_io_getpropsize("PROP_ESIZE",2,&p8.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",2,&p8.NO_DATA_VALUE);

  cmtbuffer=(char *)malloc(NCells*p8.ESIZE);
  if (cmtbuffer == NULL) {
    fprintf(stderr, "Failed to allocate CM tag buffer\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p8.FN, 
		       p8.ESIZE, NCells, cmtbuffer) != 0) {
    fprintf(stderr, "Failed to load CM tag volume\n");
    return(1);
  }

  // and vs
  sprintf(p10.NAME,"cvs");
  vx_io_getpropname("PROP_FILE",3,p10.FN);
  vx_io_getpropsize("PROP_ESIZE",3,&p10.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",3,&p10.NO_DATA_VALUE);

  cmvsbuffer=(char *)malloc(NCells*p10.ESIZE);
  if (cmvsbuffer == NULL) {
    fprintf(stderr, "Failed to allocate CM Vs buffer\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p10.FN, 
		       p10.ESIZE, NCells, cmvsbuffer) != 0) {
    fprintf(stderr, "Failed to load CM Vs volume\n");
    return(1);
  }

  vx_io_finalize();

  /**** Now we load the topo, moho, base, model top File *****/
  if (vx_io_init(TO_PAR) != 0) {
    fprintf(stderr, "Failed to load topo param file %s\n", TO_PAR);
    return(1);
  }

  vx_io_getvec("AXIS_O",to_a.O);
  vx_io_getvec("AXIS_U",to_a.U);
  vx_io_getvec("AXIS_V",to_a.V);
  vx_io_getvec("AXIS_W",to_a.W);
  vx_io_getvec("AXIS_MIN",to_a.MIN);
  vx_io_getvec("AXIS_MAX",to_a.MAX);
  vx_io_getdim("AXIS_N ",to_a.N);

  NCells=to_a.N[0]*to_a.N[1]*to_a.N[2];

  // topo
  sprintf(p4.NAME,"topo_dem");
  vx_io_getpropname("PROP_FILE",1,p4.FN);
  vx_io_getpropsize("PROP_ESIZE",1,&p4.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",1,&p4.NO_DATA_VALUE);

  tobuffer=(char *)malloc(NCells*p4.ESIZE);
  if (tobuffer == NULL) {
    fprintf(stderr, "Failed to allocate topo dem buffer\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p4.FN, 
		       p4.ESIZE, NCells, tobuffer) != 0) {
    fprintf(stderr, "Failed to load topo volume\n");
    return(1);
  }

  // moho
  sprintf(p5.NAME,"moho");
  vx_io_getpropname("PROP_FILE",3,p5.FN);
  vx_io_getpropsize("PROP_ESIZE",3,&p5.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",3,&p5.NO_DATA_VALUE);

  mobuffer=(char *)malloc(NCells*p5.ESIZE);
  if (mobuffer == NULL) {
    fprintf(stderr, "Failed to allocate topo moho buffer\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p5.FN, 
		       p5.ESIZE, NCells, mobuffer) != 0) {
    fprintf(stderr, "Failed to load moho volume\n");
    return(1);
  }

  // basement
  sprintf(p6.NAME,"base");
  vx_io_getpropname("PROP_FILE",2,p6.FN);
  vx_io_getpropsize("PROP_ESIZE",2,&p6.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",2,&p6.NO_DATA_VALUE);

  babuffer=(char *)malloc(NCells*p6.ESIZE);
  if (babuffer == NULL) {
    fprintf(stderr, "Failed to allocate topo basement buffer\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p6.FN, 
		       p6.ESIZE, NCells, babuffer) != 0) {
    fprintf(stderr, "Failed to load basement volume\n");
    return(1);
  }

  // top elevation of model
  sprintf(p13.NAME,"modeltop");
  vx_io_getpropname("PROP_FILE",4,p13.FN);
  vx_io_getpropsize("PROP_ESIZE",4,&p13.ESIZE);
  vx_io_getpropval("PROP_NO_DATA_VALUE",4,&p13.NO_DATA_VALUE);

  mtopbuffer=(char *)malloc(NCells*p13.ESIZE);
  if (mtopbuffer == NULL) {
    fprintf(stderr, "Failed to allocate topo modeltop buffer\n");
    return(1);
  }
  if (vx_io_loadvolume(data_dir, p13.FN, 
		       p13.ESIZE, NCells, mtopbuffer) != 0) {
    fprintf(stderr, "Failed to load modeltop volume\n");
    return(1);
  }

  vx_io_finalize();

  // compute steps
  step_to[0]=to_a.U[0]/(to_a.N[0]-1);
  step_to[1]=to_a.V[1]/(to_a.N[1]-1);
  step_to[2]=0.0;

  step_lr[0]=lr_a.U[0]/(lr_a.N[0]-1);
  step_lr[1]=lr_a.V[1]/(lr_a.N[1]-1);
  step_lr[2]=lr_a.W[2]/(lr_a.N[2]-1);
  
  step_hr[0]=hr_a.U[0]/(hr_a.N[0]-1);
  step_hr[1]=hr_a.V[1]/(hr_a.N[1]-1);
  step_hr[2]=hr_a.W[2]/(hr_a.N[2]-1);
  
  step_cm[0]=cm_a.U[0]/(cm_a.N[0]-1);
  step_cm[1]=cm_a.V[1]/(cm_a.N[1]-1);
  step_cm[2]=cm_a.W[2]/(cm_a.N[2]-1);

  // Load GTL
  if (gtl_setup(gtlpath) != 0) {
    fprintf(stderr, "Failed to perform GTL setup\n");
    return(1);
  }

  is_setup = True;

  return(0);
}


/* Cleanup function to free resources and restore state */
int vx_cleanup()
{
  if (!is_setup) {
    return(1);
  }

  free(hrbuffer);
  free(lrbuffer);
  free(cmbuffer);
  free(tobuffer);
  free(mobuffer);
  free(babuffer);
  free(mtopbuffer);

  free(lrtbuffer);
  free(cmtbuffer);
  free(cmvsbuffer);
  free(hrtbuffer);
  free(lrvsbuffer);
  free(hrvsbuffer);

  vx_zmode = VX_ZMODE_ELEV;
  vx_use_gtl = True;
  is_setup = False;

  callback_bkg = NULL;

  return(0);
}


/* Return current CVM-H version */
int vx_version(char *version)
{
  if (vx_use_gtl == True) {
    sprintf(version, "%s", VERSION);
  } else {
    sprintf(version, "%s (GTL Disabled)", 
	    VERSION);
  }
  return(0);
}


/* Set query mode: elevation, elevation offset, depth */
int vx_setzmode(vx_zmode_t m) {
  vx_zmode = m;
  return(0);
}


/* Enable/disable GTL (default is enabled) */
int vx_setgtl(int flag) {
  vx_use_gtl = flag;
  return(0);
}


/* Query material properties and topography at desired point. 
   Coordinates may be Geo or UTM */
int vx_getcoord(vx_entry_t *entry) {
  return(vx_getcoord_private(entry, True));
}


/* Private query function for material properties. Allows caller to 
   disable advanced features like background model, GTL, and
   depth/offset query modes.
*/ 
int vx_getcoord_private(vx_entry_t *entry, int enhanced) {
  int j;
  double SP[2],SPUTM[2];
  int gcoor[3];
  int do_bkg = False;
  float surface, mtop;
  double elev, depth, zt, topo_gap;
  double incoor[3];

  /* Initialize variables */
  elev = 0.0;
  depth = 0.0;
  zt = 0.0;
  topo_gap = 0.0;


  /* Proceed only if setup has been performed */
  if ((entry == NULL) || (is_setup != True)) {
    return(1);
  }

  /* Make copy of original input coordinates */
  memcpy(incoor, entry->coor, sizeof(double) * 3);

  /* Initialize entry structure */
  vx_init_entry(entry);

  /* Generate UTM coords */
  switch (entry->coor_type) {
  case VX_COORD_GEO:
    
    SP[0]=entry->coor[0];
    SP[1]=entry->coor[1];
    
    gctp(SP,&insys,&inzone,inparm,&inunit,&indatum,&ipr,efile,&jpr,efile,
	 SPUTM,&outsys,&outzone,inparm,&outunit,&outdatum,
	 file27, file83,&iflg);
    
    entry->coor_utm[0]=SPUTM[0];
    entry->coor_utm[1]=SPUTM[1];
    entry->coor_utm[2]=entry->coor[2];
    break;
  case VX_COORD_UTM:
    entry->coor_utm[0]=entry->coor[0];
    entry->coor_utm[1]=entry->coor[1];
    entry->coor_utm[2]=entry->coor[2];
    break;
  default:
    return(1);
    break;
  }

  /* Now we have UTM Zone 11 */
  /*** Prevent all to obvious bad coordinates from being displayed */
  if (entry->coor_utm[1] < 10000000) {
     
    // we start with the elevations; the voxet does not have a vertical 
    // dimension
    gcoor[0]=round((entry->coor_utm[0]-to_a.O[0])/step_to[0]);
    gcoor[1]=round((entry->coor_utm[1]-to_a.O[1])/step_to[1]);
    gcoor[2]=0;
    
    //check if inside
    if(gcoor[0]>=0&&gcoor[1]>=0&&
       gcoor[0]<to_a.N[0]&&gcoor[1]<to_a.N[1]) {	      
      entry->elev_cell[0]= to_a.O[0]+gcoor[0]*step_to[0];
      entry->elev_cell[1]= to_a.O[1]+gcoor[1]*step_to[1];
      j=voxbytepos(gcoor,to_a.N,p4.ESIZE);
      memcpy(&(entry->topo), &tobuffer[j], p4.ESIZE);
      memcpy(&(entry->mtop), &mtopbuffer[j], p4.ESIZE);
      memcpy(&(entry->base), &babuffer[j], p4.ESIZE);
      memcpy(&(entry->moho), &mobuffer[j], p4.ESIZE);
      if (((entry->topo - p0.NO_DATA_VALUE < 0.1) || 
	   (entry->mtop - p0.NO_DATA_VALUE < 0.1))) {
	do_bkg = True;
      }
    } else {
      do_bkg = True;
    }

    /* Convert depth/offset Z coordinate to elevation */
    if (enhanced == True) {
      elev = entry->coor_utm[2];
      vx_getsurface(entry->coor, entry->coor_type, &surface);
      if (surface < -90000.0) {
	return(1);
      }
      switch (vx_zmode) {
      case VX_ZMODE_ELEV:
	break;
      case VX_ZMODE_DEPTH:
	entry->coor[2] = surface - elev;
	entry->coor_utm[2] = entry->coor[2];
	break;
      case VX_ZMODE_ELEVOFF:
	entry->coor[2] = surface + elev;
	entry->coor_utm[2] = entry->coor[2];
	break;
      default:
	return(1);
	break;
      }
      depth = surface - entry->coor_utm[2];
    }

    if ((do_bkg == False) || ((do_bkg == True) && (callback_bkg == NULL)) || 
	(enhanced == False)) {
      /* AP: this calculates the cell numbers from the coordinates and 
	 the grid spacing. The -1 is necessary to do the counting 
	 correctly. The rounding is necessary because the data are cell 
	 centered, eg. they are valid half a cell width away from the 
	 data point */

      /* Extract vp/vs */      
      gcoor[0]=round((entry->coor_utm[0]-hr_a.O[0])/step_hr[0]);
      gcoor[1]=round((entry->coor_utm[1]-hr_a.O[1])/step_hr[1]);
      gcoor[2]=round((entry->coor_utm[2]-hr_a.O[2])/step_hr[2]);
      
      if(gcoor[0]>=0&&gcoor[1]>=0&&gcoor[2]>=0&&
	 gcoor[0]<hr_a.N[0]&&gcoor[1]<hr_a.N[1]&&gcoor[2]<hr_a.N[2]) {
	/* AP: And here are the cell centers*/
	entry->vel_cell[0]= hr_a.O[0]+gcoor[0]*step_hr[0];
	entry->vel_cell[1]= hr_a.O[1]+gcoor[1]*step_hr[1];
	entry->vel_cell[2]= hr_a.O[2]+gcoor[2]*step_hr[2];
	j=voxbytepos(gcoor,hr_a.N,p2.ESIZE);
	memcpy(&(entry->provenance), &hrtbuffer[j], p0.ESIZE);
	memcpy(&(entry->vp), &hrbuffer[j], p2.ESIZE);
	memcpy(&(entry->vs), &hrvsbuffer[j], p2.ESIZE);
	entry->data_src = VX_SRC_HR;
      } else {	  
	gcoor[0]=round((entry->coor_utm[0]-lr_a.O[0])/step_lr[0]);
	gcoor[1]=round((entry->coor_utm[1]-lr_a.O[1])/step_lr[1]);
	gcoor[2]=round((entry->coor_utm[2]-lr_a.O[2])/step_lr[2]);
	
	if(gcoor[0]>=0&&gcoor[1]>=0&&gcoor[2]>=0&&
	   gcoor[0]<lr_a.N[0]&&gcoor[1]<lr_a.N[1]&&gcoor[2]<lr_a.N[2]) {
	  /* AP: And here are the cell centers*/
	  entry->vel_cell[0]= lr_a.O[0]+gcoor[0]*step_lr[0];
	  entry->vel_cell[1]= lr_a.O[1]+gcoor[1]*step_lr[1];
	  entry->vel_cell[2]= lr_a.O[2]+gcoor[2]*step_lr[2];
	  j=voxbytepos(gcoor,lr_a.N,p0.ESIZE);
	  memcpy(&(entry->provenance), &lrtbuffer[j], p0.ESIZE);
	  memcpy(&(entry->vp), &lrbuffer[j], p0.ESIZE);
	  memcpy(&(entry->vs), &lrvsbuffer[j], p0.ESIZE);
	  entry->data_src = VX_SRC_LR;
	} else {   
	  gcoor[0]=round((entry->coor_utm[0]-cm_a.O[0])/step_cm[0]);
	  gcoor[1]=round((entry->coor_utm[1]-cm_a.O[1])/step_cm[1]);
	  gcoor[2]=round((entry->coor_utm[2]-cm_a.O[2])/step_cm[2]);
	  
	  /** AP: check if inside CM voxet; the uppermost layer of 
	      CM overlaps with the lowermost of LR, may need to be 
	      ignored but is not.
	  **/
	  if(gcoor[0]>=0&&gcoor[1]>=0&&gcoor[2]>=0&&
	     gcoor[0]<cm_a.N[0]&&gcoor[1]<cm_a.N[1]&&gcoor[2]<(cm_a.N[2])) {
	    //**** lower crust and mantle voxet *****//
	    entry->vel_cell[0]= cm_a.O[0]+gcoor[0]*step_cm[0];
	    entry->vel_cell[1]= cm_a.O[1]+gcoor[1]*step_cm[1];
	    entry->vel_cell[2]= cm_a.O[2]+gcoor[2]*step_cm[2];
	    j=voxbytepos(gcoor,cm_a.N,p3.ESIZE);
	    memcpy(&(entry->provenance), &cmtbuffer[j], p0.ESIZE);
	    memcpy(&(entry->vp), &cmbuffer[j], p3.ESIZE);
	    memcpy(&(entry->vs), &cmvsbuffer[j], p3.ESIZE);
	    entry->data_src = VX_SRC_CM;
	  } else {
	    do_bkg = True;
	  }
	}
      }
    }

    if ((enhanced == True) && (do_bkg == True) && (callback_bkg != NULL)) {
      /* background model */
      if (callback_bkg(entry, VX_REQUEST_ALL) != 0) {
	/* Restore original input coords */
	memcpy(entry->coor, incoor, sizeof(double) * 3);
	return(1);
      }
    } else {
      /* Compute rho */
      entry->rho = calc_rho(entry->vp, entry->data_src);

      if ((do_bkg == False) && (enhanced == True) && (vx_use_gtl == True)) {

	/* Compute gap between surface and mtop */
	vx_model_top(entry->coor, entry->coor_type, &mtop, True);
	if (mtop - p0.NO_DATA_VALUE > 0.1) {
	  topo_gap = surface - mtop;
	} else {
	  topo_gap = 0.0;
	}

	/* Requery at fixed zt depth if point below trans zone */
	zt = gtl_get_adj_transition(topo_gap);
	if ((entry->coor[2] > surface - zt) && (entry->coor[2] <= surface)) {
	  entry->coor[2] = surface - zt;
	  entry->coor_utm[2] = surface - zt;
	  vx_getcoord_private(entry, False);
	  entry->coor[2] = elev;
	  entry->coor_utm[2] = elev;
	  
	  // We are inside core CVM-H model. Apply GTL
	  if (vx_apply_gtl_entry(entry, depth, topo_gap) != 0) {
	    /* Restore original input coords */
	    memcpy(entry->coor, incoor, sizeof(double) * 3);
	    return(1);
	  }
	}
      }
    }
  }

  /* Restore original input coords */
  memcpy(entry->coor, incoor, sizeof(double) * 3);
  return(0);
}


/* Smooth the material properties contained in 'entry' with the GTL, at 
   effective depth 'depth' and with a local topo gap of 'topo-gap'. */
int vx_apply_gtl_entry(vx_entry_t *entry, double depth, double topo_gap) {
  int i;
  gtl_entry_t gtlentry;
  int updated = False;

  if (entry == NULL) {
    return(1);
  }

  /* Fill in GTL request */
  for (i = 0; i < 3; i++) {
    gtlentry.coor_utm[i] = entry->coor_utm[i];
  }
  gtlentry.topo_gap = topo_gap;
  gtlentry.depth = depth;
  gtlentry.vp = entry->vp;
  gtlentry.vs = entry->vs;
  gtlentry.rho = entry->rho;

  /* Query GTL and perform interpolation */
  if (gtl_interp(&gtlentry, &updated) != 0) {
    return(1);
  }

  if (updated) {
 
    /* Replace entry with GTL results */
    for (i = 0; i < 3; i++) {
      entry->vel_cell[i] = gtlentry.cell[i];
    }
    entry->data_src = VX_SRC_GT;
    entry->provenance = (float)VX_PROV_GTL;
    entry->vp = gtlentry.vp;
    entry->vs = gtlentry.vs;
    entry->rho = gtlentry.rho;
  }

  return(0);
}


/* Get raw voxel information at the supplied voxel volume coordinates */
void vx_getvoxel(vx_voxel_t *voxel) {
  int gcoor[3];
  int j;

  /* Proceed only if setup has been performed */
  if ((voxel == NULL) || (is_setup != True)) {
    return;
  }

  // Initialize entry structure
  vx_init_voxel(voxel);

  gcoor[0] = voxel->coor[0];
  gcoor[1] = voxel->coor[1];
  gcoor[2] = voxel->coor[2];
	  
  switch (voxel->data_src) {
  case VX_SRC_TO:
    if(gcoor[0]>=0&&gcoor[1]>=0&&
       gcoor[0]<to_a.N[0]&&gcoor[1]<to_a.N[1])
      {
	voxel->elev_cell[0]= to_a.O[0]+gcoor[0]*step_to[0];
        voxel->elev_cell[1]= to_a.O[1]+gcoor[1]*step_to[1];
	j=voxbytepos(gcoor,to_a.N,p4.ESIZE);
	memcpy(&(voxel->topo), &tobuffer[j], p4.ESIZE);
	memcpy(&(voxel->mtop), &mtopbuffer[j], p4.ESIZE);
	memcpy(&(voxel->base), &babuffer[j], p4.ESIZE);
	memcpy(&(voxel->moho), &mobuffer[j], p4.ESIZE);
      } 
    break;
  case VX_SRC_HR:

    if(gcoor[0]>=0&&gcoor[1]>=0&&gcoor[2]>=0&&
       gcoor[0]<hr_a.N[0]&&gcoor[1]<hr_a.N[1]&&gcoor[2]<hr_a.N[2])
      {
	voxel->vel_cell[0]= hr_a.O[0]+gcoor[0]*step_hr[0];
	voxel->vel_cell[1]= hr_a.O[1]+gcoor[1]*step_hr[1];
	voxel->vel_cell[2]= hr_a.O[2]+gcoor[2]*step_hr[2];
	j=voxbytepos(gcoor,hr_a.N,p2.ESIZE);
	memcpy(&(voxel->provenance), &hrtbuffer[j], p0.ESIZE);
	memcpy(&(voxel->vp), &hrbuffer[j], p2.ESIZE);
	memcpy(&(voxel->vs), &hrvsbuffer[j], p2.ESIZE);	
      }

    break;
  case VX_SRC_LR:

    if(gcoor[0]>=0&&gcoor[1]>=0&&gcoor[2]>=0&&
       gcoor[0]<lr_a.N[0]&&gcoor[1]<lr_a.N[1]&&gcoor[2]<lr_a.N[2])
      {
	voxel->vel_cell[0]= lr_a.O[0]+gcoor[0]*step_lr[0];
	voxel->vel_cell[1]= lr_a.O[1]+gcoor[1]*step_lr[1];
	voxel->vel_cell[2]= lr_a.O[2]+gcoor[2]*step_lr[2];
	j=voxbytepos(gcoor,lr_a.N,p2.ESIZE);
	memcpy(&(voxel->provenance), &lrtbuffer[j], p0.ESIZE);
	memcpy(&(voxel->vp), &lrbuffer[j], p2.ESIZE);
	memcpy(&(voxel->vs), &lrvsbuffer[j], p2.ESIZE);	
      }

    break;
  case VX_SRC_CM:

    if(gcoor[0]>=0&&gcoor[1]>=0&&gcoor[2]>=0&&
       gcoor[0]<cm_a.N[0]&&gcoor[1]<cm_a.N[1]&&gcoor[2]<cm_a.N[2])
      {
	voxel->vel_cell[0]= cm_a.O[0]+gcoor[0]*step_cm[0];
	voxel->vel_cell[1]= cm_a.O[1]+gcoor[1]*step_cm[1];
	voxel->vel_cell[2]= cm_a.O[2]+gcoor[2]*step_cm[2];
	j=voxbytepos(gcoor,cm_a.N,p2.ESIZE);
	memcpy(&(voxel->provenance), &cmtbuffer[j], p0.ESIZE);
	memcpy(&(voxel->vp), &cmbuffer[j], p2.ESIZE);
	memcpy(&(voxel->vs), &cmvsbuffer[j], p2.ESIZE);	
      }

    break;
  default:
    break;
  }

  return;
}


/* Query elevation of free surface at point 'coor' */
void vx_getsurface(double *coor, vx_coord_t coor_type, float *surface)
{
  vx_getsurface_private(coor, coor_type, surface, False);
  return;
}


/* Private function for querying elevation of free surface at point 'coor'.
   Allows caller to exclude background model. */
int vx_getsurface_private(double *coor, vx_coord_t coor_type, 
			  float *surface, int exclude_bkg)
{
  int gcoor[3];
  double SP[2],SPUTM[2];
  int j;
  vx_entry_t entry;
  int do_bkg = False;

  *surface = p0.NO_DATA_VALUE;

  entry.coor[0] = coor[0];
  entry.coor[1] = coor[1];
  entry.coor[2] = 0.0;
  entry.coor_type = coor_type;

  // Initialize entry structure
  vx_init_entry(&entry);

  switch (entry.coor_type) {
  case VX_COORD_GEO:
    
    SP[0]=entry.coor[0];
    SP[1]=entry.coor[1];
    
    gctp(SP,&insys,&inzone,inparm,&inunit,&indatum,&ipr,efile,&jpr,efile,
	 SPUTM,&outsys,&outzone,inparm,&outunit,&outdatum,
	 file27, file83,&iflg);
    
    entry.coor_utm[0]=SPUTM[0];
    entry.coor_utm[1]=SPUTM[1];
    entry.coor_utm[2]=entry.coor[2];
    break;
  case VX_COORD_UTM:
    entry.coor_utm[0]=entry.coor[0];
    entry.coor_utm[1]=entry.coor[1];
    entry.coor_utm[2]=entry.coor[2];
    break;
  default:
    return(1);
    break;
  }

  gcoor[0]=round((entry.coor_utm[0]-to_a.O[0])/step_to[0]);
  gcoor[1]=round((entry.coor_utm[1]-to_a.O[1])/step_to[1]);
  gcoor[2]=0;
    
  /* check if inside topo volume */
  if(gcoor[0]>=0&&gcoor[1]>=0&&
     gcoor[0]<to_a.N[0]&&gcoor[1]<to_a.N[1]) {	      
    j=voxbytepos(gcoor,to_a.N,p4.ESIZE);
    memcpy(&(entry.topo), &tobuffer[j], p4.ESIZE);
    memcpy(&(entry.mtop), &mtopbuffer[j], p4.ESIZE);

    // Test points for topo - mtop gap
    // -118.75 36.8 0.0
    // -118.5 36.8 0.0
    // 345500.000000  4059000.0 0.0

    if (vx_use_gtl == True) {
      if (entry.topo - p0.NO_DATA_VALUE > 0.1) {
	*surface = entry.topo;
	
	/* Check that this point falls within a model */
	entry.coor[2] = *surface;
	vx_getcoord_private(&entry, False);
	if (entry.data_src == VX_SRC_NR) {
	  do_bkg = True;
	}
	
      } else {
	do_bkg = True;
      }

    } else {

      /* check for valid topo values */
      if ((entry.topo - p0.NO_DATA_VALUE > 0.1) && 
	  (entry.mtop - p0.NO_DATA_VALUE > 0.1)) {
	
	if (entry.topo > entry.mtop) {
	  *surface = entry.mtop - ELEV_EPSILON;
	} else {
	  *surface = entry.topo - ELEV_EPSILON;
	}
	
	int flag = 0;
	int num_iter = 0;
	entry.coor[2] = *surface;
	while (!flag) {
	  if (num_iter > MAX_ITER_ELEV) {
	    *surface = p0.NO_DATA_VALUE;
	    flag = 1;
	  }
	  num_iter = num_iter + 1;
	  vx_getcoord_private(&entry, False);
	  if ((entry.vp < 0.0) || (entry.vs < 0.0)) {
	    switch (entry.data_src) {
	    case VX_SRC_CM:
	      entry.coor[2] -= fabs(step_cm[2]);
	      break;
	    case VX_SRC_HR:
	      entry.coor[2] -= fabs(step_hr[2]);
	      break;
	    case VX_SRC_LR:
	      entry.coor[2] -= fabs(step_lr[2]);
	      break;
	    default:
	      do_bkg = True;
	      flag = 1;
	      break;
	    }
	  } else {
	    *surface = entry.coor[2];
	    flag = 1;
	  }
	}
      } else {
	do_bkg = True;
      }
    }
      
  } else {
    do_bkg = True;
  }

  if (do_bkg) {
    if ((!exclude_bkg) && (callback_bkg != NULL)) {
      callback_bkg(&entry, VX_REQUEST_TOPO);
      *surface = entry.topo;
    } else {
      *surface = p0.NO_DATA_VALUE;
    }
  }

  return(0);
}


/* Return mtop at coordinates 'coor' in 'surface'. Caller may disable use
   of background. */
void vx_model_top(double *coor, vx_coord_t coor_type, 
		  float *surface, int exclude_bkg)
{
  int gcoor[3];
  double SP[2],SPUTM[2];
  int j;
  vx_entry_t entry;
  int do_bkg = False;

  *surface = p0.NO_DATA_VALUE;

  entry.coor[0] = coor[0];
  entry.coor[1] = coor[1];
  entry.coor[2] = 0.0;
  entry.coor_type = coor_type;

  // Initialize entry structure
  vx_init_entry(&entry);

  switch (entry.coor_type) {
  case VX_COORD_GEO:
    
    SP[0]=entry.coor[0];
    SP[1]=entry.coor[1];
    
    gctp(SP,&insys,&inzone,inparm,&inunit,&indatum,&ipr,efile,&jpr,efile,
	 SPUTM,&outsys,&outzone,inparm,&outunit,&outdatum,
	 file27, file83,&iflg);
    
    entry.coor_utm[0]=SPUTM[0];
    entry.coor_utm[1]=SPUTM[1];
    entry.coor_utm[2]=entry.coor[2];
    break;
  case VX_COORD_UTM:
    entry.coor_utm[0]=entry.coor[0];
    entry.coor_utm[1]=entry.coor[1];
    entry.coor_utm[2]=entry.coor[2];
    break;
  default:
    return;
    break;
  }

  gcoor[0]=round((entry.coor_utm[0]-to_a.O[0])/step_to[0]);
  gcoor[1]=round((entry.coor_utm[1]-to_a.O[1])/step_to[1]);
  gcoor[2]=0;
    
  /* check if inside topo volume */
  if(gcoor[0]>=0&&gcoor[1]>=0&&
     gcoor[0]<to_a.N[0]&&gcoor[1]<to_a.N[1]) {	      
    j=voxbytepos(gcoor,to_a.N,p4.ESIZE);
    memcpy(&(entry.topo), &tobuffer[j], p4.ESIZE);
    memcpy(&(entry.mtop), &mtopbuffer[j], p4.ESIZE);
  } else {
    do_bkg = True;
  }

  if (!do_bkg) {

    /* check for valid topo values */
    if ((entry.topo - p0.NO_DATA_VALUE > 0.1) && 
	(entry.mtop - p0.NO_DATA_VALUE > 0.1)) {
      if (entry.topo > entry.mtop) {
	*surface = entry.mtop - ELEV_EPSILON;
      } else {
	*surface = entry.topo - ELEV_EPSILON;
      }
      
      int flag = 0;
      int num_iter = 0;
      entry.coor[2] = *surface;
      while (!flag) {
	if (num_iter > MAX_ITER_ELEV) {
	  *surface = p0.NO_DATA_VALUE;
	  flag = 1;
	}
	num_iter = num_iter + 1;
	vx_getcoord_private(&entry, False);
	if ((entry.vp < 0.0) || (entry.vs < 0.0)) {
	  switch (entry.data_src) {
	  case VX_SRC_CM:
	    entry.coor[2] -= fabs(step_cm[2]);
	    break;
	  case VX_SRC_HR:
	    entry.coor[2] -= fabs(step_hr[2]);
	    break;
	  case VX_SRC_LR:
	    entry.coor[2] -= fabs(step_lr[2]);
	    break;
	  default:
	    do_bkg = True;
	    flag = 1;
	    break;
	  }
	} else {
	  *surface = entry.coor[2];
	  flag = 1;
	}
      }
      
    } else {
      do_bkg = True;
    }
  }

  if (do_bkg) {
    if ((!exclude_bkg) && (callback_bkg != NULL)) {
      callback_bkg(&entry, VX_REQUEST_TOPO);
      if (entry.topo > entry.mtop) {
	*surface = entry.mtop - ELEV_EPSILON;
      } else {
	*surface = entry.topo - ELEV_EPSILON;
      }
    } else {
	*surface = p0.NO_DATA_VALUE;
    }
  }

  return;
}


/* Register user-defined background model as active background model */
int vx_register_bkg( int (*backgrnd)(vx_entry_t *entry,
				     vx_request_t req_type) )
{
  callback_bkg = backgrnd;
  return(0);
}


/* Register the SCEC 1D model as the active background model */
int vx_register_scec()
{
  /* Proceed only if setup has been performed */
  if (is_setup != True) {
    return(1);
  }

  callback_bkg = vx_scec_1d;
  return(0);
}


/* Return the closest UTM coordinates in the lr and cm models to point 
   'entry'. The arg 'surface_elev' contains the surface elevation at the
   point and 'topo_gap' is (mtop-topo). */
int vx_get_closest_coords(vx_entry_t *entry, 
			  vx_entry_t *lr_entry,
			  vx_entry_t *cm_entry,
			  float *surface_elev,
			  double *topo_gap)
{
  double depth;
  float mtop;
  double zt, zt_default;

  vx_entry_t to_entry;
  vx_voxel_t lr_voxel;

  vx_init_entry(lr_entry);
  vx_init_entry(cm_entry);
  *surface_elev = -99999.0;
  *topo_gap = 0.0;

  depth = (0.0 - entry->coor_utm[2]);

  /* Check if point is in the air */
  if (depth < 0.0) {
    return(0);
  }

  /* Get closest lr voxel to POI */
  memcpy(lr_entry, entry, sizeof(vx_entry_t));
  lr_entry->data_src = VX_SRC_LR;
  vx_closest_voxel_to_coord(lr_entry, &lr_voxel);

  /* Get surface elev at this point */
  to_entry.coor_utm[0]= lr_a.O[0]+lr_voxel.coor[0]*step_lr[0];
  to_entry.coor_utm[1]= lr_a.O[1]+lr_voxel.coor[1]*step_lr[1];
  to_entry.coor_utm[2] = 0.0;
  to_entry.data_src = VX_SRC_TO;
  vx_getsurface_private(to_entry.coor_utm, VX_COORD_UTM, surface_elev, True);
  if (*surface_elev - p0.NO_DATA_VALUE >= 0.1) {
    /* Find the lr/cm voxel that corresponds to desired depth/elev */
    /* This will be the closest voxel */
    memcpy(lr_entry, &(to_entry), sizeof(vx_entry_t));

    /* Compute gap between surface and mtop */
    vx_model_top(to_entry.coor_utm, VX_COORD_UTM, &mtop, True);
    if ((entry->topo - p0.NO_DATA_VALUE > 0.1) && 
	(mtop - p0.NO_DATA_VALUE > 0.1)) {
      *topo_gap = *surface_elev - mtop;
    } else {
      *topo_gap = 0.0;
    }

    /* Get GTL transition depth */
    zt = gtl_get_adj_transition(*topo_gap);
    zt_default = gtl_get_transition();

    switch (vx_zmode) {
    case VX_ZMODE_ELEV:
      lr_entry->coor_utm[2] = entry->coor_utm[2];
      break;
    case VX_ZMODE_DEPTH:
    case VX_ZMODE_ELEVOFF:
      lr_entry->coor_utm[2] = *surface_elev - depth;
      if (zt > zt_default) {
	lr_entry->coor_utm[2] = lr_entry->coor_utm[2] - (zt - zt_default);
      }
      break;
    //case VX_ZMODE_ELEVOFF:
    //lr_entry->coor_utm[2] = *surface_elev - depth;
    //if (zt > zt_default) {
    //	lr_entry->coor_utm[2] = lr_entry->coor_utm[2] - (zt - zt_default);
    //   }
    //break;
    default:
      return(0);
      break;
    }

    //fprintf(stderr, "vx_closest_coords lr_entry->coor[2]=%lf\n",
    //	    lr_entry->coor_utm[2]);

    /* Compute gap between surface and mtop */
    vx_model_top(to_entry.coor_utm, VX_COORD_UTM, &mtop, True);
    if (mtop - p0.NO_DATA_VALUE > 0.1) {
      *topo_gap = *surface_elev - mtop;
    } else {
      *topo_gap = 0.0;
    }

    lr_entry->data_src = VX_SRC_LR;
    memcpy(cm_entry, lr_entry, sizeof(vx_entry_t));
    cm_entry->data_src = VX_SRC_CM;
  }

  return(0);
}


/* Find closest voxel among LR and CM candidate voxels */
int vx_select_closest_voxel(vx_entry_t *lr_entry,
			    vx_entry_t *cm_entry,
			    int *found_closest,
			    int *is_water_air,
			    vx_entry_t *closest_entry,
			    vx_voxel_t *closest_voxel)
{
  vx_voxel_t lr_voxel, cm_voxel;

  vx_init_entry(closest_entry);
  vx_init_voxel(closest_voxel);
  *found_closest = False;
  *is_water_air = False;

  vx_voxel_at_coord(lr_entry, &lr_voxel);
  vx_voxel_at_coord(cm_entry, &cm_voxel);

  /* Give preference to LR results */
  if ((lr_voxel.vp > 0.0) && (lr_voxel.vs > 0.0) && (lr_voxel.rho > 0.0)) {
    memcpy(closest_entry, lr_entry, sizeof(vx_entry_t));
    memcpy(closest_voxel, &lr_voxel, sizeof(vx_voxel_t));
    *found_closest = True;
  } else if ((cm_voxel.vp > 0.0) && (cm_voxel.vs > 0.0) && 
	     (cm_voxel.rho > 0.0)) {
    memcpy(closest_entry, cm_entry, sizeof(vx_entry_t));
    memcpy(closest_voxel, &cm_voxel, sizeof(vx_voxel_t));
    *found_closest = True;
  }

  if (*found_closest == False) {
    /* Consider case of closest voxel being in water, where vs = NO_DATA */
    if ((lr_voxel.provenance == VX_PROV_WATER) || 
	(lr_voxel.provenance == VX_PROV_AIR) ||
	(lr_voxel.provenance == VX_PROV_AIR_OUTER)) {
      memcpy(closest_entry, lr_entry, sizeof(vx_entry_t));
      memcpy(closest_voxel, &lr_voxel, sizeof(vx_voxel_t));
      *is_water_air = True;
      *found_closest = True;
    } else if ((cm_voxel.provenance == VX_PROV_WATER) || 
	       (cm_voxel.provenance == VX_PROV_AIR) ||
	       (cm_voxel.provenance == VX_PROV_AIR_OUTER)) {
      memcpy(closest_entry, cm_entry, sizeof(vx_entry_t));
      memcpy(closest_voxel, &cm_voxel, sizeof(vx_voxel_t));
      *is_water_air = True;
      *found_closest = True;
    }
  }
  
  return(0);
}


/* Return SCEC 1D background properties at point 'entry'. The 
   request type 'req_type' denotes what info to return: topo, 
   material properties, or both. The flag 'apply_gtl' determines if
   the GTL is applied. */
int vx_scec_1d_basic(vx_entry_t *entry, vx_request_t req_type, int apply_gtl)
{
  double depth, closest_depth;
  double dist_ratio;
  double vp, vs, rho;
  double zt, topo_gap;
  float surface_elev;
  int do_gtl = False;

  int found_closest, is_water_air;
  vx_entry_t closest_entry;
  vx_voxel_t closest_voxel;

  vx_entry_t lr_entry, cm_entry;
  float closest_dist_2d, closest_dist_3d;

  /* Save basic SCEC topo info */
  entry->topo = 0.0;
  entry->mtop = 0.0;
  entry->base = p0.NO_DATA_VALUE;
  entry->moho = p0.NO_DATA_VALUE;  
  if (req_type == VX_REQUEST_TOPO) {
    return 0;
  }

  vx_init_voxel(&closest_voxel);
  vx_init_entry(&closest_entry);

  depth = (0.0 - entry->coor_utm[2]);
  /* Check if point is in the air */
  if (depth < 0.0) {
    entry->data_src = VX_SRC_NR;
    entry->provenance = (float)VX_PROV_AIR;
    entry->vp = entry->vs = entry->rho = p0.NO_DATA_VALUE;
    return 0;
  }

  vx_get_closest_coords(entry, &lr_entry, &cm_entry, 
			&surface_elev, &topo_gap);

  /* Get GTL transition depth */
  zt = gtl_get_adj_transition(topo_gap);
  
  // Test points along border btw core and bkg
  //581000.000000 3430500 0.0
  //581000.000000 3430750 0.0
  
  closest_depth = surface_elev - lr_entry.coor_utm[2];
  
  if (apply_gtl == True) {
    if ((lr_entry.coor_utm[2] > (double)surface_elev - zt) && 
	(lr_entry.coor_utm[2] <= (double)surface_elev)) {
      lr_entry.coor_utm[2] = (double)surface_elev - zt;
      cm_entry.coor_utm[2] = (double)surface_elev - zt;
      do_gtl = True;
    }
  }

  /* Find the closest of the two */
  vx_select_closest_voxel(&lr_entry, &cm_entry, 
			  &found_closest, &is_water_air, 
			  &closest_entry, &closest_voxel);

  /* Acquire vp from depth with SCEC 1D model */
  vp = scec_vp(depth);

  /* Calculate rho */
  rho = scec_rho(vp);

  /* Calculate vs */
  vs = scec_vs(vp, rho);

  if ((found_closest == True) && (is_water_air == False)) {
    closest_entry.vp = closest_voxel.vp;
    closest_entry.vs = closest_voxel.vs;
    closest_entry.rho = closest_voxel.rho;

    if (do_gtl == True) {
      /* Apply GTL to closest voxel */
      vx_apply_gtl_entry(&closest_entry, closest_depth, topo_gap);
    }

    vx_dist_point_to_voxel(entry, &closest_voxel, 
			   &closest_dist_2d, &closest_dist_3d);
    dist_ratio = closest_dist_2d / (SCEC_SMOOTH_DIST * 1000.0);

    /* Scale vp,vs,rho by distance from voxel w/ linear interpolation */
    if (dist_ratio < 1.0) {
      vp = vx_interpolate(closest_entry.vp, vp, dist_ratio);
      rho = vx_interpolate(closest_entry.rho, rho, dist_ratio);
      vs = vx_interpolate(closest_entry.vs, vs, dist_ratio);
    }

  } else if (is_water_air == True) {
    vp = closest_voxel.vp;
    vs = closest_voxel.vs;
    rho = closest_voxel.rho;
  }

  /* Save values */
  entry->data_src = VX_SRC_BK;
  if (is_water_air == True) {
    entry->provenance = closest_voxel.provenance;
  }else {
    entry->provenance = (float)VX_PROV_BACKGND;
  }
  entry->vp = vp;
  entry->vs = vs;
  entry->rho = rho;

  return 0;
}


/* Return SCEC 1D background properties at point 'entry'. The 
   request type 'req_type' denotes what info to return: topo, 
   material properties, or both. The z coord in entry is assumed 
   to be elevation. The GTL is applied if point falls within 
   0-trans_depth meters of the surface. */
int vx_scec_1d_gtl_elev(vx_entry_t *entry, vx_request_t req_type)
{
  int i;
  double depth;
  double vp, vs, rho;
  double zt;
  int updated;
  double topo_gap;
  float surface_elev;
  int found_closest, is_water_air;

  vx_entry_t lr_entry, cm_entry, closest_entry;
  vx_voxel_t closest_voxel;
  gtl_entry_t gtl;
  double dist_ratio;

  depth = (0.0 - entry->coor_utm[2]);
  zt = gtl_get_transition();

  /* Acquire vp,vs,rho at this point from depth with SCEC 1D model */
  vp = scec_vp(depth);
  
  /* Calculate rho */
  rho = scec_rho(vp);
  
  /* Calculate vs */
  vs = scec_vs(vp, rho);
  
  entry->data_src = VX_SRC_BK;
  entry->provenance = (float)VX_PROV_BACKGND;
  entry->vp = vp;
  entry->vs = vs;
  entry->rho = rho;
  
  /* Find closest point in GTL */
  
  /* Find GTL/1D interpolated point at this elev */
  /* Fill in GTL request */
  for (i = 0; i < 2; i++) {
    gtl.coor_utm[i] = entry->coor_utm[i];
  }
  gtl.coor_utm[2] = -zt;
  gtl.topo_gap = 0.0;
  gtl.depth = depth;
  gtl.vp = vp;
  gtl.vs = vs;
  gtl.rho = rho;
  
  /* Interpolate this with vs30 */
  if (gtl_interp(&gtl, &updated) != 0) {
    return(1);
  }
  
  if (updated) {
    /* Replace entry with GTL results */
    for (i = 0; i < 3; i++) {
      entry->vel_cell[i] = gtl.cell[i];
    }
    entry->data_src = VX_SRC_GT;
    entry->provenance = (float)VX_PROV_GTL;
    entry->vp = gtl.vp;
    entry->vs = gtl.vs;
    entry->rho = gtl.rho;
  }
  
  /* Find GTL/core interpolated closest point at this elev */
  
  /* Find closest point in core */
  vx_get_closest_coords(entry, &lr_entry, &cm_entry, 
			&surface_elev, &topo_gap);
  
  /* Find the closest of the two candidates */
  vx_select_closest_voxel(&lr_entry, &cm_entry, 
			  &found_closest, &is_water_air, 
			  &closest_entry, &closest_voxel);
  
  if (found_closest == False) {
    /* Return the GTL adjusted 1D values */
    return(0);
  }
  
  vx_scec_1d_basic(&closest_entry, req_type, True);
  if ((closest_entry.provenance == VX_PROV_WATER) ||
      (closest_entry.provenance == VX_PROV_AIR) ||
      (closest_entry.provenance == VX_PROV_AIR_OUTER)) {
    entry->data_src = VX_SRC_BK;
    entry->provenance = closest_entry.provenance;
    entry->vp = closest_entry.vp;
    entry->vs = closest_entry.vs;
    entry->rho = closest_entry.rho;
    return(0);
  }
  
  /* Interpolate between the two */
  if ((closest_entry.vp > 0.0) && 
      (closest_entry.vs > 0.0) && 
      (closest_entry.rho > 0.0)) {
    
    /* Interpolate between nearest core point and this point */
    dist_ratio = vx_dist_2d(entry->coor_utm[0], 
			 entry->coor_utm[1],
			 closest_entry.coor_utm[0],
			 closest_entry.coor_utm[1]) / 
      (SCEC_SMOOTH_DIST * 1000.0);
    
    /* Scale vp,vs,rho by distance from GTL w/ linear interpolation */
    if (dist_ratio < 1.0) {
      entry->vp = vx_interpolate(closest_entry.vp, entry->vp, dist_ratio);
      entry->rho = vx_interpolate(closest_entry.rho, entry->rho, 
			       dist_ratio);
      entry->vs = vx_interpolate(closest_entry.vs, entry->vs, dist_ratio);
    }
    
  } else {
    /* Return the GTL adjusted 1D values */
    return(0);
  }
  

  return(0);
}


/* Apply SCEC 1D background at point 'entry'. The request type
   'req_type' denotes what info to return: topo, material properties,
   or both. */
int vx_scec_1d(vx_entry_t *entry, vx_request_t req_type)
{
  int i;
  double depth;
  double vp, vs, rho;
  double zt;
  double tmpz;
  int updated;

  vx_entry_t tmp_entry;
  gtl_entry_t gtl;
  double dist_ratio, closest_dist_2d;

  /* Save basic SCEC topo info */
  entry->topo = 0.0;
  entry->mtop = 0.0;
  entry->base = p0.NO_DATA_VALUE;
  entry->moho = p0.NO_DATA_VALUE;  
  if (req_type == VX_REQUEST_TOPO) {
    return 0;
  }

  depth = (0.0 - entry->coor_utm[2]);
  zt = gtl_get_transition();

  /* Check if point is in the air */
  if (depth < 0.0) {
    entry->data_src = VX_SRC_NR;
    entry->provenance = (float)VX_PROV_AIR;
    entry->vp = entry->vs = entry->rho = p0.NO_DATA_VALUE;
    return 0;
  }

  /* Check if GTL disabled */
  if (vx_use_gtl == False) {
    /* Use SCEC 1D with interpolation with core model */
    vx_scec_1d_basic(entry, req_type, False);
    return 0;
  }

  if ((vx_zmode == VX_ZMODE_DEPTH) || 
      ((vx_zmode == VX_ZMODE_ELEVOFF) && (depth >= 0.0))) {

    if (depth >= zt) {
      /* Use SCEC 1D with interpolation with core model */
      vx_scec_1d_basic(entry, req_type, False);
      return 0;
    } else {
      if (gtl_point_is_inside(entry->coor_utm) == True) {
	tmpz = entry->coor_utm[2];
	entry->coor_utm[2] = -zt;
	
	/* Get SCEC 1D / core model interpolated values */
	vx_scec_1d_basic(entry, VX_REQUEST_ALL, False);

	//fprintf(stderr, "coord=%lf,%lf, %lf\n", entry->coor_utm[0],
	//	entry->coor_utm[1], entry->coor_utm[2]);
	//fprintf(stderr, "vp=%lf, vs=%lf, rho=%f\n", entry->vp,
	//	entry->vs, entry->rho);
	
	if ((entry->provenance == VX_PROV_WATER) || 
	    (entry->provenance == VX_PROV_AIR) ||
	    (entry->provenance == VX_PROV_AIR_OUTER)) {
	  return(0);
	}

	/* Fill in GTL request */
	for (i = 0; i < 2; i++) {
	  gtl.coor_utm[i] = entry->coor_utm[i];
	}
	gtl.coor_utm[2] = -zt;
	gtl.topo_gap = 0.0;
	gtl.depth = depth;
	gtl.vp = entry->vp;
	gtl.vs = entry->vs;
	gtl.rho = entry->rho;
	
	/* Interpolate this with vs30 */
	if (gtl_interp(&gtl, &updated) != 0) {
	  return(1);
	}
	
	entry->coor_utm[2] = tmpz;
	
	if (updated) {
	  /* Replace entry with GTL results */
	  for (i = 0; i < 3; i++) {
	    entry->vel_cell[i] = gtl.cell[i];
	  }
	  entry->data_src = VX_SRC_GT;
	  entry->provenance = (float)VX_PROV_GTL;
	  entry->vp = gtl.vp;
	  entry->vs = gtl.vs;
	  entry->rho = gtl.rho;
	}
	
      } else {

	/* Find closest point in GTL */
	gtl_closest_point(entry->coor_utm, tmp_entry.coor_utm, 
			  &closest_dist_2d);
	tmp_entry.coor_utm[2] = -zt;
	
	/* Get SCEC 1D / core model interpolated values */
	vx_scec_1d_basic(&tmp_entry, VX_REQUEST_ALL, False);
	
	if ((tmp_entry.provenance == VX_PROV_WATER) || 
	    (tmp_entry.provenance == VX_PROV_AIR)) {
	  entry->data_src = tmp_entry.data_src;
	  entry->provenance = tmp_entry.provenance;
	  entry->vp = tmp_entry.vp;
	  entry->vs = tmp_entry.vs;
	  entry->rho = tmp_entry.rho;
	  return(0);
	}

	/* Fill in GTL request */
	for (i = 0; i < 3; i++) {
	  gtl.coor_utm[i] = tmp_entry.coor_utm[i];
	}
	gtl.topo_gap = 0.0;
	gtl.depth = depth;
	gtl.vp = tmp_entry.vp;
	gtl.vs = tmp_entry.vs;
	gtl.rho = tmp_entry.rho;
	
	/* Interpolate this with vs30 */
	if (gtl_interp(&gtl, &updated) != 0) {
	  return(1);
	}
	
	if (updated) {
	  /* Replace entry with GTL results */
	  for (i = 0; i < 3; i++) {
	    tmp_entry.vel_cell[i] = gtl.cell[i];
	  }
	  tmp_entry.data_src = VX_SRC_GT;
	  tmp_entry.provenance = (float)VX_PROV_GTL;
	  tmp_entry.vp = gtl.vp;
	  tmp_entry.vs = gtl.vs;
	  tmp_entry.rho = gtl.rho;
	}
	
	/* Acquire vp,vs,rho at this point from depth with SCEC 1D model */
	vp = scec_vp(depth);
	
	/* Calculate rho */
	rho = scec_rho(vp);
	
	/* Calculate vs */
	vs = scec_vs(vp, rho);
	
	/* Save values */
	entry->data_src = VX_SRC_BK;
	entry->provenance = (float)VX_PROV_BACKGND;
	entry->vp = vp;
	entry->vs = vs;
	entry->rho = rho;
	
	/* Interpolate between nearest GTL point and this point */
	dist_ratio = closest_dist_2d / (SCEC_SMOOTH_DIST * 1000.0);
	
	/* Scale vp,vs,rho by distance from GTL w/ linear interpolation */
	if (dist_ratio < 1.0) {
	  entry->vp = vx_interpolate(tmp_entry.vp, entry->vp, dist_ratio);
	  entry->rho = vx_interpolate(tmp_entry.rho, entry->rho, dist_ratio);
	  entry->vs = vx_interpolate(tmp_entry.vs, entry->vs, dist_ratio);
	}
      }
    }

  } else if ((vx_zmode == VX_ZMODE_ELEV) || 
	     ((vx_zmode == VX_ZMODE_ELEVOFF) && (depth < 0.0))) {

    if (depth >= zt) {
      /* Use SCEC 1D with interpolation with core model */
      /* GTL flag is passed because closest voxel at specified elevation 
	 may be within GTL range */
      vx_scec_1d_basic(entry, req_type, True);
      return 0;

    } else {

      if (gtl_point_is_inside(entry->coor_utm) == True) {
	return(vx_scec_1d_gtl_elev(entry, req_type));
      } else {

	/* Find closest point in GTL */
	gtl_closest_point(entry->coor_utm, tmp_entry.coor_utm, 
			  &closest_dist_2d);

	/* Find properties at closest GTL point */
	vx_scec_1d_gtl_elev(&tmp_entry, req_type);

	/* Acquire vp,vs,rho at this point from depth with SCEC 1D model */
	vp = scec_vp(depth);
	
	/* Calculate rho */
	rho = scec_rho(vp);
	
	/* Calculate vs */
	vs = scec_vs(vp, rho);
	
	/* Save values */
	entry->data_src = VX_SRC_BK;
	entry->provenance = (float)VX_PROV_BACKGND;
	entry->vp = vp;
	entry->vs = vs;
	entry->rho = rho;

	/* Interpolate between nearest GTL point and this point */
	if ((tmp_entry.vp > 0.0) && 
	    (tmp_entry.vs > 0.0) && 
	    (tmp_entry.rho > 0.0)) {

	  dist_ratio = vx_dist_2d(tmp_entry.coor_utm[0], 
			       tmp_entry.coor_utm[1],
			       entry->coor_utm[0],
			       entry->coor_utm[1]) / 
	    (SCEC_SMOOTH_DIST * 1000.0);

	  /* Scale vp,vs,rho by distance from GTL w/ linear interpolation */
	  if (dist_ratio < 1.0) {
	    entry->vp = vx_interpolate(tmp_entry.vp, entry->vp, dist_ratio);
	    entry->rho = vx_interpolate(tmp_entry.rho, entry->rho, dist_ratio);
	    entry->vs = vx_interpolate(tmp_entry.vs, entry->vs, dist_ratio);
	  }
	}
      }
    }

  } else {
    return(1);
  }

  return 0;
}


/* Return the voxel 'voxel' that lies precisely at the coord/model
   specified in 'entry'. If point lies outside of volume, the returned
   voxel will be empty. */
void vx_voxel_at_coord(vx_entry_t *entry, vx_voxel_t *voxel)
{
  int j;
  int model_coor[3]; // x,y,z of closest voxel in volume
  int model_max[3]; // max size x,y,z of volume
  double gcoor[3]; // coord of point wrt volume
  float gcoor_min[3]; // UTM coord of volume origin
  float step[3];
  int esize;

  vx_init_voxel(voxel);

  /* find min utm/max coord/step size for specified model */
  for (j = 0; j < 3; j++) {
    switch (entry->data_src) {
    case VX_SRC_TO:
      gcoor_min[j] = to_a.O[j];
      model_max[j] = to_a.N[j];
      step[j] = step_to[j];
      esize = p4.ESIZE;
    case VX_SRC_LR:
      gcoor_min[j] = lr_a.O[j];
      model_max[j] = lr_a.N[j];
      step[j] = step_lr[j];
      esize = p0.ESIZE;
      break;
    case VX_SRC_CM:
      gcoor_min[j] = cm_a.O[j];
      model_max[j] = cm_a.N[j];
      step[j] = step_cm[j];
      esize = p3.ESIZE;
      break;
    default:
      return;
      break;
    }
  }

  /* Find coord of point wrt specified volume */
  gcoor[0]=(entry->coor_utm[0]-gcoor_min[0])/step[0];
  gcoor[1]=(entry->coor_utm[1]-gcoor_min[1])/step[1];
  if (entry->data_src != VX_SRC_TO) {
    gcoor[2]=(entry->coor_utm[2]-gcoor_min[2])/step[2];
  } else {
    gcoor[2] = 0.0;
  }

  /* Find voxel */
  for (j = 0; j < 3; j++) {
    if ((gcoor[j] < 0) || (gcoor[j] > (model_max[j] - 1))) {
      return;
    } else {
      model_coor[j] = round(gcoor[j]);
    }
  }

  /* Calc index byte offset in volume */
  j = voxbytepos(model_coor, model_max, esize);

  /* Get vp/vs for closest voxel */
  switch (entry->data_src) {
  case VX_SRC_TO:
    memcpy(&(voxel->topo), &tobuffer[j], p4.ESIZE);
    memcpy(&(voxel->mtop), &mtopbuffer[j], p4.ESIZE);
    memcpy(&(voxel->base), &babuffer[j], p4.ESIZE);
    memcpy(&(voxel->moho), &mobuffer[j], p4.ESIZE);
  case VX_SRC_LR:
    memcpy(&(voxel->vp), &lrbuffer[j], p0.ESIZE);
    memcpy(&(voxel->vs), &lrvsbuffer[j], p0.ESIZE);
    memcpy(&(voxel->provenance), &lrtbuffer[j], p0.ESIZE);
    voxel->rho = calc_rho(voxel->vp, entry->data_src);
    break;
  case VX_SRC_CM:
    memcpy(&(voxel->vp), &cmbuffer[j], p3.ESIZE);
    memcpy(&(voxel->vs), &cmvsbuffer[j], p3.ESIZE);
    memcpy(&(voxel->provenance), &cmtbuffer[j], p3.ESIZE);
    voxel->rho = calc_rho(voxel->vp, entry->data_src);
    break;
  default:
    return;
    break;
  }

  voxel->coor[0] = model_coor[0];
  voxel->coor[1] = model_coor[1];
  voxel->coor[2] = model_coor[2];
  voxel->data_src = entry->data_src;

  return;
}


/* Return closest voxel to coord/model specified in 'entry' */
void vx_closest_voxel_to_coord(vx_entry_t *entry, vx_voxel_t *voxel)
{
  int j;
  int model_coor[3]; // x,y,z of closest voxel in volume
  int model_max[3]; // max size x,y,z of volume
  double gcoor[3]; // coord of point wrt volume
  float gcoor_min[3]; // UTM coord of volume origin
  float step[3];
  int esize;
  //float testval;

  vx_init_voxel(voxel);

  /* find min utm/max coord/step size for specified model */
  for (j = 0; j < 3; j++) {
    switch (entry->data_src) {
    case VX_SRC_TO:
      gcoor_min[j] = to_a.O[j];
      model_max[j] = to_a.N[j];
      step[j] = step_to[j];
      esize = p4.ESIZE;
    case VX_SRC_LR:
      gcoor_min[j] = lr_a.O[j];
      model_max[j] = lr_a.N[j];
      step[j] = step_lr[j];
      esize = p0.ESIZE;
      break;
    case VX_SRC_CM:
      gcoor_min[j] = cm_a.O[j];
      model_max[j] = cm_a.N[j];
      step[j] = step_cm[j];
      esize = p3.ESIZE;
      break;
    default:
      return;
      break;
    }
  }

  /* Find coord of point wrt specified volume */
  gcoor[0]=(entry->coor_utm[0]-gcoor_min[0])/step[0];
  gcoor[1]=(entry->coor_utm[1]-gcoor_min[1])/step[1];
  if (entry->data_src != VX_SRC_TO) {
    gcoor[2]=(entry->coor_utm[2]-gcoor_min[2])/step[2];
  } else {
    gcoor[2]=0.0;
  }

  /* Find closest voxel */
  for (j = 0; j < 3; j++) {
    if (gcoor[j] < 0) {
      model_coor[j] = 0;
    } else if (gcoor[j] > (model_max[j] - 1)) {
      model_coor[j] = model_max[j] - 1;
    } else {
      model_coor[j] = round(gcoor[j]);
    }
  }

  /* Calc index byte offset in volume */
  j = voxbytepos(model_coor, model_max, esize);

  /* Get vp/vs for closest voxel */
  switch (entry->data_src) {
  case VX_SRC_TO:
    memcpy(&(voxel->topo), &tobuffer[j], p4.ESIZE);
    memcpy(&(voxel->mtop), &mtopbuffer[j], p4.ESIZE);
    memcpy(&(voxel->base), &babuffer[j], p4.ESIZE);
    memcpy(&(voxel->moho), &mobuffer[j], p4.ESIZE);
  case VX_SRC_LR:
    memcpy(&(voxel->vp), &lrbuffer[j], p0.ESIZE);
    memcpy(&(voxel->vs), &lrvsbuffer[j], p0.ESIZE);
    memcpy(&(voxel->provenance), &lrtbuffer[j], p0.ESIZE);
    voxel->rho = calc_rho(voxel->vp, entry->data_src);
    break;
  case VX_SRC_CM:
    memcpy(&(voxel->vp), &cmbuffer[j], p3.ESIZE);
    memcpy(&(voxel->vs), &cmvsbuffer[j], p3.ESIZE);
    memcpy(&(voxel->provenance), &cmtbuffer[j], p3.ESIZE);
    voxel->rho = calc_rho(voxel->vp, entry->data_src);
    break;
  default:
    return;
    break;
  }

  voxel->coor[0] = model_coor[0];
  voxel->coor[1] = model_coor[1];
  voxel->coor[2] = model_coor[2];
  voxel->data_src = entry->data_src;

  return;
}


/* Calculate 2D/3D distance from point 'entry' to voxel 'voxel' */
void vx_dist_point_to_voxel(vx_entry_t *entry, vx_voxel_t *voxel, 
			    float *dist_2d, float *dist_3d)
{
  int j;
  int model_max[3]; // max size x,y,z of volume
  double gcoor[3]; // coord of point wrt volume
  float gcoor_min[3]; // UTM coord of volume origin
  float step[3];
  int esize;
  double dxyz[3];

  /* find min utm/max coord/step size for specified model */
  for (j = 0; j < 3; j++) {
    switch (voxel->data_src) {
    case VX_SRC_TO:
      gcoor_min[j] = to_a.O[j];
      model_max[j] = to_a.N[j];
      step[j] = step_to[j];
      esize = p4.ESIZE;
    case VX_SRC_LR:
      gcoor_min[j] = lr_a.O[j];
      model_max[j] = lr_a.N[j];
      step[j] = step_lr[j];
      esize = p0.ESIZE;
      break;
    case VX_SRC_CM:
      gcoor_min[j] = cm_a.O[j];
      model_max[j] = cm_a.N[j];
      step[j] = step_cm[j];
      esize = p3.ESIZE;
      break;
    default:
      return;
      break;
    }
  }

  /* Find coord of point wrt closest volume */
  gcoor[0]=(entry->coor_utm[0]-gcoor_min[0])/step[0];
  gcoor[1]=(entry->coor_utm[1]-gcoor_min[1])/step[1];
  if (entry->data_src != VX_SRC_TO) {
    gcoor[2]=(entry->coor_utm[2]-gcoor_min[2])/step[2];
  } else {
    gcoor[2]=0.0;
  }

  for (j = 0; j < 3; j++) {
    dxyz[j] = fabs(gcoor[j] - (double)voxel->coor[j]);
  }

  /* Calculate min distance from selected model */
  /* LR cell size is 1000x1000x100, CM is 10000x10000x1000 */
  *dist_2d = sqrt(pow(dxyz[0] * step[0], 2.0) + 
		  pow(dxyz[1] * step[1], 2.0));
  *dist_3d = sqrt(pow(dxyz[0] * step[0], 2.0) + 
		  pow(dxyz[1] * step[1], 2.0) + 
		  pow(dxyz[2] * step[2], 2.0));
  
  return;
}


/* Get voxel byte offset position by the index values 'ic'
   and datatype size 'esize' */
int voxbytepos(int *ic,int *gs,int esize) {
  int pos;

  pos=(gs[0]*gs[1]*(ic[2])+gs[0]*(ic[1])+ic[0])*esize;
  return pos;
}


/* Calculate density (rho) from vp and the source model */
double calc_rho(float vp, vx_src_t data_src)
{
  double rho = 0.0;
  float fl;

  /* Compute rho */
  switch (data_src) {
  case VX_SRC_HR:
  case VX_SRC_LR:
    /*** Density should be at least 1000 ***/
    if (vp!=1480) {
      if (vp>744.) {
	fl = vp/1000.0;
	rho = 1000.0*(fl*(1.6612 + 
			  fl*(-0.4721 + fl*(0.0671 + 
					    fl*(-0.0043 + 
						fl*0.000106)))));
      } else
	rho = 1000.0;
    } else
      rho = 1000.0;
    break;
  case VX_SRC_CM:
    fl = vp/1000;
    rho = 1000.0*(fl*(1.6612 + 
		      fl*(-0.4721 + fl*(0.0671 + 
					fl*(-0.0043 + 
					    fl*0.000106)))));
    break;
  default:
    rho = p0.NO_DATA_VALUE;
    break;
  }

  return(rho);  
}


/* Initialize contents of entry structure */
void vx_init_entry(vx_entry_t *entry) {
  int j;

  for(j = 0; j < 2; j++) {
    entry->coor_utm[j] = p0.NO_DATA_VALUE;
    entry->elev_cell[j] = p0.NO_DATA_VALUE;
    entry->vel_cell[j] = p0.NO_DATA_VALUE;
  }
  entry->vel_cell[2] = p0.NO_DATA_VALUE;
  entry->coor_utm[2] = p0.NO_DATA_VALUE;

  entry->topo = entry->mtop = entry->base = entry->moho = p0.NO_DATA_VALUE;
  entry->provenance = p0.NO_DATA_VALUE;
  entry->vp = entry->vs = entry->rho = p0.NO_DATA_VALUE;
  entry->data_src = VX_SRC_NR;

  return;
}


/* Initialize contents of voxel structure */
void vx_init_voxel(vx_voxel_t *voxel) {
  int j;

  // Initially set to no data
  for(j = 0; j < 2; j++) {
    voxel->elev_cell[j] = p0.NO_DATA_VALUE;
    voxel->vel_cell[j] = p0.NO_DATA_VALUE;
  }
  voxel->vel_cell[2] = p0.NO_DATA_VALUE;
  voxel->topo = voxel->mtop = voxel->base = voxel->moho = p0.NO_DATA_VALUE;
  voxel->provenance = p0.NO_DATA_VALUE;
  voxel->vp = voxel->vs = voxel->rho = p0.NO_DATA_VALUE;

  return;
}

