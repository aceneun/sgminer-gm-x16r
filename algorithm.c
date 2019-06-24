/*
 * Copyright 2014 sgminer developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.  See COPYING for more details.
 */

#include "algorithm.h"
#include "findnonce.h"
#include "sph/sph_sha2.h"
#include "ocl.h"
#include "ocl/build_kernel.h"

#include "algorithm/scrypt.h"
#include "algorithm/animecoin.h"
#include "algorithm/inkcoin.h"
#include "algorithm/quarkcoin.h"
#include "algorithm/qubitcoin.h"
#include "algorithm/sifcoin.h"
#include "algorithm/darkcoin.h"
#include "algorithm/myriadcoin-groestl.h"
#include "algorithm/fuguecoin.h"
#include "algorithm/groestlcoin.h"
#include "algorithm/twecoin.h"
#include "algorithm/marucoin.h"
#include "algorithm/maxcoin.h"
#include "algorithm/talkcoin.h"
#include "algorithm/bitblock.h"
#include "algorithm/x14.h"
#include "algorithm/x16r.h"
#include "algorithm/x16s.h"
#include "algorithm/fresh.h"
#include "algorithm/whirlcoin.h"
#include "algorithm/neoscrypt.h"
#include "algorithm/whirlpoolx.h"
#include "algorithm/lyra2re.h"
#include "algorithm/lyra2rev2.h"
#include "algorithm/pluck.h"
#include "algorithm/yescrypt.h"
#include "algorithm/credits.h"
#include "algorithm/blake256.h"
#include "algorithm/blakecoin.h"
#include "algorithm/ethash.h"
#include "algorithm/cryptonight.h"
#include "algorithm/equihash.h"
#include "algorithm/x17.h"
#include "algorithm/xevan.h"
#include "algorithm/phi.h"
#include "algorithm/tribus.h"
#include "algorithm/aergo.h"
#include "algorithm/c11.h"
#include "algorithm/lyra2Z.h"
#include "algorithm/polytimos.h"
#include "algorithm/geek.h"
#include "algorithm/skunk.h"

#include "compat.h"

#include <inttypes.h>
#include <string.h>

const char *algorithm_type_str[] = {
  "Unknown",
  "Credits",
  "Scrypt",
  "NScrypt",
  "X11",
  "X13",
  "X14",
  "X15",
  "X16R",
  "X16S",
  "X17",
  "Phi1612",
  "Xevan",
  "Lyra2Z",
  "Tribus",
  "Keccak",
  "Quarkcoin",
  "Twecoin",
  "Fugue256",
  "NIST",
  "Fresh",
  "Whirlcoin",
  "Neoscrypt",
  "WhirlpoolX",
  "Lyra2RE",
  "Lyra2REV2"
  "Pluck"
  "Yescrypt",
  "Yescrypt-multi",
  "Blakecoin",
  "Blake",
  "Vanilla",
  "Ethash",
  "Cryptonight",
  "Equihash",
  "Polytimos",
  "Geek",
  "Skunk"
};

void sha256(const unsigned char *message, unsigned int len, unsigned char *digest)
{
  sph_sha256_context ctx_sha2;

  sph_sha256_init(&ctx_sha2);
  sph_sha256(&ctx_sha2, message, len);
  sph_sha256_close(&ctx_sha2, (void*)digest);
}

void gen_hash(const unsigned char *data, unsigned int len, unsigned char *hash)
{
  unsigned char hash1[32];
  sph_sha256_context ctx_sha2;

  sph_sha256_init(&ctx_sha2);
  sph_sha256(&ctx_sha2, data, len);
  sph_sha256_close(&ctx_sha2, hash1);
  sph_sha256(&ctx_sha2, hash1, 32);
  sph_sha256_close(&ctx_sha2, hash);
}

void sha256d_midstate(struct work *work)
{
	unsigned char data[64];
	uint32_t *data32 = (uint32_t *)data;
	sph_sha256_context ctx;

	flip64(data32, work->data);
	sph_sha256_init(&ctx);
	sph_sha256(&ctx, data, 64);
	memcpy(work->midstate, ctx.val, 32);
	endian_flip32(work->midstate, work->midstate);
}

#define CL_SET_BLKARG(blkvar) status |= clSetKernelArg(*kernel, num++, sizeof(uint), (void *)&blk->blkvar)
#define CL_SET_VARG(args, var) status |= clSetKernelArg(*kernel, num++, args * sizeof(uint), (void *)var)
#define CL_SET_ARG_N(n, var) do { status |= clSetKernelArg(*kernel, n, sizeof(var), (void *)&var); } while (0)
#define CL_SET_ARG_0(var) CL_SET_ARG_N(0, var)
#define CL_SET_ARG(var) CL_SET_ARG_N(num++, var)
#define CL_NEXTKERNEL_SET_ARG_N(n, var) do { kernel++; CL_SET_ARG_N(n, var); } while (0)
#define CL_NEXTKERNEL_SET_ARG_0(var) CL_NEXTKERNEL_SET_ARG_N(0, var)
#define CL_NEXTKERNEL_SET_ARG(var) CL_NEXTKERNEL_SET_ARG_N(num++, var)

static void append_scrypt_compiler_options(struct _build_kernel_data *data, struct cgpu_info *cgpu, struct _algorithm_t *algorithm)
{
  char buf[255];
  sprintf(buf, " -D LOOKUP_GAP=%d -D CONCURRENT_THREADS=%u -D NFACTOR=%d",
    cgpu->lookup_gap, (unsigned int)cgpu->thread_concurrency, algorithm->nfactor);
  strcat(data->compiler_options, buf);

  sprintf(buf, "lg%utc%unf%u", cgpu->lookup_gap, (unsigned int)cgpu->thread_concurrency, algorithm->nfactor);
  strcat(data->binary_filename, buf);
}

static void append_ethash_compiler_options(struct _build_kernel_data *data, struct cgpu_info *cgpu, struct _algorithm_t *algorithm)
{
#ifdef WIN32
  strcat(data->compiler_options, "-DWINDOWS");
#endif
}

static void append_neoscrypt_compiler_options(struct _build_kernel_data *data, struct cgpu_info *cgpu, struct _algorithm_t *algorithm)
{
  char buf[255];
  sprintf(buf, " %s-D MAX_GLOBAL_THREADS=%lu ",
    ((cgpu->lookup_gap > 0) ? " -D LOOKUP_GAP=2 " : ""), (unsigned long)cgpu->thread_concurrency);
  strcat(data->compiler_options, buf);

  sprintf(buf, "%stc%lu", ((cgpu->lookup_gap > 0) ? "lg" : ""), (unsigned long)cgpu->thread_concurrency);
  strcat(data->binary_filename, buf);
}

static void append_x11_compiler_options(struct _build_kernel_data *data, struct cgpu_info *cgpu, struct _algorithm_t *algorithm)
{
  char buf[255];
  sprintf(buf, " -D SPH_COMPACT_BLAKE_64=%d -D SPH_LUFFA_PARALLEL=%d -D SPH_KECCAK_UNROLL=%u ",
    ((opt_blake_compact) ? 1 : 0), ((opt_luffa_parallel) ? 1 : 0), (unsigned int)opt_keccak_unroll);
  strcat(data->compiler_options, buf);

  sprintf(buf, "ku%u%s%s", (unsigned int)opt_keccak_unroll, ((opt_blake_compact) ? "bc" : ""), ((opt_luffa_parallel) ? "lp" : ""));
  strcat(data->binary_filename, buf);
}


static void append_x13_compiler_options(struct _build_kernel_data *data, struct cgpu_info *cgpu, struct _algorithm_t *algorithm)
{
  char buf[255];

  append_x11_compiler_options(data, cgpu, algorithm);

  sprintf(buf, " -D SPH_HAMSI_EXPAND_BIG=%d -D SPH_HAMSI_SHORT=%d ",
    (unsigned int)opt_hamsi_expand_big, ((opt_hamsi_short) ? 1 : 0));
  strcat(data->compiler_options, buf);

  sprintf(buf, "big%u%s", (unsigned int)opt_hamsi_expand_big, ((opt_hamsi_short) ? "hs" : ""));
  strcat(data->binary_filename, buf);
}

