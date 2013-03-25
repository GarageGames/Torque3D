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

#ifndef _TERRDATA_H_
#define _TERRDATA_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _RENDERPASSMANAGER_H_
#include "renderInstance/renderPassManager.h"
#endif
#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif
#ifndef _TERRFILE_H_
#include "terrain/terrFile.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif



class GBitmap;
class TerrainBlock;
class TerrCell;
class PhysicsBody;
class TerrainCellMaterial;

class TerrainBlock : public SceneObject
{
   typedef SceneObject Parent;

   friend class TerrainEditor;
   friend class TerrainCellMaterial;

protected:

   enum
   {
      TransformMask        = Parent::NextFreeMask,
      FileMask             = Parent::NextFreeMask << 1,
      SizeMask             = Parent::NextFreeMask << 2,
      MaterialMask         = Parent::NextFreeMask << 3,
      HeightMapChangeMask  = Parent::NextFreeMask << 4,
      MiscMask             = Parent::NextFreeMask << 5,

      NextFreeMask = Parent::NextFreeMask << 6,
   };

   Box3F mBounds;

   ///
   GBitmap *mLightMap;

   /// The lightmap dimensions in pixels.
   U32 mLightMapSize;

   /// The lightmap texture.
   GFXTexHandle mLightMapTex;

   /// The terrain data file.
   Resource<TerrainFile> mFile;

   /// The TerrainFile CRC sent from the server.
   U32 mCRC;

   ///
   FileName mTerrFileName;
   
   /// The maximum detail distance found in the material list.
   F32 mMaxDetailDistance;

   ///
   Vector<GFXTexHandle> mBaseTextures;

   /// 
   GFXTexHandle mLayerTex;

   /// The shader used to generate the base texture map.
   GFXShaderRef mBaseShader;

   ///
   GFXStateBlockRef mBaseShaderSB;

   ///
   GFXShaderConstBufferRef mBaseShaderConsts;

   ///
   GFXShaderConstHandle *mBaseTexScaleConst;
   GFXShaderConstHandle *mBaseTexIdConst;
   GFXShaderConstHandle *mBaseLayerSizeConst;

   ///
   GFXTextureTargetRef mBaseTarget;

   /// The base texture.
   GFXTexHandle mBaseTex;

   ///
   bool mDetailsDirty;

   ///
   bool mLayerTexDirty;

   /// The desired size for the base texture.
   U32 mBaseTexSize;

   ///
   TerrCell *mCell;

   /// The shared base material which is used to render
   /// cells that are outside the detail map range.
   TerrainCellMaterial *mBaseMaterial;
 
   /// A dummy material only used for shadow
   /// material generation.
   BaseMatInstance *mDefaultMatInst;

   F32 mSquareSize;

   PhysicsBody *mPhysicsRep;

   U32 mScreenError;

   /// The shared primitive buffer used in rendering.
   GFXPrimitiveBufferHandle mPrimBuffer;

   /// The cells used in the last render pass
   /// when doing debug rendering.
   /// @see _renderDebug
   Vector<TerrCell*> mDebugCells;

   /// Set to enable debug rendering of the terrain.  It
   /// is exposed to the console via $terrain::debugRender.
   static bool smDebugRender;

	/// Allows the terrain to cast shadows onto itself and other objects.
	bool mCastShadows;

   /// A global LOD scale used to tweak the default
   /// terrain screen error value.
   static F32 smLODScale;

   /// A global detail scale used to tweak the 
   /// material detail distances.
   static F32 smDetailScale;

   /// True if the zoning needs to be recalculated for the terrain.
   bool mZoningDirty;

   String _getBaseTexCacheFileName() const;

   void _rebuildQuadtree();

   void _updatePhysics();

