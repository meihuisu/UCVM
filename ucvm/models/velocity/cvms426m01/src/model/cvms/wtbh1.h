c include file wtbh.h
c contains info to weight contributions of geotech boreholes
c radii  = radius (km) of tori for weighting profiles
         parameter (nrad=4) 
         common /radical/radii(nrad),iradct(nrad),iradbh(nrad,numbh),
     1   radwt(nrad)
