/* $Id: simd.c 227 2010-06-16 17:28:38Z tp $ */
/*
 * SIMD implementation.
 *
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2007-2010  Projet RNRT SAPHIR
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   Thomas Pornin <thomas.pornin@cryptolog.com>
 */
 typedef sph_u32 u32;
typedef sph_s32 s32;
#define C32     SPH_C32
#define T32     SPH_T32
#define ROL32   SPH_ROTL32
 #define XCAT(x, y)    XCAT_(x, y)
#define XCAT_(x, y)   x ## y
 __constant static const s32 SIMD_Q[] = {
  4, 28, -80, -120, -47, -126, 45, -123, -92, -127, -70, 23, -23, -24, 40, -125, 101, 122, 34, -24, -119, 110, -121, -112, 32, 24, 51, 73, -117, -64, -21, 42, -60, 16, 5, 85, 107, 52, -44, -96, 42, 127, -18, -108, -47, 26, 91, 117, 112, 46, 87, 79, 126, -120, 65, -24, 121, 29, 118, -7, -53, 85, -98, -117, 32, 115, -47, -116, 63, 16, -108, 49, -119, 57, -110, 4, -76, -76, -42, -86, 58, 115, 4, 4, -83, -51, -37, 116, 32, 15, 36, -42, 73, -99, 94, 87, 60, -20, 67, 12, -76, 55, 117, -68, -82, -80, 93, -20, 92, -21, -128, -91, -11, 84, -28, 76, 94, -124, 37, 93, 17, -78, -106, -29, 88, -15, -47, 102, -4, -28, 80, 120, 47, 126, -45, 123, 92, 127, 70, -23, 23, 24, -40, 125, -101, -122, -34, 24, 119, -110, 121, 112, -32, -24, -51, -73, 117, 64, 21, -42, 60, -16, -5, -85, -107, -52, 44, 96, -42, -127, 18, 108, 47, -26, -91, -117, -112, -46, -87, -79, -126, 120, -65, 24, -121, -29, -118, 7, 53, -85, 98, 117, -32, -115, 47, 116, -63, -16, 108, -49, 119, -57, 110, -4, 76, 76, 42, 86, -58, -115, -4, -4, 83, 51, 37, -116, -32, -15, -36, 42, -73, 99, -94, -87, -60, 20, -67, -12, 76, -55, -117, 68, 82, 80, -93, 20, -92, 21, 128, 91, 11, -84, 28, -76, -94, 124, -37, -93, -17, 78, 106, 29, -88, 15, 47, -102
};

__constant static const s32 SIMD_Q_80[] = {
	-125, -101, 48, 8, 81, 2, -84, 5, 36, 1, 58, -106, 105, 104, -89, 3, -28, -7, -95, 104, 9, -19, 7, 16, -97, -105, -78, -56, 11, 64, 107, -87, 68, -113, -124, -44, -22, -77, 84, 32, -87, -2, 110, 20, 81, -103, -38, -12, -17, -83, -42, -50, -3, 8, -64, 104, -8, -100, -11, 121, 75, -44, 30, 11, -97, -14, 81, 12, -66, -113, 20, -80, 9, -72, 18, -125, 52, 52, 86, 42, -71, -14, -125, -125, 45, 77, 91, -13, -97, -114, -93, 86, -56, 29, -35, -42, -69, 108, -62, -117, 52, -74, -12, 60, 46, 48, -36, 108, -37, 107, 0, 37, 117, -45, 100, -53, -35, 4, -92, -36, -112, 50, 22, 99, -41, 113, 81, -27, 124, 100, -49, -9, -82, -3, 83, -6, -37, -2, -59, 105, -106, -105, 88, -4, 27, 6, 94, -105, -10, 18, -8, -17, 96, 104, 77, 55, -12, -65, -108, 86, -69, 112, 123, 43, 21, 76, -85, -33, 86, 1, -111, -21, -82, 102, 37, 11, 16, 82, 41, 49, 2, -9, 63, -105, 7, 99, 10, -122, -76, 43, -31, -12, 96, 13, -82, -13, 65, 112, -21, 79, -10, 71, -19, 124, -53, -53, -87, -43, 70, 13, 124, 124, -46, -78, -92, 12, 96, 113, 92, -87, 55, -30, 34, 41, 68, -109, 61, 116, -53, 73, 11, -61, -47, -49, 35, -109, 36, -108, -1, -38, -118, 44, -101, 52, 34, -5, 91, 35, 111, -51, -23, -100, 40, -114, -82, 26
};

 /*
 * The powers of 41 modulo 257. We use exponents from 0 to 255, inclusive.
 */
__constant static const s32 alpha_tab[] = {
    1,  41, 139,  45,  46,  87, 226,  14,  60, 147, 116, 130,
  190,  80, 196,  69,   2,  82,  21,  90,  92, 174, 195,  28,
  120,  37, 232,   3, 123, 160, 135, 138,   4, 164,  42, 180,
  184,  91, 133,  56, 240,  74, 207,   6, 246,  63,  13,  19,
    8,  71,  84, 103, 111, 182,   9, 112, 223, 148, 157,  12,
  235, 126,  26,  38,  16, 142, 168, 206, 222, 107,  18, 224,
  189,  39,  57,  24, 213, 252,  52,  76,  32,  27,  79, 155,
  187, 214,  36, 191, 121,  78, 114,  48, 169, 247, 104, 152,
   64,  54, 158,  53, 117, 171,  72, 125, 242, 156, 228,  96,
   81, 237, 208,  47, 128, 108,  59, 106, 234,  85, 144, 250,
  227,  55, 199, 192, 162, 217, 159,  94, 256, 216, 118, 212,
  211, 170,  31, 243, 197, 110, 141, 127,  67, 177,  61, 188,
  255, 175, 236, 167, 165,  83,  62, 229, 137, 220,  25, 254,
  134,  97, 122, 119, 253,  93, 215,  77,  73, 166, 124, 201,
   17, 183,  50, 251,  11, 194, 244, 238, 249, 186, 173, 154,
  146,  75, 248, 145,  34, 109, 100, 245,  22, 131, 231, 219,
  241, 115,  89,  51,  35, 150, 239,  33,  68, 218, 200, 233,
   44,   5, 205, 181, 225, 230, 178, 102,  70,  43, 221,  66,
  136, 179, 143, 209,  88,  10, 153, 105, 193, 203,  99, 204,
  140,  86, 185, 132,  15, 101,  29, 161, 176,  20,  49, 210,
  129, 149, 198, 151,  23, 172, 113,   7,  30, 202,  58,  65,
   95,  40,  98, 163
};
 /*
 * Ranges:
 *   REDS1: from -32768..98302 to -383..383
 *   REDS2: from -2^31..2^31-1 to -32768..98302
 */
#define REDS1(x)    (((x) & 0xFF) - ((x) >> 8))
#define REDS2(x)    (((x) & 0xFFFF) + ((x) >> 16))
 /*
 * If, upon entry, the values of q[] are all in the -N..N range (where
 * N >= 98302) then the new values of q[] are in the -2N..2N range.
 *
 * Since alpha_tab[v] <= 256, maximum allowed range is for N = 8388608.
 */
