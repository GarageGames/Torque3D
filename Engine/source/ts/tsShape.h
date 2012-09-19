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

#ifndef _TSSHAPE_H_
#define _TSSHAPE_H_

#ifndef _TSMESH_H_
#include "ts/tsMesh.h"
#endif
#ifndef _TSINTEGERSET_H_
#include "ts/tsIntegerSet.h"
#endif
#ifndef _TSTRANSFORM_H_
#include "ts/tsTransform.h"
#endif
#ifndef _TSSHAPEALLOC_H_
#include "ts/tsShapeAlloc.h"
#endif


#define DTS_EXPORTER_CURRENT_VERSION 124

class TSMaterialList;
class TSLastDetail;
class PhysicsCollision;

//
struct CollisionShapeInfo
{
   S32 colNode;
   PhysicsCollision *colShape;
};

/// TSShape stores generic data for a 3space model.
///
/// TSShape and TSShapeInstance act in conjunction to allow the rendering and
/// manipulation of a three dimensional model.
///
/// @note The material lists are the only thing that is not loaded in TSShape.
/// instead, they are loaded in TSShapeInstance because of a former restriction
/// on the resource manager where only one file could be opened at a time.
/// The resource manager restriction has been resolved, but the material
/// lists are still loaded in TSShapeInstance.
///
/// @see TSShapeInstance for a further discussion of the 3space system.
class TSShape
{
  public:
      enum
      {
         UniformScale   = BIT(0),
         AlignedScale   = BIT(1),
         ArbitraryScale = BIT(2),
         Blend          = BIT(3),
         Cyclic         = BIT(4),
         MakePath       = BIT(5),
         HasTranslucency= BIT(6),
         AnyScale       = UniformScale | AlignedScale | ArbitraryScale
      };

   /// Nodes hold the transforms in the shape's tree.  They are the bones of the skeleton.
   struct Node
   {
      S32 nameIndex;
      S32 parentIndex;

      // computed at runtime
      S32 firstObject;
      S32 firstChild;
      S32 nextSibling;
   };

   /// Objects hold renderable items (in particular meshes).
   ///
   /// Each object has a number of meshes associated with it.
   /// Each mesh corresponds to a different detail level.
   ///
   /// meshIndicesIndex points to numMeshes consecutive indices
   /// into the meshList and meshType vectors.  It indexes the
   /// meshIndexList vector (meshIndexList is merely a clearinghouse
   /// for the object's mesh lists).  Some indices may correspond to
   /// no mesh -- which means no mesh will be drawn for the part for
   /// the given detail level.  See comments on the meshIndexList
   /// for how null meshes are coded.
   ///
   /// @note Things are stored this way so that there are no pointers.
   ///       This makes serialization to disk dramatically simpler.
   struct Object
   {
      S32 nameIndex;
      S32 numMeshes;
      S32 startMeshIndex; ///< Index into meshes array.
      S32 nodeIndex;

      // computed at load
      S32 nextSibling;
      S32 firstDecal; // DEPRECATED
   };

   /// A Sequence holds all the information necessary to perform a particular animation (sequence).
   ///
   /// Sequences index a range of keyframes. Keyframes are assumed to be equally spaced in time.
   ///
   /// Each node and object is either a member of the sequence or not.  If not, they are set to
   /// default values when we switch to the sequence unless they are members of some other active sequence.
   /// Blended sequences "add" a transform to the current transform of a node.  Any object animation of
   /// a blended sequence over-rides any existing object state.  Blended sequences are always
   /// applied after non-blended sequences.
   struct Sequence
   {
      S32 nameIndex;
      S32 numKeyframes;
      F32 duration;
      S32 baseRotation;
      S32 baseTranslation;
      S32 baseScale;
      S32 baseObjectState;
      S32 baseDecalState; // DEPRECATED
      S32 firstGroundFrame;
      S32 numGroundFrames;
      S32 firstTrigger;
      S32 numTriggers;
      F32 toolBegin;

      /// @name Bitsets
      /// These bitsets code whether this sequence cares about certain aspects of animation
      /// e.g., the rotation, translation, or scale of node transforms,
      /// or the visibility, frame or material frame of objects.
      /// @{

