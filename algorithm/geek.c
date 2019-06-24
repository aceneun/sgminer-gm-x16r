/*-
 * Copyright 2009 Colin Percival, 2011 ArtForz
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
 *
 * This file was originally written by Colin Percival as part of the Tarsnap
 * online backup system.
 */

#include "config.h"
#include "miner.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#include "sph/sph_blake.h"
#include "sph/sph_bmw.h"
#include "sph/sph_echo.h"
#include "sph/sph_shabal.h"
#include "sph/sph_groestl.h"
#include "sph/sph_cubehash.h"
#include "sph/sph_keccak.h"
#include "sph/sph_hamsi.h"
#include "sph/sph_simd.h"

static void geekhash(void *state, const void *input)
{
	sph_blake512_context     ctx_blake;
	sph_bmw512_context       ctx_bmw;
	sph_groestl512_context   ctx_groestl;
	sph_keccak512_context    ctx_keccak;
	sph_cubehash512_context	ctx_cubehash1;
	sph_echo512_context		ctx_echo1;
	sph_shabal512_context       ctx_shabal1;
	sph_simd512_context		ctx_simd1;
	sph_hamsi512_context	ctx_hamsi1;


	//these uint512 in the c++ source of the client are backed by an array of uint32
	uint32_t hashA[16], hashB[16];

	sph_blake512_init(&ctx_blake);
	sph_blake512(&ctx_blake, input, 80);
	sph_blake512_close(&ctx_blake, hashA);

	sph_bmw512_init(&ctx_bmw);
	sph_bmw512(&ctx_bmw, hashA, 64);
	sph_bmw512_close(&ctx_bmw, hashB);

	sph_echo512_init(&ctx_echo1);
	sph_echo512(&ctx_echo1, hashB, 64);
	sph_echo512_close(&ctx_echo1, hashA);

	sph_shabal512_init(&ctx_shabal1);
	sph_shabal512(&ctx_shabal1, hashA, 64);
	sph_shabal512_close(&ctx_shabal1, hashB);

	sph_groestl512_init(&ctx_groestl);
	sph_groestl512(&ctx_groestl, hashB, 64);
	sph_groestl512_close(&ctx_groestl, hashA);

	sph_cubehash512_init(&ctx_cubehash1);
	sph_cubehash512(&ctx_cubehash1, hashA, 64);
	sph_cubehash512_close(&ctx_cubehash1, hashB);

	sph_keccak512_init(&ctx_keccak);
	sph_keccak512(&ctx_keccak, hashB, 64);
	sph_keccak512_close(&ctx_keccak, hashA);

	sph_hamsi512_init(&ctx_hamsi1);
	sph_hamsi512(&ctx_hamsi1, hashA, 64);
	sph_hamsi512_close(&ctx_hamsi1, hashB);

	sph_simd512_init(&ctx_simd1);
	sph_simd512(&ctx_simd1, hashB, 64);
	sph_simd512_close(&ctx_simd1, hashA);

	memcpy(state, hashA, 32);
}

static const uint32_t diff1targ = 0x0000ffff;


/* Used externally as confirmation of correct OCL code */
int geek_test(unsigned char *pdata, const unsigned char *ptarget, uint32_t nonce)
{
	uint32_t tmp_hash7, Htarg = le32toh(((const uint32_t *)ptarget)[7]);
	uint32_t data[20], ohash[8];

	be32enc_vect(data, (const uint32_t *)pdata, 19);
	data[19] = htobe32(nonce);
	geekhash(ohash, data);
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

void geek_regenhash(struct work *work)
{
	uint32_t data[20];
	uint32_t *nonce = (uint32_t *)(work->data + 76);
	uint32_t *ohash = (uint32_t *)(work->hash);

	be32enc_vect(data, (const uint32_t *)work->data, 19);
	data[19] = htobe32(*nonce);
	geekhash(ohash, data);
}

bool scanhash_geek(struct thr_info *thr, const unsigned char __maybe_unused *pmidstate,
	unsigned char *pdata, unsigned char __maybe_unused *phash1,
	unsigned char __maybe_unused *phash, const unsigned char *ptarget,
	uint32_t max_nonce, uint32_t *last_nonce, uint32_t n)
{
	uint32_t *nonce = (uint32_t *)(pdata + 76);
	uint32_t data[20];
	uint32_t tmp_hash7;
	uint32_t Htarg = le32toh(((const uint32_t *)ptarget)[7]);
	bool ret = false;

	be32enc_vect(data, (const uint32_t *)pdata, 19);

	while (1) {
		uint32_t ostate[8];
		*nonce = ++n;
		data[19] = (n);
		geekhash(ostate, data);
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
