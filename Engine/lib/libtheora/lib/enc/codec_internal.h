/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2005                *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function:
  last mod: $Id: codec_internal.h 15153 2008-08-04 18:37:55Z tterribe $

 ********************************************************************/

#ifndef ENCODER_INTERNAL_H
#define ENCODER_INTERNAL_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

typedef struct PB_INSTANCE PB_INSTANCE;
#include "dsp.h"

#include "theora/theora.h"
#include "encoder_huffman.h"

#define theora_read(x,y,z) ( *z = oggpackB_read(x,y) )

#define CURRENT_ENCODE_VERSION   1
#define HUGE_ERROR              (1<<28)  /*  Out of range test value */

/* Baseline dct height and width. */
#define BLOCK_HEIGHT_WIDTH          8
#define HFRAGPIXELS                 8
#define VFRAGPIXELS                 8

/* Blocks on INTRA/INTER Y/U/V planes */
enum BlockMode {
  BLOCK_Y,
  BLOCK_U,
  BLOCK_V,
  BLOCK_INTER_Y,
  BLOCK_INTER_U,
  BLOCK_INTER_V
};

/* Baseline dct block size */
#define BLOCK_SIZE              (BLOCK_HEIGHT_WIDTH * BLOCK_HEIGHT_WIDTH)

/* Border is for unrestricted mv's */
#define UMV_BORDER              16
#define STRIDE_EXTRA            (UMV_BORDER * 2)

#define Q_TABLE_SIZE            64

#define KEY_FRAME              0
#define DELTA_FRAME            1

#define MAX_MODES               8
#define MODE_BITS               3
#define MODE_METHODS            8
#define MODE_METHOD_BITS        3

/* Different key frame types/methods */
#define DCT_KEY_FRAME           0

#define KEY_FRAME_CONTEXT       5

/* Preprocessor defines */
#define MAX_PREV_FRAMES        16

/* Number of search sites for a 4-step search (at pixel accuracy) */
#define MAX_SEARCH_SITES       33

#define VERY_BEST_Q            10
#define MIN_BPB_FACTOR        0.3
#define MAX_BPB_FACTOR        3.0

#define MAX_MV_EXTENT 31  /* Max search distance in half pixel increments */

typedef struct CONFIG_TYPE2{
  double       OutputFrameRate;
  ogg_uint32_t TargetBandwidth;
  ogg_uint32_t KeyFrameDataTarget ;  /* Data rate target for key frames */

  ogg_uint32_t FirstFrameQ;
  ogg_uint32_t BaseQ;
  ogg_uint32_t MaxQ;            /* Absolute Max Q allowed. */
  ogg_uint32_t ActiveMaxQ;      /* Currently active Max Q */

} CONFIG_TYPE2;

typedef struct coeffNode{
  int i;
  struct coeffNode *next;
} COEFFNODE;

typedef struct{
  unsigned char * Yuv0ptr;
  unsigned char * Yuv1ptr;
  unsigned char * SrfWorkSpcPtr;
  unsigned char * disp_fragments;

  ogg_uint32_t  * RegionIndex; /* Gives pixel index for top left of
                                 each block */
  ogg_uint32_t    VideoFrameHeight;
  ogg_uint32_t    VideoFrameWidth;

} SCAN_CONFIG_DATA;

typedef unsigned char YUV_BUFFER_ENTRY;

typedef struct{
  ogg_int32_t   x;
  ogg_int32_t   y;
} MOTION_VECTOR;

typedef MOTION_VECTOR COORDINATE;

/** Quantizer matrix entry */
typedef ogg_int16_t     Q_LIST_ENTRY;

