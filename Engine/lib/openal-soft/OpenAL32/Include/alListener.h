#ifndef _AL_LISTENER_H_
#define _AL_LISTENER_H_

#include "alMain.h"
#include "alu.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ALcontextProps {
    ALfloat DopplerFactor;
    ALfloat DopplerVelocity;
    ALfloat SpeedOfSound;
    ALboolean SourceDistanceModel;
    enum DistanceModel DistanceModel;
    ALfloat MetersPerUnit;

    ATOMIC(struct ALcontextProps*) next;
};

struct ALlistenerProps {
    ALfloat Position[3];
    ALfloat Velocity[3];
    ALfloat Forward[3];
    ALfloat Up[3];
    ALfloat Gain;

    ATOMIC(struct ALlistenerProps*) next;
};

typedef struct ALlistener {
    alignas(16) ALfloat Position[3];
    ALfloat Velocity[3];
    ALfloat Forward[3];
    ALfloat Up[3];
    ALfloat Gain;

    ATOMIC_FLAG PropsClean;

    /* Pointer to the most recent property values that are awaiting an update.
     */
    ATOMIC(struct ALlistenerProps*) Update;

    struct {
        aluMatrixf Matrix;
        aluVector  Velocity;

        ALfloat Gain;
        ALfloat MetersPerUnit;

        ALfloat DopplerFactor;
        ALfloat SpeedOfSound; /* in units per sec! */
        ALfloat ReverbSpeedOfSound; /* in meters per sec! */

        ALboolean SourceDistanceModel;
        enum DistanceModel DistanceModel;
    } Params;
} ALlistener;

void UpdateListenerProps(ALCcontext *context);

#ifdef __cplusplus
}
#endif

#endif
