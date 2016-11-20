c borehole include file
c contains info on geotechnical (near surface)
c boreholes
        parameter (numbh=367,maxbh=64)
        common /boring/ rlatbh(numbh),rlonbh(numbh),isotype(numbh),
     1  numptbh(numbh),rdepbh(numbh,maxbh),rvs(numbh,maxbh),
     2  rvp(numbh,maxbh),rbhdmx(numbh)