/** Decode Post-Processor instance */
typedef struct PP_INSTANCE {
  ogg_uint32_t  PrevFrameLimit;

  ogg_uint32_t   *ScanPixelIndexTable;
  signed char    *ScanDisplayFragments;

  signed char    *PrevFragments[MAX_PREV_FRAMES];

  ogg_uint32_t   *FragScores; /* The individual frame difference ratings. */
  signed char    *SameGreyDirPixels;
  signed char    *BarBlockMap;

  /* Number of pixels changed by diff threshold in row of a fragment. */
  unsigned char  *FragDiffPixels;

  unsigned char  *PixelScores;
  unsigned char  *PixelChangedMap;
  unsigned char  *ChLocals;
  ogg_int16_t    *yuv_differences;
  ogg_int32_t    *RowChangedPixels;
  signed char    *TmpCodedMap;

  /* Plane pointers and dimension variables */
  unsigned char  * YPlanePtr0;
  unsigned char  * YPlanePtr1;
  unsigned char  * UPlanePtr0;
  unsigned char  * UPlanePtr1;
  unsigned char  * VPlanePtr0;
  unsigned char  * VPlanePtr1;

  ogg_uint32_t    VideoYPlaneWidth;
  ogg_uint32_t    VideoYPlaneHeight;
  ogg_uint32_t    VideoUVPlaneWidth;
  ogg_uint32_t    VideoUVPlaneHeight;

  ogg_uint32_t    VideoYPlaneStride;
  ogg_uint32_t    VideoUPlaneStride;
  ogg_uint32_t    VideoVPlaneStride;

  /* Scan control variables. */
  unsigned char   HFragPixels;
  unsigned char   VFragPixels;

  ogg_uint32_t    ScanFrameFragments;
  ogg_uint32_t    ScanYPlaneFragments;
  ogg_uint32_t    ScanUVPlaneFragments;
  ogg_uint32_t    ScanHFragments;
  ogg_uint32_t    ScanVFragments;

  ogg_uint32_t    YFramePixels;
  ogg_uint32_t    UVFramePixels;

  ogg_uint32_t    SgcThresh;

  ogg_uint32_t    OutputBlocksUpdated;
  ogg_uint32_t    KFIndicator;

  /* The pre-processor scan configuration. */
  SCAN_CONFIG_DATA ScanConfig;

  ogg_int32_t   SRFGreyThresh;
  ogg_int32_t   SRFColThresh;
  ogg_int32_t   SgcLevelThresh;
  ogg_int32_t   SuvcLevelThresh;

  ogg_uint32_t  NoiseSupLevel;

  /* Block Thresholds. */
  ogg_uint32_t  PrimaryBlockThreshold;
  unsigned char LineSearchTripTresh;

  int   PAKEnabled;

  int   LevelThresh;
  int   NegLevelThresh;
  int   SrfThresh;
  int   NegSrfThresh;
  int   HighChange;
  int   NegHighChange;

  /* Threshold lookup tables */
  unsigned char SrfPakThreshTable[512];
  unsigned char SrfThreshTable[512];
  unsigned char SgcThreshTable[512];

  /* Variables controlling S.A.D. break outs. */
  ogg_uint32_t GrpLowSadThresh;
  ogg_uint32_t GrpHighSadThresh;
  ogg_uint32_t ModifiedGrpLowSadThresh;
  ogg_uint32_t ModifiedGrpHighSadThresh;

  ogg_int32_t  PlaneHFragments;
  ogg_int32_t  PlaneVFragments;
  ogg_int32_t  PlaneHeight;
  ogg_int32_t  PlaneWidth;
  ogg_int32_t  PlaneStride;

  ogg_uint32_t BlockThreshold;
  ogg_uint32_t BlockSgcThresh;
  double UVBlockThreshCorrection;
  double UVSgcCorrection;

  double YUVPlaneCorrectionFactor;
  double AbsDiff_ScoreMultiplierTable[256];
  unsigned char  NoiseScoreBoostTable[256];
  unsigned char  MaxLineSearchLen;

  ogg_int32_t YuvDiffsCircularBufferSize;
  ogg_int32_t ChLocalsCircularBufferSize;
  ogg_int32_t PixelMapCircularBufferSize;

  DspFunctions dsp;  /* Selected functions for this platform */

} PP_INSTANCE;

/** block coding modes */
typedef enum{
  CODE_INTER_NO_MV        = 0x0, /* INTER prediction, (0,0) motion
                                    vector implied.  */
    CODE_INTRA            = 0x1, /* INTRA i.e. no prediction. */
    CODE_INTER_PLUS_MV    = 0x2, /* INTER prediction, non zero motion
                                    vector. */
    CODE_INTER_LAST_MV    = 0x3, /* Use Last Motion vector */
    CODE_INTER_PRIOR_LAST = 0x4, /* Prior last motion vector */
    CODE_USING_GOLDEN     = 0x5, /* 'Golden frame' prediction (no MV). */
    CODE_GOLDEN_MV        = 0x6, /* 'Golden frame' prediction plus MV. */
    CODE_INTER_FOURMV     = 0x7  /* Inter prediction 4MV per macro block. */
} CODING_MODE;

/** Huffman table entry */
typedef struct HUFF_ENTRY {
  struct HUFF_ENTRY *ZeroChild;
  struct HUFF_ENTRY *OneChild;
  struct HUFF_ENTRY *Previous;
  struct HUFF_ENTRY *Next;
  ogg_int32_t        Value;
  ogg_uint32_t       Frequency;

} HUFF_ENTRY;

