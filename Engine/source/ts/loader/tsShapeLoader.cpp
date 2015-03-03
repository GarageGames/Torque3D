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
#include "console/engineAPI.h"
#include "ts/loader/tsShapeLoader.h"

#include "core/volume.h"
#include "materials/materialList.h"
#include "materials/matInstance.h"
#include "materials/materialManager.h"
#include "ts/tsShapeInstance.h"
#include "ts/tsMaterialList.h"

MODULE_BEGIN( ShapeLoader )
   MODULE_INIT_AFTER( GFX )
   MODULE_INIT
   {
      TSShapeLoader::addFormat("Torque DTS", "dts");
      TSShapeLoader::addFormat("Torque DSQ", "dsq");
   }
MODULE_END;

const F32 TSShapeLoader::smDefaultTime = -1.0f;
const F64 TSShapeLoader::smMinFrameRate = 15.0f;
const F64 TSShapeLoader::smMaxFrameRate = 60.0f;
const F64 TSShapeLoader::smAppGroundFrameRate = 10.0f;
Torque::Path TSShapeLoader::smShapePath;

Vector<TSShapeLoader::ShapeFormat> TSShapeLoader::smFormats;

//------------------------------------------------------------------------------
// Utility functions

void TSShapeLoader::zapScale(MatrixF& mat)
{
   Point3F invScale = mat.getScale();
   invScale.x = invScale.x ? (1.0f / invScale.x) : 0;
   invScale.y = invScale.y ? (1.0f / invScale.y) : 0;
   invScale.z = invScale.z ? (1.0f / invScale.z) : 0;
   mat.scale(invScale);
}

//------------------------------------------------------------------------------
// Shape utility functions

MatrixF TSShapeLoader::getLocalNodeMatrix(AppNode* node, F32 t)
{
   MatrixF m1 = node->getNodeTransform(t);

   // multiply by inverse scale at t=0
   MatrixF m10 = node->getNodeTransform(smDefaultTime);
   m1.scale(Point3F(1.0f/m10.getScale().x, 1.0f/m10.getScale().y, 1.0f/m10.getScale().z));

   if (node->mParentIndex >= 0)
   {
      AppNode *parent = mAppNodes[node->mParentIndex];

      MatrixF m2 = parent->getNodeTransform(t);

      // multiply by inverse scale at t=0
      MatrixF m20 = parent->getNodeTransform(smDefaultTime);
      m2.scale(Point3F(1.0f/m20.getScale().x, 1.0f/m20.getScale().y, 1.0f/m20.getScale().z));

      // get local transform by pre-multiplying by inverted parent transform
      m1 = m2.inverse() * m1;
   }
   else if (mBoundsNode && node != mBoundsNode)
   {
      // make transform relative to bounds node transform at time=t
      MatrixF mb = mBoundsNode->getNodeTransform(t);
      zapScale(mb);
      m1 = mb.inverse() * m1;
   }

   return m1;
}

void TSShapeLoader::generateNodeTransform(AppNode* node, F32 t, bool blend, F32 referenceTime,
                                          QuatF& rot, Point3F& trans, QuatF& srot, Point3F& scale)
{
   MatrixF m1 = getLocalNodeMatrix(node, t);
   if (blend)
   {
      MatrixF m0 = getLocalNodeMatrix(node, referenceTime);
      m1 = m0.inverse() * m1;
   }

   rot.set(m1);
   trans = m1.getPosition();
   srot.identity();        //@todo: srot not supported yet
   scale = m1.getScale();
}

//-----------------------------------------------------------------------------

void TSShapeLoader::updateProgress(S32 major, const char* msg, S32 numMinor, S32 minor)
{
   // Calculate progress value
   F32 progress = (F32)major / NumLoadPhases;
   const char *progressMsg = msg;

   if (numMinor)
   {
      progress += (minor * (1.0f / NumLoadPhases) / numMinor);
      progressMsg = avar("%s (%d of %d)", msg, minor + 1, numMinor);
   }

   Con::executef("updateTSShapeLoadProgress", Con::getFloatArg(progress), progressMsg);
}

//-----------------------------------------------------------------------------
// Shape creation entry point

TSShape* TSShapeLoader::generateShape(const Torque::Path& path)
{
   smShapePath = path;
   mShape = new TSShape();

   mShape->mExporterVersion = 124;
   mShape->mSmallestVisibleSize = 999999;
   mShape->mSmallestVisibleDL = 0;
   mShape->mReadVersion = 24;
   mShape->mFlags = 0;
   mShape->mSequencesConstructed = 0;

   // Get all nodes, objects and sequences in the shape
   updateProgress(Load_EnumerateScene, "Enumerating scene...");
   enumerateScene();
   if (!mSubShapes.size())
   {
      delete mShape;
      Con::errorf("Failed to load shape \"%s\", no subshapes found", path.getFullPath().c_str());
      return NULL;
   }

   // Create the TSShape::Node hierarchy
   generateSubshapes();

   // Create objects (meshes and details)
   generateObjects();

   // Generate initial object states and node transforms
   generateDefaultStates();

   // Generate skins
   generateSkins();

   // Generate material list
   generateMaterialList();

   // Generate animation sequences
   generateSequences();

   // Sort detail levels and meshes
   updateProgress(Load_InitShape, "Initialising shape...");
   sortDetails();

   // Install the TS memory helper into a TSShape object.
   install();

   return mShape;
}

bool TSShapeLoader::processNode(AppNode* node)
{
   // Detect bounds node
   if ( node->isBounds() )
   {
      if ( mBoundsNode )
      {
         Con::warnf( "More than one bounds node found" );
         return false;
      }
      mBoundsNode = node;

      // Process bounds geometry
      MatrixF boundsMat(mBoundsNode->getNodeTransform(smDefaultTime));
      boundsMat.inverse();
      zapScale(boundsMat);
      for (S32 iMesh = 0; iMesh < mBoundsNode->getNumMesh(); iMesh++)
      {
         AppMesh* mesh = mBoundsNode->getMesh(iMesh);
         MatrixF transform = mesh->getMeshTransform(smDefaultTime);
         transform.mulL(boundsMat);
         mesh->lockMesh(smDefaultTime, transform);
      }
      return true;
   }

   // Detect sequence markers
   if ( node->isSequence() )
   {
      //appSequences.push_back(new AppSequence(node));
      return false;
   }

   // Add this node to the subshape (create one if needed)
   if ( mSubShapes.size() == 0 )
      mSubShapes.push_back( new TSShapeLoader::Subshape );

   mSubShapes.last()->branches.push_back( node );

   return true;
}