      TSIntegerSet rotationMatters;     ///< Set of nodes
      TSIntegerSet translationMatters;  ///< Set of nodes
      TSIntegerSet scaleMatters;        ///< Set of nodes
      TSIntegerSet visMatters;          ///< Set of objects
      TSIntegerSet frameMatters;        ///< Set of objects
      TSIntegerSet matFrameMatters;     ///< Set of objects
      /// @}

      S32 priority;
      U32 flags;
      U32 dirtyFlags; ///< determined at load time

      /// @name Source Data
      /// Store some information about where the sequence data came from (used by
      /// TSShapeConstructor and the Shape Editor)
      /// @{
      struct SeqSourceData
      {
         String from;         // The source sequence (ie. a DSQ file)
         S32 start;           // The first frame in the source sequence
         S32 end;             // The last frame in the source sequence
         S32 total;           // The total number of frames in the source sequence
         String blendSeq;     // The blend reference sequence
         S32 blendFrame;      // The blend reference frame
         SeqSourceData() : from("\t"), start(0), end(0), total(0), blendSeq(""), blendFrame(0) { }
      } sourceData;

      /// @name Flag Tests
      /// Each of these tests a different flag against the object's flag list
      /// to determine the attributes of the given object.
      /// @{

      bool testFlags(U32 comp) const      { return (flags&comp)!=0; }
      bool animatesScale() const          { return testFlags(AnyScale); }
      bool animatesUniformScale() const   { return testFlags(UniformScale); }
      bool animatesAlignedScale() const   { return testFlags(AlignedScale); }
      bool animatesArbitraryScale() const { return testFlags(ArbitraryScale); }
      bool isBlend() const                { return testFlags(Blend); }
      bool isCyclic() const               { return testFlags(Cyclic); }
      bool makePath() const               { return testFlags(MakePath); }
      /// @}

      /// @name IO
      /// @{

      void read(Stream *, bool readNameIndex = true);
      void write(Stream *, bool writeNameIndex = true) const;
      /// @}
   };

   /// Describes state of an individual object.  Includes everything in an object that can be
   /// controlled by animation.
   struct ObjectState
   {
      F32 vis;
      S32 frameIndex;
      S32 matFrameIndex;
   };

   /// When time on a sequence advances past a certain point, a trigger takes effect and changes
   /// one of the state variables to on or off. (State variables found in TSShapeInstance::mTriggerStates)
   struct Trigger
   {
      enum TriggerStates
      {
         StateOn = BIT(31),
         InvertOnReverse = BIT(30),
         StateMask = BIT(30)-1
      };

      U32 state; ///< One of TriggerStates
      F32 pos;
   };

   /// Details are used for render detail selection.
   ///
   /// As the projected size of the shape changes,
   /// a different node structure can be used (subShape) and a different objectDetail can be selected
   /// for each object drawn.   Either of these two parameters can also stay constant, but presumably
   /// not both.  If size is negative then the detail level will never be selected by the standard
   /// detail selection process.  It will have to be selected by name.  Such details are "utility
   /// details" because they exist to hold data (node positions or collision information) but not
   /// normally to be drawn.  By default there will always be a "Ground" utility detail.
   ///
   /// Note that this struct should always be 32bit aligned
   /// as its required by assembleShape/disassembleShape.
   struct Detail
   {
      S32 nameIndex;
      S32 subShapeNum;
      S32 objectDetailNum;
      F32 size;
      F32 averageError;
      F32 maxError;
      S32 polyCount;

      /// These values are new autobillboard settings stored
      /// as part of the Detail struct in version 26 and above.
      /// @{

      S32 bbDimension;     ///< The size of the autobillboard image.
      S32 bbDetailLevel;   ///< The detail to render as the autobillboard.
      U32 bbEquatorSteps;  ///< The number of autobillboard images to capture around the equator.
      U32 bbPolarSteps;    ///< The number of autobillboard images to capture along the pole.
      F32 bbPolarAngle;    ///< The angle in radians at which the top/bottom autobillboard images should be displayed.
      U32 bbIncludePoles;  ///< If non-zero then top and bottom images are generated for the autobillboard.

