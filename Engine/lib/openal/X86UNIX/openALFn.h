#ifndef AL_FUNCTION
#define AL_FUNCTION(fn_return, fn_name, fn_args);
#endif

#ifndef AL_EXTENSION
#define AL_EXTENSION(ext_name)
#endif

#ifndef AL_EXT_FUNCTION
#define AL_EXT_FUNCTION(ext_name, fn_return, fn_name, fn_args)
#endif

// AL functions
AL_FUNCTION(ALvoid, alEnable, ( ALenum capability ))
AL_FUNCTION(ALvoid, alDisable, ( ALenum capability ))
AL_FUNCTION(ALboolean, alIsEnabled, ( ALenum capability ))
AL_FUNCTION(ALvoid, alHint, ( ALenum target, ALenum mode ))
AL_FUNCTION(ALboolean, alGetBoolean, ( ALenum param ))
AL_FUNCTION(ALint, alGetInteger, ( ALenum param ))
AL_FUNCTION(ALfloat, alGetFloat, ( ALenum param ))
AL_FUNCTION(ALdouble, alGetDouble, ( ALenum param ))
AL_FUNCTION(ALvoid, alGetBooleanv, ( ALenum param, ALboolean* data ))
AL_FUNCTION(ALvoid, alGetIntegerv, ( ALenum param, ALint* data ))
AL_FUNCTION(ALvoid, alGetFloatv, ( ALenum param, ALfloat* data ))
AL_FUNCTION(ALvoid, alGetDoublev, ( ALenum param, ALdouble* data ))
AL_FUNCTION(const ALubyte*, alGetString, ( ALenum param ))
AL_FUNCTION(ALenum, alGetError, ( ALvoid ))
AL_FUNCTION(ALboolean, alIsExtensionPresent, ( const ALubyte* fname ))
AL_FUNCTION(ALvoid*, alGetProcAddress, ( const ALubyte* fname ))
AL_FUNCTION(ALenum, alGetEnumValue, ( const ALubyte* ename ))
AL_FUNCTION(ALvoid, alListenerf, ( ALenum pname, ALfloat param ))
AL_FUNCTION(ALvoid, alListener3f, ( ALenum pname, ALfloat param1, ALfloat param2, ALfloat param3 ))
AL_FUNCTION(ALvoid, alListenerfv, ( ALenum pname, ALfloat* param ))
AL_FUNCTION(ALvoid, alGetListeneri, ( ALenum pname, ALint* value ))
AL_FUNCTION(ALvoid, alGetListenerf, ( ALenum pname, ALfloat* values ))
AL_FUNCTION(ALvoid, alGetListenerfv, ( ALenum pname, ALfloat* values ))
AL_FUNCTION(ALvoid, alGenSources, ( ALsizei n, ALuint* sources ))
AL_FUNCTION(ALvoid, alDeleteSources, ( ALsizei n, ALuint* sources ))
AL_FUNCTION(ALboolean, alIsSource, ( ALuint sid ))
AL_FUNCTION(ALvoid, alSourcei, ( ALuint sid, ALenum param, ALint value ))
AL_FUNCTION(ALvoid, alSourcef, ( ALuint sid, ALenum param, ALfloat value ))
AL_FUNCTION(ALvoid, alSource3f, ( ALuint sid, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3 ))
AL_FUNCTION(ALvoid, alSourcefv, ( ALuint sid, ALenum param, ALfloat* values ))
AL_FUNCTION(ALvoid, alGetSourcei, ( ALuint sid, ALenum pname, ALint* value ))
AL_FUNCTION(ALvoid, alGetSourcef, ( ALuint sid, ALenum pname, ALfloat* value ))
AL_FUNCTION(ALvoid, alGetSourcefv, ( ALuint sid, ALenum pname, ALfloat* values ))
AL_FUNCTION(ALvoid, alSourcePlayv, ( ALuint ns, ALuint* ids ))
AL_FUNCTION(ALvoid, alSourceStopv, ( ALuint ns, ALuint* ids ))
AL_FUNCTION(ALvoid, alSourcePlay, ( ALuint sid ))
AL_FUNCTION(ALvoid, alSourcePause, ( ALuint sid ))
AL_FUNCTION(ALvoid, alSourceStop, ( ALuint sid ))
AL_FUNCTION(ALvoid, alGenBuffers, ( ALsizei n, ALuint* samples ))
AL_FUNCTION(ALvoid, alDeleteBuffers, ( ALsizei n, ALuint* samples ))
AL_FUNCTION(ALboolean, alIsBuffer, ( ALuint buffer ))
AL_FUNCTION(ALvoid, alBufferData, ( ALuint buffer, ALenum format, ALvoid* data, ALsizei size, ALsizei freq ))
AL_FUNCTION(ALsizei, alBufferAppendData, ( ALuint buffer, ALenum format, ALvoid* data, ALsizei size, ALsizei freq ))
AL_FUNCTION(ALvoid, alGetBufferi, ( ALuint buffer, ALenum param, ALint*   value ))
AL_FUNCTION(ALvoid, alGetBufferf, ( ALuint buffer, ALenum param, ALfloat* value ))

