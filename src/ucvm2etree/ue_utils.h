#ifndef UE_UTILS_H
#define UE_UTILS_H

#include "etree.h"

/* Used in addr to key conversion */
static const int theMaxLevelP1 = ETREE_MAXLEVEL + 1;


/* Convert etree address to key */
int ue_addr2key(etree_addr_t addr, void *key);


/* Convert etree key to address */
int ue_key2addr(void *key, etree_addr_t *paddr);


#endif

