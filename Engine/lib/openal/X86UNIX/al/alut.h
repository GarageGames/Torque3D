#ifndef _ALUT_H_
#define _ALUT_H_

#include <AL/altypes.h>
#include <AL/aluttypes.h>

#ifdef _WIN32
#define ALAPI         __declspec(dllexport)
#define ALAPIENTRY    __cdecl
#define AL_CALLBACK
#else  /* _WIN32 */
#define ALAPI
#define ALAPIENTRY
#define AL_CALLBACK
#endif /* _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AL_NO_PROTOTYPES

ALAPI void ALAPIENTRY alutInit(int *argc, char *argv[]);
ALAPI void ALAPIENTRY alutExit(ALvoid);

ALAPI ALboolean ALAPIENTRY alutLoadWAV( const char *fname,
                        ALvoid **wave,
			ALsizei *format,
			ALsizei *size,
			ALsizei *bits,
			ALsizei *freq );

#else
//
//      void 	(*alutInit)(int *argc, char *argv[]);
//      void 	(*alutExit)(ALvoid);
//
//      ALboolean 	(*alutLoadWAV)( const char *fname,
//                        ALvoid **wave,
//			ALsizei *format,
//			ALsizei *size,
//			ALsizei *bits,
//			ALsizei *freq );
//
//
#endif /* AL_NO_PROTOTYPES */


#ifdef __cplusplus
}
#endif

#endif
