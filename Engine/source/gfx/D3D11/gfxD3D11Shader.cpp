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

#include "platform/platform.h"
#include "gfx/D3D11/gfxD3D11Shader.h"
#include "core/frameAllocator.h"
#include "core/stream/fileStream.h"
#include "core/util/safeDelete.h"
#include "console/console.h"

extern bool gDisassembleAllShaders;

#pragma comment(lib, "d3dcompiler.lib")

gfxD3DIncludeRef GFXD3D11Shader::smD3DInclude = NULL;

class gfxD3D11Include : public ID3DInclude, public StrongRefBase
{
private:

   Vector<String> mLastPath;

public:

   void setPath(const String &path)
   {
      mLastPath.clear();
      mLastPath.push_back(path);
   }

   gfxD3D11Include() {}
   virtual ~gfxD3D11Include() {}

   STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
   STDMETHOD(Close)(THIS_ LPCVOID pData);
};

HRESULT gfxD3D11Include::Open(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
{
   using namespace Torque;
   // First try making the path relative to the parent.
   Torque::Path path = Torque::Path::Join( mLastPath.last(), '/', pFileName );
   path = Torque::Path::CompressPath( path );

   if ( !Torque::FS::ReadFile( path, (void *&)*ppData, *pBytes, true ) )
   {
      // Ok... now try using the path as is.
      path = String( pFileName );
      path = Torque::Path::CompressPath( path );

      if ( !Torque::FS::ReadFile( path, (void *&)*ppData, *pBytes, true ) )
      {
         AssertISV(false, avar( "Failed to open include '%s'.", pFileName));
         return E_FAIL;
      }
   }

   // If the data was of zero size then we cannot recurse
   // into this file and DX won't call Close() below.
   //
   // So in this case don't push on the path.
   if ( *pBytes > 0 )
      mLastPath.push_back( path.getRootAndPath() );

   return S_OK;
}

HRESULT gfxD3D11Include::Close( THIS_ LPCVOID pData )
{
   // Free the data file and pop its path off the stack.
   delete [] (U8*)pData;
   mLastPath.pop_back();

   return S_OK;
}

GFXD3D11ShaderConstHandle::GFXD3D11ShaderConstHandle()
{
   clear();
}

const String& GFXD3D11ShaderConstHandle::getName() const
{
   if ( mVertexConstant )
      return mVertexHandle.name;
   else
      return mPixelHandle.name;
}

GFXShaderConstType GFXD3D11ShaderConstHandle::getType() const
{
   if ( mVertexConstant )
      return mVertexHandle.constType;
   else
      return mPixelHandle.constType;
}

U32 GFXD3D11ShaderConstHandle::getArraySize() const
{
   if ( mVertexConstant )
      return mVertexHandle.arraySize;
   else
      return mPixelHandle.arraySize;
}

S32 GFXD3D11ShaderConstHandle::getSamplerRegister() const
{
   if ( !mValid || !isSampler() )
      return -1;

   // We always store sampler type and register index in the pixelHandle,
   // sampler registers are shared between vertex and pixel shaders anyway.

   return mPixelHandle.offset;   
}

GFXD3D11ConstBufferLayout::GFXD3D11ConstBufferLayout()
{
   mSubBuffers.reserve(CBUFFER_MAX);
}

bool GFXD3D11ConstBufferLayout::set(const ParamDesc& pd, const GFXShaderConstType constType, const U32 inSize, const void* data, U8* basePointer)
{
   PROFILE_SCOPE(GenericConstBufferLayout_set);
   S32 size = inSize;
   // Shader compilers like to optimize float4x4 uniforms into float3x3s.
   // So long as the real paramater is a matrix of-some-type and the data
   // passed in is a MatrixF ( which is will be ), we DO NOT have a
   // mismatched const type.
   AssertFatal(pd.constType == constType ||
      (
      (pd.constType == GFXSCT_Float2x2 ||
      pd.constType == GFXSCT_Float3x3 ||
      pd.constType == GFXSCT_Float4x4) &&
      (constType == GFXSCT_Float2x2 ||
      constType == GFXSCT_Float3x3 ||
      constType == GFXSCT_Float4x4)
      ), "Mismatched const type!");

   // This "cute" bit of code allows us to support 2x3 and 3x3 matrices in shader constants but use our MatrixF class.  Yes, a hack. -BTR
   switch (pd.constType)
   {
   case GFXSCT_Float2x2:
   case GFXSCT_Float3x3:
   case GFXSCT_Float4x4:
      return setMatrix(pd, constType, size, data, basePointer);
      break;
      // TODO add other AlignedVector here
   case GFXSCT_Float2:
      if (size > sizeof(Point2F))
         size = pd.size;
   default:
      break;
   }

   AssertFatal(pd.size >= size, "Not enough room in the buffer for this data!");

   // Ok, we only set data if it's different than the data we already have, this maybe more expensive than just setting the data, but 
   // we'll have to do some timings to see.  For example, the lighting shader constants rarely change, but we can't assume that at the
   // renderInstMgr level, but we can check down here. -BTR
   if (dMemcmp(basePointer + pd.offset, data, size) != 0)
   {
      dMemcpy(basePointer + pd.offset, data, size);
      return true;
   }
   return false;
}

bool GFXD3D11ConstBufferLayout::setMatrix(const ParamDesc& pd, const GFXShaderConstType constType, const U32 size, const void* data, U8* basePointer)
{
   PROFILE_SCOPE(GFXD3D11ConstBufferLayout_setMatrix);

   if (pd.constType == GFXSCT_Float4x4)
   {
      // Special case, we can just blast this guy.
      AssertFatal(pd.size >= size, "Not enough room in the buffer for this data!");
      if (dMemcmp(basePointer+pd.offset, data, size) != 0)
      {
         dMemcpy(basePointer+pd.offset, data, size);         
         return true;
      }

      return false;
   }
   else
   {
      PROFILE_SCOPE(GFXD3D11ConstBufferLayout_setMatrix_not4x4);

      // Figure out how big of a chunk we are copying.  We're going to copy 4 columns by N rows of data
      U32 csize;
      switch (pd.constType)
      {
      case GFXSCT_Float2x2 :
         csize = 24; //this takes up 16+8
         break;
      case GFXSCT_Float3x3 : 
         csize = 44; //This takes up 16+16+12
         break;
      default:
         AssertFatal(false, "Unhandled case!");
         return false;
         break;
      }

      // Loop through and copy 
      bool ret = false;
      U8* currDestPointer = basePointer+pd.offset;
      const U8* currSourcePointer = static_cast<const U8*>(data);
      const U8* endData = currSourcePointer + size;
      while (currSourcePointer < endData)
      {
         if (dMemcmp(currDestPointer, currSourcePointer, csize) != 0)
         {
            dMemcpy(currDestPointer, currSourcePointer, csize);            
            ret = true;
         }

         currDestPointer += csize;
         currSourcePointer += sizeof(MatrixF);
      }

      return ret;
   }
}

//------------------------------------------------------------------------------
GFXD3D11ShaderConstBuffer::GFXD3D11ShaderConstBuffer( GFXD3D11Shader* shader, 
                                                      GFXD3D11ConstBufferLayout* vertexLayout,
                                                      GFXD3D11ConstBufferLayout* pixelLayout)
{
    AssertFatal( shader, "GFXD3D11ShaderConstBuffer() - Got a null shader!" );

    // We hold on to this so we don't have to call
    // this virtual method during activation.
    mShader = shader;

    for (U32 i = 0; i < CBUFFER_MAX; ++i)
    {
       mConstantBuffersV[i] = NULL;
       mConstantBuffersP[i] = NULL;
    }

    // TODO: Remove buffers and layouts that don't exist for performance?
    //Mandatory
    mVertexConstBufferLayout = vertexLayout;
    mVertexConstBuffer = new GenericConstBuffer(vertexLayout);
    
    mPixelConstBufferLayout = pixelLayout;
    mPixelConstBuffer = new GenericConstBuffer(pixelLayout);

    _createBuffers();
	
}

GFXD3D11ShaderConstBuffer::~GFXD3D11ShaderConstBuffer()
{   
   // release constant buffer
   for (U32 i = 0; i < CBUFFER_MAX; ++i)
   {
      SAFE_RELEASE(mConstantBuffersP[i]);
      SAFE_RELEASE(mConstantBuffersV[i]);
   }

   SAFE_DELETE(mVertexConstBuffer);
   SAFE_DELETE(mPixelConstBuffer);


   if ( mShader )
      mShader->_unlinkBuffer( this );
}

void GFXD3D11ShaderConstBuffer::_createBuffers()
{
   HRESULT hr;
   // Create a vertex constant buffer
   if (mVertexConstBufferLayout->getBufferSize() > 0)
   {
      const Vector<ConstSubBufferDesc> &subBuffers = mVertexConstBufferLayout->getSubBufferDesc();
      for (U32 i = 0; i < subBuffers.size(); ++i)
      {
         D3D11_BUFFER_DESC cbDesc;
         cbDesc.ByteWidth = subBuffers[i].size;
         cbDesc.Usage = D3D11_USAGE_DEFAULT;
         cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
         cbDesc.CPUAccessFlags = 0;
         cbDesc.MiscFlags = 0;
         cbDesc.StructureByteStride = 0;

         hr = D3D11DEVICE->CreateBuffer(&cbDesc, NULL, &mConstantBuffersV[i]);

         if (FAILED(hr))
         {
            AssertFatal(false, "can't create constant mConstantBuffersV!");
         }
      }
   }

   // Create a pixel constant buffer
   if (mPixelConstBufferLayout->getBufferSize())
   {
      const Vector<ConstSubBufferDesc> &subBuffers = mPixelConstBufferLayout->getSubBufferDesc();
      for (U32 i = 0; i < subBuffers.size(); ++i)
      {
         // Create a pixel float constant buffer
         D3D11_BUFFER_DESC cbDesc;
         cbDesc.ByteWidth = subBuffers[i].size;
         cbDesc.Usage = D3D11_USAGE_DEFAULT;
         cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
         cbDesc.CPUAccessFlags = 0;
         cbDesc.MiscFlags = 0;
         cbDesc.StructureByteStride = 0;

         hr = D3D11DEVICE->CreateBuffer(&cbDesc, NULL, &mConstantBuffersP[i]);

         if (FAILED(hr))
         {
            AssertFatal(false, "can't create constant mConstantBuffersP!");
         }
      }
   }
}

GFXShader* GFXD3D11ShaderConstBuffer::getShader()
{
   return mShader;
}

// This is kind of cheesy, but I don't think templates would work well here because 
// these functions potentially need to be handled differently by other derived types
template<class T>
inline void GFXD3D11ShaderConstBuffer::SET_CONSTANT(  GFXShaderConstHandle* handle, const T& fv,
                                                      GenericConstBuffer *vBuffer, GenericConstBuffer *pBuffer )
{
   AssertFatal(static_cast<const GFXD3D11ShaderConstHandle*>(handle), "Incorrect const buffer type!");
   const GFXD3D11ShaderConstHandle* h = static_cast<const GFXD3D11ShaderConstHandle*>(handle);
   AssertFatal(h, "Handle is NULL!" );
   AssertFatal(h->isValid(), "Handle is not valid!" );
   AssertFatal(!h->isSampler(), "Handle is sampler constant!" );
   AssertFatal(!mShader.isNull(), "Buffer's shader is null!" );
   AssertFatal(!h->mShader.isNull(), "Handle's shader is null!" );
   AssertFatal(h->mShader.getPointer() == mShader.getPointer(), "Mismatched shaders!");

   if ( h->mInstancingConstant )
   {
      dMemcpy( mInstPtr+h->mPixelHandle.offset, &fv, sizeof( fv ) );
      return;
   }
   if (h->mVertexConstant)
      vBuffer->set(h->mVertexHandle, fv);
   if (h->mPixelConstant)
      pBuffer->set(h->mPixelHandle, fv);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const F32 fv) 
{
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point2F& fv) 
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point3F& fv) 
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point4F& fv) 
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const PlaneF& fv) 
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const ColorF& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const S32 f)
{ 
   // This is the only type that is allowed to be used
   // with a sampler shader constant type, but it is only
   // allowed to be set from GLSL.
   //
   // So we ignore it here... all other cases will assert.
   //
   if ( ((GFXD3D11ShaderConstHandle*)handle)->isSampler() )
      return;

   SET_CONSTANT(handle, f, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point2I& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point3I& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point4I& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<F32>& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point2F>& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point3F>& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point4F>& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<S32>& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point2I>& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point3I>& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point4I>& fv)
{ 
   SET_CONSTANT(handle, fv, mVertexConstBuffer, mPixelConstBuffer);
}
#undef SET_CONSTANT

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const MatrixF& mat, const GFXShaderConstType matrixType) 
{    
   AssertFatal(handle, "Handle is NULL!" );
   AssertFatal(handle->isValid(), "Handle is not valid!" );

   AssertFatal(static_cast<const GFXD3D11ShaderConstHandle*>(handle), "Incorrect const buffer type!"); 
   const GFXD3D11ShaderConstHandle* h = static_cast<const GFXD3D11ShaderConstHandle*>(handle); 
   AssertFatal(!h->isSampler(), "Handle is sampler constant!" );
   AssertFatal(h->mShader == mShader, "Mismatched shaders!"); 

   MatrixF transposed;   
   mat.transposeTo(transposed);

   if (h->mInstancingConstant) 
   {
      if ( matrixType == GFXSCT_Float4x4 )
         dMemcpy( mInstPtr+h->mPixelHandle.offset, mat, sizeof( mat ) );
         
      // TODO: Support 3x3 and 2x2 matricies?      
      return;
   }

   if (h->mVertexConstant) 
      mVertexConstBuffer->set(h->mVertexHandle, transposed, matrixType); 
   if (h->mPixelConstant) 
      mPixelConstBuffer->set(h->mPixelHandle, transposed, matrixType);
}

