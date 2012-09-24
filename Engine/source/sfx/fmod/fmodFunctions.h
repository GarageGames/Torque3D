//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

// Xcode has problems properly detecting the dependencies for this file.
// If you change something here, manually recompile all the FMOD modules.


// FMOD Ex API:

FMOD_FUNCTION( FMOD_Channel_GetPaused, (FMOD_CHANNEL *channel, FMOD_BOOL *paused))
FMOD_FUNCTION( FMOD_Channel_SetPaused, (FMOD_CHANNEL *channel, FMOD_BOOL paused));
FMOD_FUNCTION( FMOD_Channel_IsPlaying, (FMOD_CHANNEL *channel, FMOD_BOOL *isplaying))
FMOD_FUNCTION( FMOD_Channel_Set3DAttributes, (FMOD_CHANNEL *channel, const FMOD_VECTOR *pos, const FMOD_VECTOR *vel))
FMOD_FUNCTION( FMOD_Channel_SetFrequency, (FMOD_CHANNEL *channel, float frequency))
FMOD_FUNCTION( FMOD_Channel_SetLoopCount, (FMOD_CHANNEL *channel, int loopcount))
FMOD_FUNCTION( FMOD_Channel_SetPosition, (FMOD_CHANNEL *channel, unsigned int position, FMOD_TIMEUNIT postype))
FMOD_FUNCTION( FMOD_Channel_GetPosition, (FMOD_CHANNEL* channel, unsigned int* position, FMOD_TIMEUNIT postype))
FMOD_FUNCTION( FMOD_Channel_SetVolume, (FMOD_CHANNEL *channel, float volume))
FMOD_FUNCTION( FMOD_Channel_GetVolume, (FMOD_CHANNEL *channel, float *volume))
FMOD_FUNCTION( FMOD_Channel_Stop, (FMOD_CHANNEL *channel))
FMOD_FUNCTION( FMOD_Channel_SetMode, (FMOD_CHANNEL *channel, FMOD_MODE mode))
FMOD_FUNCTION( FMOD_Channel_Set3DMinMaxDistance, (FMOD_CHANNEL *channel, float mindistance, float maxdistance));
FMOD_FUNCTION( FMOD_Channel_Set3DConeSettings, (FMOD_CHANNEL *channel, float insideconeangle, float outsideconeangle, float outsidevolume))
FMOD_FUNCTION( FMOD_Channel_Set3DConeOrientation, (FMOD_CHANNEL *channel, FMOD_VECTOR *orientation))
FMOD_FUNCTION( FMOD_Channel_SetReverbProperties, ( FMOD_CHANNEL* channel, const FMOD_REVERB_CHANNELPROPERTIES* prop ) )
FMOD_FUNCTION( FMOD_Channel_SetPriority, ( FMOD_CHANNEL* channel, int priority ) )
FMOD_FUNCTION( FMOD_Channel_IsVirtual, ( FMOD_CHANNEL* channel, FMOD_BOOL* isvirtual ) )
FMOD_FUNCTION( FMOD_Channel_AddDSP, ( FMOD_CHANNEL* channel, FMOD_DSP* dsp, FMOD_DSPCONNECTION** connection ) )

FMOD_FUNCTION( FMOD_Sound_Lock, (FMOD_SOUND *sound, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2))
FMOD_FUNCTION( FMOD_Sound_Unlock, (FMOD_SOUND *sound, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2))
FMOD_FUNCTION( FMOD_Sound_Release, (FMOD_SOUND *sound))
FMOD_FUNCTION( FMOD_Sound_Set3DMinMaxDistance, (FMOD_SOUND *sound, float min, float max))
FMOD_FUNCTION( FMOD_Sound_SetMode, (FMOD_SOUND *sound, FMOD_MODE mode))
FMOD_FUNCTION( FMOD_Sound_GetMode, (FMOD_SOUND *sound, FMOD_MODE *mode))
FMOD_FUNCTION( FMOD_Sound_SetLoopCount, (FMOD_SOUND *sound, int loopcount))
FMOD_FUNCTION( FMOD_Sound_GetFormat, ( FMOD_SOUND* sound, FMOD_SOUND_TYPE* type, FMOD_SOUND_FORMAT* format, int* channels, int* bits ) )
FMOD_FUNCTION( FMOD_Sound_GetLength, ( FMOD_SOUND* sound, unsigned int* length, FMOD_TIMEUNIT lengthtype ) )
FMOD_FUNCTION( FMOD_Sound_GetDefaults, ( FMOD_SOUND* sound, float* frequency, float* volume, float* pan, int* priority ) ) 
FMOD_FUNCTION( FMOD_Sound_GetMemoryInfo, ( FMOD_SOUND* sound, unsigned int memorybits, unsigned int event_memorybits, unsigned int* memoryused, unsigned int* memoryused_array ) );

