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
static ALvoid ALnullState_update(ALnullState *state, const ALCcontext *context, const ALeffectslot *slot, const ALeffectProps *props);
static ALvoid ALnullState_process(ALnullState *state, ALsizei samplesToDo, const ALfloat (*restrict samplesIn)[BUFFERSIZE], ALfloat (*restrict samplesOut)[BUFFERSIZE], ALsizei mumChannels);
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
static ALvoid ALnullState_update(ALnullState* UNUSED(state), const ALCcontext* UNUSED(context), const ALeffectslot* UNUSED(slot), const ALeffectProps* UNUSED(props))
{
}

/* This processes the effect state, for the given number of samples from the
 * input to the output buffer. The result should be added to the output buffer,
 * not replace it.
 */
static ALvoid ALnullState_process(ALnullState* UNUSED(state), ALsizei UNUSED(samplesToDo), const ALfloatBUFFERSIZE*restrict UNUSED(samplesIn), ALfloatBUFFERSIZE*restrict UNUSED(samplesOut), ALsizei UNUSED(numChannels))
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


typedef struct NullStateFactory {
    DERIVE_FROM_TYPE(EffectStateFactory);
} NullStateFactory;

/* Creates ALeffectState objects of the appropriate type. */
ALeffectState *NullStateFactory_create(NullStateFactory *UNUSED(factory))
{
    ALnullState *state;

    NEW_OBJ0(state, ALnullState)();
    if(!state) return NULL;

    return STATIC_CAST(ALeffectState, state);
}

/* Define the EffectStateFactory vtable for this type. */
DEFINE_EFFECTSTATEFACTORY_VTABLE(NullStateFactory);

EffectStateFactory *NullStateFactory_getFactory(void)
{
    static NullStateFactory NullFactory = { { GET_VTABLE2(NullStateFactory, EffectStateFactory) } };
    return STATIC_CAST(EffectStateFactory, &NullFactory);
}


void ALnull_setParami(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint UNUSED(val))
{
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid null effect integer property 0x%04x", param);
    }
}
void ALnull_setParamiv(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, const ALint* UNUSED(vals))
{
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid null effect integer-vector property 0x%04x", param);
    }
}
void ALnull_setParamf(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALfloat UNUSED(val))
{
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid null effect float property 0x%04x", param);
    }
}
void ALnull_setParamfv(ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, const ALfloat* UNUSED(vals))
{
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid null effect float-vector property 0x%04x", param);
    }
}

void ALnull_getParami(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint* UNUSED(val))
{
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid null effect integer property 0x%04x", param);
    }
}
void ALnull_getParamiv(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALint* UNUSED(vals))
{
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid null effect integer-vector property 0x%04x", param);
    }
}
void ALnull_getParamf(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALfloat* UNUSED(val))
{
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid null effect float property 0x%04x", param);
    }
}
void ALnull_getParamfv(const ALeffect *UNUSED(effect), ALCcontext *context, ALenum param, ALfloat* UNUSED(vals))
{
    switch(param)
    {
    default:
        alSetError(context, AL_INVALID_ENUM, "Invalid null effect float-vector property 0x%04x", param);
    }
}

DEFINE_ALEFFECT_VTABLE(ALnull);
