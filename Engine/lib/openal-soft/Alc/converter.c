
#include "config.h"

#include "converter.h"

#include "fpu_modes.h"
#include "mixer/defs.h"


SampleConverter *CreateSampleConverter(enum DevFmtType srcType, enum DevFmtType dstType, ALsizei numchans, ALsizei srcRate, ALsizei dstRate)
{
    SampleConverter *converter;
    ALsizei step;

    if(numchans <= 0 || srcRate <= 0 || dstRate <= 0)
        return NULL;

    converter = al_calloc(16, FAM_SIZE(SampleConverter, Chan, numchans));
    converter->mSrcType = srcType;
    converter->mDstType = dstType;
    converter->mNumChannels = numchans;
    converter->mSrcTypeSize = BytesFromDevFmt(srcType);
    converter->mDstTypeSize = BytesFromDevFmt(dstType);

    converter->mSrcPrepCount = 0;
    converter->mFracOffset = 0;

    /* Have to set the mixer FPU mode since that's what the resampler code expects. */
    START_MIXER_MODE();
    step = (ALsizei)mind(((ALdouble)srcRate/dstRate*FRACTIONONE) + 0.5,
                         MAX_PITCH * FRACTIONONE);
    converter->mIncrement = maxi(step, 1);
    if(converter->mIncrement == FRACTIONONE)
        converter->mResample = Resample_copy_C;
    else
    {
        /* TODO: Allow other resamplers. */
        BsincPrepare(converter->mIncrement, &converter->mState.bsinc, &bsinc12);
        converter->mResample = SelectResampler(BSinc12Resampler);
    }
    END_MIXER_MODE();

    return converter;
}

void DestroySampleConverter(SampleConverter **converter)
{
    if(converter)
    {
        al_free(*converter);
        *converter = NULL;
    }
}


static inline ALfloat Sample_ALbyte(ALbyte val)
{ return val * (1.0f/128.0f); }
static inline ALfloat Sample_ALubyte(ALubyte val)
{ return Sample_ALbyte((ALint)val - 128); }

static inline ALfloat Sample_ALshort(ALshort val)
{ return val * (1.0f/32768.0f); }
static inline ALfloat Sample_ALushort(ALushort val)
{ return Sample_ALshort((ALint)val - 32768); }

static inline ALfloat Sample_ALint(ALint val)
{ return (val>>7) * (1.0f/16777216.0f); }
static inline ALfloat Sample_ALuint(ALuint val)
{ return Sample_ALint(val - INT_MAX - 1); }

static inline ALfloat Sample_ALfloat(ALfloat val)
{ return val; }

#define DECL_TEMPLATE(T)                                                      \
static inline void Load_##T(ALfloat *restrict dst, const T *restrict src,     \
                            ALint srcstep, ALsizei samples)                   \
{                                                                             \
    ALsizei i;                                                                \
    for(i = 0;i < samples;i++)                                                \
        dst[i] = Sample_##T(src[i*srcstep]);                                  \
}

DECL_TEMPLATE(ALbyte)
DECL_TEMPLATE(ALubyte)
DECL_TEMPLATE(ALshort)
DECL_TEMPLATE(ALushort)
DECL_TEMPLATE(ALint)
DECL_TEMPLATE(ALuint)
DECL_TEMPLATE(ALfloat)

#undef DECL_TEMPLATE

static void LoadSamples(ALfloat *dst, const ALvoid *src, ALint srcstep, enum DevFmtType srctype, ALsizei samples)
{
    switch(srctype)
    {
        case DevFmtByte:
            Load_ALbyte(dst, src, srcstep, samples);
            break;
        case DevFmtUByte:
            Load_ALubyte(dst, src, srcstep, samples);
            break;
        case DevFmtShort:
            Load_ALshort(dst, src, srcstep, samples);
            break;
        case DevFmtUShort:
            Load_ALushort(dst, src, srcstep, samples);
            break;
        case DevFmtInt:
            Load_ALint(dst, src, srcstep, samples);
            break;
        case DevFmtUInt:
            Load_ALuint(dst, src, srcstep, samples);
            break;
        case DevFmtFloat:
            Load_ALfloat(dst, src, srcstep, samples);
            break;
    }
}


