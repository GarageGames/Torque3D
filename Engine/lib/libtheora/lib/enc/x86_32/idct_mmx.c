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
  last mod: $Id: idct_mmx.c 15153 2008-08-04 18:37:55Z tterribe $

 ********************************************************************/

#include "../codec_internal.h"

#if defined(USE_ASM)

#define ASM asm

/****************************************************************************
*
*   Description  :     IDCT with multiple versions based on # of non 0 coeffs
*
*****************************************************************************
*/

// Dequantization + inverse discrete cosine transform.

// Constants used in MMX implementation of dequantization and idct.
// All the MMX stuff works with 4 16-bit quantities at a time and
// we create 11 constants of size 4 x 16 bits.
// The first 4 are used to mask the individual 16-bit words within a group
// and are used in the address-shuffling part of the dequantization.
// The last 7 are fixed-point approximations to the cosines of angles
// occurring in the DCT; each of these contains 4 copies of the same value.

// There is only one (statically initialized) instance of this object
// wrapped in an allocator object that forces its starting address
// to be evenly divisible by 32.  Hence the actual object occupies 2.75
// cache lines on a Pentium processor.

// Offsets in bytes used by the assembler code below
// must of course agree with the idctConstants constructor.

#define MaskOffset 0        // 4 masks come in order low word to high
#define CosineOffset 32     // 7 cosines come in order pi/16 * (1 ... 7)
#define EightOffset 88
#define IdctAdjustBeforeShift 8

/*
UINT16 idctcosTbl[ 7] =
{
    64277, 60547, 54491, 46341, 36410, 25080, 12785
};

void fillidctconstants(void)
{
    int j = 16;
    UINT16 * p;
    do
    {
        idctconstants[ --j] = 0;
    }
    while( j);

    idctconstants[0] = idctconstants[5] = idctconstants[10] = idctconstants[15] = 65535;

    j = 1;
    do
    {
        p = idctconstants + ( (j+3) << 2);
        p[0] = p[1] = p[2] = p[3] = idctcosTbl[ j - 1];
    }
    while( ++j <= 7);

    idctconstants[44] = idctconstants[45] = idctconstants[46] = idctconstants[47] = IdctAdjustBeforeShift;
}
*/

ogg_uint16_t idctconstants[(4+7+1) * 4] = {
    65535,     0,     0,     0,     0, 65535,     0,     0,
        0,     0, 65535,     0,     0,     0,     0, 65535,
    64277, 64277, 64277, 64277, 60547, 60547, 60547, 60547,
    54491, 54491, 54491, 54491, 46341, 46341, 46341, 46341,
    36410, 36410, 36410, 36410, 25080, 25080, 25080, 25080,
    12785, 12785, 12785, 12785,     8,     8,     8,     8,
};

/* Dequantization + inverse DCT.

   Dequantization multiplies user's 16-bit signed indices (range -512 to +511)
   by unsigned 16-bit quantization table entries.
   These table entries are upscaled by 4, max is 30 * 128 * 4 < 2^14.
   Result is scaled signed DCT coefficients (abs value < 2^15).

   In the data stream, the coefficients are sent in order of increasing
   total (horizontal + vertical) frequency.  The exact picture is as follows:

    00 01 05 06  16 17 33 34
    02 04 07 15  20 32 35 52
    03 10 14 21  31 36 51 53
    11 13 22 30  37 50 54 65

    12 23 27 40  47 55 64 66
    24 26 41 46  56 63 67 74
    25 42 45 57  62 70 73 75
    43 44 60 61  71 72 76 77

   Here the position in the matrix corresponds to the (horiz,vert)
   freqency indices and the octal entry in the matrix is the position
   of the coefficient in the data stream.  Thus the coefficients are sent
   in sort of a diagonal "snake".

   The dequantization stage "uncurls the snake" and stores the expanded
   coefficients in more convenient positions.  These are not exactly the
   natural positions given above but take into account our implementation
   of the idct, which basically requires two one-dimensional idcts and
   two transposes.

   We fold the first transpose into the storage of the expanded coefficients.
   We don't actually do a full transpose because this would require doubling
   the size of the idct buffer; rather, we just transpose each of the 4x4
   subblocks.  Using slightly varying addressing schemes in each of the
   four 4x8 idcts then allows these transforms to be done in place.

   Transposing the 4x4 subblocks in the matrix above gives

    00 02 03 11  16 20 31 37
    01 04 10 13  17 32 36 50
    05 07 14 22  33 35 51 54
    06 15 21 30  34 52 53 65

    12 24 25 43  47 56 62 71
    23 26 42 44  55 63 70 72
    27 41 45 60  64 67 73 76
    40 46 57 61  66 74 75 77

   Finally, we reverse the words in each 4 word group to clarify
   direction of shifts.

    11 03 02 00  37 31 20 16
    13 10 04 01  50 36 32 17
    22 14 07 05  54 51 35 33
    30 21 15 06  65 53 52 34

    43 25 24 12  71 62 56 47
    44 42 26 23  72 70 63 55
    60 45 41 27  76 73 67 64
    61 57 46 40  77 75 74 66

   This matrix then shows the 16 4x16 destination words in terms of
   the 16 4x16 input words.

   We implement this algorithm by manipulation of mmx registers,
   which seems to be the fastest way to proceed.  It is completely
   hand-written; there does not seem to be enough recurrence to
   reasonably compartmentalize any of it.  Hence the resulting
   program is ugly and bloated.  Furthermore, due to the absence of
   register pressure, it is boring and artless.  I hate it.

   The idct itself is more interesting.  Since the two-dimensional dct
   basis functions are products of the one-dimesional dct basis functions,
   we can compute an inverse (or forward) dct via two 1-D transforms,
   on rows then on columns.  To exploit MMX parallelism, we actually do
   both operations on columns, interposing a (partial) transpose between
   the two 1-D transforms, the first transpose being done by the expansion
   described above.

   The 8-sample one-dimensional DCT is a standard orthogonal expansion using
   the (unnormalized) basis functions

    b[k]( i) = cos( pi * k * (2i + 1) / 16);

   here k = 0 ... 7 is the frequency and i = 0 ... 7 is the spatial coordinate.
   To normalize, b[0] should be multiplied by 1/sqrt( 8) and the other b[k]
   should be multiplied by 1/2.

   The 8x8 two-dimensional DCT is just the product of one-dimensional DCTs
   in each direction.  The (unnormalized) basis functions are

    B[k,l]( i, j) = b[k]( i) * b[l]( j);

   this time k and l are the horizontal and vertical frequencies,
   i and j are the horizontal and vertical spatial coordinates;
   all indices vary from 0 ... 7 (as above)
   and there are now 4 cases of normalization.

   Our 1-D idct expansion uses constants C1 ... C7 given by

    (*)  Ck = C(-k) = cos( pi * k/16) = S(8-k) = -S(k-8) = sin( pi * (8-k)/16)

   and the following 1-D algorithm transforming I0 ... I7  to  R0 ... R7 :

   A = (C1 * I1) + (C7 * I7)        B = (C7 * I1) - (C1 * I7)
   C = (C3 * I3) + (C5 * I5)        D = (C3 * I5) - (C5 * I3)
   A. = C4 * (A - C)                B. = C4 * (B - D)
   C. = A + C                       D. = B + D

   E = C4 * (I0 + I4)               F = C4 * (I0 - I4)
   G = (C2 * I2) + (C6 * I6)        H = (C6 * I2) - (C2 * I6)
   E. = E - G
   G. = E + G

   A.. = F + A.                 B.. = B. - H
   F.  = F - A.                 H.  = B. + H

   R0 = G. + C. R1 = A.. + H.   R3 = E. + D.    R5 = F. + B..
   R7 = G. - C. R2 = A.. - H.   R4 = E. - D.    R6 = F. - B..

   It is due to Vetterli and Lightenberg and may be found in the JPEG
   reference book by Pennebaker and Mitchell.

   Correctness of the algorithm follows from (*) together with the
   addition formulas for sine and cosine:

    cos( A + B) = cos( A) * cos( B)  -  sin( A) * sin( B)
    sin( A + B) = sin( A) * cos( B)  +  cos( A) * sin( B)

   Note that this implementation absorbs the difference in normalization
   between the 0th and higher frequencies, although the results produced
   are actually twice as big as they should be.  Since we do this for each
   dimension, the 2-D idct results are 4x the desired results.  Finally,
   taking into account that the dequantization multiplies by 4 as well,
   our actual results are 16x too big.  We fix this by shifting the final
   results right by 4 bits.

   High precision version approximates C1 ... C7 to 16 bits.
   Since MMX only provides a signed multiply, C1 ... C5 appear to be
   negative and multiplies involving them must be adjusted to compensate
   for this.  C6 and C7 do not require this adjustment since
   they are < 1/2 and are correctly treated as positive numbers.

   Following macro does four 8-sample one-dimensional idcts in parallel.
   This is actually not such a difficult program to write once you
   make a couple of observations (I of course was unable to make these
   observations until I'd half-written a couple of other versions).

    1. Everything is easy once you are done with the multiplies.
       This is because, given X and Y in registers, one may easily
       calculate X+Y and X-Y using just those 2 registers.

    2. You always need at least 2 extra registers to calculate products,
       so storing 2 temporaries is inevitable.  C. and D. seem to be
       the best candidates.

    3. The products should be calculated in decreasing order of complexity
       (which translates into register pressure).  Since C1 ... C5 require
       adjustment (and C6, C7 do not), we begin by calculating C and D.
*/