#define SIMD_LQ(i) (q[(i)])
#define PRAGMA(X) _Pragma(#X)
#define PRAGMA_UNROLL PRAGMA(unroll)
#define PRAGMA_NOUNROLL PRAGMA(nounroll)
 #define FFT_LOOP_16_8(rb)   do { \
    s32 m = q[(rb)]; \
    s32 n = q[(rb) + 16]; \
    q[(rb)] = m + n; \
    q[(rb) + 16] = m - n; \
    s32 t; \
    m = q[(rb) + 0 + 1]; \
    n = q[(rb) + 0 + 1 + 16]; \
    t = REDS2(n * alpha_tab[0 + 1 * 8]); \
    q[(rb) + 0 + 1] = m + t; \
    q[(rb) + 0 + 1 + 16] = m - t; \
    m = q[(rb) + 0 + 2]; \
    n = q[(rb) + 0 + 2 + 16]; \
    t = REDS2(n * alpha_tab[0 + 2 * 8]); \
    q[(rb) + 0 + 2] = m + t; \
    q[(rb) + 0 + 2 + 16] = m - t; \
    m = q[(rb) + 0 + 3]; \
    n = q[(rb) + 0 + 3 + 16]; \
    t = REDS2(n * alpha_tab[0 + 3 * 8]); \
    q[(rb) + 0 + 3] = m + t; \
    q[(rb) + 0 + 3 + 16] = m - t; \
    \
    m = q[(rb) + 4 + 0]; \
    n = q[(rb) + 4 + 0 + 16]; \
    t = REDS2(n * alpha_tab[32 + 0 * 8]); \
    q[(rb) + 4 + 0] = m + t; \
    q[(rb) + 4 + 0 + 16] = m - t; \
    m = q[(rb) + 4 + 1]; \
    n = q[(rb) + 4 + 1 + 16]; \
    t = REDS2(n * alpha_tab[32 + 1 * 8]); \
    q[(rb) + 4 + 1] = m + t; \
    q[(rb) + 4 + 1 + 16] = m - t; \
    m = q[(rb) + 4 + 2]; \
    n = q[(rb) + 4 + 2 + 16]; \
    t = REDS2(n * alpha_tab[32 + 2 * 8]); \
    q[(rb) + 4 + 2] = m + t; \
    q[(rb) + 4 + 2 + 16] = m - t; \
    m = q[(rb) + 4 + 3]; \
    n = q[(rb) + 4 + 3 + 16]; \
    t = REDS2(n * alpha_tab[32 + 3 * 8]); \
    q[(rb) + 4 + 3] = m + t; \
    q[(rb) + 4 + 3 + 16] = m - t; \
    \
    m = q[(rb) + 8 + 0]; \
    n = q[(rb) + 8 + 0 + 16]; \
    t = REDS2(n * alpha_tab[64 + 0 * 8]); \
    q[(rb) + 8 + 0] = m + t; \
    q[(rb) + 8 + 0 + 16] = m - t; \
    m = q[(rb) + 8 + 1]; \
    n = q[(rb) + 8 + 1 + 16]; \
    t = REDS2(n * alpha_tab[64 + 1 * 8]); \
    q[(rb) + 8 + 1] = m + t; \
    q[(rb) + 8 + 1 + 16] = m - t; \
    m = q[(rb) + 8 + 2]; \
    n = q[(rb) + 8 + 2 + 16]; \
    t = REDS2(n * alpha_tab[64 + 2 * 8]); \
    q[(rb) + 8 + 2] = m + t; \
    q[(rb) + 8 + 2 + 16] = m - t; \
    m = q[(rb) + 8 + 3]; \
    n = q[(rb) + 8 + 3 + 16]; \
    t = REDS2(n * alpha_tab[64 + 3 * 8]); \
    q[(rb) + 8 + 3] = m + t; \
    q[(rb) + 8 + 3 + 16] = m - t; \
    \
    m = q[(rb) + 12 + 0]; \
    n = q[(rb) + 12 + 0 + 16]; \
    t = REDS2(n * alpha_tab[96 + 0 * 8]); \
    q[(rb) + 12 + 0] = m + t; \
    q[(rb) + 12 + 0 + 16] = m - t; \
    m = q[(rb) + 12 + 1]; \
    n = q[(rb) + 12 + 1 + 16]; \
    t = REDS2(n * alpha_tab[96 + 1 * 8]); \
    q[(rb) + 12 + 1] = m + t; \
    q[(rb) + 12 + 1 + 16] = m - t; \
    m = q[(rb) + 12 + 2]; \
    n = q[(rb) + 12 + 2 + 16]; \
    t = REDS2(n * alpha_tab[96 + 2 * 8]); \
    q[(rb) + 12 + 2] = m + t; \
    q[(rb) + 12 + 2 + 16] = m - t; \
    m = q[(rb) + 12 + 3]; \
    n = q[(rb) + 12 + 3 + 16]; \
    t = REDS2(n * alpha_tab[96 + 3 * 8]); \
    q[(rb) + 12 + 3] = m + t; \
    q[(rb) + 12 + 3 + 16] = m - t; \
  } while (0)
 #define FFT_LOOP_32_4(rb)   do { \
    s32 m = q[(rb)]; \
    s32 n = q[(rb) + 32]; \
    q[(rb)] = m + n; \
    q[(rb) + 32] = m - n; \
    s32 t; \
    m = q[(rb) + 0 + 1]; \
    n = q[(rb) + 0 + 1 + 32]; \
    t = REDS2(n * alpha_tab[0 + 1 * 4]); \
    q[(rb) + 0 + 1] = m + t; \
    q[(rb) + 0 + 1 + 32] = m - t; \
    m = q[(rb) + 0 + 2]; \
    n = q[(rb) + 0 + 2 + 32]; \
    t = REDS2(n * alpha_tab[0 + 2 * 4]); \
    q[(rb) + 0 + 2] = m + t; \
    q[(rb) + 0 + 2 + 32] = m - t; \
    m = q[(rb) + 0 + 3]; \
    n = q[(rb) + 0 + 3 + 32]; \
    t = REDS2(n * alpha_tab[0 + 3 * 4]); \
    q[(rb) + 0 + 3] = m + t; \
    q[(rb) + 0 + 3 + 32] = m - t; \
    \
    m = q[(rb) + 4 + 0]; \
    n = q[(rb) + 4 + 0 + 32]; \
    t = REDS2(n * alpha_tab[16 + 0 * 4]); \
    q[(rb) + 4 + 0] = m + t; \
    q[(rb) + 4 + 0 + 32] = m - t; \
    m = q[(rb) + 4 + 1]; \
    n = q[(rb) + 4 + 1 + 32]; \
    t = REDS2(n * alpha_tab[16 + 1 * 4]); \
    q[(rb) + 4 + 1] = m + t; \
    q[(rb) + 4 + 1 + 32] = m - t; \
    m = q[(rb) + 4 + 2]; \
    n = q[(rb) + 4 + 2 + 32]; \
    t = REDS2(n * alpha_tab[16 + 2 * 4]); \
    q[(rb) + 4 + 2] = m + t; \
    q[(rb) + 4 + 2 + 32] = m - t; \
    m = q[(rb) + 4 + 3]; \
    n = q[(rb) + 4 + 3 + 32]; \
    t = REDS2(n * alpha_tab[16 + 3 * 4]); \
    q[(rb) + 4 + 3] = m + t; \
    q[(rb) + 4 + 3 + 32] = m - t; \
    \
    m = q[(rb) + 8 + 0]; \
    n = q[(rb) + 8 + 0 + 32]; \
    t = REDS2(n * alpha_tab[32 + 0 * 4]); \
    q[(rb) + 8 + 0] = m + t; \
    q[(rb) + 8 + 0 + 32] = m - t; \
    m = q[(rb) + 8 + 1]; \
    n = q[(rb) + 8 + 1 + 32]; \
    t = REDS2(n * alpha_tab[32 + 1 * 4]); \
    q[(rb) + 8 + 1] = m + t; \
    q[(rb) + 8 + 1 + 32] = m - t; \
    m = q[(rb) + 8 + 2]; \
    n = q[(rb) + 8 + 2 + 32]; \
    t = REDS2(n * alpha_tab[32 + 2 * 4]); \
    q[(rb) + 8 + 2] = m + t; \
    q[(rb) + 8 + 2 + 32] = m - t; \
    m = q[(rb) + 8 + 3]; \
    n = q[(rb) + 8 + 3 + 32]; \
    t = REDS2(n * alpha_tab[32 + 3 * 4]); \
    q[(rb) + 8 + 3] = m + t; \
    q[(rb) + 8 + 3 + 32] = m - t; \
    \
    m = q[(rb) + 12 + 0]; \
    n = q[(rb) + 12 + 0 + 32]; \
    t = REDS2(n * alpha_tab[48 + 0 * 4]); \
    q[(rb) + 12 + 0] = m + t; \
    q[(rb) + 12 + 0 + 32] = m - t; \
    m = q[(rb) + 12 + 1]; \
    n = q[(rb) + 12 + 1 + 32]; \
    t = REDS2(n * alpha_tab[48 + 1 * 4]); \
    q[(rb) + 12 + 1] = m + t; \
    q[(rb) + 12 + 1 + 32] = m - t; \
    m = q[(rb) + 12 + 2]; \
    n = q[(rb) + 12 + 2 + 32]; \
    t = REDS2(n * alpha_tab[48 + 2 * 4]); \
    q[(rb) + 12 + 2] = m + t; \
    q[(rb) + 12 + 2 + 32] = m - t; \
    m = q[(rb) + 12 + 3]; \
    n = q[(rb) + 12 + 3 + 32]; \
    t = REDS2(n * alpha_tab[48 + 3 * 4]); \
    q[(rb) + 12 + 3] = m + t; \
    q[(rb) + 12 + 3 + 32] = m - t; \
    \
    m = q[(rb) + 16 + 0]; \
    n = q[(rb) + 16 + 0 + 32]; \
    t = REDS2(n * alpha_tab[64 + 0 * 4]); \
    q[(rb) + 16 + 0] = m + t; \
    q[(rb) + 16 + 0 + 32] = m - t; \
    m = q[(rb) + 16 + 1]; \
    n = q[(rb) + 16 + 1 + 32]; \
    t = REDS2(n * alpha_tab[64 + 1 * 4]); \
    q[(rb) + 16 + 1] = m + t; \
    q[(rb) + 16 + 1 + 32] = m - t; \
    m = q[(rb) + 16 + 2]; \
    n = q[(rb) + 16 + 2 + 32]; \
    t = REDS2(n * alpha_tab[64 + 2 * 4]); \
    q[(rb) + 16 + 2] = m + t; \
    q[(rb) + 16 + 2 + 32] = m - t; \
    m = q[(rb) + 16 + 3]; \
    n = q[(rb) + 16 + 3 + 32]; \
    t = REDS2(n * alpha_tab[64 + 3 * 4]); \
    q[(rb) + 16 + 3] = m + t; \
    q[(rb) + 16 + 3 + 32] = m - t; \
    \
    m = q[(rb) + 20 + 0]; \
    n = q[(rb) + 20 + 0 + 32]; \
    t = REDS2(n * alpha_tab[80 + 0 * 4]); \
    q[(rb) + 20 + 0] = m + t; \
    q[(rb) + 20 + 0 + 32] = m - t; \
    m = q[(rb) + 20 + 1]; \
    n = q[(rb) + 20 + 1 + 32]; \
    t = REDS2(n * alpha_tab[80 + 1 * 4]); \
    q[(rb) + 20 + 1] = m + t; \
    q[(rb) + 20 + 1 + 32] = m - t; \
    m = q[(rb) + 20 + 2]; \
    n = q[(rb) + 20 + 2 + 32]; \
    t = REDS2(n * alpha_tab[80 + 2 * 4]); \
    q[(rb) + 20 + 2] = m + t; \
    q[(rb) + 20 + 2 + 32] = m - t; \
    m = q[(rb) + 20 + 3]; \
    n = q[(rb) + 20 + 3 + 32]; \
    t = REDS2(n * alpha_tab[80 + 3 * 4]); \
    q[(rb) + 20 + 3] = m + t; \
    q[(rb) + 20 + 3 + 32] = m - t; \
    \
    m = q[(rb) + 24 + 0]; \
    n = q[(rb) + 24 + 0 + 32]; \
    t = REDS2(n * alpha_tab[96 + 0 * 4]); \
    q[(rb) + 24 + 0] = m + t; \
    q[(rb) + 24 + 0 + 32] = m - t; \
    m = q[(rb) + 24 + 1]; \
    n = q[(rb) + 24 + 1 + 32]; \
    t = REDS2(n * alpha_tab[96 + 1 * 4]); \
    q[(rb) + 24 + 1] = m + t; \
    q[(rb) + 24 + 1 + 32] = m - t; \
    m = q[(rb) + 24 + 2]; \
    n = q[(rb) + 24 + 2 + 32]; \
    t = REDS2(n * alpha_tab[96 + 2 * 4]); \
    q[(rb) + 24 + 2] = m + t; \
    q[(rb) + 24 + 2 + 32] = m - t; \
    m = q[(rb) + 24 + 3]; \
    n = q[(rb) + 24 + 3 + 32]; \
    t = REDS2(n * alpha_tab[96 + 3 * 4]); \
    q[(rb) + 24 + 3] = m + t; \
    q[(rb) + 24 + 3 + 32] = m - t; \
    \
    m = q[(rb) + 28 + 0]; \
    n = q[(rb) + 28 + 0 + 32]; \
    t = REDS2(n * alpha_tab[112 + 0 * 4]); \
    q[(rb) + 28 + 0] = m + t; \
    q[(rb) + 28 + 0 + 32] = m - t; \
    m = q[(rb) + 28 + 1]; \
    n = q[(rb) + 28 + 1 + 32]; \
    t = REDS2(n * alpha_tab[112 + 1 * 4]); \
    q[(rb) + 28 + 1] = m + t; \
    q[(rb) + 28 + 1 + 32] = m - t; \
    m = q[(rb) + 28 + 2]; \
    n = q[(rb) + 28 + 2 + 32]; \
    t = REDS2(n * alpha_tab[112 + 2 * 4]); \
    q[(rb) + 28 + 2] = m + t; \
    q[(rb) + 28 + 2 + 32] = m - t; \
    m = q[(rb) + 28 + 3]; \
    n = q[(rb) + 28 + 3 + 32]; \
    t = REDS2(n * alpha_tab[112 + 3 * 4]); \
    q[(rb) + 28 + 3] = m + t; \
    q[(rb) + 28 + 3 + 32] = m - t; \
  } while (0)
 #define FFT_LOOP_64_2(rb)  do { \
    s32 m = q[(rb)]; \
    s32 n = q[(rb) + 64]; \
    q[(rb)] = m + n; \
    q[(rb) + 64] = m - n; \
    s32 t; \
    m = q[(rb) + 0 + 1]; \
    n = q[(rb) + 0 + 1 + 64]; \
    t = REDS2(n * alpha_tab[0 + 1 * 2]); \
    q[(rb) + 0 + 1] = m + t; \
    q[(rb) + 0 + 1 + 64] = m - t; \
    m = q[(rb) + 0 + 2]; \
    n = q[(rb) + 0 + 2 + 64]; \
    t = REDS2(n * alpha_tab[0 + 2 * 2]); \
    q[(rb) + 0 + 2] = m + t; \
    q[(rb) + 0 + 2 + 64] = m - t; \
    m = q[(rb) + 0 + 3]; \
    n = q[(rb) + 0 + 3 + 64]; \
    t = REDS2(n * alpha_tab[0 + 3 * 2]); \
    q[(rb) + 0 + 3] = m + t; \
    q[(rb) + 0 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 4 + 0]; \
    n = q[(rb) + 4 + 0 + 64]; \
    t = REDS2(n * alpha_tab[8 + 0 * 2]); \
    q[(rb) + 4 + 0] = m + t; \
    q[(rb) + 4 + 0 + 64] = m - t; \
    m = q[(rb) + 4 + 1]; \
    n = q[(rb) + 4 + 1 + 64]; \
    t = REDS2(n * alpha_tab[8 + 1 * 2]); \
    q[(rb) + 4 + 1] = m + t; \
    q[(rb) + 4 + 1 + 64] = m - t; \
    m = q[(rb) + 4 + 2]; \
    n = q[(rb) + 4 + 2 + 64]; \
    t = REDS2(n * alpha_tab[8 + 2 * 2]); \
    q[(rb) + 4 + 2] = m + t; \
    q[(rb) + 4 + 2 + 64] = m - t; \
    m = q[(rb) + 4 + 3]; \
    n = q[(rb) + 4 + 3 + 64]; \
    t = REDS2(n * alpha_tab[8 + 3 * 2]); \
    q[(rb) + 4 + 3] = m + t; \
    q[(rb) + 4 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 8 + 0]; \
    n = q[(rb) + 8 + 0 + 64]; \
    t = REDS2(n * alpha_tab[16 + 0 * 2]); \
    q[(rb) + 8 + 0] = m + t; \
    q[(rb) + 8 + 0 + 64] = m - t; \
    m = q[(rb) + 8 + 1]; \
    n = q[(rb) + 8 + 1 + 64]; \
    t = REDS2(n * alpha_tab[16 + 1 * 2]); \
    q[(rb) + 8 + 1] = m + t; \
    q[(rb) + 8 + 1 + 64] = m - t; \
    m = q[(rb) + 8 + 2]; \
    n = q[(rb) + 8 + 2 + 64]; \
    t = REDS2(n * alpha_tab[16 + 2 * 2]); \
    q[(rb) + 8 + 2] = m + t; \
    q[(rb) + 8 + 2 + 64] = m - t; \
    m = q[(rb) + 8 + 3]; \
    n = q[(rb) + 8 + 3 + 64]; \
    t = REDS2(n * alpha_tab[16 + 3 * 2]); \
    q[(rb) + 8 + 3] = m + t; \
    q[(rb) + 8 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 12 + 0]; \
    n = q[(rb) + 12 + 0 + 64]; \
    t = REDS2(n * alpha_tab[24 + 0 * 2]); \
    q[(rb) + 12 + 0] = m + t; \
    q[(rb) + 12 + 0 + 64] = m - t; \
    m = q[(rb) + 12 + 1]; \
    n = q[(rb) + 12 + 1 + 64]; \
    t = REDS2(n * alpha_tab[24 + 1 * 2]); \
    q[(rb) + 12 + 1] = m + t; \
    q[(rb) + 12 + 1 + 64] = m - t; \
    m = q[(rb) + 12 + 2]; \
    n = q[(rb) + 12 + 2 + 64]; \
    t = REDS2(n * alpha_tab[24 + 2 * 2]); \
    q[(rb) + 12 + 2] = m + t; \
    q[(rb) + 12 + 2 + 64] = m - t; \
    m = q[(rb) + 12 + 3]; \
    n = q[(rb) + 12 + 3 + 64]; \
    t = REDS2(n * alpha_tab[24 + 3 * 2]); \
    q[(rb) + 12 + 3] = m + t; \
    q[(rb) + 12 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 16 + 0]; \
    n = q[(rb) + 16 + 0 + 64]; \
    t = REDS2(n * alpha_tab[32 + 0 * 2]); \
    q[(rb) + 16 + 0] = m + t; \
    q[(rb) + 16 + 0 + 64] = m - t; \
    m = q[(rb) + 16 + 1]; \
    n = q[(rb) + 16 + 1 + 64]; \
    t = REDS2(n * alpha_tab[32 + 1 * 2]); \
    q[(rb) + 16 + 1] = m + t; \
    q[(rb) + 16 + 1 + 64] = m - t; \
    m = q[(rb) + 16 + 2]; \
    n = q[(rb) + 16 + 2 + 64]; \
    t = REDS2(n * alpha_tab[32 + 2 * 2]); \
    q[(rb) + 16 + 2] = m + t; \
    q[(rb) + 16 + 2 + 64] = m - t; \
    m = q[(rb) + 16 + 3]; \
    n = q[(rb) + 16 + 3 + 64]; \
    t = REDS2(n * alpha_tab[32 + 3 * 2]); \
    q[(rb) + 16 + 3] = m + t; \
    q[(rb) + 16 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 20 + 0]; \
    n = q[(rb) + 20 + 0 + 64]; \
    t = REDS2(n * alpha_tab[40 + 0 * 2]); \
    q[(rb) + 20 + 0] = m + t; \
    q[(rb) + 20 + 0 + 64] = m - t; \
    m = q[(rb) + 20 + 1]; \
    n = q[(rb) + 20 + 1 + 64]; \
    t = REDS2(n * alpha_tab[40 + 1 * 2]); \
    q[(rb) + 20 + 1] = m + t; \
    q[(rb) + 20 + 1 + 64] = m - t; \
    m = q[(rb) + 20 + 2]; \
    n = q[(rb) + 20 + 2 + 64]; \
    t = REDS2(n * alpha_tab[40 + 2 * 2]); \
    q[(rb) + 20 + 2] = m + t; \
    q[(rb) + 20 + 2 + 64] = m - t; \
    m = q[(rb) + 20 + 3]; \
    n = q[(rb) + 20 + 3 + 64]; \
    t = REDS2(n * alpha_tab[40 + 3 * 2]); \
    q[(rb) + 20 + 3] = m + t; \
    q[(rb) + 20 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 24 + 0]; \
    n = q[(rb) + 24 + 0 + 64]; \
    t = REDS2(n * alpha_tab[48 + 0 * 2]); \
    q[(rb) + 24 + 0] = m + t; \
    q[(rb) + 24 + 0 + 64] = m - t; \
    m = q[(rb) + 24 + 1]; \
    n = q[(rb) + 24 + 1 + 64]; \
    t = REDS2(n * alpha_tab[48 + 1 * 2]); \
    q[(rb) + 24 + 1] = m + t; \
    q[(rb) + 24 + 1 + 64] = m - t; \
    m = q[(rb) + 24 + 2]; \
    n = q[(rb) + 24 + 2 + 64]; \
    t = REDS2(n * alpha_tab[48 + 2 * 2]); \
    q[(rb) + 24 + 2] = m + t; \
    q[(rb) + 24 + 2 + 64] = m - t; \
    m = q[(rb) + 24 + 3]; \
    n = q[(rb) + 24 + 3 + 64]; \
    t = REDS2(n * alpha_tab[48 + 3 * 2]); \
    q[(rb) + 24 + 3] = m + t; \
    q[(rb) + 24 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 28 + 0]; \
    n = q[(rb) + 28 + 0 + 64]; \
    t = REDS2(n * alpha_tab[56 + 0 * 2]); \
    q[(rb) + 28 + 0] = m + t; \
    q[(rb) + 28 + 0 + 64] = m - t; \
    m = q[(rb) + 28 + 1]; \
    n = q[(rb) + 28 + 1 + 64]; \
    t = REDS2(n * alpha_tab[56 + 1 * 2]); \
    q[(rb) + 28 + 1] = m + t; \
    q[(rb) + 28 + 1 + 64] = m - t; \
    m = q[(rb) + 28 + 2]; \
    n = q[(rb) + 28 + 2 + 64]; \
    t = REDS2(n * alpha_tab[56 + 2 * 2]); \
    q[(rb) + 28 + 2] = m + t; \
    q[(rb) + 28 + 2 + 64] = m - t; \
    m = q[(rb) + 28 + 3]; \
    n = q[(rb) + 28 + 3 + 64]; \
    t = REDS2(n * alpha_tab[56 + 3 * 2]); \
    q[(rb) + 28 + 3] = m + t; \
    q[(rb) + 28 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 32 + 0]; \
    n = q[(rb) + 32 + 0 + 64]; \
    t = REDS2(n * alpha_tab[64 + 0 * 2]); \
    q[(rb) + 32 + 0] = m + t; \
    q[(rb) + 32 + 0 + 64] = m - t; \
    m = q[(rb) + 32 + 1]; \
    n = q[(rb) + 32 + 1 + 64]; \
    t = REDS2(n * alpha_tab[64 + 1 * 2]); \
    q[(rb) + 32 + 1] = m + t; \
    q[(rb) + 32 + 1 + 64] = m - t; \
    m = q[(rb) + 32 + 2]; \
    n = q[(rb) + 32 + 2 + 64]; \
    t = REDS2(n * alpha_tab[64 + 2 * 2]); \
    q[(rb) + 32 + 2] = m + t; \
    q[(rb) + 32 + 2 + 64] = m - t; \
    m = q[(rb) + 32 + 3]; \
    n = q[(rb) + 32 + 3 + 64]; \
    t = REDS2(n * alpha_tab[64 + 3 * 2]); \
    q[(rb) + 32 + 3] = m + t; \
    q[(rb) + 32 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 36 + 0]; \
    n = q[(rb) + 36 + 0 + 64]; \
    t = REDS2(n * alpha_tab[72 + 0 * 2]); \
    q[(rb) + 36 + 0] = m + t; \
    q[(rb) + 36 + 0 + 64] = m - t; \
    m = q[(rb) + 36 + 1]; \
    n = q[(rb) + 36 + 1 + 64]; \
    t = REDS2(n * alpha_tab[72 + 1 * 2]); \
    q[(rb) + 36 + 1] = m + t; \
    q[(rb) + 36 + 1 + 64] = m - t; \
    m = q[(rb) + 36 + 2]; \
    n = q[(rb) + 36 + 2 + 64]; \
    t = REDS2(n * alpha_tab[72 + 2 * 2]); \
    q[(rb) + 36 + 2] = m + t; \
    q[(rb) + 36 + 2 + 64] = m - t; \
    m = q[(rb) + 36 + 3]; \
    n = q[(rb) + 36 + 3 + 64]; \
    t = REDS2(n * alpha_tab[72 + 3 * 2]); \
    q[(rb) + 36 + 3] = m + t; \
    q[(rb) + 36 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 40 + 0]; \
    n = q[(rb) + 40 + 0 + 64]; \
    t = REDS2(n * alpha_tab[80 + 0 * 2]); \
    q[(rb) + 40 + 0] = m + t; \
    q[(rb) + 40 + 0 + 64] = m - t; \
    m = q[(rb) + 40 + 1]; \
    n = q[(rb) + 40 + 1 + 64]; \
    t = REDS2(n * alpha_tab[80 + 1 * 2]); \
    q[(rb) + 40 + 1] = m + t; \
    q[(rb) + 40 + 1 + 64] = m - t; \
    m = q[(rb) + 40 + 2]; \
    n = q[(rb) + 40 + 2 + 64]; \
    t = REDS2(n * alpha_tab[80 + 2 * 2]); \
    q[(rb) + 40 + 2] = m + t; \
    q[(rb) + 40 + 2 + 64] = m - t; \
    m = q[(rb) + 40 + 3]; \
    n = q[(rb) + 40 + 3 + 64]; \
    t = REDS2(n * alpha_tab[80 + 3 * 2]); \
    q[(rb) + 40 + 3] = m + t; \
    q[(rb) + 40 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 44 + 0]; \
    n = q[(rb) + 44 + 0 + 64]; \
    t = REDS2(n * alpha_tab[88 + 0 * 2]); \
    q[(rb) + 44 + 0] = m + t; \
    q[(rb) + 44 + 0 + 64] = m - t; \
    m = q[(rb) + 44 + 1]; \
    n = q[(rb) + 44 + 1 + 64]; \
    t = REDS2(n * alpha_tab[88 + 1 * 2]); \
    q[(rb) + 44 + 1] = m + t; \
    q[(rb) + 44 + 1 + 64] = m - t; \
    m = q[(rb) + 44 + 2]; \
    n = q[(rb) + 44 + 2 + 64]; \
    t = REDS2(n * alpha_tab[88 + 2 * 2]); \
    q[(rb) + 44 + 2] = m + t; \
    q[(rb) + 44 + 2 + 64] = m - t; \
    m = q[(rb) + 44 + 3]; \
    n = q[(rb) + 44 + 3 + 64]; \
    t = REDS2(n * alpha_tab[88 + 3 * 2]); \
    q[(rb) + 44 + 3] = m + t; \
    q[(rb) + 44 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 48 + 0]; \
    n = q[(rb) + 48 + 0 + 64]; \
    t = REDS2(n * alpha_tab[96 + 0 * 2]); \
    q[(rb) + 48 + 0] = m + t; \
    q[(rb) + 48 + 0 + 64] = m - t; \
    m = q[(rb) + 48 + 1]; \
    n = q[(rb) + 48 + 1 + 64]; \
    t = REDS2(n * alpha_tab[96 + 1 * 2]); \
    q[(rb) + 48 + 1] = m + t; \
    q[(rb) + 48 + 1 + 64] = m - t; \
    m = q[(rb) + 48 + 2]; \
    n = q[(rb) + 48 + 2 + 64]; \
    t = REDS2(n * alpha_tab[96 + 2 * 2]); \
    q[(rb) + 48 + 2] = m + t; \
    q[(rb) + 48 + 2 + 64] = m - t; \
    m = q[(rb) + 48 + 3]; \
    n = q[(rb) + 48 + 3 + 64]; \
    t = REDS2(n * alpha_tab[96 + 3 * 2]); \
    q[(rb) + 48 + 3] = m + t; \
    q[(rb) + 48 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 52 + 0]; \
    n = q[(rb) + 52 + 0 + 64]; \
    t = REDS2(n * alpha_tab[104 + 0 * 2]); \
    q[(rb) + 52 + 0] = m + t; \
    q[(rb) + 52 + 0 + 64] = m - t; \
    m = q[(rb) + 52 + 1]; \
    n = q[(rb) + 52 + 1 + 64]; \
    t = REDS2(n * alpha_tab[104 + 1 * 2]); \
    q[(rb) + 52 + 1] = m + t; \
    q[(rb) + 52 + 1 + 64] = m - t; \
    m = q[(rb) + 52 + 2]; \
    n = q[(rb) + 52 + 2 + 64]; \
    t = REDS2(n * alpha_tab[104 + 2 * 2]); \
    q[(rb) + 52 + 2] = m + t; \
    q[(rb) + 52 + 2 + 64] = m - t; \
    m = q[(rb) + 52 + 3]; \
    n = q[(rb) + 52 + 3 + 64]; \
    t = REDS2(n * alpha_tab[104 + 3 * 2]); \
    q[(rb) + 52 + 3] = m + t; \
    q[(rb) + 52 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 56 + 0]; \
    n = q[(rb) + 56 + 0 + 64]; \
    t = REDS2(n * alpha_tab[112 + 0 * 2]); \
    q[(rb) + 56 + 0] = m + t; \
    q[(rb) + 56 + 0 + 64] = m - t; \
    m = q[(rb) + 56 + 1]; \
    n = q[(rb) + 56 + 1 + 64]; \
    t = REDS2(n * alpha_tab[112 + 1 * 2]); \
    q[(rb) + 56 + 1] = m + t; \
    q[(rb) + 56 + 1 + 64] = m - t; \
    m = q[(rb) + 56 + 2]; \
    n = q[(rb) + 56 + 2 + 64]; \
    t = REDS2(n * alpha_tab[112 + 2 * 2]); \
    q[(rb) + 56 + 2] = m + t; \
    q[(rb) + 56 + 2 + 64] = m - t; \
    m = q[(rb) + 56 + 3]; \
    n = q[(rb) + 56 + 3 + 64]; \
    t = REDS2(n * alpha_tab[112 + 3 * 2]); \
    q[(rb) + 56 + 3] = m + t; \
    q[(rb) + 56 + 3 + 64] = m - t; \
    \
    m = q[(rb) + 60 + 0]; \
    n = q[(rb) + 60 + 0 + 64]; \
    t = REDS2(n * alpha_tab[120 + 0 * 2]); \
    q[(rb) + 60 + 0] = m + t; \
    q[(rb) + 60 + 0 + 64] = m - t; \
    m = q[(rb) + 60 + 1]; \
    n = q[(rb) + 60 + 1 + 64]; \
    t = REDS2(n * alpha_tab[120 + 1 * 2]); \
    q[(rb) + 60 + 1] = m + t; \
    q[(rb) + 60 + 1 + 64] = m - t; \
    m = q[(rb) + 60 + 2]; \
    n = q[(rb) + 60 + 2 + 64]; \
    t = REDS2(n * alpha_tab[120 + 2 * 2]); \
    q[(rb) + 60 + 2] = m + t; \
    q[(rb) + 60 + 2 + 64] = m - t; \
    m = q[(rb) + 60 + 3]; \
    n = q[(rb) + 60 + 3 + 64]; \
    t = REDS2(n * alpha_tab[120 + 3 * 2]); \
    q[(rb) + 60 + 3] = m + t; \
    q[(rb) + 60 + 3 + 64] = m - t; \
  } while (0)
 #define FFT_LOOP_128_1(rb)  do { \
    s32 m = q[(rb)]; \
    s32 n = q[(rb) + 128]; \
    q[(rb)] = m + n; \
    q[(rb) + 128] = m - n; \
    s32 t; \
    m = q[(rb) + 0 + 1]; \
    n = q[(rb) + 0 + 1 + 128]; \
    t = REDS2(n * alpha_tab[0 + 1 * 1]); \
    q[(rb) + 0 + 1] = m + t; \
    q[(rb) + 0 + 1 + 128] = m - t; \
    m = q[(rb) + 0 + 2]; \
    n = q[(rb) + 0 + 2 + 128]; \
    t = REDS2(n * alpha_tab[0 + 2 * 1]); \
    q[(rb) + 0 + 2] = m + t; \
    q[(rb) + 0 + 2 + 128] = m - t; \
    m = q[(rb) + 0 + 3]; \
    n = q[(rb) + 0 + 3 + 128]; \
    t = REDS2(n * alpha_tab[0 + 3 * 1]); \
    q[(rb) + 0 + 3] = m + t; \
    q[(rb) + 0 + 3 + 128] = m - t; \
    m = q[(rb) + 4 + 0]; \
    n = q[(rb) + 4 + 0 + 128]; \
    t = REDS2(n * alpha_tab[4 + 0 * 1]); \
    q[(rb) + 4 + 0] = m + t; \
    q[(rb) + 4 + 0 + 128] = m - t; \
    m = q[(rb) + 4 + 1]; \
    n = q[(rb) + 4 + 1 + 128]; \
    t = REDS2(n * alpha_tab[4 + 1 * 1]); \
    q[(rb) + 4 + 1] = m + t; \
    q[(rb) + 4 + 1 + 128] = m - t; \
    m = q[(rb) + 4 + 2]; \
    n = q[(rb) + 4 + 2 + 128]; \
    t = REDS2(n * alpha_tab[4 + 2 * 1]); \
    q[(rb) + 4 + 2] = m + t; \
    q[(rb) + 4 + 2 + 128] = m - t; \
    m = q[(rb) + 4 + 3]; \
    n = q[(rb) + 4 + 3 + 128]; \
    t = REDS2(n * alpha_tab[4 + 3 * 1]); \
    q[(rb) + 4 + 3] = m + t; \
    q[(rb) + 4 + 3 + 128] = m - t; \
    m = q[(rb) + 8 + 0]; \
    n = q[(rb) + 8 + 0 + 128]; \
    t = REDS2(n * alpha_tab[8 + 0 * 1]); \
    q[(rb) + 8 + 0] = m + t; \
    q[(rb) + 8 + 0 + 128] = m - t; \
    m = q[(rb) + 8 + 1]; \
    n = q[(rb) + 8 + 1 + 128]; \
    t = REDS2(n * alpha_tab[8 + 1 * 1]); \
    q[(rb) + 8 + 1] = m + t; \
    q[(rb) + 8 + 1 + 128] = m - t; \
    m = q[(rb) + 8 + 2]; \
    n = q[(rb) + 8 + 2 + 128]; \
    t = REDS2(n * alpha_tab[8 + 2 * 1]); \
    q[(rb) + 8 + 2] = m + t; \
    q[(rb) + 8 + 2 + 128] = m - t; \
    m = q[(rb) + 8 + 3]; \
    n = q[(rb) + 8 + 3 + 128]; \
    t = REDS2(n * alpha_tab[8 + 3 * 1]); \
    q[(rb) + 8 + 3] = m + t; \
    q[(rb) + 8 + 3 + 128] = m - t; \
    m = q[(rb) + 12 + 0]; \
    n = q[(rb) + 12 + 0 + 128]; \
    t = REDS2(n * alpha_tab[12 + 0 * 1]); \
    q[(rb) + 12 + 0] = m + t; \
    q[(rb) + 12 + 0 + 128] = m - t; \
    m = q[(rb) + 12 + 1]; \
    n = q[(rb) + 12 + 1 + 128]; \
    t = REDS2(n * alpha_tab[12 + 1 * 1]); \
    q[(rb) + 12 + 1] = m + t; \
    q[(rb) + 12 + 1 + 128] = m - t; \
    m = q[(rb) + 12 + 2]; \
    n = q[(rb) + 12 + 2 + 128]; \
    t = REDS2(n * alpha_tab[12 + 2 * 1]); \
    q[(rb) + 12 + 2] = m + t; \
    q[(rb) + 12 + 2 + 128] = m - t; \
    m = q[(rb) + 12 + 3]; \
    n = q[(rb) + 12 + 3 + 128]; \
    t = REDS2(n * alpha_tab[12 + 3 * 1]); \
    q[(rb) + 12 + 3] = m + t; \
    q[(rb) + 12 + 3 + 128] = m - t; \
    m = q[(rb) + 16 + 0]; \
    n = q[(rb) + 16 + 0 + 128]; \
    t = REDS2(n * alpha_tab[16 + 0 * 1]); \
    q[(rb) + 16 + 0] = m + t; \
    q[(rb) + 16 + 0 + 128] = m - t; \
    m = q[(rb) + 16 + 1]; \
    n = q[(rb) + 16 + 1 + 128]; \
    t = REDS2(n * alpha_tab[16 + 1 * 1]); \
    q[(rb) + 16 + 1] = m + t; \
    q[(rb) + 16 + 1 + 128] = m - t; \
    m = q[(rb) + 16 + 2]; \
    n = q[(rb) + 16 + 2 + 128]; \
    t = REDS2(n * alpha_tab[16 + 2 * 1]); \
    q[(rb) + 16 + 2] = m + t; \
    q[(rb) + 16 + 2 + 128] = m - t; \
    m = q[(rb) + 16 + 3]; \
    n = q[(rb) + 16 + 3 + 128]; \
    t = REDS2(n * alpha_tab[16 + 3 * 1]); \
    q[(rb) + 16 + 3] = m + t; \
    q[(rb) + 16 + 3 + 128] = m - t; \
    m = q[(rb) + 20 + 0]; \
    n = q[(rb) + 20 + 0 + 128]; \
    t = REDS2(n * alpha_tab[20 + 0 * 1]); \
    q[(rb) + 20 + 0] = m + t; \
    q[(rb) + 20 + 0 + 128] = m - t; \
    m = q[(rb) + 20 + 1]; \
    n = q[(rb) + 20 + 1 + 128]; \
    t = REDS2(n * alpha_tab[20 + 1 * 1]); \
    q[(rb) + 20 + 1] = m + t; \
    q[(rb) + 20 + 1 + 128] = m - t; \
    m = q[(rb) + 20 + 2]; \
    n = q[(rb) + 20 + 2 + 128]; \
    t = REDS2(n * alpha_tab[20 + 2 * 1]); \
    q[(rb) + 20 + 2] = m + t; \
    q[(rb) + 20 + 2 + 128] = m - t; \
    m = q[(rb) + 20 + 3]; \
    n = q[(rb) + 20 + 3 + 128]; \
    t = REDS2(n * alpha_tab[20 + 3 * 1]); \
    q[(rb) + 20 + 3] = m + t; \
    q[(rb) + 20 + 3 + 128] = m - t; \
    m = q[(rb) + 24 + 0]; \
    n = q[(rb) + 24 + 0 + 128]; \
    t = REDS2(n * alpha_tab[24 + 0 * 1]); \
    q[(rb) + 24 + 0] = m + t; \
    q[(rb) + 24 + 0 + 128] = m - t; \
    m = q[(rb) + 24 + 1]; \
    n = q[(rb) + 24 + 1 + 128]; \
    t = REDS2(n * alpha_tab[24 + 1 * 1]); \
    q[(rb) + 24 + 1] = m + t; \
    q[(rb) + 24 + 1 + 128] = m - t; \
    m = q[(rb) + 24 + 2]; \
    n = q[(rb) + 24 + 2 + 128]; \
    t = REDS2(n * alpha_tab[24 + 2 * 1]); \
    q[(rb) + 24 + 2] = m + t; \
    q[(rb) + 24 + 2 + 128] = m - t; \
    m = q[(rb) + 24 + 3]; \
    n = q[(rb) + 24 + 3 + 128]; \
    t = REDS2(n * alpha_tab[24 + 3 * 1]); \
    q[(rb) + 24 + 3] = m + t; \
    q[(rb) + 24 + 3 + 128] = m - t; \
    m = q[(rb) + 28 + 0]; \
    n = q[(rb) + 28 + 0 + 128]; \
    t = REDS2(n * alpha_tab[28 + 0 * 1]); \
    q[(rb) + 28 + 0] = m + t; \
    q[(rb) + 28 + 0 + 128] = m - t; \
    m = q[(rb) + 28 + 1]; \
    n = q[(rb) + 28 + 1 + 128]; \
    t = REDS2(n * alpha_tab[28 + 1 * 1]); \
    q[(rb) + 28 + 1] = m + t; \
    q[(rb) + 28 + 1 + 128] = m - t; \
    m = q[(rb) + 28 + 2]; \
    n = q[(rb) + 28 + 2 + 128]; \
    t = REDS2(n * alpha_tab[28 + 2 * 1]); \
    q[(rb) + 28 + 2] = m + t; \
    q[(rb) + 28 + 2 + 128] = m - t; \
    m = q[(rb) + 28 + 3]; \
    n = q[(rb) + 28 + 3 + 128]; \
    t = REDS2(n * alpha_tab[28 + 3 * 1]); \
    q[(rb) + 28 + 3] = m + t; \
    q[(rb) + 28 + 3 + 128] = m - t; \
    m = q[(rb) + 32 + 0]; \
    n = q[(rb) + 32 + 0 + 128]; \
    t = REDS2(n * alpha_tab[32 + 0 * 1]); \
    q[(rb) + 32 + 0] = m + t; \
    q[(rb) + 32 + 0 + 128] = m - t; \
    m = q[(rb) + 32 + 1]; \
    n = q[(rb) + 32 + 1 + 128]; \
    t = REDS2(n * alpha_tab[32 + 1 * 1]); \
    q[(rb) + 32 + 1] = m + t; \
    q[(rb) + 32 + 1 + 128] = m - t; \
    m = q[(rb) + 32 + 2]; \
    n = q[(rb) + 32 + 2 + 128]; \
    t = REDS2(n * alpha_tab[32 + 2 * 1]); \
    q[(rb) + 32 + 2] = m + t; \
    q[(rb) + 32 + 2 + 128] = m - t; \
    m = q[(rb) + 32 + 3]; \
    n = q[(rb) + 32 + 3 + 128]; \
    t = REDS2(n * alpha_tab[32 + 3 * 1]); \
    q[(rb) + 32 + 3] = m + t; \
    q[(rb) + 32 + 3 + 128] = m - t; \
    m = q[(rb) + 36 + 0]; \
    n = q[(rb) + 36 + 0 + 128]; \
    t = REDS2(n * alpha_tab[36 + 0 * 1]); \
    q[(rb) + 36 + 0] = m + t; \
    q[(rb) + 36 + 0 + 128] = m - t; \
    m = q[(rb) + 36 + 1]; \
    n = q[(rb) + 36 + 1 + 128]; \
    t = REDS2(n * alpha_tab[36 + 1 * 1]); \
    q[(rb) + 36 + 1] = m + t; \
    q[(rb) + 36 + 1 + 128] = m - t; \
    m = q[(rb) + 36 + 2]; \
    n = q[(rb) + 36 + 2 + 128]; \
    t = REDS2(n * alpha_tab[36 + 2 * 1]); \
    q[(rb) + 36 + 2] = m + t; \
    q[(rb) + 36 + 2 + 128] = m - t; \
    m = q[(rb) + 36 + 3]; \
    n = q[(rb) + 36 + 3 + 128]; \
    t = REDS2(n * alpha_tab[36 + 3 * 1]); \
    q[(rb) + 36 + 3] = m + t; \
    q[(rb) + 36 + 3 + 128] = m - t; \
    m = q[(rb) + 40 + 0]; \
    n = q[(rb) + 40 + 0 + 128]; \
    t = REDS2(n * alpha_tab[40 + 0 * 1]); \
    q[(rb) + 40 + 0] = m + t; \
    q[(rb) + 40 + 0 + 128] = m - t; \
    m = q[(rb) + 40 + 1]; \
    n = q[(rb) + 40 + 1 + 128]; \
    t = REDS2(n * alpha_tab[40 + 1 * 1]); \
    q[(rb) + 40 + 1] = m + t; \
    q[(rb) + 40 + 1 + 128] = m - t; \
    m = q[(rb) + 40 + 2]; \
    n = q[(rb) + 40 + 2 + 128]; \
    t = REDS2(n * alpha_tab[40 + 2 * 1]); \
    q[(rb) + 40 + 2] = m + t; \
    q[(rb) + 40 + 2 + 128] = m - t; \
    m = q[(rb) + 40 + 3]; \
    n = q[(rb) + 40 + 3 + 128]; \
    t = REDS2(n * alpha_tab[40 + 3 * 1]); \
    q[(rb) + 40 + 3] = m + t; \
    q[(rb) + 40 + 3 + 128] = m - t; \
    m = q[(rb) + 44 + 0]; \
    n = q[(rb) + 44 + 0 + 128]; \
    t = REDS2(n * alpha_tab[44 + 0 * 1]); \
    q[(rb) + 44 + 0] = m + t; \
    q[(rb) + 44 + 0 + 128] = m - t; \
    m = q[(rb) + 44 + 1]; \
    n = q[(rb) + 44 + 1 + 128]; \
    t = REDS2(n * alpha_tab[44 + 1 * 1]); \
    q[(rb) + 44 + 1] = m + t; \
    q[(rb) + 44 + 1 + 128] = m - t; \
    m = q[(rb) + 44 + 2]; \
    n = q[(rb) + 44 + 2 + 128]; \
    t = REDS2(n * alpha_tab[44 + 2 * 1]); \
    q[(rb) + 44 + 2] = m + t; \
    q[(rb) + 44 + 2 + 128] = m - t; \
    m = q[(rb) + 44 + 3]; \
    n = q[(rb) + 44 + 3 + 128]; \
    t = REDS2(n * alpha_tab[44 + 3 * 1]); \
    q[(rb) + 44 + 3] = m + t; \
    q[(rb) + 44 + 3 + 128] = m - t; \
    m = q[(rb) + 48 + 0]; \
    n = q[(rb) + 48 + 0 + 128]; \
    t = REDS2(n * alpha_tab[48 + 0 * 1]); \
    q[(rb) + 48 + 0] = m + t; \
    q[(rb) + 48 + 0 + 128] = m - t; \
    m = q[(rb) + 48 + 1]; \
    n = q[(rb) + 48 + 1 + 128]; \
    t = REDS2(n * alpha_tab[48 + 1 * 1]); \
    q[(rb) + 48 + 1] = m + t; \
    q[(rb) + 48 + 1 + 128] = m - t; \
    m = q[(rb) + 48 + 2]; \
    n = q[(rb) + 48 + 2 + 128]; \
    t = REDS2(n * alpha_tab[48 + 2 * 1]); \
    q[(rb) + 48 + 2] = m + t; \
    q[(rb) + 48 + 2 + 128] = m - t; \
    m = q[(rb) + 48 + 3]; \
    n = q[(rb) + 48 + 3 + 128]; \
    t = REDS2(n * alpha_tab[48 + 3 * 1]); \
    q[(rb) + 48 + 3] = m + t; \
    q[(rb) + 48 + 3 + 128] = m - t; \
    m = q[(rb) + 52 + 0]; \
    n = q[(rb) + 52 + 0 + 128]; \
    t = REDS2(n * alpha_tab[52 + 0 * 1]); \
    q[(rb) + 52 + 0] = m + t; \
    q[(rb) + 52 + 0 + 128] = m - t; \
    m = q[(rb) + 52 + 1]; \
    n = q[(rb) + 52 + 1 + 128]; \
    t = REDS2(n * alpha_tab[52 + 1 * 1]); \
    q[(rb) + 52 + 1] = m + t; \
    q[(rb) + 52 + 1 + 128] = m - t; \
    m = q[(rb) + 52 + 2]; \
    n = q[(rb) + 52 + 2 + 128]; \
    t = REDS2(n * alpha_tab[52 + 2 * 1]); \
    q[(rb) + 52 + 2] = m + t; \
    q[(rb) + 52 + 2 + 128] = m - t; \
    m = q[(rb) + 52 + 3]; \
    n = q[(rb) + 52 + 3 + 128]; \
    t = REDS2(n * alpha_tab[52 + 3 * 1]); \
    q[(rb) + 52 + 3] = m + t; \
    q[(rb) + 52 + 3 + 128] = m - t; \
    m = q[(rb) + 56 + 0]; \
    n = q[(rb) + 56 + 0 + 128]; \
    t = REDS2(n * alpha_tab[56 + 0 * 1]); \
    q[(rb) + 56 + 0] = m + t; \
    q[(rb) + 56 + 0 + 128] = m - t; \
    m = q[(rb) + 56 + 1]; \
    n = q[(rb) + 56 + 1 + 128]; \
    t = REDS2(n * alpha_tab[56 + 1 * 1]); \
    q[(rb) + 56 + 1] = m + t; \
    q[(rb) + 56 + 1 + 128] = m - t; \
    m = q[(rb) + 56 + 2]; \
    n = q[(rb) + 56 + 2 + 128]; \
    t = REDS2(n * alpha_tab[56 + 2 * 1]); \
    q[(rb) + 56 + 2] = m + t; \
    q[(rb) + 56 + 2 + 128] = m - t; \
    m = q[(rb) + 56 + 3]; \
    n = q[(rb) + 56 + 3 + 128]; \
    t = REDS2(n * alpha_tab[56 + 3 * 1]); \
    q[(rb) + 56 + 3] = m + t; \
    q[(rb) + 56 + 3 + 128] = m - t; \
    m = q[(rb) + 60 + 0]; \
    n = q[(rb) + 60 + 0 + 128]; \
    t = REDS2(n * alpha_tab[60 + 0 * 1]); \
    q[(rb) + 60 + 0] = m + t; \
    q[(rb) + 60 + 0 + 128] = m - t; \
    m = q[(rb) + 60 + 1]; \
    n = q[(rb) + 60 + 1 + 128]; \
    t = REDS2(n * alpha_tab[60 + 1 * 1]); \
    q[(rb) + 60 + 1] = m + t; \
    q[(rb) + 60 + 1 + 128] = m - t; \
    m = q[(rb) + 60 + 2]; \
    n = q[(rb) + 60 + 2 + 128]; \
    t = REDS2(n * alpha_tab[60 + 2 * 1]); \
    q[(rb) + 60 + 2] = m + t; \
    q[(rb) + 60 + 2 + 128] = m - t; \
    m = q[(rb) + 60 + 3]; \
    n = q[(rb) + 60 + 3 + 128]; \
    t = REDS2(n * alpha_tab[60 + 3 * 1]); \
    q[(rb) + 60 + 3] = m + t; \
    q[(rb) + 60 + 3 + 128] = m - t; \
    m = q[(rb) + 64 + 0]; \
    n = q[(rb) + 64 + 0 + 128]; \
    t = REDS2(n * alpha_tab[64 + 0 * 1]); \
    q[(rb) + 64 + 0] = m + t; \
    q[(rb) + 64 + 0 + 128] = m - t; \
    m = q[(rb) + 64 + 1]; \
    n = q[(rb) + 64 + 1 + 128]; \
    t = REDS2(n * alpha_tab[64 + 1 * 1]); \
    q[(rb) + 64 + 1] = m + t; \
    q[(rb) + 64 + 1 + 128] = m - t; \
    m = q[(rb) + 64 + 2]; \
    n = q[(rb) + 64 + 2 + 128]; \
    t = REDS2(n * alpha_tab[64 + 2 * 1]); \
    q[(rb) + 64 + 2] = m + t; \
    q[(rb) + 64 + 2 + 128] = m - t; \
    m = q[(rb) + 64 + 3]; \
    n = q[(rb) + 64 + 3 + 128]; \
    t = REDS2(n * alpha_tab[64 + 3 * 1]); \
    q[(rb) + 64 + 3] = m + t; \
    q[(rb) + 64 + 3 + 128] = m - t; \
    m = q[(rb) + 68 + 0]; \
    n = q[(rb) + 68 + 0 + 128]; \
    t = REDS2(n * alpha_tab[68 + 0 * 1]); \
    q[(rb) + 68 + 0] = m + t; \
    q[(rb) + 68 + 0 + 128] = m - t; \
    m = q[(rb) + 68 + 1]; \
    n = q[(rb) + 68 + 1 + 128]; \
    t = REDS2(n * alpha_tab[68 + 1 * 1]); \
    q[(rb) + 68 + 1] = m + t; \
    q[(rb) + 68 + 1 + 128] = m - t; \
    m = q[(rb) + 68 + 2]; \
    n = q[(rb) + 68 + 2 + 128]; \
    t = REDS2(n * alpha_tab[68 + 2 * 1]); \
    q[(rb) + 68 + 2] = m + t; \
    q[(rb) + 68 + 2 + 128] = m - t; \
    m = q[(rb) + 68 + 3]; \
    n = q[(rb) + 68 + 3 + 128]; \
    t = REDS2(n * alpha_tab[68 + 3 * 1]); \
    q[(rb) + 68 + 3] = m + t; \
    q[(rb) + 68 + 3 + 128] = m - t; \
    m = q[(rb) + 72 + 0]; \
    n = q[(rb) + 72 + 0 + 128]; \
    t = REDS2(n * alpha_tab[72 + 0 * 1]); \
    q[(rb) + 72 + 0] = m + t; \
    q[(rb) + 72 + 0 + 128] = m - t; \
    m = q[(rb) + 72 + 1]; \
    n = q[(rb) + 72 + 1 + 128]; \
    t = REDS2(n * alpha_tab[72 + 1 * 1]); \
    q[(rb) + 72 + 1] = m + t; \
    q[(rb) + 72 + 1 + 128] = m - t; \
    m = q[(rb) + 72 + 2]; \
    n = q[(rb) + 72 + 2 + 128]; \
    t = REDS2(n * alpha_tab[72 + 2 * 1]); \
    q[(rb) + 72 + 2] = m + t; \
    q[(rb) + 72 + 2 + 128] = m - t; \
    m = q[(rb) + 72 + 3]; \
    n = q[(rb) + 72 + 3 + 128]; \
    t = REDS2(n * alpha_tab[72 + 3 * 1]); \
    q[(rb) + 72 + 3] = m + t; \
    q[(rb) + 72 + 3 + 128] = m - t; \
    m = q[(rb) + 76 + 0]; \
    n = q[(rb) + 76 + 0 + 128]; \
    t = REDS2(n * alpha_tab[76 + 0 * 1]); \
    q[(rb) + 76 + 0] = m + t; \
    q[(rb) + 76 + 0 + 128] = m - t; \
    m = q[(rb) + 76 + 1]; \
    n = q[(rb) + 76 + 1 + 128]; \
    t = REDS2(n * alpha_tab[76 + 1 * 1]); \
    q[(rb) + 76 + 1] = m + t; \
    q[(rb) + 76 + 1 + 128] = m - t; \
    m = q[(rb) + 76 + 2]; \
    n = q[(rb) + 76 + 2 + 128]; \
    t = REDS2(n * alpha_tab[76 + 2 * 1]); \
    q[(rb) + 76 + 2] = m + t; \
    q[(rb) + 76 + 2 + 128] = m - t; \
    m = q[(rb) + 76 + 3]; \
    n = q[(rb) + 76 + 3 + 128]; \
    t = REDS2(n * alpha_tab[76 + 3 * 1]); \
    q[(rb) + 76 + 3] = m + t; \
    q[(rb) + 76 + 3 + 128] = m - t; \
    m = q[(rb) + 80 + 0]; \
    n = q[(rb) + 80 + 0 + 128]; \
    t = REDS2(n * alpha_tab[80 + 0 * 1]); \
    q[(rb) + 80 + 0] = m + t; \
    q[(rb) + 80 + 0 + 128] = m - t; \
    m = q[(rb) + 80 + 1]; \
    n = q[(rb) + 80 + 1 + 128]; \
    t = REDS2(n * alpha_tab[80 + 1 * 1]); \
    q[(rb) + 80 + 1] = m + t; \
    q[(rb) + 80 + 1 + 128] = m - t; \
    m = q[(rb) + 80 + 2]; \
    n = q[(rb) + 80 + 2 + 128]; \
    t = REDS2(n * alpha_tab[80 + 2 * 1]); \
    q[(rb) + 80 + 2] = m + t; \
    q[(rb) + 80 + 2 + 128] = m - t; \
    m = q[(rb) + 80 + 3]; \
    n = q[(rb) + 80 + 3 + 128]; \
    t = REDS2(n * alpha_tab[80 + 3 * 1]); \
    q[(rb) + 80 + 3] = m + t; \
    q[(rb) + 80 + 3 + 128] = m - t; \
    m = q[(rb) + 84 + 0]; \
    n = q[(rb) + 84 + 0 + 128]; \
    t = REDS2(n * alpha_tab[84 + 0 * 1]); \
    q[(rb) + 84 + 0] = m + t; \
    q[(rb) + 84 + 0 + 128] = m - t; \
    m = q[(rb) + 84 + 1]; \
    n = q[(rb) + 84 + 1 + 128]; \
    t = REDS2(n * alpha_tab[84 + 1 * 1]); \
    q[(rb) + 84 + 1] = m + t; \
    q[(rb) + 84 + 1 + 128] = m - t; \
    m = q[(rb) + 84 + 2]; \
    n = q[(rb) + 84 + 2 + 128]; \
    t = REDS2(n * alpha_tab[84 + 2 * 1]); \
    q[(rb) + 84 + 2] = m + t; \
    q[(rb) + 84 + 2 + 128] = m - t; \
    m = q[(rb) + 84 + 3]; \
    n = q[(rb) + 84 + 3 + 128]; \
    t = REDS2(n * alpha_tab[84 + 3 * 1]); \
    q[(rb) + 84 + 3] = m + t; \
    q[(rb) + 84 + 3 + 128] = m - t; \
    m = q[(rb) + 88 + 0]; \
    n = q[(rb) + 88 + 0 + 128]; \
    t = REDS2(n * alpha_tab[88 + 0 * 1]); \
    q[(rb) + 88 + 0] = m + t; \
    q[(rb) + 88 + 0 + 128] = m - t; \
    m = q[(rb) + 88 + 1]; \
    n = q[(rb) + 88 + 1 + 128]; \
    t = REDS2(n * alpha_tab[88 + 1 * 1]); \
    q[(rb) + 88 + 1] = m + t; \
    q[(rb) + 88 + 1 + 128] = m - t; \
    m = q[(rb) + 88 + 2]; \
    n = q[(rb) + 88 + 2 + 128]; \
    t = REDS2(n * alpha_tab[88 + 2 * 1]); \
    q[(rb) + 88 + 2] = m + t; \
    q[(rb) + 88 + 2 + 128] = m - t; \
    m = q[(rb) + 88 + 3]; \
    n = q[(rb) + 88 + 3 + 128]; \
    t = REDS2(n * alpha_tab[88 + 3 * 1]); \
    q[(rb) + 88 + 3] = m + t; \
    q[(rb) + 88 + 3 + 128] = m - t; \
    m = q[(rb) + 92 + 0]; \
    n = q[(rb) + 92 + 0 + 128]; \
    t = REDS2(n * alpha_tab[92 + 0 * 1]); \
    q[(rb) + 92 + 0] = m + t; \
    q[(rb) + 92 + 0 + 128] = m - t; \
    m = q[(rb) + 92 + 1]; \
    n = q[(rb) + 92 + 1 + 128]; \
    t = REDS2(n * alpha_tab[92 + 1 * 1]); \
    q[(rb) + 92 + 1] = m + t; \
    q[(rb) + 92 + 1 + 128] = m - t; \
    m = q[(rb) + 92 + 2]; \
    n = q[(rb) + 92 + 2 + 128]; \
    t = REDS2(n * alpha_tab[92 + 2 * 1]); \
    q[(rb) + 92 + 2] = m + t; \
    q[(rb) + 92 + 2 + 128] = m - t; \
    m = q[(rb) + 92 + 3]; \
    n = q[(rb) + 92 + 3 + 128]; \
    t = REDS2(n * alpha_tab[92 + 3 * 1]); \
    q[(rb) + 92 + 3] = m + t; \
    q[(rb) + 92 + 3 + 128] = m - t; \
    m = q[(rb) + 96 + 0]; \
    n = q[(rb) + 96 + 0 + 128]; \
    t = REDS2(n * alpha_tab[96 + 0 * 1]); \
    q[(rb) + 96 + 0] = m + t; \
    q[(rb) + 96 + 0 + 128] = m - t; \
    m = q[(rb) + 96 + 1]; \
    n = q[(rb) + 96 + 1 + 128]; \
    t = REDS2(n * alpha_tab[96 + 1 * 1]); \
    q[(rb) + 96 + 1] = m + t; \
    q[(rb) + 96 + 1 + 128] = m - t; \
    m = q[(rb) + 96 + 2]; \
    n = q[(rb) + 96 + 2 + 128]; \
    t = REDS2(n * alpha_tab[96 + 2 * 1]); \
    q[(rb) + 96 + 2] = m + t; \
    q[(rb) + 96 + 2 + 128] = m - t; \
    m = q[(rb) + 96 + 3]; \
    n = q[(rb) + 96 + 3 + 128]; \
    t = REDS2(n * alpha_tab[96 + 3 * 1]); \
    q[(rb) + 96 + 3] = m + t; \
    q[(rb) + 96 + 3 + 128] = m - t; \
    m = q[(rb) + 100 + 0]; \
    n = q[(rb) + 100 + 0 + 128]; \
    t = REDS2(n * alpha_tab[100 + 0 * 1]); \
    q[(rb) + 100 + 0] = m + t; \
    q[(rb) + 100 + 0 + 128] = m - t; \
    m = q[(rb) + 100 + 1]; \
    n = q[(rb) + 100 + 1 + 128]; \
    t = REDS2(n * alpha_tab[100 + 1 * 1]); \
    q[(rb) + 100 + 1] = m + t; \
    q[(rb) + 100 + 1 + 128] = m - t; \
    m = q[(rb) + 100 + 2]; \
    n = q[(rb) + 100 + 2 + 128]; \
    t = REDS2(n * alpha_tab[100 + 2 * 1]); \
    q[(rb) + 100 + 2] = m + t; \
    q[(rb) + 100 + 2 + 128] = m - t; \
    m = q[(rb) + 100 + 3]; \
    n = q[(rb) + 100 + 3 + 128]; \
    t = REDS2(n * alpha_tab[100 + 3 * 1]); \
    q[(rb) + 100 + 3] = m + t; \
    q[(rb) + 100 + 3 + 128] = m - t; \
    m = q[(rb) + 104 + 0]; \
    n = q[(rb) + 104 + 0 + 128]; \
    t = REDS2(n * alpha_tab[104 + 0 * 1]); \
    q[(rb) + 104 + 0] = m + t; \
    q[(rb) + 104 + 0 + 128] = m - t; \
    m = q[(rb) + 104 + 1]; \
    n = q[(rb) + 104 + 1 + 128]; \
    t = REDS2(n * alpha_tab[104 + 1 * 1]); \
    q[(rb) + 104 + 1] = m + t; \
    q[(rb) + 104 + 1 + 128] = m - t; \
    m = q[(rb) + 104 + 2]; \
    n = q[(rb) + 104 + 2 + 128]; \
    t = REDS2(n * alpha_tab[104 + 2 * 1]); \
    q[(rb) + 104 + 2] = m + t; \
    q[(rb) + 104 + 2 + 128] = m - t; \
    m = q[(rb) + 104 + 3]; \
    n = q[(rb) + 104 + 3 + 128]; \
    t = REDS2(n * alpha_tab[104 + 3 * 1]); \
    q[(rb) + 104 + 3] = m + t; \
    q[(rb) + 104 + 3 + 128] = m - t; \
    m = q[(rb) + 108 + 0]; \
    n = q[(rb) + 108 + 0 + 128]; \
    t = REDS2(n * alpha_tab[108 + 0 * 1]); \
    q[(rb) + 108 + 0] = m + t; \
    q[(rb) + 108 + 0 + 128] = m - t; \
    m = q[(rb) + 108 + 1]; \
    n = q[(rb) + 108 + 1 + 128]; \
    t = REDS2(n * alpha_tab[108 + 1 * 1]); \
    q[(rb) + 108 + 1] = m + t; \
    q[(rb) + 108 + 1 + 128] = m - t; \
    m = q[(rb) + 108 + 2]; \
    n = q[(rb) + 108 + 2 + 128]; \
    t = REDS2(n * alpha_tab[108 + 2 * 1]); \
    q[(rb) + 108 + 2] = m + t; \
    q[(rb) + 108 + 2 + 128] = m - t; \
    m = q[(rb) + 108 + 3]; \
    n = q[(rb) + 108 + 3 + 128]; \
    t = REDS2(n * alpha_tab[108 + 3 * 1]); \
    q[(rb) + 108 + 3] = m + t; \
    q[(rb) + 108 + 3 + 128] = m - t; \
    m = q[(rb) + 112 + 0]; \
    n = q[(rb) + 112 + 0 + 128]; \
    t = REDS2(n * alpha_tab[112 + 0 * 1]); \
    q[(rb) + 112 + 0] = m + t; \
    q[(rb) + 112 + 0 + 128] = m - t; \
    m = q[(rb) + 112 + 1]; \
    n = q[(rb) + 112 + 1 + 128]; \
    t = REDS2(n * alpha_tab[112 + 1 * 1]); \
    q[(rb) + 112 + 1] = m + t; \
    q[(rb) + 112 + 1 + 128] = m - t; \
    m = q[(rb) + 112 + 2]; \
    n = q[(rb) + 112 + 2 + 128]; \
    t = REDS2(n * alpha_tab[112 + 2 * 1]); \
    q[(rb) + 112 + 2] = m + t; \
    q[(rb) + 112 + 2 + 128] = m - t; \
    m = q[(rb) + 112 + 3]; \
    n = q[(rb) + 112 + 3 + 128]; \
    t = REDS2(n * alpha_tab[112 + 3 * 1]); \
    q[(rb) + 112 + 3] = m + t; \
    q[(rb) + 112 + 3 + 128] = m - t; \
    m = q[(rb) + 116 + 0]; \
    n = q[(rb) + 116 + 0 + 128]; \
    t = REDS2(n * alpha_tab[116 + 0 * 1]); \
    q[(rb) + 116 + 0] = m + t; \
    q[(rb) + 116 + 0 + 128] = m - t; \
    m = q[(rb) + 116 + 1]; \
    n = q[(rb) + 116 + 1 + 128]; \
    t = REDS2(n * alpha_tab[116 + 1 * 1]); \
    q[(rb) + 116 + 1] = m + t; \
    q[(rb) + 116 + 1 + 128] = m - t; \
    m = q[(rb) + 116 + 2]; \
    n = q[(rb) + 116 + 2 + 128]; \
    t = REDS2(n * alpha_tab[116 + 2 * 1]); \
    q[(rb) + 116 + 2] = m + t; \
    q[(rb) + 116 + 2 + 128] = m - t; \
    m = q[(rb) + 116 + 3]; \
    n = q[(rb) + 116 + 3 + 128]; \
    t = REDS2(n * alpha_tab[116 + 3 * 1]); \
    q[(rb) + 116 + 3] = m + t; \
    q[(rb) + 116 + 3 + 128] = m - t; \
    m = q[(rb) + 120 + 0]; \
    n = q[(rb) + 120 + 0 + 128]; \
    t = REDS2(n * alpha_tab[120 + 0 * 1]); \
    q[(rb) + 120 + 0] = m + t; \
    q[(rb) + 120 + 0 + 128] = m - t; \
    m = q[(rb) + 120 + 1]; \
    n = q[(rb) + 120 + 1 + 128]; \
    t = REDS2(n * alpha_tab[120 + 1 * 1]); \
    q[(rb) + 120 + 1] = m + t; \
    q[(rb) + 120 + 1 + 128] = m - t; \
    m = q[(rb) + 120 + 2]; \
    n = q[(rb) + 120 + 2 + 128]; \
    t = REDS2(n * alpha_tab[120 + 2 * 1]); \
    q[(rb) + 120 + 2] = m + t; \
    q[(rb) + 120 + 2 + 128] = m - t; \
    m = q[(rb) + 120 + 3]; \
    n = q[(rb) + 120 + 3 + 128]; \
    t = REDS2(n * alpha_tab[120 + 3 * 1]); \
    q[(rb) + 120 + 3] = m + t; \
    q[(rb) + 120 + 3 + 128] = m - t; \
    m = q[(rb) + 124 + 0]; \
    n = q[(rb) + 124 + 0 + 128]; \
    t = REDS2(n * alpha_tab[124 + 0 * 1]); \
    q[(rb) + 124 + 0] = m + t; \
    q[(rb) + 124 + 0 + 128] = m - t; \
    m = q[(rb) + 124 + 1]; \
    n = q[(rb) + 124 + 1 + 128]; \
    t = REDS2(n * alpha_tab[124 + 1 * 1]); \
    q[(rb) + 124 + 1] = m + t; \
    q[(rb) + 124 + 1 + 128] = m - t; \
    m = q[(rb) + 124 + 2]; \
    n = q[(rb) + 124 + 2 + 128]; \
    t = REDS2(n * alpha_tab[124 + 2 * 1]); \
    q[(rb) + 124 + 2] = m + t; \
    q[(rb) + 124 + 2 + 128] = m - t; \
    m = q[(rb) + 124 + 3]; \
    n = q[(rb) + 124 + 3 + 128]; \
    t = REDS2(n * alpha_tab[124 + 3 * 1]); \
    q[(rb) + 124 + 3] = m + t; \
    q[(rb) + 124 + 3 + 128] = m - t; \
  } while (0)
 /*
 * Output ranges:
 *   d0:   min=    0   max= 1020
 *   d1:   min=  -67   max= 4587
 *   d2:   min=-4335   max= 4335
 *   d3:   min=-4147   max=  507
 *   d4:   min= -510   max=  510
 *   d5:   min= -252   max= 4402
 *   d6:   min=-4335   max= 4335
 *   d7:   min=-4332   max=  322
 */
