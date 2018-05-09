
#include "config.h"

#include "splitter.h"

#include "math_defs.h"


void bandsplit_init(BandSplitter *splitter, ALfloat f0norm)
{
    ALfloat w = f0norm * F_TAU;
    ALfloat cw = cosf(w);
    if(cw > FLT_EPSILON)
        splitter->coeff = (sinf(w) - 1.0f) / cw;
    else
        splitter->coeff = cw * -0.5f;

    splitter->lp_z1 = 0.0f;
    splitter->lp_z2 = 0.0f;
    splitter->hp_z1 = 0.0f;
}

void bandsplit_clear(BandSplitter *splitter)
{
    splitter->lp_z1 = 0.0f;
    splitter->lp_z2 = 0.0f;
    splitter->hp_z1 = 0.0f;
}

void bandsplit_process(BandSplitter *splitter, ALfloat *restrict hpout, ALfloat *restrict lpout,
                       const ALfloat *input, ALsizei count)
{
    ALfloat lp_coeff, hp_coeff, lp_y, hp_y, d;
    ALfloat lp_z1, lp_z2, hp_z1;
    ALsizei i;

    ASSUME(count > 0);

    hp_coeff = splitter->coeff;
    lp_coeff = splitter->coeff*0.5f + 0.5f;
    lp_z1 = splitter->lp_z1;
    lp_z2 = splitter->lp_z2;
    hp_z1 = splitter->hp_z1;
    for(i = 0;i < count;i++)
    {
        ALfloat in = input[i];

        /* Low-pass sample processing. */
        d = (in - lp_z1) * lp_coeff;
        lp_y = lp_z1 + d;
        lp_z1 = lp_y + d;

        d = (lp_y - lp_z2) * lp_coeff;
        lp_y = lp_z2 + d;
        lp_z2 = lp_y + d;

        lpout[i] = lp_y;

        /* All-pass sample processing. */
        hp_y = in*hp_coeff + hp_z1;
        hp_z1 = in - hp_y*hp_coeff;

        /* High-pass generated from removing low-passed output. */
        hpout[i] = hp_y - lp_y;
    }
    splitter->lp_z1 = lp_z1;
    splitter->lp_z2 = lp_z2;
    splitter->hp_z1 = hp_z1;
}


void splitterap_init(SplitterAllpass *splitter, ALfloat f0norm)
{
    ALfloat w = f0norm * F_TAU;
    ALfloat cw = cosf(w);
    if(cw > FLT_EPSILON)
        splitter->coeff = (sinf(w) - 1.0f) / cw;
    else
        splitter->coeff = cw * -0.5f;

    splitter->z1 = 0.0f;
}

void splitterap_clear(SplitterAllpass *splitter)
{
    splitter->z1 = 0.0f;
}

void splitterap_process(SplitterAllpass *splitter, ALfloat *restrict samples, ALsizei count)
{
    ALfloat coeff, in, out;
    ALfloat z1;
    ALsizei i;

    ASSUME(count > 0);

    coeff = splitter->coeff;
    z1 = splitter->z1;
    for(i = 0;i < count;i++)
    {
        in = samples[i];

        out = in*coeff + z1;
        z1 = in - out*coeff;

        samples[i] = out;
    }
    splitter->z1 = z1;
}
