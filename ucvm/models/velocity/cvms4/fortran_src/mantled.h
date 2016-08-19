c mantled.h
c more upper mantle model info
c layer depths in m
c-- these parameters deal with searching over boxes of mantle model
c and mantle 1d outside of 3d
         parameter(mancol=21, mbox=5, nman1d=6)
         dimension nearm(mbox-1),rmoxla(mbox),rmoxlo(mbox)
         dimension xm2(mbox),ym2(mbox),rma1dd(nman1d),rma1dv(nman1d)
         data(rma1dv(l),l=1,nman1d)/6800.,7800.,8100.,8200.,8300.,
     1   8400./
         data(rma1dd(k),k=1,nman1d)/65617.,98425.,262467.,656168.,
     1   721785.,787402./
