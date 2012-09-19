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
  last mod: $Id: toplevel_lookup.h 13884 2007-09-22 08:38:10Z giles $

 ********************************************************************/

#include "codec_internal.h"

const ogg_uint32_t PriorKeyFrameWeight[KEY_FRAME_CONTEXT] = { 1,2,3,4,5 };

/* Data structures controlling addition of residue blocks */
const ogg_uint32_t ResidueErrorThresh[Q_TABLE_SIZE] =  {
  750, 700, 650, 600, 590, 580, 570, 560,
  550, 540, 530, 520, 510, 500, 490, 480,
  470, 460, 450, 440, 430, 420, 410, 400,
  390, 380, 370, 360, 350, 340, 330, 320,
  310, 300, 290, 280, 270, 260, 250, 245,
  240, 235, 230, 225, 220, 215, 210, 205,
  200, 195, 190, 185, 180, 175, 170, 165,
  160, 155, 150, 145, 140, 135, 130, 130 };
const ogg_uint32_t ResidueBlockFactor[Q_TABLE_SIZE] =  {
  3,   3,   3,   3,   3,   3,   3,   3,
  3,   3,   3,   3,   3,   3,   3,   3,
  3,   3,   3,   3,   3,   3,   3,   3,
  3,   3,   3,   3,   3,   3,   3,   3,
  2,   2,   2,   2,   2,   2,   2,   2,
  2,   2,   2,   2,   2,   2,   2,   2,
  2,   2,   2,   2,   2,   2,   2,   2,
  2,   2,   2,   2,   2,   2,   2,   2 };
