        program  so_cal_basins

c
c program SOuthern CALifornia BASINS ---------------------
c generates so cal area basin velocities and densities ---
c version A    9-97   HMagistrale    ---------------------
c version A2   5-98   newer santa monica mts
c version A3   6-98   fixed SFV glitches
c version A4   6-98   same as A3, but code rewrite to be tidy
c version A5   6-98   corrected to fit oil well data
c                     added separate SGV file hooks,
c                      these currently reread LAB surfaces
c version A6   9-98   fixed more SFV, SMM glitches,
c                     installed 'smooth' H-K background
c                     with constant moho at 32 km
c scum v2_1    1-00   adds in salton trough forward model. HM.
c scum v2_2    1-00   adds in geotechnical constraints. HM.
c                     added misc upgrades
c scum v2e     4-00   added variable Moho. HM.
c scum v2f,g   6-00   modified geotech stuff. HM
c scum v2h     6-00   modified geotech , separate P and S. HM
c scum v2i     8-00   new tomo interpolator, vent glitch fixes HM
c scum v2j     0-00   various glitch fixes
c
c version 3.0  8-01   install upper mantle tomography
c version 4.0  6-05   new Vp-density, new San Berdo, new Imperial Valley
c 
c Bug fixes and modifications for binary and MPI I/O. Ely 2007/9/1
c                   
c

         include 'in.h'

         character(128) modelpath
         character(64) version
         integer ecode

         modelpath= TRIM('.')//achar(0)
         ecode = 0
         version=''

c--display version-------------------------------------
         call cvms_version(version, ecode)
         if(ecode.ne.0)then
            write(*,*)' error retrieving version '
            goto 98
         endif
         write( 0, * )'SCEC CVM-S ',version

c--read points of interest file-------------------------
         call readpts(kerr)
         if(kerr.ne.0)then
            write(*,*)' error with point in file '
            goto 98
         endif

c--perform init-----------------------------------------
         call cvms_init(modelpath, ecode)
         if(ecode.ne.0)then
           write(*,*)' error with init '
           goto 98
         endif

c--perform query-----------------------------------------
         call cvms_query(nn,rlon,rlat,rdep,alpha,beta,rho,ecode)
         if(ecode.ne.0)then
           write(*,*)' error with query '
           goto 98
         endif

c---write out points and values-------------------
         call writepts(kerr)
         if(ecode.ne.0)then
           write(*,*)' error with point out file '
           goto 98
         endif
98       stop
         end

