#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cvmsi.h"
#include "cvmsi_utils.h"
#include "cvmsi_geo2xy.h"
#include "vs30_gtl.h"

#include "../model/cvms/cvms.h"

/* Max array size for Z dimension */
#define CVMSI_MAX_ZGRID 3000
#define M_PI 3.14159265358979323846
#define ADD_GTL 0
#define ADD_ALT_GTL 0
#define ADD_ROB_GTL 0
#define ADD_TJ_GTL 1

/* Model config */
int cvmsi_init_flag = 0;
int cvmsi_izone;
int cvmsi_dim[3], cvmsi_pdim[3];
cvmsi_prop_read_t *cvmsi_buf = NULL;
double cvmsi_box[8];
double cvmsi_zgrid[CVMSI_MAX_ZGRID];

/* Version ID */
char cvmsi_version_id[CVMSI_MAX_STR_LEN];


/* Initialize */
int cvmsi_init(const char *dir)
{
  int i, lineno, num_points;
  FILE *ip;
  char line[CVMSI_MAX_STR_LEN], *token;
  double dep;

  char modelpath[CVMSI_MAX_STR_LEN];
  char inputfile[CVMSI_MAX_STR_LEN];
  char gridfile[CVMSI_MAX_STR_LEN];
  char boxfile[CVMSI_MAX_STR_LEN];
  char modelfile[CVMSI_MAX_STR_LEN];
  char verfile[CVMSI_MAX_STR_LEN];
  
  if (dir == NULL) {
    return(1);
  }

  cvmsi_izone = 0;
  strcpy(modelpath, dir);
  sprintf(inputfile, "%s/region_spec.in", modelpath);
  sprintf(gridfile, "%s/XYZGRD", modelpath);
  sprintf(boxfile, "%s/box.dat", modelpath);
  sprintf(modelfile, "%s/cvmsi.bin", modelpath);
  sprintf(verfile, "%s/cvmsi.ver", modelpath);
  for (i = 0; i < 3; i++) {
    cvmsi_dim[i] = 0;
    cvmsi_pdim[i] = 0;
  }

  /* Read in FD_GRID_XYZ input file */
  ip = fopen(inputfile, "r");
  if (ip == NULL) {
    fprintf(stderr, "Failed to open input file %s\n", inputfile);
    return(1);
  }
  lineno = 0;
  while (!feof(ip)) {
    fgets(line, CVMSI_MAX_STR_LEN, ip);
    if (lineno == 1) {
      /* utm zone */
      if (sscanf(line, "%d", &cvmsi_izone) != 1) {
	fprintf(stderr, "Failed to parse utm zone from input file\n");
	return(1);
      }
    }
    lineno++;
  }
  fclose(ip);

  if (cvmsi_izone <= 0) {
    fprintf(stderr, "Failed to parse utm zone from input file\n");
    return(1);
  }

  /* Read in grid file */
  ip = fopen(gridfile, "r");
  if (ip == NULL) {
    fprintf(stderr, "Failed to open grid file %s\n", gridfile);
    return(1);
  }

  lineno = 0;
  while (!feof(ip)) {
    fgets(line, CVMSI_MAX_STR_LEN, ip);
    if (lineno == 0) {
      token = strchr(line, '!');
      if (token != NULL) {
      	token[0] = '\0';
      	if (sscanf(line, "%d %d %d", &(cvmsi_dim[0]), 
      		   &(cvmsi_dim[1]), &(cvmsi_dim[2])) != 3) {
      	  fprintf(stderr, "Failed to parse grid dims from grid file\n");
      	  return(1);
      	}
      } else {
      	fprintf(stderr, "Failed to parse grid dims from grid file\n");
      	return(1);
      }
    } else if (lineno == 2) {
      token = strchr(line, '!');
      if (token != NULL) {
	token[0] = '\0';
	if (sscanf(line, "%d %d %d", &(cvmsi_pdim[0]), 
		   &(cvmsi_pdim[1]), &(cvmsi_pdim[2])) != 3) {
	  fprintf(stderr, "Failed to parse proc dims from grid file\n");
	  return(1);
	}
      } else {
	fprintf(stderr, "Failed to parse proc dims from grid file\n");
	return(1);
      }
    } else if ((lineno >= 3) && (lineno < 7)) {
      if (sscanf(line, "%lf %lf", &(cvmsi_box[(lineno-3)]), 
		 &(cvmsi_box[(lineno-3)+4])) != 2) {
	fprintf(stderr, "Failed to parse box dims %d from grid file\n", 
		lineno-3);
	return(1);
      }
    } else if (lineno >= 7) {
      i = 0;
      if (sscanf(line, "%d %lf", &i, &dep) != 2) {
	fprintf(stderr, "Failed to parse z grid from grid file\n");
	return(1);
      }
      cvmsi_zgrid[i-1] = dep;
    }
    lineno++;
  }
  fclose(ip);

  /* Read in the model file */
  num_points = cvmsi_dim[0]*cvmsi_dim[1]*cvmsi_dim[2];
  cvmsi_buf = malloc(num_points * sizeof(cvmsi_prop_read_t));
  if (cvmsi_buf == NULL) {
    fprintf(stderr, "Failed to allocate model buffer\n");
    return(1);
  }
  ip = fopen(modelfile, "rb");
  if (ip == NULL) {
    fprintf(stderr, "Failed to open model file %s\n", modelfile);
    return(1);
  }
  if (fread(cvmsi_buf, sizeof(cvmsi_prop_read_t), num_points, ip) != num_points) {
    fprintf(stderr, "Failed to read in model file %s\n", modelfile);
    return(1);
  }
  fclose(ip);
	
  /* Swap endian from LSB to MSB if required */
  if (!cvmsi_is_little_endian()) {
    for (i = 0; i < num_points; i++) {
      cvmsi_buf[i].vp = cvmsi_swap_endian_float(cvmsi_buf[i].vp);
      cvmsi_buf[i].vs = cvmsi_swap_endian_float(cvmsi_buf[i].vs);
      //cvmsi_buf[i].rho = cvmsi_swap_endian_float(cvmsi_buf[i].rho);
    }
  }  

  /* Read in version info */
  ip = fopen(verfile, "r");
  if (ip == NULL) {
    fprintf(stderr, "Failed to open version file %s\n", verfile);
    return(1);
  }
  memset(cvmsi_version_id, 0, CVMSI_MAX_STR_LEN);
  if (fgets(cvmsi_version_id, CVMSI_MAX_STR_LEN - 1, ip) == NULL) {
    fprintf(stderr, "Failed to read in version file %s\n", verfile);
    return(1);
  }
  fclose(ip);
  cvmsi_strip_trailing_whitespace(cvmsi_version_id);

  char modeldir[128];
  sprintf(modeldir, "%s/../cvms", modelpath);
  int errcode = 0;

  cvms_init_(modeldir, &errcode);

  char gtldir[256];
  sprintf(gtldir, "%s/cvm_vs30_wills", modelpath);

    if (ADD_GTL) {
        gtl_setup(gtldir);
    }
    
   cvmsi_init_flag = 1;
 
   return(0);
}


