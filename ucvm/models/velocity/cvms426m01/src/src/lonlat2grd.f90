program lonlat2grd
  implicit none
  
  character*200 inname, fnam, xyzfile, outname
  character*100 netid, staid
  integer N, ii, nxt, nyt, nzt, nx, ny, nz, xg, yg, zg
  double precision lon, lat, dep, fdump
  integer, dimension(3) :: coords, ns, dims
  double precision, dimension(4,2)::box
  integer izone

  call getarg(1,inname)
  open(11,file=inname,form='formatted')
  read(11,'(a)') xyzfile
  print*, xyzfile
  read(11,*) izone
  read(11,'(a)') fnam
  print*, fnam
  read(11,'(a)') outname
  print*, outname
  read(11,*) N
  print*, N
  close(11)

  open(11,file=xyzfile,form='formatted')
  read(11,*) nx, ny, nz
  print*, nx, ny, nz
  read(11,*) (dims(ii),ii=1,3)
  print*, dims
  read(11,*) nxt, nyt, nzt
  print*, nxt, nyt, nzt
  close(11)

  open(11,file=fnam,form='formatted')
  open(12,file=outname,form='formatted')
  open(13,file=trim(outname)//'.miss',form='formatted')

  do ii=1,N
     read(11,*) lat, lon, dep
     print*, '**********************************************'
     print*, ii, lon, lat, dep
     print*, '**********************************************'

     call findnode(xyzfile,lat,lon,dep,coords,ns,box,izone)
     xg=coords(1)*nxt+ns(1)
     yg=coords(2)*nyt+ns(2)
     zg=coords(3)*nzt+ns(3)
     if (xg.ge.1.and.xg.le.nx.and.yg.ge.1.and.yg.le.ny.and.zg.ge.1.and.zg.le.nz) then
        write(12,*) ii-1, xg, yg, zg
     else
        write(13,*) ii-1
     endif
  enddo
  close(11)
  close(12)
  close(13)
end program lonlat2grd