//-----------------------------------------------------------------------------
// Nodes, meshes and skins

typedef bool (*NameCmpFunc)(const String&, const Vector<String>&, void*, void*);

bool cmpShapeName(const String& key, const Vector<String>& names, void* arg1, void* arg2)
{
   for (S32 i = 0; i < names.size(); i++)
   {
      if (names[i].compare(key, 0, String::NoCase) == 0)
         return false;
   }
   return true;
}

String getUniqueName(const char* name, NameCmpFunc isNameUnique, const Vector<String>& names, void* arg1=0, void* arg2=0)
{
   const S32 MAX_ITERATIONS = 0x10000;   // maximum of 4 characters (A-P) will be appended

   String suffix;
   for (S32 i = 0; i < MAX_ITERATIONS; i++)
   {
      // Generate a suffix using the first 16 characters of the alphabet
      suffix.clear();
      for (S32 value = i; value != 0; value >>= 4)
         suffix = suffix + (char)('A' + (value & 0xF));

      String uname = name + suffix;
      if (isNameUnique(uname, names, arg1, arg2))
         return uname;
   }
   return name;
}

void TSShapeLoader::recurseSubshape(AppNode* appNode, S32 parentIndex, bool recurseChildren)
{
   // Ignore local bounds nodes
   if (appNode->isBounds())
      return;

   S32 subShapeNum = mShape->mSubShapeFirstNode.size()-1;
   Subshape* subshape = mSubShapes[subShapeNum];

   // Check if we should collapse this node
   S32 myIndex;
   if (ignoreNode(appNode->getName()))
   {
      myIndex = parentIndex;
   }
   else
   {
      // Check that adding this node will not exceed the maximum node count
      if (mShape->mNodes.size() >= MAX_TS_SET_SIZE)
         return;

      myIndex = mShape->mNodes.size();
      String nodeName = getUniqueName(appNode->getName(), cmpShapeName, mShape->mNames);

      // Create the 3space node
      mShape->mNodes.increment();
      mShape->mNodes.last().nameIndex = mShape->addName(nodeName);
      mShape->mNodes.last().parentIndex = parentIndex;
      mShape->mNodes.last().firstObject = -1;
      mShape->mNodes.last().firstChild = -1;
      mShape->mNodes.last().nextSibling = -1;

      // Add the AppNode to a matching list (so AppNodes can be accessed using 3space
      // node indices)
      mAppNodes.push_back(appNode);
      mAppNodes.last()->mParentIndex = parentIndex;

      // Check for NULL detail or AutoBillboard nodes (no children or geometry)
      if ((appNode->getNumChildNodes() == 0) &&
          (appNode->getNumMesh() == 0))
      {
         S32 size = 0x7FFFFFFF;
         String dname(String::GetTrailingNumber(appNode->getName(), size));

         if (dStrEqual(dname, "nulldetail") && (size != 0x7FFFFFFF))
         {
            mShape->addDetail("detail", size, subShapeNum);
         }
         else if (appNode->isBillboard() && (size != 0x7FFFFFFF))
         {
            // AutoBillboard detail
            S32 numEquatorSteps = 4;
            S32 numPolarSteps = 0;
            F32 polarAngle = 0.0f;
            S32 dl = 0;
            S32 dim = 64;
            bool includePoles = true;

            appNode->getInt("BB::EQUATOR_STEPS", numEquatorSteps);
            appNode->getInt("BB::POLAR_STEPS", numPolarSteps);
            appNode->getFloat("BB::POLAR_ANGLE", polarAngle);
            appNode->getInt("BB::DL", dl);
            appNode->getInt("BB::DIM", dim);
            appNode->getBool("BB::INCLUDE_POLES", includePoles);

            S32 detIndex = mShape->addDetail( "bbDetail", size, -1 );
            mShape->mDetails[detIndex].bbEquatorSteps = numEquatorSteps;
            mShape->mDetails[detIndex].bbPolarSteps = numPolarSteps;
            mShape->mDetails[detIndex].bbDetailLevel = dl;
            mShape->mDetails[detIndex].bbDimension = dim;
            mShape->mDetails[detIndex].bbIncludePoles = includePoles;
            mShape->mDetails[detIndex].bbPolarAngle = polarAngle;
         }
      }
   }

   // Collect geometry
   for (U32 iMesh = 0; iMesh < appNode->getNumMesh(); iMesh++)
   {
      AppMesh* mesh = appNode->getMesh(iMesh);
      if (!ignoreMesh(mesh->getName()))
      {
         subshape->objMeshes.push_back(mesh);
         subshape->objNodes.push_back(mesh->isSkin() ? -1 : myIndex);
      }
   }

   // Create children
   if (recurseChildren)
   {
      for (S32 iChild = 0; iChild < appNode->getNumChildNodes(); iChild++)
         recurseSubshape(appNode->getChildNode(iChild), myIndex, true);
   }
}

void TSShapeLoader::generateSubshapes()
{
   for (U32 iSub = 0; iSub < mSubShapes.size(); iSub++)
   {
      updateProgress(Load_GenerateSubshapes, "Generating subshapes...", mSubShapes.size(), iSub);

      Subshape* subshape = mSubShapes[iSub];

      // Recurse through the node hierarchy, adding 3space nodes and
      // collecting geometry
      S32 firstNode = mShape->mNodes.size();
      mShape->mSubShapeFirstNode.push_back(firstNode);      

      for (U32 iBranch = 0; iBranch < subshape->branches.size(); iBranch++)
         recurseSubshape(subshape->branches[iBranch], -1, true);

      mShape->mSubShapeNumNodes.push_back(mShape->mNodes.size() - firstNode);

      if (mShape->mNodes.size() >= MAX_TS_SET_SIZE)
      {
         Con::warnf("Shape exceeds the maximum node count (%d). Ignoring additional nodes.",
            MAX_TS_SET_SIZE);
      }
   }
}

