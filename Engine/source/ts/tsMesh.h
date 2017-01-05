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

#ifndef _TSMESH_H_
#define _TSMESH_H_

#ifndef _STREAM_H_
#include "core/stream/stream.h"
#endif
#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _ABSTRACTPOLYLIST_H_
#include "collision/abstractPolyList.h"
#endif
#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _TSPARSEARRAY_H_
#include "core/tSparseArray.h"
#endif

#include "core/util/safeDelete.h"

#if defined(TORQUE_OS_XENON)
//#  define USE_MEM_VERTEX_BUFFERS
#endif

#if defined(USE_MEM_VERTEX_BUFFERS)
#  include "gfx/D3D9/360/gfx360MemVertexBuffer.h"
#endif

namespace Opcode { class Model; class MeshInterface; }
namespace IceMaths { class IndexedTriangle; class Point; }

class Convex;

class SceneRenderState;
class SceneObject;
struct MeshRenderInst;
class TSRenderState;
class RenderPassManager;
class TSMaterialList;
class TSShapeInstance;
struct RayInfo;
class ConvexFeature;
class ShapeBase;

struct TSDrawPrimitive
{
   enum
   {
      Triangles    = 0 << 30, ///< bits 30 and 31 index element type
      Strip        = 1 << 30, ///< bits 30 and 31 index element type
      Fan          = 2 << 30, ///< bits 30 and 31 index element type
      Indexed      = BIT(29), ///< use glDrawElements if indexed, glDrawArrays o.w.
      NoMaterial   = BIT(28), ///< set if no material (i.e., texture missing)
      MaterialMask = ~(Strip|Fan|Triangles|Indexed|NoMaterial),
      TypeMask     = Strip|Fan|Triangles
   };

   S32 start;
   S32 numElements;
   S32 matIndex;    ///< holds material index & element type (see above enum)
};

#if defined(USE_MEM_VERTEX_BUFFERS)
struct __NullVertexStruct {};
typedef GFX360MemVertexBufferHandle<__NullVertexStruct> TSVertexBufferHandle;
#else
typedef GFXVertexBufferDataHandle TSVertexBufferHandle;
#endif

class TSMesh;
class TSShapeAlloc;

/// @name Vertex format serialization
/// {
struct TSBasicVertexFormat
{
   S16 texCoordOffset;
   S16 boneOffset;
   S16 colorOffset;
   S16 numBones;
   S16 vertexSize;

   TSBasicVertexFormat();
   TSBasicVertexFormat(TSMesh *mesh);
   void getFormat(GFXVertexFormat &fmt);
   void calculateSize();

   void writeAlloc(TSShapeAlloc* alloc);
   void readAlloc(TSShapeAlloc* alloc);

   void addMeshRequirements(TSMesh *mesh);
};
/// }

///
class TSMesh
{
   friend class TSShape;
public:

   /// Helper class for a freeable vector
   template<class T>
   class FreeableVector : public Vector<T>
   {
   public:
      bool free_memory() { return Vector<T>::resize(0); }

      FreeableVector<T>& operator=(const Vector<T>& p) { Vector<T>::operator=(p); return *this; }
      FreeableVector<T>& operator=(const FreeableVector<T>& p) { Vector<T>::operator=(p); return *this; }
   };

   /// @name Aligned Vertex Data 
   /// {

#pragma pack(1)

   struct __TSMeshVertexBase
   {
      Point3F _vert;
      F32 _tangentW;
      Point3F _normal;
      Point3F _tangent;
      Point2F _tvert;

      const Point3F &vert() const { return _vert; }
      void vert(const Point3F &v) { _vert = v; }

      const Point3F &normal() const { return _normal; }
      void normal(const Point3F &n) { _normal = n; }

      Point4F tangent() const { return Point4F(_tangent.x, _tangent.y, _tangent.z, _tangentW); }
      void tangent(const Point4F &t) { _tangent = t.asPoint3F(); _tangentW = t.w; }

      const Point2F &tvert() const { return _tvert; }
      void tvert(const Point2F &tv) { _tvert = tv; }
   };

   struct __TSMeshVertex_3xUVColor
   {
      Point2F _tvert2;
      GFXVertexColor _color;

      const Point2F &tvert2() const { return _tvert2; }
      void tvert2(const Point2F& c) { _tvert2 = c; }

