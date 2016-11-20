c
c ASCII I/O for SCEC CVM
c
c Bug fixes and modifications for binary and MPI I/O. Ely 2007/9/1
c Moved z value unit conversion to cvms_sub.f.        PES 2011/07/21
c Added help message                                  PES 2011/11/09
c

      subroutine readpts( kerr )
      implicit none
      include 'in.h'
      integer :: kerr, i
      character(64) arg

      arg=''
      nn=0
      call getarg(1,arg)
      if (arg.eq.'-help'.or.arg.eq.'--help'.or.arg.eq.'-h') then
         write( 0, * ) 'Format of stdin input:'
         write( 0, * ) '  line 1    : number of points'
         write( 0, * ) '  line 2-n+1: lat (dd), lon (dd), dep (m)'
         write( 0, * ) 'Format of stdout output: '
         write( 0, * ) '  line 1    : Program version'
         write( 0, * ) '  line 2-n+1: lat, lon, dep, vp, vs, rho'
      else
         read( *, * ) nn
      end if
      if ( nn.eq.0 ) then
         stop
      end if
      if ( nn.gt.ibig ) then
         print *, 'Error: nn greater than ibig', nn, ibig
         stop
      end if
      do i = 1, nn
        read( *, * ) rlat(i), rlon(i), rdep(i)
        if( rdep(i) .lt. 0 ) write( 0, * )
     $    'Error: degative depth', i, rlon(i), rlat(i), rdep(i)
        if(rlon(i)/=rlon(i).or.rlat(i)/=rlat(i).or.rdep(i)/=rdep(i))
     $    write( 0, * ) 'Error: NaN', i, rlon(i), rlat(i), rdep(i)
c        rdep(i) = rdep(i) * 3.2808399
        if( rdep(i) .lt. rdepmin ) rdep(i) = rdepmin
      end do
      kerr = 0
      end

      subroutine writepts( kerr )
      implicit none
      include 'in.h'
      integer :: kerr, i
      do i = 1, nn
c        rdep(i) = rdep(i) / 3.2808399
        if(rho(i)/=rho(i).or.alpha(i)/=alpha(i).or.beta(i)/=beta(i))
     $    write( 0, * ) 'Error: NaN', i, rlon(i), rlat(i), rdep(i)
        write( *, '(f8.5,1x,f10.5,1x,f9.2,1x,f8.1,1x,f8.1,1x,f8.1)' )
     $    rlat(i), rlon(i), rdep(i), alpha(i), beta(i), rho(i)
      end do
      kerr = 0
      end