// Custom name comparison function to compare mesh name and detail size
bool cmpMeshNameAndSize(const String& key, const Vector<String>& names, void* arg1, void* arg2)
{
   const Vector<AppMesh*>& meshes = *(Vector<AppMesh*>*)arg1;
   S32                     meshSize = (intptr_t)arg2;

   for (S32 i = 0; i < names.size(); i++)
   {
      if (names[i].compare(key, 0, String::NoCase) == 0)
      {
         if (meshes[i]->mDetailSize == meshSize)
            return false;
      }
   }
   return true;
}

void TSShapeLoader::generateObjects()
{
   for (S32 iSub = 0; iSub < mSubShapes.size(); iSub++)
   {
      Subshape* subshape = mSubShapes[iSub];
      mShape->mSubShapeFirstObject.push_back(mShape->mObjects.size());

      // Get the names and sizes of the meshes for this subshape
      Vector<String> meshNames;
      for (S32 iMesh = 0; iMesh < subshape->objMeshes.size(); iMesh++)
      {
         AppMesh* mesh = subshape->objMeshes[iMesh];
         mesh->mDetailSize = 2;
         String name = String::GetTrailingNumber( mesh->getName(), mesh->mDetailSize );
         name = getUniqueName( name, cmpMeshNameAndSize, meshNames, &(subshape->objMeshes), (void*)mesh->mDetailSize );
         meshNames.push_back( name );

         // Fix up any collision details that don't have a negative detail level.
         if (  dStrStartsWith(meshNames[iMesh], "Collision") ||
               dStrStartsWith(meshNames[iMesh], "LOSCol") )
         {
            if (mesh->mDetailSize > 0)
               mesh->mDetailSize = -mesh->mDetailSize;
         }
      }

      // An 'object' is a collection of meshes with the same base name and
      // different detail sizes. The object is attached to the node of the
      // highest detail mesh.

      // Sort the 3 arrays (objMeshes, objNodes, meshNames) by name and size
      for (S32 i = 0; i < subshape->objMeshes.size()-1; i++)
      {
         for (S32 j = i+1; j < subshape->objMeshes.size(); j++)
         {
            if ((meshNames[i].compare(meshNames[j]) < 0) ||
               ((meshNames[i].compare(meshNames[j]) == 0) &&
               (subshape->objMeshes[i]->mDetailSize < subshape->objMeshes[j]->mDetailSize)))
            {
               {
                  AppMesh* tmp = subshape->objMeshes[i];
                  subshape->objMeshes[i] = subshape->objMeshes[j];
                  subshape->objMeshes[j] = tmp;
               }
               {
                  S32 tmp = subshape->objNodes[i];
                  subshape->objNodes[i] = subshape->objNodes[j];
                  subshape->objNodes[j] = tmp;
               }
               {
                  String tmp = meshNames[i];
                  meshNames[i] = meshNames[j];
                  meshNames[j] = tmp;
               }
            }
         }
      }

      // Now create objects
      const String* lastName = 0;
      for (S32 iMesh = 0; iMesh < subshape->objMeshes.size(); iMesh++)
      {
         AppMesh* mesh = subshape->objMeshes[iMesh];

         if (!lastName || (meshNames[iMesh] != *lastName))
         {
            mShape->mObjects.increment();
            mShape->mObjects.last().nameIndex = mShape->addName(meshNames[iMesh]);
            mShape->mObjects.last().nodeIndex = subshape->objNodes[iMesh];
            mShape->mObjects.last().startMeshIndex = mAppMeshes.size();
            mShape->mObjects.last().numMeshes = 0;
            lastName = &meshNames[iMesh];
         }

         // Add this mesh to the object
         mAppMeshes.push_back(mesh);
         mShape->mObjects.last().numMeshes++;

         // Set mesh flags
         mesh->mFlags = 0;
         if (mesh->isBillboard())
         {
            mesh->mFlags |= TSMesh::Billboard;
            if (mesh->isBillboardZAxis())
               mesh->mFlags |= TSMesh::BillboardZAxis;
         }

         // Set the detail name... do fixups for collision details.
         const char* detailName = "detail";
         if ( mesh->mDetailSize < 0 )
         {
            if (  dStrStartsWith(meshNames[iMesh], "Collision") ||
                  dStrStartsWith(meshNames[iMesh], "Col") )
               detailName = "Collision";
            else if (dStrStartsWith(meshNames[iMesh], "LOSCol"))
               detailName = "LOS";
         }

         // Attempt to add the detail (will fail if it already exists)
         S32 oldNumDetails = mShape->mDetails.size();
         mShape->addDetail(detailName, mesh->mDetailSize, iSub);
         if (mShape->mDetails.size() > oldNumDetails)
         {
            Con::warnf("Object mesh \"%s\" has no matching detail (\"%s%d\" has"
               " been added automatically)", mesh->getName(false), detailName, mesh->mDetailSize);
         }
      }

      // Get object count for this subshape
      mShape->mSubShapeNumObjects.push_back(mShape->mObjects.size() - mShape->mSubShapeFirstObject.last());
   }
}

void TSShapeLoader::generateSkins()
{
   Vector<AppMesh*> skins;
   for (S32 iObject = 0; iObject < mShape->mObjects.size(); iObject++)
   {
      for (S32 iMesh = 0; iMesh < mShape->mObjects[iObject].numMeshes; iMesh++)
      {
         AppMesh* mesh = mAppMeshes[mShape->mObjects[iObject].startMeshIndex + iMesh];
         if (mesh->isSkin())
            skins.push_back(mesh);
      }
   }

   for (S32 iSkin = 0; iSkin < skins.size(); iSkin++)
   {
      updateProgress(Load_GenerateSkins, "Generating skins...", skins.size(), iSkin);

      // Get skin data (bones, vertex weights etc)
      AppMesh* skin = skins[iSkin];
      skin->lookupSkinData();

      // Just copy initial verts and norms for now
      skin->mInitialVerts.set(skin->mPoints.address(), skin->mVertsPerFrame);
      skin->mInitialNorms.set(skin->mNormals.address(), skin->mVertsPerFrame);

      // Map bones to nodes
      skin->mNodeIndex.setSize(skin->mBones.size());
      for (S32 iBone = 0; iBone < skin->mBones.size(); iBone++)
      {
         // Find the node that matches this bone
         skin->mNodeIndex[iBone] = -1;
         for (S32 iNode = 0; iNode < mAppNodes.size(); iNode++)
         {
            if (mAppNodes[iNode]->isEqual(skin->mBones[iBone]))
            {
               delete skin->mBones[iBone];
               skin->mBones[iBone] = mAppNodes[iNode];
               skin->mNodeIndex[iBone] = iNode;
               break;
            }
         }

         if (skin->mNodeIndex[iBone] == -1)
         {
            Con::warnf("Could not find bone %d. Defaulting to first node", iBone);
            skin->mNodeIndex[iBone] = 0;
         }
      }
   }
}

