#include "config.h"

#include <math.h>

#include "mastering.h"
#include "alu.h"
#include "almalloc.h"


extern inline ALuint GetCompressorSampleRate(const Compressor *Comp);

#define RMS_WINDOW_SIZE (1<<7)
#define RMS_WINDOW_MASK (RMS_WINDOW_SIZE-1)
#define RMS_VALUE_MAX  (1<<24)

static_assert(RMS_VALUE_MAX < (UINT_MAX / RMS_WINDOW_SIZE), "RMS_VALUE_MAX is too big");


/* Multichannel compression is linked via one of two modes:
 *
 *   Summed - Absolute sum of all channels.
 *   Maxed  - Absolute maximum of any channel.
 */
static void SumChannels(Compressor *Comp, const ALsizei NumChans, const ALsizei SamplesToDo,
                        ALfloat (*restrict OutBuffer)[BUFFERSIZE])
{
    ALsizei c, i;

    for(i = 0;i < SamplesToDo;i++)
        Comp->Envelope[i] = 0.0f;

    for(c = 0;c < NumChans;c++)
    {
        for(i = 0;i < SamplesToDo;i++)
            Comp->Envelope[i] += OutBuffer[c][i];
    }

    for(i = 0;i < SamplesToDo;i++)
        Comp->Envelope[i] = fabsf(Comp->Envelope[i]);
}

static void MaxChannels(Compressor *Comp, const ALsizei NumChans, const ALsizei SamplesToDo,
                        ALfloat (*restrict OutBuffer)[BUFFERSIZE])
{
    ALsizei c, i;

    for(i = 0;i < SamplesToDo;i++)
        Comp->Envelope[i] = 0.0f;

    for(c = 0;c < NumChans;c++)
    {
        for(i = 0;i < SamplesToDo;i++)
            Comp->Envelope[i] = maxf(Comp->Envelope[i], fabsf(OutBuffer[c][i]));
    }
}

/* Envelope detection/sensing can be done via:
 *
 *   RMS  - Rectangular windowed root mean square of linking stage.
 *   Peak - Implicit output from linking stage.
 */
static void RmsDetection(Compressor *Comp, const ALsizei SamplesToDo)
{
    ALuint sum = Comp->RmsSum;
    ALuint *window = Comp->RmsWindow;
    ALsizei index = Comp->RmsIndex;
    ALsizei i;

    for(i = 0;i < SamplesToDo;i++)
    {
        ALfloat sig = Comp->Envelope[i];

        sum -= window[index];
        window[index] = fastf2i(minf(sig * sig * 65536.0f, RMS_VALUE_MAX));
        sum += window[index];
        index = (index + 1) & RMS_WINDOW_MASK;

        Comp->Envelope[i] = sqrtf(sum / 65536.0f / RMS_WINDOW_SIZE);
    }

    Comp->RmsSum = sum;
    Comp->RmsIndex = index;
}

/* This isn't a very sophisticated envelope follower, but it gets the job
 * done.  First, it operates at logarithmic scales to keep transitions
 * appropriate for human hearing.  Second, it can apply adaptive (automated)
 * attack/release adjustments based on the signal.
 */
static void FollowEnvelope(Compressor *Comp, const ALsizei SamplesToDo)
{
    ALfloat attackMin = Comp->AttackMin;
    ALfloat attackMax = Comp->AttackMax;
    ALfloat releaseMin = Comp->ReleaseMin;
    ALfloat releaseMax = Comp->ReleaseMax;
    ALfloat last = Comp->EnvLast;
    ALsizei i;

    for(i = 0;i < SamplesToDo;i++)
    {
        ALfloat env = maxf(-6.0f, log10f(Comp->Envelope[i]));
        ALfloat slope = minf(1.0f, fabsf(env - last) / 4.5f);

        if(env > last)
            last = minf(env, last + lerp(attackMin, attackMax, 1.0f - (slope * slope)));
        else
            last = maxf(env, last + lerp(releaseMin, releaseMax, 1.0f - (slope * slope)));

        Comp->Envelope[i] = last;
    }

    Comp->EnvLast = last;
}

