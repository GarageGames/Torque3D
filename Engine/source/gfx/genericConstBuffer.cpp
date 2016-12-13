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
#include "gfx/genericConstBuffer.h"

#include "platform/profiler.h"
#include "core/stream/stream.h"


GenericConstBufferLayout::GenericConstBufferLayout()
{
   VECTOR_SET_ASSOCIATION( mParams );

   mBufferSize = 0;
   mCurrentIndex = 0;
   mTimesCleared = 0;
}

void GenericConstBufferLayout::addParameter(const String& name, const GFXShaderConstType constType, const U32 offset, const U32 size, const U32 arraySize, const U32 alignValue)
{
#ifdef TORQUE_DEBUG
   // Make sure we don't have overlapping parameters
   S32 start = offset;
   S32 end = offset + size;
   for (Params::iterator i = mParams.begin(); i != mParams.end(); i++)
   {
      const ParamDesc& dp = *i;
      S32 pstart = dp.offset;
      S32 pend = pstart + dp.size;
      pstart -= start;
      pend -= end;
      // This is like a minkowski sum for two line segments, if the newly formed line contains
      // the origin, then they intersect
      bool intersect = ((pstart >= 0 && 0 >= pend) || ((pend >= 0 && 0 >= pstart)));
      AssertFatal(!intersect, "Overlapping shader parameter!");
   }
#endif
   ParamDesc desc;
   desc.name = name;
   desc.constType = constType;
   desc.offset = offset;
   desc.size = size;
   desc.arraySize = arraySize;
   desc.alignValue = alignValue;
   desc.index = mCurrentIndex++;
   mParams.push_back(desc);
   mBufferSize = getMax(desc.offset + desc.size, mBufferSize);
   AssertFatal(mBufferSize, "Empty constant buffer!");
}

bool GenericConstBufferLayout::set(const ParamDesc& pd, const GFXShaderConstType constType, const U32 size, const void* data, U8* basePointer)
{
   PROFILE_SCOPE(GenericConstBufferLayout_set);

   // Shader compilers like to optimize float4x4 uniforms into float3x3s.
   // So long as the real paramater is a matrix of-some-type and the data
   // passed in is a MatrixF ( which is will be ), we DO NOT have a
   // mismatched const type.
   AssertFatal( pd.constType == constType || 
               ( 
                 ( pd.constType == GFXSCT_Float2x2 || 
                   pd.constType == GFXSCT_Float3x3 || 
                   pd.constType == GFXSCT_Float3x4 ||  
                   pd.constType == GFXSCT_Float4x3 || 
                   pd.constType == GFXSCT_Float4x4 ) && 
                 ( constType == GFXSCT_Float2x2 || 
                   constType == GFXSCT_Float3x3 ||  
                   constType == GFXSCT_Float3x4 ||  
                   constType == GFXSCT_Float4x3 ||
                   constType == GFXSCT_Float4x4 )
               ), "Mismatched const type!" );

   // This "cute" bit of code allows us to support 2x3 and 3x3 matrices in shader constants but use our MatrixF class.  Yes, a hack. -BTR
   switch (pd.constType)
   {
   case GFXSCT_Float2x2 :
   case GFXSCT_Float3x3 :
   case GFXSCT_Float4x3 :
   case GFXSCT_Float4x4 :
      return setMatrix(pd, constType, size, data, basePointer);
      break;
   default :
      break;
   }

   AssertFatal(pd.size >= size, "Not enough room in the buffer for this data!");

   // Ok, we only set data if it's different than the data we already have, this maybe more expensive than just setting the data, but 
   // we'll have to do some timings to see.  For example, the lighting shader constants rarely change, but we can't assume that at the
   // renderInstMgr level, but we can check down here. -BTR
   if (dMemcmp(basePointer+pd.offset, data, size) != 0)      
   {
      dMemcpy(basePointer+pd.offset, data, size);      
      return true;
   }   
   return false;
}

bool GenericConstBufferLayout::setMatrix(const ParamDesc& pd, const GFXShaderConstType constType, const U32 size, const void* data, U8* basePointer)
{
   PROFILE_SCOPE(GenericConstBufferLayout_setMatrix);

   // We're generic, so just copy the full MatrixF in
   AssertFatal(pd.size >= size, "Not enough room in the buffer for this data!");

   // Matrices are an annoying case because of the alignment issues.  There are alignment issues in the matrix itself, and then potential inter matrices alignment issues.
   // So GL and DX will need to derive their own GenericConstBufferLayout classes and override this method to deal with that stuff.  For GenericConstBuffer, copy the whole
   // 4x4 matrix regardless of the target case.

   if (dMemcmp(basePointer+pd.offset, data, size) != 0)
   {      
      dMemcpy(basePointer+pd.offset, data, size);         
      return true;
   }
      
   return false;
}

