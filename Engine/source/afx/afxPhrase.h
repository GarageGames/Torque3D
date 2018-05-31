
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

#ifndef _AFX_PHRASE_H_
#define _AFX_PHRASE_H_

#include "afxEffectVector.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPhrase

class afxChoreographer;
class afxConstraintMgr;
class afxEffectVector;

class afxPhrase
{
protected:
  afxEffectList*    mInit_fx_list;
  F32               mInit_dur;
  afxChoreographer* mInit_chor;
  F32               mInit_time_factor;
  F32               mExtra_time;

  afxEffectVector*  mFX;
  afxEffectVector*  mFX2;

  bool              mOn_server;
  bool              mWill_stop;

  F32               mStartTime;
  F32               mDur;
  S32               mNum_loops;
  S32               mLoop_cnt;
  F32               mExtra_stoptime;

  void              init_fx(S32 group_index=0);

public:
  /*C*/             afxPhrase(bool on_server, bool will_stop);
  virtual           ~afxPhrase();

  virtual void      init(afxEffectList&, F32 dur, afxChoreographer*, F32 time_factor, 
                         S32 n_loops, S32 group_index=0, F32 extra_time=0.0f);

  virtual void      start(F32 startstamp, F32 timestamp);
  virtual void      update(F32 dt, F32 timestamp);
  virtual void      stop(F32 timestamp);
  virtual void      interrupt(F32 timestamp);
  virtual bool      expired(F32 timestamp);
  virtual bool      recycle(F32 timestamp);
  virtual F32       elapsed(F32 timestamp);

  bool              isEmpty() { return mFX->empty(); }
  bool              isInfinite() { return (mInit_dur < 0); }
  F32               calcDoneTime();
  F32               calcAfterLife();
  bool              willStop() { return mWill_stop; }
  bool              onServer() { return mOn_server; }
  S32               count() { return mFX->count(); }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_PHRASE_H_
