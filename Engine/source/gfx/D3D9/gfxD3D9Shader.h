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

#ifndef _GFXD3D9SHADER_H_
#define _GFXD3D9SHADER_H_

#ifndef _PATH_H_
#include "core/util/path.h"
#endif
#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif
#ifndef _GFXSHADER_H_
#include "gfx/gfxShader.h"
#endif
#ifndef _GFXRESOURCE_H_
#include "gfx/gfxResource.h"
#endif
#ifndef _GENERICCONSTBUFFER_H_
#include "gfx/genericConstBuffer.h"
#endif


class GFXD3D9Shader;
struct IDirect3DVertexShader9;
struct IDirect3DPixelShader9;
struct IDirect3DDevice9;
struct ID3DXConstantTable;
struct ID3DXBuffer;
struct _D3DXMACRO;


class GFXD3D9ShaderBufferLayout : public GenericConstBufferLayout
{
protected:
   /// Set a matrix, given a base pointer
   virtual bool setMatrix(const ParamDesc& pd, const GFXShaderConstType constType, const U32 size, const void* data, U8* basePointer);
};


/// The D3D9 implementation of a shader constant handle.
class GFXD3D9ShaderConstHandle : public GFXShaderConstHandle
{
public:   

   // GFXShaderConstHandle
   const String& getName() const;
   GFXShaderConstType getType() const;
   U32 getArraySize() const;

   WeakRefPtr<GFXD3D9Shader> mShader;

   bool mVertexConstant;
   GenericConstBufferLayout::ParamDesc mVertexHandle;
   bool mPixelConstant;
   GenericConstBufferLayout::ParamDesc mPixelHandle;
   
   /// Is true if this constant is for hardware mesh instancing.
   ///
   /// Note: We currently store its settings in mPixelHandle.
   ///
   bool mInstancingConstant;

   void setValid( bool valid ) { mValid = valid; }
   S32 getSamplerRegister() const;

   // Returns true if this is a handle to a sampler register.
   bool isSampler() const 
   {
      return ( mPixelConstant && mPixelHandle.constType >= GFXSCT_Sampler ) ||
             ( mVertexConstant && mVertexHandle.constType >= GFXSCT_Sampler );
   }

   /// Restore to uninitialized state.
   void clear()
   {
      mShader = NULL;
      mVertexConstant = false;
      mPixelConstant = false;
      mInstancingConstant = false;
      mVertexHandle.clear();
      mPixelHandle.clear();
      mValid = false;
   }

   GFXD3D9ShaderConstHandle();
};


/// The D3D9 implementation of a shader constant buffer.
class GFXD3D9ShaderConstBuffer : public GFXShaderConstBuffer
{
   friend class GFXD3D9Shader;

public:

   GFXD3D9ShaderConstBuffer( GFXD3D9Shader* shader,
                             GFXD3D9ShaderBufferLayout* vertexLayoutF, 
                             GFXD3D9ShaderBufferLayout* vertexLayoutI,
                             GFXD3D9ShaderBufferLayout* pixelLayoutF, 
                             GFXD3D9ShaderBufferLayout* pixelLayoutI );
   virtual ~GFXD3D9ShaderConstBuffer();   

   /// Called by GFXD3D9Device to activate this buffer.
   /// @param mPrevShaderBuffer The previously active buffer
   void activate( GFXD3D9ShaderConstBuffer *prevShaderBuffer );
   
   /// Used internally by GXD3D9ShaderConstBuffer to determine if it's dirty.
   bool isDirty();

   /// Called from GFXD3D9Shader when constants have changed and need
   /// to be the shader this buffer references is reloaded.
   void onShaderReload( GFXD3D9Shader *shader );

   // GFXShaderConstBuffer
   virtual GFXShader* getShader();
   virtual void set(GFXShaderConstHandle* handle, const F32 fv);
   virtual void set(GFXShaderConstHandle* handle, const Point2F& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point3F& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point4F& fv);
   virtual void set(GFXShaderConstHandle* handle, const PlaneF& fv);
   virtual void set(GFXShaderConstHandle* handle, const ColorF& fv);   
   virtual void set(GFXShaderConstHandle* handle, const S32 f);
   virtual void set(GFXShaderConstHandle* handle, const Point2I& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point3I& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point4I& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<F32>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point2F>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point3F>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point4F>& fv);   
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<S32>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point2I>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point3I>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point4I>& fv);
   virtual void set(GFXShaderConstHandle* handle, const MatrixF& mat, const GFXShaderConstType matType = GFXSCT_Float4x4);
   virtual void set(GFXShaderConstHandle* handle, const MatrixF* mat, const U32 arraySize, const GFXShaderConstType matrixType = GFXSCT_Float4x4);
   
   // GFXResource
   virtual const String describeSelf() const;
   virtual void zombify();
   virtual void resurrect();

