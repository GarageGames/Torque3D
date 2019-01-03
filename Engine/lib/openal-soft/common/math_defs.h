#ifndef AL_MATH_DEFS_H
#define AL_MATH_DEFS_H

#include <math.h>
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define F_PI    (3.14159265358979323846f)
#define F_PI_2  (1.57079632679489661923f)
#define F_TAU   (6.28318530717958647692f)

#ifndef FLT_EPSILON
#define FLT_EPSILON (1.19209290e-07f)
#endif

#ifndef HUGE_VALF
static const union msvc_inf_hack {
    unsigned char b[4];
    float f;
} msvc_inf_union = {{ 0x00, 0x00, 0x80, 0x7F }};
#define HUGE_VALF (msvc_inf_union.f)
#endif

#ifndef HAVE_LOG2F
static inline float log2f(float f)
{
    return logf(f) / logf(2.0f);
}
#endif

#ifndef HAVE_CBRTF
static inline float cbrtf(float f)
{
    return powf(f, 1.0f/3.0f);
}
#endif

#define DEG2RAD(x)  ((float)(x) * (F_PI/180.0f))
#define RAD2DEG(x)  ((float)(x) * (180.0f/F_PI))

#endif /* AL_MATH_DEFS_H */
