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

#include "interior/interior.h"
#include "core/bitVector.h"
#include "core/stream/stream.h"
#include "math/mathIO.h"
#include "gfx/bitmap/gBitmap.h"
#include "interior/interiorSubObject.h"
#include "console/console.h"
#include "core/frameAllocator.h"
#include "materials/materialList.h"

int QSORT_CALLBACK cmpU32(const void* p1, const void* p2)
{
   return S32(*((U32*)p1)) - S32(*((U32*)p2));
}

//------------------------------------------------------------------------------
//-------------------------------------- PERSISTENCE IMPLEMENTATION
//
U32 Interior::smFileVersion = 14;

bool Interior::read(Stream& stream)
{
   AssertFatal(stream.hasCapability(Stream::StreamRead), "Interior::read: non-read capable stream passed");
   AssertFatal(stream.getStatus() == Stream::Ok, "Interior::read: Error, stream in inconsistent state");

   S32 i;

   // Version this stream.  We only load stream of the current version
   U32 fileVersion;
   stream.read(&fileVersion);

   // We need to store the version in case there is any post processing that
   // needs to take place that is dependent on the file version
   mFileVersion = fileVersion;
   
   if (fileVersion > smFileVersion)
   {
      Con::errorf(ConsoleLogEntry::General, "Interior::read: incompatible file version found.");
      return false;
   }

   // Geometry factors...
   stream.read(&mDetailLevel);

   stream.read(&mMinPixels);
   mathRead(stream, &mBoundingBox);
   mathRead(stream, &mBoundingSphere);
   stream.read(&mHasAlarmState);
   stream.read(&mNumLightStateEntries);

   // Now read in our data vectors.
   S32 vectorSize;

   // mPlanes
   readPlaneVector(stream);

   // mPoints
   stream.read(&vectorSize);
   mPoints.setSize(vectorSize);
   for (i = 0; i < mPoints.size(); i++)
      mathRead(stream, &mPoints[i].point);

   // mPointVisibility
   stream.read(&vectorSize);
   mPointVisibility.setSize(vectorSize);
   stream.read(vectorSize, mPointVisibility.address());

   // mTexGenEQs
   stream.read(&vectorSize);
   mTexGenEQs.setSize(vectorSize);
   for(i = 0; i < mTexGenEQs.size(); i++)
   {
      mathRead(stream, &mTexGenEQs[i].planeX);
      mathRead(stream, &mTexGenEQs[i].planeY);
   }

   // mBSPNodes;
   stream.read(&vectorSize);
   mBSPNodes.setSize(vectorSize);
   for(i = 0; i < mBSPNodes.size(); i++)
   {
      stream.read(&mBSPNodes[i].planeIndex);

      if (fileVersion >= 14)
      {
         stream.read(&mBSPNodes[i].frontIndex);
         stream.read(&mBSPNodes[i].backIndex);
      }
      else
      {
         U16 frontIndex, backIndex;
         stream.read(&frontIndex);
         stream.read(&backIndex);

         mBSPNodes[i].frontIndex = U32(frontIndex);
         mBSPNodes[i].backIndex = U32(backIndex);
      }
   }

   // mBSPSolidLeaves
   stream.read(&vectorSize);
   mBSPSolidLeaves.setSize(vectorSize);
   for(i = 0; i < mBSPSolidLeaves.size(); i++)
   {
      stream.read(&mBSPSolidLeaves[i].surfaceIndex);
      stream.read(&mBSPSolidLeaves[i].surfaceCount);
   }

   // MaterialList
   if(mMaterialList != NULL)
      delete mMaterialList;
   mMaterialList = new MaterialList;
   mMaterialList->read(stream);


   // mWindings
   stream.read(&vectorSize);
   mWindings.setSize(vectorSize);
   for(i = 0; i < mWindings.size(); i++)
   {
      stream.read(&mWindings[i]);
   }

   // mWindingIndices
   stream.read(&vectorSize);
   mWindingIndices.setSize(vectorSize);
   for(i = 0; i < mWindingIndices.size(); i++)
   {
      stream.read(&mWindingIndices[i].windingStart);
      stream.read(&mWindingIndices[i].windingCount);
   }

   // mEdges
   if (fileVersion >= 12)
   {
      stream.read(&vectorSize);

      mEdges.setSize(vectorSize);
      for (i = 0; i < mEdges.size(); i++)
      {
         stream.read(&mEdges[i].vertexes[0]);
         stream.read(&mEdges[i].vertexes[1]);
         stream.read(&mEdges[i].faces[0]);
         stream.read(&mEdges[i].faces[1]);
      }
   }

   // mZones
   stream.read(&vectorSize);
   mZones.setSize(vectorSize);
   for(i = 0; i < mZones.size(); i++)
   {
      stream.read(&mZones[i].portalStart);
      stream.read(&mZones[i].portalCount);
      stream.read(&mZones[i].surfaceStart);
      stream.read(&mZones[i].surfaceCount);

      if (fileVersion >= 12)
      {
         stream.read(&mZones[i].staticMeshStart);
         stream.read(&mZones[i].staticMeshCount);
      }
      else
      {
         mZones[i].staticMeshStart = 0;
         mZones[i].staticMeshCount = 0;
      }

      stream.read(&mZones[i].flags);
      mZones[i].zoneId = 0;
   }

   // Zone surfaces
   stream.read(&vectorSize);
   mZoneSurfaces.setSize(vectorSize);
   for(i = 0; i < mZoneSurfaces.size(); i++)
      stream.read(&mZoneSurfaces[i]);

   // Zone static meshes
   if (fileVersion >= 12)
   {
      stream.read(&vectorSize);

      mZoneStaticMeshes.setSize(vectorSize);
      for (i = 0; i < mZoneStaticMeshes.size(); i++)
         stream.read(&mZoneStaticMeshes[i]);
   }

   //  mZonePortalList;
   stream.read(&vectorSize);
   mZonePortalList.setSize(vectorSize);
   for(i = 0; i < mZonePortalList.size(); i++)
      stream.read(&mZonePortalList[i]);

   // mPortals
   stream.read(&vectorSize);
   mPortals.setSize(vectorSize);
   for(i = 0; i < mPortals.size(); i++)
   {
      stream.read(&mPortals[i].planeIndex);
      stream.read(&mPortals[i].triFanCount);
      stream.read(&mPortals[i].triFanStart);
      stream.read(&mPortals[i].zoneFront);
      stream.read(&mPortals[i].zoneBack);
   }

   // mSurfaces
   stream.read(&vectorSize);
   mSurfaces.setSize(vectorSize);
   mLMTexGenEQs.setSize(vectorSize);

   // Couple of hoops to *attempt* to detect that we are loading
   // a TGE version 0 Interior and not a TGEA verison 0
   U32 surfacePos = stream.getPosition();
   bool tgeInterior = false;

   // First attempt to read this as though it isn't a TGE version 0 Interior
   for(i = 0; i < mSurfaces.size(); i++)
   {
      // If we end up reading any invalid data in this loop then odds
      // are that we are no longer correctly reading from the stream
      // and have gotten off because this is a TGE version 0 Interior

      Surface& surface = mSurfaces[i];

      if (readSurface(stream, surface, mLMTexGenEQs[i], false) == false)
      {
         tgeInterior = true;
         break;
      }
   }

   // If this is a version 0 Interior and we failed to read it as a
   // TGEA version 0 Interior then attempt to read it as a TGE version 0
   if (fileVersion == 0 && tgeInterior)
   {
      // Set our stream position back to the start of the surfaces
      stream.setPosition(surfacePos);

      // Try reading in the surfaces again
      for(i = 0; i < mSurfaces.size(); i++)
      {
         Surface& surface = mSurfaces[i];

         // If we fail on any of the surfaces then bail
         if (readSurface(stream, surface, mLMTexGenEQs[i], true) == false)
            return false;
      }
   }
   // If we failed to read but this isn't a version 0 Interior
   // then something has gone horribly wrong
   else if (fileVersion != 0 && tgeInterior)
      return false;

   // Edges
   if (fileVersion == 5)
   {
      stream.read(&vectorSize);
      mEdges.setSize(vectorSize);
      for (i = 0; i < mEdges.size(); i++)
      {
         stream.read(&mEdges[i].vertexes[0]);
         stream.read(&mEdges[i].vertexes[1]);
         U32 normals[2];
         stream.read(&normals[0]);
         stream.read(&normals[1]);

         if (fileVersion > 2) // version 3 is where surface id's get added
         {
            stream.read(&mEdges[i].faces[0]);
            stream.read(&mEdges[i].faces[1]);
         }
      }
   }

   // mNormals
   if (fileVersion == 5)
   {
      stream.read(&vectorSize);
      Vector<Point3F> normals;
      normals.setSize(vectorSize);
      for(i = 0; i < normals.size(); i++)
         mathRead(stream, &normals[i]);

      // mNormalIndices
      stream.read(&vectorSize);
      Vector<U16> normalIndices;
      normalIndices.setSize(vectorSize);
      for (i = 0; i < normalIndices.size(); i++)
         stream.read(&normalIndices[i]);
   }
   
   // NormalLMapIndices
   stream.read(&vectorSize);
   mNormalLMapIndices.setSize(vectorSize);
   for (U32 i = 0; i < mNormalLMapIndices.size(); i++)
   {
      if (fileVersion >= 13)
         stream.read(&mNormalLMapIndices[i]);
      else
      {
         U8 index = 0;
         stream.read(&index);

         mNormalLMapIndices[i] = (U32)index;
      }
   }

   // AlarmLMapIndices
   stream.read(&vectorSize);
   mAlarmLMapIndices.setSize(vectorSize);
   for (U32 i = 0; i < mAlarmLMapIndices.size(); i++)
   {
      if (fileVersion >= 13)
         stream.read(&mAlarmLMapIndices[i]);
      else
      {
         U8 index = 0;
         stream.read(&index);

         mAlarmLMapIndices[i] = (U32)index;
      }
   }

   // mNullSurfaces
   stream.read(&vectorSize);
   mNullSurfaces.setSize(vectorSize);
   for(i = 0; i < mNullSurfaces.size(); i++)
   {
      stream.read(&mNullSurfaces[i].windingStart);
      stream.read(&mNullSurfaces[i].planeIndex);
      stream.read(&mNullSurfaces[i].surfaceFlags);

      if (fileVersion >= 13)
         stream.read(&mNullSurfaces[i].windingCount);
      else
      {
         U8 count;
         stream.read(&count);
         mNullSurfaces[i].windingCount = (U32)count;
      }
   }

   // mLightmaps
   stream.read(&vectorSize);
   mLightmaps.setSize(vectorSize);
   mLightmapKeep.setSize(vectorSize);
   GBitmap dummyBmp;
   for(i = 0; i < mLightmaps.size(); i++)
   {
      mLightmaps[i] = new GBitmap;
      mLightmaps[i]->readBitmap("png",stream);

      if (!tgeInterior && (fileVersion == 0 || fileVersion == 5 || fileVersion >= 12))
      {
         // The "light normal maps" or "light direction maps" were
         // removed from Torque 3D... this just reads and throws 
         // them away.
         dummyBmp.readBitmap("png",stream);
      }

      stream.read(&mLightmapKeep[i]);
   }

   // mSolidLeafSurfaces
   stream.read(&vectorSize);
   mSolidLeafSurfaces.setSize(vectorSize);
   for(i = 0; i < mSolidLeafSurfaces.size(); i++)
   {
      stream.read(&mSolidLeafSurfaces[i]);
   }

   // mAnimatedLights
   mNumTriggerableLights = 0;
   stream.read(&vectorSize);
   mAnimatedLights.setSize(vectorSize);
   for(i = 0; i < mAnimatedLights.size(); i++)
   {
      stream.read(&mAnimatedLights[i].nameIndex);
      stream.read(&mAnimatedLights[i].stateIndex);
      stream.read(&mAnimatedLights[i].stateCount);
      stream.read(&mAnimatedLights[i].flags);
      stream.read(&mAnimatedLights[i].duration);

      if((mAnimatedLights[i].flags & AnimationAmbient) == 0)
         mNumTriggerableLights++;
   }

   // mLightStates
   stream.read(&vectorSize);
   mLightStates.setSize(vectorSize);
   for(i = 0; i < mLightStates.size(); i++)
   {
      stream.read(&mLightStates[i].red);
      stream.read(&mLightStates[i].green);
      stream.read(&mLightStates[i].blue);
      stream.read(&mLightStates[i].activeTime);
      stream.read(&mLightStates[i].dataIndex);
      stream.read(&mLightStates[i].dataCount);
   }

   // mStateData
   stream.read(&vectorSize);
   mStateData.setSize(vectorSize);
   for(i = 0; i < mStateData.size(); i++)
   {
      stream.read(&mStateData[i].surfaceIndex);
      stream.read(&mStateData[i].mapIndex);
      stream.read(&mStateData[i].lightStateIndex);
   }

   // mStateDataBuffer
   stream.read(&vectorSize);
   mStateDataBuffer.setSize(vectorSize);
   U32 flags;
   stream.read(&flags);
   stream.read(mStateDataBuffer.size(), mStateDataBuffer.address());

   // mNameBuffer
   stream.read(&vectorSize);
   mNameBuffer.setSize(vectorSize);
   stream.read(mNameBuffer.size(), mNameBuffer.address());

   // mSubObjects
   stream.read(&vectorSize);
   mSubObjects.setSize(vectorSize);
   for (i = 0; i < mSubObjects.size(); i++)
   {
      InteriorSubObject* iso = InteriorSubObject::readISO(stream);
      AssertFatal(iso != NULL, "Error, bad sub object in stream!");
      mSubObjects[i] = iso;
   }

   // Convex hulls
   stream.read(&vectorSize);
   mConvexHulls.setSize(vectorSize);
   for(i = 0; i < mConvexHulls.size(); i++)
   {
      stream.read(&mConvexHulls[i].hullStart);
      stream.read(&mConvexHulls[i].hullCount);
      stream.read(&mConvexHulls[i].minX);
      stream.read(&mConvexHulls[i].maxX);
      stream.read(&mConvexHulls[i].minY);
      stream.read(&mConvexHulls[i].maxY);
      stream.read(&mConvexHulls[i].minZ);
      stream.read(&mConvexHulls[i].maxZ);
      stream.read(&mConvexHulls[i].surfaceStart);
      stream.read(&mConvexHulls[i].surfaceCount);
      stream.read(&mConvexHulls[i].planeStart);
      stream.read(&mConvexHulls[i].polyListPlaneStart);
      stream.read(&mConvexHulls[i].polyListPointStart);
      stream.read(&mConvexHulls[i].polyListStringStart);

      if (fileVersion >= 12)
         stream.read(&mConvexHulls[i].staticMesh);
      else
         mConvexHulls[i].staticMesh = false;
   }

   // Convex hull emit strings
   stream.read(&vectorSize);
   mConvexHullEmitStrings.setSize(vectorSize);
   stream.read(mConvexHullEmitStrings.size(), mConvexHullEmitStrings.address());

   // Hull indices
   stream.read(&vectorSize);
   mHullIndices.setSize(vectorSize);
   for(i = 0; i < mHullIndices.size(); i++)
      stream.read(&mHullIndices[i]);

   // Hull plane indices
   stream.read(&vectorSize);
   mHullPlaneIndices.setSize(vectorSize);
   for(i = 0; i < mHullPlaneIndices.size(); i++)
      stream.read(&mHullPlaneIndices[i]);

   // Hull emit string indices
   stream.read(&vectorSize);
   mHullEmitStringIndices.setSize(vectorSize);
   for(i = 0; i < mHullEmitStringIndices.size(); i++)
      stream.read(&mHullEmitStringIndices[i]);

   // Hull surface indices
   stream.read(&vectorSize);
   mHullSurfaceIndices.setSize(vectorSize);
   for(i = 0; i < mHullSurfaceIndices.size(); i++)
      stream.read(&mHullSurfaceIndices[i]);

   // PolyList planes
   stream.read(&vectorSize);
   mPolyListPlanes.setSize(vectorSize);
   for(i = 0; i < mPolyListPlanes.size(); i++)
      stream.read(&mPolyListPlanes[i]);

   // PolyList points
   stream.read(&vectorSize);
   mPolyListPoints.setSize(vectorSize);
   for(i = 0; i < mPolyListPoints.size(); i++)
      stream.read(&mPolyListPoints[i]);

   // PolyList strings
   stream.read(&vectorSize);
   mPolyListStrings.setSize(vectorSize);
   for(i = 0; i < mPolyListStrings.size(); i++)
      stream.read(&mPolyListStrings[i]);

   // Coord bins
   for(i = 0; i < NumCoordBins * NumCoordBins; i++)
   {
      stream.read(&mCoordBins[i].binStart);
      stream.read(&mCoordBins[i].binCount);
   }

   // Coord bin indices
   stream.read(&vectorSize);
   mCoordBinIndices.setSize(vectorSize);
   for(i = 0; i < mCoordBinIndices.size(); i++)
      stream.read(&mCoordBinIndices[i]);

   // Coord bin mode
   stream.read(&mCoordBinMode);

   // Ambient colors
   stream.read(&mBaseAmbient);
   stream.read(&mAlarmAmbient);

   if (fileVersion >= 10)
   {
      // Static meshes
      stream.read(&vectorSize);

      mStaticMeshes.setSize(vectorSize);
      for (i = 0; i < mStaticMeshes.size(); i++)
      {
         mStaticMeshes[i] = new InteriorSimpleMesh;
         mStaticMeshes[i]->read(stream);
      }
   }

   if (fileVersion >= 11)
   {
      // Normals
      stream.read(&vectorSize);

      mNormals.setSize(vectorSize);
      for (i = 0; i < mNormals.size(); i++)
         mathRead(stream, &mNormals[i]);

      // TexMatrices
      stream.read(&vectorSize);

      mTexMatrices.setSize(vectorSize);
      for (i = 0; i < mTexMatrices.size(); i++)
      {
         stream.read(&mTexMatrices[i].T);
         stream.read(&mTexMatrices[i].N);
         stream.read(&mTexMatrices[i].B);
      }

      // TexMatIndices
      stream.read(&vectorSize);

      mTexMatIndices.setSize(vectorSize);
      for (i = 0; i < mTexMatIndices.size(); i++)
         stream.read(&mTexMatIndices[i]);
   }

   // For future expandability
   U32 dummy;
   if (fileVersion < 10)
   {
      stream.read(&dummy); if (dummy != 0) return false;
   }
   if (fileVersion < 11)
   {
      stream.read(&dummy); if (dummy != 0) return false;
      stream.read(&dummy); if (dummy != 0) return false;
   }

   //
   // Support for interior light map border sizes.
   //
   U32 extendedlightmapdata;
   stream.read(&extendedlightmapdata);
   if(extendedlightmapdata == 1)
   {
      stream.read(&mLightMapBorderSize);

	  //future expansion under current block (avoid using too
	  //many of the above expansion slots by allowing nested
	  //blocks)...
	  stream.read(&dummy); if (dummy != 0) return false;
   }

   // Setup the zone planes
   setupZonePlanes();
   truncateZoneTree();
   
   buildSurfaceZones();

   return (stream.getStatus() == Stream::Ok);
}

