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
  last mod: $Id: encode.c 15383 2008-10-10 14:33:46Z xiphmont $

 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include "codec_internal.h"
#include "encoder_lookup.h"
#include "block_inline.h"

#define PUR 8
#define PU 4
#define PUL 2
#define PL 1
#define HIGHBITDUPPED(X) (((ogg_int16_t) X)  >> 15)

static ogg_uint32_t QuadCodeComponent ( CP_INSTANCE *cpi,
                                        ogg_uint32_t FirstSB,
                                        ogg_uint32_t SBRows,
                                        ogg_uint32_t SBCols,
                                        ogg_uint32_t PixelsPerLine){

  ogg_int32_t   FragIndex;      /* Fragment number */
  ogg_uint32_t  MB, B;          /* Macro-Block, Block indices */
  ogg_uint32_t  SBrow;          /* Super-Block row number */
  ogg_uint32_t  SBcol;          /* Super-Block row number */
  ogg_uint32_t  SB=FirstSB;     /* Super-Block index, initialised to first
                                   of this component */
  ogg_uint32_t  coded_pixels=0; /* Number of pixels coded */
  int           MBCodedFlag;

  /* actually transform and quantize the image now that we've decided
     on the modes Parse in quad-tree ordering */

  for ( SBrow=0; SBrow<SBRows; SBrow++ ) {
    for ( SBcol=0; SBcol<SBCols; SBcol++ ) {
      /* Check its four Macro-Blocks  */
      /* 'Macro-Block' is a misnomer in the chroma planes; this is
         really just a Hilbert curve iterator */
      for ( MB=0; MB<4; MB++ ) {

        if ( QuadMapToMBTopLeft(cpi->pb.BlockMap,SB,MB) >= 0 ) {

          MBCodedFlag = 0;

          /*  Now actually code the blocks */
          for ( B=0; B<4; B++ ) {
            FragIndex = QuadMapToIndex1( cpi->pb.BlockMap, SB, MB, B );

            /* Does Block lie in frame: */
            if ( FragIndex >= 0 ) {

              /* In Frame: Is it coded: */
              if ( cpi->pb.display_fragments[FragIndex] ) {

                /* transform and quantize block */
                TransformQuantizeBlock( cpi, FragIndex, PixelsPerLine );

                /* Has the block got struck off (no MV and no data
                   generated after DCT) If not then mark it and the
                   assosciated MB as coded. */
                if ( cpi->pb.display_fragments[FragIndex] ) {
                  /* Create linear list of coded block indices */
                  cpi->pb.CodedBlockList[cpi->pb.CodedBlockIndex] = FragIndex;
                  cpi->pb.CodedBlockIndex++;

                  /* MB is still coded */
                  MBCodedFlag = 1;
                  cpi->MBCodingMode = cpi->pb.FragCodingMethod[FragIndex];

                }
              }
            }
          }
          /* If the MB is marked as coded and we are in the Y plane then */
          /* the mode list needs to be updated. */
          if ( MBCodedFlag && (FirstSB == 0) ){
            /* Make a note of the selected mode in the mode list */
            cpi->ModeList[cpi->ModeListCount] = cpi->MBCodingMode;
            cpi->ModeListCount++;
          }
        }
      }

      SB++;

    }
  }

  /* Return number of pixels coded */
  return coded_pixels;
}

static void EncodeDcTokenList (CP_INSTANCE *cpi) {
  ogg_int32_t   i,j;
  ogg_uint32_t  Token;
  ogg_uint32_t  ExtraBitsToken;
  ogg_uint32_t  HuffIndex;

  ogg_uint32_t  BestDcBits;
  ogg_uint32_t  DcHuffChoice[2];
  ogg_uint32_t  EntropyTableBits[2][DC_HUFF_CHOICES];

  oggpack_buffer *opb=cpi->oggbuffer;

  /* Clear table data structure */
  memset ( EntropyTableBits, 0, sizeof(ogg_uint32_t)*DC_HUFF_CHOICES*2 );

  /* Analyse token list to see which is the best entropy table to use */
  for ( i = 0; i < cpi->OptimisedTokenCount; i++ ) {
    /* Count number of bits for each table option */
    Token = (ogg_uint32_t)cpi->OptimisedTokenList[i];
    for ( j = 0; j < DC_HUFF_CHOICES; j++ ){
      EntropyTableBits[cpi->OptimisedTokenListPl[i]][j] +=
        cpi->pb.HuffCodeLengthArray_VP3x[DC_HUFF_OFFSET + j][Token];
    }
  }

  /* Work out which table option is best for Y */
  BestDcBits = EntropyTableBits[0][0];
  DcHuffChoice[0] = 0;
  for ( j = 1; j < DC_HUFF_CHOICES; j++ ) {
    if ( EntropyTableBits[0][j] < BestDcBits ) {
      BestDcBits = EntropyTableBits[0][j];
      DcHuffChoice[0] = j;
    }
  }

  /* Add the DC huffman table choice to the bitstream */
  oggpackB_write( opb, DcHuffChoice[0], DC_HUFF_CHOICE_BITS );

  /* Work out which table option is best for UV */
  BestDcBits = EntropyTableBits[1][0];
  DcHuffChoice[1] = 0;
  for ( j = 1; j < DC_HUFF_CHOICES; j++ ) {
    if ( EntropyTableBits[1][j] < BestDcBits ) {
      BestDcBits = EntropyTableBits[1][j];
      DcHuffChoice[1] = j;
    }
  }

  /* Add the DC huffman table choice to the bitstream */
  oggpackB_write( opb, DcHuffChoice[1], DC_HUFF_CHOICE_BITS );

  /* Encode the token list */
  for ( i = 0; i < cpi->OptimisedTokenCount; i++ ) {

    /* Get the token and extra bits */
    Token = (ogg_uint32_t)cpi->OptimisedTokenList[i];
    ExtraBitsToken = (ogg_uint32_t)cpi->OptimisedTokenListEb[i];

    /* Select the huffman table */
    if ( cpi->OptimisedTokenListPl[i] == 0)
      HuffIndex = (ogg_uint32_t)DC_HUFF_OFFSET + (ogg_uint32_t)DcHuffChoice[0];
    else
      HuffIndex = (ogg_uint32_t)DC_HUFF_OFFSET + (ogg_uint32_t)DcHuffChoice[1];

    /* Add the bits to the encode holding buffer. */
    cpi->FrameBitCount += cpi->pb.HuffCodeLengthArray_VP3x[HuffIndex][Token];
    oggpackB_write( opb, cpi->pb.HuffCodeArray_VP3x[HuffIndex][Token],
                     (ogg_uint32_t)cpi->
                     pb.HuffCodeLengthArray_VP3x[HuffIndex][Token] );

    /* If the token is followed by an extra bits token then code it */
    if ( cpi->pb.ExtraBitLengths_VP3x[Token] > 0 ) {
      /* Add the bits to the encode holding buffer.  */
      cpi->FrameBitCount += cpi->pb.ExtraBitLengths_VP3x[Token];
      oggpackB_write( opb, ExtraBitsToken,
                       (ogg_uint32_t)cpi->pb.ExtraBitLengths_VP3x[Token] );
    }

  }

  /* Reset the count of second order optimised tokens */
  cpi->OptimisedTokenCount = 0;
}