      /// @}
   };

   /// @name Collision Accelerators
   ///
   /// For speeding up buildpolylist and support calls.
   /// @{
   struct ConvexHullAccelerator {
      S32      numVerts;
      Point3F* vertexList;
      Point3F* normalList;
      U8**     emitStrings;
   };
   ConvexHullAccelerator* getAccelerator(S32 dl);
   /// @}


   /// @name Shape Vector Data
   /// @{

   Vector<Node> nodes;
   Vector<Object> objects;
   Vector<ObjectState> objectStates;
   Vector<S32> subShapeFirstNode;
   Vector<S32> subShapeFirstObject;
   Vector<S32> detailFirstSkin;
   Vector<S32> subShapeNumNodes;
   Vector<S32> subShapeNumObjects;
   Vector<Detail> details;
   Vector<Quat16> defaultRotations;
   Vector<Point3F> defaultTranslations;

   /// @}

   /// These are set up at load time, but memory is allocated along with loaded data
   /// @{

   Vector<S32> subShapeFirstTranslucentObject;
   Vector<TSMesh*> meshes;

   /// @}

   /// @name Alpha Vectors
   /// these vectors describe how to transition between detail
   /// levels using alpha. "alpha-in" next detail as intraDL goes
   /// from alphaIn+alphaOut to alphaOut. "alpha-out" current
   /// detail level as intraDL goes from alphaOut to 0.
   /// @note
   ///   - intraDL is at 1 when if shape were any closer to us we'd be at dl-1
   ///   - intraDL is at 0 when if shape were any farther away we'd be at dl+1
   /// @{

   Vector<F32> alphaIn;
   Vector<F32> alphaOut
      ;
   /// @}

   /// @name Resizeable vectors
   /// @{

   Vector<Sequence>                 sequences;
   Vector<Quat16>                   nodeRotations;
   Vector<Point3F>                  nodeTranslations;
   Vector<F32>                      nodeUniformScales;
   Vector<Point3F>                  nodeAlignedScales;
   Vector<Quat16>                   nodeArbitraryScaleRots;
   Vector<Point3F>                  nodeArbitraryScaleFactors;
   Vector<Quat16>                   groundRotations;
   Vector<Point3F>                  groundTranslations;
   Vector<Trigger>                  triggers;
   Vector<TSLastDetail*>            billboardDetails;
   Vector<ConvexHullAccelerator*>   detailCollisionAccelerators;
   Vector<String>                   names;

   /// @}

   TSMaterialList * materialList;

   /// @name Bounding
   /// @{

   F32 radius;
   F32 tubeRadius;
   Point3F center;
   Box3F bounds;

   /// @}

   // various...
   U32 mExporterVersion;
   F32 mSmallestVisibleSize;  ///< Computed at load time from details vector.
   S32 mSmallestVisibleDL;    ///< @see mSmallestVisibleSize
   S32 mReadVersion;          ///< File version that this shape was read from.
   U32 mFlags;                ///< hasTranslucancy
   U32 data;                  ///< User-defined data storage.

   /// If enabled detail selection will use the
   /// legacy screen error method for lod.
   /// @see setDetailFromScreenError
   bool mUseDetailFromScreenError;

   // TODO: This would be nice as Tuple<>
   struct LodPair
   {
      S8 level; // -1 to 128
      U8 intra; // encoded 0 to 1

      inline void set( S32 dl, F32 intraDL )
      {
         level = (S8)dl;
         intra = (S8)( intraDL * 255.0f );
      }

      inline void get( S32 &dl, F32 &intraDL )
      {
         dl = level;
         intraDL = (F32)intra / 255.0f;
      }
   };

   /// The lod lookup table where we mark down the detail
   /// level and intra-detail level for each pixel size.
   Vector<LodPair> mDetailLevelLookup;

   /// The GFX vertex format for all detail meshes in the shape.
   /// @see initVertexFeatures()
   GFXVertexFormat mVertexFormat;

