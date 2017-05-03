subroutine findnode(gridfile,slat,slon,sdep,coords,ns,box,izone)
  implicit none
  integer ILONGLAT2UTM,IUTM2LONGLAT,izone
  parameter(ILONGLAT2UTM=0,IUTM2LONGLAT=1)
  integer, dimension(3) :: dims,coords,ns
  double precision :: slat,slon,sdep, dzmin
  double precision :: dlat,dlon,xx,yy,difx, e, n
  double precision, dimension(3000) :: xgrid,ygrid,zgrid
  double precision, dimension(4,2) :: box
  character (len=200) :: gridfile
  integer ix, iy, iz, nxs, nys, nzs, mxs, mys, mzs, nxst, nyst, nzst,nx,ny,nz
  integer ii, i, nxt, nyt, nzt
  
  dlat=dble(slat)
  dlon=dble(slon)
  call utm_geo(dlon,dlat,xx,yy,izone,ILONGLAT2UTM)

  !   Read info defining the FD grid.

  open(10,file=gridfile,status='old')
  read(10,*) nx,ny,nz
  read(10,*) (dims(i),i=1,3)
  read(10,*) nxt,nyt,nzt
  read(10,*) box(1,1),box(1,2)
  read(10,*) box(2,1),box(2,2)
  read(10,*) box(3,1),box(3,2)
  read(10,*) box(4,1),box(4,2)
  do iz=1,nz
     read(10,*) ii,zgrid(ii)
  end do
  close(10)

! Find out the source node numbers in the global grid.  
  call utm2grd(nxs,nys,xx,yy,box,nx,ny)
  if (nxs.lt.1.or.nxs.gt.nx.or.nys.lt.1.or.nys.gt.ny.or.sdep.gt.-zgrid(1)/1e3) then
     print*, 'receiver located outside model box!'
     print*, 'receiver gird: ', nxs, nys, sdep
     print*, 'modeling grid: ', nx, ny, -zgrid(1)/1e3
  endif
  dzmin=10000
  if (sdep.le.0.0) then
     nzs=nz
  else
     do iz=1,nz
        if (abs(abs(zgrid(iz))-abs(sdep)*1e3).le.dzmin) then
           dzmin=abs(abs(zgrid(iz))-abs(sdep)*1e3)
           nzs=iz
        endif
     enddo
  endif

  call grd2utm(e,n,nxs,nys,box,nx,ny)
  call utm_geo(dlon,dlat,e,n,izone,IUTM2LONGLAT)

! calling utm_geo back and forth will cause a difference in northing of about 12 m !!!!!!!
! strange !!!!

!  print*, dlon, dlat
!  call utm_geo(dlon,dlat,e,n,izone,ILONGLAT2UTM)
!  print*, e, n
!  call utm_geo(dlon,dlat,e,n,izone,IUTM2LONGLAT)
!  print*, dlon, dlat
!  call utm_geo(dlon,dlat,e,n,izone,ILONGLAT2UTM)
!  print*, e, n
!  stop
!   Find out the source chunk numbers.

  mxs=(nxs-1)/nxt
  mys=(nys-1)/nyt
  mzs=(nzs-1)/nzt
  
!   Find out the source node numbers in the local grid.

  nxst=nxs-mxs*nxt
  nyst=nys-mys*nyt
  nzst=nzs-mzs*nzt
  
  write(*,*) ''	
  write(*,*) '   Number of chunks (XYZ):    ',dims(1),dims(2),dims(3)
  write(*,*) '   Total grid numbers (XYZ):  ',nx,ny,nz
  write(*,*) '   Grids per chunk (XYZ):     ',nxt,nyt,nzt
  write(*,*) ''
  write(*,'(a,5(4x,e20.12))') '    Input receiver location:     ', xx,yy,sdep*1e3,slon,slat
  write(*,'(a,5(4x,e20.12))') '    Used receiver location:      ', e,n,-zgrid(nzs),dlon,dlat
  write(*,'(a,3(4x,e20.12))') '    Location differences (m):    ',abs(e-xx),abs(n-yy),abs(-zgrid(nzs)-sdep*1e3)
  write(*,*) '   Receiver grid (global):      ',nxs,nys,nzs
  write(*,*) '   Receiver chunk (0- ):        ',mxs,mys,mzs
  write(*,*) '   Receiver grid input (local): ',nxst,nyst,nzst
  
  coords(1)=mxs
  coords(2)=mys
  coords(3)=mzs
  
  ns(1)=nxst
  ns(2)=nyst
  ns(3)=nzst
  
  return
end subroutine findnode
!
!