static void EncodeAcTokenList (CP_INSTANCE *cpi) {
  ogg_int32_t   i,j;
  ogg_uint32_t  Token;
  ogg_uint32_t  ExtraBitsToken;
  ogg_uint32_t  HuffIndex;

  ogg_uint32_t  BestAcBits;
  ogg_uint32_t  AcHuffChoice[2];
  ogg_uint32_t  EntropyTableBits[2][AC_HUFF_CHOICES];

  oggpack_buffer *opb=cpi->oggbuffer;

  memset ( EntropyTableBits, 0, sizeof(ogg_uint32_t)*AC_HUFF_CHOICES*2 );

  /* Analyse token list to see which is the best entropy table to use */
  for ( i = 0; i < cpi->OptimisedTokenCount; i++ ) {
    /* Count number of bits for each table option */
    Token = (ogg_uint32_t)cpi->OptimisedTokenList[i];
    HuffIndex = cpi->OptimisedTokenListHi[i];
    for ( j = 0; j < AC_HUFF_CHOICES; j++ ) {
      EntropyTableBits[cpi->OptimisedTokenListPl[i]][j] +=
        cpi->pb.HuffCodeLengthArray_VP3x[HuffIndex + j][Token];
    }
  }

  /* Select the best set of AC tables for Y */
  BestAcBits = EntropyTableBits[0][0];
  AcHuffChoice[0] = 0;
  for ( j = 1; j < AC_HUFF_CHOICES; j++ ) {
    if ( EntropyTableBits[0][j] < BestAcBits ) {
      BestAcBits = EntropyTableBits[0][j];
      AcHuffChoice[0] = j;
    }
  }

  /* Add the AC-Y huffman table choice to the bitstream */
  oggpackB_write( opb, AcHuffChoice[0], AC_HUFF_CHOICE_BITS );

  /* Select the best set of AC tables for UV */
  BestAcBits = EntropyTableBits[1][0];
  AcHuffChoice[1] = 0;
  for ( j = 1; j < AC_HUFF_CHOICES; j++ ) {
    if ( EntropyTableBits[1][j] < BestAcBits ) {
      BestAcBits = EntropyTableBits[1][j];
      AcHuffChoice[1] = j;
    }
  }

  /* Add the AC-UV huffman table choice to the bitstream */
  oggpackB_write( opb, AcHuffChoice[1], AC_HUFF_CHOICE_BITS );

  /* Encode the token list */
  for ( i = 0; i < cpi->OptimisedTokenCount; i++ ) {
    /* Get the token and extra bits */
    Token = (ogg_uint32_t)cpi->OptimisedTokenList[i];
    ExtraBitsToken = (ogg_uint32_t)cpi->OptimisedTokenListEb[i];

    /* Select the huffman table */
    HuffIndex = (ogg_uint32_t)cpi->OptimisedTokenListHi[i] +
      AcHuffChoice[cpi->OptimisedTokenListPl[i]];

    /* Add the bits to the encode holding buffer. */
    cpi->FrameBitCount += cpi->pb.HuffCodeLengthArray_VP3x[HuffIndex][Token];
    oggpackB_write( opb, cpi->pb.HuffCodeArray_VP3x[HuffIndex][Token],
                     (ogg_uint32_t)cpi->
                     pb.HuffCodeLengthArray_VP3x[HuffIndex][Token] );

    /* If the token is followed by an extra bits token then code it */
    if ( cpi->pb.ExtraBitLengths_VP3x[Token] > 0 ) {
      /* Add the bits to the encode holding buffer. */
      cpi->FrameBitCount += cpi->pb.ExtraBitLengths_VP3x[Token];
      oggpackB_write( opb, ExtraBitsToken,
                       (ogg_uint32_t)cpi->pb.ExtraBitLengths_VP3x[Token] );
    }
  }

  /* Reset the count of second order optimised tokens */
  cpi->OptimisedTokenCount = 0;
}

static void PackModes (CP_INSTANCE *cpi) {
  ogg_uint32_t    i,j;
  unsigned char   ModeIndex;
  const unsigned char  *SchemeList;

  unsigned char   BestModeSchemes[MAX_MODES];
  ogg_int32_t     ModeCount[MAX_MODES];
  ogg_int32_t     TmpFreq = -1;
  ogg_int32_t     TmpIndex = -1;

  ogg_uint32_t    BestScheme;
  ogg_uint32_t    BestSchemeScore;
  ogg_uint32_t    SchemeScore;

  oggpack_buffer *opb=cpi->oggbuffer;

  /* Build a frequency map for the modes in this frame */
  memset( ModeCount, 0, MAX_MODES*sizeof(ogg_int32_t) );
  for ( i = 0; i < cpi->ModeListCount; i++ )
    ModeCount[cpi->ModeList[i]] ++;

  /* Order the modes from most to least frequent.  Store result as
     scheme 0 */
  for ( j = 0; j < MAX_MODES; j++ ) {
    TmpFreq = -1;  /* need to re-initialize for each loop */
    /* Find the most frequent */
    for ( i = 0; i < MAX_MODES; i++ ) {
      /* Is this the best scheme so far ??? */
      if ( ModeCount[i] > TmpFreq ) {
        TmpFreq = ModeCount[i];
        TmpIndex = i;
      }
    }
    /* I don't know if the above loop ever fails to match, but it's
       better safe than sorry.  Plus this takes care of gcc warning */
    if ( TmpIndex != -1 ) {
      ModeCount[TmpIndex] = -1;
      BestModeSchemes[TmpIndex] = (unsigned char)j;
    }
  }

  /* Default/ fallback scheme uses MODE_BITS bits per mode entry */
  BestScheme = (MODE_METHODS - 1);
  BestSchemeScore = cpi->ModeListCount * 3;
  /* Get a bit score for the available schemes. */
  for (  j = 0; j < (MODE_METHODS - 1); j++ ) {

    /* Reset the scheme score */
    if ( j == 0 ){
      /* Scheme 0 additional cost of sending frequency order */
      SchemeScore = 24;
      SchemeList = BestModeSchemes;
    } else {
      SchemeScore = 0;
      SchemeList = ModeSchemes[j-1];
    }

    /* Find the total bits to code using each avaialable scheme */
    for ( i = 0; i < cpi->ModeListCount; i++ )
      SchemeScore += ModeBitLengths[SchemeList[cpi->ModeList[i]]];

    /* Is this the best scheme so far ??? */
    if ( SchemeScore < BestSchemeScore ) {
      BestSchemeScore = SchemeScore;
      BestScheme = j;
    }
  }

  /* Encode the best scheme. */
  oggpackB_write( opb, BestScheme, (ogg_uint32_t)MODE_METHOD_BITS );

  /* If the chosen schems is scheme 0 send details of the mode
     frequency order */
  if ( BestScheme == 0 ) {
    for ( j = 0; j < MAX_MODES; j++ ){
      /* Note that the last two entries are implicit */
      oggpackB_write( opb, BestModeSchemes[j], (ogg_uint32_t)MODE_BITS );
    }
    SchemeList = BestModeSchemes;
  }
  else {
    SchemeList = ModeSchemes[BestScheme-1];
  }

  /* Are we using one of the alphabet based schemes or the fallback scheme */
  if ( BestScheme < (MODE_METHODS - 1)) {
    /* Pack and encode the Mode list */
    for ( i = 0; i < cpi->ModeListCount; i++) {
      /* Add the appropriate mode entropy token. */
      ModeIndex = SchemeList[cpi->ModeList[i]];
      oggpackB_write( opb, ModeBitPatterns[ModeIndex],
                      (ogg_uint32_t)ModeBitLengths[ModeIndex] );
    }
  }else{
    /* Fall back to MODE_BITS per entry */
    for ( i = 0; i < cpi->ModeListCount; i++)
      /* Add the appropriate mode entropy token. */
      oggpackB_write( opb, cpi->ModeList[i], MODE_BITS  );
  }

}