typedef struct qmat_range_table {
  int startq, startqi; /* index where this range starts */
  Q_LIST_ENTRY *qmat;  /* qmat at this range boundary */
} qmat_range_table;

/** codec setup data, maps to the third bitstream header */
typedef struct codec_setup_info {
  ogg_uint32_t QThreshTable[Q_TABLE_SIZE];
  Q_LIST_ENTRY DcScaleFactorTable[Q_TABLE_SIZE];

  int MaxQMatrixIndex;
  Q_LIST_ENTRY *qmats;
  qmat_range_table *range_table[6];

  HUFF_ENTRY *HuffRoot[NUM_HUFF_TABLES];

} codec_setup_info;

/** Decoder (Playback) instance -- installed in a theora_state */
struct PB_INSTANCE {
  oggpack_buffer *opb;
  theora_info     info;

  /* flag to indicate if the headers already have been written */
  int            HeadersWritten;

  /* how far do we shift the granulepos to seperate out P frame counts? */
  int             keyframe_granule_shift;


  /***********************************************************************/
  /* Decoder and Frame Type Information */

  int           DecoderErrorCode;
  int           FramesHaveBeenSkipped;

  int           PostProcessEnabled;
  ogg_uint32_t  PostProcessingLevel;    /* Perform post processing */

  /* Frame Info */
  CODING_MODE   CodingMode;
  unsigned char FrameType;
  unsigned char KeyFrameType;
  ogg_uint32_t  QualitySetting;
  ogg_uint32_t  FrameQIndex;            /* Quality specified as a
                                           table index */
  ogg_uint32_t  ThisFrameQualityValue;  /* Quality value for this frame  */
  ogg_uint32_t  LastFrameQualityValue;  /* Last Frame's Quality */
  ogg_int32_t   CodedBlockIndex;        /* Number of Coded Blocks */
  ogg_uint32_t  CodedBlocksThisFrame;   /* Index into coded blocks */
  ogg_uint32_t  FrameSize;              /* The number of bytes in the frame. */

  /**********************************************************************/
  /* Frame Size & Index Information */

  ogg_uint32_t  YPlaneSize;
  ogg_uint32_t  UVPlaneSize;
  ogg_uint32_t  YStride;
  ogg_uint32_t  UVStride;
  ogg_uint32_t  VFragments;
  ogg_uint32_t  HFragments;
  ogg_uint32_t  UnitFragments;
  ogg_uint32_t  YPlaneFragments;
  ogg_uint32_t  UVPlaneFragments;

  ogg_uint32_t  ReconYPlaneSize;
  ogg_uint32_t  ReconUVPlaneSize;

  ogg_uint32_t  YDataOffset;
  ogg_uint32_t  UDataOffset;
  ogg_uint32_t  VDataOffset;
  ogg_uint32_t  ReconYDataOffset;
  ogg_uint32_t  ReconUDataOffset;
  ogg_uint32_t  ReconVDataOffset;
  ogg_uint32_t  YSuperBlocks;   /* Number of SuperBlocks in a Y frame */
  ogg_uint32_t  UVSuperBlocks;  /* Number of SuperBlocks in a U or V frame */
  ogg_uint32_t  SuperBlocks;    /* Total number of SuperBlocks in a
                                   Y,U,V frame */

  ogg_uint32_t  YSBRows;        /* Number of rows of SuperBlocks in a
                                   Y frame */
  ogg_uint32_t  YSBCols;        /* Number of cols of SuperBlocks in a
                                   Y frame */
  ogg_uint32_t  UVSBRows;       /* Number of rows of SuperBlocks in a
                                   U or V frame */
  ogg_uint32_t  UVSBCols;       /* Number of cols of SuperBlocks in a
                                   U or V frame */

  ogg_uint32_t  MacroBlocks;    /* Total number of Macro-Blocks */

  /**********************************************************************/
  /* Frames  */
  YUV_BUFFER_ENTRY *ThisFrameRecon;
  YUV_BUFFER_ENTRY *GoldenFrame;
  YUV_BUFFER_ENTRY *LastFrameRecon;
  YUV_BUFFER_ENTRY *PostProcessBuffer;

  /**********************************************************************/
  /* Fragment Information */
  ogg_uint32_t  *pixel_index_table;        /* start address of first
                                              pixel of fragment in
                                              source */
  ogg_uint32_t  *recon_pixel_index_table;  /* start address of first
                                              pixel in recon buffer */

