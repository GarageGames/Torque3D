//alut_func.h

AL_FUNCTION(ALvoid,  alutInit, (ALint *argc,ALbyte **argv), return; )
AL_FUNCTION(ALvoid,  alutExit, (ALvoid), return; )
AL_FUNCTION(ALvoid,  alutLoadWAVFile, (ALbyte *file,ALenum *format,ALvoid **data,ALsizei *size,ALsizei *freq,ALboolean *loop), return; )
AL_FUNCTION(ALvoid,  alutLoadWAVMemory, (ALbyte *memory,ALenum *format,ALvoid **data,ALsizei *size,ALsizei *freq,ALboolean *loop), return; )
AL_FUNCTION(ALvoid,  alutUnloadWAV, (ALenum format,ALvoid *data,ALsizei size,ALsizei freq), return; )

