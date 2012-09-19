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
#include "interior/interiorSimpleMesh.h"

#include "interior/interiorLMManager.h"
#include "interior/interior.h"
#include "console/console.h"
#include "scene/sceneObject.h"
#include "math/mathIO.h"
#include "materials/matInstance.h"
#include "materials/materialManager.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "ts/tsShape.h"
#include "gfx/bitmap/gBitmap.h"


Vector<MeshRenderInst *> *InteriorSimpleMesh::renderInstList = new Vector<MeshRenderInst *>();


// Checks for polygon level collision with given planes
U32 _whichSide(PlaneF pln, Point3F* verts)
{
	Point3F currv, nextv;
	S32 csd, nsd;

	// Find out which side the first vert is on
	U32 side = PlaneF::On;
	currv = verts[0];
	csd = pln.whichSide(currv);
	if(csd != PlaneF::On)
		side = csd;

	for(U32 k = 1; k < 3; k++)
	{
		nextv = verts[k];
		nsd = pln.whichSide(nextv);
		if((csd == PlaneF::Back && nsd == PlaneF::Front) ||
			(csd == PlaneF::Front && nsd == PlaneF::Back))
			return 2;
		else if (nsd != PlaneF::On)
			side = nsd;
		currv = nextv;
		csd = nsd;
	}

	// Loop back to the first vert
	nextv = verts[0];
	nsd = pln.whichSide(nextv);
	if((csd == PlaneF::Back && nsd == PlaneF::Front) ||
		(csd == PlaneF::Front && nsd == PlaneF::Back))
		return 2;
	else if(nsd != PlaneF::On)
		side = nsd;
	return side;

}


//bool InteriorSimpleMesh::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
//{
//	bool found = false;
//	F32 best_t = F32_MAX;
//	Point3F best_normal = Point3F(0, 0, 1);
//	Point3F dir = end - start;
//
//	for(U32 p=0; p<primitives.size(); p++)
//	{
//		primitive &prim = primitives[p];
//		for(U32 t=2; t<prim.count; t++)
//		{
//			Point3F &v1 = verts[prim.start+t-2];
//			Point3F &v2 = verts[prim.start+t-1];
//			Point3F &v3 = verts[prim.start+t];
//
//			F32 cur_t = 0;
//			Point2F b;
//
//			if(castRayTriangle(start, dir, v1, v2, v3, cur_t, b))
//			{
//				if(cur_t < best_t)
//				{
//					best_t = cur_t;
//					best_normal = norms[prim.start+t];
//					found = true;
//				}
//			}
//		}
//	}
//
//	if(found && info)
//	{
//		info->t = best_t;
//		info->normal = best_normal;
//		info->material = 0;
//	}
//
//	return found;
//}

bool InteriorSimpleMesh::castPlanes(PlaneF left, PlaneF right, PlaneF top, PlaneF bottom)
{
	for(U32 p=0; p<primitives.size(); p++)
	{
		primitive &prim = primitives[p];
		for(U32 t=2; t<prim.count; t++)
		{
			Point3F v[3];
			v[0] = verts[prim.start+t-2];
			v[1] = verts[prim.start+t-1];
			v[2] = verts[prim.start+t];

			if(_whichSide(left, v) == PlaneF::Front)
				continue;
			if(_whichSide(right, v) == PlaneF::Front)
				continue;
			if(_whichSide(top, v) == PlaneF::Front)
				continue;
			if(_whichSide(bottom, v) == PlaneF::Front)
				continue;

			return true;
		}
	}

	return false;
}