void TSShapeLoader::generateDefaultStates()
{
   // Generate default object states (includes initial geometry)
   for (S32 iObject = 0; iObject < mShape->mObjects.size(); iObject++)
   {
      updateProgress(Load_GenerateDefaultStates, "Generating initial mesh and node states...",
         mShape->mObjects.size(), iObject);

      TSShape::Object& obj = mShape->mObjects[iObject];

      // Calculate the objectOffset for each mesh at T=0
      for (S32 iMesh = 0; iMesh < obj.numMeshes; iMesh++)
      {
         AppMesh* appMesh = mAppMeshes[obj.startMeshIndex + iMesh];
         AppNode* appNode = obj.nodeIndex >= 0 ? mAppNodes[obj.nodeIndex] : mBoundsNode;

         MatrixF meshMat(appMesh->getMeshTransform(smDefaultTime));
         MatrixF nodeMat(appMesh->isSkin() ? meshMat : appNode->getNodeTransform(smDefaultTime));

         zapScale(nodeMat);

         appMesh->mObjectOffset = nodeMat.inverse() * meshMat;
      }

      generateObjectState(mShape->mObjects[iObject], smDefaultTime, true, true);
   }

   // Generate default node transforms
   for (S32 iNode = 0; iNode < mAppNodes.size(); iNode++)
   {
      // Determine the default translation and rotation for the node
      QuatF rot, srot;
      Point3F trans, scale;
      generateNodeTransform(mAppNodes[iNode], smDefaultTime, false, 0, rot, trans, srot, scale);

      // Add default node translation and rotation
      addNodeRotation(rot, true);
      addNodeTranslation(trans, true);
   }
}

void TSShapeLoader::generateObjectState(TSShape::Object& obj, F32 t, bool addFrame, bool addMatFrame)
{
   mShape->mObjectStates.increment();
   TSShape::ObjectState& state = mShape->mObjectStates.last();

   state.frameIndex = 0;
   state.matFrameIndex = 0;
   state.vis = mClampF(mAppMeshes[obj.startMeshIndex]->getVisValue(t), 0.0f, 1.0f);

   if (addFrame || addMatFrame)
   {
      generateFrame(obj, t, addFrame, addMatFrame);

      // set the frame number for the object state
      state.frameIndex = mAppMeshes[obj.startMeshIndex]->mNumFrames - 1;
      state.matFrameIndex = mAppMeshes[obj.startMeshIndex]->mNumMatFrames - 1;
   }
}

void TSShapeLoader::generateFrame(TSShape::Object& obj, F32 t, bool addFrame, bool addMatFrame)
{
   for (S32 iMesh = 0; iMesh < obj.numMeshes; iMesh++)
   {
      AppMesh* appMesh = mAppMeshes[obj.startMeshIndex + iMesh];

      U32 oldNumPoints = appMesh->mPoints.size();
      U32 oldNumUvs = appMesh->mUVs.size();

      // Get the mesh geometry at time, 't'
      // Geometry verts, normals and tverts can be animated (different set for
      // each frame), but the TSDrawPrimitives stay the same, so the way lockMesh
      // works is that it will only generate the primitives once, then after that
      // will just append verts, normals and tverts each time it is called.
      appMesh->lockMesh(t, appMesh->mObjectOffset);

      // Calculate vertex normals if required
      if (appMesh->mNormals.size() != appMesh->mPoints.size())
         appMesh->computeNormals();

      // If this is the first call, set the number of points per frame
      if (appMesh->mNumFrames == 0)
      {
         appMesh->mVertsPerFrame = appMesh->mPoints.size();
      }
      else
      {
         // Check frame topology => ie. that the right number of points, normals
         // and tverts was added
         if ((appMesh->mPoints.size() - oldNumPoints) != appMesh->mVertsPerFrame)
         {
            Con::warnf("Wrong number of points (%d) added at time=%f (expected %d)",
               appMesh->mPoints.size() - oldNumPoints, t, appMesh->mVertsPerFrame);
            addFrame = false;
         }
         if ((appMesh->mNormals.size() - oldNumPoints) != appMesh->mVertsPerFrame)
         {
            Con::warnf("Wrong number of normals (%d) added at time=%f (expected %d)",
               appMesh->mNormals.size() - oldNumPoints, t, appMesh->mVertsPerFrame);
            addFrame = false;
         }
         if ((appMesh->mUVs.size() - oldNumUvs) != appMesh->mVertsPerFrame)
         {
            Con::warnf("Wrong number of tverts (%d) added at time=%f (expected %d)",
               appMesh->mUVs.size() - oldNumUvs, t, appMesh->mVertsPerFrame);
            addMatFrame = false;
         }
      }

      // Because lockMesh adds points, normals AND tverts each call, if we didn't
      // actually want another frame or matFrame, we need to remove them afterwards.
      // In the common case (we DO want the frame), we can do nothing => the
      // points/normals/tverts are already in place!
      if (addFrame)
      {
         appMesh->mNumFrames++;
      }
      else
      {
         appMesh->mPoints.setSize(oldNumPoints);
         appMesh->mNormals.setSize(oldNumPoints);
      }

      if (addMatFrame)
      {
         appMesh->mNumMatFrames++;
      }
      else
      {
         appMesh->mUVs.setSize(oldNumPoints);
      }
   }
}