      const GFXVertexColor &color() const { return _color; }
      void color(const GFXVertexColor &c) { _color = c; }
   };

   struct __TSMeshIndex_List {
      U8 x;
      U8 y;
      U8 z;
      U8 w;
   };

   struct __TSMeshVertex_BoneData
   {
      __TSMeshIndex_List _indexes;
      Point4F _weights;

      const __TSMeshIndex_List &index() const { return _indexes; }
      void index(const __TSMeshIndex_List& c) { _indexes = c; }

      const Point4F &weight() const { return _weights; }
      void weight(const Point4F &w) { _weights = w; }
   };

#pragma pack()

   struct TSMeshVertexArray
   {
   protected:
      U8 *base;
      dsize_t vertSz;
      U32 numElements;

      U32 colorOffset;
      U32 boneOffset;

      bool vertexDataReady;
      bool ownsData;

   public:
      TSMeshVertexArray() : base(NULL), numElements(0), colorOffset(0), boneOffset(0), vertexDataReady(false), ownsData(false) {}
      virtual ~TSMeshVertexArray() { set(NULL, 0, 0, 0, 0); }

      virtual void set(void *b, dsize_t s, U32 n, S32 inColorOffset, S32 inBoneOffset, bool nowOwnsData = true)
      {
         if (base && ownsData)
            dFree_aligned(base);
         base = reinterpret_cast<U8 *>(b);
         vertSz = s;
         numElements = n;
         colorOffset = inColorOffset >= 0 ? inColorOffset : 0;
         boneOffset = inBoneOffset >= 0 ? inBoneOffset : 0;
         ownsData = nowOwnsData;
      }

      /// Gets pointer to __TSMeshVertexBase for vertex idx
      __TSMeshVertexBase &getBase(int idx) const
      {
         AssertFatal(idx < numElements, "Out of bounds access!"); return *reinterpret_cast<__TSMeshVertexBase *>(base + (idx * vertSz));
      }

      /// Gets pointer to __TSMeshVertex_3xUVColor for vertex idx
      __TSMeshVertex_3xUVColor &getColor(int idx) const
      {
         AssertFatal(idx < numElements, "Out of bounds access!"); return *reinterpret_cast<__TSMeshVertex_3xUVColor *>(base + (idx * vertSz) + colorOffset);
      }

      /// Gets pointer to __TSMeshVertex_BoneData for vertex idx, additionally offsetted by subBoneList 
      __TSMeshVertex_BoneData &getBone(int idx, int subBoneList) const
      {
         AssertFatal(idx < numElements, "Out of bounds access!"); return *reinterpret_cast<__TSMeshVertex_BoneData *>(base + (idx * vertSz) + boneOffset + (sizeof(__TSMeshVertex_BoneData) * subBoneList));
      }

      /// Returns base address of vertex data
      __TSMeshVertexBase *address() const
      {
         return reinterpret_cast<__TSMeshVertexBase *>(base);
      }

      U32 size() const { return numElements; }
      dsize_t mem_size() const { return numElements * vertSz; }
      dsize_t vertSize() const { return vertSz; }
      bool isReady() const { return vertexDataReady; }
      void setReady(bool r) { vertexDataReady = r; }

      U8* getPtr() { return base; }

      inline U32 getColorOffset() const { return colorOffset; }
      inline U32 getBoneOffset() const { return boneOffset; }
   };

protected:

   U32 meshType;
   Box3F mBounds;
   Point3F mCenter;
   F32 mRadius;
   F32 mVisibility;

   const GFXVertexFormat *mVertexFormat;

   TSMesh *parentMeshObject; ///< Current parent object instance

   U32 mPrimBufferOffset;

   GFXVertexBufferDataHandle mVB;
   GFXPrimitiveBufferHandle mPB;

public:

   S32 parentMesh; ///< index into shapes mesh list
   S32 numFrames;
   S32 numMatFrames;
   S32 vertsPerFrame;

   U32 mVertOffset;
   U32 mVertSize;

protected:

   void _convertToVertexData(TSMeshVertexArray &outArray, const Vector<Point3F> &_verts, const Vector<Point3F> &_norms);

  public:

