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

#ifndef _COLLADA_APPMESH_H_
#define _COLLADA_APPMESH_H_

#ifndef _TDICTIONARY_H_
#include "core/tDictionary.h"
#endif
#ifndef _APPMESH_H_
#include "ts/loader/appMesh.h"
#endif
#ifndef _TSSHAPELOADER_H_
#include "ts/loader/tsShapeLoader.h"
#endif
#ifndef _COLLADA_APPNODE_H_
#include "ts/collada/colladaAppNode.h"
#endif
#ifndef _COLLADA_EXTENSIONS_H_
#include "ts/collada/colladaExtensions.h"
#endif

//-----------------------------------------------------------------------------
// Torque unifies the vert position, normal and UV values, so that a single index
// uniquely identifies all 3 elements. A triangle then contains just 3 indices,
// and from that we can get the 3 positions, 3 normals and 3 UVs.
//
// for i=1:3
//    index = indices[triangle.start + i]
//    points[index], normals[index], uvs[index]
//
// Collada does not use unified vertex streams, (each triangle needs 9 indices),
// so this structure is used to map a single VertTuple index to 3 indices into
// the Collada streams. The Collada (and Torque) primitive index is also stored
// because the Collada document may use different streams for different primitives.
//
// For morph geometry, we can use the same array of VertTuples to access the base
// AND all of the target geometries because they MUST have the same topology.
struct VertTuple
{
   S32 mPrim, mVertex, mNormal, mColor, mUV, mUV2;

   Point3F mDataVertex, mDataNormal;
   ColorI  mDataColor;
   Point2F mDataUV, mDataUV2;

   VertTuple(): mPrim(-1), mVertex(-1), mNormal(-1), mColor(-1), mUV(-1), mUV2(-1) {}
   bool operator==(const VertTuple& p) const 
   {
      return   mDataVertex == p.mDataVertex &&
               mDataColor == p.mDataColor &&  
               mDataNormal == p.mDataNormal &&   
               mDataUV == p.mDataUV &&  
               mDataUV2 == p.mDataUV2;
   }
};

class ColladaAppMesh : public AppMesh
{
   typedef AppMesh Parent;

protected:
   class ColladaAppNode* mAppNode;                    ///< Pointer to the node that owns this mesh
   const domInstance_geometry* mInstanceGeom;
   const domInstance_controller* mInstanceCtrl;
   ColladaExtension_geometry* mGeomExt;               ///< geometry extension

   Vector<VertTuple> mVertTuples;                     ///<
   Map<StringTableEntry,U32> mBoundMaterials;         ///< Local map of symbols to materials

   static bool mFixedSizeEnabled;                     ///< Set to true to fix the detail size to a particular value for all geometry
   static S32 mFixedSize;                             ///< The fixed detail size value for all geometry

   //-----------------------------------------------------------------------

   /// Get the morph controller for this mesh (if any)
   const domMorph* getMorph()
   {
      if (mInstanceCtrl) {
         const domController* ctrl = daeSafeCast<domController>(mInstanceCtrl->getUrl().getElement());
         if (ctrl && ctrl->getSkin())
            ctrl = daeSafeCast<domController>(ctrl->getSkin()->getSource().getElement());
         return ctrl ? ctrl->getMorph() : NULL;
      }
      return NULL;
   }

   S32 addMaterial(const char* symbol);

   bool checkGeometryType(const daeElement* element);
   void getPrimitives(const domGeometry* geometry);

   void getVertexData(  const domGeometry* geometry, F32 time, const MatrixF& objOffset,
                        Vector<Point3F>& points, Vector<Point3F>& norms, Vector<ColorI>& colors, 
                        Vector<Point2F>& uvs, Vector<Point2F>& uv2s, bool appendValues);

   void getMorphVertexData(   const domMorph* morph, F32 time, const MatrixF& objOffset,
                              Vector<Point3F>& points, Vector<Point3F>& norms, Vector<ColorI>& colors,
                              Vector<Point2F>& uvs, Vector<Point2F>& uv2s );

public:

   ColladaAppMesh(const domInstance_geometry* instance, ColladaAppNode* node);
   ColladaAppMesh(const domInstance_controller* instance, ColladaAppNode* node);
   ~ColladaAppMesh()
   {
      delete mGeomExt;
   }

   static void fixDetailSize(bool fixed, S32 size=2)
   {
      mFixedSizeEnabled = fixed;
      mFixedSize = size;
   }

   /// Get the name of this mesh
   ///
   /// @return A string containing the name of this mesh
   const char *getName(bool allowFixed=true);

   //-----------------------------------------------------------------------

   /// Get a floating point property value
   ///
   /// @param propName     Name of the property to get
   /// @param defaultVal   Reference to variable to hold return value
   ///
   /// @return True if a value was set, false if not
   bool getFloat(const char *propName, F32 &defaultVal)
   {
      return mAppNode->getFloat(propName,defaultVal);
   }

   /// Get an integer property value
   ///
   /// @param propName     Name of the property to get
   /// @param defaultVal   Reference to variable to hold return value
   ///
   /// @return True if a value was set, false if not
   bool getInt(const char *propName, S32 &defaultVal)
   {
      return mAppNode->getInt(propName,defaultVal);
   }

   /// Get a boolean property value
   ///
   /// @param propName     Name of the property to get
   /// @param defaultVal   Reference to variable to hold return value
   ///
   /// @return True if a value was set, false if not
   bool getBool(const char *propName, bool &defaultVal)
   {
      return mAppNode->getBool(propName,defaultVal);
   }

   /// Return true if this mesh is a skin
   bool isSkin()
   {
      if (mInstanceCtrl) {
         const domController* ctrl = daeSafeCast<domController>(mInstanceCtrl->getUrl().getElement());
         if (ctrl && ctrl->getSkin() &&
            (ctrl->getSkin()->getVertex_weights()->getV()->getValue().getCount() > 0))
            return true;
      }
      return false;
   }

   /// Get the skin data: bones, vertex weights etc
   void lookupSkinData();

   /// Check if the mesh visibility is animated
   ///
   /// @param appSeq   Start/end time to check
   ///
   /// @return True if the mesh visibility is animated, false if not
   bool animatesVis(const AppSequence* appSeq);

   /// Check if the material used by this mesh is animated
   ///
   /// @param appSeq   Start/end time to check
   ///
   /// @return True if the material is animated, false if not
   bool animatesMatFrame(const AppSequence* appSeq);

   /// Check if the mesh is animated
   ///
   /// @param appSeq   Start/end time to check
   ///
   /// @return True if the mesh is animated, false if not
   bool animatesFrame(const AppSequence* appSeq);

   /// Generate the vertex, normal and triangle data for the mesh.
   ///
   /// @param time           Time at which to generate the mesh data
   /// @param objOffset      Transform to apply to the generated data (bounds transform)
   void lockMesh(F32 time, const MatrixF& objOffset);

   /// Get the transform of this mesh at a certain time
   ///
   /// @param time   Time at which to get the transform
   ///
   /// @return The mesh transform at the specified time
   MatrixF getMeshTransform(F32 time);

   /// Get the visibility of this mesh at a certain time
   ///
   /// @param time   Time at which to get visibility info
   ///
   /// @return Visibility from 0 (invisible) to 1 (opaque)
   F32 getVisValue(F32 time);
};

#endif // _COLLADA_APPMESH_H_