void GFXD3D11ShaderConstBuffer::set(GFXShaderConstHandle* handle, const MatrixF* mat, const U32 arraySize, const GFXShaderConstType matrixType)
{
   AssertFatal(handle, "Handle is NULL!" );
   AssertFatal(handle->isValid(), "Handle is not valid!" );

   AssertFatal(static_cast<const GFXD3D11ShaderConstHandle*>(handle), "Incorrect const buffer type!"); 
   const GFXD3D11ShaderConstHandle* h = static_cast<const GFXD3D11ShaderConstHandle*>(handle); 
   AssertFatal(!h->isSampler(), "Handle is sampler constant!" );
   AssertFatal(h->mShader == mShader, "Mismatched shaders!"); 

   static Vector<MatrixF> transposed;
   if (arraySize > transposed.size())
      transposed.setSize(arraySize);   
   for (U32 i = 0; i < arraySize; i++)
      mat[i].transposeTo(transposed[i]);

   // TODO: Maybe support this in the future?
   if (h->mInstancingConstant) 
      return;

   if (h->mVertexConstant) 
      mVertexConstBuffer->set(h->mVertexHandle, transposed.begin(), arraySize, matrixType);
   if (h->mPixelConstant) 
      mPixelConstBuffer->set(h->mPixelHandle, transposed.begin(), arraySize, matrixType);
}

