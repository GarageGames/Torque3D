#include "config.h"

#include <stdlib.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "alMain.h"
#include "alAuxEffectSlot.h"
#include "alError.h"


typedef struct ALnullState {
    DERIVE_FROM_TYPE(ALeffectState);
} ALnullState;

/* Forward-declare "virtual" functions to define the vtable with. */
static ALvoid ALnullState_Destruct(ALnullState *state);
static ALboolean ALnullState_deviceUpdate(ALnullState *state, ALCdevice *device);
static ALvoid ALnullState_update(ALnullState *state, const ALCdevice *device, const ALeffectslot *slot, const ALeffectProps *props);
static ALvoid ALnullState_process(ALnullState *state, ALuint samplesToDo, const ALfloatBUFFERSIZE*restrict samplesIn, ALfloatBUFFERSIZE*restrict samplesOut, ALuint NumChannels);
static void *ALnullState_New(size_t size);
static void ALnullState_Delete(void *ptr);

/* Define the ALeffectState vtable for this type. */
DEFINE_ALEFFECTSTATE_VTABLE(ALnullState);


/* This constructs the effect state. It's called when the object is first
 * created. Make sure to call the parent Construct function first, and set the
 * vtable!
 */
static void ALnullState_Construct(ALnullState *state)
{
    ALeffectState_Construct(STATIC_CAST(ALeffectState, state));
    SET_VTABLE2(ALnullState, ALeffectState, state);
}

/* This destructs (not free!) the effect state. It's called only when the
 * effect slot is no longer used. Make sure to call the parent Destruct
 * function before returning!
 */
static ALvoid ALnullState_Destruct(ALnullState *state)
{
    ALeffectState_Destruct(STATIC_CAST(ALeffectState,state));
}

/* This updates the device-dependant effect state. This is called on
 * initialization and any time the device parameters (eg. playback frequency,
 * format) have been changed.
 */
static ALboolean ALnullState_deviceUpdate(ALnullState* UNUSED(state), ALCdevice* UNUSED(device))
{
    return AL_TRUE;
}

/* This updates the effect state. This is called any time the effect is
 * (re)loaded into a slot.
 */
static ALvoid ALnullState_update(ALnullState* UNUSED(state), const ALCdevice* UNUSED(device), const ALeffectslot* UNUSED(slot), const ALeffectProps* UNUSED(props))
{
}

/* This processes the effect state, for the given number of samples from the
 * input to the output buffer. The result should be added to the output buffer,
 * not replace it.
 */
static ALvoid ALnullState_process(ALnullState* UNUSED(state), ALuint UNUSED(samplesToDo), const ALfloatBUFFERSIZE*restrict UNUSED(samplesIn), ALfloatBUFFERSIZE*restrict UNUSED(samplesOut), ALuint UNUSED(NumChannels))
{
}

/* This allocates memory to store the object, before it gets constructed.
 * DECLARE_DEFAULT_ALLOCATORS can be used to declare a default method.
 */
static void *ALnullState_New(size_t size)
{
    return al_malloc(16, size);
}

/* This frees the memory used by the object, after it has been destructed.
 * DECLARE_DEFAULT_ALLOCATORS can be used to declare a default method.
 */
static void ALnullState_Delete(void *ptr)
{
    al_free(ptr);
}


typedef struct ALnullStateFactory {
    DERIVE_FROM_TYPE(ALeffectStateFactory);
} ALnullStateFactory;

/* Creates ALeffectState objects of the appropriate type. */
ALeffectState *ALnullStateFactory_create(ALnullStateFactory *UNUSED(factory))
{
    ALnullState *state;

    NEW_OBJ0(state, ALnullState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

/* Define the ALeffectStateFactory vtable for this type. */
DEFINE_ALEFFECTSTATEFACTORY_VTABLE(ALnullStateFactory);

ALeffectStateFactory *ALnullStateFactory_getFactory(void)
{
    static ALnullStateFactory NullFactory = { { GET_VTABLE2(ALnullStateFactory, ALeffectStateFactory) } };
    return STATIC_CAST(ALeffectStateFactory, &NullFactory);
}


void ALnull_setParami(ALeffect* UNUSED(effect), ALCcontext *context, ALenum param, ALint UNUSED(val))
{
    switch(param)
    {
        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALnull_setParamiv(ALeffect* UNUSED(effect), ALCcontext *context, ALenum param, const ALint* UNUSED(vals))
{
    switch(param)
    {
        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALnull_setParamf(ALeffect* UNUSED(effect), ALCcontext *context, ALenum param, ALfloat UNUSED(val))
{
    switch(param)
    {
        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALnull_setParamfv(ALeffect* UNUSED(effect), ALCcontext *context, ALenum param, const ALfloat* UNUSED(vals))
{
    switch(param)
    {
        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}

void ALnull_getParami(const ALeffect* UNUSED(effect), ALCcontext *context, ALenum param, ALint* UNUSED(val))
{
    switch(param)
    {
        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALnull_getParamiv(const ALeffect* UNUSED(effect), ALCcontext *context, ALenum param, ALint* UNUSED(vals))
{
    switch(param)
    {
        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALnull_getParamf(const ALeffect* UNUSED(effect), ALCcontext *context, ALenum param, ALfloat* UNUSED(val))
{
    switch(param)
    {
        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}
void ALnull_getParamfv(const ALeffect* UNUSED(effect), ALCcontext *context, ALenum param, ALfloat* UNUSED(vals))
{
    switch(param)
    {
        default:
            SET_ERROR_AND_RETURN(context, AL_INVALID_ENUM);
    }
}

DEFINE_ALEFFECT_VTABLE(ALnull);
