program FD_GRID_XYZ_rect

! generate the FD grid for a rectangular area
! lon1, lat1: point1 (southwest point of the rectangular area)
! lon2, lat2: point2 (northwest point of the rectangular area)
! lon3, lat3: point3 (northeast point of the rectangular area)
! lon4, lat4: point4 (southeast point of the rectangular area)
! lx: distance between point1 and point4
! ly: distance between point1 and point2

! input: lon1, lat1, lon2, lat2, lx, dep, grdsize
! output: box.dat (specify the longitude/latitude of the four cornors of the box)
! output: XYZGRD (the grid)

! to compile: mpif90 -o FD_GRID_XYZ_rect FD_GRID_XYZ_rect.f90
! to run: FD_GRID_XYZ_rect FD_GRID_XYZ_rect.inp

! Po Chen
! 2006-11-26, Lamont

  implicit none
  integer ILONGLAT2UTM, IUTM2LONGLAT, izone
  parameter(ILONGLAT2UTM=0, IUTM2LONGLAT=1)
  character*200 grdpath, grdfile,inname
  real*8 lon1, lon2, lon3, lon4, lat1, lat2, lat3, lat4, lx, ly, dep, grdsize
  real*8 e1, n1, e2, n2, e3, n3, e4, n4, tot_dim, tot_ram_mb, e, n, e2t, n2t
  integer nx, ny, nz, nxt, nyt, nzt, nd, ix, iy, iz
  integer, dimension(3) :: dims
  double precision, dimension(4,2) :: box

  call getarg(1,inname)

  write(*,'(/)')
  open(11,file=inname,form='formatted')
  read(11,'(a)') grdpath
  print*, 'grid path: ', trim(grdpath)
  read(11,*) izone
  print*, 'UTM zone: ', izone
  read(11,*) lon1, lat1
  print*, '1. longitude, latitude: ', lon1, lat1
  read(11,*) lon2, lat2
  print*, '2. longitude, latitude: ', lon2, lat2
  read(11,*) lx
  print*, 'length: ', lx, ' (m)'
  read(11,*) dep
  print*, 'depth: ', dep, ' (m)'
  read(11,*) grdsize
  print*, 'grid size: ', grdsize, ' (m)'
  read(11,*) dims(1),dims(2),dims(3)
  print*, 'dims: ', dims
  close(11)

  call utm_geo(lon1,lat1,e1,n1,izone,ILONGLAT2UTM)
  call utm_geo(lon2,lat2,e2,n2,izone,ILONGLAT2UTM)
  ly=sqrt((e2-e1)**2+(n2-n1)**2)

  print*, ly
  nx=nint(lx/grdsize)+1
  ny=nint(ly/grdsize)+1
  nz=nint(dep/grdsize)+1
  print*, nx, ny, nz
  nx=nx-mod(nx,dims(1))+dims(1)
  ny=ny-mod(ny,dims(2))+dims(2)
  nz=nz-mod(nz,dims(3))+dims(3)
  print*, nx, ny, nz
  nxt=nx/dims(1)
  nyt=ny/dims(2)
  nzt=nz/dims(3)
  print*, nxt, nyt, nzt

  lx=(nx-1)*grdsize
  ly=(ny-1)*grdsize
  print*, lx, ly
  dep=(nz-1)*grdsize
  print*, 'e1,n1: ', e1,n1
  print*, 'e2,n2: ', e2,n2
  print*, (e2-e1), ly, sqrt((e2-e1)**2+(n2-n1)**2), e1
  e2t=(e2-e1)*ly/sqrt((e2-e1)**2+(n2-n1)**2)+e1
  n2t=(n2-n1)*ly/sqrt((e2-e1)**2+(n2-n1)**2)+n1
  e2 = e2t
  n2 = n2t
  print*, 'e1,n1: ', e1,n1
  print*, 'e2,n2: ', e2,n2

  e4=(n2-n1)*lx/ly+e1
  n4=(e1-e2)*lx/ly+n1
  e3=(n2-n1)*lx/ly+e2
  n3=(e1-e2)*lx/ly+n2

  print*, sqrt((e2-e1)**2+(n2-n1)**2)
  print*, sqrt((e3-e2)**2+(n3-n2)**2)

  write(*,'(/)')
  print*, 'Global grid # nx, ny, nz: ', nx, ny, nz
  print*, 'Total global grid #: ', nx*ny*nz
  print*, 'Local grid # nxt, nyt, nzt: ', nxt, nyt, nzt
  print*, 'Total local grid #: ', nxt*nyt*nzt
  write(*,'(/)')