//-----------------------------------------------------------------------------
// Materials

/// Convert all Collada materials into a single TSMaterialList
void TSShapeLoader::generateMaterialList()
{
   // Install the materials into the material list
   mShape->mMaterialList = new TSMaterialList;
   for (S32 iMat = 0; iMat < AppMesh::mAppMaterials.size(); iMat++)
   {
      updateProgress(Load_GenerateMaterials, "Generating materials...", AppMesh::mAppMaterials.size(), iMat);

      AppMaterial* appMat = AppMesh::mAppMaterials[iMat];
      mShape->mMaterialList->push_back(appMat->getName(), appMat->getFlags(), U32(-1), U32(-1), U32(-1), 1.0f, appMat->getReflectance());
   }
}


//-----------------------------------------------------------------------------
// Animation Sequences

void TSShapeLoader::generateSequences()
{
   for (S32 iSeq = 0; iSeq < mAppSequences.size(); iSeq++)
   {
      updateProgress(Load_GenerateSequences, "Generating sequences...", mAppSequences.size(), iSeq);

      // Initialize the sequence
      mAppSequences[iSeq]->setActive(true);

      mShape->mSequences.increment();
      TSShape::Sequence& seq = mShape->mSequences.last();

      seq.nameIndex = mShape->addName(mAppSequences[iSeq]->getName());
      seq.toolBegin = mAppSequences[iSeq]->getStart();
      seq.priority = mAppSequences[iSeq]->getPriority();
      seq.flags = mAppSequences[iSeq]->getFlags();

      // Compute duration and number of keyframes (then adjust time between frames to match)
      seq.duration = mAppSequences[iSeq]->getEnd() - mAppSequences[iSeq]->getStart();
      seq.numKeyframes = (S32)(seq.duration * mAppSequences[iSeq]->fps + 0.5f) + 1;

      seq.sourceData.start = 0;
      seq.sourceData.end = seq.numKeyframes-1;
      seq.sourceData.total = seq.numKeyframes;

      // Set membership arrays (ie. which nodes and objects are affected by this sequence)
      setNodeMembership(seq, mAppSequences[iSeq]);
      setObjectMembership(seq, mAppSequences[iSeq]);

      // Generate keyframes
      generateNodeAnimation(seq);
      generateObjectAnimation(seq, mAppSequences[iSeq]);
      generateGroundAnimation(seq, mAppSequences[iSeq]);
      generateFrameTriggers(seq, mAppSequences[iSeq]);

      // Set sequence flags
      seq.dirtyFlags = 0;
      if (seq.rotationMatters.testAll() || seq.translationMatters.testAll() || seq.scaleMatters.testAll())
         seq.dirtyFlags |= TSShapeInstance::TransformDirty;
      if (seq.visMatters.testAll())
         seq.dirtyFlags |= TSShapeInstance::VisDirty;
      if (seq.frameMatters.testAll())
         seq.dirtyFlags |= TSShapeInstance::FrameDirty;
      if (seq.matFrameMatters.testAll())
         seq.dirtyFlags |= TSShapeInstance::MatFrameDirty;

      // Set shape flags (only the most significant scale type)
      U32 curVal = mShape->mFlags & TSShape::AnyScale;
      mShape->mFlags &= ~(TSShape::AnyScale);
      mShape->mFlags |= getMax(curVal, seq.flags & TSShape::AnyScale); // take the larger value (can only convert upwards)

      mAppSequences[iSeq]->setActive(false);
   }
}

void TSShapeLoader::setNodeMembership(TSShape::Sequence& seq, const AppSequence* appSeq)
{
   seq.rotationMatters.clearAll();     // node rotation (size = nodes.size())
   seq.translationMatters.clearAll();  // node translation (size = nodes.size())
   seq.scaleMatters.clearAll();        // node scale (size = nodes.size())

   // This shouldn't be allowed, but check anyway...
   if (seq.numKeyframes < 2)
      return;

   // Note: this fills the cache with current sequence data. Methods that get
   // called later (e.g. generateNodeAnimation) use this info (and assume it's set).
   fillNodeTransformCache(seq, appSeq);

   // Test to see if the transform changes over the interval in order to decide
   // whether to animate the transform in 3space. We don't use app's mechanism
   // for doing this because it functions different in different apps and we do
   // some special stuff with scale.
   setRotationMembership(seq);
   setTranslationMembership(seq);
   setScaleMembership(seq);
}

