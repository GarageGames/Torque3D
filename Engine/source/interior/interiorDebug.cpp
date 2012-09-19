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

#include "torqueConfig.h"

#ifndef TORQUE_SHIPPING

#include "interior/interior.h"
#include "interior/interiorInstance.h"
#include "console/console.h"
#include "core/color.h"
#include "math/mMatrix.h"
#include "gfx/bitmap/gBitmap.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxShader.h"
#include "materials/matInstance.h"
#include "materials/materialList.h"
#include "materials/shaderData.h"
#include "renderInstance/renderPassManager.h"
#include "shaderGen/shaderGenVars.h"

static U8 interiorDebugColors[14][3] =
{
   { 0xFF, 0xFF, 0xFF },
   { 0x00, 0x00, 0xFF },
   { 0x00, 0xFF, 0x00 },
   { 0xFF, 0x00, 0x00 },
   { 0xFF, 0xFF, 0x00 },
   { 0xFF, 0x00, 0xFF },
   { 0x00, 0xFF, 0xFF },
   { 0x80, 0x80, 0x80 },
   { 0xFF, 0x80, 0x80 },
   { 0x80, 0xFF, 0x80 },
   { 0x80, 0x80, 0xFF },
   { 0x80, 0xFF, 0xFF },
   { 0xFF, 0x80, 0xFF },
   { 0xFF, 0x80, 0x80 }
};


namespace
{

void lineLoopFromStrip(Vector<ItrPaddedPoint>& points,
                       Vector<U32>&            windings,
                       U32                     windingStart,
                       U32                     windingCount)
{
	PrimBuild::begin(GFXLineStrip, windingCount + 1);
   PrimBuild::vertex3fv(points[windings[windingStart]].point);
   S32 skip = windingStart + 1;
   while (skip < (windingStart + windingCount))
   {
      PrimBuild::vertex3fv(points[windings[skip]].point);
      skip += 2;
   }

   skip -= 1;
   while (skip > windingStart)
   {
		if (skip < (windingStart + windingCount))
         PrimBuild::vertex3fv(points[windings[skip]].point);

      skip -= 2;
   }
   PrimBuild::vertex3fv(points[windings[windingStart]].point);
   PrimBuild::end();
}

void lineStrip(Vector<ItrPaddedPoint>& points,
               Vector<U32>&            windings,
               U32                     windingStart,
               U32                     windingCount)
{
   U32 end = 2;

   while (end < windingCount)
   {
      // Even
      PrimBuild::begin(GFXLineStrip, 4);
      PrimBuild::vertex3fv(points[windings[windingStart + end - 2]].point);
      PrimBuild::vertex3fv(points[windings[windingStart + end - 1]].point);
      PrimBuild::vertex3fv(points[windings[windingStart + end - 0]].point);
      PrimBuild::vertex3fv(points[windings[windingStart + end - 2]].point);
      PrimBuild::end();
      
      end++;
      if (end >= windingCount)
         break;

      // Odd
      PrimBuild::begin(GFXLineStrip, 4);
      PrimBuild::vertex3fv(points[windings[windingStart + end - 1]].point);
      PrimBuild::vertex3fv(points[windings[windingStart + end - 2]].point);
      PrimBuild::vertex3fv(points[windings[windingStart + end - 0]].point);
      PrimBuild::vertex3fv(points[windings[windingStart + end - 1]].point);
      PrimBuild::end();

      end++;
   }
}

} // namespace {}

