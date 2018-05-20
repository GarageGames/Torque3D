
#include "config.h"

#include "bformatdec.h"
#include "ambdec.h"
#include "filters/splitter.h"
#include "alu.h"

#include "bool.h"
#include "threads.h"
#include "almalloc.h"


/* NOTE: These are scale factors as applied to Ambisonics content. Decoder
 * coefficients should be divided by these values to get proper N3D scalings.
 */
const ALfloat N3D2N3DScale[MAX_AMBI_COEFFS] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
};
const ALfloat SN3D2N3DScale[MAX_AMBI_COEFFS] = {
    1.000000000f, /* ACN  0 (W), sqrt(1) */
    1.732050808f, /* ACN  1 (Y), sqrt(3) */
    1.732050808f, /* ACN  2 (Z), sqrt(3) */
    1.732050808f, /* ACN  3 (X), sqrt(3) */
    2.236067978f, /* ACN  4 (V), sqrt(5) */
    2.236067978f, /* ACN  5 (T), sqrt(5) */
    2.236067978f, /* ACN  6 (R), sqrt(5) */
    2.236067978f, /* ACN  7 (S), sqrt(5) */
    2.236067978f, /* ACN  8 (U), sqrt(5) */
    2.645751311f, /* ACN  9 (Q), sqrt(7) */
    2.645751311f, /* ACN 10 (O), sqrt(7) */
    2.645751311f, /* ACN 11 (M), sqrt(7) */
    2.645751311f, /* ACN 12 (K), sqrt(7) */
    2.645751311f, /* ACN 13 (L), sqrt(7) */
    2.645751311f, /* ACN 14 (N), sqrt(7) */
    2.645751311f, /* ACN 15 (P), sqrt(7) */
};
const ALfloat FuMa2N3DScale[MAX_AMBI_COEFFS] = {
    1.414213562f, /* ACN  0 (W), sqrt(2) */
    1.732050808f, /* ACN  1 (Y), sqrt(3) */
    1.732050808f, /* ACN  2 (Z), sqrt(3) */
    1.732050808f, /* ACN  3 (X), sqrt(3) */
    1.936491673f, /* ACN  4 (V), sqrt(15)/2 */
    1.936491673f, /* ACN  5 (T), sqrt(15)/2 */
    2.236067978f, /* ACN  6 (R), sqrt(5) */
    1.936491673f, /* ACN  7 (S), sqrt(15)/2 */
    1.936491673f, /* ACN  8 (U), sqrt(15)/2 */
    2.091650066f, /* ACN  9 (Q), sqrt(35/8) */
    1.972026594f, /* ACN 10 (O), sqrt(35)/3 */
    2.231093404f, /* ACN 11 (M), sqrt(224/45) */
    2.645751311f, /* ACN 12 (K), sqrt(7) */
    2.231093404f, /* ACN 13 (L), sqrt(224/45) */
    1.972026594f, /* ACN 14 (N), sqrt(35)/3 */
    2.091650066f, /* ACN 15 (P), sqrt(35/8) */
};


#define HF_BAND 0
#define LF_BAND 1
#define NUM_BANDS 2

/* These points are in AL coordinates! */
static const ALfloat Ambi3DPoints[8][3] = {
    { -0.577350269f,  0.577350269f, -0.577350269f },
    {  0.577350269f,  0.577350269f, -0.577350269f },
    { -0.577350269f,  0.577350269f,  0.577350269f },
    {  0.577350269f,  0.577350269f,  0.577350269f },
    { -0.577350269f, -0.577350269f, -0.577350269f },
    {  0.577350269f, -0.577350269f, -0.577350269f },
    { -0.577350269f, -0.577350269f,  0.577350269f },
    {  0.577350269f, -0.577350269f,  0.577350269f },
};
static const ALfloat Ambi3DDecoder[8][MAX_AMBI_COEFFS] = {
    { 0.125f,  0.125f,  0.125f,  0.125f },
    { 0.125f, -0.125f,  0.125f,  0.125f },
    { 0.125f,  0.125f,  0.125f, -0.125f },
    { 0.125f, -0.125f,  0.125f, -0.125f },
    { 0.125f,  0.125f, -0.125f,  0.125f },
    { 0.125f, -0.125f, -0.125f,  0.125f },
    { 0.125f,  0.125f, -0.125f, -0.125f },
    { 0.125f, -0.125f, -0.125f, -0.125f },
};
static const ALfloat Ambi3DDecoderHFScale[MAX_AMBI_COEFFS] = {
    2.0f,
    1.15470054f, 1.15470054f, 1.15470054f
};


