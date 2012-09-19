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
#include "gfx/gl/gfxGLDevice.h"

#include "gfx/gfxCubemap.h"
#include "gfx/screenshot.h"
#include "gfx/gfxDrawUtil.h"

#include "gfx/gl/gfxGLEnumTranslate.h"
#include "gfx/gl/gfxGLVertexBuffer.h"
#include "gfx/gl/gfxGLPrimitiveBuffer.h"
#include "gfx/gl/gfxGLTextureTarget.h"
#include "gfx/gl/gfxGLTextureManager.h"
#include "gfx/gl/gfxGLTextureObject.h"
#include "gfx/gl/gfxGLCubemap.h"
#include "gfx/gl/gfxGLCardProfiler.h"
#include "gfx/gl/gfxGLWindowTarget.h"
#include "gfx/gl/ggl/ggl.h" 
#include "platform/platformDlibrary.h"
#include "gfx/gl/gfxGLShader.h"
#include "gfx/primBuilder.h"
#include "console/console.h"
#include "gfx/gl/gfxGLOcclusionQuery.h"

GFXAdapter::CreateDeviceInstanceDelegate GFXGLDevice::mCreateDeviceInstance(GFXGLDevice::createInstance); 

GFXDevice *GFXGLDevice::createInstance( U32 adapterIndex )
{
   return new GFXGLDevice(adapterIndex);
}

namespace GL
{
   extern void gglPerformBinds();
   extern void gglPerformExtensionBinds(void *context);
}

void loadGLCore()
{
   static bool coreLoaded = false; // Guess what this is for.
   if(coreLoaded)
      return;
   coreLoaded = true;
   
   // Make sure we've got our GL bindings.
   GL::gglPerformBinds();
}

void loadGLExtensions(void *context)
{
   static bool extensionsLoaded = false;
   if(extensionsLoaded)
      return;
   extensionsLoaded = true;
   
   GL::gglPerformExtensionBinds(context);
}

void GFXGLDevice::initGLState()
{  
   // We don't currently need to sync device state with a known good place because we are
   // going to set everything in GFXGLStateBlock, but if we change our GFXGLStateBlock strategy, this may
   // need to happen.
   
   // Deal with the card profiler here when we know we have a valid context.
   mCardProfiler = new GFXGLCardProfiler();
   mCardProfiler->init(); 
   glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint*)&mMaxShaderTextures);
   glGetIntegerv(GL_MAX_TEXTURE_UNITS, (GLint*)&mMaxFFTextures);
   
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   
   // Apple's drivers lie and claim that everything supports fragment shaders.  Conveniently they don't lie about the number
   // of supported image units.  Checking for 16 or more image units ensures that we don't try and use pixel shaders on
   // cards which don't support them.
   if(mCardProfiler->queryProfile("GL::suppFragmentShader") && mMaxShaderTextures >= 16)
      mPixelShaderVersion = 2.0f;
   else
      mPixelShaderVersion = 0.0f;
   
   // MACHAX - Setting mPixelShaderVersion to 3.0 will allow Advanced Lighting
   // to run.  At the time of writing (6/18) it doesn't quite work yet.
   if(Con::getBoolVariable("$pref::machax::enableAdvancedLighting", false))
      mPixelShaderVersion = 3.0f;
      
   mSupportsAnisotropic = mCardProfiler->queryProfile( "GL::suppAnisotropic" );
}

GFXGLDevice::GFXGLDevice(U32 adapterIndex) :
   mAdapterIndex(adapterIndex),
   mCurrentVB(NULL),
   mCurrentPB(NULL),
   m_mCurrentWorld(true),
   m_mCurrentView(true),
   mContext(NULL),
   mPixelFormat(NULL),
   mPixelShaderVersion(0.0f),
   mMaxShaderTextures(2),
   mMaxFFTextures(2),
   mClip(0, 0, 0, 0)
{
   loadGLCore();

   GFXGLEnumTranslate::init();

   GFXVertexColor::setSwizzle( &Swizzles::rgba );
   mDeviceSwizzle32 = &Swizzles::bgra;
   mDeviceSwizzle24 = &Swizzles::bgr;

   mTextureManager = new GFXGLTextureManager();
   gScreenShot = new ScreenShot();

   for(U32 i = 0; i < TEXTURE_STAGE_COUNT; i++)
      mActiveTextureType[i] = GL_ZERO;
}