   enum
   {
      /// types...
      StandardMeshType = 0,
      SkinMeshType     = 1,
      DecalMeshType    = 2,
      SortedMeshType   = 3,
      NullMeshType     = 4,
      TypeMask = StandardMeshType|SkinMeshType|DecalMeshType|SortedMeshType|NullMeshType,

      /// flags (stored with meshType)...
      Billboard = BIT(31), HasDetailTexture = BIT(30),
      BillboardZAxis = BIT(29), UseEncodedNormals = BIT(28),
      HasColor = BIT(27), HasTVert2 = BIT(26),
      FlagMask = Billboard|BillboardZAxis|HasDetailTexture|UseEncodedNormals|HasColor|HasTVert2
   };

   U32 getMeshType() const { return meshType & TypeMask; }
   U32 getHasColor() const { return colors.size() > 0 || meshType & HasColor; }
   U32 getHasTVert2() const { return tverts2.size() > 0 || meshType & HasTVert2; }
   void setFlags(U32 flag) { meshType |= flag; }
   void clearFlags(U32 flag) { meshType &= ~flag; }
   U32 getFlags( U32 flag = 0xFFFFFFFF ) const { return meshType & flag; }

   const Point3F* getNormals( S32 firstVert );

   TSMeshVertexArray mVertexData;
   U32 mNumVerts; ///< Number of verts allocated in main vertex buffer

   virtual void convertToVertexData();

   virtual void copySourceVertexDataFrom(const TSMesh* srcMesh);
   /// @}

   /// @name Vertex data
   /// @{

   FreeableVector<Point3F> verts;
   FreeableVector<Point3F> norms;
   FreeableVector<Point2F> tverts;
   FreeableVector<Point4F> tangents;
   
   // Optional second texture uvs.
   FreeableVector<Point2F> tverts2;

   // Optional vertex colors data.
   FreeableVector<ColorI> colors;
   /// @}

   Vector<TSDrawPrimitive> primitives;
   Vector<U8> encodedNorms;
   Vector<U32> indices;

   /// billboard data
   Point3F billboardAxis;

   /// @name Convex Hull Data
   /// Convex hulls are convex (no angles >= 180º) meshes used for collision
   /// @{

   Vector<Point3F> planeNormals;
   Vector<F32>     planeConstants;
   Vector<U32>     planeMaterials;
   S32 planesPerFrame;
   U32 mergeBufferStart;
   /// @}

   /// @name Render Methods
   /// @{

   /// This is used by sgShadowProjector to render the 
   /// mesh directly, skipping the render manager.
   virtual void render( TSVertexBufferHandle &vb );
   void innerRender( TSVertexBufferHandle &vb, GFXPrimitiveBufferHandle &pb );
   virtual void render( TSMaterialList *, 
                        const TSRenderState &data,
                        bool isSkinDirty,
                        const Vector<MatrixF> &transforms, 
                        TSVertexBufferHandle &vertexBuffer,
                        const char *meshName);

   void innerRender( TSMaterialList *, const TSRenderState &data, TSVertexBufferHandle &vb, GFXPrimitiveBufferHandle &pb, const char *meshName );

   /// @}

   /// @name Material Methods
   /// @{
   void setFade( F32 fade ) { mVisibility = fade; }
   void clearFade() { setFade( 1.0f ); }
   /// @}

   /// @name Collision Methods
   /// @{

   virtual bool buildPolyList( S32 frame, AbstractPolyList * polyList, U32 & surfaceKey, TSMaterialList* materials );
   virtual bool getFeatures( S32 frame, const MatrixF&, const VectorF&, ConvexFeature*, U32 &surfaceKey );
   virtual void support( S32 frame, const Point3F &v, F32 *currMaxDP, Point3F *currSupport );
   virtual bool castRay( S32 frame, const Point3F & start, const Point3F & end, RayInfo * rayInfo, TSMaterialList* materials );
   virtual bool castRayRendered( S32 frame, const Point3F & start, const Point3F & end, RayInfo * rayInfo, TSMaterialList* materials );
   virtual bool buildConvexHull(); ///< returns false if not convex (still builds planes)
   bool addToHull( U32 idx0, U32 idx1, U32 idx2 );
   /// @}

   /// @name Bounding Methods
   /// calculate and get bounding information
   /// @{