/* NOTE: BandSplitter filters are unused with single-band decoding */
typedef struct BFormatDec {
    ALuint Enabled; /* Bitfield of enabled channels. */

    union {
        alignas(16) ALfloat Dual[MAX_OUTPUT_CHANNELS][NUM_BANDS][MAX_AMBI_COEFFS];
        alignas(16) ALfloat Single[MAX_OUTPUT_CHANNELS][MAX_AMBI_COEFFS];
    } Matrix;

    BandSplitter XOver[MAX_AMBI_COEFFS];

    ALfloat (*Samples)[BUFFERSIZE];
    /* These two alias into Samples */
    ALfloat (*SamplesHF)[BUFFERSIZE];
    ALfloat (*SamplesLF)[BUFFERSIZE];

    alignas(16) ALfloat ChannelMix[BUFFERSIZE];

    struct {
        BandSplitter XOver;
        ALfloat Gains[NUM_BANDS];
    } UpSampler[4];

    ALsizei NumChannels;
    ALboolean DualBand;
} BFormatDec;

BFormatDec *bformatdec_alloc()
{
    return al_calloc(16, sizeof(BFormatDec));
}

void bformatdec_free(BFormatDec **dec)
{
    if(dec && *dec)
    {
        al_free((*dec)->Samples);
        (*dec)->Samples = NULL;
        (*dec)->SamplesHF = NULL;
        (*dec)->SamplesLF = NULL;

        al_free(*dec);
        *dec = NULL;
    }
}

