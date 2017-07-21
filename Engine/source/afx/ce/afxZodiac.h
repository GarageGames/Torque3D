
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

#ifndef _AFX_ZODIAC_H_
#define _AFX_ZODIAC_H_

#ifndef _ARCANE_FX_H_
#include "afx/arcaneFX.h"
#endif
#ifndef _AFX_ZODIAC_DEFS_H_
#include "afx/ce/afxZodiacDefs.h"
#endif

#include "console/typeValidators.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxZodiacData

class afxZodiacData : public GameBaseData, public afxZodiacDefs
{
  typedef GameBaseData  Parent;

public:
  enum BlendType
  {
    BLEND_NORMAL      = 0x0,
    BLEND_ADDITIVE    = 0x1,
    BLEND_SUBTRACTIVE = 0x2,
    BLEND_RESERVED    = 0x3,
    BLEND_MASK        = 0x3
  };

  static void convertGradientRangeFromDegrees(Point2F& gradrange, const Point2F& gradrange_deg);

public:
  StringTableEntry  txr_name;
  GFXTexHandle      txr;
  F32               radius_xy;
  Point2F           vert_range;
  F32               start_ang;
  F32               ang_per_sec;
  F32               grow_in_time;
  F32               shrink_out_time;
  F32               growth_rate;
  ColorF            color;
  U32               blend_flags;
  bool              terrain_ok;
  bool              interiors_ok;
  bool              reflected_ok;
  bool              non_reflected_ok;
  bool              respect_ori_cons;
  bool              scale_vert_range;
  bool              interior_h_only;
  bool              interior_v_ignore;
  bool              interior_back_ignore;
  bool              interior_opaque_ignore;
  bool              interior_transp_ignore;
  U32               zflags;

  F32               altitude_max;
  F32               altitude_falloff;
  bool              altitude_shrinks;
  bool              altitude_fades;

  F32               distance_max;
  F32               distance_falloff;

  bool              use_grade_range;
  bool              prefer_dest_grade;
  Point2F           grade_range_user;
  Point2F           grade_range;
  bool              inv_grade_range;

  static StringTableEntry GradientRangeSlot;
  static bool       sPreferDestinationGradients;

  void              expand_zflags();
  void              merge_zflags();

public:
  /*C*/             afxZodiacData();
  /*C*/             afxZodiacData(const afxZodiacData&, bool = false);

  virtual bool      onAdd();
  virtual void      packData(BitStream*);
  virtual void      unpackData(BitStream*);

  bool              preload(bool server, String &errorStr);

  virtual void      onStaticModified(const char* slotName, const char* newValue = NULL);

  virtual void      onPerformSubstitutions();
  virtual bool      allowSubstitutions() const { return true; }

  F32               calcRotationAngle(F32 elapsed, F32 rate_factor=1.0f);

  static void       initPersistFields();

  DECLARE_CONOBJECT(afxZodiacData);
  DECLARE_CATEGORY("AFX");
};

typedef afxZodiacData::BlendType afxZodiac_BlendType;
DefineEnumType( afxZodiac_BlendType );

GFX_DeclareTextureProfile(AFX_GFXZodiacTextureProfile);

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_ZODIAC_H_