static void PackMotionVectors (CP_INSTANCE *cpi) {
  ogg_int32_t  i;
  ogg_uint32_t MethodBits[2] = {0,0};
  const ogg_uint32_t * MvBitsPtr;
  const ogg_uint32_t * MvPatternPtr;

  oggpack_buffer *opb=cpi->oggbuffer;

  /* Choose the coding method */
  MvBitsPtr = &MvBits[MAX_MV_EXTENT];
  for ( i = 0; i < (ogg_int32_t)cpi->MvListCount; i++ ) {
    MethodBits[0] += MvBitsPtr[cpi->MVList[i].x];
    MethodBits[0] += MvBitsPtr[cpi->MVList[i].y];
    MethodBits[1] += 12; /* Simple six bits per mv component fallback
                             mechanism */
  }

  /* Select entropy table */
  if ( MethodBits[0] < MethodBits[1] ) {
    oggpackB_write( opb, 0, 1 );
    MvBitsPtr = &MvBits[MAX_MV_EXTENT];
    MvPatternPtr = &MvPattern[MAX_MV_EXTENT];
  }else{
    oggpackB_write( opb, 1, 1 );
    MvBitsPtr = &MvBits2[MAX_MV_EXTENT];
    MvPatternPtr = &MvPattern2[MAX_MV_EXTENT];
  }

  /* Pack and encode the motion vectors */
  for ( i = 0; i < (ogg_int32_t)cpi->MvListCount; i++ ) {
    oggpackB_write( opb, MvPatternPtr[cpi->MVList[i].x],
                     (ogg_uint32_t)MvBitsPtr[cpi->MVList[i].x] );
    oggpackB_write( opb, MvPatternPtr[cpi->MVList[i].y],
                     (ogg_uint32_t)MvBitsPtr[cpi->MVList[i].y] );
  }

}

static void PackEOBRun( CP_INSTANCE *cpi) {
  if(cpi->RunLength == 0)
        return;

  /* Note the appropriate EOB or EOB run token and any extra bits in
     the optimised token list.  Use the huffman index assosciated with
     the first token in the run */

  /* Mark out which plane the block belonged to */
  cpi->OptimisedTokenListPl[cpi->OptimisedTokenCount] =
    (unsigned char)cpi->RunPlaneIndex;

  /* Note the huffman index to be used */
  cpi->OptimisedTokenListHi[cpi->OptimisedTokenCount] =
    (unsigned char)cpi->RunHuffIndex;

  if ( cpi->RunLength <= 3 ) {
    if ( cpi->RunLength == 1 ) {
      cpi->OptimisedTokenList[cpi->OptimisedTokenCount] = DCT_EOB_TOKEN;
    } else if ( cpi->RunLength == 2 ) {
      cpi->OptimisedTokenList[cpi->OptimisedTokenCount] = DCT_EOB_PAIR_TOKEN;
    } else {
      cpi->OptimisedTokenList[cpi->OptimisedTokenCount] = DCT_EOB_TRIPLE_TOKEN;
    }

    cpi->RunLength = 0;

  } else {

    /* Choose a token appropriate to the run length. */
    if ( cpi->RunLength < 8 ) {
      cpi->OptimisedTokenList[cpi->OptimisedTokenCount] =
        DCT_REPEAT_RUN_TOKEN;
      cpi->OptimisedTokenListEb[cpi->OptimisedTokenCount] =
        cpi->RunLength - 4;
      cpi->RunLength = 0;
    } else if ( cpi->RunLength < 16 ) {
      cpi->OptimisedTokenList[cpi->OptimisedTokenCount] =
        DCT_REPEAT_RUN2_TOKEN;
      cpi->OptimisedTokenListEb[cpi->OptimisedTokenCount] =
        cpi->RunLength - 8;
      cpi->RunLength = 0;
    } else if ( cpi->RunLength < 32 ) {
      cpi->OptimisedTokenList[cpi->OptimisedTokenCount] =
        DCT_REPEAT_RUN3_TOKEN;
      cpi->OptimisedTokenListEb[cpi->OptimisedTokenCount] =
        cpi->RunLength - 16;
      cpi->RunLength = 0;
    } else if ( cpi->RunLength < 4096) {
      cpi->OptimisedTokenList[cpi->OptimisedTokenCount] =
        DCT_REPEAT_RUN4_TOKEN;
      cpi->OptimisedTokenListEb[cpi->OptimisedTokenCount] =
        cpi->RunLength;
      cpi->RunLength = 0;
    }

  }

  cpi->OptimisedTokenCount++;
  /* Reset run EOB length */
  cpi->RunLength = 0;
}

static void PackToken ( CP_INSTANCE *cpi, ogg_int32_t FragmentNumber,
                 ogg_uint32_t HuffIndex ) {
  ogg_uint32_t Token =
    cpi->pb.TokenList[FragmentNumber][cpi->FragTokens[FragmentNumber]];
  ogg_uint32_t ExtraBitsToken =
    cpi->pb.TokenList[FragmentNumber][cpi->FragTokens[FragmentNumber] + 1];
  ogg_uint32_t OneOrTwo;
  ogg_uint32_t OneOrZero;

  /* Update the record of what coefficient we have got up to for this
     block and unpack the encoded token back into the quantised data
     array. */
  if ( Token == DCT_EOB_TOKEN )
    cpi->pb.FragCoeffs[FragmentNumber] = BLOCK_SIZE;
  else
    ExpandToken( cpi->pb.QFragData[FragmentNumber],
                 &cpi->pb.FragCoeffs[FragmentNumber],
                 Token, ExtraBitsToken );

  /* Update record of tokens coded and where we are in this fragment. */
  /* Is there an extra bits token */
  OneOrTwo= 1 + ( cpi->pb.ExtraBitLengths_VP3x[Token] > 0 );
  /* Advance to the next real token. */
  cpi->FragTokens[FragmentNumber] += (unsigned char)OneOrTwo;

  /* Update the counts of tokens coded */
  cpi->TokensCoded += OneOrTwo;
  cpi->TokensToBeCoded -= OneOrTwo;

  OneOrZero = ( FragmentNumber < (ogg_int32_t)cpi->pb.YPlaneFragments );

  if ( Token == DCT_EOB_TOKEN ) {
    if ( cpi->RunLength == 0 ) {
      cpi->RunHuffIndex = HuffIndex;
      cpi->RunPlaneIndex = 1 -  OneOrZero;
    }
    cpi->RunLength++;

    /* we have exceeded our longest run length  xmit an eob run token; */
    if ( cpi->RunLength == 4095 ) PackEOBRun(cpi);

  }else{

    /* If we have an EOB run then code it up first */
    if ( cpi->RunLength > 0 ) PackEOBRun( cpi);

    /* Mark out which plane the block belonged to */
    cpi->OptimisedTokenListPl[cpi->OptimisedTokenCount] =
      (unsigned char)(1 - OneOrZero);

    /* Note the token, extra bits and hufman table in the optimised
       token list */
    cpi->OptimisedTokenList[cpi->OptimisedTokenCount] =
      (unsigned char)Token;
    cpi->OptimisedTokenListEb[cpi->OptimisedTokenCount] =
      ExtraBitsToken;
    cpi->OptimisedTokenListHi[cpi->OptimisedTokenCount] =
      (unsigned char)HuffIndex;

    cpi->OptimisedTokenCount++;
  }
}

static ogg_uint32_t GetBlockReconErrorSlow( CP_INSTANCE *cpi,
                                     ogg_int32_t BlockIndex ) {
  ogg_uint32_t  ErrorVal;

  unsigned char * SrcDataPtr =
    &cpi->ConvDestBuffer[cpi->pb.pixel_index_table[BlockIndex]];
  unsigned char * RecDataPtr =
    &cpi->pb.LastFrameRecon[cpi->pb.recon_pixel_index_table[BlockIndex]];
  ogg_int32_t   SrcStride;
  ogg_int32_t   RecStride;

  /* Is the block a Y block or a UV block. */
  if ( BlockIndex < (ogg_int32_t)cpi->pb.YPlaneFragments ) {
    SrcStride = cpi->pb.info.width;
    RecStride = cpi->pb.YStride;
  }else{
    SrcStride = cpi->pb.info.width >> 1;
    RecStride = cpi->pb.UVStride;
  }

  ErrorVal = dsp_sad8x8 (cpi->dsp, SrcDataPtr, SrcStride, RecDataPtr, RecStride);

  return ErrorVal;
}

