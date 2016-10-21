#ifndef ALC_HRTF_H
#define ALC_HRTF_H

#include "AL/al.h"
#include "AL/alc.h"

#include "alstring.h"


struct Hrtf {
    ALuint sampleRate;
    ALuint irSize;
    ALubyte evCount;

    const ALubyte *azCount;
    const ALushort *evOffset;
    const ALshort *coeffs;
    const ALubyte *delays;

    const char *filename;
    struct Hrtf *next;
};

typedef struct HrtfEntry {
    al_string name;

    const struct Hrtf *hrtf;
} HrtfEntry;
TYPEDEF_VECTOR(HrtfEntry, vector_HrtfEntry)

#define HRIR_BITS        (7)
#define HRIR_LENGTH      (1<<HRIR_BITS)
#define HRIR_MASK        (HRIR_LENGTH-1)
#define HRTFDELAY_BITS    (20)
#define HRTFDELAY_FRACONE (1<<HRTFDELAY_BITS)
#define HRTFDELAY_MASK    (HRTFDELAY_FRACONE-1)

void FreeHrtfs(void);

vector_HrtfEntry EnumerateHrtf(const_al_string devname);
void FreeHrtfList(vector_HrtfEntry *list);

void GetHrtfCoeffs(const struct Hrtf *Hrtf, ALfloat elevation, ALfloat azimuth, ALfloat spread, ALfloat gain, ALfloat (*coeffs)[2], ALuint *delays);

/* Produces HRTF filter coefficients for decoding B-Format. The result will
 * have ACN ordering with N3D normalization. NumChannels must currently be 4,
 * for first-order. Returns the maximum impulse-response length of the
 * generated coefficients.
 */
ALuint BuildBFormatHrtf(const struct Hrtf *Hrtf, ALfloat (*coeffs)[HRIR_LENGTH][2], ALuint NumChannels);

#endif /* ALC_HRTF_H */