/**************************************************************************************
 *
 *      Routine:        BeginIDCT
 *
 *      Description:    The Macro does IDct on 4 1-D Dcts
 *
 *      Input:          None
 *
 *      Output:         None
 *
 *      Return:         None
 *
 *      Special Note:   None
 *
 *      Error:          None
 *
 ***************************************************************************************
 */

#define MtoSTR(s) #s

#define Dump    "call MMX_dump\n"

#define BeginIDCT "#BeginIDCT\n"    \
                                    \
    "   movq    "   I(3)","r2"\n"   \
                                    \
    "   movq    "   C(3)","r6"\n"   \
    "   movq    "   r2","r4"\n"     \
    "   movq    "   J(5)","r7"\n"   \
    "   pmulhw  "   r6","r4"\n"     \
    "   movq    "   C(5)","r1"\n"   \
    "   pmulhw  "   r7","r6"\n"     \
    "   movq    "   r1","r5"\n"     \
    "   pmulhw  "   r2","r1"\n"     \
    "   movq    "   I(1)","r3"\n"   \
    "   pmulhw  "   r7","r5"\n"     \
    "   movq    "   C(1)","r0"\n"   \
    "   paddw   "   r2","r4"\n"     \
    "   paddw   "   r7","r6"\n"     \
    "   paddw   "   r1","r2"\n"     \
    "   movq    "   J(7)","r1"\n"   \
    "   paddw   "   r5","r7"\n"     \
    "   movq    "   r0","r5"\n"     \
    "   pmulhw  "   r3","r0"\n"     \
    "   paddsw  "   r7","r4"\n"     \
    "   pmulhw  "   r1","r5"\n"     \
    "   movq    "   C(7)","r7"\n"   \
    "   psubsw  "   r2","r6"\n"     \
    "   paddw   "   r3","r0"\n"     \
    "   pmulhw  "   r7","r3"\n"     \
    "   movq    "   I(2)","r2"\n"   \
    "   pmulhw  "   r1","r7"\n"     \
    "   paddw   "   r1","r5"\n"     \
    "   movq    "   r2","r1"\n"     \
    "   pmulhw  "   C(2)","r2"\n"   \
    "   psubsw  "   r5","r3"\n"     \
    "   movq    "   J(6)","r5"\n"   \
    "   paddsw  "   r7","r0"\n"     \
    "   movq    "   r5","r7"\n"     \
    "   psubsw  "   r4","r0"\n"     \
    "   pmulhw  "   C(2)","r5"\n"   \
    "   paddw   "   r1","r2"\n"     \
    "   pmulhw  "   C(6)","r1"\n"   \
    "   paddsw  "   r4","r4"\n"     \
    "   paddsw  "   r0","r4"\n"     \
    "   psubsw  "   r6","r3"\n"     \
    "   paddw   "   r7","r5"\n"     \
    "   paddsw  "   r6","r6"\n"     \
    "   pmulhw  "   C(6)","r7"\n"   \
    "   paddsw  "   r3","r6"\n"     \
    "   movq    "   r4","I(1)"\n"   \
    "   psubsw  "   r5","r1"\n"     \
    "   movq    "   C(4)","r4"\n"   \
    "   movq    "   r3","r5"\n"     \
    "   pmulhw  "   r4","r3"\n"     \
    "   paddsw  "   r2","r7"\n"     \
    "   movq    "   r6","I(2)"\n"   \
    "   movq    "   r0","r2"\n"     \
    "   movq    "   I(0)","r6"\n"   \
    "   pmulhw  "   r4","r0"\n"     \
    "   paddw   "   r3","r5"\n"     \
    "\n"                            \
    "   movq    "   J(4)","r3"\n"   \
    "   psubsw  "   r1","r5"\n"     \
    "   paddw   "   r0","r2"\n"     \
    "   psubsw  "   r3","r6"\n"     \
    "   movq    "   r6","r0"\n"     \
    "   pmulhw  "   r4","r6"\n"     \
    "   paddsw  "   r3","r3"\n"     \
    "   paddsw  "   r1","r1"\n"     \
    "   paddsw  "   r0","r3"\n"     \
    "   paddsw  "   r5","r1"\n"     \
    "   pmulhw  "   r3","r4"\n"     \
    "   paddsw  "   r0","r6"\n"     \
    "   psubsw  "   r2","r6"\n"     \
    "   paddsw  "   r2","r2"\n"     \
    "   movq    "   I(1)","r0"\n"   \
    "   paddsw  "   r6","r2"\n"     \
    "   paddw   "   r3","r4"\n"     \
    "   psubsw  "   r1","r2"\n"     \
    "#end BeginIDCT\n"
// end BeginIDCT macro (38 cycles).


// Two versions of the end of the idct depending on whether we're feeding
// into a transpose or dividing the final results by 16 and storing them.

/**************************************************************************************
 *
 *      Routine:        RowIDCT
 *
 *      Description:    The Macro does 1-D IDct on 4 Rows
 *
 *      Input:          None
 *
 *      Output:         None
 *
 *      Return:         None
 *
 *      Special Note:   None
 *
 *      Error:          None
 *
 ***************************************************************************************
 */

// RowIDCT gets ready to transpose.

#define RowIDCT ASM("\n"\
    "#RowIDCT\n"                                        \
    BeginIDCT                                           \
    "\n"                                                \
    "   movq    "I(2)","r3"\n"  /* r3 = D. */           \
    "   psubsw  "r7","r4"\n"    /* r4 = E. = E - G */   \
    "   paddsw  "r1","r1"\n"    /* r1 = H. + H. */      \
    "   paddsw  "r7","r7"\n"    /* r7 = G + G */        \
    "   paddsw  "r2","r1"\n"    /* r1 = R1 = A.. + H. */\
    "   paddsw  "r4","r7"\n"    /* r7 = G. = E + G */   \
    "   psubsw  "r3","r4"\n"    /* r4 = R4 = E. - D. */ \
    "   paddsw  "r3","r3"\n"                            \
    "   psubsw  "r5","r6"\n"    /* r6 = R6 = F. - B.. */\
    "   paddsw  "r5","r5"\n"                            \
    "   paddsw  "r4","r3"\n"    /* r3 = R3 = E. + D. */ \
    "   paddsw  "r6","r5"\n"    /* r5 = R5 = F. + B.. */\
    "   psubsw  "r0","r7"\n"    /* r7 = R7 = G. - C. */ \
    "   paddsw  "r0","r0"\n"                            \
    "   movq    "r1","I(1)"\n"  /* save R1 */           \
    "   paddsw  "r7","r0"\n"    /* r0 = R0 = G. + C. */ \
    "#end RowIDCT"                                                                              \
);
// end RowIDCT macro (8 + 38 = 46 cycles)


/**************************************************************************************
 *
 *      Routine:        ColumnIDCT
 *
 *      Description:    The Macro does 1-D IDct on 4 columns
 *
 *      Input:          None
 *
 *      Output:         None
 *
 *      Return:         None
 *
 *      Special Note:   None
 *
 *      Error:          None
 *
 ***************************************************************************************
 */
// Column IDCT normalizes and stores final results.

#define ColumnIDCT ASM("\n"                                 \
    "#ColumnIDCT\n"                                         \
    BeginIDCT                                               \
    "\n"                                                    \
    "   paddsw  "Eight","r2"\n"                             \
    "   paddsw  "r1","r1"\n"        /* r1 = H. + H. */      \
    "   paddsw  "r2","r1"\n"        /* r1 = R1 = A.. + H. */\
    "   psraw   ""$4"","r2"\n"      /* r2 = NR2 */          \
    "   psubsw  "r7","r4"\n"        /* r4 = E. = E - G */   \
    "   psraw   ""$4"","r1"\n"      /* r1 = NR1 */          \
    "   movq    "I(2)","r3"\n"  /* r3 = D. */               \
    "   paddsw  "r7","r7"\n"        /* r7 = G + G */        \
    "   movq    "r2","I(2)"\n"  /* store NR2 at I2 */       \
    "   paddsw  "r4","r7"\n"        /* r7 = G. = E + G */   \
    "   movq    "r1","I(1)"\n"  /* store NR1 at I1 */       \
    "   psubsw  "r3","r4"\n"        /* r4 = R4 = E. - D. */ \
    "   paddsw  "Eight","r4"\n"                             \
    "   paddsw  "r3","r3"\n"        /* r3 = D. + D. */      \
    "   paddsw  "r4","r3"\n"        /* r3 = R3 = E. + D. */ \
    "   psraw   ""$4"","r4"\n"      /* r4 = NR4 */          \
    "   psubsw  "r5","r6"\n"        /* r6 = R6 = F. - B.. */\
    "   psraw   ""$4"","r3"\n"      /* r3 = NR3 */          \
    "   paddsw  "Eight","r6"\n"                             \
    "   paddsw  "r5","r5"\n"        /* r5 = B.. + B.. */    \
    "   paddsw  "r6","r5"\n"        /* r5 = R5 = F. + B.. */\
    "   psraw   ""$4"","r6"\n"      /* r6 = NR6 */          \
    "   movq    "r4","J(4)"\n"  /* store NR4 at J4 */       \
    "   psraw   ""$4"","r5"\n"      /* r5 = NR5 */          \
    "   movq    "r3","I(3)"\n"  /* store NR3 at I3 */       \
    "   psubsw  "r0","r7"\n"        /* r7 = R7 = G. - C. */ \
    "   paddsw  "Eight","r7"\n"                             \
    "   paddsw  "r0","r0"\n"        /* r0 = C. + C. */      \
    "   paddsw  "r7","r0"\n"        /* r0 = R0 = G. + C. */ \
    "   psraw   ""$4"","r7"\n"      /* r7 = NR7 */          \
    "   movq    "r6","J(6)"\n"  /* store NR6 at J6 */       \
    "   psraw   ""$4"","r0"\n"      /* r0 = NR0 */          \
    "   movq    "r5","J(5)"\n"  /* store NR5 at J5 */       \
    "   movq    "r7","J(7)"\n"  /* store NR7 at J7 */       \
    "   movq    "r0","I(0)"\n"  /* store NR0 at I0 */       \
    "#end ColumnIDCT\n"                                                                         \
);
// end ColumnIDCT macro (38 + 19 = 57 cycles)