/* Finalize */
int cvmsi_finalize()
{
  if (cvmsi_buf != NULL) {
    free(cvmsi_buf);
  }

  cvmsi_init_flag = 0;
  return(0);
}


/* Version ID */
int cvmsi_version(char *ver, int len)
{
  int verlen;

  verlen = strlen(cvmsi_version_id);
  if (verlen > len - 1) {
    verlen = len - 1;
  }
  memset(ver, 0, len);
  strncpy(ver, cvmsi_version_id, verlen);
  return(0);
}


/* Query */
int cvmsi_query(cvmsi_point_t *pnt, cvmsi_data_t *data, int numpts) {
	
	int errcode;
	int x, y, z, i, j, k, offset;
	int counter;
	cvmsi_point_t xyz_f;
	double p[2][3], q_vp[2][2][2], q_vs[2][2][2];

	/* As per Po and En-Jui. Reproduce the starting model. */
	const float min_vp = 2000, corner_vp = 3000;
	const float min_vs = 1000, corner_vs = 1500;
	const float min_rho = 2000, corner_rho = 2300;
	
	const float min_cvms_vs = 100;
	const float min_cvms_vp = 283.637;
	const float min_cvms_rho = 1909.786;

	int flagx = 0;
	
	if (!cvmsi_init_flag) {
		return(1);
	}
	
	// First we need to query CVM-S4 for all the points.
	float *cvms_vp = malloc(numpts * sizeof(float));
	float *cvms_vs = malloc(numpts * sizeof(float));
	float *cvms_rho = malloc(numpts * sizeof(float));
	
	float *cvmslon = malloc(numpts * sizeof(float));
	float *cvmslat = malloc(numpts * sizeof(float));
	float *cvmsdep = malloc(numpts * sizeof(float));
	
	int nn = numpts;
	
	for (i = 0; i < numpts; i++) {
		cvmslon[i] = pnt[i].coord[0];
		cvmslat[i] = pnt[i].coord[1];
		if (ADD_GTL) {
            if (pnt[i].coord[2] < 350) cvmsdep[i] = 350;
            else cvmsdep[i] = pnt[i].coord[2];
        } else {
            cvmsdep[i] = pnt[i].coord[2];
        }
	}
	
	cvms_query_(&nn, cvmslon, cvmslat, cvmsdep, cvms_vp, cvms_vs, cvms_rho, &errcode);
	
	// Now we need to go through the points and interpolate the CVM-S4.26 perturbations.
	for (counter = 0; counter < numpts; counter++) {
		cvmsi_data_t *curData = malloc(sizeof(cvmsi_data_t));
		
		curData->xyz.coord[0] = -1;
		curData->xyz.coord[1] = -1;
		curData->xyz.coord[2] = -1;
	
		curData->prop.vp = 0;
		curData->prop.diff_vp = 0;
		curData->prop.vs = 0;
		curData->prop.diff_vs = 0;
		curData->prop.rho = 0;
        
        double cvmsdepptr = (double)cvmsdep[counter];
        double cvmslatptr = (double)cvmslat[counter];
        double cvmslonptr = (double)cvmslon[counter];
	
		/* Convert point from geo to xy */
		cvmsi_geo2xy_(cvmsi_dim, cvmsi_box, cvmsi_zgrid, 
					  &cvmslatptr, &cvmslonptr,
					  &cvmsdepptr, &(xyz_f.coord[0]),
					  &cvmsi_izone, &errcode);

		if (errcode == 0) {

			/* Save Chen's integer grid coordinates /w origin at 0,0,0 */
			curData->xyz.coord[0] = round(xyz_f.coord[0]);
			curData->xyz.coord[1] = round(xyz_f.coord[1]);
			curData->xyz.coord[2] = round(xyz_f.coord[2]);

			/* Determine p */
			p[0][0] = 0.0;    
			p[0][1] = 0.0;
			p[0][2] = 0.0;
			p[1][0] = 1.0;    
			p[1][1] = 1.0;
			p[1][2] = 1.0;

			/* Determine q */
			for (z = 0; z < 2; z++) {
				for (y = 0; y < 2; y++) {
					for (x = 0; x < 2; x++) {
						i = (int)(xyz_f.coord[0]) + x;
						j = (int)(xyz_f.coord[1]) + y;
						k = (int)(xyz_f.coord[2]) + z;

						if (i >= cvmsi_dim[0]) {
							i = (int)(xyz_f.coord[0]);
						}
						if (j >= cvmsi_dim[1]) {
							j = (int)(xyz_f.coord[1]);
						}
						if (k >= cvmsi_dim[2]) {
							k = (int)(xyz_f.coord[2]);
						}

						/* Query the model */
						offset = (k * cvmsi_dim[0] * cvmsi_dim[1]) + \
						(j * cvmsi_dim[0]) + i;

						// For Debug Purposes
						//printf("Cell i: %u, j: %u, k: %u, vp: %f, vs: %f\n", i + 1, j + 1, k + 1, 
						//cvmsi_buf[offset].vp, cvmsi_buf[offset].vs);

						q_vp[z][y][x] = cvmsi_buf[offset].vp;
						q_vs[z][y][x] = cvmsi_buf[offset].vs;
					}
				}
			}

			/* Perform trilinear interpolation */
			xyz_f.coord[0] = xyz_f.coord[0] - (int)(xyz_f.coord[0]);
			xyz_f.coord[1] = xyz_f.coord[1] - (int)(xyz_f.coord[1]);
			xyz_f.coord[2] = xyz_f.coord[2] - (int)(xyz_f.coord[2]);
	  
			curData->prop.diff_vp = cvmsi_interp_trilinear(xyz_f.coord[0], 
														   xyz_f.coord[1],
														   xyz_f.coord[2],
														   p, q_vp);
    
			curData->prop.diff_vs = cvmsi_interp_trilinear(xyz_f.coord[0], 
														   xyz_f.coord[1],
														   xyz_f.coord[2],
														   p, q_vs);
					
			/* Make sure our returned CVM-S values are at least the minimum they found. */
			float cvms_vs_calc = cvms_vs[counter];
			float cvms_vp_calc = cvms_vp[counter];
			float cvms_rho_calc = cvms_rho[counter];

			if (cvms_vs_calc < min_cvms_vs) cvms_vs_calc = min_cvms_vs;
			if (cvms_vp_calc < min_cvms_vp) cvms_vp_calc = min_cvms_vp;
			if (cvms_rho_calc < min_cvms_rho) cvms_rho_calc = min_cvms_rho;
			
			/* Modify if need be. */
			if (cvms_vp_calc < corner_vp)
				cvms_vp_calc = (corner_vp * (min_vp - min_cvms_vp) + cvms_vp_calc * (corner_vp - min_vp)) / (corner_vp - min_cvms_vp);
			if (cvms_vs_calc < corner_vs)
				cvms_vs_calc = (corner_vs * (min_vs - min_cvms_vs) + cvms_vs_calc * (corner_vs - min_vs)) / (corner_vs - min_cvms_vs);
			if (cvms_rho_calc < corner_rho)
				cvms_rho_calc = (corner_rho * (min_rho - min_cvms_rho) + cvms_rho_calc * (corner_rho - min_rho)) / (corner_rho - min_cvms_rho);

            /* Fix negative Lambda */
			if (cvms_vp_calc / cvms_vs_calc < 1.45) cvms_vs_calc = cvms_vp_calc / 1.45;
            

            if (ADD_ALT_GTL) {
                // Check if the starting model Vs is different than CVM-S4 Vs. Apply the directionality test to determine if we apply
                // the perturbations.
                if ((cvms_vs[counter] < cvms_vs_calc && curData->prop.diff_vs < 0) || (cvms_vs[counter] > cvms_vs_calc && curData->prop.diff_vs > 0)) {
                    // Just trying to get back so use original Vs.
                    curData->prop.vs = cvms_vs[counter];
				} else {
                    curData->prop.vs = cvms_vs_calc + curData->prop.diff_vs;
                }

                if ((cvms_vp[counter] < cvms_vp_calc && curData->prop.diff_vp < 0) || (cvms_vp[counter] > cvms_vp_calc && curData->prop.diff_vp > 0)) {
                    // Just trying to get back so use original Vp.
                    curData->prop.vp = cvms_vp[counter];
                } else {
                    curData->prop.vp = cvms_vp_calc + curData->prop.diff_vp;
                }
                
                curData->prop.rho = cvms_rho_calc;
                
	     } else if (ADD_TJ_GTL) {
				
				// Check to see if the CVM-S4 value is less than the floor value of 1000Vs.
				if (cvms_vs[counter] < 1000) {
					// Now we want to see if the perturbed model would be lower than the CVM-S Vs. If it would be, we use CVM-S Vs.
					if (cvms_vs[counter] + curData->prop.diff_vs < cvms_vs[counter]) {
						curData->prop.vs = cvms_vs[counter];
						curData->prop.vp = cvms_vp[counter];
						curData->prop.rho = cvms_rho[counter];
					} else {
						curData->prop.vs = cvms_vs[counter] + curData->prop.diff_vs;
						curData->prop.vp = cvms_vp[counter] + curData->prop.diff_vp;
						curData->prop.rho = cvms_rho[counter];	

						if (curData->prop.vp / curData->prop.vs < 1.45) {
							curData->prop.vp = curData->prop.vs * 1.45;
						}
					
					}

				} else {
					curData->prop.vs = cvms_vs_calc + curData->prop.diff_vs;
					curData->prop.vp = cvms_vp_calc + curData->prop.diff_vp;
					curData->prop.rho = cvms_rho_calc;					
				}
				
			} else { 
                curData->prop.vs = cvms_vs_calc; // + curData->prop.diff_vs;
                curData->prop.vp = cvms_vp_calc; // + curData->prop.diff_vp;
                curData->prop.rho = cvms_rho_calc;
            }

            /* Fix negative Lambda */
			//if (curData->prop.vp / curData->prop.vs < 1.45) curData->prop.vs = curData->prop.vp / 1.45;
            
		} else {
            
            // Just use whatever CVM-S returns. Outside of CVM-S4.23 perturbation boundaries.
            curData->prop.vp = cvms_vp[counter];
            curData->prop.vs = cvms_vs[counter];
            curData->prop.rho = cvms_rho[counter];

        }

        /* Now add in the GTL if we want it. */
        if (ADD_GTL) {
	    gtl_entry_t *entry = malloc(sizeof(gtl_entry_t));
            
            /* Convert lat, lon to UTM. */
            double *utm_east = malloc(sizeof(double));
            double *utm_north = malloc(sizeof(double));
            int utm_proj_zone = 11;
            int iway = 0;
            utm_geo_(&(pnt[counter].coord[0]), &(pnt[counter].coord[1]), utm_east, utm_north, &utm_proj_zone, &iway);
            
            entry->coor_utm[0] = *utm_east;
            entry->coor_utm[1] = *utm_north;
            entry->coor_utm[2] = -350.0f; //pnt[counter].coord[2];
            entry->depth = pnt[counter].coord[2];
            entry->topo_gap = 0.0f;
            entry->vp = curData->prop.vp;
            entry->vs = curData->prop.vs;
            entry->rho = curData->prop.rho;
            
            int updated = 0;
            
            gtl_interp(entry, &updated);

            curData->prop.vs = entry->vs;
            curData->prop.vp = entry->vp;
            curData->prop.rho = entry->rho;
        }
        
        memcpy(&data[counter], curData, sizeof(cvmsi_data_t));
        free(curData);
	}

  return(0);
}
