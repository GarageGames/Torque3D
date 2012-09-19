/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2007                *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function: simple static lookups for VP3 frame encoder
  last mod: $Id: encoder_lookup.h 15323 2008-09-19 19:43:59Z giles $

 ********************************************************************/

#include "codec_internal.h"

static const ogg_uint32_t MvPattern[(MAX_MV_EXTENT * 2) + 1] = {
  0x000000ff, 0x000000fd, 0x000000fb, 0x000000f9,
  0x000000f7, 0x000000f5, 0x000000f3, 0x000000f1,
  0x000000ef, 0x000000ed, 0x000000eb, 0x000000e9,
  0x000000e7, 0x000000e5, 0x000000e3, 0x000000e1,
  0x0000006f, 0x0000006d, 0x0000006b, 0x00000069,
  0x00000067, 0x00000065, 0x00000063, 0x00000061,
  0x0000002f, 0x0000002d, 0x0000002b, 0x00000029,
  0x00000009, 0x00000007, 0x00000002, 0x00000000,
  0x00000001, 0x00000006, 0x00000008, 0x00000028,
  0x0000002a, 0x0000002c, 0x0000002e, 0x00000060,
  0x00000062, 0x00000064, 0x00000066, 0x00000068,
  0x0000006a, 0x0000006c, 0x0000006e, 0x000000e0,
  0x000000e2, 0x000000e4, 0x000000e6, 0x000000e8,
  0x000000ea, 0x000000ec, 0x000000ee, 0x000000f0,
  0x000000f2, 0x000000f4, 0x000000f6, 0x000000f8,
  0x000000fa, 0x000000fc, 0x000000fe,
};

static const ogg_uint32_t MvBits[(MAX_MV_EXTENT * 2) + 1] = {
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  7, 7, 7, 7, 7, 7, 7, 7,
  6, 6, 6, 6, 4, 4, 3, 3,
  3, 4, 4, 6, 6, 6, 6, 7,
  7, 7, 7, 7, 7, 7, 7, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8,
};

static const ogg_uint32_t MvPattern2[(MAX_MV_EXTENT * 2) + 1] = {
  0x0000003f, 0x0000003d, 0x0000003b, 0x00000039,
  0x00000037, 0x00000035, 0x00000033, 0x00000031,
  0x0000002f, 0x0000002d, 0x0000002b, 0x00000029,
  0x00000027, 0x00000025, 0x00000023, 0x00000021,
  0x0000001f, 0x0000001d, 0x0000001b, 0x00000019,
  0x00000017, 0x00000015, 0x00000013, 0x00000011,
  0x0000000f, 0x0000000d, 0x0000000b, 0x00000009,
  0x00000007, 0x00000005, 0x00000003, 0x00000000,
  0x00000002, 0x00000004, 0x00000006, 0x00000008,
  0x0000000a, 0x0000000c, 0x0000000e, 0x00000010,
  0x00000012, 0x00000014, 0x00000016, 0x00000018,
  0x0000001a, 0x0000001c, 0x0000001e, 0x00000020,
  0x00000022, 0x00000024, 0x00000026, 0x00000028,
  0x0000002a, 0x0000002c, 0x0000002e, 0x00000030,
  0x00000032, 0x00000034, 0x00000036, 0x00000038,
  0x0000003a, 0x0000003c, 0x0000003e,
};

static const ogg_uint32_t MvBits2[(MAX_MV_EXTENT * 2) + 1] = {
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6,
};

static const ogg_uint32_t ModeBitPatterns[MAX_MODES] = {
  0x00, 0x02, 0x06, 0x0E, 0x1E, 0x3E, 0x7E, 0x7F };

static const ogg_int32_t ModeBitLengths[MAX_MODES] =  {
  1,    2,    3,    4,    5,    6,    7,    7 };

static const unsigned char ModeSchemes[MODE_METHODS-2][MAX_MODES] =  {
  /* Last Mv dominates */
  { 3,    4,    2,    0,    1,    5,    6,    7 },    /* L P  M N I G GM 4 */
  { 2,    4,    3,    0,    1,    5,    6,    7 },    /* L P  N M I G GM 4 */
  { 3,    4,    1,    0,    2,    5,    6,    7 },    /* L M  P N I G GM 4 */
  { 2,    4,    1,    0,    3,    5,    6,    7 },    /* L M  N P I G GM 4 */

  /* No MV dominates */
  { 0,    4,    3,    1,    2,    5,    6,    7 },    /* N L  P M I G GM 4 */
  { 0,    5,    4,    2,    3,    1,    6,    7 },    /* N G  L P M I GM 4 */

};


static const ogg_uint32_t MvThreshTable[Q_TABLE_SIZE] = {
  65, 65, 65, 65, 50, 50, 50, 50,
  40, 40, 40, 40, 40, 40, 40, 40,
  30, 30, 30, 30, 30, 30, 30, 30,
  20, 20, 20, 20, 20, 20, 20, 20,
  15, 15, 15, 15, 15, 15, 15, 15,
  10, 10, 10, 10, 10, 10, 10, 10,
  5,  5,  5,  5,  5,  5,  5,  5,
  0,  0,  0,  0,  0,  0,  0,  0
};

static const ogg_uint32_t MVChangeFactorTable[Q_TABLE_SIZE] = {
  11, 11, 11, 11, 12, 12, 12, 12,
  13, 13, 13, 13, 13, 13, 13, 13,
  14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15
};
