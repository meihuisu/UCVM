c  include file soil1.h
c soil type info
        parameter (numsoil=6, isoilbig=1000000,inums=15)
        common /soil/ rlatmax,rlatmin,rlonmax,rlonmin,nx,ny,
     1  isb(isoilbig),rdelx,rdely
        dimension igrey(numsoil),isoil(numsoil) 
c below is 'soil.pgm'
         data (igrey(i),i=1,numsoil) /200,213,167,71,26,141/
c below is 'soil2.pgm'
c       data (igrey(i),i=1,numsoil) /90,59,180,30,120,149/
        data (isoil(i),i=1,numsoil) /1,2,4,5,7,8/
