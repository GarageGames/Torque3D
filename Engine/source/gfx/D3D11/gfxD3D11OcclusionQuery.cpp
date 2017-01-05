//-----------------------------------------------------------------------------
// Copyright (c) 2015 GarageGames, LLC
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

#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/D3D11/gfxD3D11OcclusionQuery.h"

#include "gui/3d/guiTSControl.h"

#ifdef TORQUE_GATHER_METRICS
// For TickMs define
#include "T3D/gameBase/processList.h"
#endif

GFXD3D11OcclusionQuery::GFXD3D11OcclusionQuery(GFXDevice *device)
 : GFXOcclusionQuery(device), 
   mQuery(NULL)   
{
#ifdef TORQUE_GATHER_METRICS
   mTimer = PlatformTimer::create();
   mTimer->getElapsedMs();

   mTimeSinceEnd = 0;
   mBeginFrame = 0;
#endif
}

GFXD3D11OcclusionQuery::~GFXD3D11OcclusionQuery()
{
   SAFE_RELEASE(mQuery);

#ifdef TORQUE_GATHER_METRICS
   SAFE_DELETE(mTimer);
#endif
}

bool GFXD3D11OcclusionQuery::begin()
{
   if(GFXDevice::getDisableOcclusionQuery())
      return true;

   if (mQuery == NULL)
   {
      D3D11_QUERY_DESC queryDesc;
      queryDesc.Query = D3D11_QUERY_OCCLUSION;
      queryDesc.MiscFlags = 0;

      HRESULT hRes = D3D11DEVICE->CreateQuery(&queryDesc, &mQuery);

      if(FAILED(hRes))
      {
         AssertFatal(false, "GFXD3D11OcclusionQuery::begin - Hardware does not support D3D11 Occlusion-Queries, this should be caught before this type is created");
      }

      AssertISV(hRes != E_OUTOFMEMORY, "GFXD3D11OcclusionQuery::begin - Out of memory");
   }

   // Add a begin marker to the command buffer queue.
   D3D11DEVICECONTEXT->Begin(mQuery);

#ifdef TORQUE_GATHER_METRICS
   mBeginFrame = GuiTSCtrl::getFrameCount();
#endif

   return true;
}

void GFXD3D11OcclusionQuery::end()
{
   if (GFXDevice::getDisableOcclusionQuery())
      return;

   // Add an end marker to the command buffer queue.
   D3D11DEVICECONTEXT->End(mQuery);

#ifdef TORQUE_GATHER_METRICS
   AssertFatal( mBeginFrame == GuiTSCtrl::getFrameCount(), "GFXD3D11OcclusionQuery::end - ended query on different frame than begin!" );   
   mTimer->getElapsedMs();
   mTimer->reset();
#endif
}

GFXD3D11OcclusionQuery::OcclusionQueryStatus GFXD3D11OcclusionQuery::getStatus(bool block, U32 *data)
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
   PROFILE_SCOPE(GFXD3D11OcclusionQuery_getStatus);

   if ( GFXDevice::getDisableOcclusionQuery() )
      return NotOccluded;

   if ( mQuery == NULL )
      return Unset;

#ifdef TORQUE_GATHER_METRICS
   //AssertFatal( mBeginFrame < GuiTSCtrl::getFrameCount(), "GFXD3D11OcclusionQuery::getStatus - called on the same frame as begin!" );

   //U32 mTimeSinceEnd = mTimer->getElapsedMs();
   //AssertFatal( mTimeSinceEnd >= 5, "GFXD3DOcculsionQuery::getStatus - less than TickMs since called ::end!" );
#endif

   HRESULT hRes;
   U64 dwOccluded = 0;

   if ( block )
   {      
      while ((hRes = D3D11DEVICECONTEXT->GetData(mQuery, &dwOccluded, sizeof(U64), 0)) == S_FALSE);
   }
   else
   {
      hRes = D3D11DEVICECONTEXT->GetData(mQuery, &dwOccluded, sizeof(U64), 0);
   }

   if (hRes == S_OK)   
   {
      if (data != NULL)
         *data = (U32)dwOccluded;

      return dwOccluded > 0 ? NotOccluded : Occluded;   
   }

   if (hRes == S_FALSE)
      return Waiting;

   return Error;   
}

void GFXD3D11OcclusionQuery::zombify()
{
   SAFE_RELEASE( mQuery );
}

void GFXD3D11OcclusionQuery::resurrect()
{
	// Recreate the query 
	if( mQuery == NULL ) 
	{ 
      D3D11_QUERY_DESC queryDesc;
      queryDesc.Query = D3D11_QUERY_OCCLUSION;
      queryDesc.MiscFlags = 0;

      HRESULT hRes = D3D11DEVICE->CreateQuery(&queryDesc, &mQuery); 
	
      AssertISV( hRes != E_OUTOFMEMORY, "GFXD3D9QueryFence::resurrect - Out of memory" ); 
   } 
}

const String GFXD3D11OcclusionQuery::describeSelf() const
{
   // We've got nothing
   return String();
}