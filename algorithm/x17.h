#ifndef X17_H
#define X17_H

#include "miner.h"

extern int x17_test(unsigned char *pdata, const unsigned char *ptarget,
                    uint32_t nonce);
extern void x17_regenhash(struct work *work);

#endif /* X17_H */
