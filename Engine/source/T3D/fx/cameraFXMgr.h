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

#ifndef _CAMERAFXMGR_H_
#define _CAMERAFXMGR_H_

#ifndef _TORQUE_LIST_
#include "core/util/tList.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif

//**************************************************************************
// Abstract camera effect template
//**************************************************************************
class CameraFX
{
protected:
   MatrixF  mCamFXTrans;
   F32      mElapsedTime;
   F32      mDuration;

public:
   CameraFX();

   MatrixF &   getTrans(){ return mCamFXTrans; }
   virtual bool isExpired(){ return mElapsedTime >= mDuration; }
   void        setDuration( F32 duration ){ mDuration = duration; }

   virtual void update( F32 dt );
};

//--------------------------------------------------------------------------
// Camera shake effect
//--------------------------------------------------------------------------
class CameraShake : public CameraFX
{
   typedef CameraFX Parent;

   VectorF mFreq;  // these are vectors to represent these values in 3D
   VectorF mStartAmp;
   VectorF mAmp;
   VectorF mTimeOffset;
   F32     mFalloff;

public:

   /// Is controlled by someone else, ignore duration and do not delete.
   bool remoteControlled;
   bool isAdded;

   CameraShake();

   void init();
   void fadeAmplitude();
   void setFalloff( F32 falloff ){ mFalloff = falloff; }
   void setFrequency( VectorF &freq ){ mFreq = freq; }
   void setAmplitude( VectorF &amp ){ mStartAmp = amp; }
   bool isExpired();

   virtual void update( F32 dt );
};


//**************************************************************************
// CameraFXManager
//**************************************************************************
class CameraFXManager
{
   typedef CameraFX * CameraFXPtr;

   MatrixF              mCamFXTrans;
   typedef Torque::List<CameraFXPtr> CamFXList;
   CamFXList mFXList;

public:
   void addFX( CameraFX *newFX );
   void removeFX( CameraFX *fx );
   void        clear();
   MatrixF &   getTrans(){ return mCamFXTrans; }
   void        update( F32 dt );

   CameraFXManager();
   ~CameraFXManager();
};

extern CameraFXManager gCamFXMgr;


#endif
