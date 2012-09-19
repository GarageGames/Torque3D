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

#ifndef _INTERIORSIMPLEMESH_H_
#define _INTERIORSIMPLEMESH_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _MBOX_H_
#include "math/mBox.h"
#endif
#ifndef _TSMATERIALLIST_H_
#include "ts/tsMaterialList.h"
#endif
#ifndef _RENDERPASSMANAGER_H_
#include "renderInstance/renderPassManager.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif

class InteriorInstance;


class InteriorSimpleMesh
{
public:
	class primitive
	{
	public:
		bool alpha;
		U32 texS;
		U32 texT;
		S32 diffuseIndex;
		S32 lightMapIndex;
		U32 start;
		U32 count;

      // used to relight the surface in-engine...
      PlaneF lightMapEquationX;
      PlaneF lightMapEquationY;
      Point2I lightMapOffset;
      Point2I lightMapSize;

		primitive()
		{
			alpha = false;
			texS = GFXAddressWrap;
			texT = GFXAddressWrap;
			diffuseIndex = 0;
			lightMapIndex = 0;
			start = 0;
			count = 0;

         lightMapEquationX = PlaneF(0, 0, 0, 0);
         lightMapEquationY = PlaneF(0, 0, 0, 0);
         lightMapOffset = Point2I(0, 0);
         lightMapSize = Point2I(0, 0);
		}
	};

	InteriorSimpleMesh()
	{
      VECTOR_SET_ASSOCIATION( packedIndices );
      VECTOR_SET_ASSOCIATION( packedPrimitives );
      VECTOR_SET_ASSOCIATION( indices );
      VECTOR_SET_ASSOCIATION( verts );
      VECTOR_SET_ASSOCIATION( norms );
      VECTOR_SET_ASSOCIATION( diffuseUVs );
      VECTOR_SET_ASSOCIATION( lightmapUVs );

      materialList = NULL;
		clear();
	}
	~InteriorSimpleMesh(){clear();}
	void clear(bool wipeMaterials = true)
	{
      vertBuff = NULL;
      primBuff = NULL;
      packedIndices.clear();
      packedPrimitives.clear();

		hasSolid = false;
		hasTranslucency = false;
		bounds = Box3F(-1, -1, -1, 1, 1, 1);
      transform.identity();
      scale.set(1.0f, 1.0f, 1.0f);

		primitives.clear();
		indices.clear();
		verts.clear();
		norms.clear();
		diffuseUVs.clear();
		lightmapUVs.clear();

		if(wipeMaterials && materialList)
			delete materialList;

      if (wipeMaterials)
		   materialList = NULL;
	}

   void render(   SceneRenderState* state, 
                  const MeshRenderInst &copyinst, 
                  U32 interiorlmhandle, 
                  U32 instancelmhandle,
	               InteriorInstance* intInst );

	void calculateBounds()
	{
		bounds = Box3F(F32_MAX, F32_MAX, F32_MAX, -F32_MAX, -F32_MAX, -F32_MAX);
		for(U32 i=0; i<verts.size(); i++)
		{
			bounds.maxExtents.setMax(verts[i]);
			bounds.minExtents.setMin(verts[i]);
		}
	}

   Vector<U16> packedIndices;
   Vector<primitive> packedPrimitives;/// tri-list instead of strips
   GFXVertexBufferHandle<GFXVertexPNTTB> vertBuff;
   GFXPrimitiveBufferHandle primBuff;
   void buildBuffers();
   void buildTangent(U32 i0, U32 i1, U32 i2, Vector<Point3F> &tang, Vector<Point3F> &binorm);
   void packPrimitive(primitive &primnew, const primitive &primold, Vector<U16> &indicesnew,
      bool flipped, Vector<Point3F> &tang, Vector<Point3F> &binorm);
   bool prepForRendering(const char *path);

	bool hasSolid;
	bool hasTranslucency;
	Box3F bounds;
   MatrixF transform;
   Point3F scale;

	Vector<primitive> primitives;

	// same index relationship...
	Vector<U16> indices;
	Vector<Point3F> verts;
	Vector<Point3F> norms;
	Vector<Point2F> diffuseUVs;
	Vector<Point2F> lightmapUVs;

	TSMaterialList *materialList;

   bool containsPrimitiveType(bool translucent)
   {
      for(U32 i=0; i<primitives.size(); i++)
      {
         if(primitives[i].alpha == translucent)
            return true;
      }
      return false;
   }

	void copyTo(InteriorSimpleMesh &dest)
	{
		dest.clear();
		dest.hasSolid = hasSolid;
		dest.hasTranslucency = hasTranslucency;
		dest.bounds = bounds;
      dest.transform = transform;
      dest.scale = scale;
		dest.primitives = primitives;
		dest.indices = indices;
		dest.verts = verts;
		dest.norms = norms;
		dest.diffuseUVs = diffuseUVs;
		dest.lightmapUVs = lightmapUVs;

		if(materialList)
			dest.materialList = new TSMaterialList(materialList);
	}
	//bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
	bool castPlanes(PlaneF left, PlaneF right, PlaneF top, PlaneF bottom);

   bool read(Stream& stream);
   bool write(Stream& stream) const;

   private:
	   static Vector<MeshRenderInst *> *renderInstList;
};

#endif //_INTERIORSIMPLEMESH_H_

