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

  function:
  last mod: $Id: encoder_huffman.h 13884 2007-09-22 08:38:10Z giles $

 ********************************************************************/

/********************************************************************
 *  Constants
 ********************************************************************/
#define NUM_HUFF_TABLES         80
#define DC_HUFF_OFFSET          0
#define AC_HUFF_OFFSET          16
#define AC_TABLE_2_THRESH       5
#define AC_TABLE_3_THRESH       14
#define AC_TABLE_4_THRESH       27

#define DC_HUFF_CHOICES         16
#define DC_HUFF_CHOICE_BITS     4

#define AC_HUFF_CHOICES         16
#define AC_HUFF_CHOICE_BITS     4

/* Constants assosciated with entropy tokenisation. */
#define MAX_SINGLE_TOKEN_VALUE  6
#define DCT_VAL_CAT2_MIN        3
#define DCT_VAL_CAT3_MIN        7
#define DCT_VAL_CAT4_MIN        9
#define DCT_VAL_CAT5_MIN        13
#define DCT_VAL_CAT6_MIN        21
#define DCT_VAL_CAT7_MIN        37
#define DCT_VAL_CAT8_MIN        69

#define DCT_EOB_TOKEN           0
#define DCT_EOB_PAIR_TOKEN      1
#define DCT_EOB_TRIPLE_TOKEN    2
#define DCT_REPEAT_RUN_TOKEN    3
#define DCT_REPEAT_RUN2_TOKEN   4
#define DCT_REPEAT_RUN3_TOKEN   5
#define DCT_REPEAT_RUN4_TOKEN   6

#define DCT_SHORT_ZRL_TOKEN     7
#define DCT_ZRL_TOKEN           8

#define ONE_TOKEN               9       /* Special tokens for -1,1,-2,2 */
#define MINUS_ONE_TOKEN         10
#define TWO_TOKEN               11
#define MINUS_TWO_TOKEN         12

#define LOW_VAL_TOKENS          (MINUS_TWO_TOKEN + 1)
#define DCT_VAL_CATEGORY3       (LOW_VAL_TOKENS + 4)
#define DCT_VAL_CATEGORY4       (DCT_VAL_CATEGORY3 + 1)
#define DCT_VAL_CATEGORY5       (DCT_VAL_CATEGORY4 + 1)
#define DCT_VAL_CATEGORY6       (DCT_VAL_CATEGORY5 + 1)
#define DCT_VAL_CATEGORY7       (DCT_VAL_CATEGORY6 + 1)
#define DCT_VAL_CATEGORY8       (DCT_VAL_CATEGORY7 + 1)

#define DCT_RUN_CATEGORY1       (DCT_VAL_CATEGORY8 + 1)
#define DCT_RUN_CATEGORY1B      (DCT_RUN_CATEGORY1 + 5)
#define DCT_RUN_CATEGORY1C      (DCT_RUN_CATEGORY1B + 1)
#define DCT_RUN_CATEGORY2       (DCT_RUN_CATEGORY1C + 1)

/* 32 */
#define MAX_ENTROPY_TOKENS      (DCT_RUN_CATEGORY2 + 2)
