
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

#ifndef _AFX_PATH_H_
#define _AFX_PATH_H_

#include "core/util/tVector.h"

class afxPathData : public GameBaseData
{
  typedef GameBaseData  Parent;

  static StringTableEntry  POINTS_FIELD;
  static StringTableEntry  ROLL_FIELD;
  static StringTableEntry  TIMES_FIELD;

  bool                  resolved;
  bool                  update_points;
  bool                  update_rolls;
  bool                  update_times;

  void                  update_derived_values();

  void                  clear_arrays();
  void                  derive_points_array();
  void                  derive_rolls_array();
  void                  derive_times_array();

  static void           extract_floats_from_string(Vector<F32>& values, const char* points_str);
  static Point3F*       build_points_array(Vector<F32>& values, U32& n_points, bool reverse=false);
  static F32*           build_floats_array(Vector<F32>& values, U32& n_floats, bool reverse=false);

public:  
  U32                   num_points;
  StringTableEntry      points_string;
  Point3F*              points;

  StringTableEntry      roll_string;
  F32*                  rolls;

  StringTableEntry      times_string;
  F32*                  times;

  StringTableEntry      loop_string;
  F32                   delay;              
  F32                   lifetime;           

  U32                   loop_type;
  F32                   mult;
  F32                   time_offset;
  bool                  reverse;
  Point3F               offset;
  bool                  echo;
  bool                  concentric;

public:
  /*C*/                 afxPathData();
  /*C*/                 afxPathData(const afxPathData&, bool = false);
  /*D*/                 ~afxPathData();

  virtual bool          onAdd();
  virtual void          onRemove();
  virtual void          packData(BitStream*);
  virtual void          unpackData(BitStream*);

  bool                  preload(bool server, String &errorStr);

  virtual void          onStaticModified(const char* slotName, const char* newValue = NULL);
  virtual void          onPerformSubstitutions();
  virtual bool          allowSubstitutions() const { return true; }

  static void           initPersistFields();

  DECLARE_CONOBJECT(afxPathData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_PATH_H_
