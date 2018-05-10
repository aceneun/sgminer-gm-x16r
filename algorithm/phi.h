#ifndef PHI_H
#define PHI_H

#include "miner.h"

extern int phi_test(unsigned char *pdata, const unsigned char *ptarget,
			uint32_t nonce);
extern void phi_prepare_work(dev_blk_ctx *blk, uint32_t *state, uint32_t *pdata);
extern void phi_regenhash(struct work *work);
extern void phi_midstate(struct work *work);

#endif /* PHI_H */