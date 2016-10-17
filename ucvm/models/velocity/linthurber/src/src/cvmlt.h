#ifndef CVMLT_H
#define CVMLT_H


/* Constants */
#define CVMLT_MAX_STR_LEN 256


/* Point structure */
typedef struct cvmlt_point_t {
  double coord[3];
} cvmlt_point_t;


/* Return data structure */
typedef struct cvmlt_data_t {
  float vp;
  float vs;
  float rho;
} cvmlt_data_t;


/* Initialize */
int cvmlt_init(const char *dir);

/* Finalize */
int cvmlt_finalize();

/* Version ID */
int cvmlt_version(char *ver, int len);

/* Query */
int cvmlt_query(cvmlt_point_t *pnt, cvmlt_data_t *data);


#endif
