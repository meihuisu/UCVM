c regional.h
c --  regional model info---------------------------
c  nregll = number points per layer of regional model
c  nregv  = total number P or S velocities in regional model
c  nregly = number layers in regional model
c Using Egill Hauksson's so cal model at 15 km horizontal
c  spacing, variable vertical spacing
         parameter(nregll=1107,nregv=9963,nregly=9)
         common /region/regvep(nregv),regves(nregv),reglat(nregll),
     1   reglon(nregll),reglay(nregly),reg1dv(nregly)
