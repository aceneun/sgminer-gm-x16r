#ifndef BLAKE256_H
#define BLAKE256_H

#include "miner.h"

extern int blake256_test(unsigned char *pdata, const unsigned char *ptarget, uint32_t nonce);
extern void blake256_prepare_work(dev_blk_ctx *blk, uint32_t *state, uint32_t *pdata);
extern void blake256_midstate(struct work *work);
extern void blake256_regenhash(struct work *work);

#endif /* BLAKE256_H */