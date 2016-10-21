
#include "config.h"

#include "sample_cvt.h"

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "AL/al.h"
#include "alu.h"
#include "alBuffer.h"


/* IMA ADPCM Stepsize table */
static const int IMAStep_size[89] = {
       7,    8,    9,   10,   11,   12,   13,   14,   16,   17,   19,
      21,   23,   25,   28,   31,   34,   37,   41,   45,   50,   55,
      60,   66,   73,   80,   88,   97,  107,  118,  130,  143,  157,
     173,  190,  209,  230,  253,  279,  307,  337,  371,  408,  449,
     494,  544,  598,  658,  724,  796,  876,  963, 1060, 1166, 1282,
    1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660,
    4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493,10442,
   11487,12635,13899,15289,16818,18500,20350,22358,24633,27086,29794,
   32767
};

/* IMA4 ADPCM Codeword decode table */
static const int IMA4Codeword[16] = {
    1, 3, 5, 7, 9, 11, 13, 15,
   -1,-3,-5,-7,-9,-11,-13,-15,
};

/* IMA4 ADPCM Step index adjust decode table */
static const int IMA4Index_adjust[16] = {
   -1,-1,-1,-1, 2, 4, 6, 8,
   -1,-1,-1,-1, 2, 4, 6, 8
};


/* MSADPCM Adaption table */
static const int MSADPCMAdaption[16] = {
    230, 230, 230, 230, 307, 409, 512, 614,
    768, 614, 512, 409, 307, 230, 230, 230
};

/* MSADPCM Adaption Coefficient tables */
static const int MSADPCMAdaptionCoeff[7][2] = {
    { 256,    0 },
    { 512, -256 },
    {   0,    0 },
    { 192,   64 },
    { 240,    0 },
    { 460, -208 },
    { 392, -232 }
};


/* A quick'n'dirty lookup table to decode a muLaw-encoded byte sample into a
 * signed 16-bit sample */
static const ALshort muLawDecompressionTable[256] = {
    -32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956,
    -23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764,
    -15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412,
    -11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316,
     -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
     -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
     -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
     -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
     -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
     -1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,
      -876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,
      -620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,
      -372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,
      -244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,
      -120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,
       -56,   -48,   -40,   -32,   -24,   -16,    -8,     0,
     32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
     23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
     15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
     11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,
      7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,
      5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,
      3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,
      2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,
      1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,
      1372,  1308,  1244,  1180,  1116,  1052,   988,   924,
       876,   844,   812,   780,   748,   716,   684,   652,
       620,   588,   556,   524,   492,   460,   428,   396,
       372,   356,   340,   324,   308,   292,   276,   260,
       244,   228,   212,   196,   180,   164,   148,   132,
       120,   112,   104,    96,    88,    80,    72,    64,
        56,    48,    40,    32,    24,    16,     8,     0
};

/* Values used when encoding a muLaw sample */
static const int muLawBias = 0x84;
static const int muLawClip = 32635;
static const char muLawCompressTable[256] = {
     0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
     4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
     5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
     5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
     6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
     6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
     6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
     6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};


/* A quick'n'dirty lookup table to decode an aLaw-encoded byte sample into a
 * signed 16-bit sample */
static const ALshort aLawDecompressionTable[256] = {
     -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736,
     -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784,
     -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
     -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
    -22016,-20992,-24064,-23040,-17920,-16896,-19968,-18944,
    -30208,-29184,-32256,-31232,-26112,-25088,-28160,-27136,
    -11008,-10496,-12032,-11520, -8960, -8448, -9984, -9472,
    -15104,-14592,-16128,-15616,-13056,-12544,-14080,-13568,
      -344,  -328,  -376,  -360,  -280,  -264,  -312,  -296,
      -472,  -456,  -504,  -488,  -408,  -392,  -440,  -424,
       -88,   -72,  -120,  -104,   -24,    -8,   -56,   -40,
      -216,  -200,  -248,  -232,  -152,  -136,  -184,  -168,
     -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
     -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
      -688,  -656,  -752,  -720,  -560,  -528,  -624,  -592,
      -944,  -912, -1008,  -976,  -816,  -784,  -880,  -848,
      5504,  5248,  6016,  5760,  4480,  4224,  4992,  4736,
      7552,  7296,  8064,  7808,  6528,  6272,  7040,  6784,
      2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368,
      3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392,
     22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944,
     30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136,
     11008, 10496, 12032, 11520,  8960,  8448,  9984,  9472,
     15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568,
       344,   328,   376,   360,   280,   264,   312,   296,
       472,   456,   504,   488,   408,   392,   440,   424,
        88,    72,   120,   104,    24,     8,    56,    40,
       216,   200,   248,   232,   152,   136,   184,   168,
      1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184,
      1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696,
       688,   656,   752,   720,   560,   528,   624,   592,
       944,   912,  1008,   976,   816,   784,   880,   848
};

/* Values used when encoding an aLaw sample */
static const int aLawClip = 32635;
static const char aLawCompressTable[128] = {
    1,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};


