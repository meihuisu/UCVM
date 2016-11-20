! in.h - memory usage is ibig*38*4 bytes
      include 'params.h'
      common /cvmsoi/ rlat, rlon, rdep, alpha, beta, rho, nn, nnl
      real :: rlat(ibig), rlon(ibig), rdep(ibig), alpha(ibig),
     $  beta(ibig), rho(ibig)
      integer :: nn, nnl