const String GFXD3D11ShaderConstBuffer::describeSelf() const
{
   String ret;
   ret = String("   GFXD3D11ShaderConstBuffer\n");

   for (U32 i = 0; i < mVertexConstBufferLayout->getParameterCount(); i++)
   {
      GenericConstBufferLayout::ParamDesc pd;
      mVertexConstBufferLayout->getDesc(i, pd);

      ret += String::ToString("      Constant name: %s", pd.name);
   }

   return ret;
}

void GFXD3D11ShaderConstBuffer::zombify()
{
}

void GFXD3D11ShaderConstBuffer::resurrect()
{
}

bool GFXD3D11ShaderConstBuffer::isDirty()
{
   bool ret = mVertexConstBuffer->isDirty();
   ret |= mPixelConstBuffer->isDirty();
 
   return ret;
}

void GFXD3D11ShaderConstBuffer::activate( GFXD3D11ShaderConstBuffer *prevShaderBuffer )
{
   PROFILE_SCOPE(GFXD3D11ShaderConstBuffer_activate);

   // NOTE: This is a really critical function as it gets
   // called between every draw call to update the constants.
   //
   // Alot of the calls here are inlined... be careful 
   // what you change.

   // If the buffer has changed we need to compare it
   // with the new buffer to see if we can skip copying
   // equal buffer content.
   //
   // If the buffer hasn't changed then we only will
   // be copying the changes that have occured since
   // the last activate call.
   if ( prevShaderBuffer != this )
   {
      // If the previous buffer is dirty, than we can't compare 
      // against it, because it hasn't sent its contents to the
      // card yet and must be copied.
      if ( prevShaderBuffer && !prevShaderBuffer->isDirty() )
      {
         PROFILE_SCOPE(GFXD3D11ShaderConstBuffer_activate_dirty_check_1);
         // If the buffer content is equal then we set the dirty
         // flag to false knowing the current state of the card matches
         // the new buffer.
         //
         // If the content is not equal we set the dirty flag to
         // true which causes the full content of the buffer to be
         // copied to the card.
         //
         mVertexConstBuffer->setDirty( !prevShaderBuffer->mVertexConstBuffer->isEqual( mVertexConstBuffer ) );
         mPixelConstBuffer->setDirty( !prevShaderBuffer->mPixelConstBuffer->isEqual( mPixelConstBuffer ) ); 
      } 
      else
      {
         // This happens rarely... but it can happen.
         // We copy the entire dirty state to the card.
         PROFILE_SCOPE(GFXD3D11ShaderConstBuffer_activate_dirty_check_2);

         mVertexConstBuffer->setDirty( true );
         mPixelConstBuffer->setDirty( true );
      }      
   }

   ID3D11DeviceContext* devCtx = D3D11DEVICECONTEXT;

   D3D11_MAPPED_SUBRESOURCE pConstData;
   ZeroMemory(&pConstData, sizeof(D3D11_MAPPED_SUBRESOURCE));
   
   const U8* buf;
   U32 nbBuffers = 0;
   if(mVertexConstBuffer->isDirty())
   {
      const Vector<ConstSubBufferDesc> &subBuffers = mVertexConstBufferLayout->getSubBufferDesc();
      // TODO: This is not very effecient updating the whole lot, re-implement the dirty system to work with multiple constant buffers.
      // TODO: Implement DX 11.1 UpdateSubresource1 which supports updating ranges with constant buffers
      buf = mVertexConstBuffer->getEntireBuffer();
      for (U32 i = 0; i < subBuffers.size(); ++i)
      {
         const ConstSubBufferDesc &desc = subBuffers[i];
         devCtx->UpdateSubresource(mConstantBuffersV[i], 0, NULL, buf + desc.start, desc.size, 0);
         nbBuffers++;
      }

      devCtx->VSSetConstantBuffers(0, nbBuffers, mConstantBuffersV);
   }

   nbBuffers = 0;

   if(mPixelConstBuffer->isDirty())    
   {
      const Vector<ConstSubBufferDesc> &subBuffers = mPixelConstBufferLayout->getSubBufferDesc();
      // TODO: This is not very effecient updating the whole lot, re-implement the dirty system to work with multiple constant buffers.
      // TODO: Implement DX 11.1 UpdateSubresource1 which supports updating ranges with constant buffers
      buf = mPixelConstBuffer->getEntireBuffer();
      for (U32 i = 0; i < subBuffers.size(); ++i)
      {
         const ConstSubBufferDesc &desc = subBuffers[i];
         devCtx->UpdateSubresource(mConstantBuffersP[i], 0, NULL, buf + desc.start, desc.size, 0);
         nbBuffers++;
      }

      devCtx->PSSetConstantBuffers(0, nbBuffers, mConstantBuffersP);
   }

   #ifdef TORQUE_DEBUG
      // Make sure all the constants for this buffer were assigned.
      if(mWasLost)
      {
         mVertexConstBuffer->assertUnassignedConstants( mShader->getVertexShaderFile().c_str() );
         mPixelConstBuffer->assertUnassignedConstants( mShader->getPixelShaderFile().c_str() );        
      }
   #endif

   // Clear the lost state.
   mWasLost = false;
}

