      subroutine grd2utm(e,n,ix,iy,box,nx,ny)
! convert the grid number ix, iy to utm
! given the four corners of the rectangle in box
! and the total global grid number nx, ny
! input: ix, iy
! output: e, n

! 2006-11-27
! Po Chen, Lamont

      implicit none
      integer ix, iy, nx, ny
      double precision, dimension(4,2) :: box
      double precision lx, ly, e, n, x, y, a, b, c, d, dx, dy, l
!      print*, 'box corners:'
!      print*, box(1,1),box(1,2)
!      print*, box(2,1),box(2,2)
!      print*, box(3,1),box(3,2)
!      print*, box(4,1),box(4,2)
      lx=sqrt((box(4,1)-box(1,1))**2+(box(4,2)-box(1,2))**2)
      ly=sqrt((box(2,1)-box(1,1))**2+(box(2,2)-box(1,2))**2)
      dx=lx/(nx-1)
      dy=ly/(ny-1)
      x=dble(ix-1)*dx
      y=dble(iy-1)*dy
      l=sqrt(x**2+y**2)
      
      a=(lx/(box(4,2)-box(1,2)))/((box(4,1)-box(1,1))/(box(4,2)-box(1,2))-(box(2,1)-box(1,1))/(box(2,2)-box(1,2)))
      b=-a*(box(2,1)-box(1,1))/(box(2,2)-box(1,2))
      c=(ly/(box(2,2)-box(1,2)))/((box(2,1)-box(1,1))/(box(2,2)-box(1,2))-(box(4,1)-box(1,1))/(box(4,2)-box(1,2)))
      d=-c*(box(4,1)-box(1,1))/(box(4,2)-box(1,2))

      e=a*x+c*y+box(1,1)
      n=b*x+d*y+box(1,2)
      return
      end subroutine
