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
  last mod: $Id: frarray.c 15153 2008-08-04 18:37:55Z tterribe $

 ********************************************************************/

#include <string.h>
#include "codec_internal.h"
#include "block_inline.h"

/* Long run bit string coding */
static ogg_uint32_t FrArrayCodeSBRun( CP_INSTANCE *cpi, ogg_uint32_t value){
  ogg_uint32_t CodedVal = 0;
  ogg_uint32_t CodedBits = 0;

  /* Coding scheme:
        Codeword              RunLength
      0                       1
      10x                     2-3
      110x                    4-5
      1110xx                  6-9
      11110xxx                10-17
      111110xxxx              18-33
      111111xxxxxxxxxxxx      34-4129 */

  if ( value == 1 ){
    CodedVal = 0;
    CodedBits = 1;
  } else if ( value <= 3 ) {
    CodedVal = 0x0004 + (value - 2);
    CodedBits = 3;
  } else if ( value <= 5 ) {
    CodedVal = 0x000C + (value - 4);
    CodedBits = 4;
  } else if ( value <= 9 ) {
    CodedVal = 0x0038 + (value - 6);
    CodedBits = 6;
  } else if ( value <= 17 ) {
    CodedVal = 0x00F0 + (value - 10);
    CodedBits = 8;
  } else if ( value <= 33 ) {
    CodedVal = 0x03E0 + (value - 18);
    CodedBits = 10;
  } else {
    CodedVal = 0x3F000 + (value - 34);
    CodedBits = 18;
  }

  /* Add the bits to the encode holding buffer. */
  oggpackB_write( cpi->oggbuffer, CodedVal, CodedBits );

  return CodedBits;
}

/* Short run bit string coding */
static ogg_uint32_t FrArrayCodeBlockRun( CP_INSTANCE *cpi,
                                         ogg_uint32_t value ) {
  ogg_uint32_t CodedVal = 0;
  ogg_uint32_t CodedBits = 0;

  /* Coding scheme:
        Codeword                                RunLength
        0x                                      1-2
        10x                                     3-4
        110x                                    5-6
        1110xx                                  7-10
        11110xx                                 11-14
        11111xxxx                               15-30 */

  if ( value <= 2 ) {
    CodedVal = value - 1;
    CodedBits = 2;
  } else if ( value <= 4 ) {
    CodedVal = 0x0004 + (value - 3);
    CodedBits = 3;

  } else if ( value <= 6 ) {
    CodedVal = 0x000C + (value - 5);
    CodedBits = 4;

  } else if ( value <= 10 ) {
    CodedVal = 0x0038 + (value - 7);
    CodedBits = 6;

  } else if ( value <= 14 ) {
    CodedVal = 0x0078 + (value - 11);
    CodedBits = 7;
  } else {
    CodedVal = 0x01F0 + (value - 15);
    CodedBits = 9;
 }

  /* Add the bits to the encode holding buffer. */
  oggpackB_write( cpi->oggbuffer, CodedVal, CodedBits );

  return CodedBits;
}