bool Interior::write(Stream& stream) const
{
   AssertFatal(stream.hasCapability(Stream::StreamWrite), "Interior::write: non-write capable stream passed");
   AssertFatal(stream.getStatus() == Stream::Ok, "Interior::write: Error, stream in inconsistent state");

   U32 i;

   // Version this stream
   stream.write(smFileVersion);

   stream.write(mDetailLevel);
   stream.write(mMinPixels);
   mathWrite(stream, mBoundingBox);
   mathWrite(stream, mBoundingSphere);

   stream.write(mHasAlarmState);
   stream.write(mNumLightStateEntries);

   // Now write out our data vectors.  Remember, for cross-platform capability, no
   //  structure writing is allowed...

   // mPlanes
   writePlaneVector(stream);

   // mPoints
   stream.write(mPoints.size());
   for (i = 0; i < mPoints.size(); i++)
      mathWrite(stream, mPoints[i].point);

   // mPointVisibility
   stream.write(mPointVisibility.size());
   stream.write(mPointVisibility.size(), mPointVisibility.address());

   // mTexGenEQs
   stream.write(mTexGenEQs.size());
   for (i = 0; i < mTexGenEQs.size(); i++)
   {
      mathWrite(stream, mTexGenEQs[i].planeX);
      mathWrite(stream, mTexGenEQs[i].planeY);
   }

   // mBSPNodes;
   stream.write(mBSPNodes.size());
   for (i = 0; i < mBSPNodes.size(); i++)
   {
      stream.write(mBSPNodes[i].planeIndex);

      if (smFileVersion < 14)
      {
         stream.write(U16(mBSPNodes[i].frontIndex));
         stream.write(U16(mBSPNodes[i].backIndex));
      }
      else
      {
         stream.write(mBSPNodes[i].frontIndex);
         stream.write(mBSPNodes[i].backIndex);
      }
   }

   // mBSPSolidLeaves
   stream.write(mBSPSolidLeaves.size());
   for (i = 0; i < mBSPSolidLeaves.size(); i++)
   {
      stream.write(mBSPSolidLeaves[i].surfaceIndex);
      stream.write(mBSPSolidLeaves[i].surfaceCount);
   }

   // MaterialList
   mMaterialList->write(stream);

   // mWindings
   stream.write(mWindings.size());
   for (i = 0; i < mWindings.size(); i++)
   {
      stream.write(mWindings[i]);
   }

   // mWindingIndices
   stream.write(mWindingIndices.size());
   for (i = 0; i < mWindingIndices.size(); i++)
   {
      stream.write(mWindingIndices[i].windingStart);
      stream.write(mWindingIndices[i].windingCount);
   }

   // mEdges
   if (smFileVersion >= 12)
   {
      stream.write(mEdges.size());
      for (i = 0; i < mEdges.size(); i++)
      {
         stream.write(mEdges[i].vertexes[0]);
         stream.write(mEdges[i].vertexes[1]);
         stream.write(mEdges[i].faces[0]);
         stream.write(mEdges[i].faces[1]);
      }
   }

   // mZones
   stream.write(mZones.size());
   for (i = 0; i < mZones.size(); i++)
   {
      stream.write(mZones[i].portalStart);
      stream.write(mZones[i].portalCount);
      stream.write(mZones[i].surfaceStart);
      stream.write(mZones[i].surfaceCount);

      if (smFileVersion >= 12)
      {
         stream.write(mZones[i].staticMeshStart);
         stream.write(mZones[i].staticMeshCount);
      }

      stream.write(mZones[i].flags);
   }

   // Zone surfaces
   stream.write(mZoneSurfaces.size());
   for (i = 0; i < mZoneSurfaces.size(); i++)
      stream.write(mZoneSurfaces[i]);

   // Zone static meshes
   if (smFileVersion >= 12)
   {
      stream.write(mZoneStaticMeshes.size());
      for (i = 0; i < mZoneStaticMeshes.size(); i++)
         stream.write(mZoneStaticMeshes[i]);
   }

   //  mZonePortalList;
   stream.write(mZonePortalList.size());
   for (i = 0; i < mZonePortalList.size(); i++)
      stream.write(mZonePortalList[i]);

   // mPortals
   stream.write(mPortals.size());
   for (i = 0; i < mPortals.size(); i++)
   {
      stream.write(mPortals[i].planeIndex);
      stream.write(mPortals[i].triFanCount);
      stream.write(mPortals[i].triFanStart);
      stream.write(mPortals[i].zoneFront);
      stream.write(mPortals[i].zoneBack);
   }

   // mSurfaces
   stream.write(mSurfaces.size());
   for (i = 0; i < mSurfaces.size(); i++)
   {
      stream.write(mSurfaces[i].windingStart);
      
      if (smFileVersion >= 13)
         stream.write(mSurfaces[i].windingCount);
      else
      {
         U8 count = (U8)mSurfaces[i].windingCount;
         stream.write(count);
      }

      stream.write(mSurfaces[i].planeIndex);
      stream.write(mSurfaces[i].textureIndex);
      stream.write(mSurfaces[i].texGenIndex);
      stream.write(mSurfaces[i].surfaceFlags);
      stream.write(mSurfaces[i].fanMask);
      writeLMapTexGen(stream, mLMTexGenEQs[i].planeX, mLMTexGenEQs[i].planeY);

      stream.write(mSurfaces[i].lightCount);
      stream.write(mSurfaces[i].lightStateInfoStart);

      if (smFileVersion >= 13)
      {
         stream.write(mSurfaces[i].mapOffsetX);
         stream.write(mSurfaces[i].mapOffsetY);
         stream.write(mSurfaces[i].mapSizeX);
         stream.write(mSurfaces[i].mapSizeY);
      }
      else
      {
         U8 offX, offY, sizeX, sizeY;
         offX = (U8)mSurfaces[i].mapOffsetX;
         offY = (U8)mSurfaces[i].mapOffsetY;
         sizeX = (U8)mSurfaces[i].mapSizeX;
         sizeY = (U8)mSurfaces[i].mapSizeY;

         stream.write(offX);
         stream.write(offY);
         stream.write(sizeX);
         stream.write(sizeY);
      }

      if (smFileVersion == 0 || smFileVersion >= 12)
         stream.write(mSurfaces[i].unused);
   }
   // NormalLMapIndices
   stream.write(mNormalLMapIndices.size());
   for (U32 i = 0; i < mNormalLMapIndices.size(); i++)
   {
      if (smFileVersion >= 13)
         stream.write(mNormalLMapIndices[i]);
      else
      {
         U8 index = (U8)mNormalLMapIndices[i];
         stream.write(index);
      }
   }

   // AlarmLMapIndices
   stream.write(mAlarmLMapIndices.size());
   for (U32 i = 0; i < mAlarmLMapIndices.size(); i++)
   {
      if (smFileVersion >= 13)
         stream.write(mAlarmLMapIndices[i]);
      else
      {
         U8 index = (U8)mAlarmLMapIndices[i];
         stream.write(index);
      }
   }


   // mNullSurfaces
   stream.write(mNullSurfaces.size());
   for(i = 0; i < mNullSurfaces.size(); i++)
   {
      stream.write(mNullSurfaces[i].windingStart);
      stream.write(mNullSurfaces[i].planeIndex);
      stream.write(mNullSurfaces[i].surfaceFlags);

      if (smFileVersion >= 13)
         stream.write(mNullSurfaces[i].windingCount);
      else
      {
         U8 count = (U8)mNullSurfaces[i].windingCount;
         stream.write(count);
      }
   }

   // mLightmaps
   stream.write(mLightmaps.size());
   for(i = 0; i < mLightmaps.size(); i++)
   {
      mLightmaps[i]->writeBitmap("png",stream);
      
      if (smFileVersion == 0 || smFileVersion >= 12)
      {
         // The "light normal maps" or "light direction maps" were
         // removed from Torque 3D... this just writes a dummy 2x2
         // texture so that the read/write functions don't change.
         GBitmap dummyBmp( 2, 2 );
         dummyBmp.writeBitmap("png",stream);
      }

      stream.write(mLightmapKeep[i]);
   }

   // mSolidLeafSurfaces
   stream.write(mSolidLeafSurfaces.size());
   for(i = 0; i < mSolidLeafSurfaces.size(); i++)
   {
      stream.write(mSolidLeafSurfaces[i]);
   }


   // Animated lights
   stream.write(mAnimatedLights.size());
   for(i = 0; i < mAnimatedLights.size(); i++)
   {
      stream.write(mAnimatedLights[i].nameIndex);
      stream.write(mAnimatedLights[i].stateIndex);
      stream.write(mAnimatedLights[i].stateCount);
      stream.write(mAnimatedLights[i].flags);
      stream.write(mAnimatedLights[i].duration);
   }

   stream.write(mLightStates.size());
   for(i = 0; i < mLightStates.size(); i++)
   {
      stream.write(mLightStates[i].red);
      stream.write(mLightStates[i].green);
      stream.write(mLightStates[i].blue);
      stream.write(mLightStates[i].activeTime);
      stream.write(mLightStates[i].dataIndex);
      stream.write(mLightStates[i].dataCount);
   }

   // mStateData
   stream.write(mStateData.size());
   for(i = 0; i < mStateData.size(); i++)
   {
      stream.write(mStateData[i].surfaceIndex);
      stream.write(mStateData[i].mapIndex);
      stream.write(mStateData[i].lightStateIndex);
   }

   // mStateDataBuffer: Note: superfluous 0 is for flags in future versions.
   //                    that may add compression.  This way, we can maintain
   //                    compatability with previous versions.
   stream.write(mStateDataBuffer.size());
   stream.write(U32(0));
   stream.write(mStateDataBuffer.size(), mStateDataBuffer.address());

   // mNameBuffer
   stream.write(mNameBuffer.size());
   stream.write(mNameBuffer.size(), mNameBuffer.address());

   // mSubObjects
   stream.write(mSubObjects.size());
   for (i = 0; i < mSubObjects.size(); i++)
   {
      bool writeSuccess = mSubObjects[i]->writeISO(stream);
      AssertFatal(writeSuccess == true, "Error writing sub object to stream!");
   }

   // Convex hulls
   stream.write(mConvexHulls.size());
   for(i = 0; i < mConvexHulls.size(); i++)
   {
      stream.write(mConvexHulls[i].hullStart);
      stream.write(mConvexHulls[i].hullCount);
      stream.write(mConvexHulls[i].minX);
      stream.write(mConvexHulls[i].maxX);
      stream.write(mConvexHulls[i].minY);
      stream.write(mConvexHulls[i].maxY);
      stream.write(mConvexHulls[i].minZ);
      stream.write(mConvexHulls[i].maxZ);
      stream.write(mConvexHulls[i].surfaceStart);
      stream.write(mConvexHulls[i].surfaceCount);
      stream.write(mConvexHulls[i].planeStart);
      stream.write(mConvexHulls[i].polyListPlaneStart);
      stream.write(mConvexHulls[i].polyListPointStart);
      stream.write(mConvexHulls[i].polyListStringStart);

      if (smFileVersion >= 12)
         stream.write(mConvexHulls[i].staticMesh);
   }

   stream.write(mConvexHullEmitStrings.size());
   stream.write(mConvexHullEmitStrings.size(), mConvexHullEmitStrings.address());

   stream.write(mHullIndices.size());
   for(i = 0; i < mHullIndices.size(); i++)
      stream.write(mHullIndices[i]);

   stream.write(mHullPlaneIndices.size());
   for(i = 0; i < mHullPlaneIndices.size(); i++)
      stream.write(mHullPlaneIndices[i]);

   stream.write(mHullEmitStringIndices.size());
   for(i = 0; i < mHullEmitStringIndices.size(); i++)
      stream.write(mHullEmitStringIndices[i]);

   stream.write(mHullSurfaceIndices.size());
   for(i = 0; i < mHullSurfaceIndices.size(); i++)
      stream.write(mHullSurfaceIndices[i]);

   stream.write(mPolyListPlanes.size());
   for(i = 0; i < mPolyListPlanes.size(); i++)
      stream.write(mPolyListPlanes[i]);

   stream.write(mPolyListPoints.size());
   for(i = 0; i < mPolyListPoints.size(); i++)
      stream.write(mPolyListPoints[i]);

   stream.write(mPolyListStrings.size());
   for(i = 0; i < mPolyListStrings.size(); i++)
      stream.write(mPolyListStrings[i]);

   // Coord bins
   for(i = 0; i < NumCoordBins * NumCoordBins; i++)
   {
      stream.write(mCoordBins[i].binStart);
      stream.write(mCoordBins[i].binCount);
   }
   stream.write(mCoordBinIndices.size());
   for(i = 0; i < mCoordBinIndices.size(); i++)
      stream.write(mCoordBinIndices[i]);
   stream.write(mCoordBinMode);

   // Ambient colors...
   stream.write(mBaseAmbient);
   stream.write(mAlarmAmbient);

   if (smFileVersion >= 10)
   {
      // Static meshes
      stream.write(mStaticMeshes.size());
      for (i = 0; i < mStaticMeshes.size(); i++)
         mStaticMeshes[i]->write(stream);
   }
   else
      stream.write(U32(0));

   if (smFileVersion >= 11)
   {
      // Normals
      stream.write(mNormals.size());
      for (i = 0; i < mNormals.size(); i++)
         mathWrite(stream, mNormals[i]);

      // TexMatrices
      stream.write(mTexMatrices.size());
      for (i = 0; i < mTexMatrices.size(); i++)
      {
         stream.write(mTexMatrices[i].T);
         stream.write(mTexMatrices[i].N);
         stream.write(mTexMatrices[i].B);
      }

      // TexMatIndices
      stream.write(mTexMatIndices.size());
      for (i = 0; i < mTexMatIndices.size(); i++)
         stream.write(mTexMatIndices[i]);
   }
   else
   {
      stream.write(U32(0));
      stream.write(U32(0));
   }

   //
   // Support for interior light map border sizes.
   //
   if(mLightMapBorderSize > 0)
   {
      stream.write(U32(1));//flag new block...
      stream.write(U32(mLightMapBorderSize));//new block data..

	  //future expansion under current block (avoid using too
	  //many of the above expansion slots by allowing nested
	  //blocks)...
      stream.write(U32(0));
   }
   else
   {
      stream.write(U32(0));
   }

   return stream.getStatus() == Stream::Ok;
}

