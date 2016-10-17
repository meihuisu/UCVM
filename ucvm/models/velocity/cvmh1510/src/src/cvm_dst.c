/** cvmdst - A simple program to compute distance to velocity interfaces
    cvmdst accepts Geographic Coordinates or UTM Zone 11 coordinates.
7/2007: AP: added distance to interface calculations
**/


#include <string.h>
#include <stdio.h>
#include "proj.h"
#include "cproj.h"
#include "params.h"
#include "coor_para.h"
//GTS includes
#include "gts.h"
#include <stdlib.h>

void gctp ();
char cbuffer;
char lbuffer[80];

int main (int argc, char *argv[])
{
//GTS variables
  GtsSurface * s, * sto, * smo;
  GtsPoint * p, * hit;
  GtsBBox * bb;
  gdouble gdist;
  GtsFile * fp;
  GNode * tree, * totree, * motree;

  int i, n = 0;
  float coor[3];
  double SP[2],SPUTM[2];
  char file1[20]="tt";

  if(argv[1])
    if(!strcmp(argv[1], "-h"))
      {
	printf("     cvmdst4.1 - (c) A. Plesch 2007\n");
	printf("Harvard University\n");
	printf("Compute Distances to Topography, Top Basement and Moho\n");
	printf("     usage: cvmdst < file.in\n");
	printf("cvmdst accepts geographic coordinates and \n");
	printf("UTM Zone 11, WGS84, coordinates in X Y Z columns.\n");
	printf("Output is:\n");
	printf("X Y Z utmX utmY t_x t_y t_z t_dst b_x b_y b_z b_dst m_x m_y m_z m_dst\n");
	printf("The ?_dst numbers are the scalar distances, the ?_xyz numbers are the location of the closest point on the respective surface\n");
	exit(0);
      }

/** load top basement **/

  s = gts_surface_new (gts_surface_class (),
		       gts_face_class (),
		       gts_edge_class (),
		       gts_vertex_class ());

 FILE * f = fopen ("BASE.gts", "r");
 fp = gts_file_new (f);
 if (fp == NULL) {
   fputs ("cvmdist: BASE.gts could not be opened\n",
          stderr);
   return 1; /* failure */
 }
 if (gts_surface_read (s, fp)) {
   fputs ("cvmdist: BASE.gts is not a valid GTS file\n",
          stderr);
   fprintf (stderr, "BASE.gts:%d:%d: %s\n", fp->line, fp->pos, fp->error);
   return 1; /* failure */
 }

 // gts_surface_print_stats (s, stderr);
 gts_file_destroy (fp);

/** load topography **/

  sto = gts_surface_new (gts_surface_class (),
		       gts_face_class (),
		       gts_edge_class (),
		       gts_vertex_class ());

 f = fopen ("BATO.gts", "r");
 fp = gts_file_new (f);

 if (gts_surface_read (sto, fp)) {
   fputs ("cvmdist: BATO.gts is not a valid GTS file\n",
          stderr);
   fprintf (stderr, "BATO.gts:%d:%d: %s\n", fp->line, fp->pos, fp->error);
   return 1; /* failure */
 }

 // gts_surface_print_stats (s, stderr);
 gts_file_destroy (fp);

/** load moho **/

  smo = gts_surface_new (gts_surface_class (),
		       gts_face_class (),
		       gts_edge_class (),
		       gts_vertex_class ());

 f = fopen ("MOHO.gts", "r");
 fp = gts_file_new (f);

 if (gts_surface_read (smo, fp)) {
   fputs ("cvmdist: MOHO.gts is not a valid GTS file\n",
          stderr);
   fprintf (stderr, "MOHO.gts:%d:%d: %s\n", fp->line, fp->pos, fp->error);
   return 1; /* failure */
 }

 // gts_surface_print_stats (s, stderr);
 gts_file_destroy (fp);

/** tree of bounding boxes of all faces of surface **/
 tree = gts_bb_tree_surface(s);
 totree = gts_bb_tree_surface(sto);
 motree = gts_bb_tree_surface(smo);
 

/* let's make points to be reused */

 p = gts_point_new (gts_point_class (), 0, 0, 0);
 hit = gts_point_new (gts_point_class (), 0, 0, 0);

 i=0;
 while (fread(&cbuffer,sizeof(char),1,stdin)==1)
   {lbuffer[i]=cbuffer;
   i++;
   if(cbuffer=='\n')
     {sscanf(lbuffer, "%f %f %f", &coor[0],&coor[1],&coor[2]);
     if(coor[1]<10000000)
       printf("%12.4f %13.4f %9.2f ",coor[0],coor[1],coor[2]);

/* In case we got anything like degrees */

     if ((coor[0]<360.)&&(fabs(coor[1])<90.))
       {
	 for(n=0;n>15;n++) inparm[n]=0;


	 SP[0]=coor[0];
	 SP[1]=coor[1];

	 gctp(SP,&insys,&inzone,inparm,&inunit,&indatum,&ipr,efile,&jpr,file1,
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

      /** compute distance to basement with gts **/

      gts_point_set (p, coor[0], coor[1], coor[2]);

      //      printf ("%f %f %f %f\n", p->x, p->y, p->z, gdist);
      
      gdist = sqrt(gts_bb_tree_point_distance (totree, p, (GtsBBoxDistFunc) gts_point_triangle_distance2, &bb));

      gts_point_triangle_closest (p, GTS_TRIANGLE(bb->bounded), hit);

      printf ("%f %f %f %f ", hit->x, hit->y, hit->z, gdist);

      gdist = sqrt(gts_bb_tree_point_distance (tree, p, (GtsBBoxDistFunc) gts_point_triangle_distance2, &bb));

      //      printf ("bb %f %f %f\n DIST: %f\n",bb->x1,bb->y1,bb->z1,gdist);

      gts_point_triangle_closest (p, GTS_TRIANGLE(bb->bounded), hit);

      printf ("%f %f %f %f ", hit->x, hit->y, hit->z, gdist);

      gdist = sqrt(gts_bb_tree_point_distance (motree, p, (GtsBBoxDistFunc) gts_point_triangle_distance2, &bb));

      gts_point_triangle_closest (p, GTS_TRIANGLE(bb->bounded), hit);

      printf ("%f %f %f %f\n", hit->x, hit->y, hit->z, gdist);

      //      gdist = gts_point_distance2 (p, hit);

      //      printf ("DIST2: %f\n",gdist);

    }
  i=0;
     }
   }
 gts_finalize();
}