typedef ALubyte ALmulaw;
typedef ALubyte ALalaw;
typedef ALubyte ALima4;
typedef ALubyte ALmsadpcm;
typedef struct {
    ALbyte b[3];
} ALbyte3;
static_assert(sizeof(ALbyte3)==sizeof(ALbyte[3]), "ALbyte3 size is not 3");
typedef struct {
    ALubyte b[3];
} ALubyte3;
static_assert(sizeof(ALubyte3)==sizeof(ALubyte[3]), "ALubyte3 size is not 3");

static inline ALshort DecodeMuLaw(ALmulaw val)
{ return muLawDecompressionTable[val]; }

static ALmulaw EncodeMuLaw(ALshort val)
{
    ALint mant, exp, sign;

    sign = (val>>8) & 0x80;
    if(sign)
    {
        /* -32768 doesn't properly negate on a short; it results in itself.
         * So clamp to -32767 */
        val = maxi(val, -32767);
        val = -val;
    }

    val = mini(val, muLawClip);
    val += muLawBias;

    exp = muLawCompressTable[(val>>7) & 0xff];
    mant = (val >> (exp+3)) & 0x0f;

    return ~(sign | (exp<<4) | mant);
}

static inline ALshort DecodeALaw(ALalaw val)
{ return aLawDecompressionTable[val]; }

static ALalaw EncodeALaw(ALshort val)
{
    ALint mant, exp, sign;

    sign = ((~val) >> 8) & 0x80;
    if(!sign)
    {
        val = maxi(val, -32767);
        val = -val;
    }
    val = mini(val, aLawClip);

    if(val >= 256)
    {
        exp = aLawCompressTable[(val>>8) & 0x7f];
        mant = (val >> (exp+3)) & 0x0f;
    }
    else
    {
        exp = 0;
        mant = val >> 4;
    }

    return ((exp<<4) | mant) ^ (sign^0x55);
}

static void DecodeIMA4Block(ALshort *dst, const ALima4 *src, ALint numchans, ALsizei align)
{
    ALint sample[MAX_INPUT_CHANNELS], index[MAX_INPUT_CHANNELS];
    ALuint code[MAX_INPUT_CHANNELS];
    ALsizei j,k,c;

    for(c = 0;c < numchans;c++)
    {
        sample[c]  = *(src++);
        sample[c] |= *(src++) << 8;
        sample[c]  = (sample[c]^0x8000) - 32768;
        index[c]  = *(src++);
        index[c] |= *(src++) << 8;
        index[c]  = (index[c]^0x8000) - 32768;

        index[c] = clampi(index[c], 0, 88);

        dst[c] = sample[c];
    }

    for(j = 1;j < align;j += 8)
    {
        for(c = 0;c < numchans;c++)
        {
            code[c]  = *(src++);
            code[c] |= *(src++) << 8;
            code[c] |= *(src++) << 16;
            code[c] |= *(src++) << 24;
        }

        for(k = 0;k < 8;k++)
        {
            for(c = 0;c < numchans;c++)
            {
                int nibble = code[c]&0xf;
                code[c] >>= 4;

                sample[c] += IMA4Codeword[nibble] * IMAStep_size[index[c]] / 8;
                sample[c] = clampi(sample[c], -32768, 32767);

                index[c] += IMA4Index_adjust[nibble];
                index[c] = clampi(index[c], 0, 88);

                dst[(j+k)*numchans + c] = sample[c];
            }
        }
    }
}

static void EncodeIMA4Block(ALima4 *dst, const ALshort *src, ALint *sample, ALint *index, ALint numchans, ALsizei align)
{
    ALsizei j,k,c;

    for(c = 0;c < numchans;c++)
    {
        int diff = src[c] - sample[c];
        int step = IMAStep_size[index[c]];
        int nibble;

        nibble = 0;
        if(diff < 0)
        {
            nibble = 0x8;
            diff = -diff;
        }

        diff = mini(step*2, diff);
        nibble |= (diff*8/step - 1) / 2;

        sample[c] += IMA4Codeword[nibble] * step / 8;
        sample[c] = clampi(sample[c], -32768, 32767);

        index[c] += IMA4Index_adjust[nibble];
        index[c] = clampi(index[c], 0, 88);

        *(dst++) = sample[c] & 0xff;
        *(dst++) = (sample[c]>>8) & 0xff;
        *(dst++) = index[c] & 0xff;
        *(dst++) = (index[c]>>8) & 0xff;
    }

    for(j = 1;j < align;j += 8)
    {
        for(c = 0;c < numchans;c++)
        {
            for(k = 0;k < 8;k++)
            {
                int diff = src[(j+k)*numchans + c] - sample[c];
                int step = IMAStep_size[index[c]];
                int nibble;

                nibble = 0;
                if(diff < 0)
                {
                    nibble = 0x8;
                    diff = -diff;
                }

                diff = mini(step*2, diff);
                nibble |= (diff*8/step - 1) / 2;

                sample[c] += IMA4Codeword[nibble] * step / 8;
                sample[c] = clampi(sample[c], -32768, 32767);

                index[c] += IMA4Index_adjust[nibble];
                index[c] = clampi(index[c], 0, 88);

                if(!(k&1)) *dst = nibble;
                else *(dst++) |= nibble<<4;
            }
        }
    }
}