bool Interior::readVehicleCollision(Stream& stream)
{
   AssertFatal(stream.hasCapability(Stream::StreamRead), "Interior::read: non-read capable stream passed");
   AssertFatal(stream.getStatus() == Stream::Ok, "Interior::read: Error, stream in inconsistent state");

   S32 i;

   // Version this stream.  We only load stream of the current version
   U32 fileVersion;
   stream.read(&fileVersion);
   if (fileVersion > smFileVersion)
   {
      Con::errorf(ConsoleLogEntry::General, "Interior::read: incompatible file version found.");
      return false;
   }

   U32 vectorSize;

   // Convex hulls
   stream.read(&vectorSize);
   mVehicleConvexHulls.setSize(vectorSize);
   for(i = 0; i < mVehicleConvexHulls.size(); i++)
   {
      stream.read(&mVehicleConvexHulls[i].hullStart);
      stream.read(&mVehicleConvexHulls[i].hullCount);
      stream.read(&mVehicleConvexHulls[i].minX);
      stream.read(&mVehicleConvexHulls[i].maxX);
      stream.read(&mVehicleConvexHulls[i].minY);
      stream.read(&mVehicleConvexHulls[i].maxY);
      stream.read(&mVehicleConvexHulls[i].minZ);
      stream.read(&mVehicleConvexHulls[i].maxZ);
      stream.read(&mVehicleConvexHulls[i].surfaceStart);
      stream.read(&mVehicleConvexHulls[i].surfaceCount);
      stream.read(&mVehicleConvexHulls[i].planeStart);
      stream.read(&mVehicleConvexHulls[i].polyListPlaneStart);
      stream.read(&mVehicleConvexHulls[i].polyListPointStart);
      stream.read(&mVehicleConvexHulls[i].polyListStringStart);
   }

   stream.read(&vectorSize);
   mVehicleConvexHullEmitStrings.setSize(vectorSize);
   stream.read(mVehicleConvexHullEmitStrings.size(), mVehicleConvexHullEmitStrings.address());

   stream.read(&vectorSize);
   mVehicleHullIndices.setSize(vectorSize);
   for(i = 0; i < mVehicleHullIndices.size(); i++)
      stream.read(&mVehicleHullIndices[i]);

   stream.read(&vectorSize);
   mVehicleHullPlaneIndices.setSize(vectorSize);
   for(i = 0; i < mVehicleHullPlaneIndices.size(); i++)
      stream.read(&mVehicleHullPlaneIndices[i]);

   stream.read(&vectorSize);
   mVehicleHullEmitStringIndices.setSize(vectorSize);
   for(i = 0; i < mVehicleHullEmitStringIndices.size(); i++)
      stream.read(&mVehicleHullEmitStringIndices[i]);

   stream.read(&vectorSize);
   mVehicleHullSurfaceIndices.setSize(vectorSize);
   for(i = 0; i < mVehicleHullSurfaceIndices.size(); i++)
      stream.read(&mVehicleHullSurfaceIndices[i]);

   stream.read(&vectorSize);
   mVehiclePolyListPlanes.setSize(vectorSize);
   for(i = 0; i < mVehiclePolyListPlanes.size(); i++)
      stream.read(&mVehiclePolyListPlanes[i]);

   stream.read(&vectorSize);
   mVehiclePolyListPoints.setSize(vectorSize);
   for(i = 0; i < mVehiclePolyListPoints.size(); i++)
      stream.read(&mVehiclePolyListPoints[i]);

   stream.read(&vectorSize);
   mVehiclePolyListStrings.setSize(vectorSize);
   for(i = 0; i < mVehiclePolyListStrings.size(); i++)
      stream.read(&mVehiclePolyListStrings[i]);

   stream.read(&vectorSize);
   mVehicleNullSurfaces.setSize(vectorSize);
   for(i = 0; i < mVehicleNullSurfaces.size(); i++)
   {
      stream.read(&mVehicleNullSurfaces[i].windingStart);
      stream.read(&mVehicleNullSurfaces[i].planeIndex);
      stream.read(&mVehicleNullSurfaces[i].surfaceFlags);
      stream.read(&mVehicleNullSurfaces[i].windingCount);
   }

   stream.read(&vectorSize);
   mVehiclePoints.setSize(vectorSize);
   for(i = 0; i < mVehiclePoints.size(); i++)
      mathRead(stream, &mVehiclePoints[i].point);

   stream.read(&vectorSize);
   mVehiclePlanes.setSize(vectorSize);
   for(i = 0; i < mVehiclePlanes.size(); i++)
      mathRead(stream, &mVehiclePlanes[i]);

   stream.read(&vectorSize);
   mVehicleWindings.setSize(vectorSize);
   for(i = 0; i < mVehicleWindings.size(); i++)
   {
      stream.read(&mVehicleWindings[i]);
   }

   stream.read(&vectorSize);
   mVehicleWindingIndices.setSize(vectorSize);
   for(i = 0; i < mVehicleWindingIndices.size(); i++)
   {
      stream.read(&mVehicleWindingIndices[i].windingStart);
      stream.read(&mVehicleWindingIndices[i].windingCount);
   }

   return true;
}