void InteriorSimpleMesh::render( SceneRenderState* state, 
                                 const MeshRenderInst &copyinst, 
                                 U32 interiorlmhandle, 
                                 U32 instancelmhandle,
								         InteriorInstance* intInst )
{
   /*
   AssertFatal((primBuff->mPrimitiveCount == packedPrimitives.size()), "Primitive mismatch");

   renderInstList->clear();

	for(S32 i=0; i<packedPrimitives.size(); i++)
	{
		primitive &draw = packedPrimitives[i];
      MeshRenderInst *inst = state->getRenderPass()->allocInst<MeshRenderInst>();
      *inst = copyinst;

      inst->matInst = materialList->getMaterialInst(draw.diffuseIndex);
      if(!inst->matInst)
         inst->matInst = MATMGR->getWarningMatInstance();
      if(!inst->matInst)
         continue;

      inst->primBuffIndex = i;
      inst->primBuff = &primBuff;
      inst->vertBuff = &vertBuff;

	  if(draw.alpha)
	  {
		inst->translucentSort = true;
		inst->type = RenderPassManager::RIT_Translucent;
	  }

      inst->lightmap = gInteriorLMManager.getHandle(interiorlmhandle, instancelmhandle, draw.lightMapIndex);

      state->getRenderPass()->addInst(inst);
	  renderInstList->push_back(inst);
	}

	if(lightingplugin && renderInstList->size() > 0)
	{
		if(lightingplugin->interiorInstInit(intInst, this))
		{
			if(lightingplugin->allZoneInit())
			{
				Vector<MeshRenderInst *> &list = *renderInstList;

				// clone the origial instances to avoid damaging the originals' data
				for(int i=0; i<renderInstList->size(); i++)
				{
					MeshRenderInst *inst = state->getRenderPass()->allocInst<MeshRenderInst>();
					const MeshRenderInst *oldinst = list[i];
					*inst = *oldinst;
					list[i] = inst;
				}

				lightingplugin->processRI(state, list);
			}
		}
	}
   */
}

bool InteriorSimpleMesh::read(Stream& stream)
{
   // Simple serialization
   S32 vectorSize = 0;

   // Primitives
   stream.read(&vectorSize);
   primitives.setSize(vectorSize);
   for (U32 i = 0; i < primitives.size(); i++)
   {
      stream.read(&primitives[i].alpha);
		stream.read(&primitives[i].texS);
		stream.read(&primitives[i].texT);
		stream.read(&primitives[i].diffuseIndex);
		stream.read(&primitives[i].lightMapIndex);
		stream.read(&primitives[i].start);
		stream.read(&primitives[i].count);
		
      mathRead(stream, &primitives[i].lightMapEquationX);
      mathRead(stream, &primitives[i].lightMapEquationY);
      mathRead(stream, &primitives[i].lightMapOffset);
      mathRead(stream, &primitives[i].lightMapSize);
   }

   // Indices
   stream.read(&vectorSize);
   indices.setSize(vectorSize);
   for (U32 i = 0; i < indices.size(); i++)
      stream.read(&indices[i]);

   // Vertices
   stream.read(&vectorSize);
   verts.setSize(vectorSize);
   for (U32 i = 0; i < verts.size(); i++)
      mathRead(stream, &verts[i]);

   // Normals
   stream.read(&vectorSize);
   norms.setSize(vectorSize);
   for (U32 i = 0; i < norms.size(); i++)
      mathRead(stream, &norms[i]);

   // Diffuse UVs
   stream.read(&vectorSize);
   diffuseUVs.setSize(vectorSize);
   for (U32 i = 0; i < diffuseUVs.size(); i++)
      mathRead(stream, &diffuseUVs[i]);

   // Lightmap UVs
   stream.read(&vectorSize);
   lightmapUVs.setSize(vectorSize);
   for (U32 i = 0; i < lightmapUVs.size(); i++)
      mathRead(stream, &lightmapUVs[i]);

   // Material list
   bool hasMaterialList = false;
   stream.read(&hasMaterialList);
   if (hasMaterialList)
   {
      // Since we are doing this externally to a TSShape read we need to
      // make sure that our read version is the same as our write version.
      // It is possible that it was changed along the way by a loaded TSShape.
      TSShape::smReadVersion = 25;

      if (materialList)
         delete materialList;

      materialList = new TSMaterialList;
      materialList->read(stream);
   }
   else
      materialList = NULL;

   // Diffuse bitmaps
   stream.read(&vectorSize);
   for (U32 i = 0; i < vectorSize; i++)
   {
      // need to read these
      bool hasBitmap = false;
      stream.read(&hasBitmap);
      if(hasBitmap)
      {
         GBitmap* bitMap = new GBitmap;
         bitMap->readBitmap("png",stream);
         delete bitMap;
      }
   }

   // Misc data
   stream.read(&hasSolid);
	stream.read(&hasTranslucency);
	mathRead(stream, &bounds);
   mathRead(stream, &transform);
   mathRead(stream, &scale);

   calculateBounds();
   buildBuffers();

   return true;
}

