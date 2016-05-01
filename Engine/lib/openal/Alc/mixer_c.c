#include "config.h"

#include <assert.h>

#include "alMain.h"
#include "alu.h"
#include "alSource.h"
#include "alAuxEffectSlot.h"


static __inline ALfloat point32(const ALfloat *vals, ALuint frac)
{ return vals[0]; (void)frac; }
static __inline ALfloat lerp32(const ALfloat *vals, ALuint frac)
{ return lerp(vals[0], vals[1], frac * (1.0f/FRACTIONONE)); }
static __inline ALfloat cubic32(const ALfloat *vals, ALuint frac)
{ return cubic(vals[-1], vals[0], vals[1], vals[2], frac * (1.0f/FRACTIONONE)); }

void Resample_copy32_C(const ALfloat *data, ALuint frac,
  ALuint increment, ALfloat *RESTRICT OutBuffer, ALuint BufferSize)
{
    (void)frac;
    assert(increment==FRACTIONONE);
    memcpy(OutBuffer, data, (BufferSize+1)*sizeof(ALfloat));
}

#define DECL_TEMPLATE(Sampler)                                                \
void Resample_##Sampler##_C(const ALfloat *data, ALuint frac,                 \
  ALuint increment, ALfloat *RESTRICT OutBuffer, ALuint BufferSize)           \
{                                                                             \
    ALuint pos = 0;                                                           \
    ALuint i;                                                                 \
                                                                              \
    for(i = 0;i < BufferSize+1;i++)                                           \
    {                                                                         \
        OutBuffer[i] = Sampler(data + pos, frac);                             \
                                                                              \
        frac += increment;                                                    \
        pos  += frac>>FRACTIONBITS;                                           \
        frac &= FRACTIONMASK;                                                 \
    }                                                                         \
}

DECL_TEMPLATE(point32)
DECL_TEMPLATE(lerp32)
DECL_TEMPLATE(cubic32)

#undef DECL_TEMPLATE


__inline void ApplyCoeffsStep(ALuint Offset, ALfloat (*RESTRICT Values)[2],
                                     const ALuint IrSize,
                                     ALfloat (*RESTRICT Coeffs)[2],
                                     const ALfloat (*RESTRICT CoeffStep)[2],
                                     ALfloat left, ALfloat right)
{
    ALuint c;
    for(c = 0;c < IrSize;c++)
    {
        const ALuint off = (Offset+c)&HRIR_MASK;
        Values[off][0] += Coeffs[c][0] * left;
        Values[off][1] += Coeffs[c][1] * right;
        Coeffs[c][0] += CoeffStep[c][0];
        Coeffs[c][1] += CoeffStep[c][1];
    }
}

__inline void ApplyCoeffs(ALuint Offset, ALfloat (*RESTRICT Values)[2],
                                 const ALuint IrSize,
                                 ALfloat (*RESTRICT Coeffs)[2],
                                 ALfloat left, ALfloat right)
{
    ALuint c;
    for(c = 0;c < IrSize;c++)
    {
        const ALuint off = (Offset+c)&HRIR_MASK;
        Values[off][0] += Coeffs[c][0] * left;
        Values[off][1] += Coeffs[c][1] * right;
    }
}

#define SUFFIX C
#include "mixer_inc.c"
#undef SUFFIX


void MixDirect_C(const DirectParams *params, const ALfloat *RESTRICT data, ALuint srcchan,
  ALuint OutPos, ALuint SamplesToDo, ALuint BufferSize)
{
    ALfloat (*RESTRICT DryBuffer)[BUFFERSIZE] = params->OutBuffer;
    ALfloat *RESTRICT ClickRemoval = params->ClickRemoval;
    ALfloat *RESTRICT PendingClicks = params->PendingClicks;
    ALfloat DrySend;
    ALuint pos;
    ALuint c;

    for(c = 0;c < MaxChannels;c++)
    {
        DrySend = params->Gains[srcchan][c];
        if(DrySend < 0.00001f)
            continue;

        if(OutPos == 0)
            ClickRemoval[c] -= data[0]*DrySend;
        for(pos = 0;pos < BufferSize;pos++)
            DryBuffer[c][OutPos+pos] += data[pos]*DrySend;
        if(OutPos+pos == SamplesToDo)
            PendingClicks[c] += data[pos]*DrySend;
    }
}


void MixSend_C(const SendParams *params, const ALfloat *RESTRICT data,
  ALuint OutPos, ALuint SamplesToDo, ALuint BufferSize)
{
    ALeffectslot *Slot = params->Slot;
    ALfloat (*RESTRICT WetBuffer)[BUFFERSIZE] = Slot->WetBuffer;
    ALfloat *RESTRICT WetClickRemoval = Slot->ClickRemoval;
    ALfloat *RESTRICT WetPendingClicks = Slot->PendingClicks;
    ALfloat  WetSend = params->Gain;
    ALuint pos;

    if(WetSend < 0.00001f)
        return;

    if(OutPos == 0)
        WetClickRemoval[0] -= data[0] * WetSend;
    for(pos = 0;pos < BufferSize;pos++)
        WetBuffer[0][OutPos+pos] += data[pos] * WetSend;
    if(OutPos+pos == SamplesToDo)
        WetPendingClicks[0] += data[pos] * WetSend;
}