static cl_int queue_scrypt_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  unsigned char *midstate = blk->work->midstate;
  cl_kernel *kernel = &clState->kernel;
  unsigned int num = 0;
  cl_uint le_target;
  cl_int status = 0;

  le_target = *(cl_uint *)(blk->work->device_target + 28);
  memcpy(clState->cldata, blk->work->data, 80);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(clState->padbuffer8);
  CL_SET_VARG(4, &midstate[0]);
  CL_SET_VARG(4, &midstate[16]);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_neoscrypt_kernel(_clState *clState, dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel = &clState->kernel;
  unsigned int num = 0;
  cl_uint le_target;
  cl_int status = 0;

  /* This looks like a unnecessary double cast, but to make sure, that
   * the target's most significant entry is adressed as a 32-bit value
   * and not accidently by something else the double cast seems wise.
   * The compiler will get rid of it anyway. */
  le_target = (cl_uint)le32toh(((uint32_t *)blk->work->/*device_*/target)[7]);
  memcpy(clState->cldata, blk->work->data, 80);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_credits_kernel(_clState *clState, dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel = &clState->kernel;
  unsigned int num = 0;
  cl_ulong le_target;
  cl_int status = 0;


    // le_target = (*(cl_uint *)(blk->work->device_target + 24));
  le_target = (cl_ulong)le64toh(((uint64_t *)blk->work->/*device_*/target)[3]);
  //  le_target = (cl_uint)((uint32_t *)blk->work->target)[6];


  memcpy(clState->cldata, blk->work->data, 168);
//  flip168(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 168, clState->cldata, 0, NULL, NULL);

  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);
  CL_SET_ARG(blk->work->midstate);

  return status;
}

static cl_int queue_yescrypt_kernel(_clState *clState, dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel = &clState->kernel;
  unsigned int num = 0;
  cl_uint le_target;
  cl_int status = 0;


//  le_target = (*(cl_uint *)(blk->work->device_target + 28));
  le_target = (cl_uint)le32toh(((uint32_t *)blk->work->/*device_*/target)[7]);
//  le_target = (cl_uint)((uint32_t *)blk->work->target)[7];


//  memcpy(clState->cldata, blk->work->data, 80);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->buffer1);
  CL_SET_ARG(clState->buffer2);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_yescrypt_multikernel(_clState *clState, dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
//  cl_kernel *kernel = &clState->kernel;
  cl_kernel *kernel;
  unsigned int num = 0;
  cl_uint le_target;
  cl_int status = 0;


  //  le_target = (*(cl_uint *)(blk->work->device_target + 28));
  le_target = (cl_uint)le32toh(((uint32_t *)blk->work->/*device_*/target)[7]);
  memcpy(clState->cldata, blk->work->data, 80);
//  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);
//pbkdf and initial sha
  kernel = &clState->kernel;

  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->buffer1);
  CL_SET_ARG(clState->buffer2);
  CL_SET_ARG(clState->buffer3);
  CL_SET_ARG(le_target);

//inactive kernel
  num = 0;
  kernel = clState->extra_kernels;
  CL_SET_ARG_N(0,clState->buffer1);
  CL_SET_ARG_N(1,clState->buffer2);
//  CL_SET_ARG_N(3, clState->buffer3);

//mix2_2
  num = 0;
  CL_NEXTKERNEL_SET_ARG_N(0, clState->padbuffer8);
  CL_SET_ARG_N(1,clState->buffer1);
  CL_SET_ARG_N(2,clState->buffer2);
  //mix2_2
//inactive kernel
  num = 0;
  CL_NEXTKERNEL_SET_ARG_N(0, clState->buffer1);
  CL_SET_ARG_N(1, clState->buffer2);
  //mix2_2

  num = 0;
  CL_NEXTKERNEL_SET_ARG_N(0, clState->padbuffer8);
  CL_SET_ARG_N(1, clState->buffer1);
  CL_SET_ARG_N(2, clState->buffer2);

  //inactive kernel
  num = 0;
  CL_NEXTKERNEL_SET_ARG_N(0, clState->buffer1);
  CL_SET_ARG_N(1, clState->buffer2);
  //mix2_2


//pbkdf and finalization
    num=0;
  CL_NEXTKERNEL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(clState->buffer2);
  CL_SET_ARG(clState->buffer3);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_maxcoin_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel = &clState->kernel;
  unsigned int num = 0;
  cl_int status = 0;

  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->outputBuffer);

  return status;
}

static cl_int queue_sph_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel = &clState->kernel;
  unsigned int num = 0;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_darkcoin_mod_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // bmw - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // groestl - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search4
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search5
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa - search6
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // cubehash - search7
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shavite - search8
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // simd - search9
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // echo - search10
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_bitblock_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // bmw - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // groestl - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search4
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search5
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa - search6
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // cubehash - search7
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shavite - search8
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // simd - search9
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // echo - search10
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // hamsi - search11
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // fugue - search12
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // hamsi - search11
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // fugue - search12
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_bitblockold_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // bmw - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // groestl - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search4
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search5
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa - search6
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // cubehash - search7
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shavite - search8
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // simd - search9
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // combined echo, hamsi, fugue - shabal - whirlpool - search10
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}


static cl_int queue_marucoin_mod_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // bmw - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // groestl - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search4
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search5
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa - search6
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // cubehash - search7
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shavite - search8
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // simd - search9
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // echo - search10
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // hamsi - search11
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // fugue - search12
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_marucoin_mod_old_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // bmw - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // groestl - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search4
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search5
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa - search6
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // cubehash - search7
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shavite - search8
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // simd - search9
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // combined echo, hamsi, fugue - search10
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_talkcoin_mod_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // groestl - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // jh - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search4
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_x14_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // bmw - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // groestl - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search4
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search5
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa - search6
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // cubehash - search7
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shavite - search8
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // simd - search9
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // echo - search10
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // hamsi - search11
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // fugue - search12
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shabal - search13
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_x14_old_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // bmw - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // groestl - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search4
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search5
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa - search6
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // cubehash - search7
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shavite - search8
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // simd - search9
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // combined echo, hamsi, fugue - shabal - search10
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_xevan_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // bmw - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // groestl - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search4
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search5
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa - search6
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // cubehash - search7
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shavite - search8
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // simd - search9
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // echo - search10
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // hamsi - search11
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // fugue - search12
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shabal - search13
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // whirlpool - search14
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // sha - search15
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // haval - search16
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // blake - search17
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // bmw - search18
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // groestl - search19
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search20
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search21
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search22
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa - search23
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // cubehash - search24
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shavite - search25
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // simd - search26
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // echo - search27
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // hamsi - search28
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // fugue - search29
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shabal - search30
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // whirlpool - search31
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // sha - search32
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // haval - search33
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_aergo_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
	cl_kernel *kernel;
	unsigned int num;
	cl_ulong le_target;
	cl_int status = 0;

	le_target = *(cl_ulong *)(blk->work->device_target + 24);
	flip80(clState->cldata, blk->work->data);
	status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

	// echo - search
	kernel = &clState->kernel;
	num = 0;
	CL_SET_ARG(clState->CLbuffer0);
	CL_SET_ARG(clState->padbuffer8);
	// simd  - search1
	kernel = clState->extra_kernels;
	CL_SET_ARG_0(clState->padbuffer8);
	// blake - search2
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// bmw - search3
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// whirlpool - search4
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// groestl - search5
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// gost - search6
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// skein - search7
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// bmw - search8
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// jh - search9
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// luffa - search10
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// keccak - search11
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// gost - search12
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// cubehash - search13
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// echo - search14
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// simd - search15
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// hamsi - search16
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// fugue - search17
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// shavite - search18
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// shabal - search19
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// haval - search20
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// shavite - search21
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// gost - search22
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// echo - search23
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// blake - search24
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// jh - search25
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// cubehash - search26
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// simd - search27
	num = 0;
	CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
	CL_SET_ARG(clState->outputBuffer);
	CL_SET_ARG(le_target);

	return status;
}

static cl_int queue_phi_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
	cl_kernel *kernel;
	unsigned int num;
	cl_ulong le_target;
	cl_int status = 0;

	le_target = *(cl_ulong *)(blk->work->device_target + 24);
	flip80(clState->cldata, blk->work->data);
	status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);
	status |= clEnqueueWriteBuffer(clState->commandQueue, clState->MidstateBuf, true, 0, sizeof(cl_ulong) * 8, blk->work->midstate, 0, NULL, NULL);

	// skein - search
	kernel = &clState->kernel;
	num = 0;
	CL_SET_ARG(clState->CLbuffer0);
	CL_SET_ARG(clState->MidstateBuf);
	CL_SET_ARG(clState->padbuffer8);
	// jh - search1
	kernel = clState->extra_kernels;
	CL_SET_ARG_0(clState->padbuffer8);
	// cubehash - search2
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// fugue - search3
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// gost - search4
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// echo - search5
	num = 0;
	CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
	CL_SET_ARG(clState->outputBuffer);
	CL_SET_ARG(le_target);

	return status;
}

