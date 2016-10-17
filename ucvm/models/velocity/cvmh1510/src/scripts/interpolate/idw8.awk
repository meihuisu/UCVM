BEGIN {p=2}
NR%8 == 1 {
 X=$1; Y=$2; Z=$3
 svp=0; sw=0
}

{
 cX=$12; cY=$13; cZ=$14
 tag=$15; vp=$16; vs=$17; rho=$18
 dst=sqrt((cX-X)^2+(cY-Y)^2+(cZ-Z)^2)+0.0001
 w=dst^-p
 svp += w*vp
 sw += w
}

NR%8 == 0 {
 avp = svp/sw
 print X, Y, Z, avp
}
