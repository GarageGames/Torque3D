#ifndef FPU_MODES_H
#define FPU_MODES_H

#ifdef HAVE_FENV_H
#include <fenv.h>
#endif


typedef struct FPUCtl {
#if defined(__GNUC__) && defined(HAVE_SSE)
    unsigned int sse_state;
#elif defined(HAVE___CONTROL87_2)
    unsigned int state;
    unsigned int sse_state;
#elif defined(HAVE__CONTROLFP)
    unsigned int state;
#endif
} FPUCtl;
void SetMixerFPUMode(FPUCtl *ctl);
void RestoreFPUMode(const FPUCtl *ctl);

#ifdef __GNUC__
/* Use an alternate macro set with GCC to avoid accidental continue or break
 * statements within the mixer mode.
 */
#define START_MIXER_MODE() __extension__({ FPUCtl _oldMode; SetMixerFPUMode(&_oldMode)
#define END_MIXER_MODE() RestoreFPUMode(&_oldMode); })
#else
#define START_MIXER_MODE() do { FPUCtl _oldMode; SetMixerFPUMode(&_oldMode)
#define END_MIXER_MODE() RestoreFPUMode(&_oldMode); } while(0)
#endif
#define LEAVE_MIXER_MODE() RestoreFPUMode(&_oldMode)

#endif /* FPU_MODES_H */