static void PackCodedVideo (CP_INSTANCE *cpi) {
  ogg_int32_t i;
  ogg_int32_t EncodedCoeffs = 1;
  ogg_int32_t FragIndex;
  ogg_uint32_t HuffIndex; /* Index to group of tables used to code a token */

  /* Reset the count of second order optimised tokens */
  cpi->OptimisedTokenCount = 0;

  cpi->TokensToBeCoded = cpi->TotTokenCount;
  cpi->TokensCoded = 0;

  /* Calculate the bit rate at which this frame should be capped. */
  cpi->MaxBitTarget = (ogg_uint32_t)((double)(cpi->ThisFrameTargetBytes * 8) *
                                     cpi->BitRateCapFactor);

  /* Blank the various fragment data structures before we start. */
  memset(cpi->pb.FragCoeffs, 0, cpi->pb.UnitFragments);
  memset(cpi->FragTokens, 0, cpi->pb.UnitFragments);

  /* Clear down the QFragData structure for all coded blocks. */
  ClearDownQFragData(&cpi->pb);

  /* The tree is not needed (implicit) for key frames */
  if ( cpi->pb.FrameType != KEY_FRAME ){
    /* Pack the quad tree fragment mapping. */
    PackAndWriteDFArray( cpi );
  }

  /* Note the number of bits used to code the tree itself. */
  cpi->FrameBitCount = oggpackB_bytes(cpi->oggbuffer) << 3;

  /* Mode and MV data not needed for key frames. */
  if ( cpi->pb.FrameType != KEY_FRAME ){
    /* Pack and code the mode list. */
    PackModes(cpi);
    /* Pack the motion vectors */
    PackMotionVectors (cpi);
  }

  cpi->FrameBitCount = oggpackB_bytes(cpi->oggbuffer) << 3;

  /* Optimise the DC tokens */
  for ( i = 0; i < cpi->pb.CodedBlockIndex; i++ ) {
    /* Get the linear index for the current fragment. */
    FragIndex = cpi->pb.CodedBlockList[i];

    cpi->pb.FragCoefEOB[FragIndex]=(unsigned char)EncodedCoeffs;
    PackToken(cpi, FragIndex, DC_HUFF_OFFSET );

  }

  /* Pack any outstanding EOB tokens */
  PackEOBRun(cpi);

  /* Now output the optimised DC token list using the appropriate
     entropy tables. */
  EncodeDcTokenList(cpi);

  /* Work out the number of DC bits coded */

  /* Optimise the AC tokens */
  while ( EncodedCoeffs < 64 ) {
    /* Huffman table adjustment based upon coefficient number. */
    if ( EncodedCoeffs <= AC_TABLE_2_THRESH )
      HuffIndex = AC_HUFF_OFFSET;
    else if ( EncodedCoeffs <= AC_TABLE_3_THRESH )
      HuffIndex = AC_HUFF_OFFSET + AC_HUFF_CHOICES;
    else if ( EncodedCoeffs <= AC_TABLE_4_THRESH )
      HuffIndex = AC_HUFF_OFFSET + (AC_HUFF_CHOICES * 2);
    else
      HuffIndex = AC_HUFF_OFFSET + (AC_HUFF_CHOICES * 3);

    /* Repeatedly scan through the list of blocks. */
    for ( i = 0; i < cpi->pb.CodedBlockIndex; i++ ) {
      /* Get the linear index for the current fragment. */
      FragIndex = cpi->pb.CodedBlockList[i];

      /* Should we code a token for this block on this pass. */
      if ( cpi->FragTokens[FragIndex] < cpi->FragTokenCounts[FragIndex]
           && cpi->pb.FragCoeffs[FragIndex] <= EncodedCoeffs ) {
        /* Bit pack and a token for this block */
        cpi->pb.FragCoefEOB[FragIndex]=(unsigned char)EncodedCoeffs;
        PackToken( cpi, FragIndex, HuffIndex );
      }
    }

    EncodedCoeffs ++;
  }

  /* Pack any outstanding EOB tokens */
  PackEOBRun(cpi);

  /* Now output the optimised AC token list using the appropriate
     entropy tables. */
  EncodeAcTokenList(cpi);

}

