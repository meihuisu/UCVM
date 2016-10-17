#ifndef VX_VOXET_H
#define VX_VOXET_H


struct axis
    {
     float O[3];
     float U[3];
     float V[3];
     float W[3];
     float MIN[3];
     float MAX[3];
     int N[3];
};

struct flags
    {
     int ARRAY_LENGTH;
     int BIT_LENGTH;
     int ESIZE;
     int OFFSET;
     char FN[20];
    };

struct property
    {
     char NAME[20];
     float NO_DATA_VALUE;
     int ESIZE;
     int SIGNED;
     char ETYPE[5];
     int OFFSET;
     char FN[20];
    };
 
     
union zahl { float z;
             char c[4];
           };


#endif