void GFXD3D11ShaderConstBuffer::onShaderReload( GFXD3D11Shader *shader )
{
   AssertFatal( shader == mShader, "GFXD3D11ShaderConstBuffer::onShaderReload is hosed!" );

   // release constant buffers
   for (U32 i = 0; i < CBUFFER_MAX; ++i)
   {
      SAFE_RELEASE(mConstantBuffersP[i]);
      SAFE_RELEASE(mConstantBuffersV[i]);
   }

   SAFE_DELETE( mVertexConstBuffer );
   SAFE_DELETE( mPixelConstBuffer );
        
   AssertFatal( mVertexConstBufferLayout == shader->mVertexConstBufferLayout, "GFXD3D11ShaderConstBuffer::onShaderReload is hosed!" );
   AssertFatal( mPixelConstBufferLayout == shader->mPixelConstBufferLayout, "GFXD3D11ShaderConstBuffer::onShaderReload is hosed!" );

   mVertexConstBuffer = new GenericConstBuffer( mVertexConstBufferLayout );      
   mPixelConstBuffer = new GenericConstBuffer( mPixelConstBufferLayout ); 
  
   _createBuffers();
	
   // Set the lost state.
   mWasLost = true;
}

//------------------------------------------------------------------------------

GFXD3D11Shader::GFXD3D11Shader()
{
   VECTOR_SET_ASSOCIATION( mShaderConsts );

   AssertFatal(D3D11DEVICE, "Invalid device for shader.");
   mVertShader = NULL;
   mPixShader = NULL;
   mVertexConstBufferLayout = NULL;
   mPixelConstBufferLayout = NULL;

   if( smD3DInclude == NULL )
      smD3DInclude = new gfxD3D11Include;
}

//------------------------------------------------------------------------------

GFXD3D11Shader::~GFXD3D11Shader()
{
   for (HandleMap::Iterator i = mHandles.begin(); i != mHandles.end(); i++)
      delete i->value;

   // delete const buffer layouts
   SAFE_DELETE(mVertexConstBufferLayout);
   SAFE_DELETE(mPixelConstBufferLayout);

   // release shaders
   SAFE_RELEASE(mVertShader);
   SAFE_RELEASE(mPixShader);
   //maybe add SAFE_RELEASE(mVertexCode) ?
}

bool GFXD3D11Shader::_init()
{
   PROFILE_SCOPE( GFXD3D11Shader_Init );

   SAFE_RELEASE(mVertShader);
   SAFE_RELEASE(mPixShader);

   // Create the macro array including the system wide macros.
   const U32 macroCount = smGlobalMacros.size() + mMacros.size() + 2;
   FrameTemp<D3D_SHADER_MACRO> d3dMacros( macroCount );

   for ( U32 i=0; i < smGlobalMacros.size(); i++ )
   {
      d3dMacros[i].Name = smGlobalMacros[i].name.c_str();
      d3dMacros[i].Definition = smGlobalMacros[i].value.c_str();
   }

   for ( U32 i=0; i < mMacros.size(); i++ )
   {
      d3dMacros[i+smGlobalMacros.size()].Name = mMacros[i].name.c_str();
      d3dMacros[i+smGlobalMacros.size()].Definition = mMacros[i].value.c_str();
   }

   //TODO support D3D_FEATURE_LEVEL properly with shaders instead of hard coding at hlsl 5
   d3dMacros[macroCount - 2].Name = "TORQUE_SM";
   d3dMacros[macroCount - 2].Definition = "50";

   memset(&d3dMacros[macroCount - 1], 0, sizeof(D3D_SHADER_MACRO));

   if ( !mVertexConstBufferLayout )
      mVertexConstBufferLayout = new GFXD3D11ConstBufferLayout();
   else
      mVertexConstBufferLayout->clear();

   if ( !mPixelConstBufferLayout )
      mPixelConstBufferLayout = new GFXD3D11ConstBufferLayout();
   else
      mPixelConstBufferLayout->clear(); 

   
   mSamplerDescriptions.clear();
   mShaderConsts.clear();

   if ( !Con::getBoolVariable( "$shaders::forceLoadCSF", false ) )
   {
      if (!mVertexFile.isEmpty() && !_compileShader( mVertexFile, "vs_5_0", d3dMacros, mVertexConstBufferLayout, mSamplerDescriptions ) )
         return false;

      if (!mPixelFile.isEmpty() && !_compileShader( mPixelFile, "ps_5_0", d3dMacros, mPixelConstBufferLayout, mSamplerDescriptions ) )
         return false;

   } 
   else 
   {
      if ( !_loadCompiledOutput( mVertexFile, "vs_5_0", mVertexConstBufferLayout, mSamplerDescriptions ) )
      {
         if ( smLogErrors )
            Con::errorf( "GFXD3D11Shader::init - Unable to load precompiled vertex shader for '%s'.",  mVertexFile.getFullPath().c_str() );

         return false;
      }

      if ( !_loadCompiledOutput( mPixelFile, "ps_5_0", mPixelConstBufferLayout, mSamplerDescriptions ) )
      {
         if ( smLogErrors )
            Con::errorf( "GFXD3D11Shader::init - Unable to load precompiled pixel shader for '%s'.",  mPixelFile.getFullPath().c_str() );

         return false;
      }
   }

   // Existing handles are resored to an uninitialized state.
   // Those that are found when parsing the layout parameters
   // will then be re-initialized.
   HandleMap::Iterator iter = mHandles.begin();
   for ( ; iter != mHandles.end(); iter++ )        
      (iter->value)->clear();      

   _buildShaderConstantHandles(mVertexConstBufferLayout, true);
   _buildShaderConstantHandles(mPixelConstBufferLayout, false);

   _buildSamplerShaderConstantHandles( mSamplerDescriptions );
   _buildInstancingShaderConstantHandles();

   // Notify any existing buffers that the buffer 
   // layouts have changed and they need to update.
   Vector<GFXShaderConstBuffer*>::iterator biter = mActiveBuffers.begin();
   for ( ; biter != mActiveBuffers.end(); biter++ )
      ((GFXD3D11ShaderConstBuffer*)(*biter))->onShaderReload( this );

   return true;
}

