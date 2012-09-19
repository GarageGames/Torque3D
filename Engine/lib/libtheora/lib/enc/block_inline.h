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
  last mod: $Id: block_inline.h 14059 2007-10-28 23:43:27Z xiphmont $

 ********************************************************************/

#include "codec_internal.h"

static const ogg_int32_t MBOrderMap[4] = { 0, 2, 3, 1 };
static const ogg_int32_t BlockOrderMap1[4][4] = {
  { 0, 1, 3, 2 },
  { 0, 2, 3, 1 },
  { 0, 2, 3, 1 },
  { 3, 2, 0, 1 }
};

static ogg_int32_t QuadMapToIndex1( ogg_int32_t (*BlockMap)[4][4],
                                    ogg_uint32_t SB, ogg_uint32_t MB,
                                    ogg_uint32_t B ){
  return BlockMap[SB][MBOrderMap[MB]][BlockOrderMap1[MB][B]];
}

static ogg_int32_t QuadMapToMBTopLeft( ogg_int32_t (*BlockMap)[4][4],
                                       ogg_uint32_t SB, ogg_uint32_t MB ){
  return BlockMap[SB][MBOrderMap[MB]][0];
}
