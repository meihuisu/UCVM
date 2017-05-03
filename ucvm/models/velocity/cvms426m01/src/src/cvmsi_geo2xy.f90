subroutine cvmsi_geo2xy(dims,box,zgrid,slat,slon,sdep,coords,izone,errcode)
  implicit none
  integer ILONGLAT2UTM,IUTM2LONGLAT,izone,errcode
  parameter(ILONGLAT2UTM=0,IUTM2LONGLAT=1)
!  integer, dimension(3) :: coords
  double precision, dimension(3) :: coords
  double precision :: slat, slon, sdep, dzmin
  double precision :: dlat, dlon, xx, yy, spacing
  double precision, dimension(3000) :: zgrid
  double precision, dimension(4,2) :: box
  integer iz, nx, ny, nz
  integer, dimension(3) :: dims
  double precision :: nxs, nys, nzs

  coords(1)=-1
  coords(2)=-1
  coords(3)=-1
  
  dlat=dble(slat)
  dlon=dble(slon)
  call utm_geo(dlon,dlat,xx,yy,izone,ILONGLAT2UTM)

  nx = dims(1)
  ny = dims(2)
  nz = dims(3)
  
  spacing = abs(zgrid(1)) - abs(zgrid(2))

! Find out the source node numbers in the global grid.  
  call utm2grd_d(nxs,nys,xx,yy,box,nx,ny)
  if (nxs.lt.1.or.nxs.gt.nx.or.nys.lt.1.or.nys.gt.ny) then
!     print*, 'receiver located outside model box!'
!     print*, 'geo location : ', slon, slat, sdep
!     print*, 'receiver grid: ', nxs, nys, sdep
!     print*, 'modeling grid: ', nx, ny, -zgrid(1)
     errcode = 1
     return
  endif
  if ((sdep.lt.0.0).or.(sdep.ge.abs(zgrid(1))+spacing)) then
     errcode = 1
     return
  else
     if ((sdep.ge.abs(zgrid(1))).and.(sdep.lt.abs(zgrid(1))+spacing)) then
        nzs = 1
     else
        sdep = abs(sdep)
        do iz=2,nz
           if ((sdep.ge.abs(zgrid(iz))).and.(sdep.lt.abs(zgrid(iz-1)))) then
              nzs = (iz-1) + abs(sdep-abs(zgrid(iz-1)))/abs(abs(zgrid(iz))-abs(zgrid(iz-1)))
              goto 1
           endif
        enddo
     endif
  endif

1 coords(1)=nxs-1
  coords(2)=nys-1
  coords(3)=nzs-1

  errcode = 0
  return
end subroutine cvmsi_geo2xy
!
!