  unsigned char *display_fragments;        /* Fragment update map */
  unsigned char *skipped_display_fragments;/* whether fragment YUV
                                              Conversion and update is to be
                                              skipped */
  ogg_int32_t   *CodedBlockList;           /* A list of fragment indices for
                                              coded blocks. */
  MOTION_VECTOR *FragMVect;                /* Frag motion vectors */

  ogg_uint32_t  *FragTokenCounts;          /* Number of tokens per fragment */
  ogg_uint32_t  (*TokenList)[128];         /* Fragment Token Pointers */

  ogg_int32_t   *FragmentVariances;
  ogg_uint32_t  *FragQIndex;               /* Fragment Quality used in
                                              PostProcess */
  Q_LIST_ENTRY (*PPCoefBuffer)[64];        /* PostProcess Buffer for
                                              coefficients data */

  unsigned char *FragCoeffs;                /* # of coeffs decoded so far for
                                               fragment */
  unsigned char *FragCoefEOB;               /* Position of last non 0 coef
                                                within QFragData */
  Q_LIST_ENTRY (*QFragData)[64];            /* Fragment Coefficients
                                               Array Pointers */
  CODING_MODE   *FragCodingMethod;          /* coding method for the
                                               fragment */

  /***********************************************************************/
  /* pointers to addresses used for allocation and deallocation the
      others are rounded up to the nearest 32 bytes */

  COEFFNODE     *_Nodes;
  ogg_uint32_t  *transIndex;                    /* ptr to table of
                                                   transposed indexes */

  /***********************************************************************/
  ogg_int32_t    bumpLast;

  /* Macro Block and SuperBlock Information */
  ogg_int32_t  (*BlockMap)[4][4];               /* super block + sub macro
                                                   block + sub frag ->
                                                   FragIndex */

  /* Coded flag arrays and counters for them */
  unsigned char *SBCodedFlags;
  unsigned char *SBFullyFlags;
  unsigned char *MBCodedFlags;
  unsigned char *MBFullyFlags;

  /**********************************************************************/
  ogg_uint32_t   EOB_Run;

  COORDINATE    *FragCoordinates;
  MOTION_VECTOR  MVector;
  ogg_int32_t    ReconPtr2Offset;       /* Offset for second reconstruction
                                           in half pixel MC */
  Q_LIST_ENTRY  *quantized_list;
  ogg_int16_t   *ReconDataBuffer;
  Q_LIST_ENTRY   InvLastIntraDC;
  Q_LIST_ENTRY   InvLastInterDC;
  Q_LIST_ENTRY   LastIntraDC;
  Q_LIST_ENTRY   LastInterDC;

  ogg_uint32_t   BlocksToDecode;        /* Blocks to be decoded this frame */
  ogg_uint32_t   DcHuffChoice;          /* Huffman table selection variables */
  unsigned char  ACHuffChoice;
  ogg_uint32_t   QuadMBListIndex;

  ogg_int32_t    ByteCount;

  ogg_uint32_t   bit_pattern;
  unsigned char  bits_so_far;
  unsigned char  NextBit;
  ogg_int32_t    BitsLeft;

  ogg_int16_t   *DequantBuffer;

  ogg_int32_t    fp_quant_InterUV_coeffs[64];
  ogg_int32_t    fp_quant_InterUV_round[64];
  ogg_int32_t    fp_ZeroBinSize_InterUV[64];

  ogg_int16_t   *TmpReconBuffer;
  ogg_int16_t   *TmpDataBuffer;

  /* Loop filter bounding values */
  ogg_int16_t    FiltBoundingValue[256];

  /* Naming convention for all quant matrices and related data structures:
   * Fields containing "Inter" in their name are for Inter frames, the
   * rest is Intra. */

  /* Dequantiser and rounding tables */
  ogg_uint16_t   *QThreshTable;
  Q_LIST_ENTRY  dequant_Y_coeffs[64];
  Q_LIST_ENTRY  dequant_U_coeffs[64];
  Q_LIST_ENTRY  dequant_V_coeffs[64];
  Q_LIST_ENTRY  dequant_InterY_coeffs[64];
  Q_LIST_ENTRY  dequant_InterU_coeffs[64];
  Q_LIST_ENTRY  dequant_InterV_coeffs[64];

  Q_LIST_ENTRY  *dequant_coeffs;        /* currently active quantizer */
  unsigned int   zigzag_index[64];

  HUFF_ENTRY    *HuffRoot_VP3x[NUM_HUFF_TABLES];
  ogg_uint32_t  *HuffCodeArray_VP3x[NUM_HUFF_TABLES];
  unsigned char *HuffCodeLengthArray_VP3x[NUM_HUFF_TABLES];
  const unsigned char *ExtraBitLengths_VP3x;

