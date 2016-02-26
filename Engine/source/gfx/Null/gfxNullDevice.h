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

#ifndef _GFXNullDevice_H_
#define _GFXNullDevice_H_

#include "platform/platform.h"

//-----------------------------------------------------------------------------

#include "gfx/gfxDevice.h"
#include "gfx/gfxInit.h"
#include "gfx/gfxFence.h"

class GFXNullWindowTarget : public GFXWindowTarget
{
public:
   virtual bool present()
   {
      return true;
   }

   virtual const Point2I getSize()
   {
      // Return something stupid.
      return Point2I(1,1);
   }

   virtual GFXFormat getFormat() { return GFXFormatR8G8B8A8; }

   virtual void resetMode()
   {

   }

   virtual void zombify() {};
   virtual void resurrect() {};

};

class GFXNullDevice : public GFXDevice
{
public:
   GFXNullDevice();
   virtual ~GFXNullDevice();

   static GFXDevice *createInstance( U32 adapterIndex );

   static void enumerateAdapters( Vector<GFXAdapter*> &adapterList );

   void init( const GFXVideoMode &mode, PlatformWindow *window = NULL );

   virtual void activate() { };
   virtual void deactivate() { };
   virtual GFXAdapterType getAdapterType() { return NullDevice; };

   /// @name Debug Methods
   /// @{
   virtual void enterDebugEvent(ColorI color, const char *name) { };
   virtual void leaveDebugEvent() { };
   virtual void setDebugMarker(ColorI color, const char *name) { };
   /// @}

   /// Enumerates the supported video modes of the device
   virtual void enumerateVideoModes() { };

   /// Sets the video mode for the device
   virtual void setVideoMode( const GFXVideoMode &mode ) { };
protected:
   static GFXAdapter::CreateDeviceInstanceDelegate mCreateDeviceInstance; 

   /// Called by GFXDevice to create a device specific stateblock
   virtual GFXStateBlockRef createStateBlockInternal(const GFXStateBlockDesc& desc);
   /// Called by GFXDevice to actually set a stateblock.
   virtual void setStateBlockInternal(GFXStateBlock* block, bool force) { };
   /// @}

   /// Called by base GFXDevice to actually set a const buffer
   virtual void setShaderConstBufferInternal(GFXShaderConstBuffer* buffer) { };

   virtual void setTextureInternal(U32 textureUnit, const GFXTextureObject*texture) { };

   virtual void setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable);
   virtual void setLightMaterialInternal(const GFXLightMaterial mat) { };
   virtual void setGlobalAmbientInternal(ColorF color) { };

   /// @name State Initalization.
   /// @{

   /// State initalization. This MUST BE CALLED in setVideoMode after the device
   /// is created.
   virtual void initStates() { };

   virtual void setMatrix( GFXMatrixType mtype, const MatrixF &mat ) { };

   virtual GFXVertexBuffer *allocVertexBuffer(  U32 numVerts, 
                                                const GFXVertexFormat *vertexFormat, 
                                                U32 vertSize, 
                                                GFXBufferType bufferType,
                                                void* data = NULL );
   virtual GFXPrimitiveBuffer *allocPrimitiveBuffer(  U32 numIndices, 
                                                      U32 numPrimitives, 
                                                      GFXBufferType bufferType,
                                                      void* data = NULL );

   virtual GFXVertexDecl* allocVertexDecl( const GFXVertexFormat *vertexFormat ) { return NULL; }
   virtual void setVertexDecl( const GFXVertexDecl *decl ) {  }
   virtual void setVertexStream( U32 stream, GFXVertexBuffer *buffer ) { }
   virtual void setVertexStreamFrequency( U32 stream, U32 frequency ) { }

public:
   virtual GFXCubemap * createCubemap();

   virtual F32 getFillConventionOffset() const { return 0.0f; };

   ///@}

   virtual GFXTextureTarget *allocRenderToTextureTarget(){return NULL;};
   virtual GFXWindowTarget *allocWindowTarget(PlatformWindow *window)
   {
      return new GFXNullWindowTarget();
   };

   virtual void _updateRenderTargets(){};

   virtual F32 getPixelShaderVersion() const { return 0.0f; };
   virtual void setPixelShaderVersion( F32 version ) { };
   virtual U32 getNumSamplers() const { return 0; };
   virtual U32 getNumRenderTargets() const { return 0; };

   virtual GFXShader* createShader() { return NULL; };


   virtual void clear( U32 flags, ColorI color, F32 z, U32 stencil ) { };
   virtual bool beginSceneInternal() { return true; };
   virtual void endSceneInternal() { };

   virtual void drawPrimitive( GFXPrimitiveType primType, U32 vertexStart, U32 primitiveCount ) { };
   virtual void drawIndexedPrimitive(  GFXPrimitiveType primType, 
                                       U32 startVertex, 
                                       U32 minIndex, 
                                       U32 numVerts, 
                                       U32 startIndex, 
                                       U32 primitiveCount ) { };

   virtual void setClipRect( const RectI &rect ) { };
   virtual const RectI &getClipRect() const { return clip; };

   virtual void preDestroy() { Parent::preDestroy(); };

   virtual U32 getMaxDynamicVerts() { return 16384; };
   virtual U32 getMaxDynamicIndices() { return 16384; };

   virtual GFXFormat selectSupportedFormat(  GFXTextureProfile *profile, 
                                             const Vector<GFXFormat> &formats, 
                                             bool texture, 
                                             bool mustblend, 
                                             bool mustfilter ) { return GFXFormatR8G8B8A8; };

   GFXFence *createFence() { return new GFXGeneralFence( this ); }
   GFXOcclusionQuery* createOcclusionQuery() { return NULL; }
   
private:
   typedef GFXDevice Parent;
   RectI clip;
};

#endif
