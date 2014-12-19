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

#include "gfx/D3D9/gfxD3D9Device.h"
#include "gfx/D3D9/gfxD3D9OcclusionQuery.h"

#include "gui/3d/guiTSControl.h"

#ifdef TORQUE_GATHER_METRICS
// For TickMs define
#include "T3D/gameBase/processList.h"
#endif

GFXD3D9OcclusionQuery::GFXD3D9OcclusionQuery( GFXDevice *device )
 : GFXOcclusionQuery( device ), 
   mQuery( NULL )   
{
#ifdef TORQUE_GATHER_METRICS
   mTimer = PlatformTimer::create();
   mTimer->getElapsedMs();

   mTimeSinceEnd = 0;
   mBeginFrame = 0;
#endif
}

GFXD3D9OcclusionQuery::~GFXD3D9OcclusionQuery()
{
   SAFE_RELEASE( mQuery );

#ifdef TORQUE_GATHER_METRICS
   SAFE_DELETE( mTimer );
#endif
}

bool GFXD3D9OcclusionQuery::begin()
{
   if ( GFXDevice::getDisableOcclusionQuery() )
      return true;

   if ( mQuery == NULL )
   {
#ifdef TORQUE_OS_XENON
      HRESULT hRes = static_cast<GFXD3D9Device*>( mDevice )->getDevice()->CreateQueryTiled( D3DQUERYTYPE_OCCLUSION, 2, &mQuery );
#else
      HRESULT hRes = static_cast<GFXD3D9Device*>( mDevice )->getDevice()->CreateQuery( D3DQUERYTYPE_OCCLUSION, &mQuery );
#endif

      AssertFatal( hRes != D3DERR_NOTAVAILABLE, "GFXD3D9OcclusionQuery::begin - Hardware does not support D3D9 Occlusion-Queries, this should be caught before this type is created" );
      AssertISV( hRes != E_OUTOFMEMORY, "GFXD3D9OcclusionQuery::begin - Out of memory" );
   }

   // Add a begin marker to the command buffer queue.
   mQuery->Issue( D3DISSUE_BEGIN );

#ifdef TORQUE_GATHER_METRICS
   mBeginFrame = GuiTSCtrl::getFrameCount();
#endif

   return true;
}

void GFXD3D9OcclusionQuery::end()
{
   if ( GFXDevice::getDisableOcclusionQuery() )
      return;

   // Add an end marker to the command buffer queue.
   mQuery->Issue( D3DISSUE_END );

#ifdef TORQUE_GATHER_METRICS
   AssertFatal( mBeginFrame == GuiTSCtrl::getFrameCount(), "GFXD3D9OcclusionQuery::end - ended query on different frame than begin!" );   
   mTimer->getElapsedMs();
   mTimer->reset();
#endif
}

GFXD3D9OcclusionQuery::OcclusionQueryStatus GFXD3D9OcclusionQuery::getStatus( bool block, U32 *data )
{
   // If this ever shows up near the top of a profile then your system is 
   // GPU bound or you are calling getStatus too soon after submitting it.
   //
   // To test if you are GPU bound resize your window very small and see if
   // this profile no longer appears at the top.
   //
   // To test if you are calling getStatus to soon after submitting it,
   // check the value of mTimeSinceEnd in a debug build. If it is < half the length
   // of time to render an individual frame you could have problems.
   PROFILE_SCOPE(GFXD3D9OcclusionQuery_getStatus);

   if ( GFXDevice::getDisableOcclusionQuery() )
      return NotOccluded;

   if ( mQuery == NULL )
      return Unset;

#ifdef TORQUE_GATHER_METRICS
   //AssertFatal( mBeginFrame < GuiTSCtrl::getFrameCount(), "GFXD3D9OcclusionQuery::getStatus - called on the same frame as begin!" );

   //U32 mTimeSinceEnd = mTimer->getElapsedMs();
   //AssertFatal( mTimeSinceEnd >= 5, "GFXD3DOcculsionQuery::getStatus - less than TickMs since called ::end!" );
#endif

   HRESULT hRes;
   DWORD dwOccluded = 0;

   if ( block )
   {  
      while( ( hRes = mQuery->GetData( &dwOccluded, sizeof(DWORD), D3DGETDATA_FLUSH ) ) == S_FALSE )
      {
          //If we're stalled out, proceed with worst-case scenario -BJR
          if(GFX->mFrameTime->getElapsedMs()>4)
          {
              this->begin();
              this->end();
              return NotOccluded;
          }
      }
   }
   else
   {
      hRes = mQuery->GetData( &dwOccluded, sizeof(DWORD), 0 );
   }

   if ( hRes == S_OK )   
   {
      if ( data != NULL )
         *data = dwOccluded;

      return dwOccluded > 0 ? NotOccluded : Occluded;   
   }

   if ( hRes == S_FALSE )
      return Waiting;

   return Error;   
}

void GFXD3D9OcclusionQuery::zombify()
{
   SAFE_RELEASE( mQuery );
}

void GFXD3D9OcclusionQuery::resurrect()
{
}

const String GFXD3D9OcclusionQuery::describeSelf() const
{
   // We've got nothing
   return String();
}