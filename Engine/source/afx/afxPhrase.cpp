
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

#include "afx/afxEffectVector.h"
#include "afx/afxPhrase.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPhrase

void
afxPhrase::init_fx(S32 group_index)
{
  mFX->ev_init(mInit_chor, *mInit_fx_list, mOn_server, mWill_stop, mInit_time_factor, mInit_dur, group_index); 
}

//~~~~~~~~~~~~~~~~~~~~//

afxPhrase::afxPhrase(bool on_server, bool will_stop)
{
  mOn_server = on_server;
  mWill_stop = will_stop;

  mInit_fx_list = NULL;
  mInit_dur = 0.0f;
  mInit_chor = NULL;
  mInit_time_factor = 1.0f;

  mFX = new afxEffectVector;
  mFX2 = NULL;
  mStartTime = 0;
  mDur = 0;

  mNum_loops = 1;
  mLoop_cnt = 1;

  mExtra_time = 0.0f;
  mExtra_stoptime = 0.0f;
}

afxPhrase::~afxPhrase()
{
  delete mFX;
  delete mFX2;
};

void 
afxPhrase::init(afxEffectList& fx_list, F32 dur, afxChoreographer* chor, F32 time_factor, 
                S32 n_loops, S32 group_index, F32 extra_time)
{
  mInit_fx_list = &fx_list;
  mInit_dur = dur;
  mInit_chor = chor;
  mInit_time_factor = time_factor;

  mNum_loops = n_loops;
  mExtra_time = extra_time;
  mDur = (mInit_dur < 0) ? mInit_dur : mInit_dur*mInit_time_factor;

  init_fx(group_index);
}

void
afxPhrase::start(F32 startstamp, F32 timestamp)
{
  mStartTime = startstamp;

  F32 loopstart = timestamp - startstamp;

  if (mDur > 0 && loopstart > mDur)
  {
    mLoop_cnt += (S32) (loopstart/ mDur);
    loopstart = mFmod(loopstart, mDur);
  }

  if (!mFX->empty())
    mFX->start(loopstart);
}

void
afxPhrase::update(F32 dt, F32 timestamp)
{
  if (mFX->isActive())
    mFX->update(dt);

  if (mFX2 && mFX2->isActive())
    mFX2->update(dt);

  if (mExtra_stoptime > 0 && timestamp > mExtra_stoptime)
  {
    stop(timestamp);
  }
}

void
afxPhrase::stop(F32 timestamp)
{
  if (mExtra_time > 0 && !(mExtra_stoptime > 0))
  {
    mExtra_stoptime = timestamp + mExtra_time;
    return;
  }

  if (mFX->isActive())
    mFX->stop();

  if (mFX2 && mFX2->isActive())
    mFX2->stop();
}

bool
afxPhrase::expired(F32 timestamp)
{
  if (mDur < 0)
    return false;

  return ((timestamp - mStartTime) > mLoop_cnt*mDur);
}

F32
afxPhrase::elapsed(F32 timestamp)
{
  return (timestamp - mStartTime);
}

bool
afxPhrase::recycle(F32 timestamp)
{
  if (mNum_loops < 0 || mLoop_cnt < mNum_loops)
  {
    if (mFX2)
      delete mFX2;

	mFX2 = mFX;

	mFX = new afxEffectVector;
    init_fx();

    if (mFX2 && !mFX2->empty())
      mFX2->stop();

    if (!mFX->empty())
      mFX->start(0.0F);

	mLoop_cnt++;
    return true;
  }

  return false;
}

void
afxPhrase::interrupt(F32 timestamp)
{
  if (mFX->isActive())
    mFX->interrupt();

  if (mFX2 && mFX2->isActive())
    mFX2->interrupt();
}

F32 afxPhrase::calcDoneTime()
{
  return mStartTime + mFX->getTotalDur();
}

F32 afxPhrase::calcAfterLife()
{
  return mFX->getAfterLife();
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//