void TSShapeLoader::setRotationMembership(TSShape::Sequence& seq)
{
   for (S32 iNode = 0; iNode < mAppNodes.size(); iNode++)
   {
      // Check if any of the node rotations are different to
      // the default rotation
      QuatF defaultRot;
      mShape->mDefaultRotations[iNode].getQuatF(&defaultRot);

      for (S32 iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
      {
         if (mNodeRotCache[iNode][iFrame] != defaultRot)
         {
            seq.rotationMatters.set(iNode);
            break;
         }
      }
   }
}

void TSShapeLoader::setTranslationMembership(TSShape::Sequence& seq)
{
   for (S32 iNode = 0; iNode < mAppNodes.size(); iNode++)
   {
      // Check if any of the node translations are different to
      // the default translation
      Point3F& defaultTrans = mShape->mDefaultTranslations[iNode];

      for (S32 iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
      {
         if (!mNodeTransCache[iNode][iFrame].equal(defaultTrans))
         {
            seq.translationMatters.set(iNode);
            break;
         }
      }
   }
}

void TSShapeLoader::setScaleMembership(TSShape::Sequence& seq)
{
   Point3F unitScale(1,1,1);

   U32 arbitraryScaleCount = 0;
   U32 alignedScaleCount = 0;
   U32 uniformScaleCount = 0;

   for (S32 iNode = 0; iNode < mAppNodes.size(); iNode++)
   {
      // Check if any of the node scales are not the unit scale
      for (S32 iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
      {
         Point3F& scale = mNodeScaleCache[iNode][iFrame];
         if (!unitScale.equal(scale))
         {
            // Determine what type of scale this is
            if (!mNodeScaleRotCache[iNode][iFrame].isIdentity())
               arbitraryScaleCount++;
            else if (scale.x != scale.y || scale.y != scale.z)
               alignedScaleCount++;
            else
               uniformScaleCount++;

            seq.scaleMatters.set(iNode);
            break;
         }
      }
   }

   // Only one type of scale is animated
   if (arbitraryScaleCount)
      seq.flags |= TSShape::ArbitraryScale;
   else if (alignedScaleCount)
      seq.flags |= TSShape::AlignedScale;
   else if (uniformScaleCount)
      seq.flags |= TSShape::UniformScale;
}

void TSShapeLoader::setObjectMembership(TSShape::Sequence& seq, const AppSequence* appSeq)
{
   seq.visMatters.clearAll();          // object visibility (size = objects.size())
   seq.frameMatters.clearAll();        // vert animation (morph) (size = objects.size())
   seq.matFrameMatters.clearAll();     // UV animation (size = objects.size())

   for (S32 iObject = 0; iObject < mShape->mObjects.size(); iObject++)
   {
      if (!mAppMeshes[mShape->mObjects[iObject].startMeshIndex])
         continue;

      if (mAppMeshes[mShape->mObjects[iObject].startMeshIndex]->animatesVis(appSeq))
         seq.visMatters.set(iObject);
      // Morph and UV animation has been deprecated
      //if (appMeshes[shape->objects[iObject].startMeshIndex]->animatesFrame(appSeq))
         //seq.frameMatters.set(iObject);
      //if (appMeshes[shape->objects[iObject].startMeshIndex]->animatesMatFrame(appSeq))
         //seq.matFrameMatters.set(iObject);
   }
}

void TSShapeLoader::clearNodeTransformCache()
{
   // clear out the transform caches
   for (S32 i = 0; i < mNodeRotCache.size(); i++)
      delete [] mNodeRotCache[i];
   mNodeRotCache.clear();
   for (S32 i = 0; i < mNodeTransCache.size(); i++)
      delete [] mNodeTransCache[i];
   mNodeTransCache.clear();
   for (S32 i = 0; i < mNodeScaleRotCache.size(); i++)
      delete [] mNodeScaleRotCache[i];
   mNodeScaleRotCache.clear();
   for (S32 i = 0; i < mNodeScaleCache.size(); i++)
      delete [] mNodeScaleCache[i];
   mNodeScaleCache.clear();
}

void TSShapeLoader::fillNodeTransformCache(TSShape::Sequence& seq, const AppSequence* appSeq)
{
   // clear out the transform caches and set it up for this sequence
   clearNodeTransformCache();

   mNodeRotCache.setSize(mAppNodes.size());
   for (S32 i = 0; i < mNodeRotCache.size(); i++)
      mNodeRotCache[i] = new QuatF[seq.numKeyframes];
   mNodeTransCache.setSize(mAppNodes.size());
   for (S32 i = 0; i < mNodeTransCache.size(); i++)
      mNodeTransCache[i] = new Point3F[seq.numKeyframes];
   mNodeScaleRotCache.setSize(mAppNodes.size());
   for (S32 i = 0; i < mNodeScaleRotCache.size(); i++)
      mNodeScaleRotCache[i] = new QuatF[seq.numKeyframes];
   mNodeScaleCache.setSize(mAppNodes.size());
   for (S32 i = 0; i < mNodeScaleCache.size(); i++)
      mNodeScaleCache[i] = new Point3F[seq.numKeyframes];

   // get the node transforms for every frame
   for (S32 iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
   {
      F32 time = appSeq->getStart() + seq.duration * iFrame / getMax(1, seq.numKeyframes - 1);
      for (S32 iNode = 0; iNode < mAppNodes.size(); iNode++)
      {
         generateNodeTransform(mAppNodes[iNode], time, seq.isBlend(), appSeq->getBlendRefTime(),
                               mNodeRotCache[iNode][iFrame], mNodeTransCache[iNode][iFrame],
                               mNodeScaleRotCache[iNode][iFrame], mNodeScaleCache[iNode][iFrame]);
      }
   }
}

void TSShapeLoader::addNodeRotation(QuatF& rot, bool defaultVal)
{
   Quat16 rot16;
   rot16.set(rot);

   if (!defaultVal)
      mShape->mNodeRotations.push_back(rot16);
   else
      mShape->mDefaultRotations.push_back(rot16);
}

void TSShapeLoader::addNodeTranslation(Point3F& trans, bool defaultVal)
{
   if (!defaultVal)
      mShape->mNodeTranslations.push_back(trans);
   else
      mShape->mDefaultTranslations.push_back(trans);
}

void TSShapeLoader::addNodeUniformScale(F32 scale)
{
   mShape->mNodeUniformScales.push_back(scale);
}

void TSShapeLoader::addNodeAlignedScale(Point3F& scale)
{
   mShape->mNodeAlignedScales.push_back(scale);
}

void TSShapeLoader::addNodeArbitraryScale(QuatF& qrot, Point3F& scale)
{
   Quat16 rot16;
   rot16.set(qrot);
   mShape->mNodeArbitraryScaleRots.push_back(rot16);
   mShape->mNodeArbitraryScaleFactors.push_back(scale);
}

void TSShapeLoader::generateNodeAnimation(TSShape::Sequence& seq)
{
   seq.baseRotation = mShape->mNodeRotations.size();
   seq.baseTranslation = mShape->mNodeTranslations.size();
   seq.baseScale = (seq.flags & TSShape::ArbitraryScale) ? mShape->mNodeArbitraryScaleRots.size() :
                   (seq.flags & TSShape::AlignedScale) ? mShape->mNodeAlignedScales.size() :
                   mShape->mNodeUniformScales.size();

   for (S32 iNode = 0; iNode < mAppNodes.size(); iNode++)
   {
      for (S32 iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
      {
         if (seq.rotationMatters.test(iNode))
            addNodeRotation(mNodeRotCache[iNode][iFrame], false);
         if (seq.translationMatters.test(iNode))
            addNodeTranslation(mNodeTransCache[iNode][iFrame], false);
         if (seq.scaleMatters.test(iNode))
         {
            QuatF& rot = mNodeScaleRotCache[iNode][iFrame];
            Point3F scale = mNodeScaleCache[iNode][iFrame];

            if (seq.flags & TSShape::ArbitraryScale)
               addNodeArbitraryScale(rot, scale);
            else if (seq.flags & TSShape::AlignedScale)
               addNodeAlignedScale(scale);
            else if (seq.flags & TSShape::UniformScale)
               addNodeUniformScale((scale.x+scale.y+scale.z)/3.0f);
         }
      }
   }
}

void TSShapeLoader::generateObjectAnimation(TSShape::Sequence& seq, const AppSequence* appSeq)
{
   seq.baseObjectState = mShape->mObjectStates.size();

   for (S32 iObject = 0; iObject < mShape->mObjects.size(); iObject++)
   {
      bool visMatters = seq.visMatters.test(iObject);
      bool frameMatters = seq.frameMatters.test(iObject);
      bool matFrameMatters = seq.matFrameMatters.test(iObject);

      if (visMatters || frameMatters || matFrameMatters)
      {
         for (S32 iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
         {
            F32 time = appSeq->getStart() + seq.duration * iFrame / getMax(1, seq.numKeyframes - 1);
            generateObjectState(mShape->mObjects[iObject], time, frameMatters, matFrameMatters);
         }
      }
   }
}

void TSShapeLoader::generateGroundAnimation(TSShape::Sequence& seq, const AppSequence* appSeq)
{
   seq.firstGroundFrame = mShape->mGroundTranslations.size();
   seq.numGroundFrames = 0;

   if (!mBoundsNode)
      return;

   // Check if the bounds node is animated by this sequence
   seq.numGroundFrames = (S32)((seq.duration + 0.25f/smAppGroundFrameRate) * smAppGroundFrameRate);

   seq.flags |= TSShape::MakePath;

   // Get ground transform at the start of the sequence
   MatrixF invStartMat = mBoundsNode->getNodeTransform(appSeq->getStart());
   zapScale(invStartMat);
   invStartMat.inverse();

   for (S32 iFrame = 0; iFrame < seq.numGroundFrames; iFrame++)
   {
      F32 time = appSeq->getStart() + seq.duration * iFrame / getMax(1, seq.numGroundFrames - 1);

      // Determine delta bounds node transform at 't'
      MatrixF mat = mBoundsNode->getNodeTransform(time);
      zapScale(mat);
      mat = invStartMat * mat;

      // Add ground transform
      Quat16 rotation;
      rotation.set(QuatF(mat));
      mShape->mGroundTranslations.push_back(mat.getPosition());
      mShape->mGroundRotations.push_back(rotation);
   }
}

void TSShapeLoader::generateFrameTriggers(TSShape::Sequence& seq, const AppSequence* appSeq)
{
   // Initialize triggers
   seq.firstTrigger = mShape->mTriggers.size();
   seq.numTriggers  = appSeq->getNumTriggers();
   if (!seq.numTriggers)
      return;

   seq.flags |= TSShape::MakePath;

   // Add triggers
   for (S32 iTrigger = 0; iTrigger < seq.numTriggers; iTrigger++)
   {
      mShape->mTriggers.increment();
      appSeq->getTrigger(iTrigger, mShape->mTriggers.last());
   }

   // Track the triggers that get turned off by this shape...normally, triggers
   // aren't turned on/off, just on...if we are a trigger that does both then we
   // need to mark ourselves as such so that on/off can become off/on when sequence
   // is played in reverse...
   U32 offTriggers = 0;
   for (S32 iTrigger = 0; iTrigger < seq.numTriggers; iTrigger++)
   {
      U32 state = mShape->mTriggers[seq.firstTrigger+iTrigger].state;
      if ((state & TSShape::Trigger::StateOn) == 0)
         offTriggers |= (state & TSShape::Trigger::StateMask);
   }

   // We now know which states are turned off, set invert on all those (including when turned on)
   for (int iTrigger = 0; iTrigger < seq.numTriggers; iTrigger++)
   {
      if (mShape->mTriggers[seq.firstTrigger + iTrigger].state & offTriggers)
         mShape->mTriggers[seq.firstTrigger + iTrigger].state |= TSShape::Trigger::InvertOnReverse;
   }
}

//-----------------------------------------------------------------------------

void TSShapeLoader::sortDetails()
{
   // Sort objects by: transparency, material index and node index


   // Insert NULL meshes where required
   for (S32 iSub = 0; iSub < mSubShapes.size(); iSub++)
   {
      Vector<S32> validDetails;
      mShape->getSubShapeDetails(iSub, validDetails);

      for (S32 iDet = 0; iDet < validDetails.size(); iDet++)
      {
         TSShape::Detail &detail = mShape->mDetails[validDetails[iDet]];
         if (detail.subShapeNum >= 0)
            detail.objectDetailNum = iDet;

         for (S32 iObj = mShape->mSubShapeFirstObject[iSub];
            iObj < (mShape->mSubShapeFirstObject[iSub] + mShape->mSubShapeNumObjects[iSub]);
            iObj++)
         {
            TSShape::Object &object = mShape->mObjects[iObj];

            // Insert a NULL mesh for this detail level if required (ie. if the
            // object does not already have a mesh with an equal or higher detail)
            S32 meshIndex = (iDet < object.numMeshes) ? iDet : object.numMeshes-1;

            if (mAppMeshes[object.startMeshIndex + meshIndex]->mDetailSize < mShape->mDetails[iDet].size)
            {
               // Add a NULL mesh
               mAppMeshes.insert(object.startMeshIndex + iDet, NULL);
               object.numMeshes++;

               // Fixup the start index for the other objects
               for (S32 k = iObj+1; k < mShape->mObjects.size(); k++)
                  mShape->mObjects[k].startMeshIndex++;
            }
         }
      }
   }
}

// Install into the TSShape, the shape is expected to be empty.
// Data is not copied, the TSShape is modified to point to memory
// managed by this object.  This object is also bound to the TSShape
// object and will be deleted when it's deleted.
void TSShapeLoader::install()
{
   // Arrays that are filled in by ts shape init, but need
   // to be allocated beforehand.
   mShape->mSubShapeFirstTranslucentObject.setSize(mShape->mSubShapeFirstObject.size());

   // Construct TS sub-meshes
   mShape->mMeshes.setSize(mAppMeshes.size());
   for (U32 m = 0; m < mAppMeshes.size(); m++)
      mShape->mMeshes[m] = mAppMeshes[m] ? mAppMeshes[m]->constructTSMesh() : NULL;

   // Remove empty meshes and objects
   for (S32 iObj = mShape->mObjects.size()-1; iObj >= 0; iObj--)
   {
      TSShape::Object& obj = mShape->mObjects[iObj];
      for (S32 iMesh = obj.numMeshes-1; iMesh >= 0; iMesh--)
      {
         TSMesh *mesh = mShape->mMeshes[obj.startMeshIndex + iMesh];

         if (mesh && !mesh->mPrimitives.size())
         {
            S32 oldMeshCount = obj.numMeshes;
            destructInPlace(mesh);
            mShape->removeMeshFromObject(iObj, iMesh);
            iMesh -= (oldMeshCount - obj.numMeshes - 1);      // handle when more than one mesh is removed
         }
      }

      if (!obj.numMeshes)
         mShape->removeObject(mShape->getName(obj.nameIndex));
   }

   // Add a dummy object if needed so the shape loads and renders ok
   if (!mShape->mDetails.size())
   {
      mShape->addDetail("detail", 2, 0);
      mShape->mSubShapeNumObjects.last() = 1;

      mShape->mMeshes.push_back(NULL);

      mShape->mObjects.increment();
      mShape->mObjects.last().nameIndex = mShape->addName("dummy");
      mShape->mObjects.last().nodeIndex = 0;
      mShape->mObjects.last().startMeshIndex = 0;
      mShape->mObjects.last().numMeshes = 1;

      mShape->mObjectStates.increment();
      mShape->mObjectStates.last().frameIndex = 0;
      mShape->mObjectStates.last().matFrameIndex = 0;
      mShape->mObjectStates.last().vis = 1.0f;
   }

   // Update smallest visible detail
   mShape->mSmallestVisibleDL = -1;
   mShape->mSmallestVisibleSize = 999999;
   for (S32 i = 0; i < mShape->mDetails.size(); i++)
   {
      if ((mShape->mDetails[i].size >= 0) &&
         (mShape->mDetails[i].size < mShape->mSmallestVisibleSize))
      {
         mShape->mSmallestVisibleDL = i;
         mShape->mSmallestVisibleSize = mShape->mDetails[i].size;
      }
   }

   computeBounds(mShape->mBounds);
   if (!mShape->mBounds.isValidBox())
      mShape->mBounds = Box3F(1.0f);

   mShape->mBounds.getCenter(&mShape->mCenter);
   mShape->mRadius = (mShape->mBounds.maxExtents - mShape->mCenter).len();
   mShape->mTubeRadius = mShape->mRadius;

   mShape->init();
}

void TSShapeLoader::computeBounds(Box3F& bounds)
{
   // Compute the box that encloses the model geometry
   bounds = Box3F::Invalid;

   // Use bounds node geometry if present
   if ( mBoundsNode && mBoundsNode->getNumMesh() )
   {
      for (S32 iMesh = 0; iMesh < mBoundsNode->getNumMesh(); iMesh++)
      {
         AppMesh* mesh = mBoundsNode->getMesh( iMesh );
         if ( !mesh )
            continue;

         Box3F meshBounds;
         mesh->computeBounds( meshBounds );
         if ( meshBounds.isValidBox() )
            bounds.intersect( meshBounds );
      }
   }
   else
   {
      // Compute bounds based on all geometry in the model
      for (S32 iMesh = 0; iMesh < mAppMeshes.size(); iMesh++)
      {
         AppMesh* mesh = mAppMeshes[iMesh];
         if ( !mesh )
            continue;

         Box3F meshBounds;
         mesh->computeBounds( meshBounds );
         if ( meshBounds.isValidBox() )
            bounds.intersect( meshBounds );
      }
   }
}

TSShapeLoader::~TSShapeLoader()
{
   clearNodeTransformCache();

   // Clear shared AppMaterial list
   for (S32 iMat = 0; iMat < AppMesh::mAppMaterials.size(); iMat++)
      delete AppMesh::mAppMaterials[iMat];
   AppMesh::mAppMaterials.clear();

   // Delete Subshapes
   delete mBoundsNode;
   for (S32 iSub = 0; iSub < mSubShapes.size(); iSub++)
      delete mSubShapes[iSub];

   // Delete AppSequences
   for (S32 iSeq = 0; iSeq < mAppSequences.size(); iSeq++)
      delete mAppSequences[iSeq];
   mAppSequences.clear();   
}

// Static functions to handle supported formats for shape loader.
void TSShapeLoader::addFormat(String name, String extension)
{
   ShapeFormat newFormat;
   newFormat.mName = name;
   newFormat.mExtension = extension;
   smFormats.push_back(newFormat);
}

String TSShapeLoader::getFormatExtensions()
{
   // "*.dsq TAB *.dae TAB
   StringBuilder output;
   for(U32 n = 0; n < smFormats.size(); ++n)
   {
      output.append("*.");
      output.append(smFormats[n].mExtension);
      output.append("\t");
   }
   return output.end();
}

String TSShapeLoader::getFormatFilters()
{
   // "DSQ Files|*.dsq|COLLADA Files|*.dae|"
   StringBuilder output;
   for(U32 n = 0; n < smFormats.size(); ++n)
   {
      output.append(smFormats[n].mName);
      output.append("|*.");
      output.append(smFormats[n].mExtension);
      output.append("|");
   }
   return output.end();
}

DefineConsoleFunction( getFormatExtensions, const char*, ( ),, 
  "Returns a list of supported shape format extensions separated by tabs."
  "Example output: *.dsq TAB *.dae TAB")
{
   return Con::getReturnBuffer(TSShapeLoader::getFormatExtensions());
}

DefineConsoleFunction( getFormatFilters, const char*, ( ),, 
  "Returns a list of supported shape formats in filter form.\n"
  "Example output: DSQ Files|*.dsq|COLLADA Files|*.dae|")
{
   return Con::getReturnBuffer(TSShapeLoader::getFormatFilters());
}
