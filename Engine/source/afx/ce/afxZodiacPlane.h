
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

#ifndef _AFX_ZODIAC_PLANE_H_
#define _AFX_ZODIAC_PLANE_H_

#include "afx/ce/afxZodiacDefs.h"

class afxZodiacPlaneData : public GameBaseData, public afxZodiacDefs
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
  enum FacingType
  {
    FACES_UP = 0,
    FACES_DOWN,
    FACES_FORWARD,
    FACES_BACK,
    FACES_RIGHT,
    FACES_LEFT,
    FACES_BITS = 3
  };

public:
  StringTableEntry  txr_name;
  GFXTexHandle      txr;
  F32               radius_xy;
  F32               start_ang;
  F32               ang_per_sec;
  F32               grow_in_time;
  F32               shrink_out_time;
  F32               growth_rate;
  ColorF            color;
  U32               blend_flags;
  bool              respect_ori_cons;
  bool              use_full_xfm;
  U32               zflags;
  U32               face_dir;

  bool              double_sided;

  void              expand_zflags();
  void              merge_zflags();

public:
  /*C*/         afxZodiacPlaneData();
  /*C*/         afxZodiacPlaneData(const afxZodiacPlaneData&, bool = false);

  virtual void  packData(BitStream*);
  virtual void  unpackData(BitStream*);

  bool          preload(bool server, String &errorStr);

  virtual bool  allowSubstitutions() const { return true; }

  F32           calcRotationAngle(F32 elapsed, F32 rate_factor=1.0f);

  static void   initPersistFields();

  DECLARE_CONOBJECT(afxZodiacPlaneData);
  DECLARE_CATEGORY("AFX");
};

typedef afxZodiacPlaneData::BlendType afxZodiacPlane_BlendType;
DefineEnumType( afxZodiacPlane_BlendType );

typedef afxZodiacPlaneData::FacingType afxZodiacPlane_FacingType;
DefineEnumType( afxZodiacPlane_FacingType );

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxZodiacPlane

class afxZodiacPlane : public GameBase, public afxZodiacDefs
{
  typedef GameBase Parent;

private:
  afxZodiacPlaneData*   mDataBlock;
  ColorF                color;
  F32                   radius;
  bool                  is_visible;

  void                  preDraw();
  void                  draw();
  void                  postDraw();

  GFXStateBlockRef      normal_sb;
  GFXStateBlockRef      reflected_sb;

public:
  /*C*/                 afxZodiacPlane();
  /*D*/                 ~afxZodiacPlane();

  virtual bool          onNewDataBlock(GameBaseData* dptr, bool reload);
  virtual bool          onAdd();
  virtual void          onRemove();

  void                  setRadius(F32 rad) { radius = rad; }
  void                  setColor(const ColorF& clr) { color = clr; }
  void                  setVisibility(bool flag) { is_visible = flag; }

  virtual void          prepRenderImage(SceneRenderState*);
                                        
  void                  _renderZodiacPlane(ObjectRenderInst*, SceneRenderState*, BaseMatInstance*);

  DECLARE_CONOBJECT(afxZodiacPlane);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_ZODIAC_PLANE_H_
