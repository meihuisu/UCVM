BEGIN{n=0}
{n++}
n==1{n1=$0}
n==2{n2=$0}
n==3{n=0;n3=$0;
$0=n1;
if ($17>1) {print n1; next}
$0=n2;
if ($17>1) {print n2; next}
$0=n3;
if ($17>1) {print n3; next}
{print "something is wrong", n3}
}