static cl_int queue_lyra2z_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
	cl_kernel *kernel;
	unsigned int num;
	cl_int status = 0;
	cl_ulong le_target;

	//  le_target = *(cl_uint *)(blk->work->device_target + 28);
	le_target = *(cl_ulong *)(blk->work->device_target + 24);
	flip80(clState->cldata, blk->work->data);
	status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

	// blake - search
	kernel = &clState->kernel;
	num = 0;
	//  CL_SET_ARG(clState->CLbuffer0);
	CL_SET_ARG(clState->buffer1);
	CL_SET_ARG(blk->work->blk.ctx_a);
	CL_SET_ARG(blk->work->blk.ctx_b);
	CL_SET_ARG(blk->work->blk.ctx_c);
	CL_SET_ARG(blk->work->blk.ctx_d);
	CL_SET_ARG(blk->work->blk.ctx_e);
	CL_SET_ARG(blk->work->blk.ctx_f);
	CL_SET_ARG(blk->work->blk.ctx_g);
	CL_SET_ARG(blk->work->blk.ctx_h);
	CL_SET_ARG(blk->work->blk.cty_a);
	CL_SET_ARG(blk->work->blk.cty_b);
	CL_SET_ARG(blk->work->blk.cty_c);
	num = 0;
	// keccak - search1
	kernel = clState->extra_kernels;
	CL_SET_ARG(clState->buffer1);
	CL_SET_ARG(clState->Scratchpads);
	CL_SET_ARG(clState->outputBuffer);
	CL_SET_ARG(le_target);
	
	return status;
}

void initialize_lyra2Z_kernel(_clState *clState, size_t scan_size) {
	/*cl_int status;
	// cl_int blockbuff = already initialized in buff0
	const size_t buf1size = scan_size * sizeof(unsigned long long) * 4;
	const size_t big_size = scan_size * sizeof(unsigned long long) * 12 * 4 * 4; // a matrix 3 rows 4 columns, 4 of them per hyper, 4 hypers
	clState->buffer1 = clCreateBuffer(clState->context, CL_MEM_READ_WRITE, buf1size, NULL, &status);
	if (status != CL_SUCCESS) { // Failing to allocate is really a fatal. You cannot go anywhere with it and no point in even trying to go on.
		quit(1, "Error %d: clCreateBuffer (hash passing betwee stages); very unlikely to happen, system overloaded?", status);
	}
	clState->padbuffer8 = clCreateBuffer(clState->context, CL_MEM_READ_WRITE, big_size, NULL, &status);
	if (status != CL_SUCCESS) { // Failing to allocate is really a fatal. You cannot go anywhere with it and no point in even trying to go on.
		quit(1, "Error %d: clCreateBuffer (big pad); most likely scan size too big!", status);
	}*/
	cl_int status;
	/*const size_t buf1size = scan_size * sizeof(unsigned long long) * 16;
	const size_t big_size = scan_size * sizeof(unsigned long long) * LYRA2Z_SCRATCHBUF_SIZE;*/
	clState->buffer1 = clCreateBuffer(clState->context, CL_MEM_READ_WRITE, 16 * 8 * scan_size, NULL, &status);
	if (status != CL_SUCCESS) { // Failing to allocate is really a fatal. You cannot go anywhere with it and no point in even trying to go on.
		quit(1, "Error %d: clCreateBuffer (hash passing betwee stages); very unlikely to happen, system overloaded?", status);
	}


	clState->Scratchpads = clCreateBuffer(clState->context, CL_MEM_READ_WRITE, sizeof(cl_ulong)*LYRA2Z_SCRATCHBUF_SIZE * scan_size, NULL, &status);
	if (status != CL_SUCCESS) { // Failing to allocate is really a fatal. You cannot go anywhere with it and no point in even trying to go on.
		quit(1, "Error %d: clCreateBuffer (big pad); most likely scan size too big!", status);
	}
}


cl_int truly_enqueue_lyra2Z_kernel(struct __clState *clState, size_t start, size_t scan, size_t local_size) {
	cl_int status = 0;
	cl_command_queue que = clState->commandQueue;
	const size_t off2[] = { 0, start };
	const size_t gws[] = { 4, scan };
	const size_t local_work_size = 256;
	const size_t mangle[] = { 4, 8 };
	status |= clEnqueueNDRangeKernel(que, clState->kernel, 1, &start, &scan, &local_work_size, 0, NULL, NULL); // blake
	status |= clEnqueueNDRangeKernel(que, clState->extra_kernels[0], 1, &start, &scan, &local_size, 0, NULL, NULL); // lyra2
	return status;
}

static cl_int queue_geek_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
	cl_kernel *kernel;
	unsigned int num;
	cl_ulong le_target;
	cl_int status = 0;

	le_target = *(cl_ulong *)(blk->work->device_target + 24);
	flip80(clState->cldata, blk->work->data);
	status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

	// blake - search
	kernel = &clState->kernel;
	num = 0;
	CL_SET_ARG(clState->CLbuffer0);
	CL_SET_ARG(clState->padbuffer8);
	// bmw - search1
	kernel = clState->extra_kernels;
	CL_SET_ARG_0(clState->padbuffer8);
	// echo - search2
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// shaba - search3
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// groestl - search4
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// cubehash - search5
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// keccak - search6
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// hamsi - search7
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// simd - search8
	num = 0;
	CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
	CL_SET_ARG(clState->outputBuffer);
	CL_SET_ARG(le_target);

	return status;
}

extern char *bytearray2hex(const uint8_t *p, size_t len);
extern bool bytearray_eq(const uint8_t *x, const uint8_t *y, size_t len);

static cl_int queue_x16r_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;
  uint8_t hashOrder[X16R_HASH_FUNC_COUNT];

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  if (opt_benchmark) {
    for (size_t i = 0; i < X16R_HASH_FUNC_COUNT; i++)
      hashOrder[i] = opt_benchmark_seq[i];
  }
  else {
    x16r_getalgolist(&clState->cldata[4], (char*) hashOrder);
  }

  if (!bytearray_eq((uint8_t*) clState->hash_order, hashOrder, X16R_HASH_FUNC_COUNT)) {
    for (size_t i = 0; i < X16R_HASH_FUNC_COUNT; i++)
      clState->hash_order[i] = hashOrder[i];
    if (!blk->work->thr_id) {
      char *s = bytearray2hex(hashOrder, X16R_HASH_FUNC_COUNT);
      applog(LOG_NOTICE, "hash order %s", s);
      free(s);
    }
  }

  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);
  if (status != CL_SUCCESS)
    return -1;

  for (int i = 0; i < X16R_HASH_FUNC_COUNT; i++) {
    kernel = &clState->extra_kernels[2*i];
    CL_SET_ARG_0(clState->padbuffer8);
  }

  kernel = &clState->extra_kernels[2*hashOrder[0]+1];
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);

  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_x16s_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;
  uint8_t hashOrder[X16S_HASH_FUNC_COUNT];

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  if (opt_benchmark) {
    for (size_t i = 0; i < X16S_HASH_FUNC_COUNT; i++)
      hashOrder[i] = opt_benchmark_seq[i];
  }
  else {
    x16s_getalgolist(&clState->cldata[4], hashOrder);
  }

  if (!bytearray_eq((uint8_t*) clState->hash_order, hashOrder, X16S_HASH_FUNC_COUNT)) {
    for (size_t i = 0; i < X16S_HASH_FUNC_COUNT; i++)
      clState->hash_order[i] = hashOrder[i];
    if (!blk->work->thr_id) {
      char *s = bytearray2hex(hashOrder, X16S_HASH_FUNC_COUNT);
      applog(LOG_NOTICE, "hash order %s", s);
      free(s);
    }
  }

  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);
  if (status != CL_SUCCESS)
    return -1;

  for (int i = 0; i < X16S_HASH_FUNC_COUNT; i++) {
    kernel = &clState->extra_kernels[2*i];
    CL_SET_ARG_0(clState->padbuffer8);
  }

  kernel = &clState->extra_kernels[2*hashOrder[0]+1];
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);

  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_x17_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL,NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // bmw - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // groestl - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search4
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search5
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa - search6
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // cubehash - search7
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shavite - search8
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // simd - search9
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // echo - search10
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // hamsi - search11
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // fugue - search12
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shabal - search13
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // whirlpool - search14
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // sha - search15
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // haval - search16
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_tribus_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
	uint32_t *midstate = (uint32_t*)(&blk->work->midstate); // 128 bytes
	uint32_t * data_end = (uint32_t*)(&blk->work->data[64]); //  16 bytes (end of data)
	cl_kernel *kernel;
	unsigned int num;
	cl_ulong le_target;
	cl_int status = 0;

	le_target = *(cl_ulong *)(blk->work->device_target + 24);
	memcpy(clState->cldata, blk->work->midstate, 128);
	for (int i = 0; i < 3; i++)
	((uint32_t*)clState->cldata)[32 + i] = swab32(data_end[i]);
	status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 128 + 16, clState->cldata, 0, NULL, NULL);

	// jh - search
	kernel = &clState->kernel;
	num = 0;
	CL_SET_ARG(clState->CLbuffer0);
	CL_SET_ARG(clState->padbuffer8);
	// keccak - search1
	kernel = clState->extra_kernels;
	CL_SET_ARG_0(clState->padbuffer8);
	// echo - search2
	num = 0;
	CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
	CL_SET_ARG(clState->outputBuffer);
	CL_SET_ARG(le_target);

	return status;
}

