c include file genpro.h
c contains soil generic profile info
c numgen = number generic profiles
c rmxdep = max depth of s,p generic profiles
c numptgen = number well constrained pts in each generic profile
c numptge2 = number total pts in each generic profile
         parameter (numgen=30,mxptgen=201)
         common /genstuff/ rmxdep(numgen),numptgen(numgen),
     1   rdepgen(numgen,mxptgen), rvsgen(numgen,mxptgen),
     2   numptge2(numgen)
c see soil1.h for generic names
