#ifndef _AL_LISTENER_H_
#define _AL_LISTENER_H_

#include "alMain.h"
#include "alu.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ALlistenerProps {
    ATOMIC(ALfloat) Position[3];
    ATOMIC(ALfloat) Velocity[3];
    ATOMIC(ALfloat) Forward[3];
    ATOMIC(ALfloat) Up[3];
    ATOMIC(ALfloat) Gain;
    ATOMIC(ALfloat) MetersPerUnit;

    ATOMIC(ALfloat) DopplerFactor;
    ATOMIC(ALfloat) DopplerVelocity;
    ATOMIC(ALfloat) SpeedOfSound;
    ATOMIC(ALboolean) SourceDistanceModel;
    ATOMIC(enum DistanceModel) DistanceModel;

    ATOMIC(struct ALlistenerProps*) next;
};

typedef struct ALlistener {
    volatile ALfloat Position[3];
    volatile ALfloat Velocity[3];
    volatile ALfloat Forward[3];
    volatile ALfloat Up[3];
    volatile ALfloat Gain;
    volatile ALfloat MetersPerUnit;

    /* Pointer to the most recent property values that are awaiting an update.
     */
    ATOMIC(struct ALlistenerProps*) Update;

    /* A linked list of unused property containers, free to use for future
     * updates.
     */
    ATOMIC(struct ALlistenerProps*) FreeList;

    struct {
        aluMatrixf Matrix;
        aluVector  Velocity;

        ALfloat Gain;
        ALfloat MetersPerUnit;

        ALfloat DopplerFactor;
        ALfloat SpeedOfSound;

        ALboolean SourceDistanceModel;
        enum DistanceModel DistanceModel;
    } Params;
} ALlistener;

void UpdateListenerProps(ALCcontext *context);

#ifdef __cplusplus
}
#endif

#endif
