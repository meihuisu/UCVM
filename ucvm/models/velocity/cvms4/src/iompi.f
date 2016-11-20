c
c MPI I/O for SCEC CVM
c
c Bug fixes and modifications for binary and MPI I/O. Ely 2007/9/1
c Moved z value unit conversion to cvms_sub.f.        PES 2011/07/21
c

      subroutine readpts( kerr )
      implicit none
      include 'in.h'
      include 'mpif.h'
      integer(kind=mpi_offset_kind) :: offset, nn8, np8, nnl8
      integer :: kerr, ip, np, ifh, ierr, i
      character(160) :: lon_file, lat_file, dep_file
      call mpi_init( ierr )
      open( 1, file='cvm-input', status='old' )
      read( 1, * ) nn8
      read( 1, * ) lon_file
      read( 1, * ) lat_file
      read( 1, * ) dep_file
      close( 1 )
      call mpi_comm_rank( mpi_comm_world, ip, ierr )
      call mpi_comm_size( mpi_comm_world, np, ierr )
      call mpi_file_set_errhandler( mpi_file_null,
     $  MPI_ERRORS_ARE_FATAL, ierr )
      nnl = nn8 / np
      np8 = np
      if ( modulo( nn8, np8 ) /= 0 ) nnl = nnl + 1
      nnl8 = nnl
      nn = min( nnl8, nn8 - nnl8 * ip )
      if ( nn.gt.ibig ) then
         print *, 'Error: nn greater than ibig', nn, ibig
         stop
      end if
      call mpi_type_size( mpi_real, i, ierr )
      offset = i
      offset = offset * nnl * ip
      if ( ip == 0 ) write( 0, '(a)' ) 'Reading input'
      call mpi_file_open( mpi_comm_world, lon_file, mpi_mode_rdonly,
     $  mpi_info_null, ifh, ierr )
      call mpi_file_read_at( ifh, offset, rlon, nn, mpi_real,
     $  mpi_status_ignore, ierr )
      call mpi_file_close( ifh, ierr )
      call mpi_file_open( mpi_comm_world, lat_file, mpi_mode_rdonly,
     $  mpi_info_null, ifh, ierr )
      call mpi_file_read_at( ifh, offset, rlat, nn, mpi_real,
     $  mpi_status_ignore, ierr )
      call mpi_file_close( ifh, ierr )
      call mpi_file_open( mpi_comm_world, dep_file, mpi_mode_rdonly,
     $  mpi_info_null, ifh, ierr )
      call mpi_file_read_at( ifh, offset, rdep, nn, mpi_real,
     $  mpi_status_ignore, ierr )
      call mpi_file_close( ifh, ierr )
      if ( ip == 0 ) write( 0, '(a)' ) 'Sampling velocity model'
      do i = 1, nn
        if(rdep(i).lt.0) write( 0, * ) 
     $    'Error: negative depth', i, rlon(i), rlat(i), rdep(i)
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
      include 'mpif.h'
      integer(kind=mpi_offset_kind) :: offset
      integer :: kerr, ip, ifh, ierr, i
      character(160) :: rho_file, alpha_file, beta_file
      open( 1, file='cvm-input', status='old' )
      read( 1, * ) rho_file
      read( 1, * ) rho_file
      read( 1, * ) rho_file
      read( 1, * ) rho_file
      read( 1, * ) rho_file
      read( 1, * ) alpha_file
      read( 1, * ) beta_file
      close( 1 )
      call mpi_comm_rank( mpi_comm_world, ip, ierr )
      call mpi_type_size( mpi_real, i, ierr )
      offset = i
      offset = offset * nnl * ip
      if ( ip == 0 ) write( 0, '(a)' ) 'Writing output'
      call mpi_file_open( mpi_comm_world, rho_file,
     $  mpi_mode_create + mpi_mode_wronly, mpi_info_null, ifh, ierr )
      call mpi_file_write_at( ifh, offset, rho, nn, mpi_real,
     $  mpi_status_ignore, ierr )
      call mpi_file_close( ifh, ierr )
      call mpi_file_open( mpi_comm_world, alpha_file,
     $  mpi_mode_create + mpi_mode_wronly, mpi_info_null, ifh, ierr )
      call mpi_file_write_at( ifh, offset, alpha, nn, mpi_real,
     $  mpi_status_ignore, ierr )
      call mpi_file_close( ifh, ierr )
      call mpi_file_open( mpi_comm_world, beta_file,
     $  mpi_mode_create + mpi_mode_wronly, mpi_info_null, ifh, ierr )
      call mpi_file_write_at( ifh, offset, beta, nn, mpi_real,
     $  mpi_status_ignore, ierr )
      call mpi_file_close( ifh, ierr )
      kerr = 0
      do i = 1, nn
        if(rho(i)/=rho(i).or.alpha(i)/=alpha(i).or.beta(i)/=beta(i))
     $    write( 0, * ) 'Error: NaN', i, rlon(i), rlat(i), rdep(i)
      end do
      if ( ip == 0 ) write( 0, '(a)' ) 'Finished'
      call mpi_finalize( ierr )
      end