GFXGLDevice::~GFXGLDevice()
{
   mCurrentStateBlock = NULL;
   mCurrentPB = NULL;
   mCurrentVB = NULL;
   for(U32 i = 0; i < mVolatileVBs.size(); i++)
      mVolatileVBs[i] = NULL;
   for(U32 i = 0; i < mVolatilePBs.size(); i++)
      mVolatilePBs[i] = NULL;

   GFXResource* walk = mResourceListHead;
   while(walk)
   {
      walk->zombify();
      walk = walk->getNextResource();
   }
      
   if( mCardProfiler )
      SAFE_DELETE( mCardProfiler );

   SAFE_DELETE( gScreenShot );
}

void GFXGLDevice::zombify()
{
   mTextureManager->zombify();
   if(mCurrentVB)
      mCurrentVB->finish();
   if(mCurrentPB)
      mCurrentPB->finish();
   //mVolatileVBs.clear();
   //mVolatilePBs.clear();
   GFXResource* walk = mResourceListHead;
   while(walk)
   {
      walk->zombify();
      walk = walk->getNextResource();
   }
}

void GFXGLDevice::resurrect()
{
   GFXResource* walk = mResourceListHead;
   while(walk)
   {
      walk->resurrect();
      walk = walk->getNextResource();
   }
   if(mCurrentVB)
      mCurrentVB->prepare();
   if(mCurrentPB)
      mCurrentPB->prepare();
   mTextureManager->resurrect();
}

GFXVertexBuffer* GFXGLDevice::findVolatileVBO(U32 numVerts, const GFXVertexFormat *vertexFormat, U32 vertSize)
{
   for(U32 i = 0; i < mVolatileVBs.size(); i++)
      if (  mVolatileVBs[i]->mNumVerts >= numVerts &&
            mVolatileVBs[i]->mVertexFormat.isEqual( *vertexFormat ) &&
            mVolatileVBs[i]->mVertexSize == vertSize &&
            mVolatileVBs[i]->getRefCount() == 1 )
         return mVolatileVBs[i];

   // No existing VB, so create one
   StrongRefPtr<GFXGLVertexBuffer> buf(new GFXGLVertexBuffer(GFX, numVerts, vertexFormat, vertSize, GFXBufferTypeVolatile));
   buf->registerResourceWithDevice(this);
   mVolatileVBs.push_back(buf);
   return buf.getPointer();
}

GFXPrimitiveBuffer* GFXGLDevice::findVolatilePBO(U32 numIndices, U32 numPrimitives)
{
   for(U32 i = 0; i < mVolatilePBs.size(); i++)
      if((mVolatilePBs[i]->mIndexCount >= numIndices) && (mVolatilePBs[i]->getRefCount() == 1))
         return mVolatilePBs[i];
   
   // No existing PB, so create one
   StrongRefPtr<GFXGLPrimitiveBuffer> buf(new GFXGLPrimitiveBuffer(GFX, numIndices, numPrimitives, GFXBufferTypeVolatile));
   buf->registerResourceWithDevice(this);
   mVolatilePBs.push_back(buf);
   return buf.getPointer();
}

GFXVertexBuffer *GFXGLDevice::allocVertexBuffer(   U32 numVerts, 
                                                   const GFXVertexFormat *vertexFormat, 
                                                   U32 vertSize, 
                                                   GFXBufferType bufferType ) 
{
   if(bufferType == GFXBufferTypeVolatile)
      return findVolatileVBO(numVerts, vertexFormat, vertSize);
         
   GFXGLVertexBuffer* buf = new GFXGLVertexBuffer( GFX, numVerts, vertexFormat, vertSize, bufferType );
   buf->registerResourceWithDevice(this);
   return buf;
}

GFXPrimitiveBuffer *GFXGLDevice::allocPrimitiveBuffer( U32 numIndices, U32 numPrimitives, GFXBufferType bufferType ) 
{
   if(bufferType == GFXBufferTypeVolatile)
      return findVolatilePBO(numIndices, numPrimitives);
         
   GFXGLPrimitiveBuffer* buf = new GFXGLPrimitiveBuffer(GFX, numIndices, numPrimitives, bufferType);
   buf->registerResourceWithDevice(this);
   return buf;
}

void GFXGLDevice::setVertexStream( U32 stream, GFXVertexBuffer *buffer )
{
   AssertFatal( stream == 0, "GFXGLDevice::setVertexStream - We don't support multiple vertex streams!" );

   // Reset the state the old VB required, then set the state the new VB requires.
   if ( mCurrentVB ) 
      mCurrentVB->finish();

   mCurrentVB = static_cast<GFXGLVertexBuffer*>( buffer );
   if ( mCurrentVB )
      mCurrentVB->prepare();
}

