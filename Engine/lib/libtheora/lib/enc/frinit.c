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
  last mod: $Id: frinit.c 15153 2008-08-04 18:37:55Z tterribe $

 ********************************************************************/

#include <stdlib.h>
#include "codec_internal.h"


void InitializeFragCoordinates(PB_INSTANCE *pbi){

  ogg_uint32_t i, j;

  ogg_uint32_t HorizFrags = pbi->HFragments;
  ogg_uint32_t VertFrags = pbi->VFragments;
  ogg_uint32_t StartFrag = 0;

  /* Y */

  for(i = 0; i< VertFrags; i++){
    for(j = 0; j< HorizFrags; j++){

      ogg_uint32_t ThisFrag = i * HorizFrags + j;
      pbi->FragCoordinates[ ThisFrag ].x=j * BLOCK_HEIGHT_WIDTH;
      pbi->FragCoordinates[ ThisFrag ].y=i * BLOCK_HEIGHT_WIDTH;

    }
  }

  /* U */
  HorizFrags >>= 1;
  VertFrags >>= 1;
  StartFrag = pbi->YPlaneFragments;

  for(i = 0; i< VertFrags; i++) {
    for(j = 0; j< HorizFrags; j++) {
      ogg_uint32_t ThisFrag = StartFrag + i * HorizFrags + j;
      pbi->FragCoordinates[ ThisFrag ].x=j * BLOCK_HEIGHT_WIDTH;
      pbi->FragCoordinates[ ThisFrag ].y=i * BLOCK_HEIGHT_WIDTH;

    }
  }

  /* V */
  StartFrag = pbi->YPlaneFragments + pbi->UVPlaneFragments;
  for(i = 0; i< VertFrags; i++) {
    for(j = 0; j< HorizFrags; j++) {
      ogg_uint32_t ThisFrag = StartFrag + i * HorizFrags + j;
      pbi->FragCoordinates[ ThisFrag ].x=j * BLOCK_HEIGHT_WIDTH;
      pbi->FragCoordinates[ ThisFrag ].y=i * BLOCK_HEIGHT_WIDTH;

    }
  }
}

static void CalcPixelIndexTable( PB_INSTANCE *pbi){
  ogg_uint32_t i;
  ogg_uint32_t * PixelIndexTablePtr;

  /* Calculate the pixel index table for normal image buffers */
  PixelIndexTablePtr = pbi->pixel_index_table;
  for ( i = 0; i < pbi->YPlaneFragments; i++ ) {
    PixelIndexTablePtr[ i ] =
      ((i / pbi->HFragments) * VFRAGPIXELS *
       pbi->info.width);
    PixelIndexTablePtr[ i ] +=
      ((i % pbi->HFragments) * HFRAGPIXELS);
  }

  PixelIndexTablePtr = &pbi->pixel_index_table[pbi->YPlaneFragments];
  for ( i = 0; i < ((pbi->HFragments >> 1) * pbi->VFragments); i++ ) {
    PixelIndexTablePtr[ i ] =
      ((i / (pbi->HFragments / 2) ) *
       (VFRAGPIXELS *
        (pbi->info.width / 2)) );
    PixelIndexTablePtr[ i ] +=
      ((i % (pbi->HFragments / 2) ) *
       HFRAGPIXELS) + pbi->YPlaneSize;
  }

  /************************************************************************/
  /* Now calculate the pixel index table for image reconstruction buffers */
  PixelIndexTablePtr = pbi->recon_pixel_index_table;
  for ( i = 0; i < pbi->YPlaneFragments; i++ ){
    PixelIndexTablePtr[ i ] =
      ((i / pbi->HFragments) * VFRAGPIXELS *
       pbi->YStride);
    PixelIndexTablePtr[ i ] +=
      ((i % pbi->HFragments) * HFRAGPIXELS) +
      pbi->ReconYDataOffset;
  }

  /* U blocks */
  PixelIndexTablePtr = &pbi->recon_pixel_index_table[pbi->YPlaneFragments];
  for ( i = 0; i < pbi->UVPlaneFragments; i++ ) {
    PixelIndexTablePtr[ i ] =
      ((i / (pbi->HFragments / 2) ) *
       (VFRAGPIXELS * (pbi->UVStride)) );
    PixelIndexTablePtr[ i ] +=
      ((i % (pbi->HFragments / 2) ) *
       HFRAGPIXELS) + pbi->ReconUDataOffset;
  }

  /* V blocks */
  PixelIndexTablePtr =
    &pbi->recon_pixel_index_table[pbi->YPlaneFragments +
                                 pbi->UVPlaneFragments];

  for ( i = 0; i < pbi->UVPlaneFragments; i++ ) {
    PixelIndexTablePtr[ i ] =
      ((i / (pbi->HFragments / 2) ) *
       (VFRAGPIXELS * (pbi->UVStride)) );
    PixelIndexTablePtr[ i ] +=
      ((i % (pbi->HFragments / 2) ) * HFRAGPIXELS) +
      pbi->ReconVDataOffset;
  }
}

