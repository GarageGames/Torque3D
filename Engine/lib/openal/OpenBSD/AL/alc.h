#ifndef ALC_CONTEXT_H_
#define ALC_CONTEXT_H_

#include <AL/alctypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ALC_VERSION_0_1         1

#ifdef _WIN32
#define ALAPI      __declspec(dllexport)
#define ALAPIENTRY __cdecl
#else  /* _WIN32 */
#define ALAPI
#define ALAPIENTRY
#define AL_CALLBACK
#endif /* _WIN32 */

#ifndef AL_NO_PROTOTYPES

ALAPI void * ALAPIENTRY alcCreateContext( ALint* attrlist );

/**
 * There is no current context, as we can mix
 *  several active contexts. But al* calls
 *  only affect the current context.
 */
ALAPI ALCenum ALAPIENTRY alcMakeContextCurrent( ALvoid *alcHandle );

/** ??? */
ALAPI void *  ALAPIENTRY alcUpdateContext( ALvoid *alcHandle );

ALAPI ALCenum ALAPIENTRY alcDestroyContext( ALvoid *alcHandle );

ALAPI ALCenum ALAPIENTRY alcGetError( ALvoid );

ALAPI const ALubyte * ALAPIENTRY alcGetErrorString(ALenum param);

ALAPI void * ALAPIENTRY alcGetCurrentContext( ALvoid );

#else
//
//      void *	(*alcCreateContext)( ALint* attrlist );
//
//      /**
//       * There is no current context, as we can mix
//       *  several active contexts. But al* calls
//       *  only affect the current context.
//       */
//      ALCenum	(*alcMakeContextCurrent)( ALvoid *alcHandle );
//
//      /** ??? */
//      void *	(*alcUpdateContext)( ALvoid *alcHandle );
//      
//      ALCenum	(*alcDestroyContext)( ALvoid *alcHandle );
//
//      ALCenum	(*alcGetError) ( ALvoid );
//
//      const ALubyte *(*alcGetErrorString)(ALenum param);
//
//      void *         (*alcGetCurrentContext)( ALvoid );
//
#endif /* AL_NO_PROTOTYPES */

#ifdef __cplusplus
}
#endif

#endif /* ALC_CONTEXT_H_ */