FMOD_FUNCTION( FMOD_Geometry_AddPolygon, ( FMOD_GEOMETRY* geometry, float directocclusion, float reverbocclusion, FMOD_BOOL doublesided, int numvertices, const FMOD_VECTOR* vertices, int* polygonindex ) )
FMOD_FUNCTION( FMOD_Geometry_SetPosition, ( FMOD_GEOMETRY* geometry, const FMOD_VECTOR* position ) )
FMOD_FUNCTION( FMOD_Geometry_SetRotation, ( FMOD_GEOMETRY* geometry, const FMOD_VECTOR* forward, const FMOD_VECTOR* up ) )
FMOD_FUNCTION( FMOD_Geometry_SetScale, ( FMOD_GEOMETRY* geometry, const FMOD_VECTOR* scale ) )
FMOD_FUNCTION( FMOD_Geometry_Release, ( FMOD_GEOMETRY* geometry ) )

FMOD_FUNCTION( FMOD_DSP_GetInfo, ( FMOD_DSP* dsp, char* name, unsigned int* version, int* channels, int* configwidth, int* configheight ) )
FMOD_FUNCTION( FMOD_DSP_Release, ( FMOD_DSP* dsp ) )
FMOD_FUNCTION( FMOD_DSP_GetParameterInfo, ( FMOD_DSP* dsp, int index, char* name, char* label, char* description, int descriptionlen, float* min, float* max ) )
FMOD_FUNCTION( FMOD_DSP_GetNumParameters, ( FMOD_DSP* dsp, int* numparams ) )
FMOD_FUNCTION( FMOD_DSP_GetParameter, ( FMOD_DSP* dsp, int index, float* value, char* valuestr, int valuestrlen ) )
FMOD_FUNCTION( FMOD_DSP_SetParameter, ( FMOD_DSP* dsp, int index, float value ) )
FMOD_FUNCTION( FMOD_DSP_GetNumInputs, ( FMOD_DSP* dsp, int* numinputs ) )
FMOD_FUNCTION( FMOD_DSP_GetNumOutputs, ( FMOD_DSP* dsp, int* numoutputs ) )