/**************************************************************************************
 *
 *      Routine:        Transpose
 *
 *      Description:    The Macro does two 4x4 transposes in place.
 *
 *      Input:          None
 *
 *      Output:         None
 *
 *      Return:         None
 *
 *      Special Note:   None
 *
 *      Error:          None
 *
 ***************************************************************************************
 */

/* Following macro does two 4x4 transposes in place.

  At entry (we assume):

    r0 = a3 a2 a1 a0
    I(1) = b3 b2 b1 b0
    r2 = c3 c2 c1 c0
    r3 = d3 d2 d1 d0

    r4 = e3 e2 e1 e0
    r5 = f3 f2 f1 f0
    r6 = g3 g2 g1 g0
    r7 = h3 h2 h1 h0

   At exit, we have:

    I(0) = d0 c0 b0 a0
    I(1) = d1 c1 b1 a1
    I(2) = d2 c2 b2 a2
    I(3) = d3 c3 b3 a3

    J(4) = h0 g0 f0 e0
    J(5) = h1 g1 f1 e1
    J(6) = h2 g2 f2 e2
    J(7) = h3 g3 f3 e3

   I(0) I(1) I(2) I(3)  is the transpose of r0 I(1) r2 r3.
   J(4) J(5) J(6) J(7)  is the transpose of r4 r5 r6 r7.

   Since r1 is free at entry, we calculate the Js first. */


#define Transpose ASM("\n#Transpose\n"      \
                                            \
    "   movq        "r4","r1"\n"            \
    "   punpcklwd   "r5","r4"\n"            \
    "   movq        "r0","I(0)"\n"          \
    "   punpckhwd   "r5","r1"\n"            \
    "   movq        "r6","r0"\n"            \
    "   punpcklwd   "r7","r6"\n"            \
    "   movq        "r4","r5"\n"            \
    "   punpckldq   "r6","r4"\n"            \
    "   punpckhdq   "r6","r5"\n"            \
    "   movq        "r1","r6"\n"            \
    "   movq        "r4","J(4)"\n"          \
    "   punpckhwd   "r7","r0"\n"            \
    "   movq        "r5","J(5)"\n"          \
    "   punpckhdq   "r0","r6"\n"            \
    "   movq        "I(0)","r4"\n"          \
    "   punpckldq   "r0","r1"\n"            \
    "   movq        "I(1)","r5"\n"          \
    "   movq        "r4","r0"\n"            \
    "   movq        "r6","J(7)"\n"          \
    "   punpcklwd   "r5","r0"\n"            \
    "   movq        "r1","J(6)"\n"          \
    "   punpckhwd   "r5","r4"\n"            \
    "   movq        "r2","r5"\n"            \
    "   punpcklwd   "r3","r2"\n"            \
    "   movq        "r0","r1"\n"            \
    "   punpckldq   "r2","r0"\n"            \
    "   punpckhdq   "r2","r1"\n"            \
    "   movq        "r4","r2"\n"            \
    "   movq        "r0","I(0)"\n"          \
    "   punpckhwd   "r3","r5"\n"            \
    "   movq        "r1","I(1)"\n"          \
    "   punpckhdq   "r5","r4"\n"            \
    "   punpckldq   "r5","r2"\n"            \
                                            \
    "   movq        "r4","I(3)"\n"          \
                                            \
    "   movq        "r2","I(2)"\n"          \
    "#end Transpose\n"                                          \
);
// end Transpose macro (19 cycles).

/*
static void MMX_dump()
{
    ASM
    ("\
        movq    %mm0,(%edi)\n\
        movq    %mm1,8(%edi)\n\
        movq    %mm2,16(%edi)\n\
        movq    %mm3,24(%edi)\n\
        movq    %mm4,32(%edi)\n\
        movq    %mm5,40(%edi)\n\
        movq    %mm6,48(%edi)\n\
        movq    %mm7,56(%edi)\n\
        ret"
    );
}
*/

/**************************************************************************************
 *
 *      Routine:        MMX_idct
 *
 *      Description:    Perform IDCT on a 8x8 block
 *
 *      Input:          Pointer to input and output buffer
 *
 *      Output:         None
 *
 *      Return:         None
 *
 *      Special Note:   The input coefficients are in ZigZag order
 *
 *      Error:          None
 *
 ***************************************************************************************
 */