bool Interior::writeVehicleCollision(Stream& stream) const
{
   AssertFatal(stream.hasCapability(Stream::StreamWrite), "Interior::write: non-write capable stream passed");
   AssertFatal(stream.getStatus() == Stream::Ok, "Interior::write: Error, stream in inconsistent state");

   U32 i;

   // Version this stream
   stream.write(smFileVersion);

   // Convex hulls
   stream.write(mVehicleConvexHulls.size());
   for(i = 0; i < mVehicleConvexHulls.size(); i++)
   {
      stream.write(mVehicleConvexHulls[i].hullStart);
      stream.write(mVehicleConvexHulls[i].hullCount);
      stream.write(mVehicleConvexHulls[i].minX);
      stream.write(mVehicleConvexHulls[i].maxX);
      stream.write(mVehicleConvexHulls[i].minY);
      stream.write(mVehicleConvexHulls[i].maxY);
      stream.write(mVehicleConvexHulls[i].minZ);
      stream.write(mVehicleConvexHulls[i].maxZ);
      stream.write(mVehicleConvexHulls[i].surfaceStart);
      stream.write(mVehicleConvexHulls[i].surfaceCount);
      stream.write(mVehicleConvexHulls[i].planeStart);
      stream.write(mVehicleConvexHulls[i].polyListPlaneStart);
      stream.write(mVehicleConvexHulls[i].polyListPointStart);
      stream.write(mVehicleConvexHulls[i].polyListStringStart);
   }

   stream.write(mVehicleConvexHullEmitStrings.size());
   stream.write(mVehicleConvexHullEmitStrings.size(), mVehicleConvexHullEmitStrings.address());

   stream.write(mVehicleHullIndices.size());
   for(i = 0; i < mVehicleHullIndices.size(); i++)
      stream.write(mVehicleHullIndices[i]);

   stream.write(mVehicleHullPlaneIndices.size());
   for(i = 0; i < mVehicleHullPlaneIndices.size(); i++)
      stream.write(mVehicleHullPlaneIndices[i]);

   stream.write(mVehicleHullEmitStringIndices.size());
   for(i = 0; i < mVehicleHullEmitStringIndices.size(); i++)
      stream.write(mVehicleHullEmitStringIndices[i]);

   stream.write(mVehicleHullSurfaceIndices.size());
   for(i = 0; i < mVehicleHullSurfaceIndices.size(); i++)
      stream.write(mVehicleHullSurfaceIndices[i]);

   stream.write(mVehiclePolyListPlanes.size());
   for(i = 0; i < mVehiclePolyListPlanes.size(); i++)
      stream.write(mVehiclePolyListPlanes[i]);

   stream.write(mVehiclePolyListPoints.size());
   for(i = 0; i < mVehiclePolyListPoints.size(); i++)
      stream.write(mVehiclePolyListPoints[i]);

   stream.write(mVehiclePolyListStrings.size());
   for(i = 0; i < mVehiclePolyListStrings.size(); i++)
      stream.write(mVehiclePolyListStrings[i]);

   stream.write(mVehicleNullSurfaces.size());
   for(i = 0; i < mVehicleNullSurfaces.size(); i++)
   {
      stream.write(mVehicleNullSurfaces[i].windingStart);
      stream.write(mVehicleNullSurfaces[i].planeIndex);
      stream.write(mVehicleNullSurfaces[i].surfaceFlags);
      stream.write(mVehicleNullSurfaces[i].windingCount);
   }

   stream.write(mVehiclePoints.size());
   for(i = 0; i < mVehiclePoints.size(); i++)
      mathWrite(stream, mVehiclePoints[i].point);

   stream.write(mVehiclePlanes.size());
   for(i = 0; i < mVehiclePlanes.size(); i++)
      mathWrite(stream, mVehiclePlanes[i]);

   stream.write(mVehicleWindings.size());
   for(i = 0; i < mVehicleWindings.size(); i++)
      stream.write(mVehicleWindings[i]);

   stream.write(mVehicleWindingIndices.size());
   for(i = 0; i < mVehicleWindingIndices.size(); i++)
   {
      stream.write(mVehicleWindingIndices[i].windingStart);
      stream.write(mVehicleWindingIndices[i].windingCount);
   }

   return true;
}

