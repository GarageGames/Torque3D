#ifndef _AL_ERROR_H_
#define _AL_ERROR_H_

#include "alMain.h"
#include "logging.h"

#ifdef __cplusplus
extern "C" {
#endif

extern ALboolean TrapALError;

void alSetError(ALCcontext *context, ALenum errorCode, const char *msg, ...) DECL_FORMAT(printf, 3, 4);

#define SETERR_GOTO(ctx, err, lbl, ...) do {                                   \
    alSetError((ctx), (err), __VA_ARGS__);                                     \
    goto lbl;                                                                  \
} while(0)

#define SETERR_RETURN(ctx, err, retval, ...) do {                              \
    alSetError((ctx), (err), __VA_ARGS__);                                     \
    return retval;                                                             \
} while(0)

#ifdef __cplusplus
}
#endif

#endif