void Interior::debugRender(const ZoneVisDeterminer& zoneVis, SceneData &sgData, InteriorInstance *intInst, MatrixF& modelview)
{
	// We use this shader to color things
	if ( mDebugShader == NULL )
   {
      ShaderData *shaderData = NULL;
      AssertFatal( Sim::findObject( "_DebugInterior_", shaderData ), "Unable to find ShaderData _DebugInterior_" );
      
      mDebugShader = shaderData ? shaderData->getShader() : NULL;

      if ( mDebugShader )
      {
         mDebugShaderConsts = mDebugShader->allocConstBuffer();
         mDebugShaderModelViewSC = mDebugShader->getShaderConstHandle(ShaderGenVars::modelview);
         mDebugShaderShadeColorSC = mDebugShader->getShaderConstHandle("$shadeColor");
      }
	}

   // We use this to override the texture that the interior defines.
	if (mDebugTexture.isNull())
   {
		// Allocate a small white bitmap
		GBitmap temp(16, 16);
		mDebugTexture.set(&temp, &GFXDefaultStaticDiffuseProfile, false, "Blank Texture");
	}

   // Set up our buffers
   GFX->setVertexBuffer( mVertBuff );
   GFX->setPrimitiveBuffer( mPrimBuff );

   // Set the modelview matrix for the shaders
   mDebugShaderConsts->setSafe(mDebugShaderModelViewSC, modelview);

	GFX->disableShaders();

   switch (smRenderMode)
   {
     case NormalRenderLines:
      debugNormalRenderLines(zoneVis);
      break;

     case ShowDetail:
      debugShowDetail(zoneVis);
      break;

     case ShowAmbiguous:
      debugShowAmbiguous(zoneVis);
      break;

     case ShowLightmaps:
      debugShowLightmaps(zoneVis, sgData, intInst);
      break;

     case ShowPortalZones:
      debugShowPortalZones(zoneVis);
      break;

     case ShowCollisionFans:
      debugShowCollisionFans(zoneVis);
      break;

     case ShowOrphan:
      debugShowOrphan(zoneVis);
      break;

     case ShowStrips:
      debugShowStrips(zoneVis);
      break;

     case ShowTexturesOnly:
      debugShowTexturesOnly(zoneVis, sgData, intInst);
      break;

     case ShowNullSurfaces:
      debugShowNullSurfaces(zoneVis, sgData, intInst);
      break;

     case ShowLargeTextures:
      debugShowLargeTextures(zoneVis, sgData, intInst);
      break;

     case ShowOutsideVisible:
      debugShowOutsideVisible(zoneVis);
      break;

     case ShowHullSurfaces:
      debugShowHullSurfaces();
      break;

     case ShowVehicleHullSurfaces:
      debugShowVehicleHullSurfaces(zoneVis, sgData, intInst);
      break;

     case ShowDetailLevel:
      debugShowDetailLevel(zoneVis);
      break;

	  case ShowVertexColors:
//      debugShowVertexColors(pMaterials);
      break;

     default:
      AssertWarn(false, "Warning!  Misunderstood debug render mode.  Defaulting to ShowDetail");
      debugShowDetail(zoneVis);
      break;
   }
}

void Interior::preDebugRender()
{
	// Set up our rendering states.
   if( mDebugShader )
   {
      // Set our shader
      GFX->setShader( mDebugShader );

      // Set a "blank" texture
      GFX->setTexture( 0, mDebugTexture );

      // Set a state block to enable our texture
      GFX->setStateBlock(mInteriorDebugTextureSB);
   }
}

void Interior::debugNormalRenderLines(const ZoneVisDeterminer& zoneVis)
{
   // Set our "base debug states" (no texture)
   GFX->setStateBlock(mInteriorDebugNoneSB);

	for (U32 i = 0; i < mZones.size(); i++)
   {
      if (zoneVis.isZoneVisible(i) == false)
         continue;

      for( U32 j=0; j<mZones[i].surfaceCount; j++ )
      {
         U32 surfaceIndex = mZoneSurfaces[mZones[i].surfaceStart + j];
         Surface& rSurface = mSurfaces[ surfaceIndex ];
			PrimBuild::color3f(0.0f, 0.0f, 0.0f);
			lineLoopFromStrip(mPoints, mWindings, rSurface.windingStart, rSurface.windingCount);
		}
	}
}

