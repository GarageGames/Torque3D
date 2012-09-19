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

#include "platform/platform.h"
#include "T3D/fx/cameraFXMgr.h"

#include "platform/profiler.h"
#include "math/mRandom.h"
#include "math/mMatrix.h"

// global cam fx
CameraFXManager gCamFXMgr;


//**************************************************************************
// Camera effect
//**************************************************************************
CameraFX::CameraFX()
{
   mElapsedTime = 0.0;
   mDuration = 1.0;
}

//--------------------------------------------------------------------------
// Update
//--------------------------------------------------------------------------
void CameraFX::update( F32 dt )
{
   mElapsedTime += dt;
}





//**************************************************************************
// Camera shake effect
//**************************************************************************
CameraShake::CameraShake()
{
   mFreq.zero();
   mAmp.zero();
   mStartAmp.zero();
   mTimeOffset.zero();
   mCamFXTrans.identity();
   mFalloff = 10.0;
   remoteControlled = false;
   isAdded = false;
}

bool CameraShake::isExpired()
{
   if ( remoteControlled )
      return false;

   return Parent::isExpired();
}

//--------------------------------------------------------------------------
// Update
//--------------------------------------------------------------------------
void CameraShake::update( F32 dt )
{
   Parent::update( dt );

   if ( !remoteControlled )
      fadeAmplitude();
   else
      mAmp = mStartAmp;

   VectorF camOffset;
   camOffset.x = mAmp.x * sin( M_2PI * (mTimeOffset.x + mElapsedTime) * mFreq.x );
   camOffset.y = mAmp.y * sin( M_2PI * (mTimeOffset.y + mElapsedTime) * mFreq.y );
   camOffset.z = mAmp.z * sin( M_2PI * (mTimeOffset.z + mElapsedTime) * mFreq.z );

   VectorF rotAngles;
   rotAngles.x = camOffset.x * 10.0 * M_PI/180.0;
   rotAngles.y = camOffset.y * 10.0 * M_PI/180.0;
   rotAngles.z = camOffset.z * 10.0 * M_PI/180.0;
   MatrixF rotMatrix( EulerF( rotAngles.x, rotAngles.y, rotAngles.z ) );

   mCamFXTrans = rotMatrix;
   mCamFXTrans.setPosition( camOffset );
}

//--------------------------------------------------------------------------
// Fade out the amplitude over time
//--------------------------------------------------------------------------
void CameraShake::fadeAmplitude()
{
   F32 percentDone = (mElapsedTime / mDuration);
   if( percentDone > 1.0 ) percentDone = 1.0;

   F32 time = 1 + percentDone * mFalloff;
   time = 1 / (time * time);

   mAmp = mStartAmp * time;
}

//--------------------------------------------------------------------------
// Initialize
//--------------------------------------------------------------------------
void CameraShake::init()
{
   mTimeOffset.x = 0.0;
   mTimeOffset.y = gRandGen.randF();
   mTimeOffset.z = gRandGen.randF();
}

//**************************************************************************
// CameraFXManager
//**************************************************************************
CameraFXManager::CameraFXManager()
{
   mCamFXTrans.identity();
}

//--------------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------------
CameraFXManager::~CameraFXManager()
{
   clear();
}

//--------------------------------------------------------------------------
// Add new effect to currently running list
//--------------------------------------------------------------------------
void CameraFXManager::addFX( CameraFX *newFX )
{
   mFXList.pushFront( newFX );
}

void CameraFXManager::removeFX( CameraFX *fx )
{
   CamFXList::Iterator itr = mFXList.begin();
   for ( ; itr != mFXList.end(); itr++ )
   {
      if ( *itr == fx )
      {
         mFXList.erase( itr );
         return;
      }
   }
   
   return;
}

//--------------------------------------------------------------------------
// Clear all currently running camera effects
//--------------------------------------------------------------------------
void CameraFXManager::clear()
{
   for(CamFXList::Iterator i = mFXList.begin(); i != mFXList.end(); ++i)
   {
      delete *i;
   }

   mFXList.clear();
}

//--------------------------------------------------------------------------
// Update camera effects
//--------------------------------------------------------------------------
void CameraFXManager::update( F32 dt )
{
   PROFILE_SCOPE( CameraFXManager_update );

   mCamFXTrans.identity();

   CamFXList::Iterator cur;
   for(CamFXList::Iterator i = mFXList.begin(); i != mFXList.end(); /*Trickiness*/)
   {
      // Store previous iterator and increment while iterator is still valid.
      cur = i;
      ++i;
      CameraFX * curFX = *cur;
      curFX->update( dt );
      MatrixF fxTrans = curFX->getTrans();

      mCamFXTrans.mul( fxTrans );

      if( curFX->isExpired() )
      {
         delete curFX;
         mFXList.erase( cur );
      }
   }
}
