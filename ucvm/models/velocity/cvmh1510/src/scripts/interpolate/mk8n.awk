{
utmX = $4; utmY=$5; Z=$3 
cellX=$12; cellY=$13; cellZ=$14
flag=$11;
dx = ((utmX - cellX)<0)?-1:1
dy = ((utmY - cellY)<0)?-1:1
dz = ((Z - cellY)<0)?-1:1
stx = 1000; sty = stx; stz = 100;
if (flag=="hr") {stx=250; sty=stx; stz=100;}
if (flag=="cm") {stx=10000; sty=10000; stz=1000;}
stx = stx * dx
sty = sty * dy
stz = stz * dz
print utmX, utmY, Z;
print utmX+stx, utmY, Z;
print utmX+stx, utmY+sty, Z;
print utmX, utmY+sty, Z;
print utmX, utmY, Z+stz;
print utmX+stx, utmY, Z+stz;
print utmX+stx, utmY+sty, Z+stz;
print utmX, utmY+sty, Z+stz;
}