/* The envelope is converted to control gain with an optional soft knee. */
static void EnvelopeGain(Compressor *Comp, const ALsizei SamplesToDo, const ALfloat Slope)
{
    const ALfloat threshold = Comp->Threshold;
    const ALfloat knee = Comp->Knee;
    ALsizei i;

    if(!(knee > 0.0f))
    {
        for(i = 0;i < SamplesToDo;i++)
        {
            ALfloat gain = Slope * (threshold - Comp->Envelope[i]);
            Comp->Envelope[i] = powf(10.0f, minf(0.0f, gain));
        }
    }
    else
    {
        const ALfloat lower = threshold - (0.5f * knee);
        const ALfloat upper = threshold + (0.5f * knee);
        const ALfloat m = 0.5f * Slope / knee;

        for(i = 0;i < SamplesToDo;i++)
        {
            ALfloat env = Comp->Envelope[i];
            ALfloat gain;

            if(env > lower && env < upper)
                gain = m * (env - lower) * (lower - env);
            else
                gain = Slope * (threshold - env);

            Comp->Envelope[i] = powf(10.0f, minf(0.0f, gain));
        }
    }
}


Compressor *CompressorInit(const ALfloat PreGainDb, const ALfloat PostGainDb,
                           const ALboolean SummedLink, const ALboolean RmsSensing,
                           const ALfloat AttackTimeMin, const ALfloat AttackTimeMax,
                           const ALfloat ReleaseTimeMin, const ALfloat ReleaseTimeMax,
                           const ALfloat Ratio, const ALfloat ThresholdDb,
                           const ALfloat KneeDb, const ALuint SampleRate)
{
    Compressor *Comp;
    size_t size;
    ALsizei i;

    size = sizeof(*Comp);
    if(RmsSensing)
        size += sizeof(Comp->RmsWindow[0]) * RMS_WINDOW_SIZE;
    Comp = al_calloc(16, size);

    Comp->PreGain = powf(10.0f, PreGainDb / 20.0f);
    Comp->PostGain = powf(10.0f, PostGainDb / 20.0f);
    Comp->SummedLink = SummedLink;
    Comp->AttackMin = 1.0f / maxf(0.000001f, AttackTimeMin * SampleRate * logf(10.0f));
    Comp->AttackMax = 1.0f / maxf(0.000001f, AttackTimeMax * SampleRate * logf(10.0f));
    Comp->ReleaseMin = -1.0f / maxf(0.000001f, ReleaseTimeMin * SampleRate * logf(10.0f));
    Comp->ReleaseMax = -1.0f / maxf(0.000001f, ReleaseTimeMax * SampleRate * logf(10.0f));
    Comp->Ratio = Ratio;
    Comp->Threshold = ThresholdDb / 20.0f;
    Comp->Knee = maxf(0.0f, KneeDb / 20.0f);
    Comp->SampleRate = SampleRate;

    Comp->RmsSum = 0;
    if(RmsSensing)
        Comp->RmsWindow = (ALuint*)(Comp+1);
    else
        Comp->RmsWindow = NULL;
    Comp->RmsIndex = 0;

    for(i = 0;i < BUFFERSIZE;i++)
        Comp->Envelope[i] = 0.0f;
    Comp->EnvLast = -6.0f;

    return Comp;
}

void ApplyCompression(Compressor *Comp, const ALsizei NumChans, const ALsizei SamplesToDo,
                      ALfloat (*restrict OutBuffer)[BUFFERSIZE])
{
    ALsizei c, i;

    if(Comp->PreGain != 1.0f)
    {
        for(c = 0;c < NumChans;c++)
        {
            for(i = 0;i < SamplesToDo;i++)
                OutBuffer[c][i] *= Comp->PreGain;
        }
    }

    if(Comp->SummedLink)
        SumChannels(Comp, NumChans, SamplesToDo, OutBuffer);
    else
        MaxChannels(Comp, NumChans, SamplesToDo, OutBuffer);

    if(Comp->RmsWindow)
        RmsDetection(Comp, SamplesToDo);
    FollowEnvelope(Comp, SamplesToDo);

    if(Comp->Ratio > 0.0f)
        EnvelopeGain(Comp, SamplesToDo, 1.0f - (1.0f / Comp->Ratio));
    else
        EnvelopeGain(Comp, SamplesToDo, 1.0f);

    if(Comp->PostGain != 1.0f)
    {
        for(i = 0;i < SamplesToDo;i++)
            Comp->Envelope[i] *= Comp->PostGain;
    }
    for(c = 0;c < NumChans;c++)
    {
        for(i = 0;i < SamplesToDo;i++)
            OutBuffer[c][i] *= Comp->Envelope[i];
    }
}