void ClearFragmentInfo(PB_INSTANCE * pbi){

  /* free prior allocs if present */
  if(pbi->display_fragments) _ogg_free(pbi->display_fragments);
  if(pbi->pixel_index_table) _ogg_free(pbi->pixel_index_table);
  if(pbi->recon_pixel_index_table) _ogg_free(pbi->recon_pixel_index_table);
  if(pbi->FragTokenCounts) _ogg_free(pbi->FragTokenCounts);
  if(pbi->CodedBlockList) _ogg_free(pbi->CodedBlockList);
  if(pbi->FragMVect) _ogg_free(pbi->FragMVect);
  if(pbi->FragCoeffs) _ogg_free(pbi->FragCoeffs);
  if(pbi->FragCoefEOB) _ogg_free(pbi->FragCoefEOB);
  if(pbi->skipped_display_fragments) _ogg_free(pbi->skipped_display_fragments);
  if(pbi->QFragData) _ogg_free(pbi->QFragData);
  if(pbi->TokenList) _ogg_free(pbi->TokenList);
  if(pbi->FragCodingMethod) _ogg_free(pbi->FragCodingMethod);
  if(pbi->FragCoordinates) _ogg_free(pbi->FragCoordinates);

  if(pbi->FragQIndex) _ogg_free(pbi->FragQIndex);
  if(pbi->PPCoefBuffer) _ogg_free(pbi->PPCoefBuffer);
  if(pbi->FragmentVariances) _ogg_free(pbi->FragmentVariances);

  if(pbi->BlockMap) _ogg_free(pbi->BlockMap);

  if(pbi->SBCodedFlags) _ogg_free(pbi->SBCodedFlags);
  if(pbi->SBFullyFlags) _ogg_free(pbi->SBFullyFlags);
  if(pbi->MBFullyFlags) _ogg_free(pbi->MBFullyFlags);
  if(pbi->MBCodedFlags) _ogg_free(pbi->MBCodedFlags);

  if(pbi->_Nodes) _ogg_free(pbi->_Nodes);
  pbi->_Nodes = 0;

  pbi->QFragData = 0;
  pbi->TokenList = 0;
  pbi->skipped_display_fragments = 0;
  pbi->FragCoeffs = 0;
  pbi->FragCoefEOB = 0;
  pbi->display_fragments = 0;
  pbi->pixel_index_table = 0;
  pbi->recon_pixel_index_table = 0;
  pbi->FragTokenCounts = 0;
  pbi->CodedBlockList = 0;
  pbi->FragCodingMethod = 0;
  pbi->FragMVect = 0;
  pbi->MBCodedFlags = 0;
  pbi->MBFullyFlags = 0;
  pbi->BlockMap = 0;

  pbi->SBCodedFlags = 0;
  pbi->SBFullyFlags = 0;
  pbi->QFragData = 0;
  pbi->TokenList = 0;
  pbi->skipped_display_fragments = 0;
  pbi->FragCoeffs = 0;
  pbi->FragCoefEOB = 0;
  pbi->display_fragments = 0;
  pbi->pixel_index_table = 0;
  pbi->recon_pixel_index_table = 0;
  pbi->FragTokenCounts = 0;
  pbi->CodedBlockList = 0;
  pbi->FragCodingMethod = 0;
  pbi->FragCoordinates = 0;
  pbi->FragMVect = 0;

  pbi->PPCoefBuffer=0;
  pbi->PPCoefBuffer=0;
  pbi->FragQIndex = 0;
  pbi->FragQIndex = 0;
  pbi->FragmentVariances= 0;
  pbi->FragmentVariances = 0 ;
}

