#ifndef _AL_ERROR_H_
#define _AL_ERROR_H_

#include "alMain.h"

#ifdef __cplusplus
extern "C" {
#endif

extern ALboolean TrapALError;

ALvoid alSetError(ALCcontext *Context, ALenum errorCode);

#define SET_ERROR_AND_RETURN(ctx, err) do {                                    \
    alSetError((ctx), (err));                                                  \
    return;                                                                    \
} while(0)

#define SET_ERROR_AND_RETURN_VALUE(ctx, err, val) do {                         \
    alSetError((ctx), (err));                                                  \
    return (val);                                                              \
} while(0)

#define SET_ERROR_AND_GOTO(ctx, err, lbl) do {                                 \
    alSetError((ctx), (err));                                                  \
    goto lbl;                                                                  \
} while(0)

#ifdef __cplusplus
}
#endif

#endif