bool GFXD3D11Shader::_compileShader( const Torque::Path &filePath, 
                                    const String& target,                                  
                                    const D3D_SHADER_MACRO *defines, 
                                    GenericConstBufferLayout* bufferLayout,
                                    Vector<GFXShaderConstDesc> &samplerDescriptions )
{
   PROFILE_SCOPE( GFXD3D11Shader_CompileShader );

   using namespace Torque;

   HRESULT res = E_FAIL;
   ID3DBlob* code = NULL;
   ID3DBlob* errorBuff = NULL;
   ID3D11ShaderReflection* reflectionTable = NULL;

#ifdef TORQUE_DEBUG
	U32 flags = D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#else
   U32 flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3; //TODO double check load times with D3DCOMPILE_OPTIMIZATION_LEVEL3
   //recommended flags for NSight, uncomment to use. NSight should be used in release mode only. *Still works with above flags however
   //flags = D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

#ifdef D3D11_DEBUG_SPEW
   Con::printf( "Compiling Shader: '%s'", filePath.getFullPath().c_str() );
#endif

   // Is it an HLSL shader?
   if(filePath.getExtension().equal("hlsl", String::NoCase))   
   {
      // Set this so that the D3DInclude::Open will have this 
      // information for relative paths.
      smD3DInclude->setPath(filePath.getRootAndPath());

      FileStream s;
      if (!s.open(filePath, Torque::FS::File::Read))
      {
         AssertISV(false, avar("GFXD3D11Shader::initShader - failed to open shader '%s'.", filePath.getFullPath().c_str()));

         if ( smLogErrors )
            Con::errorf( "GFXD3D11Shader::_compileShader - Failed to open shader file '%s'.", filePath.getFullPath().c_str() );

         return false;
      }

      // Convert the path which might have virtualized
      // mount paths to a real file system path.
      Torque::Path realPath;
      if (!FS::GetFSPath( filePath, realPath))
         realPath = filePath;

      U32 bufSize = s.getStreamSize();

      FrameAllocatorMarker fam;
      char *buffer = NULL;

      buffer = (char*)fam.alloc(bufSize + 1);
      s.read(bufSize, buffer);
      buffer[bufSize] = 0;

      res = D3DCompile(buffer, bufSize, realPath.getFullPath().c_str(), defines, smD3DInclude, "main", target, flags, 0, &code, &errorBuff);
      
   }

   // Is it a precompiled obj shader?
   else if(filePath.getExtension().equal("obj", String::NoCase))
   {     
      FileStream  s;
      if(!s.open(filePath, Torque::FS::File::Read))
      {
         AssertISV(false, avar("GFXD3D11Shader::initShader - failed to open shader '%s'.", filePath.getFullPath().c_str()));

         if ( smLogErrors )
            Con::errorf( "GFXD3D11Shader::_compileShader - Failed to open shader file '%s'.", filePath.getFullPath().c_str() );

         return false;
      }

	  res = D3DCreateBlob(s.getStreamSize(), &code);
      AssertISV(SUCCEEDED(res), "Unable to create buffer!");
      s.read(s.getStreamSize(), code->GetBufferPointer());
   }
   else
   {
      if (smLogErrors)
         Con::errorf("GFXD3D11Shader::_compileShader - Unsupported shader file type '%s'.", filePath.getFullPath().c_str());

      return false;
   }  

   if(errorBuff)
   {
      // remove \n at end of buffer
      U8 *buffPtr = (U8*) errorBuff->GetBufferPointer();
      U32 len = dStrlen( (const char*) buffPtr );
      buffPtr[len-1] = '\0';

      if(FAILED(res))
      {
         if(smLogErrors)
          Con::errorf("failed to compile shader: %s", buffPtr);
      }
      else
      {
         if(smLogWarnings)
            Con::errorf("shader compiled with warning(s): %s", buffPtr);
      }
   }
   else if (code == NULL && smLogErrors)
      Con::errorf( "GFXD3D11Shader::_compileShader - no compiled code produced; possibly missing file '%s'.", filePath.getFullPath().c_str() );

   AssertISV(SUCCEEDED(res), "Unable to compile shader!");

   if(code != NULL)
   {
#ifndef TORQUE_SHIPPING         

         if(gDisassembleAllShaders)
         {
            ID3DBlob* disassem = NULL;
            D3DDisassemble(code->GetBufferPointer(), code->GetBufferSize(), 0, NULL, &disassem);
            mDissasembly = (const char*)disassem->GetBufferPointer();

            String filename = filePath.getFullPath();
            filename.replace( ".hlsl", "_dis.txt" );

            FileStream *fstream = FileStream::createAndOpen( filename, Torque::FS::File::Write );
            if ( fstream )
            {            
               fstream->write( mDissasembly );
               fstream->close();
               delete fstream;   
            }

            SAFE_RELEASE(disassem);
         }         

#endif

         if (target.compare("ps_", 3) == 0)      
            res = D3D11DEVICE->CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), NULL,  &mPixShader);
         else if (target.compare("vs_", 3) == 0)
            res = D3D11DEVICE->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), NULL, &mVertShader);
         
         if (FAILED(res))
         {
            AssertFatal(false, "D3D11Shader::_compilershader- failed to create shader");
         }

	      if(res == S_OK){
		      HRESULT reflectionResult = D3DReflect(code->GetBufferPointer(), code->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflectionTable);
		   if(FAILED(reflectionResult))
		      AssertFatal(false, "D3D11Shader::_compilershader - Failed to get shader reflection table interface");
	  }

	  if(res == S_OK)
		_getShaderConstants(reflectionTable, bufferLayout, samplerDescriptions);	  

