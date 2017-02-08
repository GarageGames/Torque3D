#ifndef BFORMATDEC_H
#define BFORMATDEC_H

#include "alMain.h"

struct AmbDecConf;
struct BFormatDec;
struct AmbiUpsampler;

enum BFormatDecFlags {
    BFDF_DistanceComp = 1<<0
};

struct BFormatDec *bformatdec_alloc();
void bformatdec_free(struct BFormatDec *dec);
int bformatdec_getOrder(const struct BFormatDec *dec);
void bformatdec_reset(struct BFormatDec *dec, const struct AmbDecConf *conf, ALuint chancount, ALuint srate, const ALuint chanmap[MAX_OUTPUT_CHANNELS], int flags);

/* Decodes the ambisonic input to the given output channels. */
void bformatdec_process(struct BFormatDec *dec, ALfloat (*restrict OutBuffer)[BUFFERSIZE], ALuint OutChannels, const ALfloat (*restrict InSamples)[BUFFERSIZE], ALuint SamplesToDo);

/* Up-samples a first-order input to the decoder's configuration. */
void bformatdec_upSample(struct BFormatDec *dec, ALfloat (*restrict OutBuffer)[BUFFERSIZE], const ALfloat (*restrict InSamples)[BUFFERSIZE], ALuint InChannels, ALuint SamplesToDo);


/* Stand-alone first-order upsampler. Kept here because it shares some stuff
 * with bformatdec.
 */
struct AmbiUpsampler *ambiup_alloc();
void ambiup_free(struct AmbiUpsampler *ambiup);
void ambiup_reset(struct AmbiUpsampler *ambiup, const ALCdevice *device);

void ambiup_process(struct AmbiUpsampler *ambiup, ALfloat (*restrict OutBuffer)[BUFFERSIZE], ALuint OutChannels, const ALfloat (*restrict InSamples)[BUFFERSIZE], ALuint SamplesToDo);


/* Band splitter. Splits a signal into two phase-matching frequency bands. */
typedef struct BandSplitter {
    ALfloat coeff;
    ALfloat lp_z1;
    ALfloat lp_z2;
    ALfloat hp_z1;
} BandSplitter;

void bandsplit_init(BandSplitter *splitter, ALfloat freq_mult);
void bandsplit_clear(BandSplitter *splitter);
void bandsplit_process(BandSplitter *splitter, ALfloat *restrict hpout, ALfloat *restrict lpout,
                       const ALfloat *input, ALuint count);

#endif /* BFORMATDEC_H */