#define FFT8(xb, xs, d)   \
    x0 = x[tid + WORKSIZE * (xb)]; \
    x1 = x[tid + WORKSIZE * ((xb) + (xs))]; \
    x2 = x[tid + WORKSIZE * ((xb) + 2 * (xs))]; \
    x3 = x[tid + WORKSIZE * ((xb) + 3 * (xs))]; \
    a0 = x0 + x2; \
    a1 = x0 + (x2 << 4); \
    a2 = x0 - x2; \
    a3 = x0 - (x2 << 4); \
    b0 = x1 + x3; \
    b1 = REDS1((x1 << 2) + (x3 << 6)); \
    b2 = (x1 << 4) - (x3 << 4); \
    b3 = REDS1((x1 << 6) + (x3 << 2)); \
    d ## 0 = a0 + b0; \
    d ## 1 = a1 + b1; \
    d ## 2 = a2 + b2; \
    d ## 3 = a3 + b3; \
    d ## 4 = a0 - b0; \
    d ## 5 = a1 - b1; \
    d ## 6 = a2 - b2; \
    d ## 7 = a3 - b3;

#define FFT8_80(xb, xs, d)   \
    x0 = x[(xb)]; \
    x1 = x[((xb) + (xs))]; \
    x2 = x[((xb) + 2 * (xs))]; \
    x3 = x[((xb) + 3 * (xs))]; \
    a0 = x0 + x2; \
    a1 = x0 + (x2 << 4); \
    a2 = x0 - x2; \
    a3 = x0 - (x2 << 4); \
    b0 = x1 + x3; \
    b1 = REDS1((x1 << 2) + (x3 << 6)); \
    b2 = (x1 << 4) - (x3 << 4); \
    b3 = REDS1((x1 << 6) + (x3 << 2)); \
    d ## 0 = a0 + b0; \
    d ## 1 = a1 + b1; \
    d ## 2 = a2 + b2; \
    d ## 3 = a3 + b3; \
    d ## 4 = a0 - b0; \
    d ## 5 = a1 - b1; \
    d ## 6 = a2 - b2; \
    d ## 7 = a3 - b3;
	
 /*
 * When k=16, we have alpha=2. Multiplication by alpha^i is then reduced
 * to some shifting.
 *
 * Output: within -591471..591723
 */