static ogg_uint32_t QuadCodeDisplayFragments (CP_INSTANCE *cpi) {
  ogg_int32_t   i,j;
  ogg_uint32_t  coded_pixels=0;
  int           QIndex;
  int k,m,n;

  /* predictor multiplier up-left, up, up-right,left, shift
     Entries are packed in the order L, UL, U, UR, with missing entries
      moved to the end (before the shift parameters). */
  static const ogg_int16_t pc[16][6]={
    {0,0,0,0,0,0},
    {1,0,0,0,0,0},      /* PL */
    {1,0,0,0,0,0},      /* PUL */
    {1,0,0,0,0,0},      /* PUL|PL */
    {1,0,0,0,0,0},      /* PU */
    {1,1,0,0,1,1},      /* PU|PL */
    {0,1,0,0,0,0},      /* PU|PUL */
    {29,-26,29,0,5,31}, /* PU|PUL|PL */
    {1,0,0,0,0,0},      /* PUR */
    {75,53,0,0,7,127},  /* PUR|PL */
    {1,1,0,0,1,1},      /* PUR|PUL */
    {75,0,53,0,7,127},  /* PUR|PUL|PL */
    {1,0,0,0,0,0},      /* PUR|PU */
    {75,0,53,0,7,127},  /* PUR|PU|PL */
    {3,10,3,0,4,15},    /* PUR|PU|PUL */
    {29,-26,29,0,5,31}  /* PUR|PU|PUL|PL */
  };

  /* boundary case bit masks. */
  static const int bc_mask[8]={
    /* normal case no boundary condition */
    PUR|PU|PUL|PL,
    /* left column */
    PUR|PU,
    /* top row */
    PL,
    /* top row, left column */
    0,
    /* right column */
    PU|PUL|PL,
    /* right and left column */
    PU,
    /* top row, right column */
    PL,
    /* top row, right and left column */
    0
  };

  /* value left value up-left, value up, value up-right, missing
      values skipped. */
  int v[4];

  /* fragment number left, up-left, up, up-right */
  int fn[4];

  /* predictor count. */
  int pcount;

  /*which predictor constants to use */
  ogg_int16_t wpc;

  /* last used inter predictor (Raster Order) */
  ogg_int16_t Last[3];  /* last value used for given frame */

  int FragsAcross=cpi->pb.HFragments;
  int FragsDown = cpi->pb.VFragments;
  int FromFragment,ToFragment;
  ogg_int32_t   FragIndex;
  int WhichFrame;
  int WhichCase;

  static const ogg_int16_t Mode2Frame[] = {
    1,  /* CODE_INTER_NO_MV     0 => Encoded diff from same MB last frame  */
    0,  /* CODE_INTRA           1 => DCT Encoded Block */
    1,  /* CODE_INTER_PLUS_MV   2 => Encoded diff from included MV MB last frame */
    1,  /* CODE_INTER_LAST_MV   3 => Encoded diff from MRU MV MB last frame */
    1,  /* CODE_INTER_PRIOR_MV  4 => Encoded diff from included 4 separate MV blocks */
    2,  /* CODE_USING_GOLDEN    5 => Encoded diff from same MB golden frame */
    2,  /* CODE_GOLDEN_MV       6 => Encoded diff from included MV MB golden frame */
    1   /* CODE_INTER_FOUR_MV   7 => Encoded diff from included 4 separate MV blocks */
  };

  ogg_int16_t PredictedDC;

  /* Initialise the coded block indices variables. These allow
     subsequent linear access to the quad tree ordered list of coded
     blocks */
  cpi->pb.CodedBlockIndex = 0;

  /* Set the inter/intra descision control variables. */
  QIndex = Q_TABLE_SIZE - 1;
  while ( QIndex >= 0 ) {
    if ( (QIndex == 0) ||
         ( cpi->pb.QThreshTable[QIndex] >= cpi->pb.ThisFrameQualityValue) )
      break;
    QIndex --;
  }


  /* Encode and tokenise the Y, U and V components */
  coded_pixels = QuadCodeComponent(cpi, 0, cpi->pb.YSBRows, cpi->pb.YSBCols,
                                   cpi->pb.info.width );
  coded_pixels += QuadCodeComponent(cpi, cpi->pb.YSuperBlocks,
                                    cpi->pb.UVSBRows,
                                    cpi->pb.UVSBCols,
                                    cpi->pb.info.width>>1 );
  coded_pixels += QuadCodeComponent(cpi,
                                    cpi->pb.YSuperBlocks+cpi->pb.UVSuperBlocks,
                                    cpi->pb.UVSBRows, cpi->pb.UVSBCols,
                                    cpi->pb.info.width>>1 );

  /* for y,u,v */
  for ( j = 0; j < 3 ; j++) {
    /* pick which fragments based on Y, U, V */
    switch(j){
    case 0: /* y */
      FromFragment = 0;
      ToFragment = cpi->pb.YPlaneFragments;
      FragsAcross = cpi->pb.HFragments;
      FragsDown = cpi->pb.VFragments;
      break;
    case 1: /* u */
      FromFragment = cpi->pb.YPlaneFragments;
      ToFragment = cpi->pb.YPlaneFragments + cpi->pb.UVPlaneFragments ;
      FragsAcross = cpi->pb.HFragments >> 1;
      FragsDown = cpi->pb.VFragments >> 1;
      break;
    /*case 2:  v */
    default:
      FromFragment = cpi->pb.YPlaneFragments + cpi->pb.UVPlaneFragments;
      ToFragment = cpi->pb.YPlaneFragments + (2 * cpi->pb.UVPlaneFragments) ;
      FragsAcross = cpi->pb.HFragments >> 1;
      FragsDown = cpi->pb.VFragments >> 1;
      break;
    }

    /* initialize our array of last used DC Components */
    for(k=0;k<3;k++)Last[k]=0;
    i=FromFragment;

    /* do prediction on all of Y, U or V */
    for ( m = 0 ; m < FragsDown ; m++) {
      for ( n = 0 ; n < FragsAcross ; n++, i++) {
        cpi->OriginalDC[i] = cpi->pb.QFragData[i][0];

        /* only do 2 prediction if fragment coded and on non intra or
           if all fragments are intra */
        if( cpi->pb.display_fragments[i] ||
            (cpi->pb.FrameType == KEY_FRAME) ) {
          /* Type of Fragment */

          WhichFrame = Mode2Frame[cpi->pb.FragCodingMethod[i]];

          /* Check Borderline Cases */
          WhichCase = (n==0) + ((m==0) << 1) + ((n+1 == FragsAcross) << 2);

          fn[0]=i-1;
          fn[1]=i-FragsAcross-1;
          fn[2]=i-FragsAcross;
          fn[3]=i-FragsAcross+1;

          /* fragment valid for prediction use if coded and it comes
             from same frame as the one we are predicting */
          for(k=pcount=wpc=0; k<4; k++) {
            int pflag;
            pflag=1<<k;
            if((bc_mask[WhichCase]&pflag) &&
               cpi->pb.display_fragments[fn[k]] &&
               (Mode2Frame[cpi->pb.FragCodingMethod[fn[k]]] == WhichFrame)){
              v[pcount]=cpi->OriginalDC[fn[k]];
              wpc|=pflag;
              pcount++;
            }
          }

          if(wpc==0) {

            /* fall back to the last coded fragment */
            cpi->pb.QFragData[i][0] -= Last[WhichFrame];

          } else {

            /* don't do divide if divisor is 1 or 0 */
            PredictedDC = pc[wpc][0]*v[0];
            for(k=1; k<pcount; k++){
              PredictedDC += pc[wpc][k]*v[k];
            }

            /* if we need to do a shift */
            if(pc[wpc][4] != 0 ) {

              /* If negative add in the negative correction factor */
              PredictedDC += (HIGHBITDUPPED(PredictedDC) & pc[wpc][5]);
              /* Shift in lieu of a divide */
              PredictedDC >>= pc[wpc][4];

            }

            /* check for outranging on the two predictors that can outrange */
            if((wpc&(PU|PUL|PL)) == (PU|PUL|PL)){
              if( abs(PredictedDC - v[2]) > 128) {
                PredictedDC = v[2];
              } else if( abs(PredictedDC - v[0]) > 128) {
                PredictedDC = v[0];
              } else if( abs(PredictedDC - v[1]) > 128) {
                PredictedDC = v[1];
              }
            }

            cpi->pb.QFragData[i][0] -= PredictedDC;
          }

          /* Save the last fragment coded for whatever frame we are
             predicting from */

          Last[WhichFrame] = cpi->OriginalDC[i];

        }
      }
    }
  }

  /* Pack DC tokens and adjust the ones we couldn't predict 2d */
  for ( i = 0; i < cpi->pb.CodedBlockIndex; i++ ) {
    /* Get the linear index for the current coded fragment. */
    FragIndex = cpi->pb.CodedBlockList[i];
    coded_pixels += DPCMTokenizeBlock ( cpi, FragIndex);

  }

  /* Bit pack the video data data */
  PackCodedVideo(cpi);

  /* End the bit packing run. */
  /* EndAddBitsToBuffer(cpi); */

  /* Reconstruct the reference frames */
  ReconRefFrames(&cpi->pb);

  UpdateFragQIndex(&cpi->pb);

  /* Measure the inter reconstruction error for all the blocks that
     were coded */
  /* for use as part of the recovery monitoring process in subsequent frames. */
  for ( i = 0; i < cpi->pb.CodedBlockIndex; i++ ) {
    cpi->LastCodedErrorScore[ cpi->pb.CodedBlockList[i] ] =
      GetBlockReconErrorSlow( cpi, cpi->pb.CodedBlockList[i] );

  }

  /* Return total number of coded pixels */
  return coded_pixels;
}

ogg_uint32_t EncodeData(CP_INSTANCE *cpi){
    ogg_uint32_t coded_pixels = 0;

    /* Zero the count of tokens so far this frame. */
    cpi->TotTokenCount = 0;

    /* Zero the mode and MV list indices. */
    cpi->ModeListCount = 0;

    /* Zero Decoder EOB run count */
    cpi->pb.EOB_Run = 0;

    dsp_save_fpu (cpi->dsp);

    /* Encode any fragments coded using DCT. */
    coded_pixels += QuadCodeDisplayFragments (cpi);

    dsp_restore_fpu (cpi->dsp);

    return coded_pixels;

}