void IDctSlow__mmx(  Q_LIST_ENTRY * InputData,
                ogg_int16_t *QuantMatrix,
                ogg_int16_t * OutputData ) {

#   define MIDM(M,I)    MtoSTR(M+I*8(%ecx))
#   define M(I)         MIDM( MaskOffset , I )
#   define MIDC(M,I)    MtoSTR(M+(I-1)*8(%ecx))
#   define C(I)         MIDC( CosineOffset , I )
#   define MIDEight(M)  MtoSTR(M(%ecx))
#   define Eight        MIDEight(EightOffset)

#   define r0   "%mm0"
#   define r1   "%mm1"
#   define r2   "%mm2"
#   define r3   "%mm3"
#   define r4   "%mm4"
#   define r5   "%mm5"
#   define r6   "%mm6"
#   define r7   "%mm7"

    __asm__ __volatile__ (
    /* eax = quantized input */
    /* esi = quantization table */
    /* edx = destination (= idct buffer) */
    /* ecx = idctconstants */
    ""
    :
    :"a"(InputData), "S"(QuantMatrix), "d"(OutputData), "c"(idctconstants)
    );

    ASM(
    "movq   (%eax), "r0"\n"
    "pmullw (%esi), "r0"\n"     /* r0 = 03 02 01 00 */
    "movq   16(%eax), "r1"\n"
    "pmullw 16(%esi), "r1"\n"   /* r1 = 13 12 11 10 */
    "movq   "M(0)", "r2"\n"     /* r2 = __ __ __ FF */
    "movq   "r0", "r3"\n"       /* r3 = 03 02 01 00 */
    "movq   8(%eax), "r4"\n"
    "psrlq  $16, "r0"\n"        /* r0 = __ 03 02 01 */
    "pmullw 8(%esi), "r4"\n"    /* r4 = 07 06 05 04 */
    "pand   "r2", "r3"\n"       /* r3 = __ __ __ 00 */
    "movq   "r0", "r5"\n"       /* r5 = __ 03 02 01 */
    "movq   "r1", "r6"\n"       /* r6 = 13 12 11 10 */
    "pand   "r2", "r5"\n"       /* r5 = __ __ __ 01 */
    "psllq  $32, "r6"\n"        /* r6 = 11 10 __ __ */
    "movq   "M(3)", "r7"\n"     /* r7 = FF __ __ __ */
    "pxor   "r5", "r0"\n"       /* r0 = __ 03 02 __ */
    "pand   "r6", "r7"\n"       /* r7 = 11 __ __ __ */
    "por    "r3", "r0"\n"       /* r0 = __ 03 02 00 */
    "pxor   "r7", "r6"\n"       /* r6 = __ 10 __ __ */
    "por    "r7", "r0"\n"       /* r0 = 11 03 02 00 = R0 */
    "movq   "M(3)", "r7"\n"     /* r7 = FF __ __ __ */
    "movq   "r4", "r3"\n"       /* r3 = 07 06 05 04 */
    "movq   "r0", (%edx)\n"     /* write R0 = r0 */
    "pand   "r2", "r3"\n"       /* r3 = __ __ __ 04 */
    "movq   32(%eax), "r0"\n"
    "psllq  $16, "r3"\n"        /* r3 = __ __ 04 __ */
    "pmullw 32(%esi), "r0"\n"   /* r0 = 23 22 21 20 */
    "pand   "r1", "r7"\n"       /* r7 = 13 __ __ __ */
    "por    "r3", "r5"\n"       /* r5 = __ __ 04 01 */
    "por    "r6", "r7"\n"       /* r7 = 13 10 __ __ */
    "movq   24(%eax), "r3"\n"
    "por    "r5", "r7"\n"       /* r7 = 13 10 04 01 = R1 */
    "pmullw 24(%esi), "r3"\n"   /* r3 = 17 16 15 14 */
    "psrlq  $16, "r4"\n"        /* r4 = __ 07 06 05 */
    "movq   "r7", 16(%edx)\n"   /* write R1 = r7 */
    "movq   "r4", "r5"\n"       /* r5 = __ 07 06 05 */
    "movq   "r0", "r7"\n"       /* r7 = 23 22 21 20 */
    "psrlq  $16, "r4"\n"        /* r4 = __ __ 07 06 */
    "psrlq  $48, "r7"\n"        /* r7 = __ __ __ 23 */
    "movq   "r2", "r6"\n"       /* r6 = __ __ __ FF */
    "pand   "r2", "r5"\n"       /* r5 = __ __ __ 05 */
    "pand   "r4", "r6"\n"       /* r6 = __ __ __ 06 */
    "movq   "r7", 80(%edx)\n"   /* partial R9 = __ __ __ 23 */
    "pxor   "r6", "r4"\n"       /* r4 = __ __ 07 __ */
    "psrlq  $32, "r1"\n"        /* r1 = __ __ 13 12 */
    "por    "r5", "r4"\n"       /* r4 = __ __ 07 05 */
    "movq   "M(3)", "r7"\n"     /* r7 = FF __ __ __ */
    "pand   "r2", "r1"\n"       /* r1 = __ __ __ 12 */
    "movq   48(%eax), "r5"\n"
    "psllq  $16, "r0"\n"        /* r0 = 22 21 20 __ */
    "pmullw 48(%esi), "r5"\n"   /* r5 = 33 32 31 30 */
    "pand   "r0", "r7"\n"       /* r7 = 22 __ __ __ */
    "movq   "r1", 64(%edx)\n"   /* partial R8 = __ __ __ 12 */
    "por    "r4", "r7"\n"       /* r7 = 22 __ 07 05 */
    "movq   "r3", "r4"\n"       /* r4 = 17 16 15 14 */
    "pand   "r2", "r3"\n"       /* r3 = __ __ __ 14 */
    "movq   "M(2)", "r1"\n"     /* r1 = __ FF __ __ */
    "psllq  $32, "r3"\n"        /* r3 = __ 14 __ __ */
    "por    "r3", "r7"\n"       /* r7 = 22 14 07 05 = R2 */
    "movq   "r5", "r3"\n"       /* r3 = 33 32 31 30 */
    "psllq  $48, "r3"\n"        /* r3 = 30 __ __ __ */
    "pand   "r0", "r1"\n"       /* r1 = __ 21 __ __ */
    "movq   "r7", 32(%edx)\n"   /* write R2 = r7 */
    "por    "r3", "r6"\n"       /* r6 = 30 __ __ 06 */
    "movq   "M(1)", "r7"\n"     /* r7 = __ __ FF __ */
    "por    "r1", "r6"\n"       /* r6 = 30 21 __ 06 */
    "movq   56(%eax), "r1"\n"
    "pand   "r4", "r7"\n"       /* r7 = __ __ 15 __ */
    "pmullw 56(%esi), "r1"\n"   /* r1 = 37 36 35 34 */
    "por    "r6", "r7"\n"       /* r7 = 30 21 15 06 = R3 */
    "pand   "M(1)", "r0"\n"     /* r0 = __ __ 20 __ */
    "psrlq  $32, "r4"\n"        /* r4 = __ __ 17 16 */
    "movq   "r7", 48(%edx)\n"   /* write R3 = r7 */
    "movq   "r4", "r6"\n"       /* r6 = __ __ 17 16 */
    "movq   "M(3)", "r7"\n"     /* r7 = FF __ __ __ */
    "pand   "r2", "r4"\n"       /* r4 = __ __ __ 16 */
    "movq   "M(1)", "r3"\n"     /* r3 = __ __ FF __ */
    "pand   "r1", "r7"\n"       /* r7 = 37 __ __ __ */
    "pand   "r5", "r3"\n"       /* r3 = __ __ 31 __ */
    "por    "r4", "r0"\n"       /* r0 = __ __ 20 16 */
    "psllq  $16, "r3"\n"        /* r3 = __ 31 __ __ */
    "por    "r0", "r7"\n"       /* r7 = 37 __ 20 16 */
    "movq   "M(2)", "r4"\n"     /* r4 = __ FF __ __ */
    "por    "r3", "r7"\n"       /* r7 = 37 31 20 16 = R4 */
    "movq   80(%eax), "r0"\n"
    "movq   "r4", "r3"\n"       /* r3 = __ __ FF __ */
    "pmullw 80(%esi), "r0"\n"   /* r0 = 53 52 51 50 */
    "pand   "r5", "r4"\n"       /* r4 = __ 32 __ __ */
    "movq   "r7", 8(%edx)\n"    /* write R4 = r7 */
    "por    "r4", "r6"\n"       /* r6 = __ 32 17 16 */
    "movq   "r3", "r4"\n"       /* r4 = __ FF __ __ */
    "psrlq  $16, "r6"\n"        /* r6 = __ __ 32 17 */
    "movq   "r0", "r7"\n"       /* r7 = 53 52 51 50 */
    "pand   "r1", "r4"\n"       /* r4 = __ 36 __ __ */
    "psllq  $48, "r7"\n"        /* r7 = 50 __ __ __ */
    "por    "r4", "r6"\n"       /* r6 = __ 36 32 17 */
    "movq   88(%eax), "r4"\n"
    "por    "r6", "r7"\n"       /* r7 = 50 36 32 17 = R5 */
    "pmullw 88(%esi), "r4"\n"   /* r4 = 57 56 55 54 */
    "psrlq  $16, "r3"\n"        /* r3 = __ __ FF __ */
    "movq   "r7", 24(%edx)\n"   /* write R5 = r7 */
    "pand   "r1", "r3"\n"       /* r3 = __ __ 35 __ */
    "psrlq  $48, "r5"\n"        /* r5 = __ __ __ 33 */
    "pand   "r2", "r1"\n"       /* r1 = __ __ __ 34 */
    "movq   104(%eax), "r6"\n"
    "por    "r3", "r5"\n"       /* r5 = __ __ 35 33 */
    "pmullw 104(%esi), "r6"\n"  /* r6 = 67 66 65 64 */
    "psrlq  $16, "r0"\n"        /* r0 = __ 53 52 51 */
    "movq   "r4", "r7"\n"       /* r7 = 57 56 55 54 */
    "movq   "r2", "r3"\n"       /* r3 = __ __ __ FF */
    "psllq  $48, "r7"\n"        /* r7 = 54 __ __ __ */
    "pand   "r0", "r3"\n"       /* r3 = __ __ __ 51 */
    "pxor   "r3", "r0"\n"       /* r0 = __ 53 52 __ */
    "psllq  $32, "r3"\n"        /* r3 = __ 51 __ __ */
    "por    "r5", "r7"\n"       /* r7 = 54 __ 35 33 */
    "movq   "r6", "r5"\n"       /* r5 = 67 66 65 64 */
    "pand   "M(1)", "r6"\n"     /* r6 = __ __ 65 __ */
    "por    "r3", "r7"\n"       /* r7 = 54 51 35 33 = R6 */
    "psllq  $32, "r6"\n"        /* r6 = 65 __ __ __ */
    "por    "r1", "r0"\n"       /* r0 = __ 53 52 34 */
    "movq   "r7", 40(%edx)\n"   /* write R6 = r7 */
    "por    "r6", "r0"\n"       /* r0 = 65 53 52 34 = R7 */
    "movq   120(%eax), "r7"\n"
    "movq   "r5", "r6"\n"       /* r6 = 67 66 65 64 */
    "pmullw 120(%esi), "r7"\n"  /* r7 = 77 76 75 74 */
    "psrlq  $32, "r5"\n"        /* r5 = __ __ 67 66 */
    "pand   "r2", "r6"\n"       /* r6 = __ __ __ 64 */
    "movq   "r5", "r1"\n"       /* r1 = __ __ 67 66 */
    "movq   "r0", 56(%edx)\n"   /* write R7 = r0 */
    "pand   "r2", "r1"\n"       /* r1 = __ __ __ 66 */
    "movq   112(%eax), "r0"\n"
    "movq   "r7", "r3"\n"       /* r3 = 77 76 75 74 */
    "pmullw 112(%esi), "r0"\n"  /* r0 = 73 72 71 70 */
    "psllq  $16, "r3"\n"        /* r3 = 76 75 74 __ */
    "pand   "M(3)", "r7"\n"     /* r7 = 77 __ __ __ */
    "pxor   "r1", "r5"\n"       /* r5 = __ __ 67 __ */
    "por    "r5", "r6"\n"       /* r6 = __ __ 67 64 */
    "movq   "r3", "r5"\n"       /* r5 = 76 75 74 __ */
    "pand   "M(3)", "r5"\n"     /* r5 = 76 __ __ __ */
    "por    "r1", "r7"\n"       /* r7 = 77 __ __ 66 */
    "movq   96(%eax), "r1"\n"
    "pxor   "r5", "r3"\n"       /* r3 = __ 75 74 __ */
    "pmullw 96(%esi), "r1"\n"   /* r1 = 63 62 61 60 */
    "por    "r3", "r7"\n"       /* r7 = 77 75 74 66 = R15 */
    "por    "r5", "r6"\n"       /* r6 = 76 __ 67 64 */
    "movq   "r0", "r5"\n"       /* r5 = 73 72 71 70 */
    "movq   "r7", 120(%edx)\n"  /* store R15 = r7 */
    "psrlq  $16, "r5"\n"        /* r5 = __ 73 72 71 */
    "pand   "M(2)", "r5"\n"     /* r5 = __ 73 __ __ */
    "movq   "r0", "r7"\n"       /* r7 = 73 72 71 70 */
    "por    "r5", "r6"\n"       /* r6 = 76 73 67 64 = R14 */
    "pand   "r2", "r0"\n"       /* r0 = __ __ __ 70 */
    "pxor   "r0", "r7"\n"       /* r7 = 73 72 71 __ */
    "psllq  $32, "r0"\n"        /* r0 = __ 70 __ __ */
    "movq   "r6", 104(%edx)\n"  /* write R14 = r6 */
    "psrlq  $16, "r4"\n"        /* r4 = __ 57 56 55 */
    "movq   72(%eax), "r5"\n"
    "psllq  $16, "r7"\n"        /* r7 = 72 71 __ __ */
    "pmullw 72(%esi), "r5"\n"   /* r5 = 47 46 45 44 */
    "movq   "r7", "r6"\n"       /* r6 = 72 71 __ __ */
    "movq   "M(2)", "r3"\n"     /* r3 = __ FF __ __ */
    "psllq  $16, "r6"\n"        /* r6 = 71 __ __ __ */
    "pand   "M(3)", "r7"\n"     /* r7 = 72 __ __ __ */
    "pand   "r1", "r3"\n"       /* r3 = __ 62 __ __ */
    "por    "r0", "r7"\n"       /* r7 = 72 70 __ __ */
    "movq   "r1", "r0"\n"       /* r0 = 63 62 61 60 */
    "pand   "M(3)", "r1"\n"     /* r1 = 63 __ __ __ */
    "por    "r3", "r6"\n"       /* r6 = 71 62 __ __ */
    "movq   "r4", "r3"\n"       /* r3 = __ 57 56 55 */
    "psrlq  $32, "r1"\n"        /* r1 = __ __ 63 __ */
    "pand   "r2", "r3"\n"       /* r3 = __ __ __ 55 */
    "por    "r1", "r7"\n"       /* r7 = 72 70 63 __ */
    "por    "r3", "r7"\n"       /* r7 = 72 70 63 55 = R13 */
    "movq   "r4", "r3"\n"       /* r3 = __ 57 56 55 */
    "pand   "M(1)", "r3"\n"     /* r3 = __ __ 56 __ */
    "movq   "r5", "r1"\n"       /* r1 = 47 46 45 44 */
    "movq   "r7", 88(%edx)\n"   /* write R13 = r7 */
    "psrlq  $48, "r5"\n"        /* r5 = __ __ __ 47 */
    "movq   64(%eax), "r7"\n"
    "por    "r3", "r6"\n"       /* r6 = 71 62 56 __ */
    "pmullw 64(%esi), "r7"\n"   /* r7 = 43 42 41 40 */
    "por    "r5", "r6"\n"       /* r6 = 71 62 56 47 = R12 */
    "pand   "M(2)", "r4"\n"     /* r4 = __ 57 __ __ */
    "psllq  $32, "r0"\n"        /* r0 = 61 60 __ __ */
    "movq   "r6", 72(%edx)\n"   /* write R12 = r6 */
    "movq   "r0", "r6"\n"       /* r6 = 61 60 __ __ */
    "pand   "M(3)", "r0"\n"     /* r0 = 61 __ __ __ */
    "psllq  $16, "r6"\n"        /* r6 = 60 __ __ __ */
    "movq   40(%eax), "r5"\n"
    "movq   "r1", "r3"\n"       /* r3 = 47 46 45 44 */
    "pmullw 40(%esi), "r5"\n"   /* r5 = 27 26 25 24 */
    "psrlq  $16, "r1"\n"        /* r1 = __ 47 46 45 */
    "pand   "M(1)", "r1"\n"     /* r1 = __ __ 46 __ */
    "por    "r4", "r0"\n"       /* r0 = 61 57 __ __ */
    "pand   "r7", "r2"\n"       /* r2 = __ __ __ 40 */
    "por    "r1", "r0"\n"       /* r0 = 61 57 46 __ */
    "por    "r2", "r0"\n"       /* r0 = 61 57 46 40 = R11 */
    "psllq  $16, "r3"\n"        /* r3 = 46 45 44 __ */
    "movq   "r3", "r4"\n"       /* r4 = 46 45 44 __ */
    "movq   "r5", "r2"\n"       /* r2 = 27 26 25 24 */
    "movq   "r0", 112(%edx)\n"  /* write R11 = r0 */
    "psrlq  $48, "r2"\n"        /* r2 = __ __ __ 27 */
    "pand   "M(2)", "r4"\n"     /* r4 = __ 45 __ __ */
    "por    "r2", "r6"\n"       /* r6 = 60 __ __ 27 */
    "movq   "M(1)", "r2"\n"     /* r2 = __ __ FF __ */
    "por    "r4", "r6"\n"       /* r6 = 60 45 __ 27 */
    "pand   "r7", "r2"\n"       /* r2 = __ __ 41 __ */
    "psllq  $32, "r3"\n"        /* r3 = 44 __ __ __ */
    "por    80(%edx), "r3"\n"   /* r3 = 44 __ __ 23 */
    "por    "r2", "r6"\n"       /* r6 = 60 45 41 27 = R10 */
    "movq   "M(3)", "r2"\n"     /* r2 = FF __ __ __ */
    "psllq  $16, "r5"\n"        /* r5 = 26 25 24 __ */
    "movq   "r6", 96(%edx)\n"   /* store R10 = r6 */
    "pand   "r5", "r2"\n"       /* r2 = 26 __ __ __ */
    "movq   "M(2)", "r6"\n"     /* r6 = __ FF __ __ */
    "pxor   "r2", "r5"\n"       /* r5 = __ 25 24 __ */
    "pand   "r7", "r6"\n"       /* r6 = __ 42 __ __ */
    "psrlq  $32, "r2"\n"        /* r2 = __ __ 26 __ */
    "pand   "M(3)", "r7"\n"     /* r7 = 43 __ __ __ */
    "por    "r2", "r3"\n"       /* r3 = 44 __ 26 23 */
    "por    64(%edx), "r7"\n"   /* r7 = 43 __ __ 12 */
    "por    "r3", "r6"\n"       /* r6 = 44 42 26 23 = R9 */
    "por    "r5", "r7"\n"       /* r7 = 43 25 24 12 = R8 */
    "movq   "r6", 80(%edx)\n"   /* store R9 = r6 */
    "movq   "r7", 64(%edx)\n"   /* store R8 = r7 */
    );
    /* 123c  ( / 64 coeffs  < 2c / coeff) */
#   undef M

/* Done w/dequant + descramble + partial transpose; now do the idct itself. */

#   define I( K)    MtoSTR(K*16(%edx))
#   define J( K)    MtoSTR(((K - 4)*16)+8(%edx))

    RowIDCT         /* 46 c */
    Transpose       /* 19 c */

#   undef I
#   undef J
#   define I( K)    MtoSTR((K*16)+64(%edx))
#   define J( K)    MtoSTR(((K-4)*16)+72(%edx))

    RowIDCT         /* 46 c */
    Transpose       /* 19 c */

#   undef I
#   undef J
#   define I( K)    MtoSTR((K * 16)(%edx))
#   define J( K)    I( K)

    ColumnIDCT      /* 57 c */

#   undef I
#   undef J
#   define I( K)    MtoSTR((K*16)+8(%edx))
#   define J( K)    I( K)

    ColumnIDCT      /* 57 c */

#   undef I
#   undef J
    /* 368 cycles  ( / 64 coeff  <  6 c / coeff) */

    ASM("emms\n");
}

