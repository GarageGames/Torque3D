#include "platform/platform.h"

bool Platform::openWebBrowser( const char* webAddress )
{
    return false; // TODO LINUX
}

#ifdef TORQUE_DEDICATED
// XA: New class for the unix unicode font
class PlatformFont;
PlatformFont *createPlatformFont(const char *name, dsize_t size, U32 charset /* = TGE_ANSI_CHARSET */) { return NULL; }
#endif