bool Interior::readSurface(Stream& stream, Surface& surface, TexGenPlanes& texgens, const bool tgeInterior)
{
   // If we end up reading any invalid data then odds are that we
   // are no longer correctly reading from the stream and have gotten
   // off because this is a TGE version 0 Interior so we bail.
   // That is why you will see checks all the way through
   stream.read(&surface.windingStart);

   if (surface.windingStart >= mWindings.size())
      return false;

   if (mFileVersion >= 13)
      stream.read(&surface.windingCount);
   else
   {
      U8 count;
      stream.read(&count);
      surface.windingCount = (U32)count;
   }

   if (surface.windingStart + surface.windingCount > mWindings.size())
      return false;

   stream.read(&surface.planeIndex);

   if (U32(surface.planeIndex & ~0x8000) >= mPlanes.size())
      return false;

   stream.read(&surface.textureIndex);

   if (surface.textureIndex >= mMaterialList->size())
      return false;

   stream.read(&surface.texGenIndex);

   if (surface.texGenIndex >= mTexGenEQs.size())
      return false;

   stream.read(&surface.surfaceFlags);
   stream.read(&surface.fanMask);

   // If reading the lightmap texgen fails then most likely this is a
   // TGE version 0 Interior (it gets offset by the "unused" read below
   if (readLMapTexGen(stream, texgens.planeX, texgens.planeY) == false)
      return false;

   stream.read(&surface.lightCount);
   stream.read(&surface.lightStateInfoStart);

   if (mFileVersion >= 13)
   {
      stream.read(&surface.mapOffsetX);
      stream.read(&surface.mapOffsetY);
      stream.read(&surface.mapSizeX);
      stream.read(&surface.mapSizeY);
   }
   else
   {
      U8 offX, offY, sizeX, sizeY;
      stream.read(&offX);
      stream.read(&offY);
      stream.read(&sizeX);
      stream.read(&sizeY);

      surface.mapOffsetX = (U32)offX;
      surface.mapOffsetY = (U32)offY;
      surface.mapSizeX = (U32)sizeX;
      surface.mapSizeY = (U32)sizeY;
   }

   if (!tgeInterior && (mFileVersion == 0 || mFileVersion == 5 || mFileVersion >= 12))
      stream.read(&surface.unused);

   if (mFileVersion == 5)
   {
      U32 brushId;
      stream.read(&brushId);
   }

   return true;
}