static void DecodeMSADPCMBlock(ALshort *dst, const ALmsadpcm *src, ALint numchans, ALsizei align)
{
    ALubyte blockpred[MAX_INPUT_CHANNELS];
    ALint delta[MAX_INPUT_CHANNELS];
    ALshort samples[MAX_INPUT_CHANNELS][2];
    ALint i, j;

    for(i = 0;i < numchans;i++)
    {
        blockpred[i] = *(src++);
        blockpred[i] = minu(blockpred[i], 6);
    }
    for(i = 0;i < numchans;i++)
    {
        delta[i]  = *(src++);
        delta[i] |= *(src++) << 8;
        delta[i]  = (delta[i]^0x8000) - 0x8000;
    }
    for(i = 0;i < numchans;i++)
    {
        samples[i][0]  = *(src++);
        samples[i][0] |= *(src++) << 8;
        samples[i][0]  = (samples[i][0]^0x8000) - 0x8000;
    }
    for(i = 0;i < numchans;i++)
    {
        samples[i][1]  = *(src++);
        samples[i][1] |= *(src++) << 8;
        samples[i][1]  = (samples[i][1]^0x8000) - 0x8000;
    }

    /* Second sample is written first. */
    for(i = 0;i < numchans;i++)
        *(dst++) = samples[i][1];
    for(i = 0;i < numchans;i++)
        *(dst++) = samples[i][0];

    for(j = 2;j < align;j++)
    {
        for(i = 0;i < numchans;i++)
        {
            const ALint num = (j*numchans) + i;
            ALint nibble, pred;

            /* Read the nibble (first is in the upper bits). */
            if(!(num&1))
                nibble = (*src>>4)&0x0f;
            else
                nibble = (*(src++))&0x0f;

            pred  = (samples[i][0]*MSADPCMAdaptionCoeff[blockpred[i]][0] +
                     samples[i][1]*MSADPCMAdaptionCoeff[blockpred[i]][1]) / 256;
            pred += ((nibble^0x08) - 0x08) * delta[i];
            pred  = clampi(pred, -32768, 32767);

            samples[i][1] = samples[i][0];
            samples[i][0] = pred;

            delta[i] = (MSADPCMAdaption[nibble] * delta[i]) / 256;
            delta[i] = maxi(16, delta[i]);

            *(dst++) = pred;
        }
    }
}

/* NOTE: This encoder is pretty dumb/simplistic. Some kind of pre-processing
 * that tries to find the optimal block predictors would be nice, at least. A
 * multi-pass method that can generate better deltas would be good, too. */
static void EncodeMSADPCMBlock(ALmsadpcm *dst, const ALshort *src, ALint *sample, ALint numchans, ALsizei align)
{
    ALubyte blockpred[MAX_INPUT_CHANNELS];
    ALint delta[MAX_INPUT_CHANNELS];
    ALshort samples[MAX_INPUT_CHANNELS][2];
    ALint i, j;

    /* Block predictor */
    for(i = 0;i < numchans;i++)
    {
        /* FIXME: Calculate something better. */
        blockpred[i] = 0;
        *(dst++) = blockpred[i];
    }
    /* Initial delta */
    for(i = 0;i < numchans;i++)
    {
        delta[i] = 16;
        *(dst++) = (delta[i]   ) & 0xff;
        *(dst++) = (delta[i]>>8) & 0xff;
    }
    /* Initial sample 1 */
    for(i = 0;i < numchans;i++)
    {
        samples[i][0] = src[1*numchans + i];
        *(dst++) = (samples[i][0]   ) & 0xff;
        *(dst++) = (samples[i][0]>>8) & 0xff;
    }
    /* Initial sample 2 */
    for(i = 0;i < numchans;i++)
    {
        samples[i][1] = src[i];
        *(dst++) = (samples[i][1]   ) & 0xff;
        *(dst++) = (samples[i][1]>>8) & 0xff;
    }

    for(j = 2;j < align;j++)
    {
        for(i = 0;i < numchans;i++)
        {
            const ALint num = (j*numchans) + i;
            ALint nibble = 0;
            ALint bias;

            sample[i] = (samples[i][0]*MSADPCMAdaptionCoeff[blockpred[i]][0] +
                         samples[i][1]*MSADPCMAdaptionCoeff[blockpred[i]][1]) / 256;

            nibble = src[num] - sample[i];
            if(nibble >= 0)
                bias = delta[i] / 2;
            else
                bias = -delta[i] / 2;

            nibble = (nibble + bias) / delta[i];
            nibble = clampi(nibble, -8, 7)&0x0f;

            sample[i] += ((nibble^0x08)-0x08) * delta[i];
            sample[i]  = clampi(sample[i], -32768, 32767);

            samples[i][1] = samples[i][0];
            samples[i][0] = sample[i];

            delta[i] = (MSADPCMAdaption[nibble] * delta[i]) / 256;
            delta[i] = maxi(16, delta[i]);

            if(!(num&1))
                *dst = nibble << 4;
            else
            {
                *dst |= nibble;
                dst++;
            }
        }
    }
}


static inline ALint DecodeByte3(ALbyte3 val)
{
    if(IS_LITTLE_ENDIAN)
        return (val.b[2]<<16) | (((ALubyte)val.b[1])<<8) | ((ALubyte)val.b[0]);
    return (val.b[0]<<16) | (((ALubyte)val.b[1])<<8) | ((ALubyte)val.b[2]);
}

