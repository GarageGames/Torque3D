#ifndef MIXER_DEFS_H
#define MIXER_DEFS_H

#include "AL/alc.h"
#include "AL/al.h"
#include "alMain.h"
#include "alu.h"

struct MixGains;

struct MixHrtfParams;
struct HrtfState;

/* C resamplers */
const ALfloat *Resample_copy32_C(const BsincState *state, const ALfloat *restrict src, ALuint frac, ALuint increment, ALfloat *restrict dst, ALuint dstlen);
const ALfloat *Resample_point32_C(const BsincState *state, const ALfloat *restrict src, ALuint frac, ALuint increment, ALfloat *restrict dst, ALuint dstlen);
const ALfloat *Resample_lerp32_C(const BsincState *state, const ALfloat *restrict src, ALuint frac, ALuint increment, ALfloat *restrict dst, ALuint dstlen);
const ALfloat *Resample_fir4_32_C(const BsincState *state, const ALfloat *restrict src, ALuint frac, ALuint increment, ALfloat *restrict dst, ALuint dstlen);
const ALfloat *Resample_fir8_32_C(const BsincState *state, const ALfloat *restrict src, ALuint frac, ALuint increment, ALfloat *restrict dst, ALuint dstlen);
const ALfloat *Resample_bsinc32_C(const BsincState *state, const ALfloat *restrict src, ALuint frac, ALuint increment, ALfloat *restrict dst, ALuint dstlen);


/* C mixers */
void MixHrtf_C(ALfloat (*restrict OutBuffer)[BUFFERSIZE], ALuint lidx, ALuint ridx,
               const ALfloat *data, ALuint Counter, ALuint Offset, ALuint OutPos,
               const ALuint IrSize, const struct MixHrtfParams *hrtfparams,
               struct HrtfState *hrtfstate, ALuint BufferSize);
void MixDirectHrtf_C(ALfloat (*restrict OutBuffer)[BUFFERSIZE], ALuint lidx, ALuint ridx,
                     const ALfloat *data, ALuint Offset, const ALuint IrSize,
                     ALfloat (*restrict Coeffs)[2], ALfloat (*restrict Values)[2],
                     ALuint BufferSize);
void Mix_C(const ALfloat *data, ALuint OutChans, ALfloat (*restrict OutBuffer)[BUFFERSIZE],
           ALfloat *CurrentGains, const ALfloat *TargetGains, ALuint Counter, ALuint OutPos,
           ALuint BufferSize);
void MixRow_C(ALfloat *OutBuffer, const ALfloat *Gains,
              const ALfloat (*restrict data)[BUFFERSIZE], ALuint InChans,
              ALuint InPos, ALuint BufferSize);

/* SSE mixers */
void MixHrtf_SSE(ALfloat (*restrict OutBuffer)[BUFFERSIZE], ALuint lidx, ALuint ridx,
                 const ALfloat *data, ALuint Counter, ALuint Offset, ALuint OutPos,
                 const ALuint IrSize, const struct MixHrtfParams *hrtfparams,
                 struct HrtfState *hrtfstate, ALuint BufferSize);
void MixDirectHrtf_SSE(ALfloat (*restrict OutBuffer)[BUFFERSIZE], ALuint lidx, ALuint ridx,
                       const ALfloat *data, ALuint Offset, const ALuint IrSize,
                       ALfloat (*restrict Coeffs)[2], ALfloat (*restrict Values)[2],
                       ALuint BufferSize);
void Mix_SSE(const ALfloat *data, ALuint OutChans, ALfloat (*restrict OutBuffer)[BUFFERSIZE],
             ALfloat *CurrentGains, const ALfloat *TargetGains, ALuint Counter, ALuint OutPos,
             ALuint BufferSize);
void MixRow_SSE(ALfloat *OutBuffer, const ALfloat *Gains,
                const ALfloat (*restrict data)[BUFFERSIZE], ALuint InChans,
                ALuint InPos, ALuint BufferSize);

/* SSE resamplers */
inline void InitiatePositionArrays(ALuint frac, ALuint increment, ALuint *restrict frac_arr, ALuint *restrict pos_arr, ALuint size)
{
    ALuint i;

    pos_arr[0] = 0;
    frac_arr[0] = frac;
    for(i = 1;i < size;i++)
    {
        ALuint frac_tmp = frac_arr[i-1] + increment;
        pos_arr[i] = pos_arr[i-1] + (frac_tmp>>FRACTIONBITS);
        frac_arr[i] = frac_tmp&FRACTIONMASK;
    }
}

const ALfloat *Resample_bsinc32_SSE(const BsincState *state, const ALfloat *restrict src, ALuint frac,
                                    ALuint increment, ALfloat *restrict dst, ALuint dstlen);

const ALfloat *Resample_lerp32_SSE2(const BsincState *state, const ALfloat *restrict src,
                                    ALuint frac, ALuint increment, ALfloat *restrict dst,
                                    ALuint numsamples);
const ALfloat *Resample_lerp32_SSE41(const BsincState *state, const ALfloat *restrict src,
                                     ALuint frac, ALuint increment, ALfloat *restrict dst,
                                     ALuint numsamples);

const ALfloat *Resample_fir4_32_SSE3(const BsincState *state, const ALfloat *restrict src,
                                     ALuint frac, ALuint increment, ALfloat *restrict dst,
                                     ALuint numsamples);
const ALfloat *Resample_fir4_32_SSE41(const BsincState *state, const ALfloat *restrict src,
                                      ALuint frac, ALuint increment, ALfloat *restrict dst,
                                      ALuint numsamples);

const ALfloat *Resample_fir8_32_SSE3(const BsincState *state, const ALfloat *restrict src,
                                     ALuint frac, ALuint increment, ALfloat *restrict dst,
                                     ALuint numsamples);
const ALfloat *Resample_fir8_32_SSE41(const BsincState *state, const ALfloat *restrict src,
                                      ALuint frac, ALuint increment, ALfloat *restrict dst,
                                      ALuint numsamples);

/* Neon mixers */
void MixHrtf_Neon(ALfloat (*restrict OutBuffer)[BUFFERSIZE], ALuint lidx, ALuint ridx,
                  const ALfloat *data, ALuint Counter, ALuint Offset, ALuint OutPos,
                  const ALuint IrSize, const struct MixHrtfParams *hrtfparams,
                  struct HrtfState *hrtfstate, ALuint BufferSize);
void MixDirectHrtf_Neon(ALfloat (*restrict OutBuffer)[BUFFERSIZE], ALuint lidx, ALuint ridx,
                        const ALfloat *data, ALuint Offset, const ALuint IrSize,
                        ALfloat (*restrict Coeffs)[2], ALfloat (*restrict Values)[2],
                        ALuint BufferSize);
void Mix_Neon(const ALfloat *data, ALuint OutChans, ALfloat (*restrict OutBuffer)[BUFFERSIZE],
              ALfloat *CurrentGains, const ALfloat *TargetGains, ALuint Counter, ALuint OutPos,
              ALuint BufferSize);
void MixRow_Neon(ALfloat *OutBuffer, const ALfloat *Gains,
                 const ALfloat (*restrict data)[BUFFERSIZE], ALuint InChans,
                 ALuint InPos, ALuint BufferSize);

#endif /* MIXER_DEFS_H */
