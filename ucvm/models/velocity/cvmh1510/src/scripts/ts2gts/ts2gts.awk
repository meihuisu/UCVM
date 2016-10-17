BEGIN{i = j = 0}
$1 ~ "VRTX" {vi[i]=$2; vx[i]=$3; vy[i]=$4; vz[i]=$5; i++}
$1 ~ "TRGL" {tr1[j]=$2; tr2[j]=$3; tr3[j]=$4; j++}
END{
print i, j*3, j
for (a=0; a<i; a++) {
print vx[a], vy[a], vz[a]
}
for (a=0; a<j; a++) {
print tr1[a], tr2[a]
print tr2[a], tr3[a]
print tr3[a], tr1[a]
}
for (a=0; a<j*3; a=a+3) {
print a+1, a+2, a+3
}
}
