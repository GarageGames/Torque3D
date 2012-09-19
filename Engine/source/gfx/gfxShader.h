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

#ifndef _GFXSHADER_H_
#define _GFXSHADER_H_

#ifndef _GFXRESOURCE_H_
#include "gfx/gfxResource.h"
#endif
#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _ALIGNEDARRAY_H_
#include "core/util/tAlignedArray.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h"
#endif
#ifndef _GFXSTRUCTS_H_
#include "gfx/gfxStructs.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif
#ifndef _PATH_H_
#include "core/util/path.h"
#endif
#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif

class Point2I;
class Point2F;
class ColorF;
class MatrixF;
class GFXShader;
class GFXVertexFormat;


/// Instances of this struct are returned GFXShaderConstBuffer
struct GFXShaderConstDesc 
{
public:
   String name;
   GFXShaderConstType constType;   
   U32 arraySize; // > 1 means it is an array!
};

/// This is an opaque handle used by GFXShaderConstBuffer clients to set individual shader constants.
/// Derived classes can put whatever info they need into here, these handles are owned by the shader constant buffer
/// (or shader).  Client code should not free these.
class GFXShaderConstHandle 
{
public:

   GFXShaderConstHandle() { mValid = false; }
   virtual ~GFXShaderConstHandle() {}

   /// Returns true if this constant is valid and can
   /// be set on the shader.
   bool isValid() const { return mValid; }   
   
   /// Returns the name of the constant handle.
   virtual const String& getName() const = 0;

   /// Returns the type of the constant handle.
   virtual GFXShaderConstType getType() const = 0;

   virtual U32 getArraySize() const = 0;
   
   /// Returns -1 if this handle does not point to a Sampler.
   virtual S32 getSamplerRegister() const = 0;

protected:

   /// The state of the constant which is
   /// set from the derived class.
   bool mValid;
   
};


/// GFXShaderConstBuffer is a collection of shader data which
/// are sent to the device in one call in most cases.
///
/// The content of the buffer is persistant and if a value
/// does not change frequently there is a savings in not setting
/// the value every frame.
///
class GFXShaderConstBuffer : public GFXResource, public StrongRefBase
{
protected:

   /// The lost state of the buffer content.
   ///
   /// Derived classes need to set the lost state
   /// on first creation of the buffer and shader
   /// reloads.
   ///
   /// @see wasLost
   bool mWasLost;

   GFXShaderConstBuffer()   
      :  mWasLost( true ),
         mInstPtr( NULL )
   {
   }

public:

   /// Return the shader that created this buffer
   virtual GFXShader* getShader() = 0;

   /// The content of the buffer is in the lost state when
   /// first created or when the shader is reloaded.  When 
   /// the content is lost you must refill the buffer
   /// with all the constants used by your shader.
   ///
   /// Use this property to avoid setting constants which do 
   /// not changefrom one frame to the next.
   ///
   bool wasLost() const { return mWasLost; }

   /// An inline helper which ensures the handle is valid 
   /// before the virtual set method is called.
   ///
   /// You should prefer using this method unless your sure the
   /// handle is always valid or if your doing your own test.
   ///
   template< typename VALUE >
   inline void setSafe( GFXShaderConstHandle *handle, const VALUE& v )
   {
      if ( handle->isValid() )
         set( handle, v );
   }

   /// Set a shader constant.
   ///
   /// The constant handle is assumed to be valid.
   ///
   /// Perfer using setSafe unless you can check the handle
   /// validity yourself and skip a significat amount of work.
   ///   
   /// @see GFXShaderConstHandle::isValid()
   ///
   virtual void set(GFXShaderConstHandle* handle, const F32 f) = 0;
   virtual void set(GFXShaderConstHandle* handle, const Point2F& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const Point3F& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const Point4F& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const PlaneF& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const ColorF& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const S32 f) = 0;
   virtual void set(GFXShaderConstHandle* handle, const Point2I& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const Point3I& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const Point4I& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<F32>& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point2F>& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point3F>& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point4F>& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<S32>& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point2I>& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point3I>& fv) = 0;
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point4I>& fv) = 0;
   
   /// Set a variable sized matrix shader constant.   
   virtual void set( GFXShaderConstHandle* handle, 
                     const MatrixF& mat, 
                     const GFXShaderConstType matrixType = GFXSCT_Float4x4 ) = 0;
   
   /// Set a variable sized matrix shader constant from
   /// an array of matricies.   
   virtual void set( GFXShaderConstHandle* handle, 
                     const MatrixF* mat, 
                     const U32 arraySize, 
                     const GFXShaderConstType matrixType = GFXSCT_Float4x4 ) = 0;
   
   // TODO: Make this protected and put a real API around it.
   U8 *mInstPtr;
};

typedef StrongRefPtr<GFXShaderConstBuffer> GFXShaderConstBufferRef;


