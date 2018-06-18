/*-
 * Copyright 2016 tpruvot
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

#include <sph/sph_skein.h>
#include "sph/sph_luffa.h"
#include "sph/sph_echo.h"
#include "sph/sph_shabal.h"
#include <sph/sph_fugue.h>
#include <sph/gost_streebog.h>

/* Move init out of loop, so init once externally, and then use one single memcpy with that bigger memory block */
typedef struct {
    sph_skein512_context    skein1;
    sph_shabal512_context   shabal1;
    sph_echo512_context     echo1;
    sph_luffa512_context    luffa1;
    sph_fugue512_context    fugue1;
    sph_gost512_context     gost1;
} Xhash_context_holder;

static Xhash_context_holder base_contexts;

static void init_Xhash_contexts()
{
    sph_skein512_init(&base_contexts.skein1);
    sph_shabal512_init(&base_contexts.shabal1);
    sph_echo512_init(&base_contexts.echo1);
    sph_luffa512_init(&base_contexts.luffa1);
    sph_fugue512_init(&base_contexts.fugue1);
    sph_gost512_init(&base_contexts.gost1);
}

static inline void xhash(void *state, const void *input)
{
    init_Xhash_contexts();

    Xhash_context_holder ctx;

    uint32_t hashA[16];

    memcpy(&ctx, &base_contexts, sizeof(base_contexts));

    sph_skein512(&ctx.skein1, input, 80);
    sph_skein512_close(&ctx.skein1, hashA);
    applog(LOG_WARNING, "SK: %s", bin2hex((const unsigned char*)hashA, 64));

    sph_shabal512(&ctx.shabal1, hashA, 64);
    sph_shabal512_close(&ctx.shabal1, hashA);
    applog(LOG_WARNING, "SH: %s", bin2hex((const unsigned char*)hashA, 64));

    sph_echo512(&ctx.echo1, hashA, 64);
    sph_echo512_close(&ctx.echo1, hashA);
    applog(LOG_WARNING, "EC: %s", bin2hex((const unsigned char*)hashA, 64));

    sph_luffa512(&ctx.luffa1, hashA, 64);
    sph_luffa512_close(&ctx.luffa1, hashA);
    applog(LOG_WARNING, "LU: %s", bin2hex((const unsigned char*)hashA, 64));

    sph_fugue512(&ctx.fugue1, hashA, 64);
    sph_fugue512_close(&ctx.fugue1, hashA);
    applog(LOG_WARNING, "FU: %s", bin2hex((const unsigned char*)hashA, 64));

    sph_gost512(&ctx.gost1, hashA, 64);
    sph_gost512_close(&ctx.gost1, hashA);
    applog(LOG_WARNING, "GO: %s", bin2hex((const unsigned char*)hashA, 64));

    memcpy(state, hashA, 32);
}

void polytimos_regenhash(struct work *work)
{
    uint32_t data[20];
    uint32_t *nonce = (uint32_t *)(work->data + 76);
    uint32_t *ohash = (uint32_t *)(work->hash);

    be32enc_vect(data, (const uint32_t *)work->data, 19);
    //data[19] = 0;
    data[19] = htobe32(*nonce);
    xhash(ohash, data);
}
