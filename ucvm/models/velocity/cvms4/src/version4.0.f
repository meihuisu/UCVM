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
         include 'surface.h'
         include 'innum.h'
         include 'labup.h'
         include 'ivsurface.h'
         include 'dim8.h'
         dimension inorout(ibig,isurmx),rsuqus(ibig,isurmx)
         dimension iiitemp(isurmx),inorold(isurmx)
         dimension iiiold(isurmx),rsuqold(isurmx)
         include 'surfaced.h'
         include 'genpro.h'
         include 'genprod.h'

c initialize to zero. Ely
      inct = 0
      incto = 0
      iupm = 0
      idnm = 0
      rshcor = 0.0
      do i = 1,isurmx
        iiiold(i) = 0
        inorold(i) = 0
        rsuqold(i) = 0.0
      end do
      rtemp01 = 0.0
      rtemp05 = 0.0
      rtemp07 = 0.0
      rtemp22 = 0.0
      rtemp36 = 0.0
      rtemp47 = 0.0
      rtemp50 = 0.0
      rtemp55 = 0.0
      rtemp56 = 0.0
      rtemp57 = 0.0
      rtemp62 = 0.0
      rtemp63 = 0.0
      rtemp64 = 0.0
      rtemp65 = 0.0
      rtemp68 = 0.0
      rtemp69 = 0.0
      rtemp70 = 0.0
      rtemp73 = 0.0

c some constants
         rd2rad=3.141593/180.
         rckval=5000000.