bool Interior::readLMapTexGen(Stream& stream, PlaneF& planeX, PlaneF& planeY)
{
   F32 genX[4];
   F32 genY[4];

   for(U32 i = 0; i < 4; i++)
   {
      genX[i] = 0.0f;
      genY[i] = 0.0f;
   }

   U16 finalWord;
   stream.read(&finalWord);
   stream.read(&genX[3]);
   stream.read(&genY[3]);

   // Unpack the final word.
   U32 logScaleY = (finalWord >> 0) & ((1 << 6) - 1);
   U32 logScaleX = (finalWord >> 6) & ((1 << 6) - 1);
   U16 stEnc     = (finalWord >> 13) & 7;

   S32 sc, tc;
   switch(stEnc)
   {
   case 0: sc = 0; tc = 1; break;
   case 1: sc = 0; tc = 2; break;
   case 2: sc = 1; tc = 0; break;
   case 3: sc = 1; tc = 2; break;
   case 4: sc = 2; tc = 0; break;
   case 5: sc = 2; tc = 1; break;

   default:
      sc = tc = -1;
      // This is potentially an invalid st coord encoding however *most* times
      // this is caused by attempting to load a TGE version 0 Interior
      return false;
   }

   U32 invScaleX = 1 << logScaleX;
   U32 invScaleY = 1 << logScaleY;

   genX[sc] = F32(1.0 / F64(invScaleX));
   genY[tc] = F32(1.0 / F64(invScaleY));

   planeX.x = genX[0];
   planeX.y = genX[1];
   planeX.z = genX[2];
   planeX.d = genX[3];
   planeY.x = genY[0];
   planeY.y = genY[1];
   planeY.z = genY[2];
   planeY.d = genY[3];

   return stream.getStatus() == Stream::Ok;
}