static inline ALbyte3 EncodeByte3(ALint val)
{
    if(IS_LITTLE_ENDIAN)
    {
        ALbyte3 ret = {{ val, val>>8, val>>16 }};
        return ret;
    }
    else
    {
        ALbyte3 ret = {{ val>>16, val>>8, val }};
        return ret;
    }
}

static inline ALint DecodeUByte3(ALubyte3 val)
{
    if(IS_LITTLE_ENDIAN)
        return (val.b[2]<<16) | (val.b[1]<<8) | (val.b[0]);
    return (val.b[0]<<16) | (val.b[1]<<8) | val.b[2];
}

static inline ALubyte3 EncodeUByte3(ALint val)
{
    if(IS_LITTLE_ENDIAN)
    {
        ALubyte3 ret = {{ val, val>>8, val>>16 }};
        return ret;
    }
    else
    {
        ALubyte3 ret = {{ val>>16, val>>8, val }};
        return ret;
    }
}


/* Define same-type pass-through sample conversion functions (excludes ADPCM,
 * which are block-based). */
#define DECL_TEMPLATE(T) \
static inline T Conv_##T##_##T(T val) { return val; }

DECL_TEMPLATE(ALbyte);
DECL_TEMPLATE(ALubyte);
DECL_TEMPLATE(ALshort);
DECL_TEMPLATE(ALushort);
DECL_TEMPLATE(ALint);
DECL_TEMPLATE(ALuint);
DECL_TEMPLATE(ALbyte3);
DECL_TEMPLATE(ALubyte3);
DECL_TEMPLATE(ALalaw);
DECL_TEMPLATE(ALmulaw);

/* Slightly special handling for floats and doubles (converts NaN to 0, and
 * allows float<->double pass-through).
 */
static inline ALfloat Conv_ALfloat_ALfloat(ALfloat val)
{ return (val==val) ? val : 0.0f; }
static inline ALfloat Conv_ALfloat_ALdouble(ALdouble val)
{ return (val==val) ? (ALfloat)val : 0.0f; }
static inline ALdouble Conv_ALdouble_ALfloat(ALfloat val)
{ return (val==val) ? (ALdouble)val : 0.0; }
static inline ALdouble Conv_ALdouble_ALdouble(ALdouble val)
{ return (val==val) ? val : 0.0; }

#undef DECL_TEMPLATE

/* Define alternate-sign functions. */
#define DECL_TEMPLATE(T1, T2, O)                                  \
static inline T1 Conv_##T1##_##T2(T2 val) { return (T1)val - O; } \
static inline T2 Conv_##T2##_##T1(T1 val) { return (T2)val + O; }

DECL_TEMPLATE(ALbyte, ALubyte, 128);
DECL_TEMPLATE(ALshort, ALushort, 32768);
DECL_TEMPLATE(ALint, ALuint, 2147483648u);

#undef DECL_TEMPLATE

/* Define int-type to int-type functions */
#define DECL_TEMPLATE(T, ST, UT, SH)                                          \
static inline T Conv_##T##_##ST(ST val){ return val >> SH; }                  \
static inline T Conv_##T##_##UT(UT val){ return Conv_##ST##_##UT(val) >> SH; }\
static inline ST Conv_##ST##_##T(T val){ return val << SH; }                  \
static inline UT Conv_##UT##_##T(T val){ return Conv_##UT##_##ST(val << SH); }