#define FFT16(xb, xs, rb)   \
    FFT8(xb, (xs) << 1, d1_); \
    FFT8((xb) + (xs), (xs) << 1, d2_); \
    q[(rb) +  0] = d1_0 + d2_0; \
    q[(rb) +  1] = d1_1 + (d2_1 << 1); \
    q[(rb) +  2] = d1_2 + (d2_2 << 2); \
    q[(rb) +  3] = d1_3 + (d2_3 << 3); \
    q[(rb) +  4] = d1_4 + (d2_4 << 4); \
    q[(rb) +  5] = d1_5 + (d2_5 << 5); \
    q[(rb) +  6] = d1_6 + (d2_6 << 6); \
    q[(rb) +  7] = d1_7 + (d2_7 << 7); \
    q[(rb) +  8] = d1_0 - d2_0; \
    q[(rb) +  9] = d1_1 - (d2_1 << 1); \
    q[(rb) + 10] = d1_2 - (d2_2 << 2); \
    q[(rb) + 11] = d1_3 - (d2_3 << 3); \
    q[(rb) + 12] = d1_4 - (d2_4 << 4); \
    q[(rb) + 13] = d1_5 - (d2_5 << 5); \
    q[(rb) + 14] = d1_6 - (d2_6 << 6); \
    q[(rb) + 15] = d1_7 - (d2_7 << 7);
	
