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
#include "gfx/D3D9/gfxD3D9QueryFence.h"

GFXD3D9QueryFence::~GFXD3D9QueryFence()
{
   SAFE_RELEASE( mQuery );
}

//------------------------------------------------------------------------------

void GFXD3D9QueryFence::issue()
{
   PROFILE_START( GFXD3D9QueryFence_issue );

   // Create the query if we need to
   if( mQuery == NULL )
   {
      HRESULT hRes = static_cast<GFXD3D9Device *>( mDevice )->getDevice()->CreateQuery( D3DQUERYTYPE_EVENT, &mQuery );

      AssertFatal( hRes != D3DERR_NOTAVAILABLE, "Hardware does not support D3D9 Queries, this should be caught before this fence type is created" );
      AssertISV( hRes != E_OUTOFMEMORY, "Out of memory" );
   }

   // Issue the query
   mQuery->Issue( D3DISSUE_END );

   PROFILE_END();
}

//------------------------------------------------------------------------------

GFXFence::FenceStatus GFXD3D9QueryFence::getStatus() const
{
   if( mQuery == NULL )
      return GFXFence::Unset;

   HRESULT hRes = mQuery->GetData( NULL, 0, 0 );

   return ( hRes == S_OK ? GFXFence::Processed : GFXFence::Pending );
}

//------------------------------------------------------------------------------

void GFXD3D9QueryFence::block()
{
   PROFILE_SCOPE(GFXD3D9QueryFence_block);

   // Calling block() before issue() is valid, catch this case
   if( mQuery == NULL )
      return;

   HRESULT hRes;
   while( ( hRes = mQuery->GetData( NULL, 0, D3DGETDATA_FLUSH ) ) == S_FALSE )
      ;

   // Check for D3DERR_DEVICELOST, if we lost the device, the fence will get 
   // re-created next issue()
   if( hRes == D3DERR_DEVICELOST )
      SAFE_RELEASE( mQuery );
}

void GFXD3D9QueryFence::zombify()
{
   // Release our query
   SAFE_RELEASE( mQuery );
}

void GFXD3D9QueryFence::resurrect()
{
   // Recreate the query
   if( mQuery == NULL )
   {
      HRESULT hRes = static_cast<GFXD3D9Device *>( mDevice )->getDevice()->CreateQuery( D3DQUERYTYPE_EVENT, &mQuery );

      AssertFatal( hRes != D3DERR_NOTAVAILABLE, "GFXD3D9QueryFence::resurrect - Hardware does not support D3D9 Queries, this should be caught before this fence type is created" );
      AssertISV( hRes != E_OUTOFMEMORY, "GFXD3D9QueryFence::resurrect - Out of memory" );
   }
}

const String GFXD3D9QueryFence::describeSelf() const
{
   // We've got nothing
   return String();
}