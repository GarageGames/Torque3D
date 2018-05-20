#ifndef MASTERING_H
#define MASTERING_H

#include "AL/al.h"

/* For BUFFERSIZE. */
#include "alMain.h"

typedef struct Compressor {
    ALfloat PreGain;
    ALfloat PostGain;
    ALboolean SummedLink;
    ALfloat AttackMin;
    ALfloat AttackMax;
    ALfloat ReleaseMin;
    ALfloat ReleaseMax;
    ALfloat Ratio;
    ALfloat Threshold;
    ALfloat Knee;
    ALuint SampleRate;

    ALuint RmsSum;
    ALuint *RmsWindow;
    ALsizei RmsIndex;
    ALfloat Envelope[BUFFERSIZE];
    ALfloat EnvLast;
} Compressor;

/* The compressor requires the following information for proper
 * initialization:
 *
 *   PreGainDb      - Gain applied before detection (in dB).
 *   PostGainDb     - Gain applied after compression (in dB).
 *   SummedLink     - Whether to use summed (true) or maxed (false) linking.
 *   RmsSensing     - Whether to use RMS (true) or Peak (false) sensing.
 *   AttackTimeMin  - Minimum attack time (in seconds).
 *   AttackTimeMax  - Maximum attack time.  Automates when min != max.
 *   ReleaseTimeMin - Minimum release time (in seconds).
 *   ReleaseTimeMax - Maximum release time.  Automates when min != max.
 *   Ratio          - Compression ratio (x:1).  Set to 0 for true limiter.
 *   ThresholdDb    - Triggering threshold (in dB).
 *   KneeDb         - Knee width (below threshold; in dB).
 *   SampleRate     - Sample rate to process.
 */
Compressor *CompressorInit(const ALfloat PreGainDb, const ALfloat PostGainDb,
    const ALboolean SummedLink, const ALboolean RmsSensing, const ALfloat AttackTimeMin,
    const ALfloat AttackTimeMax, const ALfloat ReleaseTimeMin, const ALfloat ReleaseTimeMax,
    const ALfloat Ratio, const ALfloat ThresholdDb, const ALfloat KneeDb,
    const ALuint SampleRate);

void ApplyCompression(struct Compressor *Comp, const ALsizei NumChans, const ALsizei SamplesToDo,
                      ALfloat (*restrict OutBuffer)[BUFFERSIZE]);

inline ALuint GetCompressorSampleRate(const Compressor *Comp)
{ return Comp->SampleRate; }

#endif /* MASTERING_H */