//**************************************************************************
// Shader
//**************************************************************************
class GFXShader : public StrongRefBase, public GFXResource
{
   friend class GFXShaderConstBuffer;

protected:
  
   /// These are system wide shader macros which are 
   /// merged with shader specific macros at creation.
   static Vector<GFXShaderMacro> smGlobalMacros;

   /// If true the shader errors are spewed to the console.
   static bool smLogErrors;

   /// If true the shader warnings are spewed to the console.
   static bool smLogWarnings;

   /// The vertex shader file.
   Torque::Path mVertexFile;  

   /// The pixel shader file.
   Torque::Path mPixelFile;  

   /// The macros to be passed to the shader.      
   Vector<GFXShaderMacro> mMacros;

   /// The pixel version this is compiled for.
   F32 mPixVersion;

   ///
   String mDescription;

   /// Counter that is incremented each time this shader is reloaded.
   U32 mReloadKey;

   Signal<void()> mReloadSignal;

   /// Vector of buffers that reference this shader.
   /// It is the responsibility of the derived shader class to populate this 
   /// vector and to notify them when this shader is reloaded.  Classes
   /// derived from GFXShaderConstBuffer should call _unlinkBuffer from
   /// their destructor.
   Vector<GFXShaderConstBuffer*> mActiveBuffers;

   /// A protected constructor so it cannot be instantiated.
   GFXShader();

public:

   // TODO: Add this into init().
   GFXVertexFormat mInstancingFormat;

   /// Adds a global shader macro which will be merged with
   /// the script defined macros on every shader reload.
   ///
   /// The macro will replace the value of an existing macro
   /// of the same name.
   ///
   /// For the new macro to take effect all the shaders/materials
   /// in the system need to be reloaded.
   ///
   /// @see MaterialManager::flushAndReInitInstances
   /// @see ShaderData::reloadAll
   static void addGlobalMacro( const String &name, const String &value = String::EmptyString );

   /// Removes an existing global macro by name.
   /// @see addGlobalMacro
   static bool removeGlobalMacro( const String &name );

   /// Toggle logging for shader errors.
   static void setLogging( bool logErrors,
                           bool logWarning ) 
   {
      smLogErrors = logErrors; 
      smLogWarnings = logWarning;
   }

   /// The destructor.
   virtual ~GFXShader();

   ///
   bool init(  const Torque::Path &vertFile, 
               const Torque::Path &pixFile, 
               F32 pixVersion, 
               const Vector<GFXShaderMacro> &macros );

   /// Reloads the shader from disk.
   bool reload();

   Signal<void()> getReloadSignal() { return mReloadSignal; }

   /// Allocate a constant buffer
   virtual GFXShaderConstBufferRef allocConstBuffer() = 0;  

   /// Returns our list of shader constants, the material can get this and just set the constants it knows about
   virtual const Vector<GFXShaderConstDesc>& getShaderConstDesc() const = 0;

   /// Returns a shader constant handle for the name constant.
   ///
   /// Since shaders can reload and later have handles that didn't 
   /// exist originally this will return a handle in an invalid state
   /// if the constant doesn't exist at this time.
   virtual GFXShaderConstHandle* getShaderConstHandle( const String& name ) = 0; 

   /// Returns the alignment value for constType
   virtual U32 getAlignmentValue(const GFXShaderConstType constType) const = 0;   

   /// Returns the required vertex format for this shader.
   /// Returns the pixel shader version.
   F32 getPixVersion() const { return mPixVersion; }

   /// Returns a counter which is incremented each time this shader is reloaded.
   U32 getReloadKey() const { return mReloadKey; }

   /// Device specific shaders can override this method to return
   /// the shader disassembly.
   virtual bool getDisassembly( String &outStr ) const { return false; }

   /// Returns the vertex shader file path.
   const String& getVertexShaderFile() const { return mVertexFile.getFullPath(); }

   /// Returns the pixel shader file path.
   const String& getPixelShaderFile() const { return mPixelFile.getFullPath(); }

   // GFXResource
   const String describeSelf() const { return mDescription; }

protected:

   /// Called when the shader files change on disk.
   void _onFileChanged( const Torque::Path &path ) { reload(); }

   /// Internal initialization function overloaded for
   /// each GFX device type.
   virtual bool _init() = 0;

   /// Buffers call this from their destructor (so we don't have to ref count them).
   void _unlinkBuffer( GFXShaderConstBuffer *buf );

   /// Called to update the description string after init.
   void _updateDesc();
};

/// A strong pointer to a reference counted GFXShader.
typedef StrongRefPtr<GFXShader> GFXShaderRef;


/// A weak pointer to a reference counted GFXShader.
typedef WeakRefPtr<GFXShader> GFXShaderWeakRef;


#endif // GFXSHADER
