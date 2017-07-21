
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#ifndef _AFX_EFFECT_DEFS_H_
#define _AFX_EFFECT_DEFS_H_

#include "afx/arcaneFX.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectBASE

class afxEffectDefs
{
public:

  enum
  {
    MAX_EFFECTS_PER_PHRASE  = 1023,
    EFFECTS_PER_PHRASE_BITS = 10
  };

  // effect networking
  enum
  {
    SERVER_ONLY       = BIT(0),
    SCOPE_ALWAYS      = BIT(1),
    GHOSTABLE         = BIT(2),
    CLIENT_ONLY       = BIT(3),
    SERVER_AND_CLIENT = BIT(4)
  };
  
  // effect condititons
  enum 
  {
    DISABLED = BIT(0),
    ENABLED = BIT(1),
    FAILING = BIT(2),
    ALIVE = ENABLED,
    DEAD = DISABLED,
    DYING = FAILING,
    //
    IMPACTED_SOMETHING  = BIT(31),
    IMPACTED_TARGET     = BIT(30),
    IMPACTED_PRIMARY    = BIT(29),
    IMPACT_IN_WATER     = BIT(28),
    CASTER_IN_WATER     = BIT(27),
  };

  enum
  {
    REQUIRES_STOP     = BIT(0),
    RUNS_ON_SERVER    = BIT(1),
    RUNS_ON_CLIENT    = BIT(2),
  };

  enum 
  {
    MAX_XFM_MODIFIERS = 32,
    INFINITE_LIFETIME = (24*60*60)
  };

  enum
  {
    POINT_CONSTRAINT,
    TRANSFORM_CONSTRAINT,
    OBJECT_CONSTRAINT,
    CAMERA_CONSTRAINT,
    OBJECT_CONSTRAINT_SANS_OBJ,
    OBJECT_CONSTRAINT_SANS_SHAPE,
    UNDEFINED_CONSTRAINT_TYPE
  };

  enum
  {
    DIRECT_DAMAGE,
    DAMAGE_OVER_TIME,
    AREA_DAMAGE
  };

  enum
  {
    TIMING_DELAY      = BIT(0), 
    TIMING_LIFETIME   = BIT(1),
    TIMING_FADE_IN    = BIT(2),
    TIMING_FADE_OUT   = BIT(3),
    TIMING_BITS       = 2
  };
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_EFFECT_DEFS_H_
