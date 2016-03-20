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
#include "primBuilder.h"
#include "gfxDevice.h"
#include "console/console.h"


//*****************************************************************************
// Primitive Builder
//*****************************************************************************
namespace PrimBuild
{
Vector<GFXVertexPCT> mTempVertBuff;
GFXVertexBufferHandle<GFXVertexPCT> mVertBuff;
GFXPrimitiveType  mType;
U32               mCurVertIndex;
ColorI            mCurColor( 255, 255, 255 );
Point2F           mCurTexCoord;
const ColorI      _colWhite( 255, 255, 255, 255 );

#ifdef TORQUE_DEBUG
U32 mMaxVerts;

#define INIT_VERTEX_SIZE(x) mMaxVerts = x;
#define VERTEX_BOUNDS_CHECK() AssertFatal( mCurVertIndex < mMaxVerts, "PrimBuilder encountered an out of bounds vertex! Break and debug!" );

// This next check shouldn't really be used a lot unless you are tracking down
// a specific bug. -pw
#define VERTEX_SIZE_CHECK() AssertFatal( mCurVertIndex <= mMaxVerts, "PrimBuilder allocated more verts than you used! Break and debug or rendering artifacts could occur." );

#else

#define INIT_VERTEX_SIZE(x)
#define VERTEX_BOUNDS_CHECK()
#define VERTEX_SIZE_CHECK()

#endif

//-----------------------------------------------------------------------------
// begin
//-----------------------------------------------------------------------------
void begin( GFXPrimitiveType type, U32 maxVerts )
{
   AssertFatal( type >= GFXPT_FIRST && type < GFXPT_COUNT, "PrimBuilder::end() - Bad primitive type!" );

   mType = type;
   mCurVertIndex = 0;
   INIT_VERTEX_SIZE( maxVerts );
   mTempVertBuff.setSize( maxVerts );
}

void beginToBuffer( GFXPrimitiveType type, U32 maxVerts )
{
   AssertFatal( type >= GFXPT_FIRST && type < GFXPT_COUNT, "PrimBuilder::end() - Bad primitive type!" );

   mType = type;
   mCurVertIndex = 0;
   INIT_VERTEX_SIZE( maxVerts );
   mTempVertBuff.setSize( maxVerts );
}

//-----------------------------------------------------------------------------
// end
//-----------------------------------------------------------------------------
GFXVertexBuffer * endToBuffer( U32 &numPrims )
{
   mVertBuff.set(GFX, mTempVertBuff.size(), GFXBufferTypeVolatile);
   GFXVertexPCT *verts = mVertBuff.lock();
   dMemcpy( verts, mTempVertBuff.address(), mTempVertBuff.size() * sizeof(GFXVertexPCT) );
   mVertBuff.unlock();

   VERTEX_SIZE_CHECK();

   switch( mType )
   {
      case GFXPointList:
      {
         numPrims = mCurVertIndex;
         break;
      }

      case GFXLineList:
      {
         numPrims = mCurVertIndex / 2;
         break;
      }

      case GFXLineStrip:
      {
         numPrims = mCurVertIndex - 1;
         break;
      }

      case GFXTriangleList:
      {
         numPrims = mCurVertIndex / 3;
         break;
      }

      case GFXTriangleStrip:
      {
         numPrims = mCurVertIndex - 2;
         break;
      }
      case GFXPT_COUNT:
         // handle warning
         break;
   }

   return mVertBuff;
}

void end( bool useGenericShaders )
{
   if ( mCurVertIndex == 0 ) 
      return; 

   VERTEX_SIZE_CHECK();

   U32 vertStride = 1;
   U32 stripStart = 0;

   AssertFatal( mType >= GFXPT_FIRST && mType < GFXPT_COUNT, "PrimBuilder::end() - Bad primitive type!" );
               
   switch( mType )
   {
      default:
      case GFXPointList:
      {
         vertStride = 1;
         break;
      }

      case GFXLineList:
      {
         vertStride = 2;
         break;
      }

      case GFXTriangleList:
      {
         vertStride = 3;
         break;
      }

      case GFXLineStrip:
      {
         stripStart = 1;
         vertStride = 1;
         break;
      }

      case GFXTriangleStrip:
      {
         stripStart = 2;
         vertStride = 1;
         break;
      }
   }

    if ( useGenericShaders )
    {
        GFXStateBlock *currentBlock = GFX->getStateBlock();
        if (currentBlock && currentBlock->getDesc().samplersDefined)
        {
            if (currentBlock->getDesc().vertexColorEnable)
                GFX->setupGenericShaders( GFXDevice::GSModColorTexture );
            else
                GFX->setupGenericShaders( GFXDevice::GSTexture );
        }
        else
            GFX->setupGenericShaders( GFXDevice::GSColor );
    }

   const GFXVertexPCT *srcVerts = mTempVertBuff.address();
   U32 numVerts = mCurVertIndex;
   
   // Make sure we don't have a dirty prim buffer left.
   GFX->setPrimitiveBuffer( NULL );

   if ( stripStart > 0 )
   {
      // TODO: Fix this to allow > MAX_DYNAMIC_VERTS!

      U32 copyVerts = getMin( (U32)MAX_DYNAMIC_VERTS, numVerts );
      mVertBuff.set( GFX, copyVerts, GFXBufferTypeVolatile );

      GFXVertexPCT *verts = mVertBuff.lock();
      dMemcpy( verts, srcVerts, copyVerts * sizeof( GFXVertexPCT ) );
      mVertBuff.unlock();

      U32 numPrims = ( copyVerts / vertStride ) - stripStart;
      GFX->setVertexBuffer( mVertBuff );
      GFX->drawPrimitive( mType, 0, numPrims );
   }
   else
   {
      while ( numVerts > 0 )
      {
         U32 copyVerts = getMin( (U32)MAX_DYNAMIC_VERTS, numVerts );
         copyVerts -= copyVerts % vertStride;

         mVertBuff.set( GFX, copyVerts, GFXBufferTypeVolatile );

         GFXVertexPCT *verts = mVertBuff.lock();
         dMemcpy( verts, srcVerts, copyVerts * sizeof( GFXVertexPCT ) );
         mVertBuff.unlock();

         U32 numPrims = copyVerts / vertStride;
         GFX->setVertexBuffer( mVertBuff );
         GFX->drawPrimitive( mType, 0, numPrims );

         srcVerts += copyVerts;
         numVerts -= copyVerts;
      }
   }
}

//-----------------------------------------------------------------------------
// vertex2f
//-----------------------------------------------------------------------------
void vertex2f( F32 x, F32 y )
{
   VERTEX_BOUNDS_CHECK();
   GFXVertexPCT *vert = &mTempVertBuff[mCurVertIndex++];

   vert->point.x = x;
   vert->point.y = y;
   vert->point.z = 0.0;
   vert->color = mCurColor;
   vert->texCoord = mCurTexCoord;
}

//-----------------------------------------------------------------------------
// vertex3f
//-----------------------------------------------------------------------------
void vertex3f( F32 x, F32 y, F32 z )
{
   VERTEX_BOUNDS_CHECK();
   GFXVertexPCT *vert = &mTempVertBuff[mCurVertIndex++];

   vert->point.x = x;
   vert->point.y = y;
   vert->point.z = z;
   vert->color = mCurColor;
   vert->texCoord = mCurTexCoord;
}

//-----------------------------------------------------------------------------
// vertex3fv
//-----------------------------------------------------------------------------
void vertex3fv( const F32 *data )
{
   VERTEX_BOUNDS_CHECK();
   GFXVertexPCT *vert = &mTempVertBuff[mCurVertIndex++];

   vert->point.set( data[0], data[1], data[2] );
   vert->color = mCurColor;
   vert->texCoord = mCurTexCoord;
}

//-----------------------------------------------------------------------------
// vertex2fv
//-----------------------------------------------------------------------------
void vertex2fv( const F32 *data )
{
   VERTEX_BOUNDS_CHECK();
   GFXVertexPCT *vert = &mTempVertBuff[mCurVertIndex++];

   vert->point.set( data[0], data[1], 0.f );
   vert->color = mCurColor;
   vert->texCoord = mCurTexCoord;
}



//-----------------------------------------------------------------------------
// color
//-----------------------------------------------------------------------------
void color( const ColorI &inColor )
{
   mCurColor = inColor;
}

void color( const ColorF &inColor )
{
   mCurColor = inColor;
}

void color3i( U8 red, U8 green, U8 blue )
{
   mCurColor.set( red, green, blue );
}

void color4i( U8 red, U8 green, U8 blue, U8 alpha )
{
   mCurColor.set( red, green, blue, alpha );
}

void color3f( F32 red, F32 green, F32 blue )
{
   mCurColor.set( U8( red * 255 ), U8( green * 255 ), U8( blue * 255 ) );
}

void color4f( F32 red, F32 green, F32 blue, F32 alpha )
{
   mCurColor.set( U8( red * 255 ), U8( green * 255 ), U8( blue * 255 ), U8( alpha * 255 ) );
}


//-----------------------------------------------------------------------------
// texCoord
//-----------------------------------------------------------------------------
void texCoord2f( F32 x, F32 y )
{
   mCurTexCoord.set( x, y );
}

void shutdown()
{
   mVertBuff = NULL;
}

}  // namespace PrimBuild
