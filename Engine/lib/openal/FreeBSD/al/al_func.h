
AL_FUNCTION(ALvoid,     alEnable, ( ALenum capability ), return; )
AL_FUNCTION(ALvoid,     alDisable, ( ALenum capability ), return; ) 
AL_FUNCTION(ALboolean,  alIsEnabled, ( ALenum capability ), return AL_FALSE; ) 

//AL_FUNCTION(ALvoid,     alHint, ( ALenum target, ALenum mode ), return; )

AL_FUNCTION(ALboolean,  alGetBoolean, ( ALenum param ), return AL_FALSE; )
AL_FUNCTION(ALint,      alGetInteger, ( ALenum param ), return 0; )
AL_FUNCTION(ALfloat,    alGetFloat, ( ALenum param ), return 0.0f; )
AL_FUNCTION(ALdouble,   alGetDouble, ( ALenum param ), return 0.0; )
AL_FUNCTION(ALvoid,     alGetBooleanv, ( ALenum param, ALboolean* data ), return; )
AL_FUNCTION(ALvoid,     alGetIntegerv, ( ALenum param, ALint* data ), return; )
AL_FUNCTION(ALvoid,     alGetFloatv, ( ALenum param, ALfloat* data ), return; )
AL_FUNCTION(ALvoid,     alGetDoublev, ( ALenum param, ALdouble* data ), return; )
AL_FUNCTION(ALubyte*,   alGetString, ( ALenum param ), return NULL; )

AL_FUNCTION(ALenum,     alGetError, ( ALvoid ), return AL_INVALID_VALUE; )
AL_FUNCTION(ALboolean,  alIsExtensionPresent, ( ALubyte* fname ), return AL_FALSE; )
AL_FUNCTION(ALvoid*,    alGetProcAddress, ( ALubyte* fname ), return NULL; )
AL_FUNCTION(ALenum,     alGetEnumValue, ( ALubyte* ename ), return AL_INVALID_ENUM; )

AL_FUNCTION(ALvoid,     alListeneri, ( ALenum param, ALint value ), return; )
AL_FUNCTION(ALvoid,     alListenerf, ( ALenum param, ALfloat value ), return; )
AL_FUNCTION(ALvoid,     alListener3f, ( ALenum param, ALfloat v1, ALfloat v2, ALfloat v3 ), return; ) 
AL_FUNCTION(ALvoid,     alListenerfv, ( ALenum param, ALfloat* values ), return; ) 

AL_FUNCTION(ALvoid,     alGetListeneri, ( ALenum param, ALint* value ), return; )
AL_FUNCTION(ALvoid,     alGetListenerf, ( ALenum param, ALfloat* value ), return; )
AL_FUNCTION(ALvoid,     alGetListener3f, ( ALenum param, ALfloat* v1, ALfloat* v2, ALfloat* v3 ), return; ) 
AL_FUNCTION(ALvoid,     alGetListenerfv, ( ALenum param, ALfloat* values ), return; ) 

AL_FUNCTION(ALvoid,     alGenSources, ( ALsizei n, ALuint* sources ), return; ) 
AL_FUNCTION(ALvoid,     alDeleteSources, ( ALsizei n, ALuint* sources ), return; )
AL_FUNCTION(ALboolean,  alIsSource, ( ALuint id ), return AL_FALSE; ) 

AL_FUNCTION(ALvoid,     alSourcei, ( ALuint source, ALenum param, ALint value ), return; ) 
AL_FUNCTION(ALvoid,     alSourcef, ( ALuint source, ALenum param, ALfloat value ), return; ) 
AL_FUNCTION(ALvoid,     alSource3f, ( ALuint source, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3 ), return; )
AL_FUNCTION(ALvoid,     alSourcefv, ( ALuint source, ALenum param, ALfloat* values ), return; ) 
AL_FUNCTION(ALvoid,     alGetSourcei, ( ALuint source,  ALenum param, ALint* value ), return; )
AL_FUNCTION(ALvoid,     alGetSourcef, ( ALuint source,  ALenum param, ALfloat* value ), return; )
//AL_FUNCTION(ALvoid,     alGetSource3f, ( ALuint source,  ALenum param, ALfloat* v1, ALfloat* v2, ALfloat* v3 ), return; )
AL_FUNCTION(ALvoid,     alGetSourcefv, ( ALuint source, ALenum param, ALfloat* values ), return; )

AL_FUNCTION(ALvoid,     alSourcePlayv, ( ALsizei n, ALuint *sources ), return; )
AL_FUNCTION(ALvoid,     alSourcePausev, ( ALsizei n, ALuint *sources ), return; )
AL_FUNCTION(ALvoid,     alSourceStopv, ( ALsizei n, ALuint *sources ), return; )
AL_FUNCTION(ALvoid,     alSourceRewindv, (ALsizei n,ALuint *sources), return; )
AL_FUNCTION(ALvoid,     alSourcePlay, ( ALuint source ), return; )
AL_FUNCTION(ALvoid,     alSourcePause, ( ALuint source ), return; )
AL_FUNCTION(ALvoid,     alSourceStop, ( ALuint source ), return; )
AL_FUNCTION(ALvoid,     alSourceRewind, ( ALuint source ), return; )

AL_FUNCTION(ALvoid,     alGenBuffers, ( ALsizei n, ALuint* buffers ), return; )
AL_FUNCTION(ALvoid,     alDeleteBuffers, ( ALsizei n, ALuint* buffers ), return; )
AL_FUNCTION(ALboolean,  alIsBuffer, ( ALuint buffer ), return AL_FALSE; )
AL_FUNCTION(ALvoid,     alBufferData, ( ALuint buffer, ALenum format, ALvoid* data, ALsizei size, ALsizei freq ), return; )
AL_FUNCTION(ALvoid,     alGetBufferi, ( ALuint buffer, ALenum param, ALint*   value ), return; )
AL_FUNCTION(ALvoid,     alGetBufferf, ( ALuint buffer, ALenum param, ALfloat* value ), return; )

AL_FUNCTION(ALvoid,     alSourceQueueBuffers, ( ALuint source, ALsizei n, ALuint* buffers ), return; )
AL_FUNCTION(ALvoid,     alSourceUnqueueBuffers, ( ALuint source, ALsizei n, ALuint* buffers ), return; )

AL_FUNCTION(ALvoid,     alDistanceModel, ( ALenum value ), return; )
AL_FUNCTION(ALvoid,     alDopplerFactor, ( ALfloat value ), return; )
AL_FUNCTION(ALvoid,     alDopplerVelocity, ( ALfloat value ), return; )