#define FFT16_80(xb, xs, rb)   \
    FFT8_80(xb, (xs) << 1, d1_); \
    FFT8_80((xb) + (xs), (xs) << 1, d2_); \
    q[(rb) +  0] = d1_0 + d2_0; \
    q[(rb) +  1] = d1_1 + (d2_1 << 1); \
    q[(rb) +  2] = d1_2 + (d2_2 << 2); \
    q[(rb) +  3] = d1_3 + (d2_3 << 3); \
    q[(rb) +  4] = d1_4 + (d2_4 << 4); \
    q[(rb) +  5] = d1_5 + (d2_5 << 5); \
    q[(rb) +  6] = d1_6 + (d2_6 << 6); \
    q[(rb) +  7] = d1_7 + (d2_7 << 7); \
    q[(rb) +  8] = d1_0 - d2_0; \
    q[(rb) +  9] = d1_1 - (d2_1 << 1); \
    q[(rb) + 10] = d1_2 - (d2_2 << 2); \
    q[(rb) + 11] = d1_3 - (d2_3 << 3); \
    q[(rb) + 12] = d1_4 - (d2_4 << 4); \
    q[(rb) + 13] = d1_5 - (d2_5 << 5); \
    q[(rb) + 14] = d1_6 - (d2_6 << 6); \
    q[(rb) + 15] = d1_7 - (d2_7 << 7);
	
 /*
 * Output range: |q| <= 1183446
 */
