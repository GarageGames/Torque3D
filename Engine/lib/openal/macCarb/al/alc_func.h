
//AL_FUNCTION(ALCubyte*,   alcGetString, (ALCdevice *device,ALCenum param), return NULL; )
//AL_FUNCTION(ALCvoid,     alcGetIntegerv, (ALCdevice * device,ALCenum param,ALCsizei size,ALCint *data), return; )

AL_FUNCTION(ALCdevice*,  alcOpenDevice, (ALubyte *deviceName), return NULL; )
AL_FUNCTION(ALCvoid,     alcCloseDevice, (ALCdevice *device), return; )

AL_FUNCTION(ALCcontext*, alcCreateContext, (ALCdevice *device,ALCint *attrList), return NULL; )
AL_FUNCTION(ALCboolean,  alcMakeContextCurrent, (ALCcontext *context), return AL_FALSE; )
AL_FUNCTION(ALCvoid,     alcProcessContext, (ALCcontext *context), return; )
AL_FUNCTION(ALCcontext*, alcGetCurrentContext, (ALCvoid), return NULL; )
AL_FUNCTION(ALCdevice*,  alcGetContextsDevice, (ALCcontext *context), return NULL; )
AL_FUNCTION(ALCvoid,     alcSuspendContext, (ALCcontext *context), return; )
AL_FUNCTION(ALCvoid,     alcDestroyContext, (ALCcontext *context), return; )

AL_FUNCTION(ALCenum,     alcGetError, (ALCdevice *device), return ALC_INVALID_DEVICE;  )

//AL_FUNCTION(ALCboolean,  alcIsExtensionPresent, (ALCdevice *device,ALCubyte *extName), return AL_FALSE; )
//AL_FUNCTION(ALCvoid*,    alcGetProcAddress, (ALCdevice *device,ALCubyte *funcName), return NULL; )
//AL_FUNCTION(ALCenum,     alcGetEnumValue, (ALCdevice *device,ALCubyte *enumName), return ALC_INVALID_ENUM; )