void bformatdec_reset(BFormatDec *dec, const AmbDecConf *conf, ALsizei chancount, ALuint srate, const ALsizei chanmap[MAX_OUTPUT_CHANNELS])
{
    static const ALsizei map2DTo3D[MAX_AMBI2D_COEFFS] = {
        0,  1, 3,  4, 8,  9, 15
    };
    const ALfloat *coeff_scale = N3D2N3DScale;
    bool periphonic;
    ALfloat ratio;
    ALsizei i;

    al_free(dec->Samples);
    dec->Samples = NULL;
    dec->SamplesHF = NULL;
    dec->SamplesLF = NULL;

    dec->NumChannels = chancount;
    dec->Samples = al_calloc(16, dec->NumChannels*2 * sizeof(dec->Samples[0]));
    dec->SamplesHF = dec->Samples;
    dec->SamplesLF = dec->SamplesHF + dec->NumChannels;

    dec->Enabled = 0;
    for(i = 0;i < conf->NumSpeakers;i++)
        dec->Enabled |= 1 << chanmap[i];

    if(conf->CoeffScale == ADS_SN3D)
        coeff_scale = SN3D2N3DScale;
    else if(conf->CoeffScale == ADS_FuMa)
        coeff_scale = FuMa2N3DScale;

    memset(dec->UpSampler, 0, sizeof(dec->UpSampler));
    ratio = 400.0f / (ALfloat)srate;
    for(i = 0;i < 4;i++)
        bandsplit_init(&dec->UpSampler[i].XOver, ratio);
    if((conf->ChanMask&AMBI_PERIPHONIC_MASK))
    {
        periphonic = true;

        dec->UpSampler[0].Gains[HF_BAND] = (conf->ChanMask > 0x1ff) ? W_SCALE_3H3P :
                                           (conf->ChanMask > 0xf) ? W_SCALE_2H2P : 1.0f;
        dec->UpSampler[0].Gains[LF_BAND] = 1.0f;
        for(i = 1;i < 4;i++)
        {
            dec->UpSampler[i].Gains[HF_BAND] = (conf->ChanMask > 0x1ff) ? XYZ_SCALE_3H3P :
                                               (conf->ChanMask > 0xf) ? XYZ_SCALE_2H2P : 1.0f;
            dec->UpSampler[i].Gains[LF_BAND] = 1.0f;
        }
    }
    else
    {
        periphonic = false;

        dec->UpSampler[0].Gains[HF_BAND] = (conf->ChanMask > 0x1ff) ? W_SCALE_3H0P :
                                           (conf->ChanMask > 0xf) ? W_SCALE_2H0P : 1.0f;
        dec->UpSampler[0].Gains[LF_BAND] = 1.0f;
        for(i = 1;i < 3;i++)
        {
            dec->UpSampler[i].Gains[HF_BAND] = (conf->ChanMask > 0x1ff) ? XYZ_SCALE_3H0P :
                                               (conf->ChanMask > 0xf) ? XYZ_SCALE_2H0P : 1.0f;
            dec->UpSampler[i].Gains[LF_BAND] = 1.0f;
        }
        dec->UpSampler[3].Gains[HF_BAND] = 0.0f;
        dec->UpSampler[3].Gains[LF_BAND] = 0.0f;
    }

    memset(&dec->Matrix, 0, sizeof(dec->Matrix));
    if(conf->FreqBands == 1)
    {
        dec->DualBand = AL_FALSE;
        for(i = 0;i < conf->NumSpeakers;i++)
        {
            ALsizei chan = chanmap[i];
            ALfloat gain;
            ALsizei j, k;

            if(!periphonic)
            {
                for(j = 0,k = 0;j < MAX_AMBI2D_COEFFS;j++)
                {
                    ALsizei l = map2DTo3D[j];
                    if(j == 0) gain = conf->HFOrderGain[0];
                    else if(j == 1) gain = conf->HFOrderGain[1];
                    else if(j == 3) gain = conf->HFOrderGain[2];
                    else if(j == 5) gain = conf->HFOrderGain[3];
                    if((conf->ChanMask&(1<<l)))
                        dec->Matrix.Single[chan][j] = conf->HFMatrix[i][k++] / coeff_scale[l] *
                                                      gain;
                }
            }
            else
            {
                for(j = 0,k = 0;j < MAX_AMBI_COEFFS;j++)
                {
                    if(j == 0) gain = conf->HFOrderGain[0];
                    else if(j == 1) gain = conf->HFOrderGain[1];
                    else if(j == 4) gain = conf->HFOrderGain[2];
                    else if(j == 9) gain = conf->HFOrderGain[3];
                    if((conf->ChanMask&(1<<j)))
                        dec->Matrix.Single[chan][j] = conf->HFMatrix[i][k++] / coeff_scale[j] *
                                                      gain;
                }
            }
        }
    }
    else
    {
        dec->DualBand = AL_TRUE;

        ratio = conf->XOverFreq / (ALfloat)srate;
        for(i = 0;i < MAX_AMBI_COEFFS;i++)
            bandsplit_init(&dec->XOver[i], ratio);

        ratio = powf(10.0f, conf->XOverRatio / 40.0f);
        for(i = 0;i < conf->NumSpeakers;i++)
        {
            ALsizei chan = chanmap[i];
            ALfloat gain;
            ALsizei j, k;

            if(!periphonic)
            {
                for(j = 0,k = 0;j < MAX_AMBI2D_COEFFS;j++)
                {
                    ALsizei l = map2DTo3D[j];
                    if(j == 0) gain = conf->HFOrderGain[0] * ratio;
                    else if(j == 1) gain = conf->HFOrderGain[1] * ratio;
                    else if(j == 3) gain = conf->HFOrderGain[2] * ratio;
                    else if(j == 5) gain = conf->HFOrderGain[3] * ratio;
                    if((conf->ChanMask&(1<<l)))
                        dec->Matrix.Dual[chan][HF_BAND][j] = conf->HFMatrix[i][k++] /
                                                             coeff_scale[l] * gain;
                }
                for(j = 0,k = 0;j < MAX_AMBI2D_COEFFS;j++)
                {
                    ALsizei l = map2DTo3D[j];
                    if(j == 0) gain = conf->LFOrderGain[0] / ratio;
                    else if(j == 1) gain = conf->LFOrderGain[1] / ratio;
                    else if(j == 3) gain = conf->LFOrderGain[2] / ratio;
                    else if(j == 5) gain = conf->LFOrderGain[3] / ratio;
                    if((conf->ChanMask&(1<<l)))
                        dec->Matrix.Dual[chan][LF_BAND][j] = conf->LFMatrix[i][k++] /
                                                             coeff_scale[l] * gain;
                }
            }
            else
            {
                for(j = 0,k = 0;j < MAX_AMBI_COEFFS;j++)
                {
                    if(j == 0) gain = conf->HFOrderGain[0] * ratio;
                    else if(j == 1) gain = conf->HFOrderGain[1] * ratio;
                    else if(j == 4) gain = conf->HFOrderGain[2] * ratio;
                    else if(j == 9) gain = conf->HFOrderGain[3] * ratio;
                    if((conf->ChanMask&(1<<j)))
                        dec->Matrix.Dual[chan][HF_BAND][j] = conf->HFMatrix[i][k++] /
                                                             coeff_scale[j] * gain;
                }
                for(j = 0,k = 0;j < MAX_AMBI_COEFFS;j++)
                {
                    if(j == 0) gain = conf->LFOrderGain[0] / ratio;
                    else if(j == 1) gain = conf->LFOrderGain[1] / ratio;
                    else if(j == 4) gain = conf->LFOrderGain[2] / ratio;
                    else if(j == 9) gain = conf->LFOrderGain[3] / ratio;
                    if((conf->ChanMask&(1<<j)))
                        dec->Matrix.Dual[chan][LF_BAND][j] = conf->LFMatrix[i][k++] /
                                                             coeff_scale[j] * gain;
                }
            }
        }
    }
}