#ifdef TORQUE_ENABLE_CSF_GENERATION

      // Ok, we've got a valid shader and constants, let's write them all out.
      if (!_saveCompiledOutput(filePath, code, bufferLayout) && smLogErrors)
         Con::errorf( "GFXD3D11Shader::_compileShader - Unable to save shader compile output for: %s", 
            filePath.getFullPath().c_str());
#endif

      if(FAILED(res) && smLogErrors)
         Con::errorf("GFXD3D11Shader::_compileShader - Unable to create shader for '%s'.", filePath.getFullPath().c_str());
   }

   //bool result = code && SUCCEEDED(res) && HasValidConstants;
   bool result = code && SUCCEEDED(res);

#ifdef TORQUE_DEBUG
   if (target.compare("vs_", 3) == 0)
   {
      String vertShader = mVertexFile.getFileName();
      mVertShader->SetPrivateData(WKPDID_D3DDebugObjectName, vertShader.size(), vertShader.c_str());
   }
   else if (target.compare("ps_", 3) == 0)
   {
      String pixelShader = mPixelFile.getFileName();
      mPixShader->SetPrivateData(WKPDID_D3DDebugObjectName, pixelShader.size(), pixelShader.c_str());
   }
#endif
  
   SAFE_RELEASE(code); 
   SAFE_RELEASE(reflectionTable);
   SAFE_RELEASE(errorBuff);

   return result;
}
void GFXD3D11Shader::_getShaderConstants( ID3D11ShaderReflection *table, 
                                         GenericConstBufferLayout *bufferLayoutIn,
                                         Vector<GFXShaderConstDesc> &samplerDescriptions )
{
   PROFILE_SCOPE( GFXD3D11Shader_GetShaderConstants );

   AssertFatal(table, "NULL constant table not allowed, is this an assembly shader?");

   GFXD3D11ConstBufferLayout *bufferLayout = (GFXD3D11ConstBufferLayout*)bufferLayoutIn;
   Vector<ConstSubBufferDesc> &subBuffers = bufferLayout->getSubBufferDesc();
   subBuffers.clear();

   D3D11_SHADER_DESC tableDesc;
   HRESULT hr = table->GetDesc(&tableDesc);
   if (FAILED(hr))
   {
	   AssertFatal(false, "Shader Reflection table unable to be created");
   }
   
   //offset for sub constant buffers
   U32 bufferOffset = 0;
   for (U32 i = 0; i < tableDesc.ConstantBuffers; i++)
   {
	   ID3D11ShaderReflectionConstantBuffer* constantBuffer = table->GetConstantBufferByIndex(i);
	   D3D11_SHADER_BUFFER_DESC constantBufferDesc;

      if (constantBuffer->GetDesc(&constantBufferDesc) == S_OK)
      {

   #ifdef TORQUE_DEBUG
         AssertFatal(constantBufferDesc.Type == D3D_CT_CBUFFER, "Only scalar cbuffers supported for now.");

         if (dStrcmp(constantBufferDesc.Name, "$Globals") != 0 && dStrcmp(constantBufferDesc.Name, "$Params") != 0)
            AssertFatal(false, "Only $Global and $Params cbuffer supported for now.");
   #endif
   #ifdef D3D11_DEBUG_SPEW
         Con::printf("Constant Buffer Name: %s", constantBufferDesc.Name);
   #endif 
         
         for(U32 j =0; j< constantBufferDesc.Variables; j++)
		   {
            GFXShaderConstDesc desc;
			   ID3D11ShaderReflectionVariable* variable = constantBuffer->GetVariableByIndex(j); 
			   D3D11_SHADER_VARIABLE_DESC variableDesc;
			   D3D11_SHADER_TYPE_DESC variableTypeDesc;

			   variable->GetDesc(&variableDesc);

			   ID3D11ShaderReflectionType* variableType =variable->GetType();

			   variableType->GetDesc(&variableTypeDesc);
			   desc.name = String(variableDesc.Name);
			   // Prepend a "$" if it doesn't exist.  Just to make things consistent.
			   if (desc.name.find("$") != 0)
			      desc.name = String::ToString("$%s", desc.name.c_str());

            bool unusedVar = variableDesc.uFlags & D3D_SVF_USED ? false : true;

            if (variableTypeDesc.Elements == 0)
               desc.arraySize = 1;
            else
               desc.arraySize = variableTypeDesc.Elements;

   #ifdef D3D11_DEBUG_SPEW
            Con::printf("Variable Name %s:, offset: %d, size: %d, constantDesc.Elements: %d", desc.name.c_str(), variableDesc.StartOffset, variableDesc.Size, desc.arraySize);
   #endif           
            if (_convertShaderVariable(variableTypeDesc, desc))
            {
               //The HLSL compiler for 4.0 and above doesn't strip out unused registered constants. We'll have to do it manually
               if (!unusedVar)
               {
                  mShaderConsts.push_back(desc);
                  U32 alignBytes = getAlignmentValue(desc.constType);
                  U32 paramSize = variableDesc.Size;
                  bufferLayout->addParameter(   desc.name,
                                                desc.constType,
                                                variableDesc.StartOffset + bufferOffset,
                                                paramSize,
                                                desc.arraySize,
                                                alignBytes);

               } //unusedVar
            } //_convertShaderVariable
		   } //constantBufferDesc.Variables

         // fill out our const sub buffer sizes etc
         ConstSubBufferDesc subBufferDesc;
         subBufferDesc.size = constantBufferDesc.Size;
         subBufferDesc.start = bufferOffset;
         subBuffers.push_back(subBufferDesc);
         // increase our bufferOffset by the constant buffer size
         bufferOffset += constantBufferDesc.Size;

      }
      else
         AssertFatal(false, "Unable to get shader constant description! (may need more elements of constantDesc");	
   }

   // Set buffer size to the aligned size
   bufferLayout->setSize(bufferOffset);


   //get the sampler descriptions from the resource binding description
   U32 resourceCount = tableDesc.BoundResources;
   for (U32 i = 0; i < resourceCount; i++)
   {
      GFXShaderConstDesc desc;
      D3D11_SHADER_INPUT_BIND_DESC bindDesc;
      table->GetResourceBindingDesc(i, &bindDesc);

      switch (bindDesc.Type)
      {
      case D3D_SIT_SAMPLER:
         // Prepend a "$" if it doesn't exist.  Just to make things consistent.
         desc.name = String(bindDesc.Name);
         if (desc.name.find("$") != 0)
            desc.name = String::ToString("$%s", desc.name.c_str());
         desc.constType = GFXSCT_Sampler;
         desc.arraySize = bindDesc.BindPoint;
         samplerDescriptions.push_back(desc);
         break;

      }
   }

}

