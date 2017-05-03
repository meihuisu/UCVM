program utm2lonlat
  implicit none
  integer, parameter :: IUTM2LONGLAT = 1
  integer UTM_PROJECTION_ZONE

  double precision, dimension(:), allocatable:: e, n, lon, lat
  character(len=200) :: inname
  integer :: NP, ii

  call getarg(1,inname)
  open(11,file=trim(inname),form='formatted')
  read(11,*) UTM_PROJECTION_ZONE
  read(11,*) NP
  allocate(e(1:NP))
  allocate(n(1:NP))
  allocate(lon(1:NP))
  allocate(lat(1:NP))
  do ii=1,NP
     read(11,*) e(ii), n(ii)
     call utm_geo(lon(ii),lat(ii),e(ii),n(ii),UTM_PROJECTION_ZONE,IUTM2LONGLAT)
     print*, lon(ii), lat(ii)
  enddo
  close(11)
end program utm2lonlat