bool GenericConstBufferLayout::getDesc(const String& name, ParamDesc& param) const
{
   for (U32 i = 0; i < mParams.size(); i++)
   {
      if (mParams[i].name.equal(name))
      {
         param = mParams[i];
         return true;
      }
   } 
   return false;
}

bool GenericConstBufferLayout::getDesc(const U32 index, ParamDesc& param) const
{
   if ( index < mParams.size() )
   {
      param = mParams[index];
      return true;
   }

   return false;
}

bool GenericConstBufferLayout::write(Stream* s)
{
   // Write out the size of the ParamDesc structure as a sanity check.
   if (!s->write((U32) sizeof(ParamDesc)))
      return false;
   // Next, write out the number of elements we've got.
   if (!s->write(mParams.size()))
      return false;
   for (U32 i = 0; i < mParams.size(); i++)
   {
      s->write(mParams[i].name);
         
      if (!s->write(mParams[i].offset))
         return false;
      if (!s->write(mParams[i].size))
         return false;
      U32 t = (U32) mParams[i].constType;
      if (!s->write(t))
         return false;
      if (!s->write(mParams[i].arraySize))
         return false;
      if (!s->write(mParams[i].alignValue))
         return false;
      if (!s->write(mParams[i].index))
         return false;
   }
   return true;
}

/// Load this layout from a stream
bool GenericConstBufferLayout::read(Stream* s)
{
   U32 structSize;
   if (!s->read(&structSize))
      return false;
   if (structSize != sizeof(ParamDesc))
   {
      AssertFatal(false, "Invalid shader layout structure size!");
      return false;
   }
   U32 numParams;
   if (!s->read(&numParams))
      return false;
   mParams.setSize(numParams);
   mBufferSize = 0;
   mCurrentIndex = 0;
   for (U32 i = 0; i < mParams.size(); i++)
   {
      s->read(&mParams[i].name);         
      if (!s->read(&mParams[i].offset))
         return false;
      if (!s->read(&mParams[i].size))
         return false;
      U32 t;
      if (!s->read(&t))
         return false;
      mParams[i].constType = (GFXShaderConstType) t;
      if (!s->read(&mParams[i].arraySize))
         return false;
      if (!s->read(&mParams[i].alignValue))
         return false;
      if (!s->read(&mParams[i].index))
         return false;
      mBufferSize = getMax(mParams[i].offset + mParams[i].size, mBufferSize);
      mCurrentIndex = getMax(mParams[i].index, mCurrentIndex);
   }
   mCurrentIndex++;
   return true;
}

void GenericConstBufferLayout::clear()
{
   mParams.clear();    
   mBufferSize = 0;
   mCurrentIndex = 0;
   mTimesCleared++;
}


GenericConstBuffer::GenericConstBuffer(GenericConstBufferLayout* layout)
   :  mBuffer( NULL ),
      mLayout( layout ),
      mDirtyStart( U32_MAX ),
      mDirtyEnd( 0 )
{
   if ( layout && layout->getBufferSize() > 0 )
   {
      mBuffer = new U8[mLayout->getBufferSize()];   

      // Always set a default value, that way our isEqual checks
      // will work in release as well.
      dMemset( mBuffer, 0xFFFF, mLayout->getBufferSize() );

      #ifdef TORQUE_DEBUG
      
         // Clear the debug assignment tracking.
         mWasAssigned.setSize( layout->getParameterCount() );
         dMemset( mWasAssigned.address(), 0, mWasAssigned.memSize() );

      #endif
   }
}

GenericConstBuffer::~GenericConstBuffer() 
{
   delete [] mBuffer;
}

#ifdef TORQUE_DEBUG

void GenericConstBuffer::assertUnassignedConstants( const char *shaderName )
{
   for ( U32 i=0; i < mWasAssigned.size(); i++ )
   {
      if ( mWasAssigned[i] )
         continue;

      GenericConstBufferLayout::ParamDesc pd;
      mLayout->getDesc( i, pd );

      // Assert on the unassigned constant.
      AssertFatal( false, avar( "The '%s' shader constant in shader '%s' was unassigned!",
         pd.name.c_str(), shaderName ) );
   }
}

#endif
