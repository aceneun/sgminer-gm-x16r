#ifndef AERGO_H
#define AERGO_H

#include "miner.h"

extern int aergo_test(unsigned char *pdata, const unsigned char *ptarget,
			uint32_t nonce);
extern void aergo_regenhash(struct work *work);

#endif /* AERGO_H */