#define FFT32(xb, xs, rb, id)   \
    FFT16(xb, (xs) << 1, rb); \
    FFT16((xb) + (xs), (xs) << 1, (rb) + 16); \
    FFT_LOOP_16_8(rb);

#define FFT32_80(xb, xs, rb, id)   \
    FFT16_80(xb, (xs) << 1, rb); \
    FFT16_80((xb) + (xs), (xs) << 1, (rb) + 16); \
    FFT_LOOP_16_8(rb);
	
 /*
 * Output range: |q| <= 2366892
 */
#define FFT64(xb, xs, rb)   \
  FFT32(xb, (xs) << 1, (rb), label_a); \
  FFT32((xb) + (xs), (xs) << 1, (rb) + 32, label_b); \
  FFT_LOOP_32_4(rb);
 
#define FFT64_80(xb, xs, rb)   \
  FFT32_80(xb, (xs) << 1, (rb), label_a); \
  FFT32_80((xb) + (xs), (xs) << 1, (rb) + 32, label_b); \
  FFT_LOOP_32_4(rb); 
  
 /*
 * Output range: |q| <= 9467568
 */
#define FFT256(xb, xs, rb, id)   \
    FFT64((xb) + ((xs) * 0), (xs) << 2, (rb + 0)); \
    FFT64((xb) + ((xs) * 2), (xs) << 2, (rb + 64)); \
    FFT_LOOP_64_2(rb); \
    FFT64((xb) + ((xs) * 1), (xs) << 2, (rb + 128)); \
    FFT64((xb) + ((xs) * 3), (xs) << 2, (rb + 192)); \
    FFT_LOOP_64_2((rb) + 128); \
    FFT_LOOP_128_1(rb);

