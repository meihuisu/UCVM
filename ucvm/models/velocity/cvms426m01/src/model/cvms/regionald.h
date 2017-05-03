c regionald.h
c more regional model info
c layer depths in m
c-- these parameters deal with searching over boxes of regional model
         parameter(ninrow=27, nbox=5)
         dimension nearn(nbox-1),rboxla(nbox),rboxlo(nbox)
         dimension x22(nbox),y22(nbox)
c layer depths in feet
         data(reglay(k),k=1,nregly)/3280.04,16404.20,19685.04,32808.40,
     1   50853.02,54133.86,72178.48,101706.04,108267.72/
c 1d regional layer velocities in m/s
         data(reg1dv(l),l=1,nregly)/5000.,5500.,6300.,6300.,6400.,
     1   6700.,6750.,6800.,7800./
