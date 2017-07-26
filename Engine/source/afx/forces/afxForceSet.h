
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

#ifndef _AFX_FORCE_SET_H_
#define _AFX_FORCE_SET_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxForce;

class afxForceSet
{
  Vector<afxForce*> force_v;
  StringTableEntry  name;

  // tick-based updating
  F32           update_dt;  // constant update interval, in seconds
  F32           elapsed_dt; // runtime elapsed delta, in seconds
  U32           elapsed_ms;
  S32           num_updates;
  S32           last_num_updates;

public:
  /*C*/         afxForceSet(const char* name=0);

  void          add(afxForce* force) { force_v.push_back(force); }
  void          remove(afxForce* force);

  S32           count() { return force_v.size(); }
  afxForce*     getForce(S32 idx) { return force_v[idx]; }
  const char*   getName() const { return name; }

  void          setUpdateDT(F32 update_dt) { this->update_dt = update_dt; }
  F32           getUpdateDT() { return update_dt; }

  S32           updateDT(F32 dt);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxForceSetMgr
{
  Vector<afxForceSet*>  forces_sets;

public:
  /*C*/         afxForceSetMgr();
  /*D*/         ~afxForceSetMgr();

  afxForceSet*  findForceSet(StringTableEntry name);
  S32           numForceSets() const { return forces_sets.size(); }

  void          registerForce(StringTableEntry forces_set_name, afxForce* force);
  void          unregisterForce(StringTableEntry forces_set_name, afxForce* force);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_FORCE_SET_H_