protected:

   template<class T>
   inline void SET_CONSTANT(  GFXShaderConstHandle* handle, 
                              const T& fv, 
                              GenericConstBuffer *vBuffer, 
                              GenericConstBuffer *pBuffer );

   /// A cached direct pointer to the device.
   IDirect3DDevice9 *mDevice;

   /// We keep a weak reference to the shader 
   /// because it will often be deleted.
   WeakRefPtr<GFXD3D9Shader> mShader;
   
   GFXD3D9ShaderBufferLayout* mVertexConstBufferLayoutF;
   GenericConstBuffer* mVertexConstBufferF;
   GFXD3D9ShaderBufferLayout* mPixelConstBufferLayoutF;
   GenericConstBuffer* mPixelConstBufferF;   
   GFXD3D9ShaderBufferLayout* mVertexConstBufferLayoutI;
   GenericConstBuffer* mVertexConstBufferI;
   GFXD3D9ShaderBufferLayout* mPixelConstBufferLayoutI;
   GenericConstBuffer* mPixelConstBufferI;   
};


class _gfxD3DXInclude;
typedef StrongRefPtr<_gfxD3DXInclude> _gfxD3DXIncludeRef;

/// The D3D9 implementation of a shader.
class GFXD3D9Shader : public GFXShader
{
   friend class GFXD3D9Device;
   friend class GFX360Device;
   friend class GFXD3D9ShaderConstBuffer;
   friend class GFX360ShaderConstBuffer;
public:
   typedef Map<String, GFXD3D9ShaderConstHandle*> HandleMap;

   GFXD3D9Shader();
   virtual ~GFXD3D9Shader();   

   // GFXShader
   virtual GFXShaderConstBufferRef allocConstBuffer();
   virtual const Vector<GFXShaderConstDesc>& getShaderConstDesc() const;
   virtual GFXShaderConstHandle* getShaderConstHandle(const String& name); 
   virtual U32 getAlignmentValue(const GFXShaderConstType constType) const;
   virtual bool getDisassembly( String &outStr ) const;

   // GFXResource
   virtual void zombify();
   virtual void resurrect();

protected:

   virtual bool _init();   

   static const U32 smCompiledShaderTag;

   IDirect3DDevice9 *mD3D9Device;

   IDirect3DVertexShader9 *mVertShader;
   IDirect3DPixelShader9 *mPixShader;

   GFXD3D9ShaderBufferLayout* mVertexConstBufferLayoutF;   
   GFXD3D9ShaderBufferLayout* mPixelConstBufferLayoutF;
   GFXD3D9ShaderBufferLayout* mVertexConstBufferLayoutI;   
   GFXD3D9ShaderBufferLayout* mPixelConstBufferLayoutI;

   static _gfxD3DXIncludeRef smD3DXInclude;

   HandleMap mHandles;

   /// The shader disassembly from DX when this shader is compiled.
   /// We only store this data in non-release builds.
   String mDissasembly;

   /// Vector of sampler type descriptions consolidated from _compileShader.
   Vector<GFXShaderConstDesc> mSamplerDescriptions;

   /// Vector of descriptions (consolidated for the getShaderConstDesc call)
   Vector<GFXShaderConstDesc> mShaderConsts;
   
   // These two functions are used when compiling shaders from hlsl
   virtual bool _compileShader( const Torque::Path &filePath, 
                                const String &target, 
                                const _D3DXMACRO *defines, 
                                GenericConstBufferLayout *bufferLayoutF, 
                                GenericConstBufferLayout *bufferLayoutI,
                                Vector<GFXShaderConstDesc> &samplerDescriptions );

   void _getShaderConstants( ID3DXConstantTable* table, 
                             GenericConstBufferLayout *bufferLayoutF, 
                             GenericConstBufferLayout *bufferLayoutI,
                             Vector<GFXShaderConstDesc> &samplerDescriptions );

   bool _saveCompiledOutput( const Torque::Path &filePath, 
                             ID3DXBuffer *buffer, 
                             GenericConstBufferLayout *bufferLayoutF, 
                             GenericConstBufferLayout *bufferLayoutI,
                             Vector<GFXShaderConstDesc> &samplerDescriptions );

   // Loads precompiled shaders
   bool _loadCompiledOutput( const Torque::Path &filePath, 
                             const String &target, 
                             GenericConstBufferLayout *bufferLayoutF, 
                             GenericConstBufferLayout *bufferLayoutI,
                             Vector<GFXShaderConstDesc> &samplerDescriptions );

   // This is used in both cases
   virtual void _buildShaderConstantHandles( GenericConstBufferLayout *layout, bool vertexConst );
   
   virtual void _buildSamplerShaderConstantHandles( Vector<GFXShaderConstDesc> &samplerDescriptions );

   /// Used to build the instancing shader constants from 
   /// the instancing vertex format.
   void _buildInstancingShaderConstantHandles();
};

inline bool GFXD3D9Shader::getDisassembly( String &outStr ) const
{
   outStr = mDissasembly;
   return ( outStr.isNotEmpty() );
}

#endif // _GFXD3D9SHADER_H_