/**************************************************************************************
 *
 *      Routine:        MMX_idct10
 *
 *      Description:    Perform IDCT on a 8x8 block with at most 10 nonzero coefficients
 *
 *      Input:          Pointer to input and output buffer
 *
 *      Output:         None
 *
 *      Return:         None
 *
 *      Special Note:   The input coefficients are in transposed ZigZag order
 *
 *      Error:          None
 *
 ***************************************************************************************
 */
/* --------------------------------------------------------------- */
// This macro does four 4-sample one-dimensional idcts in parallel.  Inputs
// 4 thru 7 are assumed to be zero.
#define BeginIDCT_10 "#BeginIDCT_10\n"  \
    "   movq    "I(3)","r2"\n"          \
                                        \
    "   movq    "C(3)","r6"\n"          \
    "   movq    "r2","r4"\n"            \
                                        \
    "   movq    "C(5)","r1"\n"          \
    "   pmulhw  "r6","r4"\n"            \
                                        \
    "   movq    "I(1)","r3"\n"          \
    "   pmulhw  "r2","r1"\n"            \
                                        \
    "   movq    "C(1)","r0"\n"          \
    "   paddw   "r2","r4"\n"            \
                                        \
    "   pxor    "r6","r6"\n"            \
    "   paddw   "r1","r2"\n"            \
                                        \
    "   movq    "I(2)","r5"\n"          \
    "   pmulhw  "r3","r0"\n"            \
                                        \
    "   movq    "r5","r1"\n"            \
    "   paddw   "r3","r0"\n"            \
                                        \
    "   pmulhw  "C(7)","r3"\n"          \
    "   psubsw  "r2","r6"\n"            \
                                        \
    "   pmulhw  "C(2)","r5"\n"          \
    "   psubsw  "r4","r0"\n"            \
                                        \
    "   movq    "I(2)","r7"\n"          \
    "   paddsw  "r4","r4"\n"            \
                                        \
    "   paddw   "r5","r7"\n"            \
    "   paddsw  "r0","r4"\n"            \
                                        \
    "   pmulhw  "C(6)","r1"\n"          \
    "   psubsw  "r6","r3"\n"            \
                                        \
    "   movq    "r4","I(1)"\n"          \
    "   paddsw  "r6","r6"\n"            \
                                        \
    "   movq    "C(4)","r4"\n"          \
    "   paddsw  "r3","r6"\n"            \
                                        \
    "   movq    "r3","r5"\n"            \
    "   pmulhw  "r4","r3"\n"            \
                                        \
    "   movq    "r6","I(2)"\n"          \
    "   movq    "r0","r2"\n"            \
                                        \
    "   movq    "I(0)","r6"\n"          \
    "   pmulhw  "r4","r0"\n"            \
                                        \
    "   paddw   "r3","r5"\n"            \
    "   paddw   "r0","r2"\n"            \
                                        \
    "   psubsw  "r1","r5"\n"            \
    "   pmulhw  "r4","r6"\n"            \
                                        \
    "   paddw   "I(0)","r6"\n"          \
    "   paddsw  "r1","r1"\n"            \
                                        \
    "   movq    "r6","r4"\n"            \
    "   paddsw  "r5","r1"\n"            \
                                        \
    "   psubsw  "r2","r6"\n"            \
    "   paddsw  "r2","r2"\n"            \
                                        \
    "   movq    "I(1)","r0"\n"          \
    "   paddsw  "r6","r2"\n"            \
                                        \
    "   psubsw  "r1","r2"\n"            \
    "#end BeginIDCT_10\n"