ogg_uint32_t PickIntra( CP_INSTANCE *cpi,
                        ogg_uint32_t SBRows,
                        ogg_uint32_t SBCols){

  ogg_int32_t   FragIndex;  /* Fragment number */
  ogg_uint32_t  MB, B;      /* Macro-Block, Block indices */
  ogg_uint32_t  SBrow;      /* Super-Block row number */
  ogg_uint32_t  SBcol;      /* Super-Block row number */
  ogg_uint32_t  SB=0;       /* Super-Block index, initialised to first of
                               this component */
  ogg_uint32_t UVRow;
  ogg_uint32_t UVColumn;
  ogg_uint32_t UVFragOffset;

  /* decide what block type and motion vectors to use on all of the frames */
  for ( SBrow=0; SBrow<SBRows; SBrow++ ) {
    for ( SBcol=0; SBcol<SBCols; SBcol++ ) {
      /* Check its four Macro-Blocks */
      for ( MB=0; MB<4; MB++ ) {
        /* There may be MB's lying out of frame which must be
           ignored. For these MB's Top left block will have a negative
           Fragment Index. */
        if ( QuadMapToMBTopLeft(cpi->pb.BlockMap,SB,MB) >= 0 ) {

          cpi->MBCodingMode = CODE_INTRA;

          /* Now actually code the blocks. */
          for ( B=0; B<4; B++ ) {
            FragIndex = QuadMapToIndex1( cpi->pb.BlockMap, SB, MB, B );
            cpi->pb.FragCodingMethod[FragIndex] = cpi->MBCodingMode;
          }

          /* Matching fragments in the U and V planes */
          UVRow = (FragIndex / (cpi->pb.HFragments * 2));
          UVColumn = (FragIndex % cpi->pb.HFragments) / 2;
          UVFragOffset = (UVRow * (cpi->pb.HFragments / 2)) + UVColumn;

          cpi->pb.FragCodingMethod[cpi->pb.YPlaneFragments + UVFragOffset] =
            cpi->MBCodingMode;
          cpi->pb.FragCodingMethod[cpi->pb.YPlaneFragments +
                                  cpi->pb.UVPlaneFragments + UVFragOffset] =
            cpi->MBCodingMode;
        }
      }

      /* Next Super-Block */
      SB++;
    }
  }
  return 0;
}

static void AddMotionVector(CP_INSTANCE *cpi,
                     MOTION_VECTOR *ThisMotionVector) {
  cpi->MVList[cpi->MvListCount].x = ThisMotionVector->x;
  cpi->MVList[cpi->MvListCount].y = ThisMotionVector->y;
  cpi->MvListCount++;
}

static void SetFragMotionVectorAndMode(CP_INSTANCE *cpi,
                                ogg_int32_t FragIndex,
                                MOTION_VECTOR *ThisMotionVector){
  /* Note the coding mode and vector for each block */
  cpi->pb.FragMVect[FragIndex].x = ThisMotionVector->x;
  cpi->pb.FragMVect[FragIndex].y = ThisMotionVector->y;
  cpi->pb.FragCodingMethod[FragIndex] = cpi->MBCodingMode;
}

static void SetMBMotionVectorsAndMode(CP_INSTANCE *cpi,
                               ogg_int32_t YFragIndex,
                               ogg_int32_t UFragIndex,
                               ogg_int32_t VFragIndex,
                               MOTION_VECTOR *ThisMotionVector){
  SetFragMotionVectorAndMode(cpi, YFragIndex, ThisMotionVector);
  SetFragMotionVectorAndMode(cpi, YFragIndex + 1, ThisMotionVector);
  SetFragMotionVectorAndMode(cpi, YFragIndex + cpi->pb.HFragments,
                             ThisMotionVector);
  SetFragMotionVectorAndMode(cpi, YFragIndex + cpi->pb.HFragments + 1,
                             ThisMotionVector);
  SetFragMotionVectorAndMode(cpi, UFragIndex, ThisMotionVector);
  SetFragMotionVectorAndMode(cpi, VFragIndex, ThisMotionVector);
}

