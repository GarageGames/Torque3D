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

#ifndef _GFXGLSHADER_H_
#define _GFXGLSHADER_H_

#include "core/util/refBase.h"
#include "gfx/gfxShader.h"
#include "gfx/gl/tGL/tGL.h"
#include "core/util/tSignal.h"
#include "core/util/tDictionary.h"

class GFXGLShaderConstHandle;
class FileStream;
class GFXGLShaderConstBuffer;

class GFXGLShader : public GFXShader
{
   typedef Map<String, GFXGLShaderConstHandle*> HandleMap;
public:
   GFXGLShader();
   virtual ~GFXGLShader();
   
   /// @name GFXShader interface
   /// @{
   virtual GFXShaderConstHandle* getShaderConstHandle(const String& name);
   virtual GFXShaderConstHandle* findShaderConstHandle(const String& name);

   /// Returns our list of shader constants, the material can get this and just set the constants it knows about
   virtual const Vector<GFXShaderConstDesc>& getShaderConstDesc() const;

   /// Returns the alignment value for constType
   virtual U32 getAlignmentValue(const GFXShaderConstType constType) const; 

   virtual GFXShaderConstBufferRef allocConstBuffer();

   /// @}
   
   /// @name GFXResource interface
   /// @{
   virtual void zombify();
   virtual void resurrect() { reload(); }
   virtual const String describeSelf() const;
   /// @}      

   /// Activates this shader in the GL context.
   void useProgram();
   
protected:

   friend class GFXGLShaderConstBuffer;
   friend class GFXGLShaderConstHandle;
   
   virtual bool _init();   

   bool initShader(  const Torque::Path &file, 
                     bool isVertex, 
                     const Vector<GFXShaderMacro> &macros );

   void clearShaders();
   void initConstantDescs();
   void initHandles();
   void setConstantsFromBuffer(GFXGLShaderConstBuffer* buffer);
   
   static char* _handleIncludes( const Torque::Path &path, FileStream *s );

   static bool _loadShaderFromStream(  GLuint shader, 
                                       const Torque::Path& path, 
                                       FileStream* s, 
                                       const Vector<GFXShaderMacro>& macros );

   /// @name Internal GL handles
   /// @{
   GLuint mVertexShader;
   GLuint mPixelShader;
   GLuint mProgram;
   /// @}
    
   Vector<GFXShaderConstDesc> mConstants;
   U32 mConstBufferSize;
   U8* mConstBuffer;
   HandleMap mHandles;
   Vector<GFXGLShaderConstHandle*> mValidHandles;
};

class GFXGLShaderConstBuffer : public GFXShaderConstBuffer
{
public:
   GFXGLShaderConstBuffer(GFXGLShader* shader, U32 bufSize, U8* existingConstants);
   ~GFXGLShaderConstBuffer();
   
   /// Called by GFXGLDevice to activate this buffer.
   void activate();

   /// Called when the shader this buffer references is reloaded.
   void onShaderReload( GFXGLShader *shader );

   // GFXShaderConstBuffer
   virtual GFXShader* getShader() { return mShader; }
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
   virtual void zombify() {}
   virtual void resurrect() {}

private:

   friend class GFXGLShader;
   U8* mBuffer;
   WeakRefPtr<GFXGLShader> mShader;
   
   template<typename ConstType>
   void internalSet(GFXShaderConstHandle* handle, const ConstType& param);
   
   template<typename ConstType>
   void internalSet(GFXShaderConstHandle* handle, const AlignedArray<ConstType>& fv);
};

#endif // _GFXGLSHADER_H_