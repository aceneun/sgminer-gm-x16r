/*-
 * Copyright 2015 djm34 
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

#include "sph/sph_sha2.h"

static const uint32_t diff1targ = 0x0000ffff;



static inline void credits_hash(void *state, const void *input)
{
	sph_sha256_context sha1, sha2;
	uint32_t hash[8], hash2[8];

	sph_sha256_init(&sha1);
	sph_sha256(&sha1, input, 168);
	sph_sha256_close(&sha1, hash);


	sph_sha256_init(&sha2);
	sph_sha256(&sha2, hash, 32);
	sph_sha256_close(&sha2, hash2);

	memcpy(state, hash2, 32);

}

/* Used externally as confirmation of correct OCL code */
int credits_test(unsigned char *pdata, const unsigned char *ptarget, uint32_t nonce)
{
	uint32_t tmp_hash7, Htarg = le32toh(((const uint32_t *)ptarget)[7]);
	uint32_t data[42], ohash[8];
	printf("coming here credits test\n");

	be32enc_vect(data, (const uint32_t *)pdata, 42);
	data[35] = htobe32(nonce);
	credits_hash((unsigned char*)data,(unsigned char*)ohash);

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

void credits_regenhash(struct work *work)
{
        uint32_t data[42];
        uint32_t *nonce = (uint32_t *)(work->data + 140);
        uint32_t *ohash = (uint32_t *)(work->hash);
		
        be32enc_vect(data, (const uint32_t *)work->data, 42);
        data[35] = htobe32(*nonce);	

		credits_hash((unsigned char*)ohash, (unsigned char*)data);
        
}


bool scanhash_credits(struct thr_info *thr, const unsigned char __maybe_unused *pmidstate,
	unsigned char *pdata, unsigned char __maybe_unused *phash1,
	unsigned char __maybe_unused *phash, const unsigned char *ptarget,
	uint32_t max_nonce, uint32_t *last_nonce, uint32_t n)
{
	uint32_t *nonce = (uint32_t *)(pdata + 140);
	uint32_t data[42];
	uint32_t tmp_hash7;
	uint32_t Htarg = le32toh(((const uint32_t *)ptarget)[7]);
	bool ret = false;

	be32enc_vect(data, (const uint32_t *)pdata, 35);
	

	while (1)
	{
		uint32_t ostate[8];

		*nonce = ++n;
		data[35] = (n);
		credits_hash(ostate, data);
		tmp_hash7 = (ostate[7]);

		applog(LOG_INFO, "data7 %08lx", (long unsigned int)ostate[7]);

		if (unlikely(tmp_hash7 <= Htarg))
		{
			((uint32_t *)pdata)[35] = htobe32(n);
			*last_nonce = n;
			ret = true;
			break;
		}

		if (unlikely((n >= max_nonce) || thr->work_restart))
		{
			*last_nonce = n;
			break;
		}
	}

	return ret;
}