bool Interior::writeLMapTexGen(Stream& stream, const PlaneF& planeX, const PlaneF& planeY) const
{
   F32 genX[4], genY[4];

   genX[0] = planeX.x;
   genX[1] = planeX.y;
   genX[2] = planeX.z;
   genX[3] = planeX.d;
   genY[0] = planeY.x;
   genY[1] = planeY.y;
   genY[2] = planeY.z;
   genY[3] = planeY.d;

   // The tex gen for lmaps is a special case.
   //  there are only 4 parameters that matter,
   //  an inverse power of 2 in the x and y, and the
   //  fp offsets in x and y.  We can encode the
   //  scales completely in U16 and we'll just write out
   //  the offsets.  First, determine which coords we're
   //  writing...
   //
   S32 sc = -1;
   S32 tc = -1;
   if(genX[0] != 0.0)      sc = 0;
   else if(genX[1] != 0.0) sc = 1;
   else if(genX[2] != 0.0) sc = 2;

   if(genY[0] != 0.0)      tc = 0;
   else if(genY[1] != 0.0) tc = 1;
   else if(genY[2] != 0.0) tc = 2;
   AssertFatal(sc != -1 && tc != -1 && sc != tc, "Hm, something wrong here.");

   U32 invScaleX = U32((1.0f / genX[sc]) + 0.5);
   U32 invScaleY = U32((1.0f / genY[tc]) + 0.5);
   AssertISV(invScaleX && isPow2(invScaleX) && invScaleY && isPow2(invScaleY), "Not a power of 2?  Something wrong");

   U32 logScaleX = getBinLog2(invScaleX);
   U32 logScaleY = getBinLog2(invScaleY);
   AssertFatal(logScaleX < 63 && logScaleY < 63, "Error, you've set the lightmap scale WAAYYY to high!");

   // We need 3 bits to encode sc and tc, which leaves us 6 bits for logScaleX
   //  and logScaleY
   S16 stEnc = -1;
   if(sc == 0 && tc == 1) stEnc = 0;
   else if(sc == 0 && tc == 2) stEnc = 1;
   else if(sc == 1 && tc == 0) stEnc = 2;
   else if(sc == 1 && tc == 2) stEnc = 3;
   else if(sc == 2 && tc == 0) stEnc = 4;
   else if(sc == 2 && tc == 1) stEnc = 5;
   AssertFatal(stEnc != -1, avar("Hm.  This should never happen. (%d, %d)", sc, tc));

   U16 finalWord = U16(stEnc) << 13;
   finalWord       |= logScaleX << 6;
   finalWord       |= logScaleY << 0;

   stream.write(finalWord);
   stream.write(genX[3]);
   stream.write(genY[3]);

   return stream.getStatus() == Stream::Ok;
}