void InitFragmentInfo(PB_INSTANCE * pbi){

  /* clear any existing info */
  ClearFragmentInfo(pbi);

  /* Perform Fragment Allocations */
  pbi->display_fragments =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->display_fragments));

  pbi->pixel_index_table =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->pixel_index_table));

  pbi->recon_pixel_index_table =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->recon_pixel_index_table));

  pbi->FragTokenCounts =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->FragTokenCounts));

  pbi->CodedBlockList =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->CodedBlockList));

  pbi->FragMVect =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->FragMVect));

  pbi->FragCoeffs =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->FragCoeffs));

  pbi->FragCoefEOB =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->FragCoefEOB));

  pbi->skipped_display_fragments =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->skipped_display_fragments));

  pbi->QFragData =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->QFragData));

  pbi->TokenList =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->TokenList));

  pbi->FragCodingMethod =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->FragCodingMethod));

  pbi->FragCoordinates =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->FragCoordinates));

  pbi->FragQIndex =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->FragQIndex));

  pbi->PPCoefBuffer =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->PPCoefBuffer));

  pbi->FragmentVariances =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->FragmentVariances));

  pbi->_Nodes =
    _ogg_malloc(pbi->UnitFragments * sizeof(*pbi->_Nodes));

  /* Super Block Initialization */
  pbi->SBCodedFlags =
    _ogg_malloc(pbi->SuperBlocks * sizeof(*pbi->SBCodedFlags));

  pbi->SBFullyFlags =
    _ogg_malloc(pbi->SuperBlocks * sizeof(*pbi->SBFullyFlags));

  /* Macro Block Initialization */
  pbi->MBCodedFlags =
    _ogg_malloc(pbi->MacroBlocks * sizeof(*pbi->MBCodedFlags));

  pbi->MBFullyFlags =
    _ogg_malloc(pbi->MacroBlocks * sizeof(*pbi->MBFullyFlags));

  pbi->BlockMap =
    _ogg_malloc(pbi->SuperBlocks * sizeof(*pbi->BlockMap));

}

void ClearFrameInfo(PB_INSTANCE * pbi){
  if(pbi->ThisFrameRecon )
    _ogg_free(pbi->ThisFrameRecon );
  if(pbi->GoldenFrame)
    _ogg_free(pbi->GoldenFrame);
  if(pbi->LastFrameRecon)
    _ogg_free(pbi->LastFrameRecon);
  if(pbi->PostProcessBuffer)
    _ogg_free(pbi->PostProcessBuffer);


  pbi->ThisFrameRecon = 0;
  pbi->GoldenFrame = 0;
  pbi->LastFrameRecon = 0;
  pbi->PostProcessBuffer = 0;


  pbi->ThisFrameRecon = 0;
  pbi->GoldenFrame = 0;
  pbi->LastFrameRecon = 0;
  pbi->PostProcessBuffer = 0;

}

void InitFrameInfo(PB_INSTANCE * pbi, unsigned int FrameSize){

  /* clear any existing info */
  ClearFrameInfo(pbi);

  /* allocate frames */
  pbi->ThisFrameRecon =
    _ogg_malloc(FrameSize*sizeof(*pbi->ThisFrameRecon));

  pbi->GoldenFrame =
    _ogg_malloc(FrameSize*sizeof(*pbi->GoldenFrame));

  pbi->LastFrameRecon =
    _ogg_malloc(FrameSize*sizeof(*pbi->LastFrameRecon));

  pbi->PostProcessBuffer =
    _ogg_malloc(FrameSize*sizeof(*pbi->PostProcessBuffer));

}

