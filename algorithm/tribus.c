/*-
 * Copyright 2014 phm
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"
#include "miner.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sph/sph_jh.h"
#include "sph/sph_keccak.h" 
#include "sph/sph_echo.h"

static void trihash(void *state, const void *input)
{
    sph_jh512_context        ctx_jh;
    sph_keccak512_context    ctx_keccak;
    sph_echo512_context      ctx_echo;
    
    unsigned char hash[64];

    // JH;
    sph_jh512_init(&ctx_jh);
    sph_jh512 (&ctx_jh, input, 80);
    sph_jh512_close(&ctx_jh, (void*) hash);

    // KECCAK;
    sph_keccak512_init(&ctx_keccak);
    sph_keccak512 (&ctx_keccak, (const void*) hash, 64);
    sph_keccak512_close(&ctx_keccak, (void*) hash);

    // ECHO
    sph_echo512_init(&ctx_echo);
    sph_echo512 (&ctx_echo, (const void*) hash, 64);
    sph_echo512_close(&ctx_echo, (void*) hash);

    memcpy(state, hash, 32);
}

void precalc_hash_tribus(dev_blk_ctx *blk, uint32_t *midstate, uint32_t *pdata)
{
  uint32_t data[20];
  sph_jh512_context ctx_jh;

  flip80(data, pdata);
  sph_jh512_init(&ctx_jh);
  sph_jh512(&ctx_jh, data, 64);
  if (midstate) memcpy(midstate, &ctx_jh.H.narrow[0], 128);
}

void tribus_regenhash(struct work *work)
{
    uint32_t data[20];
    char *scratchbuf;
    uint32_t *nonce = (uint32_t *)(work->data + 76);
    uint32_t *ohash = (uint32_t *)(work->hash);

    be32enc_vect(data, (const uint32_t *)work->data, 19);
    data[19] = htobe32(*nonce);
    trihash(ohash, data);
}


static const uint32_t diff1targ = 0x0000ffff;

/* Used externally as confirmation of correct OCL code */
int tribus_test(unsigned char *pdata, const unsigned char *ptarget, uint32_t nonce)
{
	uint32_t tmp_hash7, Htarg = le32toh(((const uint32_t *)ptarget)[7]);
	uint32_t data[20], ohash[8];
	//char *scratchbuf;

	be32enc_vect(data, (const uint32_t *)pdata, 19);
	data[19] = htobe32(nonce);
	//scratchbuf = alloca(SCRATCHBUF_SIZE);
	trihash(ohash, data);
	tmp_hash7 = be32toh(ohash[7]);

	applog(LOG_DEBUG, "htarget %08lx diff1 %08lx hash %08lx",
				(long unsigned int)Htarg,
				(long unsigned int)diff1targ,
				(long unsigned int)tmp_hash7);
	if (tmp_hash7 > diff1targ)
		return -1;
	if (tmp_hash7 > Htarg)
		return 0;
	return 1;
}

bool scanhash_tribus(struct thr_info *thr, const unsigned char __maybe_unused *pmidstate,
		     unsigned char *pdata, unsigned char __maybe_unused *phash1,
		     unsigned char __maybe_unused *phash, const unsigned char *ptarget,
		     uint32_t max_nonce, uint32_t *last_nonce, uint32_t n)
{
	uint32_t *nonce = (uint32_t *)(pdata + 76);
	char *scratchbuf;
	uint32_t data[20];
	uint32_t tmp_hash7;
	uint32_t Htarg = le32toh(((const uint32_t *)ptarget)[7]);
	bool ret = false;

	be32enc_vect(data, (const uint32_t *)pdata, 19);

	while(1) {
		uint32_t ostate[8];

		*nonce = ++n;
		data[19] = (n);
		trihash(ostate, data);
		tmp_hash7 = (ostate[7]);

		applog(LOG_INFO, "data7 %08lx",
					(long unsigned int)data[7]);

		if (unlikely(tmp_hash7 <= Htarg)) {
			((uint32_t *)pdata)[19] = htobe32(n);
			*last_nonce = n;
			ret = true;
			break;
		}

		if (unlikely((n >= max_nonce) || thr->work_restart)) {
			*last_nonce = n;
			break;
		}
	}

	return ret;
}