#define FFT256_80(xb, xs, rb, id)   \
    FFT64_80((xb) + ((xs) * 0), (xs) << 2, (rb + 0)); \
    FFT64_80((xb) + ((xs) * 2), (xs) << 2, (rb + 64)); \
    FFT_LOOP_64_2(rb); \
    FFT64_80((xb) + ((xs) * 1), (xs) << 2, (rb + 128)); \
    FFT64_80((xb) + ((xs) * 3), (xs) << 2, (rb + 192)); \
    FFT_LOOP_64_2((rb) + 128); \
    FFT_LOOP_128_1(rb);
 /*
 * beta^(255*i) mod 257
 */
__constant static const unsigned short yoff_b_n[] = {
    1, 163,  98,  40,  95,  65,  58, 202,  30,   7, 113, 172,
   23, 151, 198, 149, 129, 210,  49,  20, 176, 161,  29, 101,
   15, 132, 185,  86, 140, 204,  99, 203, 193, 105, 153,  10,
   88, 209, 143, 179, 136,  66, 221,  43,  70, 102, 178, 230,
  225, 181, 205,   5,  44, 233, 200, 218,  68,  33, 239, 150,
   35,  51,  89, 115, 241, 219, 231, 131,  22, 245, 100, 109,
   34, 145, 248,  75, 146, 154, 173, 186, 249, 238, 244, 194,
   11, 251,  50, 183,  17, 201, 124, 166,  73,  77, 215,  93,
  253, 119, 122,  97, 134, 254,  25, 220, 137, 229,  62,  83,
  165, 167, 236, 175, 255, 188,  61, 177,  67, 127, 141, 110,
  197, 243,  31, 170, 211, 212, 118, 216, 256,  94, 159, 217,
  162, 192, 199,  55, 227, 250, 144,  85, 234, 106,  59, 108,
  128,  47, 208, 237,  81,  96, 228, 156, 242, 125,  72, 171,
  117,  53, 158,  54,  64, 152, 104, 247, 169,  48, 114,  78,
  121, 191,  36, 214, 187, 155,  79,  27,  32,  76,  52, 252,
  213,  24,  57,  39, 189, 224,  18, 107, 222, 206, 168, 142,
   16,  38,  26, 126, 235,  12, 157, 148, 223, 112,   9, 182,
  111, 103,  84,  71,   8,  19,  13,  63, 246,   6, 207,  74,
  240,  56, 133,  91, 184, 180,  42, 164,   4, 138, 135, 160,
  123,   3, 232,  37, 120,  28, 195, 174,  92,  90,  21,  82,
    2,  69, 196,  80, 190, 130, 116, 147,  60,  14, 226,  87,
   46,  45, 139,  41
};
 #define INNER(l, h, mm)   (((u32)((l) * (mm)) & 0xFFFFU) \
              + ((u32)((h) * (mm)) << 16))
 #define W_BIG(sb, o1, o2, mm) \
  (INNER(q[16 * (sb) + 2 * 0 + o1], q[16 * (sb) + 2 * 0 + o2], mm), \
   INNER(q[16 * (sb) + 2 * 1 + o1], q[16 * (sb) + 2 * 1 + o2], mm), \
   INNER(q[16 * (sb) + 2 * 2 + o1], q[16 * (sb) + 2 * 2 + o2], mm), \
   INNER(q[16 * (sb) + 2 * 3 + o1], q[16 * (sb) + 2 * 3 + o2], mm), \
   INNER(q[16 * (sb) + 2 * 4 + o1], q[16 * (sb) + 2 * 4 + o2], mm), \
   INNER(q[16 * (sb) + 2 * 5 + o1], q[16 * (sb) + 2 * 5 + o2], mm), \
   INNER(q[16 * (sb) + 2 * 6 + o1], q[16 * (sb) + 2 * 6 + o2], mm), \
   INNER(q[16 * (sb) + 2 * 7 + o1], q[16 * (sb) + 2 * 7 + o2], mm)
 
// precomp
#define WB_P_0_0   (0x531B1720U, 0xAC2CDE09U, 0x0B902D87U, 0x2369B1F4U, 0x2931AA01U, 0x02E4B082U, 0xC914C914U, 0xC1DAE1A6U
#define WB_P_0_1   (0xF18C2B5CU, 0x08AC306BU, 0x27BFC914U, 0xCEDC548DU, 0xC630C4BEU, 0xF18C4335U, 0xF0D3427CU, 0xBE3DA380U
#define WB_P_0_2   (0x143C02E4U, 0xA948C630U, 0xA4F2DE09U, 0xA71D2085U, 0xA439BD84U, 0x109FCD6AU, 0xEEA8EF61U, 0xA5AB1CE8U
#define WB_P_0_3   (0x0B90D4A4U, 0x3D6D039DU, 0x25944D53U, 0xBAA0E034U, 0x5BC71E5AU, 0xB1F4F2FEU, 0x12CADE09U, 0x548D41C3U
#define WB_P_0_4   (0x3CB4F80DU, 0x36ECEBC4U, 0xA66443EEU, 0x43351ABDU, 0xC7A20C49U, 0xEB0BB366U, 0xF5293F98U, 0x49B6DE09U
#define WB_P_0_5   (0x531B29EAU, 0x02E402E4U, 0xDB25C405U, 0x53D4E543U, 0x0AD71720U, 0xE1A61A04U, 0xB87534C1U, 0x3EDF43EEU
#define WB_P_0_6   (0x213E50F0U, 0x39173EDFU, 0xA9485B0EU, 0xEEA82EF9U, 0x14F55771U, 0xFAF15546U, 0x3D6DD9B3U, 0xAB73B92EU
#define WB_P_0_7   (0x582A48FDU, 0xEEA81892U, 0x4F7EAA01U, 0xAF10A88FU, 0x11581720U, 0x34C124DBU, 0xD1C0AB73U, 0x1E5AF0D3U
#define WB_P_1_0   (0xC34C07F3U, 0xC914143CU, 0x599CBC12U, 0xBCCBE543U, 0x385EF3B7U, 0x14F54C9AU, 0x0AD7C068U, 0xB64A21F7U
#define WB_P_1_1   (0xDEC2AF10U, 0xC6E9C121U, 0x56B8A4F2U, 0x1158D107U, 0xEB0BA88FU, 0x050FAABAU, 0xC293264DU, 0x548D46D2U
#define WB_P_1_2   (0xACE5E8E0U, 0x53D421F7U, 0xF470D279U, 0xDC974E0CU, 0xD6CF55FFU, 0xFD1C4F7EU, 0x36EC36ECU, 0x3E261E5AU
#define WB_P_1_3   (0xEBC4FD1CU, 0x56B839D0U, 0x5B0E21F7U, 0x58E3DF7BU, 0x5BC7427CU, 0xEF613296U, 0x1158109FU, 0x5A55E318U
#define WB_P_1_4   (0xA7D6B703U, 0x1158E76EU, 0xB08255FFU, 0x50F05771U, 0xEEA8E8E0U, 0xCB3FDB25U, 0x2E40548DU, 0xE1A60F2DU
#define WB_P_1_5   (0xACE5D616U, 0xFD1CFD1CU, 0x24DB3BFBU, 0xAC2C1ABDU, 0xF529E8E0U, 0x1E5AE5FCU, 0x478BCB3FU, 0xC121BC12U
#define WB_P_1_6   (0xF4702B5CU, 0xC293FC63U, 0xDA6CB2ADU, 0x45601FCCU, 0xA439E1A6U, 0x4E0C0D02U, 0xED3621F7U, 0xAB73BE3DU
#define WB_P_1_7   (0x0E74D4A4U, 0xF754CF95U, 0xD84136ECU, 0x3124AB73U, 0x39D03B42U, 0x0E74BCCBU, 0x0F2DBD84U, 0x41C35C80U
#define WB_P_2_0   (0xA4135BEDU, 0xE10E1EF2U, 0x6C4F93B1U, 0x6E2191DFU, 0xE2E01D20U, 0xD1952E6BU, 0x6A7D9583U, 0x131DECE3U
#define WB_P_2_1   (0x369CC964U, 0xFB73048DU, 0x9E9D6163U, 0x280CD7F4U, 0xD9C6263AU, 0x1062EF9EU, 0x2AC7D539U, 0xAD2D52D3U
#define WB_P_2_2   (0x0A03F5FDU, 0x197CE684U, 0xAA72558EU, 0xDE5321ADU, 0xF0870F79U, 0x607A9F86U, 0xAFE85018U, 0x2AC7D539U
#define WB_P_2_3   (0xE2E01D20U, 0x2AC7D539U, 0xC6A93957U, 0x624C9DB4U, 0x6C4F93B1U, 0x641E9BE2U, 0x452CBAD4U, 0x263AD9C6U
#define WB_P_2_4   (0xC964369CU, 0xC3053CFBU, 0x452CBAD4U, 0x95836A7DU, 0x4AA2B55EU, 0xAB5B54A5U, 0xAC4453BCU, 0x74808B80U
#define WB_P_2_5   (0xCB3634CAU, 0xFC5C03A4U, 0x4B8BB475U, 0x21ADDE53U, 0xE2E01D20U, 0xDF3C20C4U, 0xBD8F4271U, 0xAA72558EU
#define WB_P_2_6   (0xFC5C03A4U, 0x48D0B730U, 0x2AC7D539U, 0xD70B28F5U, 0x53BCAC44U, 0x3FB6C04AU, 0x14EFEB11U, 0xDB982468U
#define WB_P_2_7   (0x9A1065F0U, 0xB0D14F2FU, 0x8D5272AEU, 0xC4D73B29U, 0x91DF6E21U, 0x949A6B66U, 0x303DCFC3U, 0x5932A6CEU
#define WB_P_3_0   (0x1234EDCCU, 0xF5140AECU, 0xCDF1320FU, 0x3DE4C21CU, 0x48D0B730U, 0x1234EDCCU, 0x131DECE3U, 0x52D3AD2DU
#define WB_P_3_1   (0xE684197CU, 0x6D3892C8U, 0x72AE8D52U, 0x6FF3900DU, 0x73978C69U, 0xEB1114EFU, 0x15D8EA28U, 0x71C58E3BU
#define WB_P_3_2   (0x90F66F0AU, 0x15D8EA28U, 0x9BE2641EU, 0x65F09A10U, 0xEA2815D8U, 0xBD8F4271U, 0x3A40C5C0U, 0xD9C6263AU
#define WB_P_3_3   (0xB38C4C74U, 0xBAD4452CU, 0x70DC8F24U, 0xAB5B54A5U, 0x46FEB902U, 0x1A65E59BU, 0x0DA7F259U, 0xA32A5CD6U
#define WB_P_3_4   (0xD62229DEU, 0xB81947E7U, 0x6D3892C8U, 0x15D8EA28U, 0xE59B1A65U, 0x065FF9A1U, 0xB2A34D5DU, 0x6A7D9583U
#define WB_P_3_5   (0x975568ABU, 0xFC5C03A4U, 0x2E6BD195U, 0x966C6994U, 0xF2590DA7U, 0x263AD9C6U, 0x5A1BA5E5U, 0xB0D14F2FU
#define WB_P_3_6   (0x975568ABU, 0x6994966CU, 0xF1700E90U, 0xD3672C99U, 0xCC1F33E1U, 0xFC5C03A4U, 0x452CBAD4U, 0x4E46B1BAU
#define WB_P_3_7   (0xF1700E90U, 0xB2A34D5DU, 0xD0AC2F54U, 0x5760A8A0U, 0x8C697397U, 0x624C9DB4U, 0xE85617AAU, 0x95836A7DU
  
 #define WB_0_0   W_BIG( 4,    0,    1, 185)
#define WB_0_1   W_BIG( 6,    0,    1, 185)
#define WB_0_2   W_BIG( 0,    0,    1, 185)
#define WB_0_3   W_BIG( 2,    0,    1, 185)
#define WB_0_4   W_BIG( 7,    0,    1, 185)
#define WB_0_5   W_BIG( 5,    0,    1, 185)
#define WB_0_6   W_BIG( 3,    0,    1, 185)
#define WB_0_7   W_BIG( 1,    0,    1, 185)
#define WB_1_0   W_BIG(15,    0,    1, 185)
#define WB_1_1   W_BIG(11,    0,    1, 185)
#define WB_1_2   W_BIG(12,    0,    1, 185)
#define WB_1_3   W_BIG( 8,    0,    1, 185)
#define WB_1_4   W_BIG( 9,    0,    1, 185)
#define WB_1_5   W_BIG(13,    0,    1, 185)
#define WB_1_6   W_BIG(10,    0,    1, 185)
#define WB_1_7   W_BIG(14,    0,    1, 185)
#define WB_2_0   W_BIG(17, -256, -128, 233)
#define WB_2_1   W_BIG(18, -256, -128, 233)
#define WB_2_2   W_BIG(23, -256, -128, 233)
#define WB_2_3   W_BIG(20, -256, -128, 233)
#define WB_2_4   W_BIG(22, -256, -128, 233)
#define WB_2_5   W_BIG(21, -256, -128, 233)
#define WB_2_6   W_BIG(16, -256, -128, 233)
#define WB_2_7   W_BIG(19, -256, -128, 233)
#define WB_3_0   W_BIG(30, -383, -255, 233)
#define WB_3_1   W_BIG(24, -383, -255, 233)
#define WB_3_2   W_BIG(25, -383, -255, 233)
#define WB_3_3   W_BIG(31, -383, -255, 233)
#define WB_3_4   W_BIG(27, -383, -255, 233)
#define WB_3_5   W_BIG(29, -383, -255, 233)
#define WB_3_6   W_BIG(28, -383, -255, 233)
#define WB_3_7   W_BIG(26, -383, -255, 233)
 #define IF(x, y, z)    ((((y) ^ (z)) & (x)) ^ (z))
#define MAJ(x, y, z)   (((x) & (y)) | (((x) | (y)) & (z)))
 #define PP4_0_0   1
#define PP4_0_1   0
#define PP4_0_2   3
#define PP4_0_3   2
#define PP4_1_0   2
#define PP4_1_1   3
#define PP4_1_2   0
#define PP4_1_3   1
#define PP4_2_0   3
#define PP4_2_1   2
#define PP4_2_2   1
#define PP4_2_3   0
 #define PP8_0_0   1
#define PP8_0_1   0
#define PP8_0_2   3
#define PP8_0_3   2
#define PP8_0_4   5
#define PP8_0_5   4
#define PP8_0_6   7
#define PP8_0_7   6
 #define PP8_1_0   6
#define PP8_1_1   7
#define PP8_1_2   4
#define PP8_1_3   5
#define PP8_1_4   2
#define PP8_1_5   3
#define PP8_1_6   0
#define PP8_1_7   1
 #define PP8_2_0   2
#define PP8_2_1   3
#define PP8_2_2   0
#define PP8_2_3   1
#define PP8_2_4   6
#define PP8_2_5   7
#define PP8_2_6   4
#define PP8_2_7   5
 #define PP8_3_0   3
#define PP8_3_1   2
#define PP8_3_2   1
#define PP8_3_3   0
#define PP8_3_4   7
#define PP8_3_5   6
#define PP8_3_6   5
#define PP8_3_7   4
 #define PP8_4_0   5
#define PP8_4_1   4
#define PP8_4_2   7
#define PP8_4_3   6
#define PP8_4_4   1
#define PP8_4_5   0
#define PP8_4_6   3
#define PP8_4_7   2
 #define PP8_5_0   7
#define PP8_5_1   6
#define PP8_5_2   5
#define PP8_5_3   4
#define PP8_5_4   3
#define PP8_5_5   2
#define PP8_5_6   1
#define PP8_5_7   0
 #define PP8_6_0   4
#define PP8_6_1   5
#define PP8_6_2   6
#define PP8_6_3   7
#define PP8_6_4   0
#define PP8_6_5   1
#define PP8_6_6   2
#define PP8_6_7   3
 #define STEP_ELT(n, w, fun, s, ppb)   do { \
    u32 tt = T32(D ## n + (w) + fun(A ## n, B ## n, C ## n)); \
    A ## n = T32(ROL32(tt, s) + XCAT(tA, XCAT(ppb, n))); \
    D ## n = C ## n; \
    C ## n = B ## n; \
    B ## n = tA ## n; \
  } while (0)
 #define STEP_BIG(w0, w1, w2, w3, w4, w5, w6, w7, fun, r, s, pp8b)   do { \
    tA0 = ROL32(A0, r); \
    tA1 = ROL32(A1, r); \
    tA2 = ROL32(A2, r); \
    tA3 = ROL32(A3, r); \
    tA4 = ROL32(A4, r); \
    tA5 = ROL32(A5, r); \
    tA6 = ROL32(A6, r); \
    tA7 = ROL32(A7, r); \
    STEP_ELT(0, w0, fun, s, pp8b); \
    STEP_ELT(1, w1, fun, s, pp8b); \
    STEP_ELT(2, w2, fun, s, pp8b); \
    STEP_ELT(3, w3, fun, s, pp8b); \
    STEP_ELT(4, w4, fun, s, pp8b); \
    STEP_ELT(5, w5, fun, s, pp8b); \
    STEP_ELT(6, w6, fun, s, pp8b); \
    STEP_ELT(7, w7, fun, s, pp8b); \
  } while (0)
 #define SIMD_M3_0_0   0_