#define DECL_TEMPLATE2(T1, T2, SH)          \
DECL_TEMPLATE(AL##T1, AL##T2, ALu##T2, SH)  \
DECL_TEMPLATE(ALu##T1, ALu##T2, AL##T2, SH)

DECL_TEMPLATE2(byte,  short, 8)
DECL_TEMPLATE2(short, int,   16)
DECL_TEMPLATE2(byte,  int,   24)

#undef DECL_TEMPLATE2
#undef DECL_TEMPLATE

/* Define int-type to fp functions */
#define DECL_TEMPLATE(T, ST, UT, OP)                                          \
static inline T Conv_##T##_##ST(ST val) { return (T)val * OP; }               \
static inline T Conv_##T##_##UT(UT val) { return (T)Conv_##ST##_##UT(val) * OP; }

#define DECL_TEMPLATE2(T1, T2, OP)     \
DECL_TEMPLATE(T1, AL##T2, ALu##T2, OP)

DECL_TEMPLATE2(ALfloat, byte, (1.0f/127.0f))
DECL_TEMPLATE2(ALdouble, byte, (1.0/127.0))
DECL_TEMPLATE2(ALfloat, short, (1.0f/32767.0f))
DECL_TEMPLATE2(ALdouble, short, (1.0/32767.0))
DECL_TEMPLATE2(ALdouble, int, (1.0/2147483647.0))

/* Special handling for int32 to float32, since it would overflow. */
static inline ALfloat Conv_ALfloat_ALint(ALint val)
{ return (ALfloat)(val>>7) * (1.0f/16777215.0f); }
static inline ALfloat Conv_ALfloat_ALuint(ALuint val)
{ return (ALfloat)(Conv_ALint_ALuint(val)>>7) * (1.0f/16777215.0f); }

#undef DECL_TEMPLATE2
#undef DECL_TEMPLATE

/* Define fp to int-type functions */
#define DECL_TEMPLATE(FT, T, smin, smax)        \
static inline AL##T Conv_AL##T##_##FT(FT val)   \
{                                               \
    if(val > 1.0f) return smax;                 \
    if(val < -1.0f) return smin;                \
    return (AL##T)(val * (FT)smax);             \
}                                               \
static inline ALu##T Conv_ALu##T##_##FT(FT val) \
{ return Conv_ALu##T##_AL##T(Conv_AL##T##_##FT(val)); }

DECL_TEMPLATE(ALfloat, byte, -128, 127)
DECL_TEMPLATE(ALdouble, byte, -128, 127)
DECL_TEMPLATE(ALfloat, short, -32768, 32767)
DECL_TEMPLATE(ALdouble, short, -32768, 32767)
DECL_TEMPLATE(ALdouble, int, -2147483647-1, 2147483647)

/* Special handling for float32 to int32, since it would overflow. */
static inline ALint Conv_ALint_ALfloat(ALfloat val)
{
    if(val > 1.0f) return 2147483647;
    if(val < -1.0f) return -2147483647-1;
    return (ALint)(val * 16777215.0f) << 7;
}
static inline ALuint Conv_ALuint_ALfloat(ALfloat val)
{ return Conv_ALuint_ALint(Conv_ALint_ALfloat(val)); }

#undef DECL_TEMPLATE

/* Define byte3 and ubyte3 functions (goes through int and uint functions). */
#define DECL_TEMPLATE(T)                            \
static inline ALbyte3 Conv_ALbyte3_##T(T val)       \
{ return EncodeByte3(Conv_ALint_##T(val)>>8); }     \
static inline T Conv_##T##_ALbyte3(ALbyte3 val)     \
{ return Conv_##T##_ALint(DecodeByte3(val)<<8); }   \
                                                    \
static inline ALubyte3 Conv_ALubyte3_##T(T val)     \
{ return EncodeUByte3(Conv_ALuint_##T(val)>>8); }   \
static inline T Conv_##T##_ALubyte3(ALubyte3 val)   \
{ return Conv_##T##_ALuint(DecodeUByte3(val)<<8); }

DECL_TEMPLATE(ALbyte)
DECL_TEMPLATE(ALubyte)
DECL_TEMPLATE(ALshort)
DECL_TEMPLATE(ALushort)
DECL_TEMPLATE(ALint)
DECL_TEMPLATE(ALuint)
DECL_TEMPLATE(ALfloat)
DECL_TEMPLATE(ALdouble)

#undef DECL_TEMPLATE

/* Define byte3 <-> ubyte3 functions. */
static inline ALbyte3 Conv_ALbyte3_ALubyte3(ALubyte3 val)
{ return EncodeByte3(DecodeUByte3(val)-8388608); }
static inline ALubyte3 Conv_ALubyte3_ALbyte3(ALbyte3 val)
{ return EncodeUByte3(DecodeByte3(val)+8388608); }

/* Define muLaw and aLaw functions (goes through short functions). */
#define DECL_TEMPLATE(T)                                                      \
static inline ALmulaw Conv_ALmulaw_##T(T val)                                 \
{ return EncodeMuLaw(Conv_ALshort_##T(val)); }                                \
static inline T Conv_##T##_ALmulaw(ALmulaw val)                               \
{ return Conv_##T##_ALshort(DecodeMuLaw(val)); }                              \
                                                                              \
static inline ALalaw Conv_ALalaw_##T(T val)                                   \
{ return EncodeALaw(Conv_ALshort_##T(val)); }                                 \
static inline T Conv_##T##_ALalaw(ALalaw val)                                 \
{ return Conv_##T##_ALshort(DecodeALaw(val)); }

DECL_TEMPLATE(ALbyte)
DECL_TEMPLATE(ALubyte)
DECL_TEMPLATE(ALshort)
DECL_TEMPLATE(ALushort)
DECL_TEMPLATE(ALint)
DECL_TEMPLATE(ALuint)
DECL_TEMPLATE(ALfloat)
DECL_TEMPLATE(ALdouble)
DECL_TEMPLATE(ALbyte3)
DECL_TEMPLATE(ALubyte3)

#undef DECL_TEMPLATE

/* Define muLaw <-> aLaw functions. */
static inline ALalaw Conv_ALalaw_ALmulaw(ALmulaw val)
{ return EncodeALaw(DecodeMuLaw(val)); }
static inline ALmulaw Conv_ALmulaw_ALalaw(ALalaw val)
{ return EncodeMuLaw(DecodeALaw(val)); }


#define DECL_TEMPLATE(T1, T2)                                                 \
static void Convert_##T1##_##T2(T1 *dst, const T2 *src, ALuint numchans,      \
                                ALuint len, ALsizei UNUSED(align))            \
{                                                                             \
    ALuint i, j;                                                              \
    for(i = 0;i < len;i++)                                                    \
    {                                                                         \
        for(j = 0;j < numchans;j++)                                           \
            *(dst++) = Conv_##T1##_##T2(*(src++));                            \
    }                                                                         \
}

#define DECL_TEMPLATE2(T)  \
DECL_TEMPLATE(T, ALbyte)   \
DECL_TEMPLATE(T, ALubyte)  \
DECL_TEMPLATE(T, ALshort)  \
DECL_TEMPLATE(T, ALushort) \
DECL_TEMPLATE(T, ALint)    \
DECL_TEMPLATE(T, ALuint)   \
DECL_TEMPLATE(T, ALfloat)  \
DECL_TEMPLATE(T, ALdouble) \
DECL_TEMPLATE(T, ALmulaw)  \
DECL_TEMPLATE(T, ALalaw)   \
DECL_TEMPLATE(T, ALbyte3)  \
DECL_TEMPLATE(T, ALubyte3)

DECL_TEMPLATE2(ALbyte)
DECL_TEMPLATE2(ALubyte)
DECL_TEMPLATE2(ALshort)
DECL_TEMPLATE2(ALushort)
DECL_TEMPLATE2(ALint)
DECL_TEMPLATE2(ALuint)
DECL_TEMPLATE2(ALfloat)
DECL_TEMPLATE2(ALdouble)
DECL_TEMPLATE2(ALmulaw)
DECL_TEMPLATE2(ALalaw)
DECL_TEMPLATE2(ALbyte3)
DECL_TEMPLATE2(ALubyte3)

#undef DECL_TEMPLATE2
#undef DECL_TEMPLATE

#define DECL_TEMPLATE(T)                                                      \
static void Convert_##T##_ALima4(T *dst, const ALima4 *src, ALuint numchans,  \
                                 ALuint len, ALuint align)                    \
{                                                                             \
    ALsizei byte_align = ((align-1)/2 + 4) * numchans;                        \
    DECL_VLA(ALshort, tmp, align*numchans);                                   \
    ALuint i, j, k;                                                           \
                                                                              \
    assert(align > 0 && (len%align) == 0);                                    \
    for(i = 0;i < len;i += align)                                             \
    {                                                                         \
        DecodeIMA4Block(tmp, src, numchans, align);                           \
        src += byte_align;                                                    \
                                                                              \
        for(j = 0;j < align;j++)                                              \
        {                                                                     \
            for(k = 0;k < numchans;k++)                                       \
                *(dst++) = Conv_##T##_ALshort(tmp[j*numchans + k]);           \
        }                                                                     \
    }                                                                         \
}

DECL_TEMPLATE(ALbyte)
DECL_TEMPLATE(ALubyte)
static void Convert_ALshort_ALima4(ALshort *dst, const ALima4 *src, ALuint numchans,
                                   ALuint len, ALuint align)
{
    ALsizei byte_align = ((align-1)/2 + 4) * numchans;
    ALuint i;

    assert(align > 0 && (len%align) == 0);
    for(i = 0;i < len;i += align)
    {
        DecodeIMA4Block(dst, src, numchans, align);
        src += byte_align;
        dst += align*numchans;
    }
}
DECL_TEMPLATE(ALushort)
DECL_TEMPLATE(ALint)
DECL_TEMPLATE(ALuint)
DECL_TEMPLATE(ALfloat)
DECL_TEMPLATE(ALdouble)
DECL_TEMPLATE(ALmulaw)
DECL_TEMPLATE(ALalaw)
DECL_TEMPLATE(ALbyte3)
DECL_TEMPLATE(ALubyte3)

#undef DECL_TEMPLATE

#define DECL_TEMPLATE(T)                                                      \
static void Convert_ALima4_##T(ALima4 *dst, const T *src, ALuint numchans,    \
                               ALuint len, ALuint align)                      \
{                                                                             \
    ALint sample[MAX_INPUT_CHANNELS] = {0,0,0,0,0,0,0,0};                     \
    ALint index[MAX_INPUT_CHANNELS] = {0,0,0,0,0,0,0,0};                      \
    ALsizei byte_align = ((align-1)/2 + 4) * numchans;                        \
    DECL_VLA(ALshort, tmp, align*numchans);                                   \
    ALuint i, j, k;                                                           \
                                                                              \
    assert(align > 0 && (len%align) == 0);                                    \
    for(i = 0;i < len;i += align)                                             \
    {                                                                         \
        for(j = 0;j < align;j++)                                              \
        {                                                                     \
            for(k = 0;k < numchans;k++)                                       \
                tmp[j*numchans + k] = Conv_ALshort_##T(*(src++));             \
        }                                                                     \
        EncodeIMA4Block(dst, tmp, sample, index, numchans, align);            \
        dst += byte_align;                                                    \
    }                                                                         \
}

DECL_TEMPLATE(ALbyte)
DECL_TEMPLATE(ALubyte)
static void Convert_ALima4_ALshort(ALima4 *dst, const ALshort *src,
                                   ALuint numchans, ALuint len, ALuint align)
{
    ALint sample[MAX_INPUT_CHANNELS] = {0,0,0,0,0,0,0,0};
    ALint index[MAX_INPUT_CHANNELS] = {0,0,0,0,0,0,0,0};
    ALsizei byte_align = ((align-1)/2 + 4) * numchans;
    ALuint i;

    assert(align > 0 && (len%align) == 0);
    for(i = 0;i < len;i += align)
    {
        EncodeIMA4Block(dst, src, sample, index, numchans, align);
        src += align*numchans;
        dst += byte_align;
    }
}
DECL_TEMPLATE(ALushort)
DECL_TEMPLATE(ALint)
DECL_TEMPLATE(ALuint)
DECL_TEMPLATE(ALfloat)
DECL_TEMPLATE(ALdouble)
DECL_TEMPLATE(ALmulaw)
DECL_TEMPLATE(ALalaw)
DECL_TEMPLATE(ALbyte3)
DECL_TEMPLATE(ALubyte3)

#undef DECL_TEMPLATE


#define DECL_TEMPLATE(T)                                                      \
static void Convert_##T##_ALmsadpcm(T *dst, const ALmsadpcm *src,             \
                                    ALuint numchans, ALuint len,              \
                                    ALuint align)                             \
{                                                                             \
    ALsizei byte_align = ((align-2)/2 + 7) * numchans;                        \
    DECL_VLA(ALshort, tmp, align*numchans);                                   \
    ALuint i, j, k;                                                           \
                                                                              \
    assert(align > 1 && (len%align) == 0);                                    \
    for(i = 0;i < len;i += align)                                             \
    {                                                                         \
        DecodeMSADPCMBlock(tmp, src, numchans, align);                        \
        src += byte_align;                                                    \
                                                                              \
        for(j = 0;j < align;j++)                                              \
        {                                                                     \
            for(k = 0;k < numchans;k++)                                       \
                *(dst++) = Conv_##T##_ALshort(tmp[j*numchans + k]);           \
        }                                                                     \
    }                                                                         \
}

DECL_TEMPLATE(ALbyte)
DECL_TEMPLATE(ALubyte)
static void Convert_ALshort_ALmsadpcm(ALshort *dst, const ALmsadpcm *src,
                                      ALuint numchans, ALuint len,
                                      ALuint align)
{
    ALsizei byte_align = ((align-2)/2 + 7) * numchans;
    ALuint i;

    assert(align > 1 && (len%align) == 0);
    for(i = 0;i < len;i += align)
    {
        DecodeMSADPCMBlock(dst, src, numchans, align);
        src += byte_align;
        dst += align*numchans;
    }
}
DECL_TEMPLATE(ALushort)
DECL_TEMPLATE(ALint)
DECL_TEMPLATE(ALuint)
DECL_TEMPLATE(ALfloat)
DECL_TEMPLATE(ALdouble)
DECL_TEMPLATE(ALmulaw)
DECL_TEMPLATE(ALalaw)
DECL_TEMPLATE(ALbyte3)
DECL_TEMPLATE(ALubyte3)

#undef DECL_TEMPLATE

#define DECL_TEMPLATE(T)                                                      \
static void Convert_ALmsadpcm_##T(ALmsadpcm *dst, const T *src,               \
                                  ALuint numchans, ALuint len, ALuint align)  \
{                                                                             \
    ALint sample[MAX_INPUT_CHANNELS] = {0,0,0,0,0,0,0,0};                     \
    ALsizei byte_align = ((align-2)/2 + 7) * numchans;                        \
    DECL_VLA(ALshort, tmp, align*numchans);                                   \
    ALuint i, j, k;                                                           \
                                                                              \
    assert(align > 1 && (len%align) == 0);                                    \
    for(i = 0;i < len;i += align)                                             \
    {                                                                         \
        for(j = 0;j < align;j++)                                              \
        {                                                                     \
            for(k = 0;k < numchans;k++)                                       \
                tmp[j*numchans + k] = Conv_ALshort_##T(*(src++));             \
        }                                                                     \
        EncodeMSADPCMBlock(dst, tmp, sample, numchans, align);                \
        dst += byte_align;                                                    \
    }                                                                         \
}

DECL_TEMPLATE(ALbyte)
DECL_TEMPLATE(ALubyte)
static void Convert_ALmsadpcm_ALshort(ALmsadpcm *dst, const ALshort *src,
                                      ALuint numchans, ALuint len, ALuint align)
{
    ALint sample[MAX_INPUT_CHANNELS] = {0,0,0,0,0,0,0,0};
    ALsizei byte_align = ((align-2)/2 + 7) * numchans;
    ALuint i;

    assert(align > 1 && (len%align) == 0);
    for(i = 0;i < len;i += align)
    {
        EncodeMSADPCMBlock(dst, src, sample, numchans, align);
        src += align*numchans;
        dst += byte_align;
    }
}
DECL_TEMPLATE(ALushort)
DECL_TEMPLATE(ALint)
DECL_TEMPLATE(ALuint)
DECL_TEMPLATE(ALfloat)
DECL_TEMPLATE(ALdouble)
DECL_TEMPLATE(ALmulaw)
DECL_TEMPLATE(ALalaw)
DECL_TEMPLATE(ALbyte3)
DECL_TEMPLATE(ALubyte3)

#undef DECL_TEMPLATE

/* NOTE: We don't store compressed samples internally, so these conversions
 * should never happen. */
static void Convert_ALima4_ALima4(ALima4* UNUSED(dst), const ALima4* UNUSED(src),
                                  ALuint UNUSED(numchans), ALuint UNUSED(len),
                                  ALuint UNUSED(align))
{
    ERR("Unexpected IMA4-to-IMA4 conversion!\n");
}

static void Convert_ALmsadpcm_ALmsadpcm(ALmsadpcm* UNUSED(dst), const ALmsadpcm* UNUSED(src),
                                        ALuint UNUSED(numchans), ALuint UNUSED(len),
                                        ALuint UNUSED(align))
{
    ERR("Unexpected MSADPCM-to-MSADPCM conversion!\n");
}

static void Convert_ALmsadpcm_ALima4(ALmsadpcm* UNUSED(dst), const ALima4* UNUSED(src),
                                     ALuint UNUSED(numchans), ALuint UNUSED(len),
                                     ALuint UNUSED(align))
{
    ERR("Unexpected IMA4-to-MSADPCM conversion!\n");
}

static void Convert_ALima4_ALmsadpcm(ALima4* UNUSED(dst), const ALmsadpcm* UNUSED(src),
                                     ALuint UNUSED(numchans), ALuint UNUSED(len),
                                     ALuint UNUSED(align))
{
    ERR("Unexpected MSADPCM-to-IMA4 conversion!\n");
}


#define DECL_TEMPLATE(T)                                                      \
static void Convert_##T(T *dst, const ALvoid *src, enum UserFmtType srcType,  \
                        ALsizei numchans, ALsizei len, ALsizei align)         \
{                                                                             \
    switch(srcType)                                                           \
    {                                                                         \
        case UserFmtByte:                                                     \
            Convert_##T##_ALbyte(dst, src, numchans, len, align);             \
            break;                                                            \
        case UserFmtUByte:                                                    \
            Convert_##T##_ALubyte(dst, src, numchans, len, align);            \
            break;                                                            \
        case UserFmtShort:                                                    \
            Convert_##T##_ALshort(dst, src, numchans, len, align);            \
            break;                                                            \
        case UserFmtUShort:                                                   \
            Convert_##T##_ALushort(dst, src, numchans, len, align);           \
            break;                                                            \
        case UserFmtInt:                                                      \
            Convert_##T##_ALint(dst, src, numchans, len, align);              \
            break;                                                            \
        case UserFmtUInt:                                                     \
            Convert_##T##_ALuint(dst, src, numchans, len, align);             \
            break;                                                            \
        case UserFmtFloat:                                                    \
            Convert_##T##_ALfloat(dst, src, numchans, len, align);            \
            break;                                                            \
        case UserFmtDouble:                                                   \
            Convert_##T##_ALdouble(dst, src, numchans, len, align);           \
            break;                                                            \
        case UserFmtMulaw:                                                    \
            Convert_##T##_ALmulaw(dst, src, numchans, len, align);            \
            break;                                                            \
        case UserFmtAlaw:                                                     \
            Convert_##T##_ALalaw(dst, src, numchans, len, align);             \
            break;                                                            \
        case UserFmtIMA4:                                                     \
            Convert_##T##_ALima4(dst, src, numchans, len, align);             \
            break;                                                            \
        case UserFmtMSADPCM:                                                  \
            Convert_##T##_ALmsadpcm(dst, src, numchans, len, align);          \
            break;                                                            \
        case UserFmtByte3:                                                    \
            Convert_##T##_ALbyte3(dst, src, numchans, len, align);            \
            break;                                                            \
        case UserFmtUByte3:                                                   \
            Convert_##T##_ALubyte3(dst, src, numchans, len, align);           \
            break;                                                            \
    }                                                                         \
}

DECL_TEMPLATE(ALbyte)
DECL_TEMPLATE(ALubyte)
DECL_TEMPLATE(ALshort)
DECL_TEMPLATE(ALushort)
DECL_TEMPLATE(ALint)
DECL_TEMPLATE(ALuint)
DECL_TEMPLATE(ALfloat)
DECL_TEMPLATE(ALdouble)
DECL_TEMPLATE(ALmulaw)
DECL_TEMPLATE(ALalaw)
DECL_TEMPLATE(ALima4)
DECL_TEMPLATE(ALmsadpcm)
DECL_TEMPLATE(ALbyte3)
DECL_TEMPLATE(ALubyte3)

#undef DECL_TEMPLATE


void ConvertData(ALvoid *dst, enum UserFmtType dstType, const ALvoid *src, enum UserFmtType srcType, ALsizei numchans, ALsizei len, ALsizei align)
{
    switch(dstType)
    {
        case UserFmtByte:
            Convert_ALbyte(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtUByte:
            Convert_ALubyte(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtShort:
            Convert_ALshort(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtUShort:
            Convert_ALushort(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtInt:
            Convert_ALint(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtUInt:
            Convert_ALuint(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtFloat:
            Convert_ALfloat(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtDouble:
            Convert_ALdouble(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtMulaw:
            Convert_ALmulaw(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtAlaw:
            Convert_ALalaw(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtIMA4:
            Convert_ALima4(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtMSADPCM:
            Convert_ALmsadpcm(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtByte3:
            Convert_ALbyte3(dst, src, srcType, numchans, len, align);
            break;
        case UserFmtUByte3:
            Convert_ALubyte3(dst, src, srcType, numchans, len, align);
            break;
    }
}