bool InteriorSimpleMesh::write(Stream& stream) const
{
   // Simple serialization
   // Primitives
   stream.write(primitives.size());
   for (U32 i = 0; i < primitives.size(); i++)
   {
      stream.write(primitives[i].alpha);
		stream.write(primitives[i].texS);
		stream.write(primitives[i].texT);
		stream.write(primitives[i].diffuseIndex);
		stream.write(primitives[i].lightMapIndex);
		stream.write(primitives[i].start);
		stream.write(primitives[i].count);

      mathWrite(stream, primitives[i].lightMapEquationX);
      mathWrite(stream, primitives[i].lightMapEquationY);
      mathWrite(stream, primitives[i].lightMapOffset);
      mathWrite(stream, primitives[i].lightMapSize);
   }

   // Indices
   stream.write(indices.size());
   for (U32 i = 0; i < indices.size(); i++)
      stream.write(indices[i]);

   // Vertices
   stream.write(verts.size());
   for (U32 i = 0; i < verts.size(); i++)
      mathWrite(stream, verts[i]);

   // Normals
   stream.write(norms.size());
   for (U32 i = 0; i < norms.size(); i++)
      mathWrite(stream, norms[i]);

   // Diffuse UVs
   stream.write(diffuseUVs.size());
   for (U32 i = 0; i < diffuseUVs.size(); i++)
      mathWrite(stream, diffuseUVs[i]);

   // Lightmap UVs
   stream.write(lightmapUVs.size());
   for (U32 i = 0; i < lightmapUVs.size(); i++)
      mathWrite(stream, lightmapUVs[i]);

   // Material list
   if (materialList)
   {
      stream.write(true);
      materialList->write(stream);
   }
   else
      stream.write(false);

   // Diffuse bitmaps
   if (!materialList)
      stream.write(0);
   else
   {
      stream.write(materialList->size());

      for (U32 i = 0; i < materialList->size(); i++)
      {
         GFXTexHandle handle(materialList->getDiffuseTexture(i));

         if (handle.isValid())
         {
            GBitmap* bitMap = handle.getBitmap();

            if (bitMap)
            {
               stream.write(true);
               bitMap->writeBitmap("png",stream);
            }
            else
               stream.write(false);
         }
         else
            stream.write(false);
      }
   }

   // Misc data
   stream.write(hasSolid);
	stream.write(hasTranslucency);
	mathWrite(stream, bounds);
   mathWrite(stream, transform);
   mathWrite(stream, scale);

   return true;
}