   void computeBounds();
   virtual void computeBounds( const MatrixF &transform, Box3F &bounds, S32 frame = 0, Point3F *center = NULL, F32 *radius = NULL );
   void computeBounds( const Point3F *, S32 numVerts, S32 stride, const MatrixF &transform, Box3F &bounds, Point3F *center, F32 *radius );
   const Box3F& getBounds() const { return mBounds; }
   const Point3F& getCenter() const { return mCenter; }
   F32 getRadius() const { return mRadius; }
   virtual S32 getNumPolys() const;

   static U8 encodeNormal( const Point3F &normal );
   static const Point3F& decodeNormal( U8 ncode ) { return smU8ToNormalTable[ncode]; }
   /// @}

   virtual U32 getMaxBonesPerVert() { return 0; }

   /// persist methods...
   virtual void assemble( bool skip );
   static TSMesh* assembleMesh( U32 meshType, bool skip );
   virtual void disassemble();

   void createTangents(const Vector<Point3F> &_verts, const Vector<Point3F> &_norms);
   void findTangent( U32 index1, 
                     U32 index2, 
                     U32 index3, 
                     Point3F *tan0, 
                     Point3F *tan1,
                     const Vector<Point3F> &_verts);

   /// on load...optionally convert primitives to other form
   static bool smUseTriangles;
   static bool smUseOneStrip;
   static S32  smMinStripSize;
   static bool smUseEncodedNormals;

   /// Enables mesh instancing on non-skin meshes that
   /// have less that this count of verts.
   static S32 smMaxInstancingVerts;

   /// Default node transform for standard meshes which have blend indices
   static MatrixF smDummyNodeTransform;

   /// convert primitives on load...
   void convertToTris(const TSDrawPrimitive *primitivesIn, const S32 *indicesIn,
                      S32 numPrimIn, S32 & numPrimOut, S32 & numIndicesOut,
                      TSDrawPrimitive *primitivesOut, S32 *indicesOut) const;
   void convertToSingleStrip(const TSDrawPrimitive *primitivesIn, const S32 *indicesIn,
                             S32 numPrimIn, S32 &numPrimOut, S32 &numIndicesOut,
                             TSDrawPrimitive *primitivesOut, S32 *indicesOut) const;
   void leaveAsMultipleStrips(const TSDrawPrimitive *primitivesIn, const S32 *indicesIn,
                              S32 numPrimIn, S32 &numPrimOut, S32 &numIndicesOut,
                              TSDrawPrimitive *primitivesOut, S32 *indicesOut) const;

   /// Moves vertices from the vertex buffer back into the split vert lists, unless verts already exist
   virtual void makeEditable();

   /// Clears split vertex lists
   virtual void clearEditable();

   void updateMeshFlags();

   /// methods used during assembly to share vertexand other info
   /// between meshes (and for skipping detail levels on load)
   S32* getSharedData32( S32 parentMesh, S32 size, S32 **source, bool skip );
   S8* getSharedData8( S32 parentMesh, S32 size, S8  **source, bool skip );

   /// @name Assembly Variables
   /// variables used during assembly (for skipping mesh detail levels
   /// on load and for sharing verts between meshes)
   /// @{

   static Vector<Point3F*> smVertsList;
   static Vector<Point3F*> smNormsList;
   static Vector<U8*>      smEncodedNormsList;
   
   static Vector<Point2F*> smTVertsList;

   // Optional second texture uvs.
   static Vector<Point2F*> smTVerts2List;

   // Optional vertex colors.
   static Vector<ColorI*> smColorsList;

   static Vector<bool>     smDataCopied;

   static const Point3F smU8ToNormalTable[];
   /// @}


   TSMesh();
   virtual ~TSMesh();
   
   Opcode::Model *mOptTree;
   Opcode::MeshInterface* mOpMeshInterface;
   IceMaths::IndexedTriangle* mOpTris;
   IceMaths::Point* mOpPoints;

   void prepOpcodeCollision();
   bool buildConvexOpcode( const MatrixF &mat, const Box3F &bounds, Convex *c, Convex *list );
   bool buildPolyListOpcode( const S32 od, AbstractPolyList *polyList, const Box3F &nodeBox, TSMaterialList *materials );
   bool castRayOpcode( const Point3F &start, const Point3F &end, RayInfo *rayInfo, TSMaterialList *materials );

