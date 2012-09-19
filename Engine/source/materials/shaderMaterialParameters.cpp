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
#include "materials/shaderMaterialParameters.h"

#include "console/console.h"

//
// ShaderMaterialParameters
//
ShaderMaterialParameterHandle::ShaderMaterialParameterHandle(const String& name)
{
   VECTOR_SET_ASSOCIATION( mHandles );

   mName = name;
}

ShaderMaterialParameterHandle::ShaderMaterialParameterHandle(const String& name, Vector<GFXShader*>& shaders)
{
   VECTOR_SET_ASSOCIATION( mHandles );

   mName = name;
   mHandles.setSize(shaders.size());

   for (U32 i = 0; i < shaders.size(); i++)
      mHandles[i] = shaders[i]->getShaderConstHandle(name);
}

ShaderMaterialParameterHandle::~ShaderMaterialParameterHandle()
{
}

S32 ShaderMaterialParameterHandle::getSamplerRegister( U32 pass ) const
{
   AssertFatal( mHandles.size() > pass, "ShaderMaterialParameterHandle::getSamplerRegister - out of bounds" );   
   return mHandles[pass]->getSamplerRegister();
}

//
// ShaderMaterialParameters
//
ShaderMaterialParameters::ShaderMaterialParameters()
: MaterialParameters()
{
   VECTOR_SET_ASSOCIATION( mBuffers );
}

ShaderMaterialParameters::~ShaderMaterialParameters()
{
   releaseBuffers();   
}

void ShaderMaterialParameters::setBuffers(Vector<GFXShaderConstDesc>& constDesc, Vector<GFXShaderConstBufferRef>& buffers)
{
   mShaderConstDesc = constDesc;
   mBuffers = buffers;
}

void ShaderMaterialParameters::releaseBuffers()
{
   for (U32 i = 0; i < mBuffers.size(); i++)
   {
      mBuffers[i] = NULL;
   }
   mBuffers.setSize(0);
}

U32 ShaderMaterialParameters::getAlignmentValue(const GFXShaderConstType constType)
{
   if (mBuffers.size() > 0)
      return mBuffers[0]->getShader()->getAlignmentValue(constType);
   else
      return 0;
}

#define SHADERMATPARAM_SET(handle, f) \
   AssertFatal(handle, "Handle is NULL!" ); \
   AssertFatal(handle->isValid(), "Handle is not valid!" ); \
   AssertFatal(dynamic_cast<ShaderMaterialParameterHandle*>(handle), "Invalid handle type!"); \
   ShaderMaterialParameterHandle* h = static_cast<ShaderMaterialParameterHandle*>(handle); \
   AssertFatal(h->mHandles.size() == mBuffers.size(), "Handle length differs from buffer length!"); \
   for (U32 i = 0; i < h->mHandles.size(); i++) \
{ \
   GFXShaderConstHandle* shaderHandle = h->mHandles[i]; \
   if (shaderHandle->isValid()) \
      mBuffers[i]->set(shaderHandle, f); \
} 

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const F32 f)
{
   SHADERMATPARAM_SET(handle, f);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const Point2F& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const Point3F& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const Point4F& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const PlaneF& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const ColorF& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const S32 f)
{
   SHADERMATPARAM_SET(handle, f);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const Point2I& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const Point3I& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const Point4I& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const AlignedArray<F32>& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const AlignedArray<Point2F>& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const AlignedArray<Point3F>& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const AlignedArray<Point4F>& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const AlignedArray<S32>& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const AlignedArray<Point2I>& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const AlignedArray<Point3I>& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const AlignedArray<Point4I>& fv)
{
   SHADERMATPARAM_SET(handle, fv);
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const MatrixF& mat, const GFXShaderConstType matrixType)
{
   if ((!handle) || !handle->isValid())
      return;
   AssertFatal(dynamic_cast<ShaderMaterialParameterHandle*>(handle), "Invalid handle type!");
   ShaderMaterialParameterHandle* h = static_cast<ShaderMaterialParameterHandle*>(handle);
   AssertFatal(h->mHandles.size() == mBuffers.size(), "Handle length differs from buffer length!");
   for (U32 i = 0; i < h->mHandles.size(); i++)
   {
      GFXShaderConstHandle* shaderHandle = h->mHandles[i];
      if (shaderHandle && shaderHandle->isValid())
         mBuffers[i]->set(shaderHandle, mat, matrixType);
   } 
}

void ShaderMaterialParameters::set(MaterialParameterHandle* handle, const MatrixF* mat, const U32 arraySize, const GFXShaderConstType matrixType)
{
   if ((!handle) || !handle->isValid())
      return;
   AssertFatal(dynamic_cast<ShaderMaterialParameterHandle*>(handle), "Invalid handle type!");
   ShaderMaterialParameterHandle* h = static_cast<ShaderMaterialParameterHandle*>(handle);
   AssertFatal(h->mHandles.size() == mBuffers.size(), "Handle length differs from buffer length!");
   for (U32 i = 0; i < h->mHandles.size(); i++)
   {
      GFXShaderConstHandle* shaderHandle = h->mHandles[i];
      if (shaderHandle && shaderHandle->isValid())
         mBuffers[i]->set(shaderHandle, mat, arraySize, matrixType);
   } 
}

#undef SHADERMATPARAM_SET
