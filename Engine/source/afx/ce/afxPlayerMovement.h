
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

#ifndef _AFX_PLAYER_MOVEMENT_H_
#define _AFX_PLAYER_MOVEMENT_H_

#include "afx/ce/afxComponentEffect.h"
#include "afx/afxEffectDefs.h"
#include "afx/afxConstraint.h"

class afxPlayerMovementData : public GameBaseData, public afxEffectDefs
{
  typedef GameBaseData  Parent;
public:
  enum OpType
  {
    OP_ADD = 0,
    OP_MULTIPLY,
    OP_REPLACE,
    OP_BITS = 2,
  };

public:
  F32               speed_bias;
  Point3F           movement;
  U32               movement_op;

public:
  /*C*/             afxPlayerMovementData();
  /*C*/             afxPlayerMovementData(const afxPlayerMovementData&, bool = false);

  virtual bool      onAdd();
  virtual void      packData(BitStream*);
  virtual void      unpackData(BitStream*);

  bool              hasMovementOverride();

  virtual bool      allowSubstitutions() const { return true; }

  static void       initPersistFields();

  DECLARE_CONOBJECT(afxPlayerMovementData);
  DECLARE_CATEGORY("AFX");
};

typedef afxPlayerMovementData::OpType afxPlayerMovement_OpType;
DefineEnumType( afxPlayerMovement_OpType );

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_PLAYER_MOVEMENT_H_