void bformatdec_process(struct BFormatDec *dec, ALfloat (*restrict OutBuffer)[BUFFERSIZE], ALsizei OutChannels, const ALfloat (*restrict InSamples)[BUFFERSIZE], ALsizei SamplesToDo)
{
    ALsizei chan, i;

    OutBuffer = ASSUME_ALIGNED(OutBuffer, 16);
    if(dec->DualBand)
    {
        for(i = 0;i < dec->NumChannels;i++)
            bandsplit_process(&dec->XOver[i], dec->SamplesHF[i], dec->SamplesLF[i],
                              InSamples[i], SamplesToDo);

        for(chan = 0;chan < OutChannels;chan++)
        {
            if(!(dec->Enabled&(1<<chan)))
                continue;

            memset(dec->ChannelMix, 0, SamplesToDo*sizeof(ALfloat));
            MixRowSamples(dec->ChannelMix, dec->Matrix.Dual[chan][HF_BAND],
                dec->SamplesHF, dec->NumChannels, 0, SamplesToDo
            );
            MixRowSamples(dec->ChannelMix, dec->Matrix.Dual[chan][LF_BAND],
                dec->SamplesLF, dec->NumChannels, 0, SamplesToDo
            );

            for(i = 0;i < SamplesToDo;i++)
                OutBuffer[chan][i] += dec->ChannelMix[i];
        }
    }
    else
    {
        for(chan = 0;chan < OutChannels;chan++)
        {
            if(!(dec->Enabled&(1<<chan)))
                continue;

            memset(dec->ChannelMix, 0, SamplesToDo*sizeof(ALfloat));
            MixRowSamples(dec->ChannelMix, dec->Matrix.Single[chan], InSamples,
                          dec->NumChannels, 0, SamplesToDo);

            for(i = 0;i < SamplesToDo;i++)
                OutBuffer[chan][i] += dec->ChannelMix[i];
        }
    }
}


void bformatdec_upSample(struct BFormatDec *dec, ALfloat (*restrict OutBuffer)[BUFFERSIZE], const ALfloat (*restrict InSamples)[BUFFERSIZE], ALsizei InChannels, ALsizei SamplesToDo)
{
    ALsizei i;

    /* This up-sampler leverages the differences observed in dual-band second-
     * and third-order decoder matrices compared to first-order. For the same
     * output channel configuration, the low-frequency matrix has identical
     * coefficients in the shared input channels, while the high-frequency
     * matrix has extra scalars applied to the W channel and X/Y/Z channels.
     * Mixing the first-order content into the higher-order stream with the
     * appropriate counter-scales applied to the HF response results in the
     * subsequent higher-order decode generating the same response as a first-
     * order decode.
     */
    for(i = 0;i < InChannels;i++)
    {
        /* First, split the first-order components into low and high frequency
         * bands.
         */
        bandsplit_process(&dec->UpSampler[i].XOver,
            dec->Samples[HF_BAND], dec->Samples[LF_BAND],
            InSamples[i], SamplesToDo
        );

        /* Now write each band to the output. */
        MixRowSamples(OutBuffer[i], dec->UpSampler[i].Gains,
            dec->Samples, NUM_BANDS, 0, SamplesToDo
        );
    }
}