void InteriorSimpleMesh::buildBuffers()
{
   bool flipped = false;

   MatrixF trans = transform;
   trans.scale(scale);

   Point3F r0, r1, r2;
   trans.getRow(0, &r0);
   trans.getRow(1, &r1);
   trans.getRow(2, &r2);
   F32 det = r0.x * (r1.y * r2.z - r1.z * r2.y) -
      r0.y * (r1.x * r2.z - r1.z * r2.x) +
      r0.z * (r1.x * r2.y - r1.y * r2.x);
   flipped = det < 0.0f;

   // setup the repack vectors
   packedIndices.clear();
   packedPrimitives.clear();
   packedIndices.reserve(indices.size() * 2);
   packedPrimitives.reserve(primitives.size());

   Vector<bool> addedprim;
   addedprim.setSize(primitives.size());
   dMemset(addedprim.address(), 0, (addedprim.size() * sizeof(bool)));

   Vector<Point3F> tang;
   Vector<Point3F> binorm;
   tang.setSize(verts.size());
   binorm.setSize(verts.size());
   dMemset(tang.address(), 0, (tang.size() * sizeof(Point3F)));
   dMemset(binorm.address(), 0, (binorm.size() * sizeof(Point3F)));

   // fill the repack vectors
   for(U32 p=0; p<primitives.size(); p++)
   {
      if(addedprim[p])
         continue;
      addedprim[p] = true;

      const primitive &primold = primitives[p];
      packedPrimitives.increment();
      primitive &primnew = packedPrimitives.last();

      primnew.start = packedIndices.size();
      primnew.count = 0;

      primnew.alpha = primold.alpha;
      primnew.diffuseIndex = primold.diffuseIndex;
      primnew.lightMapIndex = primold.lightMapIndex;

      packPrimitive(primnew, primold, packedIndices, flipped, tang, binorm);

      for(U32 gp=(p+1); gp<primitives.size(); gp++)
      {
         if(addedprim[gp])
            continue;

         const primitive &primgrouped = primitives[gp];
         if((primnew.alpha != primgrouped.alpha) || (primnew.diffuseIndex != primgrouped.diffuseIndex) ||
            (primnew.lightMapIndex != primgrouped.lightMapIndex))
            continue;

         addedprim[gp] = true;
         packPrimitive(primnew, primgrouped, packedIndices, flipped, tang, binorm);
      }
   }

   // normalize
   for(U32 i=0; i<tang.size(); i++)
   {
      tang[i].normalize();
      binorm[i].normalize();
   }

   // verify...
   F32 oldcount = 0;
   F32 newcount = 0;
   for(U32 i=0; i<primitives.size(); i++)
      oldcount += F32(primitives[i].count) - 2.0f;
   for(U32 i=0; i<packedPrimitives.size(); i++)
      newcount += F32(packedPrimitives[i].count) / 3.0f;

   AssertFatal((oldcount == newcount), "Invalid primitive pack.");

   // build the GFX buffers...
   Vector<GFXPrimitive> packedprims;
   packedprims.setSize(packedPrimitives.size());

   for(U32 i=0; i<packedprims.size(); i++)
   {
      GFXPrimitive &p = packedprims[i];
      primitive &prim = packedPrimitives[i];

      p.type = GFXTriangleList;
      p.numPrimitives = prim.count / 3;
      p.startIndex = prim.start;

      p.minIndex = U32_MAX;
      U32 maxindex = 0;
      for(U32 ii=0; ii<prim.count; ii++)
      {
         if(p.minIndex > packedIndices[prim.start + ii])
            p.minIndex = packedIndices[prim.start + ii];
         if(maxindex < packedIndices[prim.start + ii])
            maxindex = packedIndices[prim.start + ii];
      }

      // D3D voodoo - not the actual numverts, only the max span (maxindex - minindex) - this needs a better variable name...
      p.numVertices = (maxindex - p.minIndex) + 1;
   }

   // create vb style sysmem buffer
   Vector<GFXVertexPNTTB> packedverts;
   packedverts.setSize(verts.size());

   // fill it
   for(U32 i=0; i<packedverts.size(); i++)
   {
      GFXVertexPNTTB &v = packedverts[i];

      trans.mulP(verts[i], &v.point);
      trans.mulV(norms[i], &v.normal);
      trans.mulV(tang[i], &v.T);
      trans.mulV(binorm[i], &v.B);

      v.texCoord = diffuseUVs[i];
      v.texCoord2 = lightmapUVs[i];

      v.T = v.T - v.normal * mDot(v.normal, v.T);
      v.T.normalize();

      Point3F b;
      mCross(v.normal, v.T, &b);
      b *= (mDot(b, v.B) < 0.0F) ? -1.0F : 1.0F;
      v.B = b;
   }

   // set up the vb and fill all at once
   vertBuff.set(GFX, packedverts.size(), GFXBufferTypeStatic);
   GFXVertexPNTTB *rawvb = vertBuff.lock();
   dMemcpy(rawvb, packedverts.address(), packedverts.size() * sizeof(GFXVertexPNTTB));
   vertBuff.unlock();

   // set up the pb and fill all at once
   U16 *rawi;
   GFXPrimitive *rawp;
   primBuff.set(GFX, packedIndices.size(), packedprims.size(), GFXBufferTypeStatic);
   primBuff.lock(&rawi, &rawp);
   dMemcpy(rawi, packedIndices.address(), packedIndices.size() * sizeof(U16));
   dMemcpy(rawp, packedprims.address(), packedprims.size() * sizeof(GFXPrimitive));
   primBuff.unlock();
}