// end BeginIDCT_10 macro (25 cycles).

#define RowIDCT_10 ASM("\n"                                 \
    "#RowIDCT_10\n"                                         \
    BeginIDCT_10                                            \
    "\n"                                                    \
    "   movq    "I(2)","r3"\n"  /* r3 = D. */               \
    "   psubsw  "r7","r4"\n"        /* r4 = E. = E - G */   \
    "   paddsw  "r1","r1"\n"        /* r1 = H. + H. */      \
    "   paddsw  "r7","r7"\n"        /* r7 = G + G */        \
    "   paddsw  "r2","r1"\n"        /* r1 = R1 = A.. + H. */\
    "   paddsw  "r4","r7"\n"        /* r7 = G. = E + G */   \
    "   psubsw  "r3","r4"\n"        /* r4 = R4 = E. - D. */ \
    "   paddsw  "r3","r3"\n"                                \
    "   psubsw  "r5","r6"\n"        /* r6 = R6 = F. - B.. */\
    "   paddsw  "r5","r5"\n"                                \
    "   paddsw  "r4","r3"\n"        /* r3 = R3 = E. + D. */ \
    "   paddsw  "r6","r5"\n"        /* r5 = R5 = F. + B.. */\
    "   psubsw  "r0","r7"\n"        /* r7 = R7 = G. - C. */ \
    "   paddsw  "r0","r0"\n"                                \
    "   movq    "r1","I(1)"\n"  /* save R1 */               \
    "   paddsw  "r7","r0"\n"        /* r0 = R0 = G. + C. */ \
    "#end RowIDCT_10\n"                                                                         \
);
// end RowIDCT macro (8 + 38 = 46 cycles)

// Column IDCT normalizes and stores final results.

#define ColumnIDCT_10 ASM("\n"                          \
    "#ColumnIDCT_10\n"                                  \
    BeginIDCT_10                                        \
    "\n"                                                \
    "   paddsw  "Eight","r2"\n"                         \
    "   paddsw  "r1","r1"\n"    /* r1 = H. + H. */      \
    "   paddsw  "r2","r1"\n"    /* r1 = R1 = A.. + H. */\
    "   psraw   ""$4"","r2"\n"      /* r2 = NR2 */      \
    "   psubsw  "r7","r4"\n"    /* r4 = E. = E - G */   \
    "   psraw   ""$4"","r1"\n"      /* r1 = NR1 */      \
    "   movq    "I(2)","r3"\n"  /* r3 = D. */           \
    "   paddsw  "r7","r7"\n"    /* r7 = G + G */        \
    "   movq    "r2","I(2)"\n"  /* store NR2 at I2 */   \
    "   paddsw  "r4","r7"\n"    /* r7 = G. = E + G */   \
    "   movq    "r1","I(1)"\n"  /* store NR1 at I1 */   \
    "   psubsw  "r3","r4"\n"    /* r4 = R4 = E. - D. */ \
    "   paddsw  "Eight","r4"\n"                         \
    "   paddsw  "r3","r3"\n"    /* r3 = D. + D. */      \
    "   paddsw  "r4","r3"\n"    /* r3 = R3 = E. + D. */ \
    "   psraw   ""$4"","r4"\n"      /* r4 = NR4 */      \
    "   psubsw  "r5","r6"\n"    /* r6 = R6 = F. - B.. */\
    "   psraw   ""$4"","r3"\n"      /* r3 = NR3 */      \
    "   paddsw  "Eight","r6"\n"                         \
    "   paddsw  "r5","r5"\n"    /* r5 = B.. + B.. */    \
    "   paddsw  "r6","r5"\n"    /* r5 = R5 = F. + B.. */\
    "   psraw   ""$4"","r6"\n"      /* r6 = NR6 */      \
    "   movq    "r4","J(4)"\n"  /* store NR4 at J4 */   \
    "   psraw   ""$4"","r5"\n"      /* r5 = NR5 */      \
    "   movq    "r3","I(3)"\n"  /* store NR3 at I3 */   \
    "   psubsw  "r0","r7"\n"    /* r7 = R7 = G. - C. */ \
    "   paddsw  "Eight","r7"\n"                         \
    "   paddsw  "r0","r0"\n"    /* r0 = C. + C. */      \
    "   paddsw  "r7","r0"\n"    /* r0 = R0 = G. + C. */ \
    "   psraw   ""$4"","r7"\n"      /* r7 = NR7 */      \
    "   movq    "r6","J(6)"\n"  /* store NR6 at J6 */   \
    "   psraw   ""$4"","r0"\n"      /* r0 = NR0 */      \
    "   movq    "r5","J(5)"\n"  /* store NR5 at J5 */   \
                                                        \
    "   movq    "r7","J(7)"\n"  /* store NR7 at J7 */   \
                                                        \
    "   movq    "r0","I(0)"\n"  /* store NR0 at I0 */   \
    "#end ColumnIDCT_10\n"                                                              \
);
// end ColumnIDCT macro (38 + 19 = 57 cycles)
/* --------------------------------------------------------------- */


