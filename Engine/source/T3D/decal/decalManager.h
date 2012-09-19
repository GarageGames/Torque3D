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

#ifndef _DECALMANAGER_H_
#define _DECALMANAGER_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif

#ifndef _MATHUTIL_FRUSTUM_H_
#include "math/util/frustum.h"
#endif

#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif

#ifndef _CLIPPEDPOLYLIST_H_
#include "collision/clippedPolyList.h"
#endif

#ifndef _DECALDATAFILE_H_
#include "decalDataFile.h"
#endif

#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif

#ifndef _DECALINSTANCE_H_
#include "decalInstance.h"
#endif

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif

#ifndef _DATACHUNKER_H_
#include "core/dataChunker.h"
#endif


//#define DECALMANAGER_DEBUG


struct ObjectRenderInst;
class Material;


enum DecalFlags 
{
   PermanentDecal = 1 << 0,
   SaveDecal      = 1 << 1,
   ClipDecal      = 1 << 2,
   CustomDecal    = 1 << 3 // DecalManager will not attempt to clip or remove this decal
                           // it is managed by someone else.
};


/// Manage decals in the scene.
class DecalManager : public SceneObject
{
   public:
      
      typedef SceneObject Parent;

      // [rene, 11-Mar-11] This vector is very poorly managed; the logic is spread all over the place
      Vector<DecalInstance *> mDecalInstanceVec;

   protected:
      
      /// The clipper we keep around between decal updates
      /// to avoid excessive memory allocations.
      ClippedPolyList mClipper;

      Vector<DecalInstance*> mDecalQueue;

      StringTableEntry mDataFileName;
      Resource<DecalDataFile> mData;
      
      Signal< void() > mClearDataSignal;
      
      Vector< GFXVertexBufferHandle<DecalVertex>* > mVBs;
      Vector< GFXPrimitiveBufferHandle* > mPBs;

      Vector< GFXVertexBufferHandle<DecalVertex>* > mVBPool;
      Vector< GFXPrimitiveBufferHandle* > mPBPool;

      FreeListChunkerUntyped *mChunkers[3];

      #ifdef DECALMANAGER_DEBUG
      Vector<PlaneF> mDebugPlanes;
      #endif

      bool mDirty;

      struct DecalBatch
      {
         U32 startDecal;
         U32 decalCount;
         U32 iCount;
         U32 vCount;
         U8 priority;
         Material *mat;
         BaseMatInstance *matInst;
         bool dynamic;
      };

      /// Whether to render visualizations for debugging in the editor.
      static bool smDebugRender;

      static bool smDecalsOn;
      static F32 smDecalLifeTimeScale;   
      static bool smPoolBuffers;
      static const U32 smMaxVerts;
      static const U32 smMaxIndices;

      // Assume that a class is already given for the object:
      //    Point with coordinates {float x, y;}
      //===================================================================

      // isLeft(): tests if a point is Left|On|Right of an infinite line.
      //    Input:  three points P0, P1, and P2
      //    Return: >0 for P2 left of the line through P0 and P1
      //            =0 for P2 on the line
      //            <0 for P2 right of the line
      //    See: the January 2001 Algorithm on Area of Triangles
      inline F32 isLeft( const Point3F &P0, const Point3F &P1, const Point3F &P2 )
      {
          return (P1.x - P0.x)*(P2.y - P0.y) - (P2.x - P0.x)*(P1.y - P0.y);
      }

      U32 _generateConvexHull( const Vector<Point3F> &points, Vector<Point3F> *outPoints );

      // Rendering
      void prepRenderImage( SceneRenderState *state );
      
      void _generateWindingOrder( const Point3F &cornerPoint, Vector<Point3F> *sortPoints );

      // Helpers for creating and deleting the vert and index arrays
      // held by DecalInstance.
      void _allocBuffers( DecalInstance *inst );
      void _freeBuffers( DecalInstance *inst );
      void _freePools();

      /// Returns index used to index into the correct sized FreeListChunker for
      /// allocating vertex and index arrays.
      S32 _getSizeClass( DecalInstance *inst ) const;

      // Hide this from Doxygen
      /// @cond
      bool _handleGFXEvent(GFXDevice::GFXDeviceEventType event);
      /// @endcond

      void _renderDecalSpheres( ObjectRenderInst* inst, SceneRenderState* state, BaseMatInstance* overrideMat );

      ///
      void _handleZoningChangedEvent( SceneZoneSpaceManager* zoneManager );

      bool _createDataFile();

      // SceneObject.
      virtual bool onSceneAdd();
      virtual void onSceneRemove();   public:

   public:

      DecalManager();
      ~DecalManager();

      /// @name Decal Addition
      /// @{

      /// Adds a decal using a normal and a rotation.
      ///
      /// @param pos The 3d position for the decal center.
      /// @param normal The decal up vector.
      /// @param rotAroundNormal The decal rotation around the normal.
      /// @param decalData The datablock which defines this decal.
      /// @param decalScale A scalar for adjusting the default size of the decal.
      /// @param decalTexIndex   Selects the texture coord index within the decal
      ///                        data to use.  If it is less than zero then a random
      ///                        coord is selected.
      /// @param flags  The decal flags. @see DecalFlags
      ///
      DecalInstance* addDecal( const Point3F &pos,
                               const Point3F &normal,
                               F32 rotAroundNormal,
                               DecalData *decalData,
                               F32 decalScale = 1.0f,
                               S32 decalTexIndex = 0,
                               U8 flags = 0x000 );

      /// Adds a decal using a normal and a tangent.
      ///
      /// @param pos The 3d position for the decal center.
      /// @param normal The decal up vector.
      /// @param tanget The decal right vector.
      /// @param decalData The datablock which defines this decal.
      /// @param decalScale A scalar for adjusting the default size of the decal.
      /// @param decalTexIndex   Selects the texture coord index within the decal
      ///                        data to use.  If it is less than zero then a random
      ///                        coord is selected.
      /// @param flags  The decal flags. @see DecalFlags
      ///
      DecalInstance* addDecal( const Point3F &pos,
                               const Point3F &normal,
                               const Point3F &tangent,
                               DecalData *decalData,
                               F32 decalScale = 1.0f,
                               S32 decalTexIndex = 0,
                               U8 flags = 0 );

      /// @}

      /// @name Decal Removal
      /// @{

      void removeDecal( DecalInstance *inst );

      /// @}

      DecalInstance* getDecal( S32 id );

      DecalInstance* getClosestDecal( const Point3F &pos );

      /// Return the closest DecalInstance hit by a ray.
      DecalInstance* raycast( const Point3F &start, const Point3F &end, bool savedDecalsOnly = true );

      //void dataDeleted( DecalData *data );

      void saveDecals( const UTF8 *fileName );
      bool loadDecals( const UTF8 *fileName );
      void clearData();

      /// Returns true if changes have been made since the last load/save
      bool isDirty() const { return mDirty; }

      bool clipDecal( DecalInstance *decal, Vector<Point3F> *edgeVerts = NULL, const Point2F *clipDepth = NULL );

      void notifyDecalModified( DecalInstance *inst );
      
      Signal< void() >& getClearDataSignal() { return mClearDataSignal; }

	   Resource<DecalDataFile> getDecalDataFile() { return mData; }

      // SimObject.
      DECLARE_CONOBJECT( DecalManager );
      static void consoleInit();
};

extern DecalManager* gDecalManager;

#endif // _DECALMANAGER_H_
