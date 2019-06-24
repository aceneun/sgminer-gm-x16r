#ifndef C11_H
#define C11_H

#include "miner.h"

extern int c11_test(unsigned char *pdata, const unsigned char *ptarget,
			uint32_t nonce);
extern void c11_regenhash(struct work *work);

#endif /* C11_H */