#define SIMD_M3_1_0   1_
#define SIMD_M3_2_0   2_
#define SIMD_M3_3_0   0_
#define SIMD_M3_4_0   1_
#define SIMD_M3_5_0   2_
#define SIMD_M3_6_0   0_
#define SIMD_M3_7_0   1_
 #define SIMD_M3_0_1   1_
#define SIMD_M3_1_1   2_
#define SIMD_M3_2_1   0_
#define SIMD_M3_3_1   1_
#define SIMD_M3_4_1   2_
#define SIMD_M3_5_1   0_
#define SIMD_M3_6_1   1_
#define SIMD_M3_7_1   2_
 #define SIMD_M3_0_2   2_
#define SIMD_M3_1_2   0_
#define SIMD_M3_2_2   1_
#define SIMD_M3_3_2   2_
#define SIMD_M3_4_2   0_
#define SIMD_M3_5_2   1_
#define SIMD_M3_6_2   2_
#define SIMD_M3_7_2   0_
 #define M7_0_0   0_
#define M7_1_0   1_
#define M7_2_0   2_
#define M7_3_0   3_
#define M7_4_0   4_
#define M7_5_0   5_
#define M7_6_0   6_
#define M7_7_0   0_
 #define M7_0_1   1_
#define M7_1_1   2_
#define M7_2_1   3_
#define M7_3_1   4_
#define M7_4_1   5_
#define M7_5_1   6_
#define M7_6_1   0_
#define M7_7_1   1_
 #define M7_0_2   2_
#define M7_1_2   3_
#define M7_2_2   4_
#define M7_3_2   5_
#define M7_4_2   6_
#define M7_5_2   0_
#define M7_6_2   1_
#define M7_7_2   2_
 #define M7_0_3   3_
#define M7_1_3   4_
#define M7_2_3   5_
#define M7_3_3   6_
#define M7_4_3   0_
#define M7_5_3   1_
#define M7_6_3   2_
#define M7_7_3   3_
 #define STEP_BIG_(w, fun, r, s, pp8b)   STEP_BIG w, fun, r, s, pp8b)
 #define ONE_ROUND_BIG(ri, isp, p0, p1, p2, p3)   do { \
    STEP_BIG_(WB_ ## ri ## 0, \
      IF,  p0, p1, XCAT(PP8_, M7_0_ ## isp)); \
    STEP_BIG_(WB_ ## ri ## 1, \
      IF,  p1, p2, XCAT(PP8_, M7_1_ ## isp)); \
    STEP_BIG_(WB_ ## ri ## 2, \
      IF,  p2, p3, XCAT(PP8_, M7_2_ ## isp)); \
    STEP_BIG_(WB_ ## ri ## 3, \
      IF,  p3, p0, XCAT(PP8_, M7_3_ ## isp)); \
    STEP_BIG_(WB_ ## ri ## 4, \
      MAJ, p0, p1, XCAT(PP8_, M7_4_ ## isp)); \
    STEP_BIG_(WB_ ## ri ## 5, \
      MAJ, p1, p2, XCAT(PP8_, M7_5_ ## isp)); \
    STEP_BIG_(WB_ ## ri ## 6, \
      MAJ, p2, p3, XCAT(PP8_, M7_6_ ## isp)); \
    STEP_BIG_(WB_ ## ri ## 7, \
      MAJ, p3, p0, XCAT(PP8_, M7_7_ ## isp)); \
  } while (0)
 #define ONE_ROUND_BIG_PRECOMP(ri, isp, p0, p1, p2, p3)   do { \
    STEP_BIG_(WB_P_ ## ri ## 0, \
      IF,  p0, p1, XCAT(PP8_, M7_0_ ## isp)); \
    STEP_BIG_(WB_P_ ## ri ## 1, \
      IF,  p1, p2, XCAT(PP8_, M7_1_ ## isp)); \
    STEP_BIG_(WB_P_ ## ri ## 2, \
      IF,  p2, p3, XCAT(PP8_, M7_2_ ## isp)); \
    STEP_BIG_(WB_P_ ## ri ## 3, \
      IF,  p3, p0, XCAT(PP8_, M7_3_ ## isp)); \
    STEP_BIG_(WB_P_ ## ri ## 4, \
      MAJ, p0, p1, XCAT(PP8_, M7_4_ ## isp)); \
    STEP_BIG_(WB_P_ ## ri ## 5, \
      MAJ, p1, p2, XCAT(PP8_, M7_5_ ## isp)); \
    STEP_BIG_(WB_P_ ## ri ## 6, \
      MAJ, p2, p3, XCAT(PP8_, M7_6_ ## isp)); \
    STEP_BIG_(WB_P_ ## ri ## 7, \
      MAJ, p3, p0, XCAT(PP8_, M7_7_ ## isp)); \
  } while (0)
 __constant static const u32 SIMD_IV512[] = {
  C32(0x0BA16B95), C32(0x72F999AD), C32(0x9FECC2AE), C32(0xBA3264FC),
  C32(0x5E894929), C32(0x8E9F30E5), C32(0x2F1DAA37), C32(0xF0F2C558),
  C32(0xAC506643), C32(0xA90635A5), C32(0xE25B878B), C32(0xAAB7878F),
  C32(0x88817F7A), C32(0x0A02892B), C32(0x559A7550), C32(0x598F657E),
  C32(0x7EEF60A1), C32(0x6B70E3E8), C32(0x9C1714D1), C32(0xB958E2A8),
  C32(0xAB02675E), C32(0xED1C014F), C32(0xCD8D65BB), C32(0xFDB7A257),
  C32(0x09254899), C32(0xD699C7BC), C32(0x9019B6DC), C32(0x2B9022E4),
  C32(0x8FA14956), C32(0x21BF9BD3), C32(0xB94D0943), C32(0x6FFDDC22)
};
