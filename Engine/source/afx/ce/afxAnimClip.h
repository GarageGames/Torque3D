
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

#ifndef _AFX_ANIM_CLIP_H_
#define _AFX_ANIM_CLIP_H_

class afxAnimClipData : public GameBaseData
{
  typedef GameBaseData  Parent;

  enum 
  {
    IGNORE_DISABLED     = BIT(0),
    IGNORE_ENABLED      = BIT(1),
    IS_DEATH_ANIM       = BIT(2),
    BLOCK_USER_CONTROL  = BIT(3),
    IGNORE_FIRST_PERSON = BIT(4),
    IGNORE_THIRD_PERSON = BIT(5)
  };

public:
  StringTableEntry      clip_name;
  F32                   rate;
  F32                   pos_offset;
  F32                   trans;
  U8                    flags;

  bool                  ignore_disabled;
  bool                  ignore_enabled;
  bool                  is_death_anim;
  bool                  lock_anim;
  bool                  ignore_first_person;
  bool                  ignore_third_person;

  void                  expand_flags();
  void                  merge_flags();

public:
  /*C*/                 afxAnimClipData();
  /*C*/                 afxAnimClipData(const afxAnimClipData&, bool = false);

  virtual bool          onAdd();
  virtual void          packData(BitStream*);
  virtual void          unpackData(BitStream*);
  virtual bool          writeField(StringTableEntry fieldname, const char* value);

  virtual void          onStaticModified(const char* slotName, const char* newValue = NULL);

  virtual bool          allowSubstitutions() const { return true; }

  static void           initPersistFields();

  DECLARE_CONOBJECT(afxAnimClipData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_ANIM_CLIP_H_
