#ifndef SCEC_1D_H
#define SCEC_1D_H

/* 1D CVM from SCEC CVM-4 */
double scec_vp(double depth);
double scec_rho(double vp);
double scec_vs(double vp, double rho);

#endif