   /// The GFX vertex size in bytes for all detail meshes in the shape.
   /// @see initVertexFeatures()
   U32 mVertSize;

   /// Is true if this shape contains skin meshes.
   bool mHasSkinMesh;

   bool mSequencesConstructed;

   S8* mShapeData;
   U32 mShapeDataSize;

   // shape class has few methods --
   // just constructor/destructor, io, and lookup methods

   // constructor/destructor
   TSShape();
   ~TSShape();
   void init();
   void initMaterialList();    ///< you can swap in a new material list, but call this if you do
   bool preloadMaterialList(const Torque::Path &path); ///< called to preload and validate the materials in the mat list

   void setupBillboardDetails( const String &cachePath );

   /// Called from init() to calcuate the GFX vertex features for
   /// all detail meshes in the shape.
   void initVertexFeatures();

   bool getSequencesConstructed() const { return mSequencesConstructed; }
   void setSequencesConstructed(const bool c) { mSequencesConstructed = c; }

   /// @name Lookup Animation Info
   /// indexed by keyframe number and offset (which object/node/decal
   /// of the animated objects/nodes/decals you want information for).
   /// @{

   QuatF & getRotation(const Sequence & seq, S32 keyframeNum, S32 rotNum, QuatF *) const;
   const Point3F & getTranslation(const Sequence & seq, S32 keyframeNum, S32 tranNum) const;
   F32 getUniformScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum) const;
   const Point3F & getAlignedScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum) const;
   TSScale & getArbitraryScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum, TSScale *) const;
   const ObjectState & getObjectState(const Sequence & seq, S32 keyframeNum, S32 objectNum) const;
   /// @}

   /// build LOS collision detail
   void computeAccelerator(S32 dl);
   bool buildConvexHull(S32 dl) const;
   void computeBounds(S32 dl, Box3F & bounds) const; // uses default transforms to compute bounding box around a detail level
                                                     // see like named method on shapeInstance if you want to use animated transforms

   /// Used to find collision detail meshes in the DTS.
   ///
   /// @param useVisibleMesh If true return the highest visible detail level.
   /// @param outDetails The output detail index vector.
   /// @param outLOSDetails The optional output LOS detail vector.
   ///
   void findColDetails( bool useVisibleMesh, Vector<S32> *outDetails, Vector<S32> *outLOSDetails ) const;

   /// Builds a physics collision shape at the requested scale.
   ///
   /// If using the visible mesh one or more triangle meshes are created
   /// from the first visible detail level.
   ///
   /// If using collision meshes we look for mesh names prefixed with the
   /// following hints:
   //
   ///  "colbox"
   ///  "colsphere"
   ///  "colcapsule"
   ///  "colmesh"
   ///
   /// In the case of the primitives the mesh bounding box is used to generate
   /// a box, sphere, or capsule collision shape.  The "colmesh" will create a
   /// concave triangle mesh for collision.
   ///
   /// Any other named collision shape is interpreted as a regular convex hull.
   ///
   /// @return The collision object or NULL if no collision data could be generated.
   ///
   PhysicsCollision* buildColShape( bool useVisibleMesh, const Point3F &scale );
   
   /// Like buildColShape except we build one PhysicsCollision object per 
   /// collision node.
   ///
   /// Results are returned by filling in the CollisionShapeInfo Vector, which also
   /// specifies the collision node index for each PhysicsCollision built.
   ///
   void buildColShapes( bool useVisibleMesh, const Point3F &scale, Vector< CollisionShapeInfo > *list );

   /// For internal use.
   PhysicsCollision* _buildColShapes( bool useVisibleMesh, const Point3F &scale, Vector< CollisionShapeInfo > *list, bool perMesh );

   /// @name Lookup Methods
   /// @{

   /// Returns index into the name vector that equals the passed name.
   S32 findName( const String &name ) const;
   
   /// Returns name string at the passed name vector index.
   const String& getName( S32 nameIndex ) const;
   
   /// Returns name string for mesh at the passed index.
   const String& getMeshName( S32 meshIndex ) const;
   
   /// Returns name string for node at the passed index.
   const String& getNodeName( S32 nodeIndex ) const;
   
   /// Returns name string for sequence at the passed index.
   const String& getSequenceName( S32 seqIndex ) const;

	S32 getTargetCount() const;
	const String& getTargetName( S32 mapToNameIndex ) const;

   S32 findNode(S32 nameIndex) const;
   S32 findNode(const String &name) const { return findNode(findName(name)); }

   S32 findObject(S32 nameIndex) const;
   S32 findObject(const String &name) const { return findObject(findName(name)); }

   S32 findDetail(S32 nameIndex) const;
   S32 findDetail(const String &name) const { return findDetail(findName(name)); }
   S32 findDetailBySize(S32 size) const;

   S32 findSequence(S32 nameIndex) const;
   S32 findSequence(const String &name) const { return findSequence(findName(name)); }

   S32 getSubShapeForNode(S32 nodeIndex);
   S32 getSubShapeForObject(S32 objIndex);
   void getSubShapeDetails(S32 subShapeIndex, Vector<S32>& validDetails);

   void getNodeWorldTransform(S32 nodeIndex, MatrixF* mat) const;
   void getNodeKeyframe(S32 nodeIndex, const TSShape::Sequence& seq, S32 keyframe, MatrixF* mat) const;
   void getNodeObjects(S32 nodeIndex, Vector<S32>& nodeObjects);
   void getNodeChildren(S32 nodeIndex, Vector<S32>& nodeChildren);

   void getObjectDetails(S32 objIndex, Vector<S32>& objDetails);

   bool findMeshIndex(const String &meshName, S32& objIndex, S32& meshIndex);
   TSMesh* findMesh(const String &meshName);

   bool hasTranslucency() const { return (mFlags & HasTranslucency)!=0; }

   const GFXVertexFormat* getVertexFormat() const { return &mVertexFormat; }

   U32 getVertexSize() const { return mVertSize; }

   /// @}

   /// @name Alpha Transitions
   /// These control default values for alpha transitions between detail levels
   /// @{

   static F32 smAlphaOutLastDetail;
   static F32 smAlphaInBillboard;
   static F32 smAlphaOutBillboard;
   static F32 smAlphaInDefault;
   static F32 smAlphaOutDefault;
   /// @}

   /// don't load this many of the highest detail levels (although we always
   /// load one renderable detail if there is one)
   static S32 smNumSkipLoadDetails;

   /// by default we initialize shape when we read...
   static bool smInitOnRead;

   /// @name Version Info
   /// @{

   /// Most recent version...the one we write
   static S32 smVersion;
   /// Version currently being read, only valid during read
   static S32 smReadVersion;
   static const U32 smMostRecentExporterVersion;
   ///@}

   /// @name Persist Methods
   /// Methods for saving/loading shapes to/from streams
   /// @{

   bool canWriteOldFormat() const;
   void write(Stream *, bool saveOldFormat=false);
   bool read(Stream *);
   void readOldShape(Stream * s, S32 * &, S16 * &, S8 * &, S32 &, S32 &, S32 &);
   void writeName(Stream *, S32 nameIndex);
   S32  readName(Stream *, bool addName);

   /// Initializes our TSShape to be ready to receive put mesh data
   void createEmptyShape();

   void exportSequences(Stream *);
   void exportSequence(Stream * s, const TSShape::Sequence& seq, bool saveOldFormat);
   bool importSequences(Stream *, const String& sequencePath);

   /// @}

   /// @name Persist Helper Functions
   /// @{

   static TSShapeAlloc smTSAlloc;

   void fixEndian(S32 *, S16 *, S8 *, S32, S32, S32);
   /// @}

   /// @name Memory Buffer Transfer Methods
   /// uses TSShape::Alloc structure
   /// @{

   void assembleShape();
   void disassembleShape();
   ///@}

   /// mem buffer transfer helper (indicate when we don't want to include a particular mesh/decal)
   bool checkSkip(S32 meshNum, S32 & curObject, S32 skipDL);

   void fixupOldSkins(S32 numMeshes, S32 numSkins, S32 numDetails, S32 * detailFirstSkin, S32 * detailNumSkins);

   /// @name Shape Editing
   /// @{
   S32 addName(const String& name);
   bool removeName(const String& name);
   void updateSmallestVisibleDL();
   S32 addDetail(const String& dname, S32 size, S32 subShapeNum);

   S32 addImposter(  const String& cachePath,
                     S32 size,
                     S32 numEquatorSteps,
                     S32 numPolarSteps,
                     S32 dl,
                     S32 dim,
                     bool includePoles,
                     F32 polarAngle );
   bool removeImposter();

   bool renameNode(const String& oldName, const String& newName);
   bool renameObject(const String& oldName, const String& newName);
   bool renameDetail(const String& oldName, const String& newName);
   bool renameSequence(const String& oldName, const String& newName);

   bool setNodeTransform(const String& name, const Point3F& pos, const QuatF& rot);
   bool addNode(const String& name, const String& parentName, const Point3F& pos, const QuatF& rot);
   bool removeNode(const String& name);

   S32 addObject(const String& objName, S32 subShapeIndex);
   void addMeshToObject(S32 objIndex, S32 meshIndex, TSMesh* mesh);
   void removeMeshFromObject(S32 objIndex, S32 meshIndex);
   bool setObjectNode(const String& objName, const String& nodeName);
   bool removeObject(const String& objName);

   TSMesh* copyMesh( const TSMesh* srcMesh ) const;
   bool addMesh(TSShape* srcShape, const String& srcMeshName, const String& meshName);
   bool addMesh(TSMesh* mesh, const String& meshName);
   bool setMeshSize(const String& meshName, S32 size);
   bool removeMesh(const String& meshName);

   S32 setDetailSize(S32 oldSize, S32 newSize);
   bool removeDetail(S32 size);

   bool addSequence(const Torque::Path& path, const String& fromSeq, const String& name, S32 startFrame, S32 endFrame, bool padRotKeys, bool padTransKeys);
   bool removeSequence(const String& name);

   bool addTrigger(const String& seqName, S32 keyframe, S32 state);
   bool removeTrigger(const String& seqName, S32 keyframe, S32 state);

   bool setSequenceBlend(const String& seqName, bool blend, const String& blendRefSeqName, S32 blendRefFrame);
   bool setSequenceGroundSpeed(const String& seqName, const Point3F& trans, const Point3F& rot);
   /// @}
};


