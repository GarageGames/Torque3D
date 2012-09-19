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
#include "ts/loader/tsShapeLoader.h"

#include "core/volume.h"
#include "materials/materialList.h"
#include "materials/matInstance.h"
#include "materials/materialManager.h"
#include "ts/tsShapeInstance.h"
#include "ts/tsMaterialList.h"


const F32 TSShapeLoader::DefaultTime = -1.0f;
const double TSShapeLoader::MinFrameRate = 15.0f;
const double TSShapeLoader::MaxFrameRate = 60.0f;
const double TSShapeLoader::AppGroundFrameRate = 10.0f;
Torque::Path TSShapeLoader::shapePath;

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
   MatrixF m10 = node->getNodeTransform(DefaultTime);
   m1.scale(Point3F(1.0f/m10.getScale().x, 1.0f/m10.getScale().y, 1.0f/m10.getScale().z));

   if (node->mParentIndex >= 0)
   {
      AppNode *parent = appNodes[node->mParentIndex];

      MatrixF m2 = parent->getNodeTransform(t);

      // multiply by inverse scale at t=0
      MatrixF m20 = parent->getNodeTransform(DefaultTime);
      m2.scale(Point3F(1.0f/m20.getScale().x, 1.0f/m20.getScale().y, 1.0f/m20.getScale().z));

      // get local transform by pre-multiplying by inverted parent transform
      m1 = m2.inverse() * m1;
   }
   else if (boundsNode && node != boundsNode)
   {
      // make transform relative to bounds node transform at time=t
      MatrixF mb = boundsNode->getNodeTransform(t);
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

void TSShapeLoader::updateProgress(int major, const char* msg, int numMinor, int minor)
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
   shapePath = path;
   shape = new TSShape();

   shape->mExporterVersion = 124;
   shape->mSmallestVisibleSize = 999999;
   shape->mSmallestVisibleDL = 0;
   shape->mReadVersion = 24;
   shape->mFlags = 0;
   shape->mSequencesConstructed = 0;

   // Get all nodes, objects and sequences in the shape
   updateProgress(Load_EnumerateScene, "Enumerating scene...");
   enumerateScene();
   if (!subshapes.size())
   {
      delete shape;
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

   return shape;
}

bool TSShapeLoader::processNode(AppNode* node)
{
   // Detect bounds node
   if ( node->isBounds() )
   {
      if ( boundsNode )
      {
         Con::warnf( "More than one bounds node found" );
         return false;
      }
      boundsNode = node;

      // Process bounds geometry
      MatrixF boundsMat(boundsNode->getNodeTransform(DefaultTime));
      boundsMat.inverse();
      zapScale(boundsMat);
      for (S32 iMesh = 0; iMesh < boundsNode->getNumMesh(); iMesh++)
      {
         AppMesh* mesh = boundsNode->getMesh(iMesh);
         MatrixF transform = mesh->getMeshTransform(DefaultTime);
         transform.mulL(boundsMat);
         mesh->lockMesh(DefaultTime, transform);
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
   if ( subshapes.size() == 0 )
      subshapes.push_back( new TSShapeLoader::Subshape );

   subshapes.last()->branches.push_back( node );

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
   const int MAX_ITERATIONS = 0x10000;   // maximum of 4 characters (A-P) will be appended

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

   S32 subShapeNum = shape->subShapeFirstNode.size()-1;
   Subshape* subshape = subshapes[subShapeNum];

   // Check if we should collapse this node
   S32 myIndex;
   if (ignoreNode(appNode->getName()))
   {
      myIndex = parentIndex;
   }
   else
   {
      // Check that adding this node will not exceed the maximum node count
      if (shape->nodes.size() >= MAX_TS_SET_SIZE)
         return;

      myIndex = shape->nodes.size();
      String nodeName = getUniqueName(appNode->getName(), cmpShapeName, shape->names);

      // Create the 3space node
      shape->nodes.increment();
      shape->nodes.last().nameIndex = shape->addName(nodeName);
      shape->nodes.last().parentIndex = parentIndex;
      shape->nodes.last().firstObject = -1;
      shape->nodes.last().firstChild = -1;
      shape->nodes.last().nextSibling = -1;

      // Add the AppNode to a matching list (so AppNodes can be accessed using 3space
      // node indices)
      appNodes.push_back(appNode);
      appNodes.last()->mParentIndex = parentIndex;

      // Check for NULL detail or AutoBillboard nodes (no children or geometry)
      if ((appNode->getNumChildNodes() == 0) &&
          (appNode->getNumMesh() == 0))
      {
         S32 size = 0x7FFFFFFF;
         String dname(String::GetTrailingNumber(appNode->getName(), size));

         if (dStrEqual(dname, "nulldetail") && (size != 0x7FFFFFFF))
         {
            shape->addDetail("detail", size, subShapeNum);
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

            S32 detIndex = shape->addDetail( "bbDetail", size, -1 );
            shape->details[detIndex].bbEquatorSteps = numEquatorSteps;
            shape->details[detIndex].bbPolarSteps = numPolarSteps;
            shape->details[detIndex].bbDetailLevel = dl;
            shape->details[detIndex].bbDimension = dim;
            shape->details[detIndex].bbIncludePoles = includePoles;
            shape->details[detIndex].bbPolarAngle = polarAngle;
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
      for (int iChild = 0; iChild < appNode->getNumChildNodes(); iChild++)
         recurseSubshape(appNode->getChildNode(iChild), myIndex, true);
   }
}

void TSShapeLoader::generateSubshapes()
{
   for (U32 iSub = 0; iSub < subshapes.size(); iSub++)
   {
      updateProgress(Load_GenerateSubshapes, "Generating subshapes...", subshapes.size(), iSub);

      Subshape* subshape = subshapes[iSub];

      // Recurse through the node hierarchy, adding 3space nodes and
      // collecting geometry
      S32 firstNode = shape->nodes.size();
      shape->subShapeFirstNode.push_back(firstNode);      

      for (U32 iBranch = 0; iBranch < subshape->branches.size(); iBranch++)
         recurseSubshape(subshape->branches[iBranch], -1, true);

      shape->subShapeNumNodes.push_back(shape->nodes.size() - firstNode);

      if (shape->nodes.size() >= MAX_TS_SET_SIZE)
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
   S32                     meshSize = (S32)arg2;

   for (S32 i = 0; i < names.size(); i++)
   {
      if (names[i].compare(key, 0, String::NoCase) == 0)
      {
         if (meshes[i]->detailSize == meshSize)
            return false;
      }
   }
   return true;
}

void TSShapeLoader::generateObjects()
{
   for (S32 iSub = 0; iSub < subshapes.size(); iSub++)
   {
      Subshape* subshape = subshapes[iSub];
      shape->subShapeFirstObject.push_back(shape->objects.size());

      // Get the names and sizes of the meshes for this subshape
      Vector<String> meshNames;
      for (S32 iMesh = 0; iMesh < subshape->objMeshes.size(); iMesh++)
      {
         AppMesh* mesh = subshape->objMeshes[iMesh];
         mesh->detailSize = 2;
         String name = String::GetTrailingNumber( mesh->getName(), mesh->detailSize );
         name = getUniqueName( name, cmpMeshNameAndSize, meshNames, &(subshape->objMeshes), (void*)mesh->detailSize );
         meshNames.push_back( name );

         // Fix up any collision details that don't have a negative detail level.
         if (  dStrStartsWith(meshNames[iMesh], "Collision") ||
               dStrStartsWith(meshNames[iMesh], "LOSCol") )
         {
            if (mesh->detailSize > 0)
               mesh->detailSize = -mesh->detailSize;
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
               (subshape->objMeshes[i]->detailSize < subshape->objMeshes[j]->detailSize)))
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
            shape->objects.increment();
            shape->objects.last().nameIndex = shape->addName(meshNames[iMesh]);
            shape->objects.last().nodeIndex = subshape->objNodes[iMesh];
            shape->objects.last().startMeshIndex = appMeshes.size();
            shape->objects.last().numMeshes = 0;
            lastName = &meshNames[iMesh];
         }

         // Add this mesh to the object
         appMeshes.push_back(mesh);
         shape->objects.last().numMeshes++;

         // Set mesh flags
         mesh->flags = 0;
         if (mesh->isBillboard())
         {
            mesh->flags |= TSMesh::Billboard;
            if (mesh->isBillboardZAxis())
               mesh->flags |= TSMesh::BillboardZAxis;
         }

         // Set the detail name... do fixups for collision details.
         const char* detailName = "detail";
         if ( mesh->detailSize < 0 )
         {
            if (  dStrStartsWith(meshNames[iMesh], "Collision") ||
                  dStrStartsWith(meshNames[iMesh], "Col") )
               detailName = "Collision";
            else if (dStrStartsWith(meshNames[iMesh], "LOSCol"))
               detailName = "LOS";
         }

         // Attempt to add the detail (will fail if it already exists)
         S32 oldNumDetails = shape->details.size();
         shape->addDetail(detailName, mesh->detailSize, iSub);
         if (shape->details.size() > oldNumDetails)
         {
            Con::warnf("Object mesh \"%s\" has no matching detail (\"%s%d\" has"
               " been added automatically)", mesh->getName(false), detailName, mesh->detailSize);
         }
      }

      // Get object count for this subshape
      shape->subShapeNumObjects.push_back(shape->objects.size() - shape->subShapeFirstObject.last());
   }
}

void TSShapeLoader::generateSkins()
{
   Vector<AppMesh*> skins;
   for (int iObject = 0; iObject < shape->objects.size(); iObject++)
   {
      for (int iMesh = 0; iMesh < shape->objects[iObject].numMeshes; iMesh++)
      {
         AppMesh* mesh = appMeshes[shape->objects[iObject].startMeshIndex + iMesh];
         if (mesh->isSkin())
            skins.push_back(mesh);
      }
   }

   for (int iSkin = 0; iSkin < skins.size(); iSkin++)
   {
      updateProgress(Load_GenerateSkins, "Generating skins...", skins.size(), iSkin);

      // Get skin data (bones, vertex weights etc)
      AppMesh* skin = skins[iSkin];
      skin->lookupSkinData();

      // Just copy initial verts and norms for now
      skin->initialVerts.set(skin->points.address(), skin->vertsPerFrame);
      skin->initialNorms.set(skin->normals.address(), skin->vertsPerFrame);

      // Map bones to nodes
      skin->nodeIndex.setSize(skin->bones.size());
      for (int iBone = 0; iBone < skin->bones.size(); iBone++)
      {
         // Find the node that matches this bone
         skin->nodeIndex[iBone] = -1;
         for (int iNode = 0; iNode < appNodes.size(); iNode++)
         {
            if (appNodes[iNode]->isEqual(skin->bones[iBone]))
            {
               delete skin->bones[iBone];
               skin->bones[iBone] = appNodes[iNode];
               skin->nodeIndex[iBone] = iNode;
               break;
            }
         }

         if (skin->nodeIndex[iBone] == -1)
         {
            Con::warnf("Could not find bone %d. Defaulting to first node", iBone);
            skin->nodeIndex[iBone] = 0;
         }
      }
   }
}

void TSShapeLoader::generateDefaultStates()
{
   // Generate default object states (includes initial geometry)
   for (int iObject = 0; iObject < shape->objects.size(); iObject++)
   {
      updateProgress(Load_GenerateDefaultStates, "Generating initial mesh and node states...",
         shape->objects.size(), iObject);

      TSShape::Object& obj = shape->objects[iObject];

      // Calculate the objectOffset for each mesh at T=0
      for (int iMesh = 0; iMesh < obj.numMeshes; iMesh++)
      {
         AppMesh* appMesh = appMeshes[obj.startMeshIndex + iMesh];
         AppNode* appNode = obj.nodeIndex >= 0 ? appNodes[obj.nodeIndex] : boundsNode;

         MatrixF meshMat(appMesh->getMeshTransform(DefaultTime));
         MatrixF nodeMat(appMesh->isSkin() ? meshMat : appNode->getNodeTransform(DefaultTime));

         zapScale(nodeMat);

         appMesh->objectOffset = nodeMat.inverse() * meshMat;
      }

      generateObjectState(shape->objects[iObject], DefaultTime, true, true);
   }

   // Generate default node transforms
   for (int iNode = 0; iNode < appNodes.size(); iNode++)
   {
      // Determine the default translation and rotation for the node
      QuatF rot, srot;
      Point3F trans, scale;
      generateNodeTransform(appNodes[iNode], DefaultTime, false, 0, rot, trans, srot, scale);

      // Add default node translation and rotation
      addNodeRotation(rot, true);
      addNodeTranslation(trans, true);
   }
}

void TSShapeLoader::generateObjectState(TSShape::Object& obj, F32 t, bool addFrame, bool addMatFrame)
{
   shape->objectStates.increment();
   TSShape::ObjectState& state = shape->objectStates.last();

   state.frameIndex = 0;
   state.matFrameIndex = 0;
   state.vis = mClampF(appMeshes[obj.startMeshIndex]->getVisValue(t), 0.0f, 1.0f);

   if (addFrame || addMatFrame)
   {
      generateFrame(obj, t, addFrame, addMatFrame);

      // set the frame number for the object state
      state.frameIndex = appMeshes[obj.startMeshIndex]->numFrames - 1;
      state.matFrameIndex = appMeshes[obj.startMeshIndex]->numMatFrames - 1;
   }
}

void TSShapeLoader::generateFrame(TSShape::Object& obj, F32 t, bool addFrame, bool addMatFrame)
{
   for (int iMesh = 0; iMesh < obj.numMeshes; iMesh++)
   {
      AppMesh* appMesh = appMeshes[obj.startMeshIndex + iMesh];

      U32 oldNumPoints = appMesh->points.size();
      U32 oldNumUvs = appMesh->uvs.size();

      // Get the mesh geometry at time, 't'
      // Geometry verts, normals and tverts can be animated (different set for
      // each frame), but the TSDrawPrimitives stay the same, so the way lockMesh
      // works is that it will only generate the primitives once, then after that
      // will just append verts, normals and tverts each time it is called.
      appMesh->lockMesh(t, appMesh->objectOffset);

      // Calculate vertex normals if required
      if (appMesh->normals.size() != appMesh->points.size())
         appMesh->computeNormals();

      // If this is the first call, set the number of points per frame
      if (appMesh->numFrames == 0)
      {
         appMesh->vertsPerFrame = appMesh->points.size();
      }
      else
      {
         // Check frame topology => ie. that the right number of points, normals
         // and tverts was added
         if ((appMesh->points.size() - oldNumPoints) != appMesh->vertsPerFrame)
         {
            Con::warnf("Wrong number of points (%d) added at time=%f (expected %d)",
               appMesh->points.size() - oldNumPoints, t, appMesh->vertsPerFrame);
            addFrame = false;
         }
         if ((appMesh->normals.size() - oldNumPoints) != appMesh->vertsPerFrame)
         {
            Con::warnf("Wrong number of normals (%d) added at time=%f (expected %d)",
               appMesh->normals.size() - oldNumPoints, t, appMesh->vertsPerFrame);
            addFrame = false;
         }
         if ((appMesh->uvs.size() - oldNumUvs) != appMesh->vertsPerFrame)
         {
            Con::warnf("Wrong number of tverts (%d) added at time=%f (expected %d)",
               appMesh->uvs.size() - oldNumUvs, t, appMesh->vertsPerFrame);
            addMatFrame = false;
         }
      }

      // Because lockMesh adds points, normals AND tverts each call, if we didn't
      // actually want another frame or matFrame, we need to remove them afterwards.
      // In the common case (we DO want the frame), we can do nothing => the
      // points/normals/tverts are already in place!
      if (addFrame)
      {
         appMesh->numFrames++;
      }
      else
      {
         appMesh->points.setSize(oldNumPoints);
         appMesh->normals.setSize(oldNumPoints);
      }

      if (addMatFrame)
      {
         appMesh->numMatFrames++;
      }
      else
      {
         appMesh->uvs.setSize(oldNumPoints);
      }
   }
}

//-----------------------------------------------------------------------------
// Materials

/// Convert all Collada materials into a single TSMaterialList
void TSShapeLoader::generateMaterialList()
{
   // Install the materials into the material list
   shape->materialList = new TSMaterialList;
   for (int iMat = 0; iMat < AppMesh::appMaterials.size(); iMat++)
   {
      updateProgress(Load_GenerateMaterials, "Generating materials...", AppMesh::appMaterials.size(), iMat);

      AppMaterial* appMat = AppMesh::appMaterials[iMat];
      shape->materialList->push_back(appMat->getName(), appMat->getFlags(), U32(-1), U32(-1), U32(-1), 1.0f, appMat->getReflectance());
   }
}


//-----------------------------------------------------------------------------
// Animation Sequences

void TSShapeLoader::generateSequences()
{
   for (int iSeq = 0; iSeq < appSequences.size(); iSeq++)
   {
      updateProgress(Load_GenerateSequences, "Generating sequences...", appSequences.size(), iSeq);

      // Initialize the sequence
      appSequences[iSeq]->setActive(true);

      shape->sequences.increment();
      TSShape::Sequence& seq = shape->sequences.last();

      seq.nameIndex = shape->addName(appSequences[iSeq]->getName());
      seq.toolBegin = appSequences[iSeq]->getStart();
      seq.priority = appSequences[iSeq]->getPriority();
      seq.flags = appSequences[iSeq]->getFlags();

      // Compute duration and number of keyframes (then adjust time between frames to match)
      seq.duration = appSequences[iSeq]->getEnd() - appSequences[iSeq]->getStart();
      seq.numKeyframes = (S32)(seq.duration * appSequences[iSeq]->fps + 0.5f) + 1;

      seq.sourceData.start = 0;
      seq.sourceData.end = seq.numKeyframes-1;
      seq.sourceData.total = seq.numKeyframes;

      // Set membership arrays (ie. which nodes and objects are affected by this sequence)
      setNodeMembership(seq, appSequences[iSeq]);
      setObjectMembership(seq, appSequences[iSeq]);

      // Generate keyframes
      generateNodeAnimation(seq);
      generateObjectAnimation(seq, appSequences[iSeq]);
      generateGroundAnimation(seq, appSequences[iSeq]);
      generateFrameTriggers(seq, appSequences[iSeq]);

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
      U32 curVal = shape->mFlags & TSShape::AnyScale;
      shape->mFlags &= ~(TSShape::AnyScale);
      shape->mFlags |= getMax(curVal, seq.flags & TSShape::AnyScale); // take the larger value (can only convert upwards)

      appSequences[iSeq]->setActive(false);
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
   for (int iNode = 0; iNode < appNodes.size(); iNode++)
   {
      // Check if any of the node rotations are different to
      // the default rotation
      QuatF defaultRot;
      shape->defaultRotations[iNode].getQuatF(&defaultRot);

      for (int iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
      {
         if (nodeRotCache[iNode][iFrame] != defaultRot)
         {
            seq.rotationMatters.set(iNode);
            break;
         }
      }
   }
}

void TSShapeLoader::setTranslationMembership(TSShape::Sequence& seq)
{
   for (int iNode = 0; iNode < appNodes.size(); iNode++)
   {
      // Check if any of the node translations are different to
      // the default translation
      Point3F& defaultTrans = shape->defaultTranslations[iNode];

      for (int iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
      {
         if (!nodeTransCache[iNode][iFrame].equal(defaultTrans))
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

   for (int iNode = 0; iNode < appNodes.size(); iNode++)
   {
      // Check if any of the node scales are not the unit scale
      for (int iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
      {
         Point3F& scale = nodeScaleCache[iNode][iFrame];
         if (!unitScale.equal(scale))
         {
            // Determine what type of scale this is
            if (!nodeScaleRotCache[iNode][iFrame].isIdentity())
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

   for (int iObject = 0; iObject < shape->objects.size(); iObject++)
   {
      if (!appMeshes[shape->objects[iObject].startMeshIndex])
         continue;

      if (appMeshes[shape->objects[iObject].startMeshIndex]->animatesVis(appSeq))
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
   for (int i = 0; i < nodeRotCache.size(); i++)
      delete [] nodeRotCache[i];
   nodeRotCache.clear();
   for (int i = 0; i < nodeTransCache.size(); i++)
      delete [] nodeTransCache[i];
   nodeTransCache.clear();
   for (int i = 0; i < nodeScaleRotCache.size(); i++)
      delete [] nodeScaleRotCache[i];
   nodeScaleRotCache.clear();
   for (int i = 0; i < nodeScaleCache.size(); i++)
      delete [] nodeScaleCache[i];
   nodeScaleCache.clear();
}

void TSShapeLoader::fillNodeTransformCache(TSShape::Sequence& seq, const AppSequence* appSeq)
{
   // clear out the transform caches and set it up for this sequence
   clearNodeTransformCache();

   nodeRotCache.setSize(appNodes.size());
   for (int i = 0; i < nodeRotCache.size(); i++)
      nodeRotCache[i] = new QuatF[seq.numKeyframes];
   nodeTransCache.setSize(appNodes.size());
   for (int i = 0; i < nodeTransCache.size(); i++)
      nodeTransCache[i] = new Point3F[seq.numKeyframes];
   nodeScaleRotCache.setSize(appNodes.size());
   for (int i = 0; i < nodeScaleRotCache.size(); i++)
      nodeScaleRotCache[i] = new QuatF[seq.numKeyframes];
   nodeScaleCache.setSize(appNodes.size());
   for (int i = 0; i < nodeScaleCache.size(); i++)
      nodeScaleCache[i] = new Point3F[seq.numKeyframes];

   // get the node transforms for every frame
   for (int iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
   {
      F32 time = appSeq->getStart() + seq.duration * iFrame / getMax(1, seq.numKeyframes - 1);
      for (int iNode = 0; iNode < appNodes.size(); iNode++)
      {
         generateNodeTransform(appNodes[iNode], time, seq.isBlend(), appSeq->getBlendRefTime(),
                               nodeRotCache[iNode][iFrame], nodeTransCache[iNode][iFrame],
                               nodeScaleRotCache[iNode][iFrame], nodeScaleCache[iNode][iFrame]);
      }
   }
}

void TSShapeLoader::addNodeRotation(QuatF& rot, bool defaultVal)
{
   Quat16 rot16;
   rot16.set(rot);

   if (!defaultVal)
      shape->nodeRotations.push_back(rot16);
   else
      shape->defaultRotations.push_back(rot16);
}

void TSShapeLoader::addNodeTranslation(Point3F& trans, bool defaultVal)
{
   if (!defaultVal)
      shape->nodeTranslations.push_back(trans);
   else
      shape->defaultTranslations.push_back(trans);
}

void TSShapeLoader::addNodeUniformScale(F32 scale)
{
   shape->nodeUniformScales.push_back(scale);
}

void TSShapeLoader::addNodeAlignedScale(Point3F& scale)
{
   shape->nodeAlignedScales.push_back(scale);
}

void TSShapeLoader::addNodeArbitraryScale(QuatF& qrot, Point3F& scale)
{
   Quat16 rot16;
   rot16.set(qrot);
   shape->nodeArbitraryScaleRots.push_back(rot16);
   shape->nodeArbitraryScaleFactors.push_back(scale);
}

void TSShapeLoader::generateNodeAnimation(TSShape::Sequence& seq)
{
   seq.baseRotation = shape->nodeRotations.size();
   seq.baseTranslation = shape->nodeTranslations.size();
   seq.baseScale = (seq.flags & TSShape::ArbitraryScale) ? shape->nodeArbitraryScaleRots.size() :
                   (seq.flags & TSShape::AlignedScale) ? shape->nodeAlignedScales.size() :
                   shape->nodeUniformScales.size();

   for (int iNode = 0; iNode < appNodes.size(); iNode++)
   {
      for (int iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
      {
         if (seq.rotationMatters.test(iNode))
            addNodeRotation(nodeRotCache[iNode][iFrame], false);
         if (seq.translationMatters.test(iNode))
            addNodeTranslation(nodeTransCache[iNode][iFrame], false);
         if (seq.scaleMatters.test(iNode))
         {
            QuatF& rot = nodeScaleRotCache[iNode][iFrame];
            Point3F scale = nodeScaleCache[iNode][iFrame];

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
   seq.baseObjectState = shape->objectStates.size();

   for (int iObject = 0; iObject < shape->objects.size(); iObject++)
   {
      bool visMatters = seq.visMatters.test(iObject);
      bool frameMatters = seq.frameMatters.test(iObject);
      bool matFrameMatters = seq.matFrameMatters.test(iObject);

      if (visMatters || frameMatters || matFrameMatters)
      {
         for (int iFrame = 0; iFrame < seq.numKeyframes; iFrame++)
         {
            F32 time = appSeq->getStart() + seq.duration * iFrame / getMax(1, seq.numKeyframes - 1);
            generateObjectState(shape->objects[iObject], time, frameMatters, matFrameMatters);
         }
      }
   }
}

void TSShapeLoader::generateGroundAnimation(TSShape::Sequence& seq, const AppSequence* appSeq)
{
   seq.firstGroundFrame = shape->groundTranslations.size();
   seq.numGroundFrames = 0;

   if (!boundsNode)
      return;

   // Check if the bounds node is animated by this sequence
   seq.numGroundFrames = (S32)((seq.duration + 0.25f/AppGroundFrameRate) * AppGroundFrameRate);

   seq.flags |= TSShape::MakePath;

   // Get ground transform at the start of the sequence
   MatrixF invStartMat = boundsNode->getNodeTransform(appSeq->getStart());
   zapScale(invStartMat);
   invStartMat.inverse();

   for (int iFrame = 0; iFrame < seq.numGroundFrames; iFrame++)
   {
      F32 time = appSeq->getStart() + seq.duration * iFrame / getMax(1, seq.numGroundFrames - 1);

      // Determine delta bounds node transform at 't'
      MatrixF mat = boundsNode->getNodeTransform(time);
      zapScale(mat);
      mat = invStartMat * mat;

      // Add ground transform
      Quat16 rotation;
      rotation.set(QuatF(mat));
      shape->groundTranslations.push_back(mat.getPosition());
      shape->groundRotations.push_back(rotation);
   }
}

void TSShapeLoader::generateFrameTriggers(TSShape::Sequence& seq, const AppSequence* appSeq)
{
   // Initialize triggers
   seq.firstTrigger = shape->triggers.size();
   seq.numTriggers  = appSeq->getNumTriggers();
   if (!seq.numTriggers)
      return;

   seq.flags |= TSShape::MakePath;

   // Add triggers
   for (int iTrigger = 0; iTrigger < seq.numTriggers; iTrigger++)
   {
      shape->triggers.increment();
      appSeq->getTrigger(iTrigger, shape->triggers.last());
   }

   // Track the triggers that get turned off by this shape...normally, triggers
   // aren't turned on/off, just on...if we are a trigger that does both then we
   // need to mark ourselves as such so that on/off can become off/on when sequence
   // is played in reverse...
   U32 offTriggers = 0;
   for (int iTrigger = 0; iTrigger < seq.numTriggers; iTrigger++)
   {
      U32 state = shape->triggers[seq.firstTrigger+iTrigger].state;
      if ((state & TSShape::Trigger::StateOn) == 0)
         offTriggers |= (state & TSShape::Trigger::StateMask);
   }

   // We now know which states are turned off, set invert on all those (including when turned on)
   for (int iTrigger = 0; iTrigger < seq.numTriggers; iTrigger++)
   {
      if (shape->triggers[seq.firstTrigger + iTrigger].state & offTriggers)
         shape->triggers[seq.firstTrigger + iTrigger].state |= TSShape::Trigger::InvertOnReverse;
   }
}

//-----------------------------------------------------------------------------

void TSShapeLoader::sortDetails()
{
   // Sort objects by: transparency, material index and node index


   // Insert NULL meshes where required
   for (int iSub = 0; iSub < subshapes.size(); iSub++)
   {
      Vector<S32> validDetails;
      shape->getSubShapeDetails(iSub, validDetails);

      for (int iDet = 0; iDet < validDetails.size(); iDet++)
      {
         TSShape::Detail &detail = shape->details[validDetails[iDet]];
         if (detail.subShapeNum >= 0)
            detail.objectDetailNum = iDet;

         for (int iObj = shape->subShapeFirstObject[iSub];
            iObj < (shape->subShapeFirstObject[iSub] + shape->subShapeNumObjects[iSub]);
            iObj++)
         {
            TSShape::Object &object = shape->objects[iObj];

            // Insert a NULL mesh for this detail level if required (ie. if the
            // object does not already have a mesh with an equal or higher detail)
            S32 meshIndex = (iDet < object.numMeshes) ? iDet : object.numMeshes-1;

            if (appMeshes[object.startMeshIndex + meshIndex]->detailSize < shape->details[iDet].size)
            {
               // Add a NULL mesh
               appMeshes.insert(object.startMeshIndex + iDet, NULL);
               object.numMeshes++;

               // Fixup the start index for the other objects
               for (int k = iObj+1; k < shape->objects.size(); k++)
                  shape->objects[k].startMeshIndex++;
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
   shape->subShapeFirstTranslucentObject.setSize(shape->subShapeFirstObject.size());

   // Construct TS sub-meshes
   shape->meshes.setSize(appMeshes.size());
   for (U32 m = 0; m < appMeshes.size(); m++)
      shape->meshes[m] = appMeshes[m] ? appMeshes[m]->constructTSMesh() : NULL;

   // Remove empty meshes and objects
   for (S32 iObj = shape->objects.size()-1; iObj >= 0; iObj--)
   {
      TSShape::Object& obj = shape->objects[iObj];
      for (S32 iMesh = obj.numMeshes-1; iMesh >= 0; iMesh--)
      {
         TSMesh *mesh = shape->meshes[obj.startMeshIndex + iMesh];

         if (mesh && !mesh->primitives.size())
         {
            S32 oldMeshCount = obj.numMeshes;
            destructInPlace(mesh);
            shape->removeMeshFromObject(iObj, iMesh);
            iMesh -= (oldMeshCount - obj.numMeshes - 1);      // handle when more than one mesh is removed
         }
      }

      if (!obj.numMeshes)
         shape->removeObject(shape->getName(obj.nameIndex));
   }

   // Add a dummy object if needed so the shape loads and renders ok
   if (!shape->details.size())
   {
      shape->addDetail("detail", 2, 0);
      shape->subShapeNumObjects.last() = 1;

      shape->meshes.push_back(NULL);

      shape->objects.increment();
      shape->objects.last().nameIndex = shape->addName("dummy");
      shape->objects.last().nodeIndex = 0;
      shape->objects.last().startMeshIndex = 0;
      shape->objects.last().numMeshes = 1;

      shape->objectStates.increment();
      shape->objectStates.last().frameIndex = 0;
      shape->objectStates.last().matFrameIndex = 0;
      shape->objectStates.last().vis = 1.0f;
   }

   // Update smallest visible detail
   shape->mSmallestVisibleDL = -1;
   shape->mSmallestVisibleSize = 999999;
   for (S32 i = 0; i < shape->details.size(); i++)
   {
      if ((shape->details[i].size >= 0) &&
         (shape->details[i].size < shape->mSmallestVisibleSize))
      {
         shape->mSmallestVisibleDL = i;
         shape->mSmallestVisibleSize = shape->details[i].size;
      }
   }

   computeBounds(shape->bounds);
   if (!shape->bounds.isValidBox())
      shape->bounds = Box3F(1.0f);

   shape->bounds.getCenter(&shape->center);
   shape->radius = (shape->bounds.maxExtents - shape->center).len();
   shape->tubeRadius = shape->radius;

   shape->init();
}

void TSShapeLoader::computeBounds(Box3F& bounds)
{
   // Compute the box that encloses the model geometry
   bounds = Box3F::Invalid;

   // Use bounds node geometry if present
   if ( boundsNode && boundsNode->getNumMesh() )
   {
      for (S32 iMesh = 0; iMesh < boundsNode->getNumMesh(); iMesh++)
      {
         AppMesh* mesh = boundsNode->getMesh( iMesh );
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
      for (S32 iMesh = 0; iMesh < appMeshes.size(); iMesh++)
      {
         AppMesh* mesh = appMeshes[iMesh];
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
   for (int iMat = 0; iMat < AppMesh::appMaterials.size(); iMat++)
      delete AppMesh::appMaterials[iMat];
   AppMesh::appMaterials.clear();

   // Delete Subshapes
   delete boundsNode;
   for (int iSub = 0; iSub < subshapes.size(); iSub++)
      delete subshapes[iSub];

   // Delete AppSequences
   for (int iSeq = 0; iSeq < appSequences.size(); iSeq++)
      delete appSequences[iSeq];
   appSequences.clear();   
}
