#ifndef __alu_h_
#define __alu_h_

#ifdef _WIN32
#define ALAPI       __declspec(dllexport)
#define ALAPIENTRY  __cdecl
#else  /* _WIN32 */
#define ALAPI
#define ALAPIENTRY
#define AL_CALLBACK
#endif /* _WIN32 */

#include <AL/al.h>
#include <AL/alutypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AL_NO_PROTOTYPES



#else





#endif /* AL_NO_PROTOTYPES */

#ifdef __cplusplus
}
#endif

#endif /* __alu_h_ */

