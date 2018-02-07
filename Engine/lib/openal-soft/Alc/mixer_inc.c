#include "config.h"

#include "alMain.h"
#include "alSource.h"

#include "hrtf.h"
#include "mixer_defs.h"
#include "align.h"
#include "alu.h"


static inline void ApplyCoeffs(ALsizei Offset, ALfloat (*restrict Values)[2],
                               const ALsizei irSize,
                               const ALfloat (*restrict Coeffs)[2],
                               ALfloat left, ALfloat right);


void MixHrtf(ALfloat *restrict LeftOut, ALfloat *restrict RightOut,
             const ALfloat *data, ALsizei Offset, ALsizei OutPos,
             const ALsizei IrSize, MixHrtfParams *hrtfparams, HrtfState *hrtfstate,
             ALsizei BufferSize)
{
    const ALfloat (*Coeffs)[2] = ASSUME_ALIGNED(hrtfparams->Coeffs, 16);
    const ALsizei Delay[2] = { hrtfparams->Delay[0], hrtfparams->Delay[1] };
    ALfloat gainstep = hrtfparams->GainStep;
    ALfloat gain = hrtfparams->Gain;
    ALfloat left, right;
    ALsizei i;

    LeftOut  += OutPos;
    RightOut += OutPos;
    for(i = 0;i < BufferSize;i++)
    {
        hrtfstate->History[Offset&HRTF_HISTORY_MASK] = *(data++);
        left = hrtfstate->History[(Offset-Delay[0])&HRTF_HISTORY_MASK]*gain;
        right = hrtfstate->History[(Offset-Delay[1])&HRTF_HISTORY_MASK]*gain;

        hrtfstate->Values[(Offset+IrSize-1)&HRIR_MASK][0] = 0.0f;
        hrtfstate->Values[(Offset+IrSize-1)&HRIR_MASK][1] = 0.0f;

        ApplyCoeffs(Offset, hrtfstate->Values, IrSize, Coeffs, left, right);
        *(LeftOut++)  += hrtfstate->Values[Offset&HRIR_MASK][0];
        *(RightOut++) += hrtfstate->Values[Offset&HRIR_MASK][1];

        gain += gainstep;
        Offset++;
    }
    hrtfparams->Gain = gain;
}

void MixHrtfBlend(ALfloat *restrict LeftOut, ALfloat *restrict RightOut,
                  const ALfloat *data, ALsizei Offset, ALsizei OutPos,
                  const ALsizei IrSize, const HrtfParams *oldparams,
                  MixHrtfParams *newparams, HrtfState *hrtfstate,
                  ALsizei BufferSize)
{
    const ALfloat (*OldCoeffs)[2] = ASSUME_ALIGNED(oldparams->Coeffs, 16);
    const ALsizei OldDelay[2] = { oldparams->Delay[0], oldparams->Delay[1] };
    ALfloat oldGain = oldparams->Gain;
    ALfloat oldGainStep = -oldGain / (ALfloat)BufferSize;
    const ALfloat (*NewCoeffs)[2] = ASSUME_ALIGNED(newparams->Coeffs, 16);
    const ALsizei NewDelay[2] = { newparams->Delay[0], newparams->Delay[1] };
    ALfloat newGain = newparams->Gain;
    ALfloat newGainStep = newparams->GainStep;
    ALfloat left, right;
    ALsizei i;

    LeftOut  += OutPos;
    RightOut += OutPos;
    for(i = 0;i < BufferSize;i++)
    {
        hrtfstate->Values[(Offset+IrSize-1)&HRIR_MASK][0] = 0.0f;
        hrtfstate->Values[(Offset+IrSize-1)&HRIR_MASK][1] = 0.0f;

        hrtfstate->History[Offset&HRTF_HISTORY_MASK] = *(data++);

        left = hrtfstate->History[(Offset-OldDelay[0])&HRTF_HISTORY_MASK]*oldGain;
        right = hrtfstate->History[(Offset-OldDelay[1])&HRTF_HISTORY_MASK]*oldGain;
        ApplyCoeffs(Offset, hrtfstate->Values, IrSize, OldCoeffs, left, right);

        left = hrtfstate->History[(Offset-NewDelay[0])&HRTF_HISTORY_MASK]*newGain;
        right = hrtfstate->History[(Offset-NewDelay[1])&HRTF_HISTORY_MASK]*newGain;
        ApplyCoeffs(Offset, hrtfstate->Values, IrSize, NewCoeffs, left, right);

        *(LeftOut++)  += hrtfstate->Values[Offset&HRIR_MASK][0];
        *(RightOut++) += hrtfstate->Values[Offset&HRIR_MASK][1];

        oldGain += oldGainStep;
        newGain += newGainStep;
        Offset++;
    }
    newparams->Gain = newGain;
}

void MixDirectHrtf(ALfloat *restrict LeftOut, ALfloat *restrict RightOut,
                   const ALfloat *data, ALsizei Offset, const ALsizei IrSize,
                   const ALfloat (*restrict Coeffs)[2], ALfloat (*restrict Values)[2],
                   ALsizei BufferSize)
{
    ALfloat insample;
    ALsizei i;

    for(i = 0;i < BufferSize;i++)
    {
        Values[(Offset+IrSize)&HRIR_MASK][0] = 0.0f;
        Values[(Offset+IrSize)&HRIR_MASK][1] = 0.0f;
        Offset++;

        insample = *(data++);
        ApplyCoeffs(Offset, Values, IrSize, Coeffs, insample, insample);
        *(LeftOut++)  += Values[Offset&HRIR_MASK][0];
        *(RightOut++) += Values[Offset&HRIR_MASK][1];
    }
}