bool GFXD3D11Shader::_convertShaderVariable(const D3D11_SHADER_TYPE_DESC &typeDesc, GFXShaderConstDesc &desc)
{
   switch (typeDesc.Type)
   {
   case D3D_SVT_INT:
   {
      switch (typeDesc.Class)
      {
      case D3D_SVC_SCALAR:
         desc.constType = GFXSCT_Int;
         break;
      case D3D_SVC_VECTOR:
      {
         switch (typeDesc.Columns)
         {
         case 1:
            desc.constType = GFXSCT_Int;
            break;
         case 2:
            desc.constType = GFXSCT_Int2;
            break;
         case 3:
            desc.constType = GFXSCT_Int3;
            break;
         case 4:
            desc.constType = GFXSCT_Int4;
            break;
         }
      }
      break;
      }
      break;
   }   
   case D3D_SVT_FLOAT:
   {
      switch (typeDesc.Class)
      {
      case D3D_SVC_SCALAR:
         desc.constType = GFXSCT_Float;
         break;
      case D3D_SVC_VECTOR:
      {
         switch (typeDesc.Columns)
         {
         case 1:
            desc.constType = GFXSCT_Float;
            break;
         case 2:
            desc.constType = GFXSCT_Float2;
            break;
         case 3:
            desc.constType = GFXSCT_Float3;
            break;
         case 4:
            desc.constType = GFXSCT_Float4;
            break;
         }
      }
      break;
      case D3D_SVC_MATRIX_ROWS:
      case D3D_SVC_MATRIX_COLUMNS:
      {
         switch (typeDesc.Columns)
         {
         case 3:
            if (typeDesc.Rows == 3)
            {
               desc.constType = GFXSCT_Float3x3;
            }
            break;
         case 4:
            if (typeDesc.Rows == 4)
            {
               desc.constType = GFXSCT_Float4x4;
            }
            break;
         }
      }
      break;
      case D3D_SVC_OBJECT:
      case D3D_SVC_STRUCT:
         return false;
      }
   }
   break;

   default:
      AssertFatal(false, "Unknown shader constant class enum");
      break;
   }

   return true;
}

const U32 GFXD3D11Shader::smCompiledShaderTag = MakeFourCC('t','c','s','f');

bool GFXD3D11Shader::_saveCompiledOutput( const Torque::Path &filePath, 
                                         ID3DBlob *buffer, 
                                         GenericConstBufferLayout *bufferLayout,
                                         Vector<GFXShaderConstDesc> &samplerDescriptions )
{
   Torque::Path outputPath(filePath);
   outputPath.setExtension("csf");     // "C"ompiled "S"hader "F"ile (fancy!)

   FileStream f;
   if (!f.open(outputPath, Torque::FS::File::Write))
      return false;
   if (!f.write(smCompiledShaderTag))
      return false;
   // We could reverse engineer the structure in the compiled output, but this
   // is a bit easier because we can just read it into the struct that we want.
   if (!bufferLayout->write(&f))
      return false;

   U32 bufferSize = buffer->GetBufferSize();
   if (!f.write(bufferSize))
      return false;
   if (!f.write(bufferSize, buffer->GetBufferPointer()))
      return false;

   // Write out sampler descriptions.

   f.write( samplerDescriptions.size() );   

   for ( U32 i = 0; i < samplerDescriptions.size(); i++ )
   {
      f.write( samplerDescriptions[i].name );
      f.write( (U32)(samplerDescriptions[i].constType) );
      f.write( samplerDescriptions[i].arraySize );
   }

   f.close();

   return true;
}

bool GFXD3D11Shader::_loadCompiledOutput( const Torque::Path &filePath, 
                                         const String &target, 
                                         GenericConstBufferLayout *bufferLayout,
                                         Vector<GFXShaderConstDesc> &samplerDescriptions )
{
   Torque::Path outputPath(filePath);
   outputPath.setExtension("csf");     // "C"ompiled "S"hader "F"ile (fancy!)

   FileStream f;
   if (!f.open(outputPath, Torque::FS::File::Read))
      return false;
   U32 fileTag;
   if (!f.read(&fileTag))
      return false;
   if (fileTag != smCompiledShaderTag)
      return false;
   if (!bufferLayout->read(&f))
      return false;
   U32 bufferSize;
   if (!f.read(&bufferSize))
      return false;
   U32 waterMark = FrameAllocator::getWaterMark();
   DWORD* buffer = static_cast<DWORD*>(FrameAllocator::alloc(bufferSize));
   if (!f.read(bufferSize, buffer))
      return false;

   // Read sampler descriptions.

   U32 samplerCount;
   f.read( &samplerCount );   

   for ( U32 i = 0; i < samplerCount; i++ )
   {
      GFXShaderConstDesc samplerDesc;
      f.read( &(samplerDesc.name) );
      f.read( (U32*)&(samplerDesc.constType) );
      f.read( &(samplerDesc.arraySize) );

      samplerDescriptions.push_back( samplerDesc );
   }

   f.close();

   HRESULT res;
   if (target.compare("ps_", 3) == 0)      
      res = D3D11DEVICE->CreatePixelShader(buffer, bufferSize, NULL, &mPixShader);
   else
      res = D3D11DEVICE->CreateVertexShader(buffer, bufferSize, NULL, &mVertShader);
   AssertFatal(SUCCEEDED(res), "Unable to load shader!");

   FrameAllocator::setWaterMark(waterMark);
   return SUCCEEDED(res);
}