  th_quant_info   quant_info;
  oc_quant_tables quant_tables[2][3];

  /* Quantiser and rounding tables */
  /* this is scheduled to be replaced a new mechanism
     that will simply reuse the dequantizer information. */
  ogg_int32_t    fp_quant_Y_coeffs[64]; /* used in reiniting quantizers */
  ogg_int32_t    fp_quant_U_coeffs[64];
  ogg_int32_t    fp_quant_V_coeffs[64];
  ogg_int32_t    fp_quant_Inter_Y_coeffs[64];
  ogg_int32_t    fp_quant_Inter_U_coeffs[64];
  ogg_int32_t    fp_quant_Inter_V_coeffs[64];

  ogg_int32_t    fp_quant_Y_round[64];
  ogg_int32_t    fp_quant_U_round[64];
  ogg_int32_t    fp_quant_V_round[64];
  ogg_int32_t    fp_quant_Inter_Y_round[64];
  ogg_int32_t    fp_quant_Inter_U_round[64];
  ogg_int32_t    fp_quant_Inter_V_round[64];

  ogg_int32_t    fp_ZeroBinSize_Y[64];
  ogg_int32_t    fp_ZeroBinSize_U[64];
  ogg_int32_t    fp_ZeroBinSize_V[64];
  ogg_int32_t    fp_ZeroBinSize_Inter_Y[64];
  ogg_int32_t    fp_ZeroBinSize_Inter_U[64];
  ogg_int32_t    fp_ZeroBinSize_Inter_V[64];

  ogg_int32_t   *fquant_coeffs;
  ogg_int32_t   *fquant_round;
  ogg_int32_t   *fquant_ZbSize;

  /* Predictor used in choosing entropy table for decoding block patterns. */
  unsigned char  BlockPatternPredictor;

  short          Modifier[4][512];
  short         *ModifierPointer[4];

  unsigned char *DataOutputInPtr;

  DspFunctions   dsp;  /* Selected functions for this platform */

};