FMOD_FUNCTION( FMOD_System_Close, (FMOD_SYSTEM *system))
FMOD_FUNCTION( FMOD_System_Create, (FMOD_SYSTEM **system))
FMOD_FUNCTION( FMOD_System_Release, (FMOD_SYSTEM *system))
FMOD_FUNCTION( FMOD_System_CreateSound, (FMOD_SYSTEM *system, const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, FMOD_SOUND **sound))
FMOD_FUNCTION( FMOD_System_CreateGeometry, ( FMOD_SYSTEM* system, int maxpolygons, int maxvertices, FMOD_GEOMETRY** geometry ) )
FMOD_FUNCTION( FMOD_System_SetDriver, (FMOD_SYSTEM *system, int driver))
FMOD_FUNCTION( FMOD_System_GetDriverCaps, (FMOD_SYSTEM *system, int id, FMOD_CAPS *caps, int *controlpaneloutputrate, FMOD_SPEAKERMODE *controlpanelspeakermode))
FMOD_FUNCTION( FMOD_System_GetDriverInfo, (FMOD_SYSTEM *system, int id, char *name, int namelen, FMOD_GUID *GUID))
FMOD_FUNCTION( FMOD_System_GetNumDrivers, (FMOD_SYSTEM *system, int *numdrivers))
FMOD_FUNCTION( FMOD_System_GetVersion, (FMOD_SYSTEM *system, unsigned int *version))
FMOD_FUNCTION( FMOD_System_Init, (FMOD_SYSTEM *system, int maxchannels, FMOD_INITFLAGS flags, void *extradriverdata))
FMOD_FUNCTION( FMOD_System_PlaySound, (FMOD_SYSTEM *system, FMOD_CHANNELINDEX channelid, FMOD_SOUND *sound, FMOD_BOOL paused, FMOD_CHANNEL **channel))
FMOD_FUNCTION( FMOD_System_Set3DListenerAttributes, (FMOD_SYSTEM *system, int listener, const FMOD_VECTOR *pos, const FMOD_VECTOR *vel, const FMOD_VECTOR *forward, const FMOD_VECTOR *up))
FMOD_FUNCTION( FMOD_System_Set3DNumListeners, ( FMOD_SYSTEM* system, int numlisteners ) )
FMOD_FUNCTION( FMOD_System_SetGeometrySettings, ( FMOD_SYSTEM* system, float maxworldsize ) )
FMOD_FUNCTION( FMOD_System_Get3DSettings, (FMOD_SYSTEM* system, float* dopplerFactor, float* distanceFactor, float* rolloffFactor))
FMOD_FUNCTION( FMOD_System_Set3DSettings, (FMOD_SYSTEM *system, float dopplerscale, float distancefactor, float rolloffscale))
FMOD_FUNCTION( FMOD_System_SetDSPBufferSize, (FMOD_SYSTEM *system, unsigned int bufferlength, int numbuffers))
FMOD_FUNCTION( FMOD_System_SetSpeakerMode, (FMOD_SYSTEM *system, FMOD_SPEAKERMODE speakermode))
FMOD_FUNCTION( FMOD_System_SetReverbProperties, ( FMOD_SYSTEM* system, const FMOD_REVERB_PROPERTIES* prop ) )
FMOD_FUNCTION( FMOD_System_SetReverbAmbientProperties, ( FMOD_SYSTEM* system, FMOD_REVERB_PROPERTIES* prop ) )
FMOD_FUNCTION( FMOD_System_Update, ( FMOD_SYSTEM *system ) )
FMOD_FUNCTION( FMOD_System_CreateDSPByType, ( FMOD_SYSTEM* system, FMOD_DSP_TYPE type, FMOD_DSP** dsp ) )
FMOD_FUNCTION( FMOD_System_AddDSP, ( FMOD_SYSTEM* system, FMOD_DSP* dsp, FMOD_DSPCONNECTION** connection ) )
FMOD_FUNCTION( FMOD_System_GetMemoryInfo, ( FMOD_SYSTEM* system, unsigned int memorybits, unsigned int event_memorybits, unsigned int* memoryused, unsigned int* memoryused_array ) )
FMOD_FUNCTION( FMOD_System_SetFileSystem, ( FMOD_SYSTEM* system, FMOD_FILE_OPENCALLBACK useropen, FMOD_FILE_CLOSECALLBACK userclose, FMOD_FILE_READCALLBACK userread, FMOD_FILE_SEEKCALLBACK userseek, int blockalign ) )
FMOD_FUNCTION( FMOD_System_SetPluginPath, ( FMOD_SYSTEM* system, const char* path ) )
FMOD_FUNCTION( FMOD_System_GetHardwareChannels, ( FMOD_SYSTEM* system, int* num2d, int* num3d, int* total ) )

FMOD_FUNCTION( FMOD_Memory_GetStats, ( int*, int* ) )
FMOD_FUNCTION( FMOD_Memory_Initialize, ( void *poolmem, int poollen, FMOD_MEMORY_ALLOCCALLBACK useralloc, FMOD_MEMORY_REALLOCCALLBACK userrealloc, FMOD_MEMORY_FREECALLBACK userfree ) )

// FMOD Designer API:

FMOD_EVENT_FUNCTION( FMOD_EventSystem_Create, ( FMOD_EVENTSYSTEM** eventsystem ) )
FMOD_EVENT_FUNCTION( FMOD_EventSystem_GetSystemObject, ( FMOD_EVENTSYSTEM* eventsystem, FMOD_SYSTEM** system ) )
FMOD_EVENT_FUNCTION( FMOD_EventSystem_GetVersion, ( FMOD_EVENTSYSTEM* eventsystem, unsigned int* version ) )
FMOD_EVENT_FUNCTION( FMOD_EventSystem_Init, ( FMOD_EVENTSYSTEM* eventsystem, int maxchannels, FMOD_INITFLAGS flags, void* extradriverdata, FMOD_EVENT_INITFLAGS eventflags ) )
FMOD_EVENT_FUNCTION( FMOD_EventSystem_Release, ( FMOD_EVENTSYSTEM* eventsystem ) )
FMOD_EVENT_FUNCTION( FMOD_EventSystem_Load, ( FMOD_EVENTSYSTEM* eventsystem, const char* name_or_data, FMOD_EVENT_LOADINFO* loadinfo, FMOD_EVENTPROJECT** project ) )
FMOD_EVENT_FUNCTION( FMOD_EventSystem_Update, ( FMOD_EVENTSYSTEM* eventsystem ) )
FMOD_EVENT_FUNCTION( FMOD_EventSystem_GetMemoryInfo, ( FMOD_EVENTSYSTEM* eventsystem, unsigned int memorybits, unsigned int event_memorybits, unsigned int* memoryused, unsigned int* memoryused_array ) )
FMOD_EVENT_FUNCTION( FMOD_EventSystem_SetMediaPath, ( FMOD_EVENTSYSTEM* eventsystem, const char* path ) )

FMOD_EVENT_FUNCTION( FMOD_EventProject_Release, ( FMOD_EVENTPROJECT* eventproject ) )
FMOD_EVENT_FUNCTION( FMOD_EventProject_GetInfo, ( FMOD_EVENTPROJECT* eventproject, FMOD_EVENT_PROJECTINFO* info ) )
FMOD_EVENT_FUNCTION( FMOD_EventProject_GetNumEvents, ( FMOD_EVENTPROJECT* eventproject, int* numevents ) )
FMOD_EVENT_FUNCTION( FMOD_EventProject_GetNumGroups, ( FMOD_EVENTPROJECT* eventproject, int* numgroups ) )
FMOD_EVENT_FUNCTION( FMOD_EventProject_GetGroupByIndex, ( FMOD_EVENTPROJECT* eventproject, int index, FMOD_BOOL cacheevents, FMOD_EVENTGROUP** group ) )
FMOD_EVENT_FUNCTION( FMOD_EventProject_GetGroup, ( FMOD_EVENTPROJECT* eventproject, const char* name, FMOD_BOOL cacheevents, FMOD_EVENTGROUP** group ) )

FMOD_EVENT_FUNCTION( FMOD_EventGroup_GetInfo, ( FMOD_EVENTGROUP* eventgroup, int* index, char** name ) )
FMOD_EVENT_FUNCTION( FMOD_EventGroup_LoadEventData, ( FMOD_EVENTGROUP* eventgroup, FMOD_EVENT_RESOURCE resource, FMOD_EVENT_MODE mode ) )
FMOD_EVENT_FUNCTION( FMOD_EventGroup_FreeEventData, ( FMOD_EVENTGROUP* eventgroup, FMOD_EVENT* event, FMOD_BOOL waituntilready ) )
FMOD_EVENT_FUNCTION( FMOD_EventGroup_GetNumEvents, ( FMOD_EVENTGROUP* eventgroup, int* numevents ) )
FMOD_EVENT_FUNCTION( FMOD_EventGroup_GetNumGroups, ( FMOD_EVENTGROUP* eventgroup, int* numgroups ) )
FMOD_EVENT_FUNCTION( FMOD_EventGroup_GetEventByIndex, ( FMOD_EVENTGROUP* eventgroup, int index, FMOD_EVENT_MODE mode, FMOD_EVENT** event ) )
FMOD_EVENT_FUNCTION( FMOD_EventGroup_GetEvent, ( FMOD_EVENTGROUP* eventgroup, const char* name, FMOD_EVENT_MODE mode, FMOD_EVENT** event ) )
FMOD_EVENT_FUNCTION( FMOD_EventGroup_GetGroupByIndex, ( FMOD_EVENTGROUP* eventgroup, int index, FMOD_BOOL cacheevents, FMOD_EVENTGROUP** group ) )
FMOD_EVENT_FUNCTION( FMOD_EventGroup_GetGroup, ( FMOD_EVENTGROUP* eventgroup, const char* name, FMOD_BOOL cacheevents, FMOD_EVENTGROUP** group ) )

