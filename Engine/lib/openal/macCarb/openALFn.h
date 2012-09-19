#ifndef AL_FUNCTION
#define AL_FUNCTION(fn_name)
#endif

// AL Functions
AL_FUNCTION( alEnable );                 
AL_FUNCTION( alDisable );                
AL_FUNCTION( alIsEnabled );              
AL_FUNCTION( alHint );                   
AL_FUNCTION( alGetBoolean );             
AL_FUNCTION( alGetInteger );             
AL_FUNCTION( alGetFloat );               
AL_FUNCTION( alGetDouble );              
AL_FUNCTION( alGetBooleanv );            
AL_FUNCTION( alGetIntegerv );            
AL_FUNCTION( alGetFloatv );              
AL_FUNCTION( alGetDoublev );             
AL_FUNCTION( alGetString );              
AL_FUNCTION( alGetError );               
AL_FUNCTION( alIsExtensionPresent );     
AL_FUNCTION( alGetProcAddress );         
AL_FUNCTION( alGetEnumValue );           
AL_FUNCTION( alListeneri );              
AL_FUNCTION( alListenerf );              
AL_FUNCTION( alListener3f );             
AL_FUNCTION( alListenerfv );             
AL_FUNCTION( alGetListeneri );           
AL_FUNCTION( alGetListenerf );           
AL_FUNCTION( alGetListener3f );          
AL_FUNCTION( alGetListenerfv );          
AL_FUNCTION( alGenSources );             
AL_FUNCTION( alDeleteSources );          
AL_FUNCTION( alIsSource );               
AL_FUNCTION( alSourcei );                
AL_FUNCTION( alSourcef );                
AL_FUNCTION( alSource3f );               
AL_FUNCTION( alSourcefv );               
AL_FUNCTION( alGetSourcei );             
AL_FUNCTION( alGetSourcef );             
AL_FUNCTION( alGetSource3f );            
AL_FUNCTION( alGetSourcefv );            
AL_FUNCTION( alSourcePlayv );            
AL_FUNCTION( alSourcePausev );           
AL_FUNCTION( alSourceStopv );            
AL_FUNCTION( alSourceRewindv );          
AL_FUNCTION( alSourcePlay );             
AL_FUNCTION( alSourcePause );            
AL_FUNCTION( alSourceStop );             
AL_FUNCTION( alSourceRewind );           
AL_FUNCTION( alGenBuffers );             
AL_FUNCTION( alDeleteBuffers );          
AL_FUNCTION( alIsBuffer );               
AL_FUNCTION( alBufferData );             
AL_FUNCTION( alGetBufferi );             
AL_FUNCTION( alGetBufferf );             
AL_FUNCTION( alSourceQueueBuffers );     
AL_FUNCTION( alSourceUnqueueBuffers );   
AL_FUNCTION( alDistanceModel );          
AL_FUNCTION( alDopplerFactor );          
AL_FUNCTION( alDopplerVelocity );        

// ALC Functions
AL_FUNCTION( alcGetString );             
AL_FUNCTION( alcGetIntegerv );           
AL_FUNCTION( alcOpenDevice );            
AL_FUNCTION( alcCloseDevice );           
AL_FUNCTION( alcCreateContext );         
AL_FUNCTION( alcMakeContextCurrent );    
AL_FUNCTION( alcProcessContext );        
AL_FUNCTION( alcGetCurrentContext );     
AL_FUNCTION( alcGetContextsDevice );     
AL_FUNCTION( alcSuspendContext );        
AL_FUNCTION( alcDestroyContext );        
AL_FUNCTION( alcGetError );              
AL_FUNCTION( alcIsExtensionPresent );    
AL_FUNCTION( alcGetProcAddress );        
AL_FUNCTION( alcGetEnumValue );          


#undef AL_FUNCTION