#define TSNode TSShape::Node
#define TSObject TSShape::Object
#define TSSequence TSShape::Sequence
#define TSDetail TSShape::Detail

inline QuatF & TSShape::getRotation(const Sequence & seq, S32 keyframeNum, S32 rotNum, QuatF * quat) const
{
   return nodeRotations[seq.baseRotation + rotNum*seq.numKeyframes + keyframeNum].getQuatF(quat);
}

inline const Point3F & TSShape::getTranslation(const Sequence & seq, S32 keyframeNum, S32 tranNum) const
{
   return nodeTranslations[seq.baseTranslation + tranNum*seq.numKeyframes + keyframeNum];
}

inline F32 TSShape::getUniformScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum) const
{
   return nodeUniformScales[seq.baseScale + scaleNum*seq.numKeyframes + keyframeNum];
}

inline const Point3F & TSShape::getAlignedScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum) const
{
   return nodeAlignedScales[seq.baseScale + scaleNum*seq.numKeyframes + keyframeNum];
}

inline TSScale & TSShape::getArbitraryScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum, TSScale * scale) const
{
   nodeArbitraryScaleRots[seq.baseScale + scaleNum*seq.numKeyframes + keyframeNum].getQuatF(&scale->mRotate);
   scale->mScale = nodeArbitraryScaleFactors[seq.baseScale + scaleNum*seq.numKeyframes + keyframeNum];
   return *scale;
}

inline const TSShape::ObjectState & TSShape::getObjectState(const Sequence & seq, S32 keyframeNum, S32 objectNum) const
{
   return objectStates[seq.baseObjectState + objectNum*seq.numKeyframes + keyframeNum];
}

#endif