FMOD_EVENT_FUNCTION( FMOD_Event_GetInfo, ( FMOD_EVENT* event, int* index, char** name, FMOD_EVENT_INFO* info ) )
FMOD_EVENT_FUNCTION( FMOD_Event_Release, ( FMOD_EVENT* event, FMOD_BOOL freeeventdata, FMOD_BOOL waituntilready ) )
FMOD_EVENT_FUNCTION( FMOD_Event_Start, ( FMOD_EVENT* event ) )
FMOD_EVENT_FUNCTION( FMOD_Event_Stop, ( FMOD_EVENT* event, FMOD_BOOL immediate ) )
FMOD_EVENT_FUNCTION( FMOD_Event_SetPaused, ( FMOD_EVENT* event, FMOD_BOOL paused ) )
FMOD_EVENT_FUNCTION( FMOD_Event_SetVolume, ( FMOD_EVENT* event, float volume ) )
FMOD_EVENT_FUNCTION( FMOD_Event_SetPitch, ( FMOD_EVENT* event, float pitch, FMOD_EVENT_PITCHUNITS units ) )
FMOD_EVENT_FUNCTION( FMOD_Event_Set3DAttributes, ( FMOD_EVENT* event, const FMOD_VECTOR* position, const FMOD_VECTOR* velocity, const FMOD_VECTOR* orientation ) )
FMOD_EVENT_FUNCTION( FMOD_Event_GetState, ( FMOD_EVENT* event, FMOD_EVENT_STATE* state ) )
FMOD_EVENT_FUNCTION( FMOD_Event_GetNumParameters, ( FMOD_EVENT* event, int* numparameters ) )
FMOD_EVENT_FUNCTION( FMOD_Event_GetParameter, ( FMOD_EVENT* event, const char* name, FMOD_EVENTPARAMETER** parameter ) )
FMOD_EVENT_FUNCTION( FMOD_Event_GetParameterByIndex, ( FMOD_EVENT* event, int index, FMOD_EVENTPARAMETER** parameter ) )
FMOD_EVENT_FUNCTION( FMOD_Event_GetPropertyByIndex, ( FMOD_EVENT* event, int propertyidex, void* value, FMOD_BOOL this_instance ) )
FMOD_EVENT_FUNCTION( FMOD_Event_SetPropertyByIndex, ( FMOD_EVENT* event, int propertyidex, void* value, FMOD_BOOL this_instance ) )
FMOD_EVENT_FUNCTION( FMOD_Event_GetProperty, ( FMOD_EVENT* event, const char* propertyname, void* value, FMOD_BOOL this_instance ) )
FMOD_EVENT_FUNCTION( FMOD_Event_GetPropertyInfo, ( FMOD_EVENT* event, int* propertyindex, char** propertyname, FMOD_EVENTPROPERTY_TYPE* type ) )

FMOD_EVENT_FUNCTION( FMOD_EventParameter_GetInfo, ( FMOD_EVENTPARAMETER* eventparameter, int* index, char** name ) )
FMOD_EVENT_FUNCTION( FMOD_EventParameter_GetValue, ( FMOD_EVENTPARAMETER* eventparameter, float* value ) )
FMOD_EVENT_FUNCTION( FMOD_EventParameter_SetValue, ( FMOD_EVENTPARAMETER* eventparameter, float value ) )
FMOD_EVENT_FUNCTION( FMOD_EventParameter_GetRange, ( FMOD_EVENTPARAMETER* eventparameter, float* rangemin, float* rangemax ) )