/* --------------------------------------------------------------- */
/* IDCT 10 */
void IDct10__mmx( Q_LIST_ENTRY * InputData,
             ogg_int16_t *QuantMatrix,
             ogg_int16_t * OutputData ) {

#   define MIDM(M,I)    MtoSTR(M+I*8(%ecx))
#   define M(I)         MIDM( MaskOffset , I )
#   define MIDC(M,I)    MtoSTR(M+(I-1)*8(%ecx))
#   define C(I)         MIDC( CosineOffset , I )
#   define MIDEight(M)  MtoSTR(M(%ecx))
#   define Eight        MIDEight(EightOffset)

#   define r0   "%mm0"
#   define r1   "%mm1"
#   define r2   "%mm2"
#   define r3   "%mm3"
#   define r4   "%mm4"
#   define r5   "%mm5"
#   define r6   "%mm6"
#   define r7   "%mm7"

    __asm__ __volatile__ (
    /* eax = quantized input */
    /* esi = quantization table */
    /* edx = destination (= idct buffer) */
    /* ecx = idctconstants */
    ""
    :
    :"a"(InputData), "S"(QuantMatrix), "d"(OutputData), "c"(idctconstants)
    );

    ASM(
    "movq   (%eax), "r0"\n"
    "pmullw (%esi), "r0"\n"     /* r0 = 03 02 01 00 */
    "movq   16(%eax), "r1"\n"
    "pmullw 16(%esi), "r1"\n"   /* r1 = 13 12 11 10 */
    "movq   "M(0)", "r2"\n"     /* r2 = __ __ __ FF */
    "movq   "r0", "r3"\n"       /* r3 = 03 02 01 00 */
    "movq   8(%eax), "r4"\n"
    "psrlq  $16, "r0"\n"        /* r0 = __ 03 02 01 */
    "pmullw 8(%esi), "r4"\n"    /* r4 = 07 06 05 04 */
    "pand   "r2", "r3"\n"       /* r3 = __ __ __ 00 */
    "movq   "r0", "r5"\n"       /* r5 = __ 03 02 01 */
    "pand   "r2", "r5"\n"       /* r5 = __ __ __ 01 */
    "psllq  $32, "r1"\n"        /* r1 = 11 10 __ __ */
    "movq   "M(3)", "r7"\n"     /* r7 = FF __ __ __ */
    "pxor   "r5", "r0"\n"       /* r0 = __ 03 02 __ */
    "pand   "r1", "r7"\n"       /* r7 = 11 __ __ __ */
    "por    "r3", "r0"\n"       /* r0 = __ 03 02 00 */
    "pxor   "r7", "r1"\n"       /* r1 = __ 10 __ __ */
    "por    "r7", "r0"\n"       /* r0 = 11 03 02 00 = R0 */
    "movq   "r4", "r3"\n"       /* r3 = 07 06 05 04 */
    "movq   "r0", (%edx)\n"     /* write R0 = r0 */
    "pand   "r2", "r3"\n"       /* r3 = __ __ __ 04 */
    "psllq  $16, "r3"\n"        /* r3 = __ __ 04 __ */
    "por    "r3", "r5"\n"       /* r5 = __ __ 04 01 */
    "por    "r5", "r1"\n"       /* r1 = __ 10 04 01 = R1 */
    "psrlq  $16, "r4"\n"        /* r4 = __ 07 06 05 */
    "movq   "r1", 16(%edx)\n"   /* write R1 = r1 */
    "movq   "r4", "r5"\n"       /* r5 = __ 07 06 05 */
    "psrlq  $16, "r4"\n"        /* r4 = __ __ 07 06 */
    "movq   "r2", "r6"\n"       /* r6 = __ __ __ FF */
    "pand   "r2", "r5"\n"       /* r5 = __ __ __ 05 */
    "pand   "r4", "r6"\n"       /* r6 = __ __ __ 06 */
    "pxor   "r6", "r4"\n"       /* r4 = __ __ 07 __ */
    "por    "r5", "r4"\n"       /* r4 = __ __ 07 05 */
    "movq   "r4", 32(%edx)\n"   /* write R2 = r4 */
    "movq   "r6", 48(%edx)\n"   /* write R3 = r6 */
    );
#   undef M

/* Done w/dequant + descramble + partial transpose; now do the idct itself. */

#   define I( K)    MtoSTR((K*16)(%edx))
#   define J( K)    MtoSTR(((K - 4) * 16)+8(%edx))

    RowIDCT_10      /* 33 c */
    Transpose       /* 19 c */

#   undef I
#   undef J
//# define I( K)    [edx + (  K      * 16) + 64]
//# define J( K)    [edx + ( (K - 4) * 16) + 72]

//  RowIDCT         ; 46 c
//  Transpose       ; 19 c

//# undef I
//# undef J
#   define I( K)    MtoSTR((K * 16)(%edx))
#   define J( K)    I( K)

    ColumnIDCT_10       /* 44 c */

#   undef I
#   undef J
#   define I( K)    MtoSTR((K * 16)+8(%edx))
#   define J( K)    I( K)

    ColumnIDCT_10       /* 44 c */

#   undef I
#   undef J

    ASM("emms\n");
}

/**************************************************************************************
 *
 *      Routine:        MMX_idct3
 *
 *      Description:    Perform IDCT on a 8x8 block with at most 3 nonzero coefficients
 *
 *      Input:          Pointer to input and output buffer
 *
 *      Output:         None
 *
 *      Return:         None
 *
 *      Special Note:   Only works for three nonzero coefficients.
 *
 *      Error:          None
 *
 ***************************************************************************************
 */
/***************************************************************************************
    In IDCT 3, we are dealing with only three Non-Zero coefficients in the 8x8 block.
    In the case that we work in the fashion RowIDCT -> ColumnIDCT, we only have to
    do 1-D row idcts on the first two rows, the rest six rows remain zero anyway.
    After row IDCTs, since every column could have nonzero coefficients, we need do
    eight 1-D column IDCT. However, for each column, there are at most two nonzero
    coefficients, coefficient 0 and coefficient 1. Same for the coefficents for the
    two 1-d row idcts. For this reason, the process of a 1-D IDCT is simplified

    from a full version:

    A = (C1 * I1) + (C7 * I7)       B = (C7 * I1) - (C1 * I7)
    C = (C3 * I3) + (C5 * I5)       D = (C3 * I5) - (C5 * I3)
    A. = C4 * (A - C)               B. = C4 * (B - D)
    C. = A + C                      D. = B + D

    E = C4 * (I0 + I4)              F = C4 * (I0 - I4)
    G = (C2 * I2) + (C6 * I6)       H = (C6 * I2) - (C2 * I6)
    E. = E - G
    G. = E + G

    A.. = F + A.                    B.. = B. - H
    F.  = F - A.                    H.  = B. + H

    R0 = G. + C.    R1 = A.. + H.   R3 = E. + D.    R5 = F. + B..
    R7 = G. - C.    R2 = A.. - H.   R4 = E. - D.    R6 = F. - B..

    To:


    A = (C1 * I1)                   B = (C7 * I1)
    C = 0                           D = 0
    A. = C4 * A                     B. = C4 * B
    C. = A                          D. = B

    E = C4 * I0                     F = E
    G = 0                           H = 0
    E. = E
    G. = E

    A.. = E + A.                    B.. = B.
    F.  = E - A.                    H.  = B.

    R0 = E + A      R1 = E + A. + B.    R3 = E + B      R5 = E - A. + B.
    R7 = E - A      R2 = E + A. - B.    R4 = E - B      R6 = F - A. - B.

******************************************************************************************/

#define RowIDCT_3 ASM("\n"\
    "#RowIDCT_3\n"\
    "   movq        "I(1)","r7"\n"  /* r7 = I1                      */  \
    "   movq        "C(1)","r0"\n"  /* r0 = C1                      */  \
    "   movq        "C(7)","r3"\n"  /* r3 = C7                      */  \
    "   pmulhw      "r7","r0"\n"    /* r0 = C1 * I1 - I1            */  \
    "   pmulhw      "r7","r3"\n"    /* r3 = C7 * I1 = B, D.         */  \
    "   movq        "I(0)","r6"\n"  /* r6 = I0                      */  \
    "   movq        "C(4)","r4"\n"  /* r4 = C4                      */  \
    "   paddw       "r7","r0"\n"    /* r0 = C1 * I1 = A, C.         */  \
    "   movq        "r6","r1"\n"    /* make a copy of I0            */  \
    "   pmulhw      "r4","r6"\n"    /* r2 = C4 * I0 - I0            */  \
    "   movq        "r0","r2"\n"    /* make a copy of A             */  \
    "   movq        "r3","r5"\n"    /* make a copy of B             */  \
    "   pmulhw      "r4","r2"\n"    /* r2 = C4 * A - A              */  \
    "   pmulhw      "r4","r5"\n"    /* r5 = C4 * B - B              */  \
    "   paddw       "r1","r6"\n"    /* r2 = C4 * I0 = E, F          */  \
    "   movq        "r6","r4"\n"    /* r4 = E                       */  \
    "   paddw       "r0","r2"\n"    /* r2 = A.                      */  \
    "   paddw       "r3","r5"\n"    /* r5 = B.                      */  \
    "   movq        "r6","r7"\n"    /* r7 = E                       */  \
    "   movq        "r5","r1"\n"    /* r1 = B.                      */  \
    /*  r0 = A      */   \
    /*  r3 = B      */   \
    /*  r2 = A.     */   \
    /*  r5 = B.     */   \
    /*  r6 = E      */   \
    /*  r4 = E      */   \
    /*  r7 = E      */   \
    /*  r1 = B.     */   \
    "   psubw       "r2","r6"\n"    /* r6 = E - A.                  */  \
    "   psubw       "r3","r4"\n"    /* r4 = E - B ----R4            */  \
    "   psubw       "r0","r7"\n"    /* r7 = E - A ----R7            */  \
    "   paddw       "r2","r2"\n"    /* r2 = A. + A.                 */  \
    "   paddw       "r3","r3"\n"    /* r3 = B + B                   */  \
    "   paddw       "r0","r0"\n"    /* r0 = A + A                   */  \
    "   paddw       "r6","r2"\n"    /* r2 = E + A.                  */  \
    "   paddw       "r4","r3"\n"    /* r3 = E + B ----R3            */  \
    "   psubw       "r1","r2"\n"    /* r2 = E + A. - B. ----R2      */  \
    "   psubw       "r5","r6"\n"    /* r6 = E - A. - B. ----R6      */  \
    "   paddw       "r1","r1"\n"    /* r1 = B. + B.                 */  \
    "   paddw       "r5","r5"\n"    /* r5 = B. + B.                 */  \
    "   paddw       "r7","r0"\n"    /* r0 = E + A ----R0            */  \
    "   paddw       "r2","r1"\n"    /* r1 = E + A. + B. -----R1     */  \
    "   movq        "r1","I(1)"\n"  /* save r1                      */  \
    "   paddw       "r6","r5"\n"    /* r5 = E - A. + B. -----R5     */  \
    "#end RowIDCT_3\n"\
);
//End of RowIDCT_3