/* Encoder (Compressor) instance -- installed in a theora_state */
typedef struct CP_INSTANCE {
  /*This structure must be first.
    It contains entry points accessed by the decoder library's API wrapper, and
     is the only assumption that library makes about our internal format.*/
  oc_state_dispatch_vtbl dispatch_vtbl;

  /* Compressor Configuration */
  SCAN_CONFIG_DATA ScanConfig;
  CONFIG_TYPE2     Configuration;
  int              GoldenFrameEnabled;
  int              InterPrediction;
  int              MotionCompensation;

  ogg_uint32_t     LastKeyFrame ;
  ogg_int32_t      DropCount ;
  ogg_int32_t      MaxConsDroppedFrames ;
  ogg_int32_t      DropFrameTriggerBytes;
  int              DropFrameCandidate;

  /* Compressor Statistics */
  double           TotErrScore;
  ogg_int64_t      KeyFrameCount; /* Count of key frames. */
  ogg_int64_t      TotKeyFrameBytes;
  ogg_uint32_t     LastKeyFrameSize;
  ogg_uint32_t     PriorKeyFrameSize[KEY_FRAME_CONTEXT];
  ogg_uint32_t     PriorKeyFrameDistance[KEY_FRAME_CONTEXT];
  ogg_int32_t      FrameQuality[6];
  int              DecoderErrorCode; /* Decoder error flag. */
  ogg_int32_t      ThreshMapThreshold;
  ogg_int32_t      TotalMotionScore;
  ogg_int64_t      TotalByteCount;
  ogg_int32_t      FixedQ;

  /* Frame Statistics  */
  signed char      InterCodeCount;
  ogg_int64_t      CurrentFrame;
  ogg_int64_t      CarryOver ;
  ogg_uint32_t     LastFrameSize;
  ogg_uint32_t     FrameBitCount;
  int              ThisIsFirstFrame;
  int              ThisIsKeyFrame;

  ogg_int32_t      MotionScore;
  ogg_uint32_t     RegulationBlocks;
  ogg_int32_t      RecoveryMotionScore;
  int              RecoveryBlocksAdded ;
  double           ProportionRecBlocks;
  double           MaxRecFactor ;

  /* Rate Targeting variables. */
  ogg_uint32_t     ThisFrameTargetBytes;
  double           BpbCorrectionFactor;

  /* Up regulation variables */
  ogg_uint32_t     FinalPassLastPos;  /* Used to regulate a final
                                         unrestricted high quality
                                         pass. */
  ogg_uint32_t     LastEndSB;         /* Where we were in the loop
                                         last time. */
  ogg_uint32_t     ResidueLastEndSB;  /* Where we were in the residue
                                         update loop last time. */

  /* Controlling Block Selection */
  ogg_uint32_t     MVChangeFactor;
  ogg_uint32_t     FourMvChangeFactor;
  ogg_uint32_t     MinImprovementForNewMV;
  ogg_uint32_t     ExhaustiveSearchThresh;
  ogg_uint32_t     MinImprovementForFourMV;
  ogg_uint32_t     FourMVThreshold;

  /* Module shared data structures. */
  ogg_int32_t      frame_target_rate;
  ogg_int32_t      BaseLineFrameTargetRate;
  ogg_int32_t      min_blocks_per_frame;
  ogg_uint32_t     tot_bytes_old;

  /*********************************************************************/
  /* Frames  Used in the selecetive convolution filtering of the Y plane. */
  unsigned char    *ConvDestBuffer;
  YUV_BUFFER_ENTRY *yuv0ptr;
  YUV_BUFFER_ENTRY *yuv1ptr;
  /*********************************************************************/

  /*********************************************************************/
  /* Token Buffers */
  ogg_uint32_t     *OptimisedTokenListEb; /* Optimised token list extra bits */
  unsigned char    *OptimisedTokenList;   /* Optimised token list. */
  unsigned char    *OptimisedTokenListHi; /* Optimised token list huffman
                                             table index */

  unsigned char    *OptimisedTokenListPl; /* Plane to which the token
                                             belongs Y = 0 or UV = 1 */
  ogg_int32_t       OptimisedTokenCount;           /* Count of Optimized tokens */
  ogg_uint32_t      RunHuffIndex;         /* Huffman table in force at
                                             the start of a run */
  ogg_uint32_t      RunPlaneIndex;        /* The plane (Y=0 UV=1) to
                                             which the first token in
                                             an EOB run belonged. */


  ogg_uint32_t      TotTokenCount;
  ogg_int32_t       TokensToBeCoded;
  ogg_int32_t       TokensCoded;
  /********************************************************************/

  /* SuperBlock, MacroBLock and Fragment Information */
  /* Coded flag arrays and counters for them */
  unsigned char    *PartiallyCodedFlags;
  unsigned char    *PartiallyCodedMbPatterns;
  unsigned char    *UncodedMbFlags;

  unsigned char    *extra_fragments;   /* extra updates not
                                          recommended by pre-processor */
  ogg_int16_t      *OriginalDC;

  ogg_uint32_t     *FragmentLastQ;     /* Array used to keep track of
                                          quality at which each
                                          fragment was last
                                          updated. */
  unsigned char    *FragTokens;
  ogg_uint32_t     *FragTokenCounts;   /* Number of tokens per fragment */

  ogg_uint32_t     *RunHuffIndices;
  ogg_uint32_t     *LastCodedErrorScore;
  ogg_uint32_t     *ModeList;
  MOTION_VECTOR    *MVList;

  unsigned char    *BlockCodedFlags;

  ogg_uint32_t      MvListCount;
  ogg_uint32_t      ModeListCount;


  unsigned char    *DataOutputBuffer;
  /*********************************************************************/

  ogg_uint32_t      RunLength;
  ogg_uint32_t      MaxBitTarget;     /* Cut off target for rate capping */
  double            BitRateCapFactor; /* Factor relating delta frame target
                                         to cut off target. */

  unsigned char     MBCodingMode;     /* Coding mode flags */

  ogg_int32_t       MVPixelOffsetY[MAX_SEARCH_SITES];
  ogg_uint32_t      InterTripOutThresh;
  unsigned char     MVEnabled;
  ogg_uint32_t      MotionVectorSearchCount;
  ogg_uint32_t      FrameMVSearcOunt;
  ogg_int32_t       MVSearchSteps;
  ogg_int32_t       MVOffsetX[MAX_SEARCH_SITES];
  ogg_int32_t       MVOffsetY[MAX_SEARCH_SITES];
  ogg_int32_t       HalfPixelRef2Offset[9]; /* Offsets for half pixel
                                               compensation */
  signed char       HalfPixelXOffset[9];    /* Half pixel MV offsets for X */
  signed char       HalfPixelYOffset[9];    /* Half pixel MV offsets for Y */

  ogg_uint32_t      bit_pattern ;
  unsigned char     bits_so_far ;
  ogg_uint32_t      lastval ;
  ogg_uint32_t      lastrun ;

  Q_LIST_ENTRY     *quantized_list;

  MOTION_VECTOR     MVector;
  ogg_uint32_t      TempBitCount;
  ogg_int16_t      *DCT_codes; /* Buffer that stores the result of
                                  Forward DCT */
  ogg_int16_t      *DCTDataBuffer; /* Input data buffer for Forward DCT */

  /* Motion compensation related variables */
  ogg_uint32_t      MvMaxExtent;

  double            QTargetModifier[Q_TABLE_SIZE];

  /* instances (used for reconstructing buffers and to hold tokens etc.) */
  PP_INSTANCE       pp;   /* preprocessor */
  PB_INSTANCE       pb;   /* playback */

  /* ogg bitpacker for use in packet coding, other API state */
  oggpack_buffer   *oggbuffer;
  int               readyflag;
  int               packetflag;
  int               doneflag;

  DspFunctions   dsp;  /* Selected functions for this platform */

} CP_INSTANCE;

