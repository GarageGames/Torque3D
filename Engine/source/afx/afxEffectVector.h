
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

#ifndef _AFX_EFFECT_VECTOR_H_
#define _AFX_EFFECT_VECTOR_H_

#include "afx/afxEffectWrapper.h"
#include "afx/afxEffectGroup.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEffectVector

class afxEffectWrapper;
class afxChoreographer;

class afxEffectVector
{
  Vector<afxEffectWrapper*>*  fx_v;
  Vector<afxEffectWrapper*>*  fx_v2;

  bool          active;
  bool          on_server;
  F32           phrase_dur;
  F32           total_fx_dur;
  F32           after_life;

  void          swap_vecs();
  void          filter_client_server();
  void          calc_fx_dur_and_afterlife();

  void          effects_init(afxChoreographer*, afxEffectList&, bool will_stop, F32 time_factor, 
                             S32 group_index, const afxGroupTimingData* group_timing=0);

public:
  /*C*/         afxEffectVector();
  /*D*/         ~afxEffectVector();

  void          ev_init(afxChoreographer*, afxEffectList&, bool on_server, bool will_stop, 
                        F32 time_factor, F32 phrase_dur, S32 group_index=0);

  void          start(F32 timestamp);
  void          update(F32 dt);
  void          stop(bool force_cleanup=false);
  void          interrupt();
  bool          empty() { return (!fx_v || fx_v->empty()); }
  bool          isActive() { return active; }
  S32           count() { return (fx_v) ? fx_v->size() : 0; }

  F32           getTotalDur() { return total_fx_dur; }
  F32           getAfterLife() { return after_life; }

  Vector<afxEffectWrapper*>* getFX() { return fx_v; }
};

inline void afxEffectVector::swap_vecs()
{
  Vector<afxEffectWrapper*>* tmp = fx_v;
  fx_v = fx_v2;
  fx_v2 = tmp;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_EFFECT_VECTOR_H_
