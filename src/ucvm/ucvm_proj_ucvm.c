#include <stdio.h>
#include <string.h>
#include "ucvm_utils.h"
#include "ucvm_proj_ucvm.h"


/* Initialize projection */
int ucvm_proj_ucvm_init(const char* pstr, const ucvm_point_t *o, 
			double r, ucvm_point_t *s, ucvm_proj_t *p)
{
  if (p == NULL) {
    return(UCVM_CODE_ERROR);
  }

  /* Setup projection */
  p->ipj = pj_init_plus(UCVM_PROJ_GEO);
  p->opj = pj_init_plus(pstr);

  if (p->ipj == NULL) {
    fprintf(stderr, "Failed to create input UCVM projection\n");
    return(UCVM_CODE_ERROR);
  }
  if (p->opj == NULL) {
    fprintf(stderr, "Failed to create output UCVM projection %s\n", pstr);
    fprintf(stderr, "Proj.4 Error: %s\n", pj_strerrno(pj_errno));
    return(UCVM_CODE_ERROR);
  }

  memcpy(&(p->p1), o, sizeof(ucvm_point_t));
  p->rot = r * DEG_TO_RAD;
  memcpy(&(p->size), s, sizeof(ucvm_point_t));
  memcpy(&(p->offset), o, sizeof(ucvm_point_t));

  /* Convert degress to radians */
  p->offset.coord[0] = p->offset.coord[0] * DEG_TO_RAD;
  p->offset.coord[1] = p->offset.coord[1] * DEG_TO_RAD;

  /* Compute origin in proj coords */
  if (pj_transform(p->ipj, p->opj, 1, 1, 
		   &(p->offset.coord[0]), 
		   &(p->offset.coord[1]), 
		   NULL) != 0) {
    fprintf(stderr, "Failed to project origin point\n");
    return(UCVM_CODE_ERROR);
  }

  return(UCVM_CODE_SUCCESS);
}


/* Finalize projection */
int ucvm_proj_ucvm_finalize(ucvm_proj_t *p)
{
  if (p == NULL) {
    return(UCVM_CODE_ERROR);
  }

  if (p->ipj != NULL) {
    pj_free(p->ipj);
  }
  p->ipj = NULL;
  if (p->opj != NULL) {
    pj_free(p->opj);
  }
  p->opj = NULL;

  return(UCVM_CODE_SUCCESS);
}


/* Convert lon,lat to x,y */
int ucvm_proj_ucvm_geo2xy(ucvm_proj_t *p, ucvm_point_t *geo, 
			  ucvm_point_t *xy)
{

  if ((geo == NULL) || (xy == NULL) || (p == NULL)) {
    return(UCVM_CODE_ERROR);
  }

  xy->coord[0] = geo->coord[0] * DEG_TO_RAD;
  xy->coord[1] = geo->coord[1] * DEG_TO_RAD;

  /* Convert point to proj coords */
  if (pj_transform(p->ipj, p->opj, 1, 1, 
		   &(xy->coord[0]), &(xy->coord[1]), NULL) != 0) {
    fprintf(stderr, "Failed to project point %lf,%lf from geo to xy\n",
	    geo->coord[0], geo->coord[1]);
    return(UCVM_CODE_ERROR);
  }

  xy->coord[0] = xy->coord[0] - p->offset.coord[0];
  xy->coord[1] = xy->coord[1] - p->offset.coord[1];

  rot_point_2d(xy, p->rot);

  if ((xy->coord[0] < 0.0) || (xy->coord[1] < 0.0) || 
      (xy->coord[0] > p->size.coord[0]) || 
      (xy->coord[1] > p->size.coord[1])) {
    return(UCVM_CODE_ERROR);
  }

 /* Copy z value */
 xy->coord[2] = geo->coord[2];

  return(UCVM_CODE_SUCCESS);
}


/* Convert x,y to lon,lat */
int ucvm_proj_ucvm_xy2geo(ucvm_proj_t *p, ucvm_point_t *xy, 
			  ucvm_point_t *geo)
{
  if ((geo == NULL) || (xy == NULL) || (p == NULL)) {
    return(UCVM_CODE_ERROR);
  }

  if ((xy->coord[0] < 0.0) || (xy->coord[1] < 0.0) || 
      (xy->coord[0] > p->size.coord[0]) || 
      (xy->coord[1] > p->size.coord[1])) {
    return(UCVM_CODE_ERROR);
  }

  rot_point_2d(xy, -(p->rot));

  geo->coord[0] = xy->coord[0] + p->offset.coord[0];
  geo->coord[1] = xy->coord[1] + p->offset.coord[1];

  if (pj_transform(p->opj, p->ipj, 1, 1, 
		   &(geo->coord[0]), &(geo->coord[1]), NULL) != 0) {
    fprintf(stderr, "Failed to project point %lf,%lf from xy to geo\n",
	    xy->coord[0], xy->coord[1]);
    return(UCVM_CODE_ERROR);
  }

  /* Convert radians to degrees */
  geo->coord[0] = geo->coord[0] * RAD_TO_DEG;
  geo->coord[1] = geo->coord[1] * RAD_TO_DEG;

  /* Copy z value */
  geo->coord[2] = xy->coord[2];

  return(UCVM_CODE_SUCCESS);
}