void GFXGLDevice::setVertexStreamFrequency( U32 stream, U32 frequency )
{
   // We don't support vertex stream frequency or mesh instancing in OGL yet.
}

GFXCubemap* GFXGLDevice::createCubemap()
{ 
   GFXGLCubemap* cube = new GFXGLCubemap();
   cube->registerResourceWithDevice(this);
   return cube; 
};

void GFXGLDevice::endSceneInternal() 
{
   // nothing to do for opengl
   mCanCurrentlyRender = false;
}

void GFXGLDevice::clear(U32 flags, ColorI color, F32 z, U32 stencil)
{
   // Make sure we have flushed our render target state.
   _updateRenderTargets();
   
   bool zwrite = true;
   if (mCurrentGLStateBlock)
   {
      zwrite = mCurrentGLStateBlock->getDesc().zWriteEnable;
   }   
   
   glDepthMask(true);
   ColorF c = color;
   glClearColor(c.red, c.green, c.blue, c.alpha);
   glClearDepth(z);
   glClearStencil(stencil);

   GLbitfield clearflags = 0;
   clearflags |= (flags & GFXClearTarget)   ? GL_COLOR_BUFFER_BIT : 0;
   clearflags |= (flags & GFXClearZBuffer)  ? GL_DEPTH_BUFFER_BIT : 0;
   clearflags |= (flags & GFXClearStencil)  ? GL_STENCIL_BUFFER_BIT : 0;

   glClear(clearflags);
   
   if(!zwrite)
      glDepthMask(false);
}

// Given a primitive type and a number of primitives, return the number of indexes/vertexes used.
GLsizei GFXGLDevice::primCountToIndexCount(GFXPrimitiveType primType, U32 primitiveCount)
{
   switch (primType)
   {
      case GFXPointList :
         return primitiveCount;
         break;
      case GFXLineList :
         return primitiveCount * 2;
         break;
      case GFXLineStrip :
         return primitiveCount + 1;
         break;
      case GFXTriangleList :
         return primitiveCount * 3;
         break;
      case GFXTriangleStrip :
         return 2 + primitiveCount;
         break;
      case GFXTriangleFan :
         return 2 + primitiveCount;
         break;
      default:
         AssertFatal(false, "GFXGLDevice::primCountToIndexCount - unrecognized prim type");
         break;
   }
   
   return 0;
}

inline void GFXGLDevice::preDrawPrimitive()
{
   if( mStateDirty )
   {
      updateStates();
   }
   
   if(mCurrentShaderConstBuffer)
      setShaderConstBufferInternal(mCurrentShaderConstBuffer);
}

inline void GFXGLDevice::postDrawPrimitive(U32 primitiveCount)
{
   mDeviceStatistics.mDrawCalls++;
   mDeviceStatistics.mPolyCount += primitiveCount;
}

void GFXGLDevice::drawPrimitive( GFXPrimitiveType primType, U32 vertexStart, U32 primitiveCount ) 
{
   preDrawPrimitive();
   
   // There are some odd performance issues if a buffer is bound to GL_ELEMENT_ARRAY_BUFFER when glDrawArrays is called.  Unbinding the buffer
   // improves performance by 10%.
   if(mCurrentPB)
      mCurrentPB->finish();

   glDrawArrays(GFXGLPrimType[primType], vertexStart, primCountToIndexCount(primType, primitiveCount));
   
   if(mCurrentPB)
      mCurrentPB->prepare();

   postDrawPrimitive(primitiveCount);
}

void GFXGLDevice::drawIndexedPrimitive(   GFXPrimitiveType primType, 
                                          U32 startVertex, 
                                          U32 minIndex, 
                                          U32 numVerts, 
                                          U32 startIndex, 
                                          U32 primitiveCount )
{
   AssertFatal( startVertex == 0, "GFXGLDevice::drawIndexedPrimitive() - Non-zero startVertex unsupported!" );

   preDrawPrimitive();

   U16* buf = (U16*)static_cast<GFXGLPrimitiveBuffer*>(mCurrentPrimitiveBuffer.getPointer())->getBuffer() + startIndex;

   glDrawElements(GFXGLPrimType[primType], primCountToIndexCount(primType, primitiveCount), GL_UNSIGNED_SHORT, buf);

   postDrawPrimitive(primitiveCount);
}