void InteriorSimpleMesh::buildTangent(U32 i0, U32 i1, U32 i2, Vector<Point3F> &tang, Vector<Point3F> &binorm)
{
   const Point3F& va = verts[i0];
   const Point3F& vb = verts[i1];
   const Point3F& vc = verts[i2];
   const Point2F& uva = diffuseUVs[i0];
   const Point2F& uvb = diffuseUVs[i1];
   const Point2F& uvc = diffuseUVs[i2];

   float x1 = vb.x - va.x;
   float x2 = vc.x - va.x;
   float y1 = vb.y - va.y;
   float y2 = vc.y - va.y;
   float z1 = vb.z - va.z;
   float z2 = vc.z - va.z;
   float s1 = uvb.x - uva.x;
   float s2 = uvc.x - uva.x;
   float t1 = uvb.y - uva.y;
   float t2 = uvc.y - uva.y;

   F32 denom = (s1 * t2 - s2 * t1);
   if(fabs(denom) < 0.0001)
      return;

   float r = 1.0F / denom;
   Point3F s((t2 * x1 - t1 * x2) * r, 
      (t2 * y1 - t1 * y2) * r, 
      (t2 * z1 - t1 * z2) * r);
   Point3F t((s1 * x2 - s2 * x1) * r, 
      (s1 * y2 - s2 * y1) * r, 
      (s1 * z2 - s2 * z1) * r);

   tang[i0] += s;
   tang[i1] += s;
   tang[i2] += s;
   binorm[i0] += t;
   binorm[i1] += t;
   binorm[i2] += t;
}

void InteriorSimpleMesh::packPrimitive(primitive &primnew, const primitive &primold, Vector<U16> &indicesnew,
                                          bool flipped, Vector<Point3F> &tang, Vector<Point3F> &binorm)
{
   // convert from strip to list and add to primnew
   for(U32 p=2; p<primold.count; p++)
   {
      bool direction = (p & 0x1);
      U32 i0, i1, i2;

      if(flipped)
         direction = !direction;
      if(direction)
      {
         i0 = indices[p + primold.start - 1];
         i1 = indices[p + primold.start - 2];
         i2 = indices[p + primold.start];
      }
      else
      {
         i0 = indices[p + primold.start - 2];
         i1 = indices[p + primold.start - 1];
         i2 = indices[p + primold.start];
      }
      
      indicesnew.push_back(i0);
      indicesnew.push_back(i1);
      indicesnew.push_back(i2);

      buildTangent(i0, i1, i2, tang, binorm);

      primnew.count += 3;
   }
}

bool InteriorSimpleMesh::prepForRendering(const char* path)
{
   //materialList->load(InteriorTexture, path, false);
   materialList->mapMaterials();

   // GFX2_RENDER_MERGE
   materialList->initMatInstances( MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPNTTB>() );

   return true;
}
