
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

#ifndef _AFX_EFFECT_GROUP_H_
#define _AFX_EFFECT_GROUP_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "console/typeValidators.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"

class afxEffectWrapperData;

struct afxGroupTimingData
{
  F32     delay;
  F32     lifetime;
  F32     fade_in_time;
  F32     fade_out_time;

  afxGroupTimingData()
  {
    delay = 0.0f;
    lifetime = 0.0f;
    fade_in_time = 0.0f;
    fade_out_time = 0.0f;
  }
};

class afxEffectGroupData : public afxEffectBaseData
{
  typedef afxEffectBaseData Parent;

  class egValidator : public TypeValidator
  {
    U32 id;
  public:
    egValidator(U32 id) { this->id = id; }
    void validateType(SimObject *object, void *typePtr);
  };

  bool          do_id_convert;

public:
  afxEffectList       fx_list;
  bool                group_enabled;
  S32                 group_count;
  U8                  idx_offset;
  bool                assign_idx;
  afxGroupTimingData  timing;
  afxEffectBaseData*  dummy_fx_entry;

private:
  void          pack_fx(BitStream* stream, const afxEffectList& fx, bool packed);
  void          unpack_fx(BitStream* stream, afxEffectList& fx);
  
public:
  /*C*/         afxEffectGroupData();
  /*C*/         afxEffectGroupData(const afxEffectGroupData&, bool = false);

  virtual void  reloadReset();

  virtual void  packData(BitStream*);
  virtual void  unpackData(BitStream*);

  bool          preload(bool server, String &errorStr);

  virtual void  gather_cons_defs(Vector<afxConstraintDef>& defs);

  virtual bool  allowSubstitutions() const { return true; }

  static void   initPersistFields();

  DECLARE_CONOBJECT(afxEffectGroupData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
#endif // _AFX_EFFECT_GROUP_H_
