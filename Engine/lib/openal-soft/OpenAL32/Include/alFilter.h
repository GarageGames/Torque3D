#ifndef _AL_FILTER_H_
#define _AL_FILTER_H_

#include "AL/alc.h"
#include "AL/al.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOWPASSFREQREF  (5000.0f)
#define HIGHPASSFREQREF  (250.0f)


struct ALfilter;

typedef struct ALfilterVtable {
    void (*const setParami)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALint val);
    void (*const setParamiv)(struct ALfilter *filter, ALCcontext *context, ALenum param, const ALint *vals);
    void (*const setParamf)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALfloat val);
    void (*const setParamfv)(struct ALfilter *filter, ALCcontext *context, ALenum param, const ALfloat *vals);

    void (*const getParami)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALint *val);
    void (*const getParamiv)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALint *vals);
    void (*const getParamf)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *val);
    void (*const getParamfv)(struct ALfilter *filter, ALCcontext *context, ALenum param, ALfloat *vals);
} ALfilterVtable;

#define DEFINE_ALFILTER_VTABLE(T)           \
const struct ALfilterVtable T##_vtable = {  \
    T##_setParami, T##_setParamiv,          \
    T##_setParamf, T##_setParamfv,          \
    T##_getParami, T##_getParamiv,          \
    T##_getParamf, T##_getParamfv,          \
}

typedef struct ALfilter {
    // Filter type (AL_FILTER_NULL, ...)
    ALenum type;

    ALfloat Gain;
    ALfloat GainHF;
    ALfloat HFReference;
    ALfloat GainLF;
    ALfloat LFReference;

    const struct ALfilterVtable *vtab;

    /* Self ID */
    ALuint id;
} ALfilter;
#define ALfilter_setParami(o, c, p, v)   ((o)->vtab->setParami(o, c, p, v))
#define ALfilter_setParamf(o, c, p, v)   ((o)->vtab->setParamf(o, c, p, v))
#define ALfilter_setParamiv(o, c, p, v)  ((o)->vtab->setParamiv(o, c, p, v))
#define ALfilter_setParamfv(o, c, p, v)  ((o)->vtab->setParamfv(o, c, p, v))
#define ALfilter_getParami(o, c, p, v)   ((o)->vtab->getParami(o, c, p, v))
#define ALfilter_getParamf(o, c, p, v)   ((o)->vtab->getParamf(o, c, p, v))
#define ALfilter_getParamiv(o, c, p, v)  ((o)->vtab->getParamiv(o, c, p, v))
#define ALfilter_getParamfv(o, c, p, v)  ((o)->vtab->getParamfv(o, c, p, v))

void ReleaseALFilters(ALCdevice *device);

#ifdef __cplusplus
}
#endif

#endif
