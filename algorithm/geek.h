#ifndef GEEK_H
#define GEEK_H

#include "miner.h"

extern int geek_test(unsigned char *pdata, const unsigned char *ptarget,
			uint32_t nonce);
extern void geek_regenhash(struct work *work);

#endif /* GEEK_H */