c--read points of interest file
         call readpts(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with point file '
         go to 98
         endif
c--read stratigraphic surfaces---------------------------
         call readsurf(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with strat files '
         go to 98
         endif
c--read surface edges------------------------------------
         call readedge(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with edges files '
         go to 98
         endif
c----read model edge (realm) file----------------------
         call modedge(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with model edge file '
         go to 98
         endif
c----read surface geology where needed: LAB, SAN BERDO----
         call readgeo(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with surface geology file '
         go to 98
         endif
c----read uplift where needed: LAB, -------------
         call readup(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with uplift file '
         go to 98
         endif
c---read Imperial Valley surface info---------------
         call readivsurf(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with IV surface file '
         go to 98
         endif
c---read Imperial Valley edge info---------------
         call readivedge(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with IV edge file '
         go to 98
         endif
c--read regional tomography model
         call readreg(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with regional model file '
         go to 98
         endif
c--read geotech borehole data--------------
         call readbore(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with geotech data file '
         go to 98
         endif
c--read generic boreholes--------------
         call readgene(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with geotech generic file '
         go to 98
         endif
c--read soil type surface--------------
         call readsoil(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with soil type file '
         go to 98
         endif
c--read moho surface-------------------
         call readmoho(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with Moho file '
         go to 98
         endif
c--read mantle tomography model--------
         call readman(kerr)
         if(kerr.eq.1)then
         write(*,*)' error with mantle file '
         go to 98
         endif
c---see whos in and out of basin model area realm-------------
c prepare for inside
         xref=rmodtx(i3)
         yref=rmodty(i3)
         reflat=yref*rd2rad
         do 179 i=1,i3
          y2(i) = (rmodty(i) - yref)*111.1
          x2(i) = (rmodtx(i) - xref)*111.1*cos(reflat)
179      continue
c loop over points-find whose in and out of basin model
         do 555 k=1,nn
          yp=(rlat(k)-yref)*111.1
          xp=(rlon(k)-xref)*111.1*cos(reflat)
          call inside(xp,yp,x2,y2,i3,ins)
          inout(k)=ins
555      continue
c--see if points are in Imperial Valley area----
c prepare for inside
         xref=rivvax(ivi2)
         yref=rivvay(ivi2)
         reflat=yref*rd2rad
         do 1799 j=1,ivi2
          y8iv(j) = (rivvay(j) - yref)*111.1
          x8iv(j) = (rivvax(j) - xref)*111.1*cos(reflat)
1799      continue
c -- loop over points-find whose in and out of Imp Valley-
         do 5559 k=1,nn
          yp=(rlat(k)-yref)*111.1
          xp=(rlon(k)-xref)*111.1*cos(reflat)
          call inside(xp,yp,x8iv,y8iv,ivi2,ins)
          ivinot(k)=ins
5559      continue
c 
         roldlat=0.
         roldlon=0.
         ioldfg=0
         ioff=0
         itake=0
c--start loop over points in question-------------------
         do 800 l0=1,nn 
         iregfl=0
         imanfl=0
         rnelat=rlat(l0)
         rnelon=rlon(l0)
c---see if same lat and long as previous---------------
         if(rlat(l0).eq.roldlat.and.rlon(l0).eq.roldlon.and.
     1   ivinot(l0).ne.1)then
         ioldfg=1
         if(incto.eq.0)go to 980
         do 333 i=1,incto
         inorout(l0,i)=inorold(i)
         iiitemp(i)=iiiold(i)
         rsuqus(l0,i)=rsuqold(i)
333      continue
         else
         ioldfg=0
         ioff=0
         endif
c---if in Imperial Valley, go there----------------------
         if(ivinot(l0).eq.1)then
         call makevel2(rlat(l0),rlon(l0),rdep(l0),alp,betm,imanfl)
         alpha(l0)=alp
         beta(l0)=betm
         go to 7999
         endif
c---outside basin or Imperial Valley model area-----------
c-----give it basement or tomo--------
         if(inout(l0).eq.0)goto 980
c skip ahead if same lat and long
         if(ioldfg.eq.1)go to 4454
c--find out what surface it's in and out----------------
c--prepare for inside-----------------------------------
          inct=0
          do 444 j=1,numsur
          n9=nedge(j)
          xref=rtx(j,nedge(j))
          yref=rty(j,nedge(j))
          reflat=yref*rd2rad
           do 410 i=1,nedge(j)
           yv(i)=(rty(j,i)-yref)*111.1
           xv(i)=(rtx(j,i)-xref)*111.1*cos(reflat)
410        continue
          rxp=(rlon(l0)-xref)*111.1*cos(reflat)
          ryp=(rlat(l0)-yref)*111.1
          call inside(rxp,ryp,xv,yv,n9,ins)
          if(ins.eq.1)then
           inct=inct+1
            if(inct.gt.isurmx)then
            write(*,*)'inct gt 15: too many surface hits'
            write(*,*)'l0 =',l0
            go to 98
            endif
           inorout(l0,inct)=j
           iiitemp(inct)=j
           endif
444       continue
c--finding surface depths-------------------
c---case for no surfaces---give it background or tomo-----
         if(inct.eq.0)goto 980
c---find appropriate surface depths-------------------
         do 600 i9=1,inct
         i=(inorout(l0,i9))
c---find valid surfaces-------------------
         do 813 l3=1,nlasur(i)-1
         if(rlat(l0).le.rlasur(i,l3).and.rlat(l0).gt.rlasur(i,l3+1))then
         do 824 l4=1,nlosur(i)-1
         if(rlon(l0).gt.rlosur(i,l4).and.rlon(l0).le.rlosur(i,l4+1))then
         rrt=(rlon(l0)-rlosur(i,l4))/(rlosur(i,l4+1)-rlosur(i,l4))
         rru=(rlat(l0)-rlasur(i,l3+1))/(rlasur(i,l3)-rlasur(i,l3+1))
c--note here rsuval indexes are (surface number, long, lat)------
         rsuqus(l0,i9)=((1-rrt)*(1-rru)*rsuval(i,l4,l3+1))+(rrt*(1-rru)*
     1       rsuval(i,l4+1,l3+1))+(rrt*rru*rsuval(i,l4+1,l3))+((1-rrt)*
     2       rru*rsuval(i,l4,l3))
c diag   write(*,*)'num ',l0,' surface ',i,' depth ',rsuqus(l0,i9)
          endif
824        continue
          endif
813      continue
600      continue
4454       continue
c -- correct some surfaces here for cases where know a surface should
c --always be below another-------------------------------------------
c --62 and 64 sometimes arsy-versy----
c --47 and 55 sometimes arsy-versy--
c --1, 57, and 50 sometimes arsy-versy--
c --1 and 56 sometimes arsy-versy
c --1 and 47 sometimes arsy-versy
c --65 and 63 sometimes arsy-versy
           iflag64=0
           iflag62=0
           iflag47=0
           iflag55=0
           iflag73=0
c A5 to A6 changes
           iflag01=0
           iflag50=0
           iflag56=0
           iflag57=0
           iflag65=0
           iflag63=0
c end A6 changes
           inum62=0
           inum64=0
           inum47=0
           inum55=0
           inum73=0
c A5 to A6 changes
           inum01=0
           inum50=0
           inum56=0
           inum57=0
           inum65=0
           inum63=0
c end A6 changes
c for h to i changes
           inum22=0
           inum07=0
           iflag22=0
           iflag07=0
           inum36=0
           inum05=0
           iflag36=0
           iflag05=0
           inum68=0
           inum69=0
           inum70=0
           inum73=0
           iflag68=0
           iflag69=0
           iflag70=0
           iflag73=0
c
           do 676 nx=1,inct
            if(inorout(l0,nx).eq.62)then
             iflag62=1
             inum62=nx
             rtemp62=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.64)then
             iflag64=1
             inum64=nx
             rtemp64=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.47)then
             iflag47=1
             inum47=nx
             rtemp47=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.55)then
             iflag55=1
             inum55=nx
             rtemp55=rsuqus(l0,nx)
             if(rsuqus(l0,nx).lt.16.4)rsuqus(l0,nx)=0.
            endif
c A5 to A6 changes
            if(inorout(l0,nx).eq.65)then
             iflag65=1
             inum65=nx
             rtemp65=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.63)then 
             iflag63=1 
             inum63=nx 
             rtemp63=rsuqus(l0,nx) 
            endif
            if(inorout(l0,nx).eq.1)then
             iflag01=1
             inum01=nx
             rtemp01=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.50)then
             iflag50=1
             inum50=nx
             rtemp50=rsuqus(l0,nx) 
            endif
            if(inorout(l0,nx).eq.56)then 
             iflag56=1 
             inum56=nx 
             rtemp56=rsuqus(l0,nx) 
            endif
            if(inorout(l0,nx).eq.57)then 
             iflag57=1 
             inum57=nx 
             rtemp57=rsuqus(l0,nx) 
            endif
c end A6 changes
c h to i
            if(inorout(l0,nx).eq.22)then
             iflag22=1
             inum22=nx
             rtemp22=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.7)then
             iflag07=1
             inum07=nx
             rtemp07=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.36)then
             iflag36=1
             inum36=nx
             rtemp36=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.5)then
             iflag05=1
             inum05=nx
             rtemp05=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.68)then
             iflag68=1
             inum68=nx
             rtemp68=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.69)then
             iflag69=1
             inum69=nx
             rtemp69=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.70)then
             iflag70=1
             inum70=nx
             rtemp70=rsuqus(l0,nx)
            endif
            if(inorout(l0,nx).eq.73)then
             iflag73=1
             inum73=nx
             rtemp73=rsuqus(l0,nx)
            endif
c need this for SGV velocity inversion
            if(inorout(l0,nx).eq.73)then
             inum73=nx
             iflag73=1
            endif
676        continue
c A5 to A6 changes
           if(iflag56.eq.1.and.iflag01.eq.1)then
            if(rtemp56.gt.rtemp01)then
             rsuqus(l0,inum01)=rtemp56
             rsuqus(l0,inum56)=rtemp01
            endif
           endif
           if(iflag57.eq.1.and.iflag50.eq.1)then
            if(rtemp50.gt.rtemp57)then
             rsuqus(l0,inum57)=rtemp50
             rsuqus(l0,inum50)=rtemp57
             rtemp57=rtemp50
            endif
           endif
           if(iflag01.eq.1.and.iflag57.eq.1)then
            if(rtemp57.gt.rtemp01)then
            rsuqus(l0,inum01)=rtemp57
            rsuqus(l0,inum57)=rtemp01
            endif 
           endif
       if(iflag01.eq.1.and.iflag50.eq.1.and.iflag57.eq.0)then
            if(rtemp50.gt.rtemp01)then
            rsuqus(l0,inum01)=rtemp50
            rsuqus(l0,inum50)=rtemp01
            endif
           endif
           if(iflag65.eq.1.and.iflag63.eq.1)then
            if(rtemp65.gt.rtemp63)then
             rsuqus(l0,inum65)=rtemp63
             rsuqus(l0,inum63)=rtemp65
            endif
           endif
c end A6 changes
c h to i
           if(iflag07.eq.1.and.iflag22.eq.1)then
            if(rtemp22.gt.rtemp07)then
            rsuqus(l0,inum22)=rtemp07
            rsuqus(l0,inum07)=rtemp22
            endif
           endif
           if(iflag05.eq.1.and.iflag36.eq.1)then 
            if(rtemp36.gt.rtemp05)then 
            rsuqus(l0,inum36)=rtemp05 
            rsuqus(l0,inum05)=rtemp36 
            endif 
           endif
           if(iflag70.eq.1.and.iflag69.eq.1)then
            if(rtemp70.gt.rtemp69)then
            rsuqus(l0,inum69)=rtemp70
            rsuqus(l0,inum70)=rtemp69
            endif
            if(rtemp70.eq.rtemp69)then
            rsuqus(l0,inum70)=rsuqus(l0,inum70)-1.
            endif
           endif
           if(iflag68.eq.1.and.iflag69.eq.1)then 
            if(rsuqus(l0,inum69).gt.rtemp68)then 
            rsuqus(l0,inum68)=rsuqus(l0,inum69)
            rsuqus(l0,inum69)=rtemp68 
            endif 
            if(rtemp68.eq.rtemp69)then 
            rsuqus(l0,inum68)=rsuqus(l0,inum68)+1. 
            endif
           endif
           if(iflag68.eq.1.and.iflag73.eq.1)then
            if(rtemp73.gt.rsuqus(l0,inum68))then
            rsuqus(l0,inum73)=rsuqus(l0,inum68)
            rsuqus(l0,inum68)=rtemp73
            endif
            if(rtemp68.eq.rtemp73)then
            rsuqus(l0,inum68)=rsuqus(l0,inum73)+1.
            endif
           endif
c
           if(iflag62.eq.1.and.iflag64.eq.1)then
            if(rtemp64.gt.rtemp62)then
             rsuqus(l0,inum64)=rtemp62
             rsuqus(l0,inum62)=rtemp64
            endif
           endif
           if(iflag55.eq.1.and.iflag47.eq.1)then
            rd55=(rtemp47-rtemp55)
c           if(rtemp55.eq.rtemp47)then
            if(rd55.lt.2..and.rd55.ge.0.)then
             rsuqus(l0,inum47)=rsuqus(l0,inum47)+32.8
             endif
            if(rtemp55.gt.rtemp47)then
             rsuqus(l0,inum55)=rtemp47
             rsuqus(l0,inum47)=rtemp55
c A5 to A6 change
             rtemp47=rtemp55
c end A6 change
            endif
           endif
c A5 to A6 change
           if(iflag01.eq.1.and.iflag47.eq.1)then
            if(rtemp47.gt.rtemp01)then
             rsuqus(l0,inum47)=rtemp01
             rsuqus(l0,inum01)=rtemp47
            endif
           endif
c end A6 change
c---check which surface is above------------------------------------
c---surface sign note: (+) are below sea level, (-) are above-------
c---check which surface is below------------------------------------
c--also, find shallowest surface-----
           rchk=rckval
           rchk2=rckval
           rshal=rckval
           iupflag=0
           iup=0
           idn=0
           ishal=0
           do 142 i8=1,inct
           rdelt=abs(rdep(l0)-rsuqus(l0,i8))
           rdelt2=abs(rsuqus(l0,i8)-rdep(l0))
           if(rdelt.lt.rchk.and.rsuqus(l0,i8).le.rdep(l0))then
            rchk=rdelt
            iup=(inorout(l0,i8))
            iupm=i8
            endif
           if(rdelt2.lt.rchk2.and.rdep(l0).lt.rsuqus(l0,i8))then
            rchk2=rdelt2
            idn=(inorout(l0,i8))
            idnm=i8
            endif 
           if(rsuqus(l0,i8).lt.rshal)then
            rshal=rsuqus(l0,i8)
            ishal=(inorout(l0,i8))
            endif
142        continue
c diag     write(*,*)l0, iup, idn, ishal
           if(iup.ne.0.and.idn.ne.0)goto 1177
c---hey, if at ground surface, won't find a surface above-----------
c--of course, expect this in LAB and SAN BERDO where have--
c--surface geology --
           if(iup.eq.0.and.idn.ne.0)then
c check for LAB and SMM and SAN BERDO ---99 is a flag---
            if(idn.ge.58)then
            iupflag=99
c diag      write(*,*)l0,iup,idn
            go to 1177
            endif
           iup=idn
           iupm=idnm
           ishal=idn
           goto 1177
           endif
c--in VENT and SFV can have case where at bottom of model but
c--no crystalline basement---so
c---check if current point below max control depths----------------
c ie for cases where found something above and nothing below
c--really need a separate surface for max depth control--
c--here, assign basement if > 2 km below control----
c--this takes care of single surface control
           if(iup.ne.0.and.idn.eq.0)then
c -- smm line H fix--
           if(iup.eq.64)go to 980
           rmaxck=rdep(l0)-rsuqus(l0,iupm)
           if(rmaxck.lt.6562.)then
           idn=iup
           idnm=iupm
           ishal=iup
           else
           go to 980
           endif
           endif
c----in crystalline basement use background velos-------------
c this is for: b1, b3,b4,b5, sbb2_sur,smb1_sur,smb2_sur,laba_sur
c---(recall incorrectly joined sfv b1 and vent b1)
1177       continue
c-b1 surface----------------------------------
           if(iup.eq.1)go to 980
c-b3 surface------------------------------------
           if(iup.eq.3)go to 980
c-b4 surface------------------------------------
           if(iup.eq.4)go to 980
c-b5 surface------------------------------------
           if(iup.eq.5)go to 980
c-sbb2_sur surface (aka sb_base2_sur_A)--------
           if(iup.eq.58)go to 980
c-sgba_sur surface (san gab valley basement)----
           if(iup.eq.72)go to 980
c--smb3 rules--and other smm rules-------
           if(iup.eq.71.and.idn.eq.67)then
            call getup(rlat(l0),rlon(l0),ruplc) 
            rupcor=amax1(ruplc,0.)
            rd01=0.
            rd02=rsuqus(l0,idnm)
            call getgeo(rlat(l0),rlon(l0),ragesur)
            ra1=ragesur
            ra2=rage(idn)
            rk1=rkall(idn)
            rk2=rkall(idn)
            rf1=rfacs(idn)
            rf2=rfacs(idn)
            rscal=(rdep(l0)-rd01)/(rd02-rd01)
            go to 9191
           endif
           if(iup.eq.71.and.idn.eq.65)then
           call getup(rlat(l0),rlon(l0),ruplc)
            rupcor=amax1(ruplc,0.)
            rd01=rsuqus(l0,iupm)
            rd02=rsuqus(l0,idnm)
            ra1=rage(67)
            ra2=rage(idn)
            rk1=rkall(67) 
            rk2=rkall(idn) 
            rf1=rfacs(67) 
            rf2=rfacs(idn)
            rscal=(rdep(l0)-rd01)/(rd02-rd01)
            go to 9191
           endif
           if(iup.eq.64.and.idn.eq.67)then
            call getup(rlat(l0),rlon(l0),ruplc)
            rupcor=amax1(ruplc,0.)
            rd01=0.
            rd02=rsuqus(l0,idnm)
            call getgeo(rlat(l0),rlon(l0),ragesur)
            ra1=ragesur
            ra2=rage(idn)
            rk1=rkall(idn) 
            rk2=rkall(idn) 
            rf1=rfacs(idn) 
            rf2=rfacs(idn)
            rscal=(rdep(l0)-rd01)/(rd02-rd01)
            go to 9191
           endif
           if(iup.eq.71.and.idn.eq.63)then
           call getup(rlat(l0),rlon(l0),ruplc)
            rupcor=amax1(ruplc,0.)
            rd01=rsuqus(l0,iupm)
            rd02=rsuqus(l0,idnm)
            ra1=rage(65)
            ra2=rage(idn)
            rk1=rkall(65) 
            rk2=rkall(idn) 
            rf1=rfacs(65) 
            rf2=rfacs(idn)
            rscal=(rdep(l0)-rd01)/(rd02-rd01)
            go to 9191
           endif
c -- fix for south sfv-north smm
c          if(iup.eq.55.and.idn.eq.55)go to 980
           if(iup.eq.47.and.idn.eq.47)go to 980
c -- between smm basement layers-------------
           if(iup.eq.62.and.idn.eq.71)go to 980
           if(iup.eq.62.and.idn.eq.63)go to 980
c -- fix for b3 over m1-----------------------
           if(iup.eq.71.and.idn.eq.64)then
            call getup(rlat(l0),rlon(l0),ruplc)
            rupcor=amax1(ruplc,0.)
            ra1=rage(64)
            ra2=rage(64)
            rk1=rkall(64) 
            rk2=rkall(64) 
            rf1=rfacs(64) 
            rf2=rfacs(64)
            rscal=1.
            go to 9191
           endif
c --- smm fix for between b9 and m2-for Line I
c change it to between b3 and m2 (as above)
c for line J this worked:if(iup.eq.62.and.idn.eq.65)go to 980
           if(iup.eq.62.and.idn.eq.65)then
            call getup(rlat(l0),rlon(l0),ruplc)
            rupcor=amax1(ruplc,0.)
            ra1=rage(67)
            ra2=rage(idn)
            rk1=rkall(67) 
            rk2=rkall(idn) 
            rf1=rfacs(67) 
            rf2=rfacs(idn)
             do 9988 i=1,inct
             if(inorout(l0,i).eq.71)then
c-- line J fix--
              if(rsuqus(l0,i).gt.rtemp62)go to 980
              rd01=rsuqus(l0,i)
              rd02=rsuqus(l0,idnm)
              go to 9989
             endif
9988         continue
              rd01=rsuqus(l0,iupm)
              rd02=rsuqus(l0,idnm)
9989        rscal=(rdep(l0)-rd01)/(rd02-rd01)
            go to 9191             
            endif
c-smb1_sur surface (aka smm_b1_sur)-b9 in this version--
           if(iup.eq.62.and.idn.eq.iup)go to 980
c-smb3_sur surface (aka smm_b1_sur)---------------------
           if(iup.eq.71.and.idn.eq.iup)go to 980
c-smb2_sur surface (aka smm_b2_sur)--------------
           if(iup.eq.63)go to 980
c-laba_sur surface (aka basement.ascii.B_no_num)--
           if(iup.eq.68)go to 980
c----assign reference depths------------rupcor is 'uplift'---------
c---------------------------------------rshcor is elev correction--
           if(rshal.lt.0.)then
           rshcor=abs(rshal)
           else
           rshcor=0.
           endif
c---need a top layer or not here--(top here)--------------
           if(iupflag.ne.99)then
            rupcor=(rupl(iup)-rdep(l0))
            if(rupcor.lt.0.)rupcor=0.
c---if in LAB, must look up uplift------------------------ 
            if(idn.ge.62)then 
             call getup(rlat(l0),rlon(l0),ruplc) 
             rupcor=amax1(ruplc,0.) 
            endif
            rd01=rsuqus(l0,iupm)
            rd02=rsuqus(l0,idnm)
           else
c---------------------------------(BERDO no top layer here)-----
            rupcor=0.
c-----------------------------------LAB, no top----------
            if(idn.ge.62)then
             call getup(rlat(l0),rlon(l0),ruplc)
             rupcor=amax1(ruplc,0.)
            endif
             rd01=0.
             rd02=rsuqus(l0,idnm)
           endif 
c----assign reference ages-------(no surface geology)---------------
           if(iupflag.ne.99)then
            ra1=rage(iup)
            ra2=rage(idn)
            rk1=rkall(iup) 
            rk2=rkall(idn) 
            rf1=rfacs(iup) 
            rf2=rfacs(idn)
c---------------------------------(must look up surface geology)---
            else
            call getgeo(rlat(l0),rlon(l0),ragesur)
            ra1=ragesur
            ra2=rage(idn)
            rk1=rkall(idn) 
            rk2=rkall(idn) 
            rf1=rfacs(idn) 
            rf2=rfacs(idn)
           endif
c---interpolate age from strat--------------------------------------
c--------(get rscal without uplift)---------------------------------
          if(iup.ne.idn)then
          rscal=(rdep(l0)-rd01)/(rd02-rd01)
          else
          rscal=1.
          endif
c----scale sed age, define final depth------------
9191      rtage=(rscal*ra2)+((1.-rscal)*ra1)
          rtdep=rdep(l0)+rupcor+rshcor
          if(rtdep.eq.0.)rtdep=3.28084
c  diag   write(*,*)rdep(l0),rupcor,rshcor
c----scale constant, exponent---------------------
          rk=(rscal*rk2)+((1.-rscal)*rk1)
          rfac=(rscal*rf2)+((1.-rscal)*rf1)
c---find alpha in ft/s---------------------------------------------
          alpha(l0)=rk*(rtdep**rfac)*(rtage**rfac)
c---San Gabriel Valley velocity inversion--SGV---------------------
c 1280 ft (about 390 m) is depth over inversion of
c 4087 ft/s is tapered in
c 0.6 of mohnian depth is where inversion occurs
c         if(idn.ge.72.and.idn.le.74)then
          if(idn.ge.72)then
          if(iflag73.eq.1)then
           rsgvck=0.6*rsuqus(l0,inum73)
           if(rdep(l0).gt.rsgvck)then
            rsgv2=(rdep(l0)-rsgvck)/1280.
            if(rsgv2.gt.1.)rsgv2=1.0
            rdelve=(exp(rsgv2)/2.718281828)*4087.
            alpha(l0)=alpha(l0)-rdelve
           endif
          endif
          endif
          go to 799
c--- assign regional tomo model---------------------
980       call makereg(rlat(l0),rlon(l0),rdep(l0),alp,bet,iregfl)
          alpha(l0)=alp
          beta(l0)=bet
c -- find moho depth, rdemoh-- 
          call mohodepth(rlat(l0),rlon(l0),rdemoh)
          if(rdep(l0).ge.rdemoh)then
          iregfl=0
c---assign upper mantle velocities
          call makeman(rlat(l0),rlon(l0),rdep(l0),alpm,betm,imanfl)
          alpha(l0)=alpm
          beta(l0)=betm
          endif
c check if in basement within basin area
          if(inout(l0).eq.1)goto 7998
          go to 7999
c----convert alpha to m/s-------------------------------
799       alpha(l0)=alpha(l0)*0.30480
c--- screen if want geotech info ----
c now simply by depth, 304.8 m or 1000.0 ft
7998      ifs=0
          ifp=0
          if(rdep(l0).le.1000.)then
c--- add shallow geotech -----
c--find type of soil class-------------------------------
          call getsoil(rlat(l0),rlon(l0),isoilt,idgens,idgenp,
     1    iiitemp,inct)
          if(rdep(l0).le.(rmxdep(idgens)*3.2808399))ifs=1
          if(rdep(l0).le.(rmxdep(idgenp)*3.2808399))ifp=1
c--look up nearby boreholes------------------------------
c         if(ifs.eq.1.or.ifp.eq.1)then
          call nearhole(rlat(l0),rlon(l0),isoilt)
c         endif
c--get geotech P wave----
          call addtopp(rdep(l0),alpha(l0),idgenp,ifp,alpha2,ioldfg)
          alpha(l0)=alpha2
          endif
7999      continue
c---clamp here if wanted---------------------------------
c clamp for beta=1.0 km/s
c         if(alpha(l0).lt.2414.)alpha(l0)=2414.
c         if(beta(l0).lt.1000.)beta(l0)=1000.
c clamp for beta =0.5 km/s
c         if(alpha(l0).lt.1225.)alpha(l0)=1225.
c         if(beta(l0).lt.500.)beta(l0)=500.
C comment out with version 3 Vp-density-------------
cc---find rho----------------------------------------------
cc --see lab model for citations---------------------------
cc--assume density in case of velocity lt 1500 m/s---------
cc        rho(l0)=1000.
          rho(l0)=1500.
cc--for velocities 1586.1 to 1625 m/s------------------------
c         if(alpha(l0).ge.1586.1)rho(l0)=(alpha(l0)*5.8971675)-7853.4328
cc--for velocities 1500 to 1625 m/s------------------------
cc        if(alpha(l0).ge.1500.)rho(l0)=(alpha(l0)*5.8971675)-7853.4328
cc--for velocities 1625 to 2500 m/s------------------------ 
c         if(alpha(l0).gt.1625.)rho(l0)=(alpha(l0)*.444174)+1000.
cc--for velocities 2500 to 7000 (and above) m/s------------
c         if(alpha(l0).gt.2500.)rho(l0)=(alpha(l0)*.17333)+1695.65
C
C version 4 Vp-density  linear 
          rho(l0)=1865.+(.1579*alpha(l0))
c------ find beta------------------------------------------
c --see lab model for citations----------------------------
         sigma=0.40
         if(rho(l0).ge.2060.)sigma=.40-((rho(l0)-2060.)*.00034091)
         if(rho(l0).gt.2500.)sigma=0.25
         beta(l0)=alpha(l0)/(sqrt((1.-sigma)/(.5-sigma)))
c--if regional tomo or mantle beta was looked up, replace beta(l0)----
         if(iregfl.ne.0)beta(l0)=bet
         if(imanfl.ne.0)beta(l0)=betm
         if(isoilt.eq.99)go to 808
c--get geotech S wave----
         if(inout(l0).eq.1)then
         if(rdep(l0).le.1000.)then
         call addtops(rdep(l0),beta(l0),idgens,ifs,beta2,ioldfg,ioff)
         beta(l0)=beta2
c    check on Vp/Vs, control from Vs
         bett=beta2*(sqrt(2.))
         if(bett.gt.alpha2)then
          alpha(l0)=bett
c     update density
         if(alpha(l0).ge.1586.1)rho(l0)=(alpha(l0)*5.8971675)-7853.4328
         if(alpha(l0).gt.1625.)rho(l0)=(alpha(l0)*.444174)+1000.
         if(alpha(l0).gt.2500.)rho(l0)=(alpha(l0)*.17333)+1695.65
         endif
         endif
         endif
c----assign some stuff
808      continue
         roldlat=rlat(l0)
         roldlon=rlon(l0)
         incto=inct
         do 335 i=1,inct
         inorold(i)=inorout(l0,i)
         iiiold(i)=iiitemp(i) 
         rsuqold(i)=rsuqus(l0,i)
335      continue
c
c end main loop------------------------------------------
800      continue
c----------postpro here if wanted------------------------
c        call postpro
c------------- write out points and values---------------
         call writepts(kerr)
98       stop
         end

         subroutine readedge(kerr)
c-----reads strat surface edges---------------
c---reads x-y pairs--------------------------
         include 'surface.h'
         character(9) aname2, asuf2*5
         include 'names.h'
         asuf2='_edge'
         kerr=0
c-------------------------------
         do 217 i=1,numsur
          aname2=aedname(i)//asuf2
          open(17,file=aname2,status='old',err=299)
           do 218 j=1,nedge(i)
           read(17,*)rtx(i,j),rty(i,j)
218        continue
          close(17)
217       continue
         go to 201
299      kerr=1
         write(*,*)aname2,i
201      return
         end

           subroutine modedge(kerr)
c-----read model edge file, kept separate from surface edge-----
c-------to simplify index counts--------------------------------
          include 'surface.h'
          character(9) aname3
          kerr=0
c---file name assignment-------
          aname3='bmod_edge'
c----i3=number of xy pairs-------
          open(18,file=aname3,status='old',err=259)
c        read(18,*)i3   !i3 now set at top
         do 511 i=1,i3
         read(18,*)rmodtx(i),rmodty(i)
511      continue
         close(18)
         go to 291
259      kerr=1
291      return
         end

         subroutine readgeo(k2err)
c reads geologic contours
         include 'sgeo.h'
         character(16) fileii,a1*10
         include 'sgeod.h'
c---file name assignment-------
         fileii='lab_geo2_geology'
         k2err=0
c read file
           open(12,file=fileii,status='old',err=977)
           do 300 k=1,ngeo
           read(12,*)np(k)
            do 310 k1=1,np(k)
            read(12,*)rloi(k,k1),rlai(k,k1)
310         continue
c skip label line. Label follows lat-long points
            read(12,*)a1
300        continue
           close(12)
           go to 976
977        k2err=1
976        return
           end

          subroutine getgeo(rlatl0,rlonl0,ragesur)
c---------get surface geology age-----------
c
c finds surface age from surface geology
c looks inside geology contours for hit.
c contours are crudely digitized, so if no hit, uses closest point
         include 'sgeo.h'
         dimension rdelt(ngeo,ngeo2),xv(ngeo2),yv(ngeo2)
           iflag=5
           rdmin=100.
           rd2rad=3.141593/180.
c look inside
           do 400 j=1,ngeo
           xref=rloi(j,np(j))
           yref=rlai(j,np(j))
           reflat=yref*rd2rad
            do 410 i=1,np(j)
            yv(i)=(rlai(j,i)-yref)*111.1
            xv(i)=(rloi(j,i)-xref)*111.1*cos(reflat)
410         continue
           rxp=(rlonl0-xref)*111.1*cos(reflat)
           ryp=(rlatl0-yref)*111.1
           call inside(rxp,ryp,xv,yv,np(j),ins)
           if(ins.eq.1)then
           ragesur=ra(j)
           go to 199
           endif
400        continue
c okay, missed, use backup-nearest contour point defines it
           do 500 l=1,ngeo
            do 510 l2=1,np(l)
            rlodif=(rlonl0-rloi(l,l2))*92.3820
            rladif=(rlatl0-rlai(l,l2))*110.9220
            rdelt(l,l2)=sqrt((rlodif**2.)+(rladif**2.))
            if(rdelt(l,l2).lt.rdmin)then
            rdmin=rdelt(l,l2)
            iflag=l
            endif
510         continue
500        continue
           ragesur=ra(iflag)
           go to 199
c
199        return
           end

         subroutine readsurf(kerr)
c---reads stratigraphic surfaces-------------------------
c---reads spyglass ascii output--------------------------
         include 'surface.h'
         character(8) aname, asuf*4
         character(9) aname2, a18*1
         include 'names.h'
         asuf='_sur'
         a18='2'
         kerr=0
c---loop to read-------------------
         do 117 i=1,numsur
          aname=aedname(i)//asuf
          aname2=aedname(i)//asuf//a18
          open(16,file=aname2,status='old',err=99)
           do 118 k=1,nlasur(i)
           do 118 j=1,nlosur(i)
       read(16,11188)rlasur(i,k),rlosur(i,j),rsuval(i,j,k)
c118       continue
11188  format(f9.5,1x,f10.5,1x,f10.2)
c---For VENT and SFV (i <= 57) convert depth meters to feet-------
            if(i.le.57)then
             rsuval(i,j,k)=rsuval(i,j,k)*3.28084
             endif
cc---For LAB (i=68,69,70) convert depth thousands of feet to feet---
             if(i.ge.68.and.i.le.70)then
             rsuval(i,j,k)=rsuval(i,j,k)*1000.
             endif
cc---For SGV (i=72,73,74) convert depth thousands of feet to feet---
             if(i.ge.72.and.i.le.74)then
             rsuval(i,j,k)=rsuval(i,j,k)*1000.
             endif
118       continue
         close(16)
117      continue
         go to 101
99       kerr=1
101      return
         end

        subroutine readup(kerr)
c-reads uplift file for LAB--------------------
c-----just like readsur-------------
        include 'labup.h'
        character(9) filei4
c file name assignment
        filei4='laup_sur2'
c
        open(16,file=filei4,status='old',err=99)
c
         do 400 i=1,nlaup 
          do 440 j=1,nloup
           rlaup(i) = 0.0
           rloup(i) = 0.0
c temp turned off for historical reason
c          read(16,11440) rlaup(i),rloup(j),rzupl(i,j) 
c11440     format(f9.5,1x,f10.5,1x,f10.2)
c convert thousands of feet to feet---------------
          rzupl(i,j)=rzupl(i,j)*1000. 
440       continue 
400      continue  
         close(16)
         go to 101
99       kerr=1
101      return
         end

        subroutine getup(rlatl0,rlonl0,ruplc)
c--gets uplift for LAB---------------
c find uplift amount at current lat long
        include 'labup.h'
           do 817 l7=1,nlaup-1
           if(rlatl0.le.rlaup(l7).and.rlatl0.gt.rlaup(l7+1))then
            do 828 l8=1,nloup-1 
            if(rlonl0.gt.rloup(l8).and.rlonl0.le.rloup(l8+1))then
            rmt=(rlonl0-rloup(l8))/(rloup(l8+1)-rloup(l8))
            rmu=(rlatl0-rlaup(l7+1))/(rlaup(l7)-rlaup(l7+1))
            ruplc=((1-rmt)*(1-rmu)*rzupl(l7+1,l8))+(rmt*(1-rmu)*
     1       rzupl(l7+1,l8+1))+(rmt*rmu*rzupl(l7,l8+1))+((1-rmt)*
     2       rmu*rzupl(l7,l8)) 
            go to 899 
            endif    
828        continue 
           endif   
817       continue
          ruplc=0.  
899       return
          end
c
        subroutine inside (xp,yp,xv,yv,n,ins)
        dimension xv(n),yv(n)
        nleft=0
        x2=xv(n)
        y2=yv(n)
        do 14 j1=1,n
        x1=x2
        y1=y2
        x2=xv(j1)
        y2=yv(j1)
        if(amin1(y1,y2).ge.yp) goto 14
        if(amax1(y1,y2).lt.yp) goto 14
        if (y1.eq.y2) goto 27
        xi=x1+(yp-y1)*(x2-x1)/(y2 -y1)
        if(xi.eq.xp) goto 17
        if(xi.gt.xp) nleft=nleft+1
        goto 14
   27   if(xp.gt.amax1(x1,x2)) goto 14
        if(xp.ge.amin1(x1,x2)) goto 17
        nleft=nleft+1
   14   continue
        ins=mod(nleft,2)
        return
   17   ins=1
        return
        end
c
c
        subroutine postpro
c POST PROcessing for LAB model
c continues velocities out from realm of
c credibility to beyond
         include 'in.h'
c
c linear interpolation distance rinterp
c    - KLM -
c
         rinterp = 5.0
c continue those outside realm
         do 556 n=1,nn
         if(inout(n).ne.1)then
c
c save the regional model
c   - KLM -
c
          alpha1 = alpha(n)
          beta1 = beta(n)
          rho1 = rho(n)
c look for closest point inside
          rdmin=300.
          do 557 n2=1,nn
          if(inout(n2).ne.1)go to 557
          if(rdep(n2).ne.rdep(n))go to 557
          rlodif=(rlon(n2)-rlon(n))*92.3820
          rladif=(rlat(n2)-rlat(n))*110.9220
c          rdelt=sqrt((rlodif**2.)+(rladif**2.))
c replaced L2 norm with L1 norm to make things faster
c    -  KLM -
c
          rdelt=abs(rlodif)+abs(rladif)
          if(rdelt.lt.rdmin)then
          rdmin=rdelt
          alpha(n)=alpha(n2)
          rho(n)=rho(n2)
          beta(n)=beta(n2)
          endif
557       continue
          endif
c
c do a linear interpolation from closest inside point to regional model
c   - KLM -
c
          if(inout(n).eq.1)go to 556
          if( rdmin .gt. rinterp ) then
              alpha(n) = alpha1
              rho(n) = rho1
              beta(n) = beta1
          else if( rdmin .le. rinterp ) then
              alpha(n) = alpha(n) + (alpha1-alpha(n))*rdmin/rinterp
              beta(n) = beta(n) + (beta1-beta(n))*rdmin/rinterp
              rho(n) = rho(n) + (rho1-rho(n))*rdmin/rinterp
          endif
556      continue
         return
         end

         subroutine readivsurf(kerr)
c-----read Imperial Valley surfaces--------------
         include 'in.h'
         include 'ivsurface.h'
         kerr=0
c---loop to read-------------------
         do 2117 i=1,numsiv
          open(16,file='salton_base.sur',status='old',err=2199)
           do 2118 k=1,nlasiv(i)
           do 2118 j=1,nlosiv(i)
           read(16,*)rlasiv(i,k),rlosiv(i,j),rsuvil(i,j,k)
c11777      format(t1,f7.2,1x,f10.5,1x,f10.2)
cc convert km depths to feet
          rsuvil(i,j,k)=rsuvil(i,j,k)*3280.84
2118        continue
         close(16)
2117      continue
         go to 2101
2199       kerr=1
2101      return
         end

         subroutine readivedge(kerr)
c-----read Imp valley- Salton Trough edge file,-----------------
c  and iv model edge file
         include 'in.h'
         include 'ivsurface.h'
         kerr=0
c----ivi2=number of xy pairs-------
         open(17,file='impva.edge',status='old',err=2599)
         do 5119 i=1,ivi2
         read(17,*)rivvax(i),rivvay(i)
5119     continue
         close(17)
         open(17,file='ivmod.edge',status='old',err=2599)
         do 2511 k=1,ivi3  
         read(17,*)rmoivx(k),rmoivy(k)
2511      continue
         close(17)
         go to 2919
2599     kerr=1 
2919     return
         end

          subroutine readreg(kerr)
c -- read regional model info---------------------------
c  nregll = number points per layer of regional model
c  nregv  = total number P or S velocities in regional model
c  nregly = number layers in regional model
c Using Egill Hauksson's so cal model at 15 km horizontal
c  spacing, variable vertical spacing
         include 'in.h'
         include 'regional.h'
          kerr=0
         open(19,file='eh.modPS',status='old',err=2999)
         do 1119 j=1,nregll
         read(19,1818)reglat(j),reglon(j)
1119     continue
         rewind(19)
         do 1120 j2=1,nregv
         read(19,1819)regvep(j2),regves(j2)
c -- convert to m/s
         regvep(j2)=regvep(j2)*1000.
         regves(j2)=regves(j2)*1000.
1120     continue
         close(19)
         go to 1901
1818     format(f10.5,f13.5)
1819     format(t32,f6.2,1x,f6.2)
2999     kerr=1
1901     return
         end

          subroutine makevel2(rla,rlo,rde,alp,betm,imanfl)
c--Calculates the Imperial Valley model velocities--
c note betm returned is temporary dummy valus unless it is from mantle
         include 'in.h'
         include 'ivsurface.h'
         include 'dim2.h'
         dimension rsuqiv(numsiv)
         include 'ivsurfaced.h'
c ver 4  include 'generic_loc.h'
c VER 4 USING ORDINARY regional moho FOR NOW
c
c---find appropriate surface depths-------------------
c        do 6009 i9=1,numsiv
c---find valid surfaces-------------------
         rd2rad = 3.141593 / 180.0
         do i = 1, numsiv
           rsuqiv(i) = 0.0
         end do
         i9=1

         
         do 8139 l3=1,nlasiv(i9)-1
      if(rla.le.rlasiv(i9,l3).and.rla.gt.rlasiv(i9,l3+1))then
         do 8249 l4=1,nlosiv(i9)-1
      if(rlo.gt.rlosiv(i9,l4).and.rlo.le.rlosiv(i9,l4+1))then
         rrt=(rlo-rlosiv(i9,l4))/(rlosiv(i9,l4+1)-rlosiv(i9,l4))
         rru=(rla-rlasiv(i9,l3+1))/(rlasiv(i9,l3)-rlasiv(i9,l3+1))
       rsuqiv(i9)=((1-rrt)*(1-rru)*rsuvil(i9,l4,l3+1))+(rrt*(1-rru)*
     1 rsuvil(i9,l4+1,l3+1))+(rrt*rru*rsuvil(i9,l4+1,l3))+((1-rrt)*
     2 rru*rsuvil(i9,l4,l3))
          endif
8249       continue
          endif
8139     continue
c6009     continue
c---if basin = 0 depth or point below basement, use tomo
c rsugiv index of 1 is the salton basement
         if(rde.gt.rsuqiv(1))then
         call makereg(rla,rlo,rde,alp,betm,imanfl)
c -- find moho depth, rdemoh--
          call mohodepth(rla,rlo,rdemoh)
          if(rde.ge.rdemoh)then
          iregfl=0
c---assign upper mantle velocities
          call makeman(rla,rlo,rde,alpm,betm2,imanfl)
          alp=alpm
          betm=betm2
          endif
         return
         endif
c---see if metased basement or crystalline basement
         xref=rmoivx(ivi3)
         yref=rmoivy(ivi3)
         reflat=yref*rd2rad
         do 2179 i=1,ivi3
          y2iv(i) = (rmoivy(i) - yref)*111.1
          x2iv(i) = (rmoivx(i) - xref)*111.1*cos(reflat)
2179      continue
         yp=(rla-yref)*111.1
         xp=(rlo-xref)*111.1*cos(reflat)
         call inside(xp,yp,x2iv,y2iv,ivi3,ins)
c
         if(ins.eq.0)then
c over crystalline basement
         alp=1800.+(430.*(rde/3280.84))
         go to 1181
         endif
c over metaseds
           rscal=(rde)/(rsuqiv(1))
           alp=(rscal*rv(1))+((1.-rscal)*1800.)
c
1181      betm=alp/(sqrt(2.))
c -- all done
          return
          end

          subroutine makereg(rla,rlo,rde,alp,bet,iregfl)
c -- define the regional tomo velocities -----------------------
         include 'regional.h'
         include 'regionald.h'
         dimension vervep(4),verves(4)
         alp = 0.0
         bet = 0.0
         rscal = 0.0
         iinum = 0
         rd2rad=3.141593/180.
c -- find which box point is in--
         do 1927 n=1,nregll-ninrow
          do 8181 j=1,4
          nearn(j)=0
8181      continue
         rckbox=mod(n,ninrow)
c -- avoid trying to make box off row ends
         if(rckbox.eq.0.)go to 1927
         rboxla(1)=reglat(n)
         rboxlo(1)=reglon(n)
         rboxla(2)=reglat(n+1)
         rboxlo(2)=reglon(n+1)
         rboxla(3)=reglat(n+1+ninrow)
         rboxlo(3)=reglon(n+1+ninrow)
         rboxla(4)=reglat(n+ninrow)
         rboxlo(4)=reglon(n+ninrow)
         rboxla(5)=rboxla(1)
         rboxlo(5)=rboxlo(1)
c -- prepare for inside
         xref=rboxlo(nbox)
         yref=rboxla(nbox)
         reflat=yref*rd2rad
         do 8179 i=1,nbox
          y22(i) = (rboxla(i) - yref)*111.1
          x22(i) = (rboxlo(i) - xref)*111.1*cos(reflat)
8179     continue
c -- see if inside this box
          yp=(rla-yref)*111.1
          xp=(rlo-xref)*111.1*cos(reflat)
          call inside(xp,yp,x22,y22,nbox,ins)
          if(ins.eq.1)then
            nearn(1)=(n)
            nearn(2)=(n+1)
            nearn(3)=(n+1+ninrow)
            nearn(4)=(n+ninrow)
           go to 9595
           endif
1927      continue
c--
9595      continue
c-- if not in a box, was near regional
c-- model edge, so use regional 1d velocities
c-- (this uses a modified hadley-kanamori)
        do 6510 kk=1,4
        if(nearn(kk).eq.0)then
         if(rde.lt.reglay(1))then
          alp=reg1dv(1)
          go to 6520
          endif
         if(rde.ge.reglay(nregly))then
          alp=reg1dv(nregly)
          go to 6520
          endif
         do 6509 i=1,nregly-1
          if(rde.ge.reglay(i).and.rde.lt.reglay(i+1))then
           rscal=(rde-reglay(i))/(reglay(i+1)-reglay(i))
           alp=(rscal*reg1dv(i+1))+((1.-rscal)*reg1dv(i))
           go to 6520
          endif
6509     continue
        endif
6510    continue
c-- interpolate from nearest 8 regional model points
c-- above first layer, interpolate from just 4 pts
c-- below bottom layer, interpolate from just 4 pts
c-- figure out what layer
        if(rde.lt.reglay(1))then
         iinum=0
         rscal=0.
         go to 6555
         endif
c changes for moho addition
        if(rde.ge.reglay(nregly-1))then
         iinum=nregly-2
         rscal=0.
         go to 6555
        endif
c-- find layer
        do 6557 i=1,nregly-1
         if(rde.ge.reglay(i).and.rde.lt.reglay(i+1))then
         iinum=i-1
         rscal=(rde-reglay(i))/(reglay(i+1)-reglay(i))
         go to 6555
         endif
6557    continue
c-- find velo by interpolation
6555    continue
c
        do 6868 n=1,4
          vervep(n)=regvep(nearn(n)+(iinum*nregll))
          verves(n)=regves(nearn(n)+(iinum*nregll))
6868    continue
c interp upper layer
        call trint(rboxlo,rboxla,vervep,verves,rlo,rla,velop1,velos1)
        do 6869 n=1,4
          vervep(n)=regvep(nearn(n)+((iinum+1)*nregll))
          verves(n)=regves(nearn(n)+((iinum+1)*nregll))
6869    continue
c interp lower layer
        call trint(rboxlo,rboxla,vervep,verves,rlo,rla,velop2,velos2)
c interp layers
        alp=(rscal*velop2)+((1.-rscal)*velop1)
        bet=(rscal*velos2)+((1.-rscal)*velos1)
c     check Vp/Vs, control by Vp
        bet2=bet*(sqrt(2.))
        if(bet2.gt.alp)bet=alp/(sqrt(2.))
        iregfl=1
6520     return
         end

      subroutine trint (verlon,verlat,vervep,verves,lon,lat,velp,vels)
c interpolate on any quad
c Steve Day 08/00
      dimension verlon(5),verlat(5),vervep(4),verves(4)
      dimension xb(2,2),dvelp(4),dvels(4),g(2,2),ginv(2,2)
      dimension xp(2),xcoef(2),indx(2)
      real lon, lat
      real lon0,lat0
c  find centroids
      lat0=0.
      lon0=0.
      velp0=0.
      vels0=0.
      do 1 m=1,4
        lat0=lat0+verlat(m)
        lon0=lon0+verlon(m)
        velp0=velp0+vervep(m)
        vels0=vels0+verves(m)
    1 continue
      lon0=0.25*lon0
      lat0=0.25*lat0
      velp0=0.25*velp0
      vels0=0.25*vels0
c  form basis vectors
      do 2 m=1,2
        xb(m,1)=verlon(m)-lon0
        xb(m,2)=verlat(m)-lat0
    2 continue
      do 3 m=1,4
        dvelp(m)=vervep(m)-velp0
        dvels(m)=verves(m)-vels0
    3 continue
c Form metric tensor
      do 4 m=1,2
      do 4 n=1,2
        g(m,n)=0.
      do 4 i=1,2
        g(m,n)=g(m,n)+xb(m,i)*xb(n,i)
    4 continue
c Form inverse metric tensor
      detinv=1./(g(1,1)*g(2,2)-g(1,2)*g(2,1))
      ginv(1,1)=detinv*g(2,2)
      ginv(2,2)=detinv*g(1,1)
      ginv(1,2)=-detinv*g(2,1)
      ginv(2,1)=-detinv*g(1,2)
c Form location-point vector (relative to centroid)
      xp(1)=lon-lon0
      xp(2)=lat-lat0
c  Find contrvariant coordinates
      do 5 m=1,2
        xcoef(m)=0.
      do 5 n=1,2
      do 5 k=1,2
    5   xcoef(m)=xcoef(m)+ginv(m,n)*xb(n,k)*xp(k)
c Use signs of the coordinates to determine quadrant
      indx(1)=nint(0.5*(1+sign(1.,xcoef(1)))+1.5*(1-sign(1.,xcoef(1))))
      indx(2)=nint((1+sign(1.,xcoef(2)))+2*(1-sign(1.,xcoef(2))))
c Do the interpolation
      velp=velp0
      vels=vels0
      do 6 i=1,2
      vels=vels+abs(xcoef(i))*dvels(indx(i))
    6 velp=velp+abs(xcoef(i))*dvelp(indx(i))
      return
      end

         subroutine readbore(k2err) 
c--read geotech borehole data-------------- 
         include 'borehole.h'
         character(9) fileib
c---file name assignment-------
         fileib='boreholes'
         k2err=0
c read file
         open(15,file=fileib,status='old',err=2978)
         iprono=0
         ibhct=0
         ieach=0
          do 8101 j=1,maxbh*numbh
          read(15,*,end=2971)rla7,rlo7,rs7,rp7,rd7,is7,ipron7
          ieach=ieach+1
          if(ipron7.ne.iprono)then
          ibhct=ibhct+1
          iprono=ipron7
          rlatbh(ibhct)=rla7
          rlonbh(ibhct)=rlo7
          isotype(ibhct)=is7
           if(ibhct.ge.2)then
           numptbh(ibhct-1)=ieach-1
           rbhdmx(ibhct-1)=rdepbh(ibhct-1,ieach-1)
           endif
          ieach=1
          endif
          rdepbh(ibhct,ieach)=rd7*3.2808399
          rvs(ibhct,ieach)=rs7
          rvp(ibhct,ieach)=rp7
8101     continue
2971      close(15)
          numptbh(ibhct)=ieach
          rbhdmx(ibhct)=rdepbh(ibhct,ieach)
          go to 2915
2978      k2err=1
2915      return
           end

         subroutine readgene(k2err)
c--read generic borehole profiles--------------
         include 'genpro.h'
         character(12) fileig,ag1*50
c---file name assignment-------
         fileig='soil_generic'
         k2err=0
c read file
         open(12,file=fileig,status='old',err=2977)
         do 2300 k=1,numgen
          read(12,*)irt2
          numptge2(k)=irt2
          do 2310 k1=1,irt2
           read(12,*)rvsgen(k,k1),rdepgen(k,k1)
           rdepgen(k,k1)=rdepgen(k,k1)*3.2808399
2310      continue
c skip label line. Label follows data points
          read(12,*)ag1
c find max useful
          irrt=0
          do 2333 i=1,irt2
          irrt=irrt+1
          if(rdepgen(k,i).gt.(rmxdep(k)*3.2808399))then 
           numptgen(k)=irrt
          go to 2300
           endif
2333      continue
2300      continue
          close(12)
          go to 2976
2977      k2err=1
2976      return
           end

         subroutine nearhole(rlat,rlon,isoilt)
c--finds nearby geotech boreholes---
         include 'borehole.h'
         include 'wtbh1.h'
         include 'wtbh1d.h'
         rtemdp=0.
         do 2227 i=1,nrad
         iradct(i)=0
          do 2228 k=1,numbh
          iradbh(i,k)=0
2228      continue
2227     continue
c--loop over boreholes----
         do 667 k=1,numbh
c keep borehole loc in case soil mis-id
          if(rlat.ne.rlatbh(k).and.rlon.ne.rlonbh(k))then
          if(isoilt.ne.isotype(k))go to 667
          endif
          rlod2=(rlon-rlonbh(k))*92.3820
          rlad2=(rlat-rlatbh(k))*110.9220
          rdel2=sqrt((rlod2**2.)+(rlad2**2.))
          if(rdel2.gt.radii(nrad))go to 667
c--count close ones. keep only deepest within first radius
          do 668 l=2,nrad
           if(rdel2.ge.radii(l-1).and.rdel2.lt.radii(l))then
             if(l.eq.2)then
              if(rbhdmx(k).gt.rtemdp)then
              rtemdp=rbhdmx(k)
              iradct(2)=1
              iradbh(2,iradct(2))=k
              else
              go to 668
              endif
             endif
            iradct(l)=iradct(l)+1
            iradbh(l,iradct(l))=k
           endif
668       continue
667      continue  
         return
         end

         subroutine getsoil(rlatl0,rlonl0,isoilt,idgens,idgenp,
     1   inindex,inct)
c--looks up soil type---------------------------
        include 'soil1.h'
        dimension inindex(inct)
        dimension rdelz(nx,ny)
        rdmi2=40.
        iteisb = 0
c
        icolnm=abs(int((rlonmax-rlonl0)/rdelx))
        irownm=int((rlatmax-rlatl0)/rdely)
        indx=((irownm-1)*nx)+icolnm
c check if in ocean
        if(isb(indx).eq.245)then
c look for nearest onshore
           do 6500 l=1,nx
            do 6510 l2=1,ny
            rloix=(l*rdelx)+rlonmax
            rlaix=rlatmax-(l2*rdely)
            rlodix=(rlonl0-rloix)*92.3820
            rladix=(rlatl0-rlaix)*110.9220
            rdelz(l,l2)=sqrt((rlodix**2.)+(rladix**2.))
            if(rdelz(l,l2).lt.rdmi2)then
            indx=((l2-1)*nx)+l
        if(isb(indx).ne.245.and.isb(indx).ne.0)then
             rdmi2=rdelz(l,l2)
             iteisb=indx
             ratx1=rlaix
             ratx2=rloix
            endif
            endif
6510         continue
6500        continue
            indx=iteisb
        endif
c       
        do 9510 i=1,numsoil
        if(isb(indx).eq.igrey(i))then
        isoilt=isoil(i)
        idgens=i
        go to 513
        endif
9510    continue
c have a few off grey scale numbers-look for neighbors
c Look in this order: northwest, west, southwest,
c     north, same, south, northeast, east, southeast
        itemp=indx
        do 524 i=-1,1
         do 525 j=-1,1
         indx=itemp+(nx*i)+j
           do 9519 k=1,numsoil
           if(isb(indx).eq.igrey(k))then
           isoilt=isoil(k)
           idgens=k    
           go to 513
           endif   
9519    continue
525      continue
524     continue
        write(*,*)' soil scale case error, indx,col,row=',indx,
     1  icolnm,irownm,isb(indx)
        write(*,*)'rlatl0, rlonl0=',rlatl0, rlonl0
        write(*,*)'rlatmax,rlatmin,rlonmax,rlonmin,nx,ny,rdelx,rdely=',
     3   rlatmax,rlatmin,rlonmax,rlonmin,nx,ny,rdelx,rdely
        stop
513     continue
c
c simple screens for which basin
c to select specific generic soil profiles
c count backwards from inct to get sfv separate from ventura
        if(isoilt.eq.7)then
        do 6677 k=inct,1,-1
c sfv
         if(inindex(k).le.57.and.inindex(k).ge.47)then
         idgens=13
         go to 5132
         endif
c ventura
         if(inindex(k).le.46)then
         idgens=12
         go to 5132
         endif
c sgv
         if(inindex(k).ge.72)then
         idgens=14
         go to 5132  
         endif
c berdo-chino
        if(inindex(k).le.61.and.inindex(k).ge.58)then
         idgens=15   
         go to 5132   
         endif
6677    continue
        endif
        if(isoilt.eq.5)then
        do 6678 k=inct,1,-1
c sfv
         if(inindex(k).le.57.and.inindex(k).ge.47)then
         idgens=8
         go to 5132
         endif
c ventura
         if(inindex(k).le.46)then
         idgens=7
         go to 5132 
         endif
c sgv
         if(inindex(k).ge.72)then
         idgens=9
         go to 5132  
         endif
c chino - berdo need to split this!
         if(inindex(k).le.61.and.inindex(k).ge.58)then
          idgens=10    
          go to 5132    
          endif
6678      continue
          endif
5132     idgenp=idgens+inums
         return
        end

        subroutine readsoil(k2err)
c--reads soil type info---------------------------------
c Reads a modified .pgm ascii file
        include 'soil1.h'
        character(50) filesb
        k2err = 0
c here's input file name-----------------------------------
        filesb='soil.pgm'
        open(16,file=filesb,status='old',err=5977)
        read(16,*)rlonmax,rlonmin,rlatmax,rlatmin
        read(16,*)nx,ny
c isbct is number of points in file
        isbct=nx*ny
c  compare to dimension of isb, the array that will hold 'em
        if(isbct.gt.isoilbig)then
        write(*,*)'too many soil points-redim array isb in soil1.h'
        stop
        endif
        read(16,5310)(isb(i),i=1,isbct)
5310     format(17(i4))
c  useful numbers
        rdely=(rlatmax-rlatmin)/ny
        rdelx=abs(rlonmax-rlonmin)/nx
        go to 5976
5977    k2err=1
5976    return
        end

       subroutine addtops(rdep2,rvelo,idgen,ifs,rveln,ioldfg,ioff)
c -- returns S velocity from 'geotech' constraints
c rvelo=current velocity from main code
c rveln=new (from here) velocity passed back
c iradct array=number of nearby boreholes
c iradcts=number of nearby boreholes with data
         include 'borehole.h'
         include 'genpro.h'
         include 'wtbh1.h'
         include 'wtbh2.h'
         roff = 0.0
         rvte3 = 0.0
         rvte8 = 0.0
         ihtfg=0
         rtvelges=0.
         rdep=rdep2
         do 7013 n=1,nrad
         iradcts(n)=0
         radvs(n)=0.
         rtvels(n) = 0.0
         rtewts(n) = 0.0
7013     continue
c--check ifs flag--
         if(ifs.eq.0)then
          if(ioff.eq.0)then
          rdep=rmxdep(idgen)*3.2808399
          else
          go to 133
          endif
         endif
          do 7011 l=2,nrad
           do 7012 i=1,iradct(l)
           k=iradbh(l,i)
            do 669 n=2,numptbh(k)
c--check for data at this depth
            if(l.eq.2.)then
             if(rdep2.ge.rbhdmx(k))then
             rv1=rvs(k,numptbh(k)-1)
             ihtfg=1
             go to 133 
             else
             rdep=rdep2
             endif
            endif
            if(rdep.ge.rdepbh(k,n-1).and.rdep.lt.rdepbh(k,n))then
             rva=rvs(k,n-1)
            if(rva.ne.0.)then
c this gives borehole within 50 m
             if(l.eq.2)then
             rveln=rva
             go to 671
             else
             iradcts(l)=iradcts(l)+1
             radvs(l)=radvs(l)+rva
             go to 669
             endif
            endif
           endif
669         continue
7012       continue
7011      continue
c--weight for velocity--------------------------
         do 670 j=3,nrad
         if(iradcts(j).ne.0)then
          rtvels(j)=radvs(j)/iradcts(j)
          rtewts(j)=radwt(j)
          else
          rtewts(j)=0.
         endif
670      continue
c--get generic profile velocity--------------
         do 870 n=2,numptgen(idgen)
        if(rdep.ge.rdepgen(idgen,n-1).and.rdep.lt.rdepgen(idgen,n))then
          rtvelges=rvsgen(idgen,n-1)
          go to 871
          endif
870      continue
871      continue
c--get the velocities--------------------------
         rb=0.
         rscfac=1./((nrad-3)+1)
c better always have generic velo
         do 1110 n=3,nrad
         rb=rb+(((rtewts(n)*rtvels(n))+((1.-rtewts(n))*rtvelges))
     1   *rscfac)
1110     continue
         rveln=rb
         if(ifs.eq.1)go to 671
c--below generics so continue gradient; compare to rule based ---
133      continue
         if(ihtfg.eq.0)rv1=rveln
c find continued generic for this depth
         do 8702 n=2,numptge2(idgen)
       if(rdep.ge.rdepgen(idgen,n-1).and.rdep.lt.rdepgen(idgen,n))then
          if(ioff.eq.0)then
           rvte4=rvsgen(idgen,n-1)
           roff=rv1-rvte4
           ioff=ioff+1
            do 87022 i=2,numptge2(idgen)
            if(rdep2.ge.rdepgen(idgen,i-1).and.rdep2.lt.
     1      rdepgen(idgen,i))then
            rvte8=rvsgen(idgen,i-1)
            go to 8777
            endif
87022       continue
8777       rvte3=rvte8+roff
           go to 8712
           else
           rvte5=rvsgen(idgen,n-1)
           rvte3=rvte5+roff
           endif
          go to 8712
          endif
8702      continue
c compare to rule based
8712     if(rvte3.le.rvelo)then
         rveln=rvte3
         go to 6711
         else
         rveln=rvelo
         go to 6711
         endif
c--done
671      continue
c remember a number to interpolate from 
            if(ioldfg.eq.1)then
             rvsold=rveln
             rdepold=rdep
            endif
6711      continue
         return
         end

         subroutine addtopp(rdep,rvelo,idgen,ifs,rveln,ioldfg)
c -- returns P velocity from 'geotech' constraints
c rvelo=current velocity from main code
c rveln=new (from here) velocity passed back
c iradct array=number of nearby boreholes
c iradctp=number of nearby boreholes with data
         include 'borehole.h'
         include 'genpro.h'
         include 'wtbh1.h'
         include 'wtbh3.h'
         rtvelges=0.
         rva = 0.0
         do 97013 n=1,nrad
         iradctp(n)=0
         radvp(n)=0.
         rtvelp(n) = 0.0
         rtewtp(n) = 0.0
97013     continue
          do 97011 l=2,nrad
           do 97012 i=1,iradct(l)
           k=iradbh(l,i)
            do 9669 n=2,numptbh(k)
c--check for data at this depth
            if(rdep.ge.rdepbh(k,n-1).and.rdep.lt.rdepbh(k,n))then
             rva=rvp(k,n-1)
c
            if(rva.ne.0.)then
c this gives borehole within 50 m
             if(l.eq.2)then
             rveln=rva
             go to 9671
             else
             iradctp(l)=iradctp(l)+1
             radvp(l)=radvp(l)+rva
             go to 9669
             endif
            endif
            endif
9669         continue
97012       continue
97011      continue
c--check ifs flag--
         if(ifs.eq.0)go to 9133
c--weight for velocity--------------------------
         do 9670 j=3,nrad
         if(iradctp(j).ne.0)then
          rtvelp(j)=radvp(j)/iradctp(j)
          rtewtp(j)=radwt(j)
          else
          rtewtp(j)=0.
         endif
9670      continue
c--get generic profile velocity--------------
         do 9870 n=2,numptgen(idgen)
        if(rdep.ge.rdepgen(idgen,n-1).and.rdep.lt.rdepgen(idgen,n))then
          rtvelges=rvsgen(idgen,n-1)
          go to 9871
          endif
9870      continue
9871      continue
c--get the velocities--------------------------
         rb=0.
         rscfac=1./((nrad-3)+1)
c better always have generic velo
         do 91110 n=3,nrad
         rb=rb+(((rtewtp(n)*rtvelp(n))+((1.-rtewtp(n))*rtvelges))
     1   *rscfac)
91110     continue
         rveln=rb
         go to 9671
c--between generics and geotech depth, so taper---
9133      continue
         rxxdep=(rmxdep(idgen))*3.2808399
         rtfac=((rdep-rxxdep)/(656.17-rxxdep))
         if(rtfac.gt.1.)rtfac=1.
         rtfac2=cos(rtfac*(3.141592654/2.))
         rv1=rvsgen(idgen,numptgen(idgen))
         rv2=rvelo
         rvte3=(rtfac2*rv1)+((1.-rtfac2)*rv2)
         rveln=rvte3
         go to 96711
c--done
9671      continue
c remember a better number to interpolate from rather than hard generic
            if(ioldfg.eq.1)then
             rvpold=rveln
             rdepold=rdep
            endif
96711      continue
         return
         end

        subroutine readmoho(k2err)
c--reads the moho surface file--------------------------
        include 'moho1.h'
        character(50) filemo
        k2err = 0
c here's input file name-----------------------------------
        filemo='moho_sur'
        open(16,file=filemo,status='old',err=3233)
        do 3235 i=1,imohla
         do 3236 k=1,imohlo
         read(16,*)rmohlo(k),rmohla(i),rmohde(k,i)
c convert from km to feet
         rmohde(k,i)=rmohde(k,i)*3.28084*1000.
3236     continue
3235    continue
        close(16)
        go to 3234
3233    k2err=1
3234    return
        end

         subroutine mohodepth(rla,rlo,rdemoh)
c--finds moho depth
        include 'moho1.h'
        dimension rrtemp(imohlo,imohla)
         do 3313 n3=1,imohla-1
         if(rla.le.rmohla(n3).and.rla.gt.rmohla(n3+1))then
         do 3324 n4=1,imohlo-1
         if(rlo.gt.rmohlo(n4).and.rlo.le.rmohlo(n4+1))then
         rrx=(rlo-rmohlo(n4))/(rmohlo(n4+1)-rmohlo(n4))
         rry=(rla-rmohla(n3+1))/(rmohla(n3)-rmohla(n3+1))
c--note here rmohde indexes are (long, lat)------
         rdemoh=((1-rrx)*(1-rry)*rmohde(n4,n3+1))+(rrx*(1-rry)*
     1       rmohde(n4+1,n3+1))+(rrx*rry*rmohde(n4+1,n3))+((1-rrx)*
     2       rry*rmohde(n4,n3))
          go to 3315
          endif
3324      continue
          endif
3313      continue
c fall here if point is outside of moho definition
c as may happen for southern Salton Trough,
c find nearest moho definition and use that
c  going counterclockwise around moho surface from SW corner
           rtedis=1000.
           do 5101 k=1,imohlo
           rrtelo=(rlo-rmohlo(k))*92.3820
           rrtela=(rla-rmohla(imohla))*110.9220
           rrtemp(k,imohla)=sqrt((rrtelo**2.)+(rrtela**2.))
           if(rrtemp(k,imohla).lt.rtedis)then
           rtedis=rrtemp(k,imohla)
           rdemoh=rmohde(k,imohla)
           endif
5101       continue
           do 5102 k2=imohla,1,-1
           rrtelo=(rlo-rmohlo(imohlo))*92.3820
           rrtela=(rla-rmohla(k2))*110.9220
           rrtemp(imohlo,k2)=sqrt((rrtelo**2.)+(rrtela**2.))
           if(rrtemp(imohlo,k2).lt.rtedis)then 
           rtedis=rrtemp(imohlo,k2) 
           rdemoh=rmohde(imohlo,k2) 
           endif
5102       continue
           do 5103 k3=imohlo,1,-1
           rrtelo=(rlo-rmohlo(k3))*92.3820
           rrtela=(rla-rmohla(1))*110.9220
           rrtemp(k3,imohla)=sqrt((rrtelo**2.)+(rrtela**2.))
           if(rrtemp(k3,imohla).lt.rtedis)then  
           rtedis=rrtemp(k3,imohla)  
           rdemoh=rmohde(k3,imohla)  
           endif
5103       continue
           do 5104 k4=1,imohla
           rrtelo=(rlo-rmohlo(1))*92.3820
           rrtela=(rla-rmohla(k4))*110.9220
           rrtemp(imohlo,k4)=sqrt((rrtelo**2.)+(rrtela**2.))
           if(rrtemp(imohlo,k4).lt.rtedis)then   
           rtedis=rrtemp(imohlo,k4)   
           rdemoh=rmohde(imohlo,k4)   
           endif
5104       continue
3315      return
          end

          subroutine readman(kerr)
c -- read upper mantle model info---------------------------
c Using Monica Kohler's so cal model at xx km horizontal
c  spacing, 10 km vertical spacing
         include 'mantle.h'
         character(1) atra
         kerr=0
         open(29,file='3D.out',status='old',err=2998)
         do 1129 j=1,nmanll
         read(29,1828)rmanlo(j),rmanla(j)
1129     continue    
         rewind(29)
         do 1122 j2=1,nmanv
         read(29,1829)rmanvp(j2),rmanvs(j2)
c -- convert to m/s  
         rmanvp(j2)=rmanvp(j2)*1000.
         rmanvs(j2)=rmanvs(j2)*1000.
1122     continue
         rewind(29)
         j4=0
         do 1125 j3=1,nmanly
          j4=j4+1
          read(29,1839)rmalay(j4)
           do 1126 j5=1,nmanll-1
1126       read(29,'(a1)')atra
         rmalay(j4)=rmalay(j4)*3.28084
1125     continue
         close(29)
         go to 1921
1828     format(f10.4,f10.4)
1829     format(t32,f6.2,1x,f6.2)
1839     format(t24,f7.0)
2998     kerr=1
1921     return
         end

          subroutine makeman(rla,rlo,rde,alpm,betm,imanfl)
c -- define the upper mantle velocities -----------------------
         include 'mantle.h'
         include 'mantled.h'
         dimension vemanp(4),vemans(4)
         rd2rad=3.141593/180.
c -- find which box point is in--
         do 1227 n=1,nmanll-mancol
          do 8182 j=1,4
          nearm(j)=0
8182      continue
         rmkbox=mod(n,mancol)
c -- avoid trying to make box off column ends
         if(rmkbox.eq.0.)go to 1227
         rmoxla(1)=rmanla(n)
         rmoxlo(1)=rmanlo(n)
         rmoxla(2)=rmanla(n+1)
         rmoxlo(2)=rmanlo(n+1)
         rmoxla(3)=rmanla(n+1+mancol)
         rmoxlo(3)=rmanlo(n+1+mancol)
         rmoxla(4)=rmanla(n+mancol)
         rmoxlo(4)=rmanlo(n+mancol)
         rmoxla(5)=rmoxla(1)
         rmoxlo(5)=rmoxlo(1)
c -- prepare for inside
         xrem=rmoxlo(mbox)
         yrem=rmoxla(mbox)
         remlat=yrem*rd2rad
         do 8129 i=1,mbox
          ym2(i) = (rmoxla(i) - yrem)*111.1
          xm2(i) = (rmoxlo(i) - xrem)*111.1*cos(remlat)
8129     continue
c -- see if inside this box
          ypm=(rla-yrem)*111.1
          xpm=(rlo-xrem)*111.1*cos(remlat)
          call inside(xpm,ypm,xm2,ym2,mbox,ins)
          if(ins.eq.1)then
            nearm(1)=(n)
            nearm(2)=(n+1)
            nearm(3)=(n+1+mancol)
            nearm(4)=(n+mancol)
           go to 9596
           endif
1227      continue
c--
9596      continue
c-- if not in a box, was near regional
c-- model edge, so use regional 1d velocities
c assign a dummy betm
        do 6512 kk=1,4
        if(nearm(kk).eq.0)then
         if(rde.lt.rma1dd(1))then
          alpm=rma1dv(1)
          betm=alpm/1.73
          go to 6522
          endif
         if(rde.ge.rma1dd(nman1d))then
          alpm=rma1dv(nman1d)
          betm=alpm/1.73
          go to 6522
          endif
         do 6529 i=1,nman1d-1
          if(rde.ge.rma1dd(i).and.rde.lt.rma1dd(i+1))then
           rscal=(rde-rma1dd(i))/(rma1dd(i+1)-rma1dd(i))
           alpm=(rscal*rma1dv(i+1))+((1.-rscal)*rma1dv(i))
           betm=alpm/1.73
           go to 6522
          endif
6529     continue
        endif
6512    continue
c-- interpolate from nearest 8 regional model points
c-- find layer
        do 6552 i=1,nmanly-1
         if(rde.ge.rmalay(i).and.rde.lt.rmalay(i+1))then
         imnum=i-1
         rscal=(rde-rmalay(i))/(rmalay(i+1)-rmalay(i))
         go to 6553
         endif
6552    continue
c-- if below
         imnum=nmanly-2
         rscal=1.
6553    continue
c
        do 6862 n=1,4
          vemanp(n)=rmanvp(nearm(n)+(imnum*nmanll))
          vemans(n)=rmanvs(nearm(n)+(imnum*nmanll))
6862    continue
c interp upper layer
        call trint(rmoxlo,rmoxla,vemanp,vemans,rlo,rla,vemop1,vemos1)
        do 6863 n=1,4
          vemanp(n)=rmanvp(nearm(n)+((imnum+1)*nmanll))
          vemans(n)=rmanvs(nearm(n)+((imnum+1)*nmanll))
6863    continue
c interp lower layer
        call trint(rmoxlo,rmoxla,vemanp,vemans,rlo,rla,vemop2,vemos2)
c interp layers
        alpm=(rscal*vemop2)+((1.-rscal)*vemop1)
        betm=(rscal*vemos2)+((1.-rscal)*vemos1)
c     check Vp/Vs, control by Vp
        bet2=betm*(sqrt(2.))
        if(bet2.gt.alpm)betm=alpm/(sqrt(2.))
        imanfl=1
6522     return
         end
