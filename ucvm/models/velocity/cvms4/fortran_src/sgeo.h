c  sgeo.h   contains info for surface geology
c ngeo  number surface geo contours
c ngeo2   max number pts in geo contours
c age in years
         parameter (ngeo=52,ngeo2=240)
         common /rgeo/ rlai(ngeo,ngeo2),rloi(ngeo,ngeo2),ra(ngeo),
     1   np(ngeo)