void Interior::debugShowSurfaceFlag(const ZoneVisDeterminer& zoneVis, const U32 flag, const ColorF& c)
{
	preDebugRender();

	for (U32 i = 0; i < mZones.size(); i++)
   {
      if (zoneVis.isZoneVisible(i) == false)
         continue;

      for( U32 j=0; j<mZones[i].surfaceCount; j++ )
      {
         U32 surfaceIndex = mZoneSurfaces[mZones[i].surfaceStart + j];
         Surface& rSurface = mSurfaces[ surfaceIndex ];
			ColorF col(0.0f, 0.0f, 0.0f);
			if (rSurface.surfaceFlags & flag)
				col = c;
			else
         {
				if (smFocusedDebug == true)
					continue;
				else
					col.set(1.0f, 1.0f, 1.0f);
			}

         mDebugShaderConsts->setSafe(mDebugShaderShadeColorSC, col);

         GFX->setShaderConstBuffer(mDebugShaderConsts);

			GFXPrimitive* info = &rSurface.surfaceInfo;
			GFX->drawIndexedPrimitive( info->type, info->startVertex, info->minIndex, info->numVertices, info->startIndex, info->numPrimitives );
		}
	}

	GFX->disableShaders();
	debugNormalRenderLines(zoneVis);
}

void Interior::debugShowDetail(const ZoneVisDeterminer& zoneVis)
{
	debugShowSurfaceFlag(zoneVis, SurfaceDetail, ColorF(1.0f, 0.0f, 0.0f));
}

void Interior::debugShowAmbiguous(const ZoneVisDeterminer& zoneVis)
{
	debugShowSurfaceFlag(zoneVis, SurfaceAmbiguous, ColorF(0.0f, 1.0f, 0.0f));
}

void Interior::debugShowOrphan(const ZoneVisDeterminer& zoneVis)
{
	debugShowSurfaceFlag(zoneVis, SurfaceOrphan, ColorF(0.0f, 0.0f, 1.0f));
}

void Interior::debugShowOutsideVisible(const ZoneVisDeterminer& zoneVis)
{
	debugShowSurfaceFlag(zoneVis, SurfaceOutsideVisible, ColorF(1.0f, 0.0f, 0.0f));
}

void Interior::debugShowPortalZones(const ZoneVisDeterminer& zoneVis)
{
	preDebugRender();

   for (U32 i = 0; i < mZones.size(); i++)
   {
      U8* color;
      if (i == 0)
         color = interiorDebugColors[0];
      else
         color = interiorDebugColors[(i % 13) + 1];

      for (U32 j = mZones[i].surfaceStart; j < mZones[i].surfaceStart + mZones[i].surfaceCount; j++)
      {
         Surface& rSurface = mSurfaces[mZoneSurfaces[j]];
			ColorF c((F32) color[0] / 255.0f, (F32) color[1] / 255.0f, (F32) color[2] / 255.0f);

         mDebugShaderConsts->setSafe(mDebugShaderShadeColorSC, c);

         GFX->setShaderConstBuffer(mDebugShaderConsts);

			GFXPrimitive* info = &rSurface.surfaceInfo;
			GFX->drawIndexedPrimitive( info->type, info->startVertex, info->minIndex, info->numVertices, info->startIndex, info->numPrimitives );
		}
	}

	GFX->disableShaders();
	debugRenderPortals();
	debugNormalRenderLines(zoneVis);
}

// Render portals
void Interior::debugRenderPortals()
{
   // Set our portal rendering state block
   GFX->setStateBlock(mInteriorDebugPortalSB);

   for (U32 i = 0; i < mPortals.size(); i++)
   {
      const Portal& rPortal = mPortals[i];

      for (U16 j = 0; j < rPortal.triFanCount; j++)
      {
         const TriFan& rFan = mWindingIndices[rPortal.triFanStart + j];
         U32 k;

			PrimBuild::color4f(0.75f, 0.5f, 0.75f, 0.45f);
         PrimBuild::begin(GFXTriangleFan, rFan.windingCount);

         for (k = 0; k < rFan.windingCount; k++)
            PrimBuild::vertex3fv(mPoints[mWindings[rFan.windingStart + k]].point);

         PrimBuild::end();

         PrimBuild::color4f(0, 0, 1, 1);
         PrimBuild::begin(GFXLineStrip, rFan.windingCount+1);

         for (k = 0; k < rFan.windingCount; k++)
            PrimBuild::vertex3fv(mPoints[mWindings[rFan.windingStart + k]].point);

         PrimBuild::vertex3fv(mPoints[mWindings[rFan.windingStart]].point);

         PrimBuild::end();
      }
   }
}