void GFXD3D11Shader::_buildShaderConstantHandles(GenericConstBufferLayout* layout, bool vertexConst)
{                     
   for (U32 i = 0; i < layout->getParameterCount(); i++)
   {
      GenericConstBufferLayout::ParamDesc pd;
      layout->getDesc(i, pd);

      GFXD3D11ShaderConstHandle* handle;
      HandleMap::Iterator j = mHandles.find(pd.name);

      if (j != mHandles.end())
      {
         handle = j->value;         
         handle->mShader = this;
         handle->setValid( true );
      } 
      else
      {
         handle = new GFXD3D11ShaderConstHandle();
         handle->mShader = this;
         mHandles[pd.name] = handle;
         handle->setValid( true );
      }

      if (vertexConst)
      {
         handle->mVertexConstant = true;
         handle->mVertexHandle = pd;
      }
      else
      {
         handle->mPixelConstant = true;
         handle->mPixelHandle = pd;
      }
   }
}

void GFXD3D11Shader::_buildSamplerShaderConstantHandles( Vector<GFXShaderConstDesc> &samplerDescriptions )
{                     
   Vector<GFXShaderConstDesc>::iterator iter = samplerDescriptions.begin();
   for ( ; iter != samplerDescriptions.end(); iter++ )
   {
      const GFXShaderConstDesc &desc = *iter;

      AssertFatal(   desc.constType == GFXSCT_Sampler || 
                     desc.constType == GFXSCT_SamplerCube, 
                     "GFXD3D11Shader::_buildSamplerShaderConstantHandles - Invalid samplerDescription type!" );

      GFXD3D11ShaderConstHandle *handle;
      HandleMap::Iterator j = mHandles.find(desc.name);

      if ( j != mHandles.end() )
         handle = j->value;
      else
      {
         handle = new GFXD3D11ShaderConstHandle();
         mHandles[desc.name] = handle;         
      }

      handle->mShader = this;         
      handle->setValid( true );         
      handle->mPixelConstant = true;
      handle->mPixelHandle.name = desc.name;
      handle->mPixelHandle.constType = desc.constType;
      handle->mPixelHandle.offset = desc.arraySize;         
   }
}

void GFXD3D11Shader::_buildInstancingShaderConstantHandles()
{
   // If we have no instancing than just return
   if (!mInstancingFormat)
      return;

   U32 offset = 0;
   for ( U32 i=0; i < mInstancingFormat->getElementCount(); i++ )
   {
      const GFXVertexElement &element = mInstancingFormat->getElement( i );
      
      String constName = String::ToString( "$%s", element.getSemantic().c_str() );

      GFXD3D11ShaderConstHandle *handle;
      HandleMap::Iterator j = mHandles.find( constName );

      if ( j != mHandles.end() )
         handle = j->value; 
      else
      {
         handle = new GFXD3D11ShaderConstHandle();
         mHandles[ constName ] = handle;         
      }

      handle->mShader = this;
      handle->setValid( true );         
      handle->mInstancingConstant = true;

      // We shouldn't have an instancing constant that is also 
      // a vertex or pixel constant!  This means the shader features
      // are confused as to what is instanced.
      //
      AssertFatal(   !handle->mVertexConstant &&
                     !handle->mPixelConstant,
                     "GFXD3D11Shader::_buildInstancingShaderConstantHandles - Bad instanced constant!" );

      // HACK:  The GFXD3D11ShaderConstHandle will check mVertexConstant then
      // fall back to reading the mPixelHandle values.  We depend on this here
      // and store the data we need in the mPixelHandle constant although its
      // not a pixel shader constant.
      //
      handle->mPixelHandle.name = constName;
      handle->mPixelHandle.offset = offset;

      // If this is a matrix we will have 2 or 3 more of these
      // semantics with the same name after it.
      for ( ; i < mInstancingFormat->getElementCount(); i++ )
      {
         const GFXVertexElement &nextElement = mInstancingFormat->getElement( i );
         if ( nextElement.getSemantic() != element.getSemantic() )
         {
            i--;
            break;
         }
         offset += nextElement.getSizeInBytes();
      }
   }
}

GFXShaderConstBufferRef GFXD3D11Shader::allocConstBuffer()
{
   if (mVertexConstBufferLayout && mPixelConstBufferLayout)
   {
      GFXD3D11ShaderConstBuffer* buffer = new GFXD3D11ShaderConstBuffer(this, mVertexConstBufferLayout, mPixelConstBufferLayout);
      mActiveBuffers.push_back( buffer );
      buffer->registerResourceWithDevice(getOwningDevice());
      return buffer;
   } 

   return NULL;
}

/// Returns a shader constant handle for name, if the variable doesn't exist NULL is returned.
GFXShaderConstHandle* GFXD3D11Shader::getShaderConstHandle(const String& name)
{
   HandleMap::Iterator i = mHandles.find(name);   
   if ( i != mHandles.end() )
   {
      return i->value;
   } 
   else 
   {     
      GFXD3D11ShaderConstHandle *handle = new GFXD3D11ShaderConstHandle();
      handle->setValid( false );
      handle->mShader = this;
      mHandles[name] = handle;

      return handle;      
   }      
}

GFXShaderConstHandle* GFXD3D11Shader::findShaderConstHandle(const String& name)
{
   HandleMap::Iterator i = mHandles.find(name);
   if(i != mHandles.end())
      return i->value;
   else
   {
      return NULL;
   }
}

const Vector<GFXShaderConstDesc>& GFXD3D11Shader::getShaderConstDesc() const
{
   return mShaderConsts;
}

U32 GFXD3D11Shader::getAlignmentValue(const GFXShaderConstType constType) const
{   
   const U32 mRowSizeF = 16;
   const U32 mRowSizeI = 16;

   switch (constType)
   {
      case GFXSCT_Float :
      case GFXSCT_Float2 :
      case GFXSCT_Float3 : 
      case GFXSCT_Float4 :
         return mRowSizeF;
         break;
         // Matrices
      case GFXSCT_Float2x2 :
         return mRowSizeF * 2;
         break;
      case GFXSCT_Float3x3 : 
         return mRowSizeF * 3;
         break;
      case GFXSCT_Float4x4 :
         return mRowSizeF * 4;
         break;   
      //// Scalar
      case GFXSCT_Int :
      case GFXSCT_Int2 :
      case GFXSCT_Int3 : 
      case GFXSCT_Int4 :
         return mRowSizeI;
         break;
      default:
         AssertFatal(false, "Unsupported type!");
         return 0;
         break;
   }
}

void GFXD3D11Shader::zombify()
{
   // Shaders don't need zombification
}

void GFXD3D11Shader::resurrect()
{
   // Shaders are never zombies, and therefore don't have to be brought back.
}