#define INVALID_UPSAMPLE_INDEX INT_MAX

static ALsizei GetACNIndex(const BFChannelConfig *chans, ALsizei numchans, ALsizei acn)
{
    ALsizei i;
    for(i = 0;i < numchans;i++)
    {
        if(chans[i].Index == acn)
            return i;
    }
    return INVALID_UPSAMPLE_INDEX;
}
#define GetChannelForACN(b, a) GetACNIndex((b).Ambi.Map, (b).NumChannels, (a))

typedef struct AmbiUpsampler {
    alignas(16) ALfloat Samples[NUM_BANDS][BUFFERSIZE];

    BandSplitter XOver[4];

    ALfloat Gains[4][MAX_OUTPUT_CHANNELS][NUM_BANDS];
} AmbiUpsampler;

AmbiUpsampler *ambiup_alloc()
{
    return al_calloc(16, sizeof(AmbiUpsampler));
}

void ambiup_free(struct AmbiUpsampler **ambiup)
{
    if(ambiup)
    {
        al_free(*ambiup);
        *ambiup = NULL;
    }
}

void ambiup_reset(struct AmbiUpsampler *ambiup, const ALCdevice *device, ALfloat w_scale, ALfloat xyz_scale)
{
    ALfloat ratio;
    ALsizei i;

    ratio = 400.0f / (ALfloat)device->Frequency;
    for(i = 0;i < 4;i++)
        bandsplit_init(&ambiup->XOver[i], ratio);

    memset(ambiup->Gains, 0, sizeof(ambiup->Gains));
    if(device->Dry.CoeffCount > 0)
    {
        ALfloat encgains[8][MAX_OUTPUT_CHANNELS];
        ALsizei j;
        size_t k;

        for(k = 0;k < COUNTOF(Ambi3DPoints);k++)
        {
            ALfloat coeffs[MAX_AMBI_COEFFS] = { 0.0f };
            CalcDirectionCoeffs(Ambi3DPoints[k], 0.0f, coeffs);
            ComputeDryPanGains(&device->Dry, coeffs, 1.0f, encgains[k]);
        }

        /* Combine the matrices that do the in->virt and virt->out conversions
         * so we get a single in->out conversion. NOTE: the Encoder matrix
         * (encgains) and output are transposed, so the input channels line up
         * with the rows and the output channels line up with the columns.
         */
        for(i = 0;i < 4;i++)
        {
            for(j = 0;j < device->Dry.NumChannels;j++)
            {
                ALfloat gain=0.0f;
                for(k = 0;k < COUNTOF(Ambi3DDecoder);k++)
                    gain += Ambi3DDecoder[k][i] * encgains[k][j];
                ambiup->Gains[i][j][HF_BAND] = gain * Ambi3DDecoderHFScale[i];
                ambiup->Gains[i][j][LF_BAND] = gain;
            }
        }
    }
    else
    {
        for(i = 0;i < 4;i++)
        {
            ALsizei index = GetChannelForACN(device->Dry, i);
            if(index != INVALID_UPSAMPLE_INDEX)
            {
                ALfloat scale = device->Dry.Ambi.Map[index].Scale;
                ambiup->Gains[i][index][HF_BAND] = scale * ((i==0) ? w_scale : xyz_scale);
                ambiup->Gains[i][index][LF_BAND] = scale;
            }
        }
    }
}

void ambiup_process(struct AmbiUpsampler *ambiup, ALfloat (*restrict OutBuffer)[BUFFERSIZE], ALsizei OutChannels, const ALfloat (*restrict InSamples)[BUFFERSIZE], ALsizei SamplesToDo)
{
    ALsizei i, j;

    for(i = 0;i < 4;i++)
    {
        bandsplit_process(&ambiup->XOver[i],
            ambiup->Samples[HF_BAND], ambiup->Samples[LF_BAND],
            InSamples[i], SamplesToDo
        );

        for(j = 0;j < OutChannels;j++)
            MixRowSamples(OutBuffer[j], ambiup->Gains[i][j],
                ambiup->Samples, NUM_BANDS, 0, SamplesToDo
            );
    }
}