void InitFrameDetails(PB_INSTANCE *pbi){
  int FrameSize;

  /*pbi->PostProcessingLevel = 0;
    pbi->PostProcessingLevel = 4;
    pbi->PostProcessingLevel = 5;
    pbi->PostProcessingLevel = 6;*/

  pbi->PostProcessingLevel = 0;


    /* Set the frame size etc. */

  pbi->YPlaneSize = pbi->info.width *
    pbi->info.height;
  pbi->UVPlaneSize = pbi->YPlaneSize / 4;
  pbi->HFragments = pbi->info.width / HFRAGPIXELS;
  pbi->VFragments = pbi->info.height / VFRAGPIXELS;
  pbi->UnitFragments = ((pbi->VFragments * pbi->HFragments)*3)/2;
  pbi->YPlaneFragments = pbi->HFragments * pbi->VFragments;
  pbi->UVPlaneFragments = pbi->YPlaneFragments / 4;

  pbi->YStride = (pbi->info.width + STRIDE_EXTRA);
  pbi->UVStride = pbi->YStride / 2;
  pbi->ReconYPlaneSize = pbi->YStride *
    (pbi->info.height + STRIDE_EXTRA);
  pbi->ReconUVPlaneSize = pbi->ReconYPlaneSize / 4;
  FrameSize = pbi->ReconYPlaneSize + 2 * pbi->ReconUVPlaneSize;

  pbi->YDataOffset = 0;
  pbi->UDataOffset = pbi->YPlaneSize;
  pbi->VDataOffset = pbi->YPlaneSize + pbi->UVPlaneSize;
  pbi->ReconYDataOffset =
    (pbi->YStride * UMV_BORDER) + UMV_BORDER;
  pbi->ReconUDataOffset = pbi->ReconYPlaneSize +
    (pbi->UVStride * (UMV_BORDER/2)) + (UMV_BORDER/2);
  pbi->ReconVDataOffset = pbi->ReconYPlaneSize + pbi->ReconUVPlaneSize +
    (pbi->UVStride * (UMV_BORDER/2)) + (UMV_BORDER/2);

  /* Image dimensions in Super-Blocks */
  pbi->YSBRows = (pbi->info.height/32)  +
    ( pbi->info.height%32 ? 1 : 0 );
  pbi->YSBCols = (pbi->info.width/32)  +
    ( pbi->info.width%32 ? 1 : 0 );
  pbi->UVSBRows = ((pbi->info.height/2)/32)  +
    ( (pbi->info.height/2)%32 ? 1 : 0 );
  pbi->UVSBCols = ((pbi->info.width/2)/32)  +
    ( (pbi->info.width/2)%32 ? 1 : 0 );

  /* Super-Blocks per component */
  pbi->YSuperBlocks = pbi->YSBRows * pbi->YSBCols;
  pbi->UVSuperBlocks = pbi->UVSBRows * pbi->UVSBCols;
  pbi->SuperBlocks = pbi->YSuperBlocks+2*pbi->UVSuperBlocks;

  /* Useful externals */
  pbi->MacroBlocks = ((pbi->VFragments+1)/2)*((pbi->HFragments+1)/2);

  InitFragmentInfo(pbi);
  InitFrameInfo(pbi, FrameSize);
  InitializeFragCoordinates(pbi);

  /* Configure mapping between quad-tree and fragments */
  CreateBlockMapping ( pbi->BlockMap, pbi->YSuperBlocks,
                       pbi->UVSuperBlocks, pbi->HFragments, pbi->VFragments);

  /* Re-initialise the pixel index table. */

  CalcPixelIndexTable( pbi );

}

