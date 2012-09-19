//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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
//-----------------------------------------------------------------------------

#ifndef _APPSEQUENCE_H_
#define _APPSEQUENCE_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif

class AppSequence
{
public:
   S32 fps;

public:
   AppSequence() { }
   virtual ~AppSequence() { }

   virtual void setActive(bool active) { }

   virtual S32 getNumTriggers() const { return 0; }
   virtual void getTrigger(S32 index, TSShape::Trigger& trigger) const { trigger.state = 0;}

   virtual const char* getName() const { return "ambient"; }

   virtual F32 getStart() const { return 0.0f; }
   virtual F32 getEnd() const { return 0.0f; }

   virtual U32 getFlags() const { return 0; }
   virtual F32 getPriority() const { return 5; }
   virtual F32 getBlendRefTime() const { return 0.0f; }
};

#endif // _APPSEQUENCE_H_