#define clamp255(x) ((unsigned char)((((x)<0)-1) & ((x) | -((x)>255))))

extern void ConfigurePP( PP_INSTANCE *ppi, int Level ) ;
extern ogg_uint32_t YUVAnalyseFrame( PP_INSTANCE *ppi,
                                     ogg_uint32_t * KFIndicator );

extern void ClearPPInstance(PP_INSTANCE *ppi);
extern void InitPPInstance(PP_INSTANCE *ppi, DspFunctions *funcs);
extern void InitPBInstance(PB_INSTANCE *pbi);
extern void ClearPBInstance(PB_INSTANCE *pbi);

extern void IDct1( Q_LIST_ENTRY * InputData,
                   ogg_int16_t *QuantMatrix,
                   ogg_int16_t * OutputData );

extern void ReconIntra( PB_INSTANCE *pbi, unsigned char * ReconPtr,
                        ogg_int16_t * ChangePtr, ogg_uint32_t LineStep );

extern void ReconInter( PB_INSTANCE *pbi, unsigned char * ReconPtr,
                        unsigned char * RefPtr, ogg_int16_t * ChangePtr,
                        ogg_uint32_t LineStep ) ;

extern void ReconInterHalfPixel2( PB_INSTANCE *pbi, unsigned char * ReconPtr,
                                  unsigned char * RefPtr1,
                                  unsigned char * RefPtr2,
                                  ogg_int16_t * ChangePtr,
                                  ogg_uint32_t LineStep ) ;

extern void SetupLoopFilter(PB_INSTANCE *pbi);
extern void CopyBlock(unsigned char *src,
                      unsigned char *dest,
                      unsigned int srcstride);
extern void LoopFilter(PB_INSTANCE *pbi);
extern void ReconRefFrames (PB_INSTANCE *pbi);
extern void ExpandToken( Q_LIST_ENTRY * ExpandedBlock,
                         unsigned char * CoeffIndex, ogg_uint32_t Token,
                         ogg_int32_t ExtraBits );
extern void ClearDownQFragData(PB_INSTANCE *pbi);

extern void select_quantiser (PB_INSTANCE *pbi, int type);

extern void quantize( PB_INSTANCE *pbi,
                      ogg_int16_t * DCT_block,
                      Q_LIST_ENTRY * quantized_list);
extern void UpdateQ( PB_INSTANCE *pbi, int NewQIndex );
extern void UpdateQC( CP_INSTANCE *cpi, ogg_uint32_t NewQ );
extern void fdct_short ( ogg_int16_t * InputData, ogg_int16_t * OutputData );
extern ogg_uint32_t DPCMTokenizeBlock (CP_INSTANCE *cpi,
                                       ogg_int32_t FragIndex);
extern void TransformQuantizeBlock (CP_INSTANCE *cpi, ogg_int32_t FragIndex,
                                    ogg_uint32_t PixelsPerLine ) ;
extern void ClearFragmentInfo(PB_INSTANCE * pbi);
extern void InitFragmentInfo(PB_INSTANCE * pbi);
extern void ClearFrameInfo(PB_INSTANCE * pbi);
extern void InitFrameInfo(PB_INSTANCE * pbi, unsigned int FrameSize);
extern void InitializeFragCoordinates(PB_INSTANCE *pbi);
extern void InitFrameDetails(PB_INSTANCE *pbi);
extern void WriteQTables(PB_INSTANCE *pbi,oggpack_buffer *opb);
extern void InitQTables( PB_INSTANCE *pbi );
extern void quant_tables_init( PB_INSTANCE *pbi, const th_quant_info *qinfo);
extern void InitHuffmanSet( PB_INSTANCE *pbi );
extern void ClearHuffmanSet( PB_INSTANCE *pbi );
extern int  ReadHuffmanTrees(codec_setup_info *ci, oggpack_buffer *opb);
extern void WriteHuffmanTrees(HUFF_ENTRY *HuffRoot[NUM_HUFF_TABLES],
                              oggpack_buffer *opb);
