BEGIN {p=2}
{
#assumes x y z tag pasted before the 8 neighbour records in one line
 tag=$4; X=$5; Y=$6; Z=$7;
 svp=0; sw=0;
#go through all 8 neighbours and use just the ones with correct tag
 for (i=0; i<=7; i++) {
   if ($(i*19+4+16)==tag) {
     cX=$(i*19+4+13); cY=$(i*19+4+14); cZ=$(i*19+4+15)
     vp=$(i*19+4+17); vs=$(i*19+4+18); rho=$(i*19+4+19)
     dst=sqrt((cX-X)^2+(cY-Y)^2+(cZ-Z)^2)+0.0001
     w=dst^-p
     svp += w*vp
     sw += w
   }
 }
 avp=-99999  
 avp=(sw>0)?svp/sw:-99999
 print X, Y, Z, tag, avp
}