static inline ALbyte ALbyte_Sample(ALfloat val)
{ return fastf2i(clampf(val*128.0f, -128.0f, 127.0f)); }
static inline ALubyte ALubyte_Sample(ALfloat val)
{ return ALbyte_Sample(val)+128; }

static inline ALshort ALshort_Sample(ALfloat val)
{ return fastf2i(clampf(val*32768.0f, -32768.0f, 32767.0f)); }
static inline ALushort ALushort_Sample(ALfloat val)
{ return ALshort_Sample(val)+32768; }

static inline ALint ALint_Sample(ALfloat val)
{ return fastf2i(clampf(val*16777216.0f, -16777216.0f, 16777215.0f)) << 7; }
static inline ALuint ALuint_Sample(ALfloat val)
{ return ALint_Sample(val)+INT_MAX+1; }

static inline ALfloat ALfloat_Sample(ALfloat val)
{ return val; }

#define DECL_TEMPLATE(T)                                                      \
static inline void Store_##T(T *restrict dst, const ALfloat *restrict src,    \
                             ALint dststep, ALsizei samples)                  \
{                                                                             \
    ALsizei i;                                                                \
    for(i = 0;i < samples;i++)                                                \
        dst[i*dststep] = T##_Sample(src[i]);                                  \
}

DECL_TEMPLATE(ALbyte)
DECL_TEMPLATE(ALubyte)
DECL_TEMPLATE(ALshort)
DECL_TEMPLATE(ALushort)
DECL_TEMPLATE(ALint)
DECL_TEMPLATE(ALuint)
DECL_TEMPLATE(ALfloat)

#undef DECL_TEMPLATE

static void StoreSamples(ALvoid *dst, const ALfloat *src, ALint dststep, enum DevFmtType dsttype, ALsizei samples)
{
    switch(dsttype)
    {
        case DevFmtByte:
            Store_ALbyte(dst, src, dststep, samples);
            break;
        case DevFmtUByte:
            Store_ALubyte(dst, src, dststep, samples);
            break;
        case DevFmtShort:
            Store_ALshort(dst, src, dststep, samples);
            break;
        case DevFmtUShort:
            Store_ALushort(dst, src, dststep, samples);
            break;
        case DevFmtInt:
            Store_ALint(dst, src, dststep, samples);
            break;
        case DevFmtUInt:
            Store_ALuint(dst, src, dststep, samples);
            break;
        case DevFmtFloat:
            Store_ALfloat(dst, src, dststep, samples);
            break;
    }
}


ALsizei SampleConverterAvailableOut(SampleConverter *converter, ALsizei srcframes)
{
    ALint prepcount = converter->mSrcPrepCount;
    ALsizei increment = converter->mIncrement;
    ALsizei DataPosFrac = converter->mFracOffset;
    ALuint64 DataSize64;

    if(prepcount < 0)
    {
        /* Negative prepcount means we need to skip that many input samples. */
        if(-prepcount >= srcframes)
            return 0;
        srcframes += prepcount;
        prepcount = 0;
    }

    if(srcframes < 1)
    {
        /* No output samples if there's no input samples. */
        return 0;
    }

    if(prepcount < MAX_RESAMPLE_PADDING*2 &&
       MAX_RESAMPLE_PADDING*2 - prepcount >= srcframes)
    {
        /* Not enough input samples to generate an output sample. */
        return 0;
    }

    DataSize64  = prepcount;
    DataSize64 += srcframes;
    DataSize64 -= MAX_RESAMPLE_PADDING*2;
    DataSize64 <<= FRACTIONBITS;
    DataSize64 -= DataPosFrac;

    /* If we have a full prep, we can generate at least one sample. */
    return (ALsizei)clampu64((DataSize64 + increment-1)/increment, 1, BUFFERSIZE);
}