static cl_int queue_c11_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL,NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // bmw - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // groestl - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // keccak - search4
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search5
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa - search6
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // cubehash - search7
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // shavite - search8
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // simd - search9
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // echo - search10
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_polytimos_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);
  // skein search()
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // shabal search1()
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // echo search2()
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // luffa search3()
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // fugue search4()
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);

  // gost/streebog search5()
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_skunk_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
	cl_kernel *kernel;
	unsigned int num;
	cl_ulong le_target;
	cl_int status = 0;

	le_target = *(cl_ulong *)(blk->work->device_target + 24);
	flip80(clState->cldata, blk->work->data);
	status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);
	status |= clEnqueueWriteBuffer(clState->commandQueue, clState->MidstateBuf, true, 0, sizeof(cl_ulong) * 8, blk->work->midstate, 0, NULL, NULL);
	// skein search()
	kernel = &clState->kernel;
	num = 0;
	CL_SET_ARG(clState->CLbuffer0);
	CL_SET_ARG(clState->MidstateBuf);
	CL_SET_ARG(clState->padbuffer8);
	// cubehash search1()
	kernel = clState->extra_kernels;
	CL_SET_ARG_0(clState->padbuffer8);
	// fugue search2()
	CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
	// gost/streebog search3()
	num = 0;
	CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
	CL_SET_ARG(clState->outputBuffer);
	CL_SET_ARG(le_target);

	return status;
}

static cl_int enqueue_x16r_kernels(struct __clState *clState,
                                   size_t *p_global_work_offset, size_t *globalThreads, size_t *localThreads)
{
  cl_int status;

  status = clEnqueueNDRangeKernel(clState->commandQueue,
      clState->extra_kernels[2*clState->hash_order[0]+1],
      1, p_global_work_offset,
      globalThreads, localThreads, 0, NULL, NULL);
  if (unlikely(status != CL_SUCCESS)) {
    applog(LOG_ERR, "Error %d: Enqueueing kernel onto command queue. (clEnqueueNDRangeKernel)", status);
    return status;
  }

  for (int i = 1; i < X16R_HASH_FUNC_COUNT; i++) {
    status = clEnqueueNDRangeKernel(clState->commandQueue,
        clState->extra_kernels[2*clState->hash_order[i]],
        1, p_global_work_offset,
        globalThreads, localThreads, 0, NULL, NULL);
    if (unlikely(status != CL_SUCCESS)) {
      applog(LOG_ERR, "Error %d: Enqueueing kernel onto command queue. (clEnqueueNDRangeKernel)", status);
      return status;
    }
  }

  status = clEnqueueNDRangeKernel(clState->commandQueue,
      clState->kernel,
      1, p_global_work_offset,
      globalThreads, localThreads, 0, NULL, NULL);
  if (unlikely(status != CL_SUCCESS)) {
    applog(LOG_ERR, "Error %d: Enqueueing kernel onto command queue. (clEnqueueNDRangeKernel)", status);
    return status;
  }

  return 0;
}

static cl_int enqueue_x16s_kernels(struct __clState *clState,
                                   size_t *p_global_work_offset, size_t *globalThreads, size_t *localThreads)
{
  cl_int status;

  status = clEnqueueNDRangeKernel(clState->commandQueue,
      clState->extra_kernels[2*clState->hash_order[0]+1],
      1, p_global_work_offset,
      globalThreads, localThreads, 0, NULL, NULL);
  if (unlikely(status != CL_SUCCESS)) {
    applog(LOG_ERR, "Error %d: Enqueueing kernel onto command queue. (clEnqueueNDRangeKernel)", status);
    return status;
  }

  for (int i = 1; i < X16S_HASH_FUNC_COUNT; i++) {
    status = clEnqueueNDRangeKernel(clState->commandQueue,
        clState->extra_kernels[2*clState->hash_order[i]],
        1, p_global_work_offset,
        globalThreads, localThreads, 0, NULL, NULL);
    if (unlikely(status != CL_SUCCESS)) {
      applog(LOG_ERR, "Error %d: Enqueueing kernel onto command queue. (clEnqueueNDRangeKernel)", status);
      return status;
    }
  }

  status = clEnqueueNDRangeKernel(clState->commandQueue,
      clState->kernel,
      1, p_global_work_offset,
      globalThreads, localThreads, 0, NULL, NULL);
  if (unlikely(status != CL_SUCCESS)) {
    applog(LOG_ERR, "Error %d: Enqueueing kernel onto command queue. (clEnqueueNDRangeKernel)", status);
    return status;
  }

  return 0;
}


static cl_int queue_fresh_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // shavite 1 - search
  kernel = &clState->kernel;
  num = 0;
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->padbuffer8);
  // smid 1 - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // shavite 2 - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // smid 2 - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // echo - search4
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_whirlcoin_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  cl_ulong le_target;
  cl_int status = 0;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  //clbuffer, hashes
  kernel = &clState->kernel;
  CL_SET_ARG_N(0, clState->CLbuffer0);
  CL_SET_ARG_N(1, clState->padbuffer8);

  kernel = clState->extra_kernels;
  CL_SET_ARG_N(0, clState->padbuffer8);

  CL_NEXTKERNEL_SET_ARG_N(0, clState->padbuffer8);

  //hashes, output, target
  CL_NEXTKERNEL_SET_ARG_N(0, clState->padbuffer8);
  CL_SET_ARG_N(1, clState->outputBuffer);
  CL_SET_ARG_N(2, le_target);

  return status;
}

static cl_int queue_whirlpoolx_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  uint64_t midblock[8], key[8] = { 0 }, tmp[8] = { 0 };
  cl_ulong le_target;
  cl_int status;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);

  memcpy(midblock, clState->cldata, 64);

  // midblock = n, key = h
  for (int i = 0; i < 10; ++i) {
    tmp[0] = WHIRLPOOL_ROUND_CONSTANTS[i];
    whirlpool_round(key, tmp);
    tmp[0] = 0;
    whirlpool_round(midblock, tmp);

    for (int x = 0; x < 8; ++x) {
      midblock[x] ^= key[x];
    }
  }

  for (int i = 0; i < 8; ++i) {
    midblock[i] ^= ((uint64_t *)(clState->cldata))[i];
  }

  status = clSetKernelArg(clState->kernel, 0, sizeof(cl_ulong8), (cl_ulong8 *)&midblock);
  status |= clSetKernelArg(clState->kernel, 1, sizeof(cl_ulong), (void *)(((uint64_t *)clState->cldata) + 8));
  status |= clSetKernelArg(clState->kernel, 2, sizeof(cl_ulong), (void *)(((uint64_t *)clState->cldata) + 9));
  status |= clSetKernelArg(clState->kernel, 3, sizeof(cl_mem), (void *)&clState->outputBuffer);
  status |= clSetKernelArg(clState->kernel, 4, sizeof(cl_ulong), (void *)&le_target);

  return status;
}

static cl_int queue_lyra2re_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_int status = 0;
  cl_ulong le_target;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;

  CL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(blk->work->blk.ctx_a);
  CL_SET_ARG(blk->work->blk.ctx_b);
  CL_SET_ARG(blk->work->blk.ctx_c);
  CL_SET_ARG(blk->work->blk.ctx_d);
  CL_SET_ARG(blk->work->blk.ctx_e);
  CL_SET_ARG(blk->work->blk.ctx_f);
  CL_SET_ARG(blk->work->blk.ctx_g);
  CL_SET_ARG(blk->work->blk.ctx_h);
  CL_SET_ARG(blk->work->blk.cty_a);
  CL_SET_ARG(blk->work->blk.cty_b);
  CL_SET_ARG(blk->work->blk.cty_c);

  // bmw - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->padbuffer8);
  // groestl - search2
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // skein - search3
  CL_NEXTKERNEL_SET_ARG_0(clState->padbuffer8);
  // jh - search4
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_lyra2rev2_kernel(struct __clState *clState, struct _dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel;
  unsigned int num;
  cl_int status = 0;
  cl_ulong le_target;

  //  le_target = *(cl_uint *)(blk->work->device_target + 28);
  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  // blake - search
  kernel = &clState->kernel;
  num = 0;
  //  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->buffer1);
  CL_SET_ARG(blk->work->blk.ctx_a);
  CL_SET_ARG(blk->work->blk.ctx_b);
  CL_SET_ARG(blk->work->blk.ctx_c);
  CL_SET_ARG(blk->work->blk.ctx_d);
  CL_SET_ARG(blk->work->blk.ctx_e);
  CL_SET_ARG(blk->work->blk.ctx_f);
  CL_SET_ARG(blk->work->blk.ctx_g);
  CL_SET_ARG(blk->work->blk.ctx_h);
  CL_SET_ARG(blk->work->blk.cty_a);
  CL_SET_ARG(blk->work->blk.cty_b);
  CL_SET_ARG(blk->work->blk.cty_c);

  // keccak - search1
  kernel = clState->extra_kernels;
  CL_SET_ARG_0(clState->buffer1);
  // cubehash - search2
  num = 0;
  CL_NEXTKERNEL_SET_ARG_0(clState->buffer1);
  // lyra - search3
  num = 0;
  CL_NEXTKERNEL_SET_ARG_N(0, clState->buffer1);
  CL_SET_ARG_N(1, clState->padbuffer8);
  // skein -search4
  num = 0;
  CL_NEXTKERNEL_SET_ARG_0(clState->buffer1);
  // cubehash - search5
  num = 0;
  CL_NEXTKERNEL_SET_ARG_0(clState->buffer1);
  // bmw - search6
  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->buffer1);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(le_target);

  return status;
}

