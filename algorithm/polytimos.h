
#ifndef POLYTIMOS_H
#define POLYTIMOS_H

#include "miner.h"

extern int polytimos_test(unsigned char *pdata, const unsigned char *ptarget,
			uint32_t nonce);
extern void polytimos_regenhash(struct work *work);

#endif
