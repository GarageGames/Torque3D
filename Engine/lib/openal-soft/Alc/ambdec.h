#ifndef AMBDEC_H
#define AMBDEC_H

#include "alstring.h"
#include "alMain.h"

/* Helpers to read .ambdec configuration files. */

enum AmbDecScaleType {
    ADS_N3D,
    ADS_SN3D,
    ADS_FuMa,
};
typedef struct AmbDecConf {
    al_string Description;
    ALuint Version; /* Must be 3 */

    ALuint ChanMask;
    ALuint FreqBands; /* Must be 1 or 2 */
    ALsizei NumSpeakers;
    enum AmbDecScaleType CoeffScale;

    ALfloat XOverFreq;
    ALfloat XOverRatio;

    struct {
        al_string Name;
        ALfloat Distance;
        ALfloat Azimuth;
        ALfloat Elevation;
        al_string Connection;
    } Speakers[MAX_OUTPUT_CHANNELS];

    /* Unused when FreqBands == 1 */
    ALfloat LFOrderGain[MAX_AMBI_ORDER+1];
    ALfloat LFMatrix[MAX_OUTPUT_CHANNELS][MAX_AMBI_COEFFS];

    ALfloat HFOrderGain[MAX_AMBI_ORDER+1];
    ALfloat HFMatrix[MAX_OUTPUT_CHANNELS][MAX_AMBI_COEFFS];
} AmbDecConf;

void ambdec_init(AmbDecConf *conf);
void ambdec_deinit(AmbDecConf *conf);
int ambdec_load(AmbDecConf *conf, const char *fname);

#endif /* AMBDEC_H */