void initialize_lyra2rev2_mdz(_clState *clState, size_t scan_size) {
	cl_int status;
	// cl_int blockbuff = already initialized in buff0
	const size_t buf1size = scan_size * sizeof(unsigned long long) * 4;
	const size_t big_size = scan_size * sizeof(unsigned long long) * 12 * 4 * 4; // a matrix 3 rows 4 columns, 4 of them per hyper, 4 hypers
	clState->buffer1 = clCreateBuffer(clState->context, CL_MEM_READ_WRITE, buf1size, NULL, &status);
	if (status != CL_SUCCESS) { // Failing to allocate is really a fatal. You cannot go anywhere with it and no point in even trying to go on.
		quit(1, "Error %d: clCreateBuffer (hash passing betwee stages); very unlikely to happen, system overloaded?", status);
	}
	clState->padbuffer8 = clCreateBuffer(clState->context, CL_MEM_READ_WRITE, big_size, NULL, &status);
	if (status != CL_SUCCESS) { // Failing to allocate is really a fatal. You cannot go anywhere with it and no point in even trying to go on.
		quit(1, "Error %d: clCreateBuffer (big pad); most likely scan size too big!", status);
	}
}


cl_int truly_enqueue_lyra2rev2_mdz_kernel(struct __clState *clState, size_t start, size_t scan, size_t local_size) {
	cl_int status = 0;
	cl_command_queue que = clState->commandQueue;
	const size_t off2[] = { 0, start };
	const size_t gws[] = { 4, scan };
	const size_t expand[] = { 4, 16 };
	const size_t mangle[] = { 4, 8 };
	status |= clEnqueueNDRangeKernel(que, clState->kernel, 1, &start, &scan, &local_size, 0, NULL, NULL); // blake
	status |= clEnqueueNDRangeKernel(que, clState->extra_kernels[0], 1, &start, &scan, &local_size, 0, NULL, NULL); // keccak
	status |= clEnqueueNDRangeKernel(que, clState->extra_kernels[1], 1, &start, &scan, &local_size, 0, NULL, NULL); // cubehash
	status |= clEnqueueNDRangeKernel(que, clState->extra_kernels[2], 2, off2, gws, expand, 0, NULL, NULL); // lyra 4w monolithic
	status |= clEnqueueNDRangeKernel(que, clState->extra_kernels[3], 1, &start, &scan, &local_size, 0, NULL, NULL); // skein
	status |= clEnqueueNDRangeKernel(que, clState->extra_kernels[4], 1, &start, &scan, &local_size, 0, NULL, NULL); // cubehash again
	status |= clEnqueueNDRangeKernel(que, clState->extra_kernels[5], 1, &start, &scan, &local_size, 0, NULL, NULL); // blue midnight wish
	return status;
}

static cl_int queue_pluck_kernel(_clState *clState, dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel = &clState->kernel;
  unsigned int num = 0;
  cl_uint le_target;
  cl_int status = 0;

  le_target = (cl_uint)le32toh(((uint32_t *)blk->work->/*device_*/target)[7]);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(clState->padbuffer8);
  CL_SET_ARG(le_target);

  return status;
}

static cl_int queue_blake_kernel(_clState *clState, dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel = &clState->kernel;
  unsigned int num = 0;
  cl_int status = 0;
  cl_ulong le_target;

  le_target = *(cl_ulong *)(blk->work->device_target + 24);
  flip80(clState->cldata, blk->work->data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, true, 0, 80, clState->cldata, 0, NULL, NULL);

  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(blk->work->blk.ctx_a);
  CL_SET_ARG(blk->work->blk.ctx_b);
  CL_SET_ARG(blk->work->blk.ctx_c);
  CL_SET_ARG(blk->work->blk.ctx_d);
  CL_SET_ARG(blk->work->blk.ctx_e);
  CL_SET_ARG(blk->work->blk.ctx_f);
  CL_SET_ARG(blk->work->blk.ctx_g);
  CL_SET_ARG(blk->work->blk.ctx_h);

  CL_SET_ARG(blk->work->blk.cty_a);
  CL_SET_ARG(blk->work->blk.cty_b);
  CL_SET_ARG(blk->work->blk.cty_c);

  return status;
}

extern pthread_mutex_t eth_nonce_lock;
extern uint32_t eth_nonce;
static const int eth_future_epochs = 6;
static cl_int queue_ethash_kernel(_clState *clState, dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  struct pool *pool = blk->work->pool;
  eth_dag_t *dag;
  cl_kernel *kernel;
  unsigned int num = 0;
  cl_int status = 0;
  cl_ulong le_target;
  cl_uint Isolate = UINT32_MAX;

  dag = &blk->work->thr->cgpu->eth_dag;
  cg_ilock(&dag->lock);
  cg_ilock(&pool->data_lock);
  if (pool->eth_cache.disabled || pool->eth_cache.dag_cache == NULL) {
    cg_iunlock(&pool->data_lock);
    cg_iunlock(&dag->lock);
    cgsleep_ms(200);
    applog(LOG_DEBUG, "THR[%d]: stop ETHASH mining (%d, %p)", blk->work->thr_id, pool->eth_cache.disabled, pool->eth_cache.dag_cache);
    return 1;
  }
  if (dag->current_epoch != blk->work->eth_epoch) {
    applog(LOG_NOTICE, "GPU%d: begin DAG creation...", blk->work->thr->cgpu->device_id);
    cl_ulong CacheSize = EthGetCacheSize(blk->work->eth_epoch);
    cg_ulock(&dag->lock);
    if (dag->dag_buffer == NULL || blk->work->eth_epoch >= dag->max_epoch + 1U) {
      if (dag->dag_buffer != NULL) {
        cg_dlock(&pool->data_lock);
        clReleaseMemObject(dag->dag_buffer);
      }
      else {
        cg_ulock(&pool->data_lock);
        int size = ++pool->eth_cache.nDevs;
        pool->eth_cache.dags = (eth_dag_t **) realloc(pool->eth_cache.dags, sizeof(void*) * size);
        pool->eth_cache.dags[size-1] = dag;
        dag->pool = pool;
        cg_dwlock(&pool->data_lock);
      }
      dag->max_epoch = blk->work->eth_epoch + eth_future_epochs;
      dag->dag_buffer = clCreateBuffer(clState->context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, EthGetDAGSize(dag->max_epoch), NULL, &status);
      if (status != CL_SUCCESS) {
        cg_runlock(&pool->data_lock);
        dag->max_epoch = 0;
        dag->dag_buffer = NULL;
        cg_wunlock(&dag->lock);
        applog(LOG_ERR, "Error %d: Creating the DAG buffer failed.", status);
        return status;
      }
    }
    else
      cg_dlock(&pool->data_lock);

    applog(LOG_DEBUG, "DAG being regenerated.");
    cl_mem eth_cache = clCreateBuffer(clState->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_WRITE_ONLY, CacheSize, pool->eth_cache.dag_cache, &status);
    cg_runlock(&pool->data_lock);
    if (status != CL_SUCCESS) {
      clReleaseMemObject(eth_cache);
      cg_wunlock(&dag->lock);
      applog(LOG_ERR, "Error %d: Creating the ethash cache buffer failed.", status);
      return status;
    }

    // enqueue DAG gen kernel
    kernel = &clState->GenerateDAG;

    cl_uint zero = 0;
    cl_uint CacheSize64 = CacheSize / 64;

    CL_SET_ARG(zero);
    CL_SET_ARG(eth_cache);
    CL_SET_ARG(dag->dag_buffer);
    CL_SET_ARG(CacheSize64);
    CL_SET_ARG(Isolate);

    cl_ulong DAGSize = EthGetDAGSize(blk->work->eth_epoch);
    size_t DAGItems = (size_t) (DAGSize / 64);
    cgsleep_ms(128 * blk->work->thr->cgpu->device_id);
    status |= clEnqueueNDRangeKernel(clState->commandQueue, clState->GenerateDAG, 1, NULL, &DAGItems, NULL, 0, NULL, NULL);
    clFinish(clState->commandQueue);

    clReleaseMemObject(eth_cache);
    if (status != CL_SUCCESS) {
      cg_wunlock(&dag->lock);
      applog(LOG_ERR, "Error %d: Setting args for the DAG kernel and/or executing it.", status);
      return status;
    }
    dag->current_epoch = blk->work->eth_epoch;
    cg_dwlock(&dag->lock);
    applog(LOG_NOTICE, "GPU%d: new DAG created", blk->work->thr->cgpu->device_id);
  }
  else {
    cg_dlock(&dag->lock);
    cg_iunlock(&pool->data_lock);
  }

  memcpy(&le_target, blk->work->device_target + 24, 8);
  blk->work->Nonce = (blk->work->Nonce >> 32 << 32) | blk->work->blk.nonce;

  num = 0;
  kernel = &clState->kernel;

  // Not nodes now (64 bytes), but DAG entries (128 bytes)
  cl_ulong DAGSize = EthGetDAGSize(blk->work->eth_epoch);
  cl_uint ItemsArg = DAGSize / 128;

  // DO NOT flip80.
  status |= clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, CL_FALSE, 0, 32, blk->work->data, 0, NULL, NULL);

  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(dag->dag_buffer);
  CL_SET_ARG(ItemsArg);
  CL_SET_ARG(blk->work->Nonce);
  CL_SET_ARG(le_target);
  CL_SET_ARG(Isolate);

  if (status != CL_SUCCESS)
    cg_runlock(&dag->lock);
  return status;
}