#define ColumnIDCT_3 ASM("\n"\
    "#ColumnIDCT_3\n"\
    "   movq        "I(1)","r7"\n"  /* r7 = I1                      */  \
    "   movq        "C(1)","r0"\n"  /* r0 = C1                      */  \
    "   movq        "C(7)","r3"\n"  /* r3 = C7                      */  \
    "   pmulhw      "r7","r0"\n"    /* r0 = C1 * I1 - I1            */  \
    "   pmulhw      "r7","r3"\n"    /* r3 = C7 * I1 = B, D.         */  \
    "   movq        "I(0)","r6"\n"  /* r6 = I0                      */  \
    "   movq        "C(4)","r4"\n"  /* r4 = C4                      */  \
    "   paddw       "r7","r0"\n"    /* r0 = C1 * I1 = A, C.         */  \
    "   movq        "r6","r1"\n"    /* make a copy of I0            */  \
    "   pmulhw      "r4","r6"\n"    /* r2 = C4 * I0 - I0            */  \
    "   movq        "r0","r2"\n"    /* make a copy of A             */  \
    "   movq        "r3","r5"\n"    /* make a copy of B             */  \
    "   pmulhw      "r4","r2"\n"    /* r2 = C4 * A - A              */  \
    "   pmulhw      "r4","r5"\n"    /* r5 = C4 * B - B              */  \
    "   paddw       "r1","r6"\n"    /* r2 = C4 * I0 = E, F          */  \
    "   movq        "r6","r4"\n"    /* r4 = E                       */  \
    "   paddw       "Eight","r6"\n" /* +8 for shift                 */  \
    "   paddw       "Eight","r4"\n" /* +8 for shift                 */  \
    "   paddw       "r0","r2"\n"    /* r2 = A.                      */  \
    "   paddw       "r3","r5"\n"    /* r5 = B.                      */  \
    "   movq        "r6","r7"\n"    /* r7 = E                       */  \
    "   movq        "r5","r1"\n"    /* r1 = B.                      */  \
/*  r0 = A      */   \
/*  r3 = B      */   \
/*  r2 = A.     */   \
/*  r5 = B.     */   \
/*  r6 = E      */   \
/*  r4 = E      */   \
/*  r7 = E      */   \
/*  r1 = B.     */   \
    "   psubw       "r2","r6"\n"    /* r6 = E - A.                  */  \
    "   psubw       "r3","r4"\n"    /* r4 = E - B ----R4            */  \
    "   psubw       "r0","r7"\n"    /* r7 = E - A ----R7            */  \
    "   paddw       "r2","r2"\n"    /* r2 = A. + A.                 */  \
    "   paddw       "r3","r3"\n"    /* r3 = B + B                   */  \
    "   paddw       "r0","r0"\n"    /* r0 = A + A                   */  \
    "   paddw       "r6","r2"\n"    /* r2 = E + A.                  */  \
    "   paddw       "r4","r3"\n"    /* r3 = E + B ----R3            */  \
    "   psraw        $4,"r4"\n"     /* shift                        */  \
    "   movq        "r4","J(4)"\n"  /* store R4 at J4               */  \
    "   psraw       $4,"r3"\n"      /* shift                        */  \
    "   movq        "r3","I(3)"\n"  /* store R3 at I3               */  \
    "   psubw       "r1","r2"\n"    /* r2 = E + A. - B. ----R2      */  \
    "   psubw       "r5","r6"\n"    /* r6 = E - A. - B. ----R6      */  \
    "   paddw       "r1","r1"\n"    /* r1 = B. + B.                 */  \
    "   paddw       "r5","r5"\n"    /* r5 = B. + B.                 */  \
    "   paddw       "r7","r0"\n"    /* r0 = E + A ----R0            */  \
    "   paddw       "r2","r1"\n"    /* r1 = E + A. + B. -----R1     */  \
    "   psraw       $4,"r7"\n"      /* shift                        */  \
    "   psraw       $4,"r2"\n"      /* shift                        */  \
    "   psraw       $4,"r0"\n"      /* shift                        */  \
    "   psraw       $4,"r1"\n"      /* shift                        */  \
    "   movq        "r7","J(7)"\n"  /* store R7 to J7               */  \
    "   movq        "r0","I(0)"\n"  /* store R0 to I0               */  \
    "   movq        "r1","I(1)"\n"  /* store R1 to I1               */  \
    "   movq        "r2","I(2)"\n"  /* store R2 to I2               */  \
    "   movq        "r1","I(1)"\n"  /* save r1                      */  \
    "   paddw       "r6","r5"\n"    /* r5 = E - A. + B. -----R5     */  \
    "   psraw       $4,"r5"\n"      /* shift                        */  \
    "   movq        "r5","J(5)"\n"  /* store R5 at J5               */  \
    "   psraw       $4,"r6"\n"      /* shift                        */  \
    "   movq        "r6","J(6)"\n"  /* store R6 at J6               */  \
    "#end ColumnIDCT_3\n"\
);
//End of ColumnIDCT_3

void IDct3__mmx( Q_LIST_ENTRY * InputData,
            ogg_int16_t *QuantMatrix,
            ogg_int16_t * OutputData ) {

#   define MIDM(M,I)    MtoSTR(M+I*8(%ecx))
#   define M(I)         MIDM( MaskOffset , I )
#   define MIDC(M,I)    MtoSTR(M+(I-1)*8(%ecx))
#   define C(I)         MIDC( CosineOffset , I )
#   define MIDEight(M)  MtoSTR(M(%ecx))
#   define Eight        MIDEight(EightOffset)

#   define r0   "%mm0"
#   define r1   "%mm1"
#   define r2   "%mm2"
#   define r3   "%mm3"
#   define r4   "%mm4"
#   define r5   "%mm5"
#   define r6   "%mm6"
#   define r7   "%mm7"

    __asm__ __volatile__ (
    /* eax = quantized input */
    /* esi = quantization table */
    /* edx = destination (= idct buffer) */
    /* ecx = idctconstants */
    ""
    :
    :"a"(InputData), "S"(QuantMatrix), "d"(OutputData), "c"(idctconstants)
    );

    ASM(
    "movq   (%eax), "r0"\n"
    "pmullw (%esi), "r0"\n"     /* r0 = 03 02 01 00 */
    "movq   "M(0)", "r2"\n"     /* r2 = __ __ __ FF */
    "movq   "r0", "r3"\n"       /* r3 = 03 02 01 00 */
    "psrlq  $16, "r0"\n"        /* r0 = __ 03 02 01 */
    "pand   "r2", "r3"\n"       /* r3 = __ __ __ 00 */
    "movq   "r0", "r5"\n"       /* r5 = __ 03 02 01 */
    "pand   "r2", "r5"\n"       /* r5 = __ __ __ 01 */
    "pxor   "r5", "r0"\n"       /* r0 = __ 03 02 __ */
    "por    "r3", "r0"\n"       /* r0 = __ 03 02 00 */
    "movq   "r0", (%edx)\n"     /* write R0 = r0 */
    "movq   "r5", 16(%edx)\n"   /* write R1 = r5 */
    );
#   undef M

/* Done partial transpose; now do the idct itself. */

#   define I( K)    MtoSTR(K*16(%edx))
#   define J( K)    MtoSTR(((K - 4)*16)+8(%edx))

    RowIDCT_3       /* 33 c */
    Transpose       /* 19 c */

#   undef I
#   undef J
//# define I( K)    [edx + (  K      * 16) + 64]
//# define J( K)    [edx + ( (K - 4) * 16) + 72]

//  RowIDCT         ; 46 c
//  Transpose       ; 19 c

//# undef I
//# undef J
#   define I( K)    MtoSTR((K * 16)(%edx))
#   define J( K)    I( K)

    ColumnIDCT_3    /* 44 c */

#   undef I
#   undef J
#   define I( K)    MtoSTR((K*16)+8(%edx))
#   define J( K)    I( K)

    ColumnIDCT_3    /* 44 c */

#   undef I
#   undef J

    ASM("emms\n");
}


/* install our implementation in the function table */
void dsp_mmx_idct_init(DspFunctions *funcs)
{
  funcs->IDctSlow = IDctSlow__mmx;
  funcs->IDct10 = IDct10__mmx;
  funcs->IDct3 = IDct3__mmx;
}

#endif /* USE_ASM */