void GFXGLDevice::setPB(GFXGLPrimitiveBuffer* pb)
{
   if(mCurrentPB)
      mCurrentPB->finish();
   mCurrentPB = pb;
}

void GFXGLDevice::setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable)
{
   if(!lightEnable)
   {
      glDisable(GL_LIGHT0 + lightStage);
      return;
   }

   if(light.mType == GFXLightInfo::Ambient)
   {
      AssertFatal(false, "Instead of setting an ambient light you should set the global ambient color.");
      return;
   }

   GLenum lightEnum = GL_LIGHT0 + lightStage;
   glLightfv(lightEnum, GL_AMBIENT, (GLfloat*)&light.mAmbient);
   glLightfv(lightEnum, GL_DIFFUSE, (GLfloat*)&light.mColor);
   glLightfv(lightEnum, GL_SPECULAR, (GLfloat*)&light.mColor);

   F32 pos[4];

   if(light.mType != GFXLightInfo::Vector)
   {
      dMemcpy(pos, &light.mPos, sizeof(light.mPos));
      pos[3] = 1.0;
   }
   else
   {
      dMemcpy(pos, &light.mDirection, sizeof(light.mDirection));
      pos[3] = 0.0;
   }
   // Harcoded attenuation
   glLightf(lightEnum, GL_CONSTANT_ATTENUATION, 1.0f);
   glLightf(lightEnum, GL_LINEAR_ATTENUATION, 0.1f);
   glLightf(lightEnum, GL_QUADRATIC_ATTENUATION, 0.0f);

   glLightfv(lightEnum, GL_POSITION, (GLfloat*)&pos);
   glEnable(lightEnum);
}

void GFXGLDevice::setLightMaterialInternal(const GFXLightMaterial mat)
{
   // CodeReview - Setting these for front and back is unnecessary.  We should consider
   // checking what faces we're culling and setting this only for the unculled faces.
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (GLfloat*)&mat.ambient);
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (GLfloat*)&mat.diffuse);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (GLfloat*)&mat.specular);
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, (GLfloat*)&mat.emissive);
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.shininess);
}

void GFXGLDevice::setGlobalAmbientInternal(ColorF color)
{
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (GLfloat*)&color);
}

void GFXGLDevice::setTextureInternal(U32 textureUnit, const GFXTextureObject*texture)
{
   const GFXGLTextureObject *tex = static_cast<const GFXGLTextureObject*>(texture);
   glActiveTexture(GL_TEXTURE0 + textureUnit);
   if (tex)
   {
      // GFXGLTextureObject::bind also handles applying the current sampler state.
      if(mActiveTextureType[textureUnit] != tex->getBinding() && mActiveTextureType[textureUnit] != GL_ZERO)
      {
         glBindTexture(mActiveTextureType[textureUnit], 0);
         glDisable(mActiveTextureType[textureUnit]);
      }
      mActiveTextureType[textureUnit] = tex->getBinding();
      tex->bind(textureUnit);
   } 
   else if(mActiveTextureType[textureUnit] != GL_ZERO)
   {
      glBindTexture(mActiveTextureType[textureUnit], 0);
      glDisable(mActiveTextureType[textureUnit]);
      mActiveTextureType[textureUnit] = GL_ZERO;
   }
   
   glActiveTexture(GL_TEXTURE0);
}

void GFXGLDevice::setCubemapInternal(U32 textureUnit, const GFXGLCubemap* texture)
{
   glActiveTexture(GL_TEXTURE0 + textureUnit);
   if(texture)
   {
      if(mActiveTextureType[textureUnit] != GL_TEXTURE_CUBE_MAP && mActiveTextureType[textureUnit] != GL_ZERO)
      {
         glBindTexture(mActiveTextureType[textureUnit], 0);
         glDisable(mActiveTextureType[textureUnit]);
      }
      mActiveTextureType[textureUnit] = GL_TEXTURE_CUBE_MAP;
      texture->bind(textureUnit);
   }
   else if(mActiveTextureType[textureUnit] != GL_ZERO)
   {
      glBindTexture(mActiveTextureType[textureUnit], 0);
      glDisable(mActiveTextureType[textureUnit]);
      mActiveTextureType[textureUnit] = GL_ZERO;
   }

   glActiveTexture(GL_TEXTURE0);
}

