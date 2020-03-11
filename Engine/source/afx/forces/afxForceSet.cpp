
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

#include "afx/arcaneFX.h"

#include "afx/forces/afxForceSet.h"
#include "afx/forces/afxForce.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

afxForceSet::afxForceSet(const char* name)
{
  mName = (name) ? StringTable->insert(name) : ST_NULLSTRING;
  mUpdate_dt = 10.0f; // seems like an ok maximum, force-xmods will probably lower it.
  mElapsed_dt = 0.0f;
  mElapsed_ms = 0;
  mNum_updates = 0;
  mLast_num_updates = 0;
}

void afxForceSet::remove(afxForce* force)
{
  for (S32 i = 0; i < mForce_v.size(); i++)
  {
    if (mForce_v[i] == force)
    {
      mForce_v.erase(i);
      return;
    }
  }
}

S32 afxForceSet::updateDT(F32 dt)
{
  U32 now = Platform::getVirtualMilliseconds();

  if (mElapsed_ms == now) 
    return mLast_num_updates;

  mElapsed_ms = now;
  mElapsed_dt += dt;

  if (mElapsed_dt < mUpdate_dt) 
  {
    mLast_num_updates = 0;
    return 0;
  }

  mNum_updates = mFloor(mElapsed_dt/mUpdate_dt);
  mElapsed_dt -= mUpdate_dt*mNum_updates;
  mLast_num_updates = mNum_updates;

  return mNum_updates;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

afxForceSetMgr::afxForceSetMgr()
{
}

afxForceSetMgr::~afxForceSetMgr()
{
  for (S32 i = 0; i < forces_sets.size(); i++)
  {
    if (forces_sets[i])
      delete forces_sets[i];
  }
}

afxForceSet* afxForceSetMgr::findForceSet(StringTableEntry forces_set_name)
{
  for (S32 i = 0; i < forces_sets.size(); i++)
  {
    if (forces_sets[i] && forces_sets[i]->getName() == forces_set_name)
      return forces_sets[i];
  }

  return 0;
}

void afxForceSetMgr::registerForce(StringTableEntry forces_set_name, afxForce* force)
{
  if (!force)
    return;

  // find forceSet by name
  afxForceSet* fset = findForceSet(forces_set_name);

  // create forceSet if it does not already exist
  if (!fset)
  {
    fset = new afxForceSet(forces_set_name);
    forces_sets.push_back(fset);
  }

  // add force to set
  fset->add(force);
}

void afxForceSetMgr::unregisterForce(StringTableEntry forces_set_name, afxForce* force)
{
  if (!force)
    return;

  afxForceSet* fset = findForceSet(forces_set_name);
  if (!fset) 
    return;

  fset->remove(force);
}