ALsizei SampleConverterInput(SampleConverter *converter, const ALvoid **src, ALsizei *srcframes, ALvoid *dst, ALsizei dstframes)
{
    const ALsizei SrcFrameSize = converter->mNumChannels * converter->mSrcTypeSize;
    const ALsizei DstFrameSize = converter->mNumChannels * converter->mDstTypeSize;
    const ALsizei increment = converter->mIncrement;
    ALsizei pos = 0;

    START_MIXER_MODE();
    while(pos < dstframes && *srcframes > 0)
    {
        ALfloat *restrict SrcData = ASSUME_ALIGNED(converter->mSrcSamples, 16);
        ALfloat *restrict DstData = ASSUME_ALIGNED(converter->mDstSamples, 16);
        ALint prepcount = converter->mSrcPrepCount;
        ALsizei DataPosFrac = converter->mFracOffset;
        ALuint64 DataSize64;
        ALsizei DstSize;
        ALint toread;
        ALsizei chan;

        if(prepcount < 0)
        {
            /* Negative prepcount means we need to skip that many input samples. */
            if(-prepcount >= *srcframes)
            {
                converter->mSrcPrepCount = prepcount + *srcframes;
                *srcframes = 0;
                break;
            }
            *src = (const ALbyte*)*src + SrcFrameSize*-prepcount;
            *srcframes += prepcount;
            converter->mSrcPrepCount = 0;
            continue;
        }
        toread = mini(*srcframes, BUFFERSIZE - MAX_RESAMPLE_PADDING*2);

        if(prepcount < MAX_RESAMPLE_PADDING*2 &&
           MAX_RESAMPLE_PADDING*2 - prepcount >= toread)
        {
            /* Not enough input samples to generate an output sample. Store
             * what we're given for later.
             */
            for(chan = 0;chan < converter->mNumChannels;chan++)
                LoadSamples(&converter->Chan[chan].mPrevSamples[prepcount],
                    (const ALbyte*)*src + converter->mSrcTypeSize*chan,
                    converter->mNumChannels, converter->mSrcType, toread
                );

            converter->mSrcPrepCount = prepcount + toread;
            *srcframes = 0;
            break;
        }

        DataSize64  = prepcount;
        DataSize64 += toread;
        DataSize64 -= MAX_RESAMPLE_PADDING*2;
        DataSize64 <<= FRACTIONBITS;
        DataSize64 -= DataPosFrac;

        /* If we have a full prep, we can generate at least one sample. */
        DstSize = (ALsizei)clampu64((DataSize64 + increment-1)/increment, 1, BUFFERSIZE);
        DstSize = mini(DstSize, dstframes-pos);

        for(chan = 0;chan < converter->mNumChannels;chan++)
        {
            const ALbyte *SrcSamples = (const ALbyte*)*src + converter->mSrcTypeSize*chan;
            ALbyte *DstSamples = (ALbyte*)dst + converter->mDstTypeSize*chan;
            const ALfloat *ResampledData;
            ALsizei SrcDataEnd;

            /* Load the previous samples into the source data first, then the
             * new samples from the input buffer.
             */
            memcpy(SrcData, converter->Chan[chan].mPrevSamples,
                   prepcount*sizeof(ALfloat));
            LoadSamples(SrcData + prepcount, SrcSamples,
                converter->mNumChannels, converter->mSrcType, toread
            );

            /* Store as many prep samples for next time as possible, given the
             * number of output samples being generated.
             */
            SrcDataEnd = (DataPosFrac + increment*DstSize)>>FRACTIONBITS;
            if(SrcDataEnd >= prepcount+toread)
                memset(converter->Chan[chan].mPrevSamples, 0,
                       sizeof(converter->Chan[chan].mPrevSamples));
            else
            {
                size_t len = mini(MAX_RESAMPLE_PADDING*2, prepcount+toread-SrcDataEnd);
                memcpy(converter->Chan[chan].mPrevSamples, &SrcData[SrcDataEnd],
                       len*sizeof(ALfloat));
                memset(converter->Chan[chan].mPrevSamples+len, 0,
                       sizeof(converter->Chan[chan].mPrevSamples) - len*sizeof(ALfloat));
            }

            /* Now resample, and store the result in the output buffer. */
            ResampledData = converter->mResample(&converter->mState,
                SrcData+MAX_RESAMPLE_PADDING, DataPosFrac, increment,
                DstData, DstSize
            );

            StoreSamples(DstSamples, ResampledData, converter->mNumChannels,
                         converter->mDstType, DstSize);
        }

        /* Update the number of prep samples still available, as well as the
         * fractional offset.
         */
        DataPosFrac += increment*DstSize;
        converter->mSrcPrepCount = mini(prepcount + toread - (DataPosFrac>>FRACTIONBITS),
                                        MAX_RESAMPLE_PADDING*2);
        converter->mFracOffset = DataPosFrac & FRACTIONMASK;

        /* Update the src and dst pointers in case there's still more to do. */
        *src = (const ALbyte*)*src + SrcFrameSize*(DataPosFrac>>FRACTIONBITS);
        *srcframes -= mini(*srcframes, (DataPosFrac>>FRACTIONBITS));

        dst = (ALbyte*)dst + DstFrameSize*DstSize;
        pos += DstSize;
    }
    END_MIXER_MODE();

    return pos;
}