!   Estimate the RAM required to run FD simulations, assuming nd=12.

  nd=12
  tot_dim=20.*real(nxt+4)*real(nyt+4)*real(nzt+4)+18.*real(nd)*real(nxt*nyt+2*nxt*nzt+2*nyt*nzt)
  tot_ram_mb=4.*tot_dim/(1024.**2)
  write(*,'(a,f10.4/)') 'Estimated RAM for FD simulation (MB): ',tot_ram_mb

  print*, 'four corners (clockwise), easting and northing:'
  print*, e1, n1
  print*, e2, n2
  print*, e3, n3
  print*, e4, n4
  box(1,1)=e1
  box(1,2)=n1
  box(2,1)=e2
  box(2,2)=n2
  box(3,1)=e3
  box(3,2)=n3
  box(4,1)=e4
  box(4,2)=n4
  call utm_geo(lon3,lat3,e3,n3,izone,IUTM2LONGLAT)
  call utm_geo(lon4,lat4,e4,n4,izone,IUTM2LONGLAT)

  write(*,'(/)')
  print*, 'box dimensions:'
  print*, lx/1e3, ' (km)'
  print*, ly/1e3, ' (km)'
  print*, dep/1e3, ' (km)'

  write(*,'(/)')
  print*, 'four corners (clockwise), longitude and latitude:'
  print*, lon1, lat1
  print*, lon2, lat2
  print*, lon3, lat3
  print*, lon4, lat4
  open(11,file=trim(grdpath)//'box.dat',form='formatted')
  write(11,*) lon1, lat1
  write(11,*) lon2, lat2
  write(11,*) lon3, lat3
  write(11,*) lon4, lat4
  write(11,*) lon1, lat1
  close(11)

  open(1,file=trim(grdpath)//'XYZGRD',form='formatted')
  write(1,'(3(1x,i5),6x,a)') nx,ny,nz,'! Global grid numbers NX, NY, NZ'
  write(1,'(3(1x,i5),6x,a)') dims(1),dims(2),dims(3),'! Chunks (#of proc.) in X, Y, Z'
  write(1,'(3(1x,i5),6x,a)') nxt,nyt,nzt,'! Grid numbers per chunk NXT, NYT, NZT'
  write(1,*) e1, n1
  write(1,*) e2, n2
  write(1,*) e3, n3
  write(1,*) e4, n4
!  do ix=1,nx
!     write(1,*) ix,e1+(ix-1)*grdsize*(e4-e1)/lx, n1+(ix-1)*grdsize*(n4-n1)/lx
!  end do
!  do iy=1,ny
!     write(1,*) iy,e1+(iy-1)*grdsize*(e2-e1)/ly, n1+(iy-1)*grdsize*(n2-n1)/ly
!  end do
  do iz=nz,1,-1
     write(1,*) iz,-dble(nz-iz)*grdsize
  end do
  close(1)

  write(*,'(/)')
  call grd2utm(e,n,1,1,box,nx,ny)
  print*, e, n
  call grd2utm(e,n,1,ny,box,nx,ny)
  print*, e, n
  call grd2utm(e,n,nx,ny,box,nx,ny)
  print*, e, n
  call grd2utm(e,n,nx,1,box,nx,ny)
  print*, e, n

  write(*,'(/)')
  call utm2grd(ix,iy,e1,n1,box,nx,ny)
  print*, ix,iy
  call utm2grd(ix,iy,e2,n2,box,nx,ny)
  print*, ix,iy
  call utm2grd(ix,iy,e3,n3,box,nx,ny)
  print*, ix,iy
  call utm2grd(ix,iy,e4,n4,box,nx,ny)
  print*, ix,iy

end program FD_GRID_XYZ_rect
