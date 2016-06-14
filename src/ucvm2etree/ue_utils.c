#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ue_utils.h"
#include "code.h"
#include "etree.h"


/* Convert etree address to key */
int ue_addr2key(etree_addr_t addr, void *key)
{
  if (addr.level >= theMaxLevelP1) 
    return -1;
  
  /* Use the newer version */
  code_coord2morton(theMaxLevelP1, addr.x, addr.y, addr.z, (char *)key + 1); 
  
  *(unsigned char *)key = (unsigned char)addr.level;
  if (addr.type == ETREE_LEAF) 
    *(unsigned char *)key |= 0x80;
  
  return 0;
}


/* Convert etree key to address */
int ue_key2addr(void *key, etree_addr_t *paddr)
{
  unsigned char LSB;
  int level;
  
  LSB = *(unsigned char *)key;
  level = LSB & 0x7F;
  
  if (level >= theMaxLevelP1) 
    return -1;
  
  paddr->level = level;
  paddr->type = (LSB & 0x80) ? ETREE_LEAF : ETREE_INTERIOR;
  code_morton2coord(theMaxLevelP1, (char *)key + 1, 
                    &paddr->x, &paddr->y, &paddr->z);
  
  return 0;
}
