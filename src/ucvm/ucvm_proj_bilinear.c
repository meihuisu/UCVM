#include <stdio.h>
#include "ucvm_utils.h"
#include "ucvm_proj_bilinear.h"


/* Bilinear interpolation parameters */
double csii[] =  {-1.0, -1.0, 1.0,  1.0 };
double ethai[] = {-1.0,  1.0, 1.0, -1.0 };


/* Convert lon,lat to x,y */
int ucvm_bilinear_geo2xy(ucvm_bilinear_t *par,
                         ucvm_point_t *geo, ucvm_point_t *xy)
{
  int i, k=0;
  double x=0, y=0, x0, y0, dx, dy;
  double j[4], j1[4], j2[4], jinv[4];
  double xce=0, yce=0;
  double res=1, d, p, q;
  
  j1[0] = 0;
  j1[1] = 0;
  j1[2] = 0;
  j1[3] = 0;
  
  for(i=0; i<4; i++){
    j1[0] += par->xi[i] * csii[i];
    j1[1] += par->xi[i] * ethai[i];
    j1[2] += par->yi[i] * csii[i];
    j1[3] += par->yi[i] * ethai[i];
    xce += par->xi[i] * csii[i] * ethai[i];
    yce += par->yi[i] * csii[i] * ethai[i];
  }
  
  do {
    k++;
    
    j2[0] = y * xce;
    j2[1] = x * xce;
    j2[2] = y * yce;
    j2[3] = x * yce;
    
    j[0] = .25 * (j1[0] + j2[0]);
    j[1] = .25 * (j1[1] + j2[1]);
    j[2] = .25 * (j1[2] + j2[2]);
    j[3] = .25 * (j1[3] + j2[3]);
    
    d = (j[0]*j[3]) - (j[2]*j[1]);
    jinv[0] =  j[3] / d;
    jinv[1] = -j[1] / d;
    jinv[2] = -j[2] / d;
    jinv[3] =  j[0] / d;
    
    x0 = 0;
    y0 = 0;
    
    for(i=0; i<4; i++){
      x0 += par->xi[i] * (.25 * (1 + (csii[i]  * x))
			  * (1 + (ethai[i] * y)));
      y0 += par->yi[i] * (.25 * (1 + (csii[i]  * x))
			  * (1 + (ethai[i] * y)));
    }
    
    p = geo->coord[0] - x0;
    q = geo->coord[1] - y0;
    dx = (jinv[0]*p) + (jinv[1]*q);
    dy = (jinv[2]*p) + (jinv[3]*q);
    
    x += dx;
    y += dy;
    
    res = dx*dx + dy*dy;
    
  } while(res > 1e-12 && k<10);
  
  if(k>=10){
    //fprintf(stderr,"Unable to convert %lf %lf\n",lat,lon);
    return(UCVM_CODE_ERROR);
  }
  
  x = (x + 1) * par->dims[0]/2.0;
  y = (y + 1) * par->dims[1]/2.0;
  
  xy->coord[0] = x;
  xy->coord[1] = y;
  
  return(UCVM_CODE_SUCCESS);
}


/* Convert x,y to lon,lat */
int ucvm_bilinear_xy2geo(ucvm_bilinear_t *p,
                         ucvm_point_t *xy, ucvm_point_t *geo)
{
  // Latitudes of ShakeOut region corners
  geo->coord[1] = interpolate_bilinear(xy->coord[0], xy->coord[1],
			      0, 0, p->dims[0], p->dims[1],
			      p->yi[0], p->yi[3], p->yi[1], p->yi[2]);
  
  // Longitudes of ShakeOut region corners
  geo->coord[0] = interpolate_bilinear(xy->coord[0], xy->coord[1],
			      0, 0, p->dims[0], p->dims[1],
			      p->xi[0], p->xi[3], p->xi[1], p->xi[2]);

  return(UCVM_CODE_SUCCESS);
}



