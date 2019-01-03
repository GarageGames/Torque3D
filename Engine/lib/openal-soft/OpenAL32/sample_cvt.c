
#include "config.h"

#include "sample_cvt.h"

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
const ALshort muLawDecompressionTable[256] = {
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


/* A quick'n'dirty lookup table to decode an aLaw-encoded byte sample into a
 * signed 16-bit sample */
const ALshort aLawDecompressionTable[256] = {
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


static void DecodeIMA4Block(ALshort *dst, const ALubyte *src, ALint numchans, ALsizei align)
{
    ALint sample[MAX_INPUT_CHANNELS] = { 0 };
    ALint index[MAX_INPUT_CHANNELS] = { 0 };
    ALuint code[MAX_INPUT_CHANNELS] = { 0 };
    ALsizei c, i;

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

    for(i = 1;i < align;i++)
    {
        if((i&7) == 1)
        {
            for(c = 0;c < numchans;c++)
            {
                code[c]  = *(src++);
                code[c] |= *(src++) << 8;
                code[c] |= *(src++) << 16;
                code[c] |= *(src++) << 24;
            }
        }

        for(c = 0;c < numchans;c++)
        {
            int nibble = code[c]&0xf;
            code[c] >>= 4;

            sample[c] += IMA4Codeword[nibble] * IMAStep_size[index[c]] / 8;
            sample[c] = clampi(sample[c], -32768, 32767);

            index[c] += IMA4Index_adjust[nibble];
            index[c] = clampi(index[c], 0, 88);

            *(dst++) = sample[c];
        }
    }
}

static void DecodeMSADPCMBlock(ALshort *dst, const ALubyte *src, ALint numchans, ALsizei align)
{
    ALubyte blockpred[MAX_INPUT_CHANNELS] = { 0 };
    ALint delta[MAX_INPUT_CHANNELS] = { 0 };
    ALshort samples[MAX_INPUT_CHANNELS][2] = { { 0, 0 } };
    ALint c, i;

    for(c = 0;c < numchans;c++)
    {
        blockpred[c] = *(src++);
        blockpred[c] = minu(blockpred[c], 6);
    }
    for(c = 0;c < numchans;c++)
    {
        delta[c]  = *(src++);
        delta[c] |= *(src++) << 8;
        delta[c]  = (delta[c]^0x8000) - 32768;
    }
    for(c = 0;c < numchans;c++)
    {
        samples[c][0]  = *(src++);
        samples[c][0] |= *(src++) << 8;
        samples[c][0]  = (samples[c][0]^0x8000) - 32768;
    }
    for(c = 0;c < numchans;c++)
    {
        samples[c][1]  = *(src++);
        samples[c][1] |= *(src++) << 8;
        samples[c][1]  = (samples[c][1]^0x8000) - 0x8000;
    }

    /* Second sample is written first. */
    for(c = 0;c < numchans;c++)
        *(dst++) = samples[c][1];
    for(c = 0;c < numchans;c++)
        *(dst++) = samples[c][0];

    for(i = 2;i < align;i++)
    {
        for(c = 0;c < numchans;c++)
        {
            const ALint num = (i*numchans) + c;
            ALint nibble, pred;

            /* Read the nibble (first is in the upper bits). */
            if(!(num&1))
                nibble = (*src>>4)&0x0f;
            else
                nibble = (*(src++))&0x0f;

            pred  = (samples[c][0]*MSADPCMAdaptionCoeff[blockpred[c]][0] +
                     samples[c][1]*MSADPCMAdaptionCoeff[blockpred[c]][1]) / 256;
            pred += ((nibble^0x08) - 0x08) * delta[c];
            pred  = clampi(pred, -32768, 32767);

            samples[c][1] = samples[c][0];
            samples[c][0] = pred;

            delta[c] = (MSADPCMAdaption[nibble] * delta[c]) / 256;
            delta[c] = maxi(16, delta[c]);

            *(dst++) = pred;
        }
    }
}


void Convert_ALshort_ALima4(ALshort *dst, const ALubyte *src, ALsizei numchans, ALsizei len,
                            ALsizei align)
{
    ALsizei byte_align = ((align-1)/2 + 4) * numchans;
    ALsizei i;

    assert(align > 0 && (len%align) == 0);
    for(i = 0;i < len;i += align)
    {
        DecodeIMA4Block(dst, src, numchans, align);
        src += byte_align;
        dst += align*numchans;
    }
}

void Convert_ALshort_ALmsadpcm(ALshort *dst, const ALubyte *src, ALsizei numchans, ALsizei len,
                               ALsizei align)
{
    ALsizei byte_align = ((align-2)/2 + 7) * numchans;
    ALsizei i;

    assert(align > 1 && (len%align) == 0);
    for(i = 0;i < len;i += align)
    {
        DecodeMSADPCMBlock(dst, src, numchans, align);
        src += byte_align;
        dst += align*numchans;
    }
}