ChannelConverter *CreateChannelConverter(enum DevFmtType srcType, enum DevFmtChannels srcChans, enum DevFmtChannels dstChans)
{
    ChannelConverter *converter;

    if(srcChans != dstChans && !((srcChans == DevFmtMono && dstChans == DevFmtStereo) ||
                                 (srcChans == DevFmtStereo && dstChans == DevFmtMono)))
        return NULL;

    converter = al_calloc(DEF_ALIGN, sizeof(*converter));
    converter->mSrcType = srcType;
    converter->mSrcChans = srcChans;
    converter->mDstChans = dstChans;

    return converter;
}

void DestroyChannelConverter(ChannelConverter **converter)
{
    if(converter)
    {
        al_free(*converter);
        *converter = NULL;
    }
}


#define DECL_TEMPLATE(T)                                                       \
static void Mono2Stereo##T(ALfloat *restrict dst, const T *src, ALsizei frames)\
{                                                                              \
    ALsizei i;                                                                 \
    for(i = 0;i < frames;i++)                                                  \
        dst[i*2 + 1] = dst[i*2 + 0] = Sample_##T(src[i]) * 0.707106781187f;    \
}                                                                              \
                                                                               \
static void Stereo2Mono##T(ALfloat *restrict dst, const T *src, ALsizei frames)\
{                                                                              \
    ALsizei i;                                                                 \
    for(i = 0;i < frames;i++)                                                  \
        dst[i] = (Sample_##T(src[i*2 + 0])+Sample_##T(src[i*2 + 1])) *         \
                 0.707106781187f;                                              \
}

DECL_TEMPLATE(ALbyte)
DECL_TEMPLATE(ALubyte)
DECL_TEMPLATE(ALshort)
DECL_TEMPLATE(ALushort)
DECL_TEMPLATE(ALint)
DECL_TEMPLATE(ALuint)
DECL_TEMPLATE(ALfloat)

#undef DECL_TEMPLATE

void ChannelConverterInput(ChannelConverter *converter, const ALvoid *src, ALfloat *dst, ALsizei frames)
{
    if(converter->mSrcChans == converter->mDstChans)
    {
        LoadSamples(dst, src, 1, converter->mSrcType,
                    frames*ChannelsFromDevFmt(converter->mSrcChans, 0));
        return;
    }

    if(converter->mSrcChans == DevFmtStereo && converter->mDstChans == DevFmtMono)
    {
        switch(converter->mSrcType)
        {
            case DevFmtByte:
                Stereo2MonoALbyte(dst, src, frames);
                break;
            case DevFmtUByte:
                Stereo2MonoALubyte(dst, src, frames);
                break;
            case DevFmtShort:
                Stereo2MonoALshort(dst, src, frames);
                break;
            case DevFmtUShort:
                Stereo2MonoALushort(dst, src, frames);
                break;
            case DevFmtInt:
                Stereo2MonoALint(dst, src, frames);
                break;
            case DevFmtUInt:
                Stereo2MonoALuint(dst, src, frames);
                break;
            case DevFmtFloat:
                Stereo2MonoALfloat(dst, src, frames);
                break;
        }
    }
    else /*if(converter->mSrcChans == DevFmtMono && converter->mDstChans == DevFmtStereo)*/
    {
        switch(converter->mSrcType)
        {
            case DevFmtByte:
                Mono2StereoALbyte(dst, src, frames);
                break;
            case DevFmtUByte:
                Mono2StereoALubyte(dst, src, frames);
                break;
            case DevFmtShort:
                Mono2StereoALshort(dst, src, frames);
                break;
            case DevFmtUShort:
                Mono2StereoALushort(dst, src, frames);
                break;
            case DevFmtInt:
                Mono2StereoALint(dst, src, frames);
                break;
            case DevFmtUInt:
                Mono2StereoALuint(dst, src, frames);
                break;
            case DevFmtFloat:
                Mono2StereoALfloat(dst, src, frames);
                break;
        }
    }
}