void PackAndWriteDFArray( CP_INSTANCE *cpi ){
  ogg_uint32_t  i;
  unsigned char val;
  ogg_uint32_t  run_count;

  ogg_uint32_t  SB, MB, B;   /* Block, MB and SB loop variables */
  ogg_uint32_t  BListIndex = 0;
  ogg_uint32_t  LastSbBIndex = 0;
  ogg_int32_t   DfBlockIndex;  /* Block index in display_fragments */

  /* Initialise workspaces */
  memset( cpi->pb.SBFullyFlags, 1, cpi->pb.SuperBlocks);
  memset( cpi->pb.SBCodedFlags, 0, cpi->pb.SuperBlocks );
  memset( cpi->PartiallyCodedFlags, 0, cpi->pb.SuperBlocks );
  memset( cpi->BlockCodedFlags, 0, cpi->pb.UnitFragments);

  for( SB = 0; SB < cpi->pb.SuperBlocks; SB++ ) {
    /* Check for coded blocks and macro-blocks */
    for ( MB=0; MB<4; MB++ ) {
      /* If MB in frame */
      if ( QuadMapToMBTopLeft(cpi->pb.BlockMap,SB,MB) >= 0 ) {
        for ( B=0; B<4; B++ ) {
          DfBlockIndex = QuadMapToIndex1( cpi->pb.BlockMap,SB, MB, B );

          /* Does Block lie in frame: */
          if ( DfBlockIndex >= 0 ) {
            /* In Frame: If it is not coded then this SB is only
               partly coded.: */
            if ( cpi->pb.display_fragments[DfBlockIndex] ) {
              cpi->pb.SBCodedFlags[SB] = 1; /* SB at least partly coded */
              cpi->BlockCodedFlags[BListIndex] = 1; /* Block is coded */

            }else{
              cpi->pb.SBFullyFlags[SB] = 0; /* SB not fully coded */
              cpi->BlockCodedFlags[BListIndex] = 0; /* Block is not coded */
            }

            BListIndex++;
          }
        }
      }
    }

    /* Is the SB fully coded or uncoded.
       If so then backup BListIndex and MBListIndex */
    if ( cpi->pb.SBFullyFlags[SB] || !cpi->pb.SBCodedFlags[SB] ) {
      BListIndex = LastSbBIndex; /* Reset to values from previous SB */
    }else{
      cpi->PartiallyCodedFlags[SB] = 1; /* Set up list of partially
                                           coded SBs */
      LastSbBIndex = BListIndex;
    }
  }

  /* Code list of partially coded Super-Block.  */
  val = cpi->PartiallyCodedFlags[0];
  oggpackB_write( cpi->oggbuffer, (ogg_uint32_t)val, 1);

  i = 0;
  while ( i < cpi->pb.SuperBlocks ) {
    run_count = 0;
    while ( (i<cpi->pb.SuperBlocks) &&
            (cpi->PartiallyCodedFlags[i]==val) &&
            run_count<4129 ) {
      i++;
      run_count++;
    }

    /* Code the run */
    FrArrayCodeSBRun( cpi, run_count);

    if(run_count >= 4129 && i < cpi->pb.SuperBlocks ){
      val = cpi->PartiallyCodedFlags[i];
      oggpackB_write( cpi->oggbuffer, (ogg_uint32_t)val, 1);

    }else
      val = ( val == 0 ) ? 1 : 0;
  }

  /* RLC Super-Block fully/not coded. */
  i = 0;

  /* Skip partially coded blocks */
  while( (i < cpi->pb.SuperBlocks) && cpi->PartiallyCodedFlags[i] )
    i++;

  if ( i < cpi->pb.SuperBlocks ) {
    val = cpi->pb.SBFullyFlags[i];
    oggpackB_write( cpi->oggbuffer, (ogg_uint32_t)val, 1);

    while ( i < cpi->pb.SuperBlocks ) {
      run_count = 0;
      while ( (i < cpi->pb.SuperBlocks) &&
              (cpi->pb.SBFullyFlags[i] == val) &&
              run_count < 4129) {
        i++;
        /* Skip partially coded blocks */
        while( (i < cpi->pb.SuperBlocks) && cpi->PartiallyCodedFlags[i] )
          i++;
        run_count++;
      }

      /* Code the run */
      FrArrayCodeSBRun( cpi, run_count );

    if(run_count >= 4129 && i < cpi->pb.SuperBlocks ){
      val = cpi->PartiallyCodedFlags[i];
      oggpackB_write( cpi->oggbuffer, (ogg_uint32_t)val, 1);
    }else
      val = ( val == 0 ) ? 1 : 0;
    }
  }


  /*  Now code the block flags */
  if ( BListIndex > 0 ) {
    /* Code the block flags start value */
    val = cpi->BlockCodedFlags[0];
    oggpackB_write( cpi->oggbuffer, (ogg_uint32_t)val, 1);

    /* Now code the block flags. */
    for ( i = 0; i < BListIndex; ) {
      run_count = 0;
      while ( (i < BListIndex) && (cpi->BlockCodedFlags[i] == val) ) {
        i++;
        run_count++;
      }

      FrArrayCodeBlockRun( cpi, run_count );

      val = ( val == 0 ) ? 1 : 0;
    }
  }
}
