subroutine utm2grd(ix,iy,e,n,box,nx,ny)
! convert utm coordinates e, n to grid number ix, iy
! given the four corners of the rectangle in box
! and the total global grid number nx, ny
! input: e, n
! output: ix, iy

! 2006-11-27
! Po Chen, Lamont
  implicit none
  integer ix, iy, nx, ny
  double precision, dimension(4,2) :: box
  double precision lx, ly, e, n, x, y, a, b, c, d, dx, dy, l
      
  lx=sqrt((box(4,1)-box(1,1))**2+(box(4,2)-box(1,2))**2)
  ly=sqrt((box(2,1)-box(1,1))**2+(box(2,2)-box(1,2))**2)
  dx=lx/(nx-1)
  dy=ly/(ny-1)

  a=(lx/(box(4,2)-box(1,2)))/((box(4,1)-box(1,1))/(box(4,2)-box(1,2))-(box(2,1)-box(1,1))/(box(2,2)-box(1,2)))
  b=-a*(box(2,1)-box(1,1))/(box(2,2)-box(1,2))
  c=(ly/(box(2,2)-box(1,2)))/((box(2,1)-box(1,1))/(box(2,2)-box(1,2))-(box(4,1)-box(1,1))/(box(4,2)-box(1,2)))
  d=-c*(box(4,1)-box(1,1))/(box(4,2)-box(1,2))

  x=a*(e-box(1,1))+b*(n-box(1,2))
  y=c*(e-box(1,1))+d*(n-box(1,2))
  ix=nint(x/dx)+1
  iy=nint(y/dy)+1
  return
end subroutine utm2grd


subroutine utm2grd_d(ix,iy,e,n,box,nx,ny)
! convert utm coordinates e, n to grid number ix, iy
! given the four corners of the rectangle in box
! and the total global grid number nx, ny
! input: e, n
! output: ix, iy

! 2006-11-27
! Po Chen, Lamont
  implicit none
  integer nx, ny
  double precision, dimension(4,2) :: box
  double precision ix, iy, lx, ly, e, n, x, y, a, b, c, d, dx, dy, l
      
  lx=sqrt((box(4,1)-box(1,1))**2+(box(4,2)-box(1,2))**2)
  ly=sqrt((box(2,1)-box(1,1))**2+(box(2,2)-box(1,2))**2)
  dx=lx/(nx-1)
  dy=ly/(ny-1)

  a=(lx/(box(4,2)-box(1,2)))/((box(4,1)-box(1,1))/(box(4,2)-box(1,2))-(box(2,1)-box(1,1))/(box(2,2)-box(1,2)))
  b=-a*(box(2,1)-box(1,1))/(box(2,2)-box(1,2))
  c=(ly/(box(2,2)-box(1,2)))/((box(2,1)-box(1,1))/(box(2,2)-box(1,2))-(box(4,1)-box(1,1))/(box(4,2)-box(1,2)))
  d=-c*(box(4,1)-box(1,1))/(box(4,2)-box(1,2))

  x=a*(e-box(1,1))+b*(n-box(1,2))
  y=c*(e-box(1,1))+d*(n-box(1,2))
  ix=(x/dx)+1
  iy=(y/dy)+1
  return
end subroutine utm2grd_d