void Interior::debugShowCollisionFans(const ZoneVisDeterminer& zoneVis)
{
   // Set our "base debug states" (no texture)
   GFX->setStateBlock(mInteriorDebugNoneSB);

	for (U32 i = 0; i < mZones.size(); i++)
   {
      if (zoneVis.isZoneVisible(i) == false)
         continue;

      for( U32 j=0; j<mZones[i].surfaceCount; j++ )
      {
         U32 surfaceIndex = mZoneSurfaces[mZones[i].surfaceStart + j];
         Surface& rSurface = mSurfaces[ surfaceIndex ];
			U32 numIndices;
			U32 fanIndices[32];
			collisionFanFromSurface(rSurface, fanIndices, &numIndices);

			// Filled brush
			PrimBuild::color3f(1.0f, 1.0f, 1.0f);
			PrimBuild::begin(GFXTriangleFan, numIndices);
			for (U32 i = 0; i < numIndices; i++)
				PrimBuild::vertex3fv(mPoints[fanIndices[i]].point);
			PrimBuild::end();

			// Outline
			PrimBuild::color3f(0.0f, 0.0f, 0.0f);
			PrimBuild::begin(GFXLineStrip, numIndices+1);
			for (U32 i = 0; i < numIndices; i++)
				PrimBuild::vertex3fv(mPoints[fanIndices[i]].point);
			if (numIndices > 0)
				PrimBuild::vertex3fv(mPoints[fanIndices[0]].point);
			PrimBuild::end();

			// Normal
			PrimBuild::color3f(1, 0, 0);
			PrimBuild::begin(GFXLineList, numIndices * 2);
			for (U32 j = 0; j < numIndices; j++)
         {
				Point3F up   = mPoints[fanIndices[j]].point;
				Point3F norm = getPlane(rSurface.planeIndex);
				if (planeIsFlipped(rSurface.planeIndex))
					up -= norm * 0.4f;
				else
					up += norm * 0.4f;

				PrimBuild::vertex3fv(mPoints[fanIndices[j]].point);
				PrimBuild::vertex3fv(up);
			}
			PrimBuild::end();
		}
	}
}

// This doesn't show strip (they don't go to the card that way)
// But it does show the batches of primitives we send.
void Interior::debugShowStrips(const ZoneVisDeterminer& zoneVis)
{
	// Set up our rendering states.
	preDebugRender();

	for (U32 i = 0; i < mZones.size(); i++)
   {
      if (zoneVis.isZoneVisible(i) == false)
         continue;

      for( U32 j=0; j<mZoneRNList[i].renderNodeList.size(); j++ )
      {
         RenderNode &node = mZoneRNList[i].renderNodeList[j];
			U32 index = (i+j) % 14;
			ColorF col((F32)interiorDebugColors[index][0] / 255.0f, (F32)interiorDebugColors[index][1] / 255.0f, 
				(F32)interiorDebugColors[index][2] / 255.0f);

         mDebugShaderConsts->setSafe(mDebugShaderShadeColorSC, col);

         GFX->setShaderConstBuffer(mDebugShaderConsts);

			GFX->drawPrimitive(node.primInfoIndex);
		}
	}

	GFX->disableShaders();
	debugNormalRenderLines(zoneVis);
}

void Interior::debugShowDetailLevel(const ZoneVisDeterminer& zoneVis)
{
	// Set up our rendering states.
	preDebugRender();

	U32 index = getDetailLevel();
	ColorF col((F32)interiorDebugColors[index][0] / 255.0f, (F32)interiorDebugColors[index][1] / 255.0f, 
		(F32)interiorDebugColors[index][2] / 255.0f);

   mDebugShaderConsts->setSafe(mDebugShaderShadeColorSC, col);

   GFX->setShaderConstBuffer(mDebugShaderConsts);

	for (U32 i = 0; i < mZones.size(); i++)
   {
      if (zoneVis.isZoneVisible(i) == false)
         continue;

      for( U32 j=0; j<mZoneRNList[i].renderNodeList.size(); j++ )
      {
         RenderNode &node = mZoneRNList[i].renderNodeList[j];
			GFX->drawPrimitive(node.primInfoIndex);
		}
	}

	GFX->disableShaders();
	debugNormalRenderLines(zoneVis);
}