static void append_equihash_compiler_options(struct _build_kernel_data *data, struct cgpu_info *cgpu, struct _algorithm_t *algorithm)
{
  strcat(data->compiler_options, "");
}

static cl_int queue_cryptonight_kernel(_clState *clState, dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_kernel *kernel = &clState->kernel;
  unsigned int num = 0;
  cl_int status = 0, tgt32 = *(uint32_t*)(blk->work->target + 28);

  int variant = MIN(1, monero_variant(blk->work)); // limit variant to prevent DOS
  if (variant != clState->monero_variant) {
    applog(LOG_NOTICE, "switch to monero variant %d", variant);
    char kernel_name[20] = "search1";
    if (variant > 0)
      snprintf(kernel_name + 7, sizeof(kernel_name) - 7, "_var%d", variant);

	clReleaseKernel(clState->extra_kernels[0]);
    clState->extra_kernels[0] = clCreateKernel(clState->program, kernel_name, &status);
    if (status != CL_SUCCESS) {
      applog(LOG_ERR, "Error %d: Creating Kernel \"%s\" from program. (clCreateKernel)", kernel_name, status);
      return status;
    }
    clState->monero_variant = variant;
  }

  memcpy(clState->cldata, blk->work->data, blk->work->XMRBlobLen);

  status = clEnqueueWriteBuffer(clState->commandQueue, clState->CLbuffer0, CL_FALSE, 0, blk->work->XMRBlobLen, clState->cldata , 0, NULL, NULL);

  CL_SET_ARG(clState->CLbuffer0);
  CL_SET_ARG(blk->work->XMRBlobLen);
  CL_SET_ARG(clState->Scratchpads);
  CL_SET_ARG(clState->States);

  num = 0;
  kernel = clState->extra_kernels;
  CL_SET_ARG(clState->Scratchpads);
  CL_SET_ARG(clState->States);
  if (variant > 0)
	  CL_SET_ARG(*(cl_uint*)(clState->cldata + 35));

  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->Scratchpads);
  CL_SET_ARG(clState->States);
  CL_SET_ARG(clState->BranchBuffer[0]);
  CL_SET_ARG(clState->BranchBuffer[1]);
  CL_SET_ARG(clState->BranchBuffer[2]);
  CL_SET_ARG(clState->BranchBuffer[3]);

  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->States);
  CL_SET_ARG(clState->BranchBuffer[0]);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(tgt32);

  // last to be set in driver-opencl.c

  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->States);
  CL_SET_ARG(clState->BranchBuffer[1]);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(tgt32);


  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->States);
  CL_SET_ARG(clState->BranchBuffer[2]);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(tgt32);


  num = 0;
  CL_NEXTKERNEL_SET_ARG(clState->States);
  CL_SET_ARG(clState->BranchBuffer[3]);
  CL_SET_ARG(clState->outputBuffer);
  CL_SET_ARG(tgt32);

  return(status);
}


#define WORKSIZE clState->wsize

static cl_int queue_equihash_kernel(_clState *clState, dev_blk_ctx *blk, __maybe_unused cl_uint threads)
{
  cl_int status = 0;
  size_t work_items = threads;
  size_t worksize = clState->wsize;

  uint64_t mid_hash[8];
  equihash_calc_mid_hash(mid_hash, blk->work->equihash_data);
  status = clEnqueueWriteBuffer(clState->commandQueue, clState->MidstateBuf, CL_TRUE, 0, sizeof(mid_hash), mid_hash, 0, NULL, NULL);
  uint32_t dbg[2] = {0};
  status |= clEnqueueWriteBuffer(clState->commandQueue, clState->padbuffer8, CL_TRUE, 0, sizeof(dbg), &dbg, 0, NULL, NULL);

  cl_mem rowCounters[2] = {clState->buffer2, clState->buffer3};
  for (int round = 0; round < PARAM_K; round++) {
    size_t global_ws = RC_SIZE;
    size_t local_ws = 256;
    unsigned int num = 0;
    cl_kernel *kernel = &clState->extra_kernels[0];
    // Now on every round!!!!
    CL_SET_ARG(clState->index_buf[round]);
    CL_SET_ARG(rowCounters[round % 2]);
    CL_SET_ARG(clState->outputBuffer);
    CL_SET_ARG(clState->CLbuffer0);
    status |= clEnqueueNDRangeKernel(clState->commandQueue, *kernel, 1, NULL, &global_ws, &local_ws, 0, NULL, NULL);

    kernel = &clState->extra_kernels[1 + round];
    if (!round) {
      worksize = LOCAL_WORK_SIZE_ROUND0;
      work_items = NR_INPUTS / ROUND0_INPUTS_PER_WORK_ITEM;
    }
    else {
      worksize = LOCAL_WORK_SIZE;
      work_items = NR_ROWS * worksize;
    }
    status |= clEnqueueNDRangeKernel(clState->commandQueue, clState->extra_kernels[1 + round], 1, NULL, &work_items, &worksize, 0, NULL, NULL);
  }

  worksize = LOCAL_WORK_SIZE_POTENTIAL_SOLS;
  work_items = NR_ROWS * worksize;
  status |= clEnqueueNDRangeKernel(clState->commandQueue, clState->extra_kernels[1 + 9], 1, NULL, &work_items, &worksize, 0, NULL, NULL);

  worksize = LOCAL_WORK_SIZE_SOLS;
  work_items = MAX_POTENTIAL_SOLS * worksize;
  status |= clEnqueueNDRangeKernel(clState->commandQueue, clState->kernel, 1, NULL, &work_items, &worksize, 0, NULL, NULL);

  return status;
}
#undef WORKSIZE


