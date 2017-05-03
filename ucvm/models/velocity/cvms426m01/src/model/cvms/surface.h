c  surface.h   contains reference surface numbers, age,
c              uplift and edge info
c i3 number x,y pairs in model edge
c numsur number reference surfaces
c isurmx  max number surfaces for a pt
c ilahi, ilohi max number lat, lon points in surface depth files
c nedmx  max number pairs of points in surface edge files
         parameter(numsur=74,ilahi=400,ilohi=580,i3=126,isurmx=15)
         parameter (nedmx=686)
         common /rsurfs/ rlosur(numsur,ilohi), rlasur(numsur,ilahi),
     1   nlosur(numsur), nlasur(numsur), rsuval(numsur,ilohi,ilahi)
         common /edges/ nedge(numsur),rtx(numsur,nedmx),
     1   rty(numsur,nedmx)
         common/rmedge/rmodtx(i3),rmodty(i3)