extern void InitHuffmanTrees(PB_INSTANCE *pbi, const codec_setup_info *ci);
extern void ClearHuffmanTrees(HUFF_ENTRY *HuffRoot[NUM_HUFF_TABLES]);
extern int  ReadFilterTables(codec_setup_info *ci, oggpack_buffer *opb);
extern void QuadDecodeDisplayFragments ( PB_INSTANCE *pbi );
extern void PackAndWriteDFArray( CP_INSTANCE *cpi );
extern void UpdateFragQIndex(PB_INSTANCE *pbi);
extern void PostProcess(PB_INSTANCE *pbi);
extern void InitMotionCompensation ( CP_INSTANCE *cpi );
extern ogg_uint32_t GetMBIntraError (CP_INSTANCE *cpi, ogg_uint32_t FragIndex,
                                     ogg_uint32_t PixelsPerLine ) ;
extern ogg_uint32_t GetMBInterError (CP_INSTANCE *cpi,
                                     unsigned char * SrcPtr,
                                     unsigned char * RefPtr,
                                     ogg_uint32_t FragIndex,
                                     ogg_int32_t LastXMV,
                                     ogg_int32_t LastYMV,
                                     ogg_uint32_t PixelsPerLine ) ;
extern void WriteFrameHeader( CP_INSTANCE *cpi) ;
extern ogg_uint32_t GetMBMVInterError (CP_INSTANCE *cpi,
                                       unsigned char * RefFramePtr,
                                       ogg_uint32_t FragIndex,
                                       ogg_uint32_t PixelsPerLine,
                                       ogg_int32_t *MVPixelOffset,
                                       MOTION_VECTOR *MV );
extern ogg_uint32_t GetMBMVExhaustiveSearch (CP_INSTANCE *cpi,
                                             unsigned char * RefFramePtr,
                                             ogg_uint32_t FragIndex,
                                             ogg_uint32_t PixelsPerLine,
                                             MOTION_VECTOR *MV );
extern ogg_uint32_t GetFOURMVExhaustiveSearch (CP_INSTANCE *cpi,
                                               unsigned char * RefFramePtr,
                                               ogg_uint32_t FragIndex,
                                               ogg_uint32_t PixelsPerLine,
                                               MOTION_VECTOR *MV ) ;
extern ogg_uint32_t EncodeData(CP_INSTANCE *cpi);
extern ogg_uint32_t PickIntra( CP_INSTANCE *cpi,
                               ogg_uint32_t SBRows,
                               ogg_uint32_t SBCols);
extern ogg_uint32_t PickModes(CP_INSTANCE *cpi,
                              ogg_uint32_t SBRows,
                              ogg_uint32_t SBCols,
                              ogg_uint32_t PixelsPerLine,
                              ogg_uint32_t *InterError,
                              ogg_uint32_t *IntraError);

extern CODING_MODE FrArrayUnpackMode(PB_INSTANCE *pbi);
extern void CreateBlockMapping ( ogg_int32_t  (*BlockMap)[4][4],
                                 ogg_uint32_t YSuperBlocks,
                                 ogg_uint32_t UVSuperBlocks,
                                 ogg_uint32_t HFrags, ogg_uint32_t VFrags );
extern void UpRegulateDataStream (CP_INSTANCE *cpi, ogg_uint32_t RegulationQ,
                                  ogg_int32_t RecoveryBlocks ) ;
extern void RegulateQ( CP_INSTANCE *cpi, ogg_int32_t UpdateScore );
extern void CopyBackExtraFrags(CP_INSTANCE *cpi);

extern void UpdateUMVBorder( PB_INSTANCE *pbi,
                             unsigned char * DestReconPtr );
extern void PInitFrameInfo(PP_INSTANCE * ppi);

extern double GetEstimatedBpb( CP_INSTANCE *cpi, ogg_uint32_t TargetQ );
extern void ClearTmpBuffers(PB_INSTANCE * pbi);
extern void InitTmpBuffers(PB_INSTANCE * pbi);
extern void ScanYUVInit( PP_INSTANCE *  ppi,
                         SCAN_CONFIG_DATA * ScanConfigPtr);

#endif /* ENCODER_INTERNAL_H */