static algorithm_settings_t algos[] = {
  // kernels starting from this will have difficulty calculated by using litecoin algorithm
#define A_SCRYPT(a) \
  { a, ALGO_SCRYPT, "", 1, 65536, 65536, 0, 0, 0xFF, 0xFFFFFFFFULL, 0x0000ffffUL, 0, -1, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, scrypt_regenhash, NULL, NULL, queue_scrypt_kernel, gen_hash, append_scrypt_compiler_options }
  A_SCRYPT("ckolivas"),
  A_SCRYPT("alexkarnew"),
  A_SCRYPT("alexkarnold"),
  A_SCRYPT("bufius"),
  A_SCRYPT("psw"),
  A_SCRYPT("zuikkis"),
  A_SCRYPT("arebyp"),
#undef A_SCRYPT

#define A_NEOSCRYPT(a) \
  { a, ALGO_NEOSCRYPT, "", 1, 65536, 65536, 0, 0, 0xFF, 0xFFFF000000000000ULL, 0x0000ffffUL, 0, -1, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, neoscrypt_regenhash, NULL, NULL, queue_neoscrypt_kernel, gen_hash, append_neoscrypt_compiler_options }
  A_NEOSCRYPT("neoscrypt"),
#undef A_NEOSCRYPT

#define A_PLUCK(a) \
  { a, ALGO_PLUCK, "", 1, 65536, 65536, 0, 0, 0xFF, 0xFFFF000000000000ULL, 0x0000ffffUL, 0, -1, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, pluck_regenhash, NULL, NULL, queue_pluck_kernel, gen_hash, append_neoscrypt_compiler_options }
  A_PLUCK("pluck"),
#undef A_PLUCK

#define A_CREDITS(a) \
  { a, ALGO_CRE, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFF000000000000ULL, 0x0000ffffUL, 0, -1, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, credits_regenhash, NULL, NULL, queue_credits_kernel, gen_hash, NULL}
  A_CREDITS("credits"),
#undef A_CREDITS

#define A_YESCRYPT(a) \
  { a, ALGO_YESCRYPT, "", 1, 65536, 65536, 0, 0, 0xFF, 0xFFFF000000000000ULL, 0x0000ffffUL, 0, -1, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, yescrypt_regenhash, NULL, NULL, queue_yescrypt_kernel, gen_hash, append_neoscrypt_compiler_options}
  A_YESCRYPT("yescrypt"),
#undef A_YESCRYPT

#define A_YESCRYPT_MULTI(a) \
  { a, ALGO_YESCRYPT_MULTI, "", 1, 65536, 65536, 0, 0, 0xFF, 0x00000000FFFFULL, 0x0000ffffUL, 6,-1,CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE , yescrypt_regenhash, NULL, NULL, queue_yescrypt_multikernel, gen_hash, append_neoscrypt_compiler_options}
  A_YESCRYPT_MULTI("yescrypt-multi"),
#undef A_YESCRYPT_MULTI

  // kernels starting from this will have difficulty calculated by using quarkcoin algorithm
#define A_QUARK(a, b) \
  { a, ALGO_QUARK, "", 256, 256, 256, 0, 0, 0xFF, 0xFFFFFFULL, 0x0000ffffUL, 0, 0, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, b, NULL, NULL, queue_sph_kernel, gen_hash, append_x11_compiler_options }
  A_QUARK("quarkcoin", quarkcoin_regenhash),
  A_QUARK("qubitcoin", qubitcoin_regenhash),
  A_QUARK("animecoin", animecoin_regenhash),
  A_QUARK("sifcoin", sifcoin_regenhash),
#undef A_QUARK

  // kernels starting from this will have difficulty calculated by using bitcoin algorithm
#define A_DARK(a, b) \
  { a, ALGO_X11, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 0, 0, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, b, NULL, NULL, queue_sph_kernel, gen_hash, append_x11_compiler_options }
  A_DARK("darkcoin", darkcoin_regenhash),
  A_DARK("inkcoin", inkcoin_regenhash),
  A_DARK("myriadcoin-groestl", myriadcoin_groestl_regenhash),
#undef A_DARK

  { "twecoin", ALGO_TWE, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 0, 0, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, twecoin_regenhash, NULL, NULL, queue_sph_kernel, sha256, NULL },
  { "maxcoin", ALGO_KECCAK, "", 1, 256, 1, 4, 15, 0x0F, 0xFFFFULL, 0x000000ffUL, 0, 0, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, maxcoin_regenhash, NULL, NULL, queue_maxcoin_kernel, sha256, NULL },

  { "darkcoin-mod", ALGO_X11, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 10, 8 * 16 * 4194304, 0, darkcoin_regenhash, NULL, NULL, queue_darkcoin_mod_kernel, gen_hash, append_x11_compiler_options },

  { "marucoin", ALGO_X13, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 0, 0, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, marucoin_regenhash, NULL, NULL, queue_sph_kernel, gen_hash, append_x13_compiler_options },
  { "marucoin-mod", ALGO_X13, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 12, 8 * 16 * 4194304, 0, marucoin_regenhash, NULL, NULL, queue_marucoin_mod_kernel, gen_hash, append_x13_compiler_options },
  { "marucoin-modold", ALGO_X13, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 10, 8 * 16 * 4194304, 0, marucoin_regenhash, NULL, NULL, queue_marucoin_mod_old_kernel, gen_hash, append_x13_compiler_options },

  { "x14", ALGO_X14, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 13, 8 * 16 * 4194304, 0, x14_regenhash, NULL, NULL, queue_x14_kernel, gen_hash, append_x13_compiler_options },
  { "x14old", ALGO_X14, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 10, 8 * 16 * 4194304, 0, x14_regenhash, NULL, NULL, queue_x14_old_kernel, gen_hash, append_x13_compiler_options },

  { "bitblock", ALGO_X15, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 14, 4 * 16 * 4194304, 0, bitblock_regenhash, NULL, NULL, queue_bitblock_kernel, gen_hash, append_x13_compiler_options },
  { "bitblockold", ALGO_X15, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 10, 4 * 16 * 4194304, 0, bitblock_regenhash, NULL, NULL, queue_bitblockold_kernel, gen_hash, append_x13_compiler_options },

  { "x16r", ALGO_X16R, "x16", 1, 256, 256, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 32, 8 * 16 * 4194304, 0, x16r_regenhash, NULL, NULL, queue_x16r_kernel, gen_hash, append_x13_compiler_options, enqueue_x16r_kernels },
  { "x16s", ALGO_X16S, "x16", 1, 256, 256, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 32, 8 * 16 * 4194304, 0, x16s_regenhash, NULL, NULL, queue_x16s_kernel, gen_hash, append_x13_compiler_options, enqueue_x16s_kernels },
  { "x17", ALGO_X17, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 16,  8 * 16 * 4194304, 0, x17_regenhash, NULL, NULL, queue_x17_kernel, gen_hash, append_x13_compiler_options},
  { "xevan", ALGO_XEVAN, "", 1, 256, 256, 0, 0, 0xFF, 0xFFFFULL, 0x00ffffffUL, 33, 8 * 16 * 4194304, 0, xevan_regenhash, NULL, NULL, queue_xevan_kernel, gen_hash, append_x13_compiler_options },
  { "phi", ALGO_PHI, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 5, 8 * 16 * 4194304, 0, phi_regenhash, phi_midstate, phi_prepare_work, queue_phi_kernel, gen_hash, append_x11_compiler_options },
  { "tribus", ALGO_TRIBUS, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 2, 8 * 16 * 4194304, 0, tribus_regenhash, NULL, precalc_hash_tribus, queue_tribus_kernel, gen_hash, append_x11_compiler_options, NULL},
  { "skunk", ALGO_SKUNK, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 3, 8 * 16 * 4194304, 0, skunk_regenhash, skunk_midstate, skunk_prepare_work, queue_skunk_kernel, gen_hash, append_x11_compiler_options },
  { "aergo", ALGO_AERGO, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x00ffffffUL, 27, 8 * 16 * 4194304, 0, aergo_regenhash, NULL, NULL, queue_aergo_kernel, gen_hash, append_x13_compiler_options },
  { "c11", ALGO_C11, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 10,  8 * 16 * 4194304, 0, c11_regenhash, NULL, NULL, queue_c11_kernel, gen_hash, append_x11_compiler_options},
  { "polytimos", ALGO_POLYTIMOS, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 5, 8 * 16 * 4194304, 0, polytimos_regenhash, NULL, NULL, queue_polytimos_kernel, gen_hash, append_x11_compiler_options },
  { "geek", ALGO_GEEK, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 8, 4 * 16 * 4194304, 0, geek_regenhash, NULL, NULL, queue_geek_kernel, gen_hash, append_x13_compiler_options },

  { "talkcoin-mod", ALGO_NIST, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 4, 8 * 16 * 4194304, 0, talkcoin_regenhash, NULL, NULL, queue_talkcoin_mod_kernel, gen_hash, append_x11_compiler_options },

  { "fresh", ALGO_FRESH, "", 1, 256, 256, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 4, 4 * 16 * 4194304, 0, fresh_regenhash, NULL, NULL, queue_fresh_kernel, gen_hash, NULL },

  { "lyra2re", ALGO_LYRA2RE, "", 1, 128, 128, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 4, 2 * 8 * 4194304, 0, lyra2re_regenhash, blake256_midstate, blake256_prepare_work, queue_lyra2re_kernel, gen_hash, NULL },
  { "lyra2rev2", ALGO_LYRA2REV2, "", 1, 256, 256, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 6, -1, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, lyra2rev2_regenhash, blake256_midstate, blake256_prepare_work, queue_lyra2rev2_kernel, gen_hash, append_neoscrypt_compiler_options },
  { "lyra2Z"   , ALGO_LYRA2Z   , "", 1, 256, 256, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 1, 0, 0, lyra2Z_regenhash,  blake256_midstate, blake256_prepare_work, queue_lyra2z_kernel, gen_hash, append_neoscrypt_compiler_options, NULL,
  {
	initialize_lyra2Z_kernel,
	truly_enqueue_lyra2Z_kernel
  } 
},
  { "lyra2rev2.mdz", ALGO_LYRA2REV2, "", 1, 256, 256, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 6, -1, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, lyra2rev2_regenhash, blake256_midstate, blake256_prepare_work, queue_lyra2rev2_kernel, gen_hash, append_neoscrypt_compiler_options, NULL,
{
	initialize_lyra2rev2_mdz,
	truly_enqueue_lyra2rev2_mdz_kernel
} },

  // kernels starting from this will have difficulty calculated by using fuguecoin algorithm
#define A_FUGUE(a, b, c) \
  { a, ALGO_FUGUE, "", 1, 256, 256, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 0, 0, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, b, NULL, NULL, queue_sph_kernel, c, NULL }
  A_FUGUE("fuguecoin", fuguecoin_regenhash, sha256),
  A_FUGUE("groestlcoin", groestlcoin_regenhash, sha256),
  A_FUGUE("diamond", groestlcoin_regenhash, gen_hash),
#undef A_FUGUE

  { "whirlcoin", ALGO_WHIRL, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 3, 8 * 16 * 4194304, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, whirlcoin_regenhash, NULL, NULL, queue_whirlcoin_kernel, sha256, NULL },
  { "whirlpoolx", ALGO_WHIRLPOOLX, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x0000FFFFUL, 0, 0, 0, whirlpoolx_regenhash, NULL, NULL, queue_whirlpoolx_kernel, gen_hash, NULL },

  { "blake256r8",  ALGO_BLAKECOIN, "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x000000ffUL, 0, 128, 0, blakecoin_regenhash, NULL, precalc_hash_blakecoin, queue_blake_kernel, sha256,   NULL },
  { "blake256r14", ALGO_BLAKE,     "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x00000000UL, 0, 128, 0, blake256_regenhash, blake256_midstate, blake256_prepare_work, queue_blake_kernel, gen_hash, NULL },
  { "vanilla",     ALGO_VANILLA,   "", 1, 1, 1, 0, 0, 0xFF, 0xFFFFULL, 0x000000ffUL, 0, 128, 0, blakecoin_regenhash, NULL, precalc_hash_blakecoin, queue_blake_kernel, gen_hash, NULL },

  { "ethash",        ALGO_ETHASH,   "", 0x100010001LLU, 0x100010001LLU, 0x100010001LLU, 0, 0, 0xFF, 0xFFFF000000000000ULL, 72UL, 0, 128, 0, ethash_regenhash, NULL, NULL, queue_ethash_kernel, gen_hash, append_ethash_compiler_options },
  { "ethash-genoil", ALGO_ETHASH,   "", 0x100010001LLU, 0x100010001LLU, 0x100010001LLU, 0, 0, 0xFF, 0xFFFF000000000000ULL, 72UL, 0, 128, 0, ethash_regenhash, NULL, NULL, queue_ethash_kernel, gen_hash, append_ethash_compiler_options },
  { "ethash-new",    ALGO_ETHASH,   "", 0x100010001LLU, 0x100010001LLU, 0x100010001LLU, 0, 0, 0xFF, 0xFFFF000000000000ULL, 72UL, 0, 128, 0, ethash_regenhash, NULL, NULL, queue_ethash_kernel, gen_hash, append_ethash_compiler_options },

  { "cryptonight", ALGO_CRYPTONIGHT, "", 1, 0x100010001LLU, 0x100010001LLU, 0, 0, 0xFF, 0xFFFFULL, 0x0000ffffUL, 6, 0, 0, cryptonight_regenhash, NULL, NULL, queue_cryptonight_kernel, gen_hash, NULL },

  { "equihash",     ALGO_EQUIHASH,   "", 1, (1ULL << 28), (1ULL << 28), 0, 0, 0x20000, 0xFFFF000000000000ULL, 0x00000000UL, 0, 128, 0, equihash_regenhash, NULL, NULL, queue_equihash_kernel, gen_hash, append_equihash_compiler_options },

  // Terminator (do not remove)
  { NULL, ALGO_UNK, "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL }
};

void copy_algorithm_settings(algorithm_t* dest, const char* algo)
{
  algorithm_settings_t* src;

  // Find algorithm settings and copy
  for (src = algos; src->name; src++)
  {
    if (strcasecmp(src->name, algo) == 0)
    {
      strcpy(dest->name, src->name);
      dest->kernelfile = src->kernelfile;
      dest->type = src->type;

      dest->diff_multiplier1 = src->diff_multiplier1;
      dest->diff_multiplier2 = src->diff_multiplier2;
      dest->share_diff_multiplier = src->share_diff_multiplier;
      dest->xintensity_shift = src->xintensity_shift;
      dest->intensity_shift = src->intensity_shift;
      dest->found_idx = src->found_idx;
      dest->diff_numerator = src->diff_numerator;
      dest->diff1targ = src->diff1targ;
      dest->n_extra_kernels = src->n_extra_kernels;
      dest->rw_buffer_size = src->rw_buffer_size;
      dest->cq_properties = src->cq_properties;
      dest->regenhash = src->regenhash;
	  dest->calc_midstate = src->calc_midstate;
	  dest->prepare_work = src->prepare_work;
      dest->queue_kernel = src->queue_kernel;
      dest->enqueue_kernels = src->enqueue_kernels;
      dest->gen_hash = src->gen_hash;
      dest->set_compile_options = src->set_compile_options;
	  dest->flexibility = src->flexibility;
      break;
    }
  }

  // if not found
  if (src->name == NULL)
  {
    applog(LOG_WARNING, "Algorithm %s not found, using %s.", algo, algos->name);
    copy_algorithm_settings(dest, algos->name);
  }
}

static const char *lookup_algorithm_alias(const char *lookup_alias, uint8_t *nfactor)
{
#define ALGO_ALIAS_NF(alias, name, nf) \
  if (strcasecmp(alias, lookup_alias) == 0) { *nfactor = nf; return name; }
#define ALGO_ALIAS(alias, name) \
  if (strcasecmp(alias, lookup_alias) == 0) return name;

  ALGO_ALIAS_NF("scrypt", "ckolivas", 10);
  ALGO_ALIAS_NF("scrypt", "ckolivas", 10);
  ALGO_ALIAS_NF("adaptive-n-factor", "ckolivas", 11);
  ALGO_ALIAS_NF("adaptive-nfactor", "ckolivas", 11);
  ALGO_ALIAS_NF("nscrypt", "ckolivas", 11);
  ALGO_ALIAS_NF("adaptive-nscrypt", "ckolivas", 11);
  ALGO_ALIAS_NF("adaptive-n-scrypt", "ckolivas", 11);
  ALGO_ALIAS("x11mod", "darkcoin-mod");
  ALGO_ALIAS("x11", "darkcoin-mod");
  ALGO_ALIAS("x13mod", "marucoin-mod");
  ALGO_ALIAS("x13", "marucoin-mod");
  ALGO_ALIAS("x13old", "marucoin-modold");
  ALGO_ALIAS("x13modold", "marucoin-modold");
  ALGO_ALIAS("x15mod", "bitblock");
  ALGO_ALIAS("x15", "bitblock");
  ALGO_ALIAS("x15modold", "bitblockold");
  ALGO_ALIAS("x15old", "bitblockold");
  ALGO_ALIAS("x16r", "x16r");
  ALGO_ALIAS("x16s", "x16s");
  ALGO_ALIAS("x17", "x17");
  ALGO_ALIAS("xevan", "xevan");
  ALGO_ALIAS("phi", "phi");
  ALGO_ALIAS("nist5", "talkcoin-mod");
  ALGO_ALIAS("keccak", "maxcoin");
  ALGO_ALIAS("whirlpool", "whirlcoin");
  ALGO_ALIAS("lyra2", "lyra2re");
  ALGO_ALIAS("lyra2v2", "lyra2rev2");
  ALGO_ALIAS("blakecoin", "blake256r8");
  ALGO_ALIAS("blake", "blake256r14");
  ALGO_ALIAS("zcash", "equihash");
  ALGO_ALIAS("thorsriddle", "polytimos");

#undef ALGO_ALIAS
#undef ALGO_ALIAS_NF

  return NULL;
}

void set_algorithm(algorithm_t* algo, const char* newname_alias)
{
  const char *newname;

  //load previous algorithm nfactor in case nfactor was applied before algorithm... or default to 10
  uint8_t old_nfactor = ((algo->nfactor) ? algo->nfactor : 0);
  //load previous kernel file name if was applied before algorithm...
  const char *kernelfile = algo->kernelfile;
  uint8_t nfactor = 10;

  if (!(newname = lookup_algorithm_alias(newname_alias, &nfactor))) {
    newname = newname_alias;
  }

  copy_algorithm_settings(algo, newname);

  // use old nfactor if it was previously set and is different than the one set by alias
  if ((old_nfactor > 0) && (old_nfactor != nfactor)) {
    nfactor = old_nfactor;
  }

  set_algorithm_nfactor(algo, nfactor);

  //reapply kernelfile if was set
  if (!empty_string(kernelfile)) {
    algo->kernelfile = kernelfile;
  }
}

void set_algorithm_nfactor(algorithm_t* algo, const uint8_t nfactor)
{
  algo->nfactor = nfactor;
  algo->n = (1 << nfactor);

  //adjust algo type accordingly
  switch (algo->type)
  {
  case ALGO_SCRYPT:
    //if nfactor isnt 10, switch to NSCRYPT
    if (algo->nfactor != 10)
      algo->type = ALGO_NSCRYPT;
    break;
    //nscrypt
  case ALGO_NSCRYPT:
    //if nfactor is 10, switch to SCRYPT
    if (algo->nfactor == 10)
      algo->type = ALGO_SCRYPT;
    break;
    //ignore rest
  default:
    break;
  }
}

bool cmp_algorithm(const algorithm_t* algo1, const algorithm_t* algo2)
{
  return (!safe_cmp(algo1->name, algo2->name) && !safe_cmp(algo1->kernelfile, algo2->kernelfile) && (algo1->nfactor == algo2->nfactor));
}
