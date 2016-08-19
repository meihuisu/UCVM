c mantle.h
c --  mantle model info---------------------------
c  nmanll = number points per layer of mantle model
c  nmanly = number layers in mantle model
c Using Monica Kohler's so cal model at xx km horizontal
c  spacing, 10 km vertical spacing
         parameter(nmanll=441,nmanly=24,nmanv=nmanll*nmanly)
         common /mantle/rmanvp(nmanv),rmanvs(nmanv),rmanla(nmanll),
     1   rmanlo(nmanll),rmalay(nmanly)