ogg_uint32_t PickModes(CP_INSTANCE *cpi,
                       ogg_uint32_t SBRows, ogg_uint32_t SBCols,
                       ogg_uint32_t PixelsPerLine,
                       ogg_uint32_t *InterError, ogg_uint32_t *IntraError) {
  ogg_int32_t   YFragIndex;
  ogg_int32_t   UFragIndex;
  ogg_int32_t   VFragIndex;
  ogg_uint32_t  MB, B;      /* Macro-Block, Block indices */
  ogg_uint32_t  SBrow;      /* Super-Block row number */
  ogg_uint32_t  SBcol;      /* Super-Block row number */
  ogg_uint32_t  SB=0;       /* Super-Block index, initialised to first
                               of this component */

  ogg_uint32_t  MBIntraError;           /* Intra error for macro block */
  ogg_uint32_t  MBGFError;              /* Golden frame macro block error */
  ogg_uint32_t  MBGF_MVError;           /* Golden frame plus MV error */
  ogg_uint32_t  LastMBGF_MVError;       /* Golden frame error with
                                           last used GF motion
                                           vector. */
  ogg_uint32_t  MBInterError;           /* Inter no MV macro block error */
  ogg_uint32_t  MBLastInterError;       /* Inter with last used MV */
  ogg_uint32_t  MBPriorLastInterError;  /* Inter with prior last MV */
  ogg_uint32_t  MBInterMVError;         /* Inter MV macro block error */
  ogg_uint32_t  MBInterMVExError;       /* Inter MV (exhaustive
                                           search) macro block error */
  ogg_uint32_t  MBInterFOURMVError;     /* Inter MV error when using 4
                                           motion vectors per macro
                                           block */
  ogg_uint32_t  BestError;              /* Best error so far. */

  MOTION_VECTOR FourMVect[6];     /* storage for last used vectors (one
                                     entry for each block in MB) */
  MOTION_VECTOR LastInterMVect;   /* storage for last used Inter frame
                                     MB motion vector */
  MOTION_VECTOR PriorLastInterMVect;  /* storage for prior last used
                                         Inter frame MB motion vector */
  MOTION_VECTOR TmpMVect;         /* Temporary MV storage */
  MOTION_VECTOR LastGFMVect;      /* storage for last used Golden
                                     Frame MB motion vector */
  MOTION_VECTOR InterMVect;       /* storage for motion vector */
  MOTION_VECTOR InterMVectEx;     /* storage for motion vector result
                                     from exhaustive search */
  MOTION_VECTOR GFMVect;          /* storage for motion vector */
  MOTION_VECTOR ZeroVect;

  ogg_uint32_t UVRow;
  ogg_uint32_t UVColumn;
  ogg_uint32_t UVFragOffset;

  int          MBCodedFlag;
  unsigned char QIndex;

  /* initialize error scores */
  *InterError = 0;
  *IntraError = 0;

  /* clear down the default motion vector. */
  cpi->MvListCount = 0;
  FourMVect[0].x = 0;
  FourMVect[0].y = 0;
  FourMVect[1].x = 0;
  FourMVect[1].y = 0;
  FourMVect[2].x = 0;
  FourMVect[2].y = 0;
  FourMVect[3].x = 0;
  FourMVect[3].y = 0;
  FourMVect[4].x = 0;
  FourMVect[4].y = 0;
  FourMVect[5].x = 0;
  FourMVect[5].y = 0;
  LastInterMVect.x = 0;
  LastInterMVect.y = 0;
  PriorLastInterMVect.x = 0;
  PriorLastInterMVect.y = 0;
  LastGFMVect.x = 0;
  LastGFMVect.y = 0;
  InterMVect.x = 0;
  InterMVect.y = 0;
  GFMVect.x = 0;
  GFMVect.y = 0;

  ZeroVect.x = 0;
  ZeroVect.y = 0;

  QIndex = (unsigned char)cpi->pb.FrameQIndex;


  /* change the quatization matrix to the one at best Q to compute the
     new error score */
  cpi->MinImprovementForNewMV = (MvThreshTable[QIndex] << 12);
  cpi->InterTripOutThresh = (5000<<12);
  cpi->MVChangeFactor = MVChangeFactorTable[QIndex]; /* 0.9 */

  if ( cpi->pb.info.quick_p ) {
    cpi->ExhaustiveSearchThresh = (1000<<12);
    cpi->FourMVThreshold = (2500<<12);
  } else {
    cpi->ExhaustiveSearchThresh = (250<<12);
    cpi->FourMVThreshold = (500<<12);
  }
  cpi->MinImprovementForFourMV = cpi->MinImprovementForNewMV * 4;

  if(cpi->MinImprovementForFourMV < (40<<12))
    cpi->MinImprovementForFourMV = (40<<12);

  cpi->FourMvChangeFactor = 8; /* cpi->MVChangeFactor - 0.05;  */

  /* decide what block type and motion vectors to use on all of the frames */
  for ( SBrow=0; SBrow<SBRows; SBrow++ ) {
    for ( SBcol=0; SBcol<SBCols; SBcol++ ) {
      /* Check its four Macro-Blocks */
      for ( MB=0; MB<4; MB++ ) {
        /* There may be MB's lying out of frame which must be
           ignored. For these MB's Top left block will have a negative
           Fragment Index. */
        if ( QuadMapToMBTopLeft(cpi->pb.BlockMap,SB,MB) < 0 ) continue;

        /* Is the current macro block coded (in part or in whole) */
        MBCodedFlag = 0;
        for ( B=0; B<4; B++ ) {
          YFragIndex = QuadMapToIndex1( cpi->pb.BlockMap, SB, MB, B );

          /* Does Block lie in frame: */
          if ( YFragIndex >= 0 ) {
            /* In Frame: Is it coded: */
            if ( cpi->pb.display_fragments[YFragIndex] ) {
              MBCodedFlag = 1;
              break;
            }
          } else
            MBCodedFlag = 0;
        }

        /* This one isn't coded go to the next one */
        if(!MBCodedFlag) continue;

        /* Calculate U and V FragIndex from YFragIndex */
        YFragIndex = QuadMapToMBTopLeft(cpi->pb.BlockMap, SB,MB);
        UVRow = (YFragIndex / (cpi->pb.HFragments * 2));
        UVColumn = (YFragIndex % cpi->pb.HFragments) / 2;
        UVFragOffset = (UVRow * (cpi->pb.HFragments / 2)) + UVColumn;
        UFragIndex = cpi->pb.YPlaneFragments + UVFragOffset;
        VFragIndex = cpi->pb.YPlaneFragments + cpi->pb.UVPlaneFragments +
          UVFragOffset;


        /**************************************************************
         Find the block choice with the lowest error

         NOTE THAT if U or V is coded but no Y from a macro block then
         the mode will be CODE_INTER_NO_MV as this is the default
         state to which the mode data structure is initialised in
         encoder and decoder at the start of each frame. */

        BestError = HUGE_ERROR;


        /* Look at the intra coding error. */
        MBIntraError = GetMBIntraError( cpi, YFragIndex, PixelsPerLine );
        BestError = (BestError > MBIntraError) ? MBIntraError : BestError;

        /* Get the golden frame error */
        MBGFError = GetMBInterError( cpi, cpi->ConvDestBuffer,
                                     cpi->pb.GoldenFrame, YFragIndex,
                                     0, 0, PixelsPerLine );
        BestError = (BestError > MBGFError) ? MBGFError : BestError;

        /* Calculate the 0,0 case. */
        MBInterError = GetMBInterError( cpi, cpi->ConvDestBuffer,
                                        cpi->pb.LastFrameRecon,
                                        YFragIndex, 0, 0, PixelsPerLine );
        BestError = (BestError > MBInterError) ? MBInterError : BestError;

        /* Measure error for last MV */
        MBLastInterError =  GetMBInterError( cpi, cpi->ConvDestBuffer,
                                             cpi->pb.LastFrameRecon,
                                             YFragIndex, LastInterMVect.x,
                                             LastInterMVect.y, PixelsPerLine );
        BestError = (BestError > MBLastInterError) ?
          MBLastInterError : BestError;

        /* Measure error for prior last MV */
        MBPriorLastInterError =  GetMBInterError( cpi, cpi->ConvDestBuffer,
                                                  cpi->pb.LastFrameRecon,
                                                  YFragIndex,
                                                  PriorLastInterMVect.x,
                                                  PriorLastInterMVect.y,
                                                  PixelsPerLine );
        BestError = (BestError > MBPriorLastInterError) ?
          MBPriorLastInterError : BestError;

        /* Temporarily force usage of no motionvector blocks */
        MBInterMVError = HUGE_ERROR;
        InterMVect.x = 0;  /* Set 0,0 motion vector */
        InterMVect.y = 0;

        /* If the best error is above the required threshold search
           for a new inter MV */
        if ( BestError > cpi->MinImprovementForNewMV && cpi->MotionCompensation) {
          /* Use a mix of heirachical and exhaustive searches for
             quick mode. */
          if ( cpi->pb.info.quick_p ) {
            MBInterMVError = GetMBMVInterError( cpi, cpi->pb.LastFrameRecon,
                                                YFragIndex, PixelsPerLine,
                                                cpi->MVPixelOffsetY,
                                                &InterMVect );

            /* If we still do not have a good match try an exhaustive
               MBMV search */
            if ( (MBInterMVError > cpi->ExhaustiveSearchThresh) &&
                 (BestError > cpi->ExhaustiveSearchThresh) ) {

              MBInterMVExError =
                GetMBMVExhaustiveSearch( cpi, cpi->pb.LastFrameRecon,
                                         YFragIndex, PixelsPerLine,
                                         &InterMVectEx );

              /* Is the Variance measure for the EX search
                 better... If so then use it. */
              if ( MBInterMVExError < MBInterMVError ) {
                MBInterMVError = MBInterMVExError;
                InterMVect.x = InterMVectEx.x;
                InterMVect.y = InterMVectEx.y;
              }
            }
          }else{
            /* Use an exhaustive search */
            MBInterMVError =
              GetMBMVExhaustiveSearch( cpi, cpi->pb.LastFrameRecon,
                                       YFragIndex, PixelsPerLine,
                                       &InterMVect );
          }


          /* Is the improvement, if any, good enough to justify a new MV */
          if ( (16 * MBInterMVError < (BestError * cpi->MVChangeFactor)) &&
               ((MBInterMVError + cpi->MinImprovementForNewMV) < BestError) ){
            BestError = MBInterMVError;
          }

        }

        /* If the best error is still above the required threshold
           search for a golden frame MV */
        MBGF_MVError = HUGE_ERROR;
        GFMVect.x = 0; /* Set 0,0 motion vector */
        GFMVect.y = 0;
        if ( BestError > cpi->MinImprovementForNewMV && cpi->MotionCompensation) {
          /* Do an MV search in the golden reference frame */
          MBGF_MVError = GetMBMVInterError( cpi, cpi->pb.GoldenFrame,
                                            YFragIndex, PixelsPerLine,
                                            cpi->MVPixelOffsetY, &GFMVect );

          /* Measure error for last GFMV */
          LastMBGF_MVError =  GetMBInterError( cpi, cpi->ConvDestBuffer,
                                               cpi->pb.GoldenFrame,
                                               YFragIndex, LastGFMVect.x,
                                               LastGFMVect.y, PixelsPerLine );

          /* Check against last GF motion vector and reset if the
             search has thrown a worse result. */
          if ( LastMBGF_MVError < MBGF_MVError ) {
            GFMVect.x = LastGFMVect.x;
            GFMVect.y = LastGFMVect.y;
            MBGF_MVError = LastMBGF_MVError;
          }else{
            LastGFMVect.x = GFMVect.x;
            LastGFMVect.y = GFMVect.y;
          }

          /* Is the improvement, if any, good enough to justify a new MV */
          if ( (16 * MBGF_MVError < (BestError * cpi->MVChangeFactor)) &&
               ((MBGF_MVError + cpi->MinImprovementForNewMV) < BestError) ) {
            BestError = MBGF_MVError;
          }
        }

        /* Finally... If the best error is still to high then consider
           the 4MV mode */
        MBInterFOURMVError = HUGE_ERROR;
        if ( BestError > cpi->FourMVThreshold && cpi->MotionCompensation) {
          /* Get the 4MV error. */
          MBInterFOURMVError =
            GetFOURMVExhaustiveSearch( cpi, cpi->pb.LastFrameRecon,
                                       YFragIndex, PixelsPerLine, FourMVect );

          /* If the improvement is great enough then use the four MV mode */
          if ( ((MBInterFOURMVError + cpi->MinImprovementForFourMV) <
                BestError) && (16 * MBInterFOURMVError <
                               (BestError * cpi->FourMvChangeFactor))) {
            BestError = MBInterFOURMVError;
          }
        }

        /********************************************************
         end finding the best error
         *******************************************************

         Figure out what to do with the block we chose

         Over-ride and force intra if error high and Intra error similar
         Now choose a mode based on lowest error (with bias towards no MV) */

        if ( (BestError > cpi->InterTripOutThresh) &&
             (10 * BestError > MBIntraError * 7 ) ) {
          cpi->MBCodingMode = CODE_INTRA;
          SetMBMotionVectorsAndMode(cpi,YFragIndex,UFragIndex,
                                    VFragIndex,&ZeroVect);
        } else if ( BestError == MBInterError ) {
          cpi->MBCodingMode = CODE_INTER_NO_MV;
          SetMBMotionVectorsAndMode(cpi,YFragIndex,UFragIndex,
                                    VFragIndex,&ZeroVect);
        } else if ( BestError == MBGFError ) {
          cpi->MBCodingMode = CODE_USING_GOLDEN;
          SetMBMotionVectorsAndMode(cpi,YFragIndex,UFragIndex,
                                    VFragIndex,&ZeroVect);
        } else if ( BestError == MBLastInterError ) {
          cpi->MBCodingMode = CODE_INTER_LAST_MV;
          SetMBMotionVectorsAndMode(cpi,YFragIndex,UFragIndex,
                                    VFragIndex,&LastInterMVect);
        } else if ( BestError == MBPriorLastInterError ) {
          cpi->MBCodingMode = CODE_INTER_PRIOR_LAST;
          SetMBMotionVectorsAndMode(cpi,YFragIndex,UFragIndex,
                                    VFragIndex,&PriorLastInterMVect);

          /* Swap the prior and last MV cases over */
          TmpMVect.x = PriorLastInterMVect.x;
          TmpMVect.y = PriorLastInterMVect.y;
          PriorLastInterMVect.x = LastInterMVect.x;
          PriorLastInterMVect.y = LastInterMVect.y;
          LastInterMVect.x = TmpMVect.x;
          LastInterMVect.y = TmpMVect.y;

        } else if ( BestError == MBInterMVError ) {

          cpi->MBCodingMode = CODE_INTER_PLUS_MV;
          SetMBMotionVectorsAndMode(cpi,YFragIndex,UFragIndex,
                                    VFragIndex,&InterMVect);

          /* Update Prior last mv with last mv */
          PriorLastInterMVect.x = LastInterMVect.x;
          PriorLastInterMVect.y = LastInterMVect.y;

          /* Note last inter MV for future use */
          LastInterMVect.x = InterMVect.x;
          LastInterMVect.y = InterMVect.y;

          AddMotionVector( cpi, &InterMVect);

        } else if ( BestError == MBGF_MVError ) {

          cpi->MBCodingMode = CODE_GOLDEN_MV;
          SetMBMotionVectorsAndMode(cpi,YFragIndex,UFragIndex,
                                    VFragIndex,&GFMVect);

          /* Note last inter GF MV for future use */
          LastGFMVect.x = GFMVect.x;
          LastGFMVect.y = GFMVect.y;

          AddMotionVector( cpi, &GFMVect);
        } else if ( BestError == MBInterFOURMVError ) {
          cpi->MBCodingMode = CODE_INTER_FOURMV;

          /* Calculate the UV vectors as the average of the Y plane ones. */
          /* First .x component */
          FourMVect[4].x = FourMVect[0].x + FourMVect[1].x +
            FourMVect[2].x + FourMVect[3].x;
          if ( FourMVect[4].x >= 0 )
            FourMVect[4].x = (FourMVect[4].x + 2) / 4;
          else
            FourMVect[4].x = (FourMVect[4].x - 2) / 4;
          FourMVect[5].x = FourMVect[4].x;

          /* Then .y component */
          FourMVect[4].y = FourMVect[0].y + FourMVect[1].y +
            FourMVect[2].y + FourMVect[3].y;
          if ( FourMVect[4].y >= 0 )
            FourMVect[4].y = (FourMVect[4].y + 2) / 4;
          else
            FourMVect[4].y = (FourMVect[4].y - 2) / 4;
          FourMVect[5].y = FourMVect[4].y;

          SetFragMotionVectorAndMode(cpi, YFragIndex, &FourMVect[0]);
          SetFragMotionVectorAndMode(cpi, YFragIndex + 1, &FourMVect[1]);
          SetFragMotionVectorAndMode(cpi, YFragIndex + cpi->pb.HFragments,
                                     &FourMVect[2]);
          SetFragMotionVectorAndMode(cpi, YFragIndex + cpi->pb.HFragments + 1,
                                     &FourMVect[3]);
          SetFragMotionVectorAndMode(cpi, UFragIndex, &FourMVect[4]);
          SetFragMotionVectorAndMode(cpi, VFragIndex, &FourMVect[5]);

          /* Note the four MVs values for current macro-block. */
          AddMotionVector( cpi, &FourMVect[0]);
          AddMotionVector( cpi, &FourMVect[1]);
          AddMotionVector( cpi, &FourMVect[2]);
          AddMotionVector( cpi, &FourMVect[3]);

          /* Update Prior last mv with last mv */
          PriorLastInterMVect.x = LastInterMVect.x;
          PriorLastInterMVect.y = LastInterMVect.y;

          /* Note last inter MV for future use */
          LastInterMVect.x = FourMVect[3].x;
          LastInterMVect.y = FourMVect[3].y;

        } else {

          cpi->MBCodingMode = CODE_INTRA;
          SetMBMotionVectorsAndMode(cpi,YFragIndex,UFragIndex,
                                    VFragIndex,&ZeroVect);
        }


        /* setting up mode specific block types
           *******************************************************/

        *InterError += (BestError>>8);
        *IntraError += (MBIntraError>>8);


      }
      SB++;

    }
  }

  /* Return number of pixels coded */
  return 0;
}

void WriteFrameHeader( CP_INSTANCE *cpi) {
  ogg_uint32_t i;
  oggpack_buffer *opb=cpi->oggbuffer;
  /* Output the frame type (base/key frame or inter frame) */
  oggpackB_write( opb, cpi->pb.FrameType, 1 );
  /* Write out details of the current value of Q... variable resolution. */
  for ( i = 0; i < Q_TABLE_SIZE; i++ ) {
    if ( cpi->pb.ThisFrameQualityValue == cpi->pb.QThreshTable[i] ) {
      oggpackB_write( opb, i, 6 );
      break;
    }
  }

  if ( i == Q_TABLE_SIZE ) {
    /* An invalid DCT value was specified.  */
    /*IssueWarning( "Invalid Q Multiplier" );*/
    oggpackB_write( opb, 31, 6 );
  }

  /* we only support one Q index per frame */
  oggpackB_write( opb, 0, 1 );

  /* If the frame was a base frame then write out the frame dimensions. */
  if ( cpi->pb.FrameType == KEY_FRAME ) {
    /* all bits reserved! */
    oggpackB_write( opb, 0, 3 );
  }
}