// ALC functions
AL_FUNCTION(ALvoid*, alcCreateContext, ( ALint* attrlist ))
AL_FUNCTION(ALCenum, alcMakeContextCurrent, ( ALvoid* context ))
AL_FUNCTION(ALvoid*, alcUpdateContext, ( ALvoid* context ))
AL_FUNCTION(ALCenum, alcDestroyContext, ( ALvoid* context ))
AL_FUNCTION(ALCenum, alcGetError, ( ALvoid ))
AL_FUNCTION(const ALubyte *, alcGetErrorString, ( ALvoid ))
AL_FUNCTION(ALvoid*, alcGetCurrentContext, ( ALvoid ))

// ALUT functions
AL_FUNCTION(void, alutInit, ( int* argc, char** argv ))
AL_FUNCTION(void, alutExit, ( ALvoid ))
AL_FUNCTION(ALboolean, alutLoadWAV, ( const char* fname, ALvoid** data, ALsizei* format, ALsizei* size, ALsizei* bits, ALsizei* freq ))

// Extensions
AL_EXTENSION(AL_EXT_IASIG)
AL_EXT_FUNCTION(AL_EXT_IASIG, ALvoid, alGenEnvironmentIASIG, ( ALsizei n, ALuint* environs ))
AL_EXT_FUNCTION(AL_EXT_IASIG, ALvoid, alDeleteEnvironmentIASIG, ( ALsizei n, ALuint* environs ))
AL_EXT_FUNCTION(AL_EXT_IASIG, ALboolean, alIsEnvironmentIASIG, ( ALuint environment ))
AL_EXT_FUNCTION(AL_EXT_IASIG, ALvoid, alEnvironmentiIASIG, ( ALuint eid, ALenum param, ALint value ))

AL_EXTENSION(AL_EXT_DYNAMIX)
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALboolean, alBufferi_EXT, ( ALuint buffer, ALenum pname, ALint value ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALboolean, alBufferSyncData_EXT, ( ALuint buffer, ALenum format, ALvoid* data, ALsizei size, ALsizei freq ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALboolean, alBufferStreamFile_EXT, ( ALuint buffer, const ALubyte* filename ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALboolean, alSourceCallback_EXT, ( ALuint source, ALvoid* callback ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALvoid, alSourceResetEnvironment_EXT, ( ALuint source ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALboolean, alContexti_EXT, ( ALenum pname, ALint value ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALboolean, alGetContexti_EXT, ( ALenum pname, ALint* value ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALboolean, alGetContextstr_EXT, ( ALenum pname, ALuint idx, ALubyte** value ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALboolean, alCaptureInit_EXT, ( ALenum format, ALuint rate, ALsizei bufferSize ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALboolean, alCaptureDestroy_EXT, ( ALvoid ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALboolean, alCaptureStart_EXT, ( ALvoid ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALboolean, alCaptureStop_EXT, ( ALvoid ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALsizei, alCaptureGetData_EXT, ( ALvoid* data, ALsizei n, ALenum format, ALuint rate ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALvoid, alEnvironmentfIASIG, ( ALuint eid, ALenum param, ALfloat value ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALvoid, alGetEnvironmentiIASIG_EXT, ( ALuint eid, ALenum param, ALint * value ))
AL_EXT_FUNCTION(AL_EXT_DYNAMIX, ALvoid, alGetEnvironmentfIASIG_EXT, ( ALuint eid, ALenum param, ALfloat * value ))

#undef AL_EXTENSION
#undef AL_FUNCTION
#undef AL_EXT_FUNCTION