   void dumpPrimitives(U32 startVertex, U32 startIndex, GFXPrimitive *piArray, U16* ibIndices);
   virtual U32 getNumVerts();

   static const F32 VISIBILITY_EPSILON; 
};


class TSSkinMesh : public TSMesh
{
public:
   struct BatchData
   {
      enum Constants
      {
         maxBonePerVert = 16,  // Assumes a maximum of 4 blocks of bone indices for HW skinning
      };

      /// @name Batch by vertex
      /// These are used for batches where each element is a vertex, built by
      /// iterating over 0..maxBonePerVert bone transforms
      /// @{
      struct TransformOp
      {
         S32 transformIndex;
         F32 weight;

         TransformOp() : transformIndex( -1 ), weight( -1.0f ) {}
         TransformOp( const S32 tIdx, const F32 w ) :  transformIndex( tIdx ), weight( w ) {};
      };

      struct BatchedVertex
      {
         S32 vertexIndex;
         S32 transformCount;
         TransformOp transform[maxBonePerVert];

         BatchedVertex() : vertexIndex( -1 ), transformCount( -1 ) {}
      };

      Vector<BatchedVertex> vertexBatchOperations;
      /// @}

      // # = num bones
      Vector<S32> nodeIndex;
      Vector<MatrixF> initialTransforms;

      // # = numverts
      Vector<Point3F> initialVerts;
      Vector<Point3F> initialNorms;

      bool initialized;

      BatchData() : initialized(false) { ; }
   };

   /// This method will build the batch operations and prepare the BatchData
   /// for use.
   void createSkinBatchData();

   /// Inserts transform indices and weights into vertex data
   void setupVertexTransforms();

   /// Returns maximum bones used per vertex
   virtual U32 getMaxBonesPerVert();

   virtual void convertToVertexData();
   virtual void copySourceVertexDataFrom(const TSMesh* srcMesh);

   void printVerts();

   void addWeightsFromVertexBuffer();

   void makeEditable();
   void clearEditable();

public:
   typedef TSMesh Parent;
   
   /// @name Vertex tuples
   /// {
   FreeableVector<F32> weight;      ///< blend weight
   FreeableVector<S32> boneIndex;   ///< Maps from mesh node to bone in shape
   FreeableVector<S32> vertexIndex; ///< index of affected vertex
   /// }

   /// Maximum number of bones referenced by this skin mesh
   S32 maxBones;

   /// Structure containing data needed to batch skinning
   BatchData batchData;

   /// set verts and normals...
   void updateSkinBuffer( const Vector<MatrixF> &transforms, U8 *buffer );

   /// update bone transforms for this mesh
   void updateSkinBones( const Vector<MatrixF> &transforms, Vector<MatrixF>& destTransforms );

   // render methods..
   void render( TSVertexBufferHandle &instanceVB );
   void render(   TSMaterialList *, 
                  const TSRenderState &data,
                  bool isSkinDirty,
                  const Vector<MatrixF> &transforms, 
                  TSVertexBufferHandle &vertexBuffer,
                  const char *meshName );

   // collision methods...
   bool buildPolyList( S32 frame, AbstractPolyList *polyList, U32 &surfaceKey, TSMaterialList *materials );
   bool castRay( S32 frame, const Point3F &start, const Point3F &end, RayInfo *rayInfo, TSMaterialList *materials );
   bool buildConvexHull(); // does nothing, skins don't use this

   void computeBounds( const MatrixF &transform, Box3F &bounds, S32 frame, Point3F *center, F32 *radius );

   /// persist methods...
   void assemble( bool skip );
   void disassemble();

   /// Helper method to add a blend tuple for a vertex
   inline void addWeightForVert(U32 vi, U32 bi, F32 w)
   {
      weight.push_back(w);
      boneIndex.push_back(bi);
      vertexIndex.push_back(vi);
   }

   /// variables used during assembly (for skipping mesh detail levels
   /// on load and for sharing verts between meshes)
   static Vector<MatrixF*> smInitTransformList;
   static Vector<S32*>     smVertexIndexList;
   static Vector<S32*>     smBoneIndexList;
   static Vector<F32*>     smWeightList;
   static Vector<S32*>     smNodeIndexList;

   static bool smDebugSkinVerts;

   TSSkinMesh();
};


#endif // _TSMESH_H_
