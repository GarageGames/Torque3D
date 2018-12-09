#ifndef FILTER_SPLITTER_H
#define FILTER_SPLITTER_H

#include "alMain.h"


/* Band splitter. Splits a signal into two phase-matching frequency bands. */
typedef struct BandSplitter {
    ALfloat coeff;
    ALfloat lp_z1;
    ALfloat lp_z2;
    ALfloat hp_z1;
} BandSplitter;

void bandsplit_init(BandSplitter *splitter, ALfloat f0norm);
void bandsplit_clear(BandSplitter *splitter);
void bandsplit_process(BandSplitter *splitter, ALfloat *restrict hpout, ALfloat *restrict lpout,
                       const ALfloat *input, ALsizei count);

/* The all-pass portion of the band splitter. Applies the same phase shift
 * without splitting the signal.
 */
typedef struct SplitterAllpass {
    ALfloat coeff;
    ALfloat z1;
} SplitterAllpass;

void splitterap_init(SplitterAllpass *splitter, ALfloat f0norm);
void splitterap_clear(SplitterAllpass *splitter);
void splitterap_process(SplitterAllpass *splitter, ALfloat *restrict samples, ALsizei count);


typedef struct FrontStablizer {
    SplitterAllpass APFilter[MAX_OUTPUT_CHANNELS];
    BandSplitter LFilter, RFilter;
    alignas(16) ALfloat LSplit[2][BUFFERSIZE];
    alignas(16) ALfloat RSplit[2][BUFFERSIZE];
} FrontStablizer;

#endif /* FILTER_SPLITTER_H */