void GFXGLDevice::setMatrix( GFXMatrixType mtype, const MatrixF &mat )
{
   MatrixF modelview;
   switch (mtype)
   {
      case GFXMatrixWorld :
         {
            glMatrixMode(GL_MODELVIEW);
            m_mCurrentWorld = mat;
            modelview = m_mCurrentWorld;
            modelview *= m_mCurrentView;
            modelview.transpose();
            glLoadMatrixf((F32*) modelview);
         }
         break;
      case GFXMatrixView :
         {
            glMatrixMode(GL_MODELVIEW);
            m_mCurrentView = mat;
            modelview = m_mCurrentView;
            modelview *= m_mCurrentWorld;
            modelview.transpose();
            glLoadMatrixf((F32*) modelview);
         }
         break;
      case GFXMatrixProjection :
         {        
            glMatrixMode(GL_PROJECTION);
            MatrixF t(mat);
            t.transpose();
            glLoadMatrixf((F32*) t);
            glMatrixMode(GL_MODELVIEW);
         }
         break;
      // CodeReview - Add support for texture transform matrix types
      default:
         AssertFatal(false, "GFXGLDevice::setMatrix - Unknown matrix mode!");
         return;
   }
}

void GFXGLDevice::setClipRect( const RectI &inRect )
{
   AssertFatal(mCurrentRT.isValid(), "GFXGLDevice::setClipRect - must have a render target set to do any rendering operations!");

   // Clip the rect against the renderable size.
   Point2I size = mCurrentRT->getSize();
   RectI maxRect(Point2I(0,0), size);
   mClip = inRect;
   mClip.intersect(maxRect);

   // Create projection matrix.  See http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/ortho.html
   const F32 left = mClip.point.x;
   const F32 right = mClip.point.x + mClip.extent.x;
   const F32 bottom = mClip.extent.y;
   const F32 top = 0.0f;
   const F32 near = 0.0f;
   const F32 far = 1.0f;
   
   const F32 tx = -(right + left)/(right - left);
   const F32 ty = -(top + bottom)/(top - bottom);
   const F32 tz = -(far + near)/(far - near);
   
   static Point4F pt;
   pt.set(2.0f / (right - left), 0.0f, 0.0f, 0.0f);
   mProjectionMatrix.setColumn(0, pt);
   
   pt.set(0.0f, 2.0f/(top - bottom), 0.0f, 0.0f);
   mProjectionMatrix.setColumn(1, pt);
   
   pt.set(0.0f, 0.0f, -2.0f/(far - near), 0.0f);
   mProjectionMatrix.setColumn(2, pt);
   
   pt.set(tx, ty, tz, 1.0f);
   mProjectionMatrix.setColumn(3, pt);
   
   // Translate projection matrix.
   static MatrixF translate(true);
   pt.set(0.0f, -mClip.point.y, 0.0f, 1.0f);
   translate.setColumn(3, pt);
   
   mProjectionMatrix *= translate;
   
   setMatrix(GFXMatrixProjection, mProjectionMatrix);
   
   MatrixF mTempMatrix(true);
   setViewMatrix( mTempMatrix );
   setWorldMatrix( mTempMatrix );

   // Set the viewport to the clip rect (with y flip)
   RectI viewport(mClip.point.x, size.y - (mClip.point.y + mClip.extent.y), mClip.extent.x, mClip.extent.y);
   setViewport(viewport);
}

/// Creates a state block object based on the desc passed in.  This object
/// represents an immutable state.
GFXStateBlockRef GFXGLDevice::createStateBlockInternal(const GFXStateBlockDesc& desc)
{
   return GFXStateBlockRef(new GFXGLStateBlock(desc));
}

/// Activates a stateblock
void GFXGLDevice::setStateBlockInternal(GFXStateBlock* block, bool force)
{
   AssertFatal(dynamic_cast<GFXGLStateBlock*>(block), "GFXGLDevice::setStateBlockInternal - Incorrect stateblock type for this device!");
   GFXGLStateBlock* glBlock = static_cast<GFXGLStateBlock*>(block);
   GFXGLStateBlock* glCurrent = static_cast<GFXGLStateBlock*>(mCurrentStateBlock.getPointer());
   if (force)
      glCurrent = NULL;
      
   glBlock->activate(glCurrent); // Doesn't use current yet.
   mCurrentGLStateBlock = glBlock;
}

//------------------------------------------------------------------------------

GFXTextureTarget * GFXGLDevice::allocRenderToTextureTarget()
{
   GFXGLTextureTarget *targ = new GFXGLTextureTarget();
   targ->registerResourceWithDevice(this);
   return targ;
}