bool Interior::writePlaneVector(Stream& stream) const
{
   // This is pretty slow, but who cares?
   //
   Vector<Point3F> uniqueNormals(mPlanes.size());
   Vector<U16>  uniqueIndices(mPlanes.size());

   U32 i;

   for(i = 0; i < mPlanes.size(); i++)
   {
      bool inserted = false;
      for(U32 j = 0; j < uniqueNormals.size(); j++)
      {
         if(mPlanes[i] == uniqueNormals[j])
         {
            // Hah!  Already have this one...
            uniqueIndices.push_back(j);
            inserted = true;
            break;
         }
      }

      if(inserted == false)
      {
         // Gotta do it ourselves...
         uniqueIndices.push_back(uniqueNormals.size());
         uniqueNormals.push_back(Point3F(mPlanes[i].x, mPlanes[i].y, mPlanes[i].z));
      }
   }

   // Ok, what we have now, is a list of unique normals, a set of indices into
   //  that vector, and the distances that we still have to write out by hand.
   //  Hop to it!
   stream.write(uniqueNormals.size());
   for(i = 0; i < uniqueNormals.size(); i++)
      mathWrite(stream, uniqueNormals[i]);

   stream.write(mPlanes.size());
   for(i = 0; i < mPlanes.size(); i++)
   {
      stream.write(uniqueIndices[i]);
      stream.write(mPlanes[i].d);
   }

   return(stream.getStatus() == Stream::Ok);
}

bool Interior::readPlaneVector(Stream& stream)
{
   U32 vectorSize;

   stream.read(&vectorSize);
   Point3F* normals = new Point3F[vectorSize];
   U32 i;
   for(i = 0; i < vectorSize; i++)
      mathRead(stream, &normals[i]);

   U16 index;
   stream.read(&vectorSize);
   mPlanes.setSize(vectorSize);
   for(i = 0; i < mPlanes.size(); i++)
   {
      stream.read(&index);
      stream.read(&mPlanes[i].d);
      mPlanes[i].x = normals[index].x;
      mPlanes[i].y = normals[index].y;
      mPlanes[i].z = normals[index].z;
   }

   delete [] normals;

   return(stream.getStatus() == Stream::Ok);
}


bool Interior::getUnifiedZone(const U32 index, S32* zone)
{
   if(isBSPLeafIndex(index))
   {
      if(isBSPSolidLeaf(index))
         *zone = -1;
      else
         *zone = S32(getBSPEmptyLeafZone(index));
      return true;
   }
   else
   {
      S32 frontZone, backZone;
      bool frontUnified = getUnifiedZone(mBSPNodes[index].frontIndex, &frontZone);
      bool backUnified  = getUnifiedZone(mBSPNodes[index].backIndex, &backZone);
      if(frontUnified && backUnified)
      {
         if(frontZone == backZone)
         {
            *zone = frontZone;
            return true;
         }
         else
         {
            if(frontZone == -1 || backZone == -1)
            {
               // DMMFIX: Once the interior file format is able to distinguish
               //  between structural and detail nodes in the runtime bsp,
               //  we can make this work a little better.
               return false;
            }
            else
            {
               // Not equal, and neither is -1, no unified zone possible
               return false;
            }
         }
      }
      else
      {
         return false;
      }
   }
}

void Interior::truncateZoneNode(const U32 index)
{
   S32 unifiedZone;
   bool unified = getUnifiedZone(index, &unifiedZone);
   if(unified)
   {
      // Aha!
      if(isBSPLeafIndex(index))
         return;

      if(unifiedZone == -1)
         mBSPNodes[index].terminalZone = U16(0xFFFF);
      else
         mBSPNodes[index].terminalZone = U16(0x8000) | U16(unifiedZone);
   }
   else
   {
      // Sigh.
      if(isBSPLeafIndex(mBSPNodes[index].frontIndex) == false)
         truncateZoneNode(mBSPNodes[index].frontIndex);
      if(isBSPLeafIndex(mBSPNodes[index].backIndex) == false)
         truncateZoneNode(mBSPNodes[index].backIndex);
   }
}

void Interior::truncateZoneTree()
{
   for(U32 i = 0; i < mBSPNodes.size(); i++)
   {
      mBSPNodes[i].terminalZone = 0;
   }

   if(mBSPNodes.size() > 0)
      truncateZoneNode(0);
}


void Interior::setupZonePlanes()
{
   U16* temp = new U16[mPlanes.size() * mZones.size()];
   U32  tempSize = 0;

   for(U32 i = 0; i < mZones.size(); i++)
   {
      Zone& rZone = mZones[i];

      BitVector usedPlanes;
      usedPlanes.setSize(mPlanes.size());
      usedPlanes.clear();

      U32 j;
      for(j = 0; j < rZone.surfaceCount; j++)
      {
         Surface& rSurface = mSurfaces[mZoneSurfaces[rZone.surfaceStart + j]];
         usedPlanes.set(getPlaneIndex(rSurface.planeIndex));
      }

      rZone.planeStart = tempSize;
      for(j = 0; j < mPlanes.size(); j++)
      {
         if(usedPlanes.test(j))
         {
            AssertFatal(tempSize < mPlanes.size() * mZones.size(), "Error, out of bounds plane list!");
            temp[tempSize++] = j;
         }
      }
      rZone.planeCount = tempSize - rZone.planeStart;
   }

   mZonePlanes.setSize(tempSize);
   for(U32 j = 0; j < tempSize; j++)
      mZonePlanes[j] = temp[j];

   delete [] temp;
}
