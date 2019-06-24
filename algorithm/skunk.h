#ifndef SKUNKHASH_H
#define SKUNKHASH_H

#include "miner.h"

void precalc_hash_skunk(dev_blk_ctx *blk, uint32_t *midstate, uint32_t *pdata);
void skunk_regenhash(struct work *work);
extern void skunk_prepare_work(dev_blk_ctx *blk, uint32_t *state, uint32_t *pdata);
extern void skunk_midstate(struct work *work);

#endif