   void _renderBlock( SceneRenderState *state );
   void _renderDebug( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

   /// The callback used to get texture events.
   /// @see GFXTextureManager::addEventDelegate
   void _onTextureEvent( GFXTexCallbackCode code );

   /// Used to release terrain materials when
   /// the material manager flushes them.
   /// @see MaterialManager::getFlushSignal
   void _onFlushMaterials();

   /// 
   bool _initBaseShader();

   ///
   void _updateMaterials();

   /// 
   void _updateBaseTexture( bool writeToCache );

   void _updateLayerTexture();

   void _updateBounds();

   void _onZoningChanged( SceneZoneSpaceManager *zoneManager );

   void _updateZoning();

   // Protected fields
   static bool _setTerrainFile( void *obj, const char *index, const char *data );
   static bool _setSquareSize( void *obj, const char *index, const char *data );
   static bool _setBaseTexSize( void *obj, const char *index, const char *data );
   static bool _setLightMapSize( void *obj, const char *index, const char *data );

public:

   enum 
   {
      LightmapUpdate    = BIT(0),
      HeightmapUpdate   = BIT(1),
      LayersUpdate      = BIT(2),
      EmptyUpdate       = BIT(3)
   };

   static Signal<void(U32,TerrainBlock*,const Point2I& ,const Point2I&)> smUpdateSignal;

   ///
   bool import(   const GBitmap &heightMap, 
                  F32 heightScale, 
                  F32 metersPerPixel,
                  const Vector<U8> &layerMap, 
                  const Vector<String> &materials,
                  bool flipYAxis = true );

#ifdef TORQUE_TOOLS
   bool exportHeightMap( const UTF8 *filePath, const String &format ) const;
   bool exportLayerMaps( const UTF8 *filePrefix, const String &format ) const;
#endif

public:

   TerrainBlock();
   virtual ~TerrainBlock();

   U32 getCRC() const { return(mCRC); }

   Resource<TerrainFile> getFile() const { return mFile; };

   bool onAdd();
   void onRemove();

   void onEditorEnable();
   void onEditorDisable();

   /// Adds a new material as the top layer or 
   /// inserts it at the specified index.
   void addMaterial( const String &name, U32 insertAt = -1 );

   /// Removes the material at the index.
   void removeMaterial( U32 index );

   /// Updates the material at the index.
   void updateMaterial( U32 index, const String &name );

   /// Deletes all the materials on the terrain.
   void deleteAllMaterials();

   //void setMaterialName( U32 index, const String &name );

   /// Accessors and mutators for TerrainMaterialUndoAction.
   /// @{
   const Vector<TerrainMaterial*>& getMaterials() const { return mFile->mMaterials; }   
   const Vector<U8>& getLayerMap() const { return mFile->mLayerMap; }
   void setMaterials( const Vector<TerrainMaterial*> &materials ) { mFile->mMaterials = materials; }
   void setLayerMap( const Vector<U8> &layers ) { mFile->mLayerMap = layers; }
   /// @}

   TerrainMaterial* getMaterial( U32 index ) const;

   const char* getMaterialName( U32 index ) const;

   U32 getMaterialCount() const;

   //BaseMatInstance* getMaterialInst( U32 x, U32 y );

   void setHeight( const Point2I &pos, F32 height );
   F32 getHeight( const Point2I &pos );

   // Performs an update to the selected range of the terrain
   // grid including the collision and rendering structures.
   void updateGrid(  const Point2I &minPt, 
                     const Point2I &maxPt, 
                     bool updateClient = false );

   void updateGridMaterials( const Point2I &minPt, const Point2I &maxPt );

   Point2I getGridPos( const Point3F &worldPos ) const;
   
   /// This returns true and the terrain z height for
   /// a 2d position in the terrains object space.
   ///
   /// If the terrain at that point is within an empty block
   /// or the 2d position is outside of the terrain area then
   /// it returns false.
   ///
   bool getHeight( const Point2F &pos, F32 *height ) const;

   void getMinMaxHeight( F32 *minHeight, F32 *maxHeight ) const;

   /// This returns true and the terrain normal for a 
   /// 2d position in the terrains object space.
   ///
   /// If the terrain at that point is within an empty block
   /// or the 2d position is outside of the terrain area then
   /// it returns false.
   ///
   bool getNormal(   const Point2F &pos, 
                     Point3F *normal, 
                     bool normalize = true, 
                     bool skipEmpty = true ) const;

   /// This returns true and the smoothed terrain normal 
   // for a 2d position in the terrains object space.
   ///
   /// If the terrain at that point is within an empty block
   /// or the 2d position is outside of the terrain area then
   /// it returns false.
   ///
   bool getSmoothNormal(   const Point2F &pos, 
                           Point3F *normal, 
                           bool normalize = true,
                           bool skipEmpty = true ) const;

   /// This returns true and the terrain normal and z height
   /// for a 2d position in the terrains object space.
   ///
   /// If the terrain at that point is within an empty block
   /// or the 2d position is outside of the terrain area then
   /// it returns false.
   ///
   bool getNormalAndHeight(   const Point2F &pos, 
                              Point3F *normal, 
                              F32 *height, 
                              bool normalize = true ) const;

   /// This returns true and the terrain normal, z height, and
   /// material name for a 2d position in the terrains object
   /// space.
   ///
   /// If the terrain at that point is within an empty block
   /// or the 2d position is outside of the terrain area then
   /// it returns false.
   ///
   bool getNormalHeightMaterial( const Point2F &pos, 
                                 Point3F *normal, 
                                 F32 *height, 
                                 StringTableEntry &matName ) const;

   // only the editor currently uses this method - should always be using a ray to collide with
   bool collideBox( const Point3F &start, const Point3F &end, RayInfo* info )
   {
      return castRay( start, end, info );
   }

   ///
   void setLightMap( GBitmap *newLightMap );

   /// Fills the lightmap with white.
   void clearLightMap();

   /// Retuns the dimensions of the light map.
   U32 getLightMapSize() const { return mLightMapSize; }

   const GBitmap* getLightMap() const { return mLightMap; }

   GBitmap* getLightMap() { return mLightMap; }

   ///
   GFXTextureObject* getLightMapTex();

public:

   bool setFile( const FileName& terrFileName );

   void setFile( Resource<TerrainFile> file );

   bool save(const char* filename);

   F32 getSquareSize() const { return mSquareSize; }

   /// Returns the dimensions of the terrain in world space.
   F32 getWorldBlockSize() const { return mSquareSize * (F32)mFile->mSize; }

   /// Retuns the dimensions of the terrain in samples.
   U32 getBlockSize() const { return mFile->mSize; }

   U32 getScreenError() const { return smLODScale * mScreenError; }

   // SceneObject
   void setTransform( const MatrixF &mat );
   void setScale( const VectorF &scale );

   void prepRenderImage  ( SceneRenderState* state );

   void buildConvex(const Box3F& box,Convex* convex);
   bool buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere);
   bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
   bool castRayI(const Point3F &start, const Point3F &end, RayInfo* info, bool emptyCollide);
   
   bool castRayBlock(   const Point3F &pStart, 
                        const Point3F &pEnd, 
                        const Point2I &blockPos, 
      					   U32 level, 
                        F32 invDeltaX, 
                        F32 invDeltaY, 
                        F32 startT, 
                        F32 endT, 
                        RayInfo *info, 
                        bool collideEmpty );

   const FileName& getTerrainFile() const { return mTerrFileName; }

   void postLight(Vector<TerrainBlock *> &terrBlocks) {};


   DECLARE_CONOBJECT(TerrainBlock);
   static void initPersistFields();
   U32 packUpdate   (NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn,           BitStream *stream);
   void inspectPostApply();
};

#endif // _TERRDATA_H_