void Interior::debugShowHullSurfaces()
{
   // Set our "base debug states" (no texture)
   GFX->setStateBlock(mInteriorDebugNoneSB);

   for (U32 i = 0; i < mConvexHulls.size(); i++)
   {
      const ConvexHull& rHull = mConvexHulls[i];

      for (U32 j = rHull.surfaceStart; j < rHull.surfaceCount + rHull.surfaceStart; j++)
      {
         U32 index = mHullSurfaceIndices[j];

         if (!isNullSurfaceIndex(index))
         {
            const Interior::Surface& rSurface = mSurfaces[index];
            U32 fanVerts[32];
            U32 numVerts;
            collisionFanFromSurface(rSurface, fanVerts, &numVerts);

            PrimBuild::color3i(interiorDebugColors[(i%13)+1][0], interiorDebugColors[(i%13)+1][1], 
					interiorDebugColors[(i%13)+1][2]);
            Point3F center(0, 0, 0);
            PrimBuild::begin(GFXTriangleFan, numVerts);
            for (U32 k = 0; k < numVerts; k++)
            {
               PrimBuild::vertex3fv(mPoints[fanVerts[k]].point);
               center += mPoints[fanVerts[k]].point;
            }
            PrimBuild::end();
            center /= F32(numVerts);
            PrimBuild::color3f(0, 0, 0);
            lineLoopFromStrip(mPoints, mWindings, rSurface.windingStart, rSurface.windingCount);

            PlaneF plane;
            plane.set(mPoints[fanVerts[0]].point, mPoints[fanVerts[1]].point, mPoints[fanVerts[2]].point);
            PrimBuild::begin(GFXLineList, 2);
               PrimBuild::vertex3fv(center);
               PrimBuild::vertex3fv(center + (plane * 0.25));
            PrimBuild::end();
         }
      }
   }
}

void Interior::debugDefaultRender(const ZoneVisDeterminer& zoneVis, SceneData &sgData, InteriorInstance *intInst)
{
   // Set up a state block with two textures enabled
   GFX->setStateBlock(mInteriorDebugTwoTextureSB);

   for( U32 i=0; i<getNumZones(); i++ )
   {
      if (zoneVis.isZoneVisible(i) == false)
         continue;

      for( U32 j=0; j<mZoneRNList[i].renderNodeList.size(); j++ )
      {
         RenderNode &node = mZoneRNList[i].renderNodeList[j];
         static U16 curBaseTexIndex = 0;

         // setup base map
			if( node.baseTexIndex != U16_MAX )
				curBaseTexIndex = node.baseTexIndex;

			// setup lightmap
			if( node.lightMapIndex != U8(-1) )
				sgData.lightmap = gInteriorLMManager.getHandle(mLMHandle, intInst->getLMHandle(), node.lightMapIndex );

         GFX->setTexture( 0, mMaterialList->getDiffuseTexture( curBaseTexIndex ));
         GFX->setTexture(1, sgData.lightmap);
			GFX->drawPrimitive( node.primInfoIndex );
		}
	}
}

void Interior::debugShowNullSurfaces(const ZoneVisDeterminer& zoneVis, SceneData &sgData, InteriorInstance *intInst)
{	
	debugDefaultRender(zoneVis, sgData, intInst);
   
   PrimBuild::color3f(1.0f, 0.0f, 0.0f);
   for (U32 i = 0; i < mNullSurfaces.size(); i++)
   {
      const NullSurface& rSurface = mNullSurfaces[i];

      PrimBuild::begin(GFXTriangleFan, rSurface.windingCount);
      for (U32 k = 0; k < rSurface.windingCount; k++)
      {
         PrimBuild::vertex3fv(mPoints[mWindings[rSurface.windingStart+k]].point);
      }
      PrimBuild::end();
	}
}