GFXFence * GFXGLDevice::createFence()
{
   GFXFence* fence = _createPlatformSpecificFence();
   if(!fence)
      fence = new GFXGeneralFence( this );
      
   fence->registerResourceWithDevice(this);
   return fence;
}

GFXOcclusionQuery* GFXGLDevice::createOcclusionQuery()
{   
   GFXOcclusionQuery *query = new GFXGLOcclusionQuery( this );
   query->registerResourceWithDevice(this);
   return query;
}

void GFXGLDevice::setupGenericShaders( GenericShaderType type ) 
{
   TORQUE_UNUSED(type);
   // We have FF support, use that.
   disableShaders();
}

GFXShader* GFXGLDevice::createShader()
{
   GFXGLShader* shader = new GFXGLShader();
   shader->registerResourceWithDevice( this );
   return shader;
}

void GFXGLDevice::setShader( GFXShader *shader )
{
   if ( shader )
   {
      GFXGLShader *glShader = static_cast<GFXGLShader*>( shader );
      glShader->useProgram();
   }
   else
      glUseProgram(0);
}

void GFXGLDevice::disableShaders()
{
   setShader(NULL);
   setShaderConstBuffer( NULL );
}

void GFXGLDevice::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
   static_cast<GFXGLShaderConstBuffer*>(buffer)->activate();
}

U32 GFXGLDevice::getNumSamplers() const
{
   return mPixelShaderVersion > 0.001f ? mMaxShaderTextures : mMaxFFTextures;
}

U32 GFXGLDevice::getNumRenderTargets() const 
{ 
   return 1; 
}

void GFXGLDevice::_updateRenderTargets()
{
   if ( mRTDirty || mCurrentRT->isPendingState() )
   {
      if ( mRTDeactivate )
      {
         mRTDeactivate->deactivate();
         mRTDeactivate = NULL;   
      }
      
      // NOTE: The render target changes is not really accurate
      // as the GFXTextureTarget supports MRT internally.  So when
      // we activate a GFXTarget it could result in multiple calls
      // to SetRenderTarget on the actual device.
      mDeviceStatistics.mRenderTargetChanges++;

      GFXGLTextureTarget *tex = dynamic_cast<GFXGLTextureTarget*>( mCurrentRT.getPointer() );
      if ( tex )
      {
         tex->applyState();
         tex->makeActive();
      }
      else
      {
         GFXGLWindowTarget *win = dynamic_cast<GFXGLWindowTarget*>( mCurrentRT.getPointer() );
         AssertFatal( win != NULL, 
                     "GFXGLDevice::_updateRenderTargets() - invalid target subclass passed!" );
         
         win->makeActive();
         
         if( win->mContext != static_cast<GFXGLDevice*>(GFX)->mContext )
         {
            mRTDirty = false;
            GFX->updateStates(true);
         }
      }
      
      mRTDirty = false;
   }
   
   if ( mViewportDirty )
   {
      glViewport( mViewport.point.x, mViewport.point.y, mViewport.extent.x, mViewport.extent.y ); 
      mViewportDirty = false;
   }
}

GFXFormat GFXGLDevice::selectSupportedFormat(   GFXTextureProfile* profile, 
                                                const Vector<GFXFormat>& formats, 
                                                bool texture, 
                                                bool mustblend,
                                                bool mustfilter )
{
   for(U32 i = 0; i < formats.size(); i++)
   {
      // Single channel textures are not supported by FBOs.
      if(profile->testFlag(GFXTextureProfile::RenderTarget) && (formats[i] == GFXFormatA8 || formats[i] == GFXFormatL8 || formats[i] == GFXFormatL16))
         continue;
      if(GFXGLTextureInternalFormat[formats[i]] == GL_ZERO)
         continue;
      
      return formats[i];
   }
   
   return GFXFormatR8G8B8A8;
}

//
// Register this device with GFXInit
//
class GFXGLRegisterDevice
{
public:
   GFXGLRegisterDevice()
   {
      GFXInit::getRegisterDeviceSignal().notify(&GFXGLDevice::enumerateAdapters);
   }
};

static GFXGLRegisterDevice pGLRegisterDevice;

ConsoleFunction(cycleResources, void, 1, 1, "")
{
   static_cast<GFXGLDevice*>(GFX)->zombify();
   static_cast<GFXGLDevice*>(GFX)->resurrect();
}