void Interior::debugShowVehicleHullSurfaces(const ZoneVisDeterminer& zoneVis, SceneData &sgData, 
												 InteriorInstance *intInst)
{
	debugDefaultRender(zoneVis, sgData, intInst);

   PrimBuild::color3f(1.0f, 0.0f, 0.0f);
   for (U32 i = 0; i < mVehicleNullSurfaces.size(); i++)
   {
      const NullSurface& rSurface = mNullSurfaces[i];
      PrimBuild::begin(GFXTriangleFan, rSurface.windingCount);
      for (U32 k = 0; k < rSurface.windingCount; k++)
      {
         PrimBuild::vertex3fv(mPoints[mWindings[rSurface.windingStart+k]].point);
      }
   }
}

void Interior::debugShowTexturesOnly(const ZoneVisDeterminer& zoneVis, SceneData &sgData, 
												 InteriorInstance *intInst)
{
   // Set up a state block with one texture unit enabled
   GFX->setStateBlock(mInteriorDebugTextureSB);

   for( U32 i=0; i<getNumZones(); i++ )
   {
      if (zoneVis.isZoneVisible(i) == false)
         continue;

      for( U32 j=0; j<mZoneRNList[i].renderNodeList.size(); j++ )
      {
         RenderNode &node = mZoneRNList[i].renderNodeList[j];
			static U16 curBaseTexIndex = 0;

			// setup base map
			if ( node.baseTexIndex != U16_MAX )
				curBaseTexIndex = node.baseTexIndex;

         GFX->setTexture( 0, mMaterialList->getDiffuseTexture( curBaseTexIndex ));
			GFX->drawPrimitive( node.primInfoIndex );
		}
	}
}

void Interior::debugShowLargeTextures(const ZoneVisDeterminer& zoneVis, SceneData &sgData, 
												 InteriorInstance *intInst)
{
	preDebugRender();

   for( U32 i=0; i<getNumZones(); i++ )
   {
      if (zoneVis.isZoneVisible(i) == false)
         continue;

      for( U32 j=0; j<mZoneRNList[i].renderNodeList.size(); j++ )
      {
         RenderNode &node = mZoneRNList[i].renderNodeList[j];
			static U16 curBaseTexIndex = 0;

			// setup base map
			if( node.baseTexIndex != U16_MAX )
				curBaseTexIndex = node.baseTexIndex;

			GFXTexHandle t = mMaterialList->getDiffuseTexture( curBaseTexIndex );
			ColorF texSizeColor(1.0f, 1.0f, 1.0f, 1.0f);
			if (t)
         {
				U32 width  = t.getWidth();
				U32 height = t.getHeight();

            if (width <= 256 && height <= 256)
               texSizeColor = ColorF(0.25f, 0.25f, 1.0f); // small texture
            else if (width <= 512 && height <= 512)
               texSizeColor = ColorF(0.25f, 1.0f, 0.25f); // medium texture
            else
               texSizeColor = ColorF(1.0f, 0.25f, 0.25f); // large texture
			}

         mDebugShaderConsts->setSafe(mDebugShaderShadeColorSC, texSizeColor);

         GFX->setShaderConstBuffer(mDebugShaderConsts);

			GFX->setTexture( 0, t);
			GFX->drawPrimitive( node.primInfoIndex );
		}
	} 
}

void Interior::debugShowLightmaps(const ZoneVisDeterminer& zoneVis, SceneData &sgData, InteriorInstance *intInst)
{
	GFX->setTexture(0, mDebugTexture);
	
   // Set up a state block with two textures enabled
   GFX->setStateBlock(mInteriorDebugTwoTextureSB);

   for( U32 i=0; i<getNumZones(); i++ )
   {
      if (zoneVis.isZoneVisible(i) == false)
         continue;

      for( U32 j=0; j<mZoneRNList[i].renderNodeList.size(); j++ )
      {
         RenderNode &node = mZoneRNList[i].renderNodeList[j];
			// setup lightmap
			if( node.lightMapIndex != U8(-1) )
				sgData.lightmap = gInteriorLMManager.getHandle(mLMHandle, intInst->getLMHandle(), node.lightMapIndex );

         GFX->setTexture(1, sgData.lightmap);			
			GFX->drawPrimitive( node.primInfoIndex );
		}
	}
}

#endif
