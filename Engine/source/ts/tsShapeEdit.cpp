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

#include "console/consoleTypes.h"
#include "core/resourceManager.h"
#include "ts/tsShape.h"
#include "ts/tsShapeInstance.h"
#include "ts/tsLastDetail.h"
#include "ts/tsMaterialList.h"
#include "core/stream/fileStream.h"
#include "core/volume.h"


//-----------------------------------------------------------------------------

S32 TSShape::addName(const String& name)
{
   // Check for empty names
   if (name.isEmpty())
      return -1;

   // Return the index of the new name (add if it is unique)
   S32 index = findName(name);
   if (index >= 0)
      return index;

   names.push_back(StringTable->insert(name));
   return names.size()-1;
}

void TSShape::updateSmallestVisibleDL()
{
   // Update smallest visible detail
   mSmallestVisibleDL = -1;
   mSmallestVisibleSize = F32_MAX;
   F32 maxSize = 0.0f;
   for (S32 i = 0; i < details.size(); i++)
   {
      maxSize = getMax( maxSize, details[i].size );

      if ((details[i].size >= 0) && (details[i].size < mSmallestVisibleSize))
      {
         mSmallestVisibleDL = i;
         mSmallestVisibleSize = details[i].size;
      }
   }

   // Initialize the detail level lod lookup table.
   mDetailLevelLookup.setSize( (U32)( maxSize * 2.0f ) + 2 );
   for ( U32 l=0; l < mDetailLevelLookup.size(); l++ )
   {
      F32 pixelSize = (F32)l;
      S32 dl = -1;

      for ( U32 d=0; d < details.size(); d++ )
      {
         // Break when we get to hidden detail 
         // levels like collision shapes.
         if ( details[d].size < 0 )
            break;

         if ( pixelSize > details[d].size )
         {
            dl = d;
            break;
         }

         if ( d + 1 >= details.size() || details[d+1].size < 0 )
         {
            // We've run out of details and haven't found anything?
            // Let's just grab this one.
            dl = d;
            break;
         }
      }

      // Calculate the intra detail level.
      F32 intraDL = 0;
      if ( dl > -1 )
      {
         F32 curSize = details[dl].size;
         F32 nextSize = dl == 0 ? 2.0f * curSize : details[dl - 1].size;
         intraDL = mClampF( nextSize - curSize > 0.01f ? (pixelSize - curSize) / (nextSize - curSize) : 1.0f, 0, 1 );
      }

      mDetailLevelLookup[l].set( dl, intraDL );
   }

   // Test for using the legacy screen error
   // lod method here instead of runtime.
   //
   // See setDetailFromDistance().
   //
   mUseDetailFromScreenError =   mSmallestVisibleDL >= 0 && 
                                 details.first().maxError >= 0;
}

S32 TSShape::addDetail(const String& dname, S32 size, S32 subShapeNum)
{
   S32 nameIndex = addName(avar("%s%d", dname.c_str(), size));

   // Check if this detail size has already been added
   S32 index;
   for (index = 0; index < details.size(); index++)
   {
      if ((details[index].size == size) &&
         (details[index].subShapeNum == subShapeNum) &&
         (details[index].nameIndex == nameIndex))
         return index;
      if (details[index].size < size)
         break;
   }

   // Create a new detail level at the right index, so array
   // remains sorted by detail size (from largest to smallest)
   details.insert(index);
   TSShape::Detail &detail = details[index];

   // Clear the detail to ensure no garbage values
   // are left in any vars we don't set.
   dMemset( &detail, 0, sizeof( Detail ) );

   // Setup the detail.
   detail.nameIndex = nameIndex;
   detail.size = size;
   detail.subShapeNum = subShapeNum;
   detail.objectDetailNum = 0;
   detail.averageError = -1;
   detail.maxError = -1;
   detail.polyCount = 0;

   // Resize alpha vectors
   alphaIn.increment();
   alphaOut.increment();

   // Fixup objectDetailNum in other detail levels
   for (int i = index+1; i < details.size(); i++)
   {
      if ((details[i].subShapeNum >= 0) &&
         ((subShapeNum == -1) || (details[i].subShapeNum == subShapeNum)))
         details[i].objectDetailNum++;
   }

   // Update smallest visible detail
   updateSmallestVisibleDL();

   return index;
}

S32 TSShape::addImposter(const String& cachePath, S32 size, S32 numEquatorSteps,
                        S32 numPolarSteps, S32 dl, S32 dim, bool includePoles, F32 polarAngle)
{
   // Check if the desired size is already in use
   bool isNewDetail = false;
   S32 detIndex = findDetailBySize( size );

   if ( detIndex >= 0 )
   {
      // Size is in use. If the detail is already an imposter, we can just change
      // the settings, otherwise quit
      if ( details[detIndex].subShapeNum >= 0 )
      {
         Con::errorf( "TSShape::addImposter: A non-billboard detail already "
            "exists at size %d", size );
         return -1;
      }
   }
   else
   {
      // Size is not in use. If an imposter already exists, change its size, otherwise
      // create a new detail
      for ( detIndex = 0; detIndex < details.size(); ++detIndex )
      {
         if ( details[detIndex].subShapeNum < 0 )
         {
            // Change the imposter detail size
            setDetailSize( details[detIndex].size, size );
            break;
         }
      }
      if ( detIndex == details.size() )
      {
         isNewDetail = true;
         detIndex = addDetail( "bbDetail", size, -1 );
      }
   }

   // Now set the billboard properties.
   Detail &detail = details[detIndex];

   // In prior to DTS version 26 we would pack the autobillboard
   // into this single 32bit value.  This was prone to overflows
   // of parameters caused random bugs.
   //
   // Set the old autobillboard properties var to zero.
   detail.objectDetailNum = 0;
   
   // We now use the new vars.
   detail.bbEquatorSteps = numEquatorSteps;
   detail.bbPolarSteps = numPolarSteps;
   detail.bbPolarAngle = polarAngle;
   detail.bbDetailLevel = dl;
   detail.bbDimension = dim;
   detail.bbIncludePoles = includePoles;

   // Rebuild billboard details or force an update of the modified detail
   if ( isNewDetail )
   {
      // Add NULL meshes for this detail
      for ( S32 iObj = 0; iObj < objects.size(); ++iObj )
      {
         if ( detIndex < objects[iObj].numMeshes )
         {
            objects[iObj].numMeshes++;
            meshes.insert( objects[iObj].startMeshIndex + detIndex, NULL );
            for (S32 j = iObj + 1; j < objects.size(); ++j )
               objects[j].startMeshIndex++;
         }
      }

      // Could be dedicated server.
      if ( GFXDevice::devicePresent() )
         setupBillboardDetails( cachePath );

      while ( detailCollisionAccelerators.size() < details.size() )
         detailCollisionAccelerators.push_back( NULL );
   }
   else
   {
      if ( billboardDetails.size() && GFXDevice::devicePresent() )
      {
         delete billboardDetails[detIndex];
         billboardDetails[detIndex] = new TSLastDetail(
                                          this,
                                          cachePath,
                                          detail.bbEquatorSteps,
                                          detail.bbPolarSteps,
                                          detail.bbPolarAngle,
                                          detail.bbIncludePoles,
                                          detail.bbDetailLevel,
                                          detail.bbDimension );

         billboardDetails[detIndex]->update( true );
      }
   }

   return detIndex;
}

bool TSShape::removeImposter()
{
   // Find the imposter detail level
   S32 detIndex;
   for ( detIndex = 0; detIndex < details.size(); ++detIndex )
   {
      if ( details[detIndex].subShapeNum < 0 )
         break;
   }

   if ( detIndex == details.size() )
   {
      Con::errorf( "TSShape::removeImposter: No imposter detail level found in shape" );
      return false;
   }

   // Remove the detail level
   details.erase( detIndex );

   if ( detIndex < billboardDetails.size() )
   {
      // Delete old textures
      TSLastDetail* bb = billboardDetails[detIndex];
      bb->deleteImposterCacheTextures();
      delete billboardDetails[detIndex];
   }
   billboardDetails.clear();

   detailCollisionAccelerators.erase( detIndex );

   // Remove the (NULL) meshes from each object
   for ( S32 iObj = 0; iObj < objects.size(); ++iObj )
   {
      if ( detIndex < objects[iObj].numMeshes )
      {
         objects[iObj].numMeshes--;
         meshes.erase( objects[iObj].startMeshIndex + detIndex );
         for (S32 j = iObj + 1; j < objects.size(); ++j )
            objects[j].startMeshIndex--;
      }
   }

   // Update smallest visible size
   updateSmallestVisibleDL();

   return true;
}

//-----------------------------------------------------------------------------

/// Get the index of the element in the group with a given name
template<class T> S32 findByName(Vector<T>& group, S32 nameIndex)
{
   for (S32 i = 0; i < group.size(); i++)
      if (group[i].nameIndex == nameIndex)
         return i;
   return -1;
}

/// Adjust the nameIndex for elements in the group
template<class T> void adjustForNameRemoval(Vector<T>& group, S32 nameIndex)
{
   for (S32 i = 0; i < group.size(); i++)
      if (group[i].nameIndex > nameIndex)
         group[i].nameIndex--;
}

bool TSShape::removeName(const String& name)
{
   // Check if the name is still in use
   S32 nameIndex = findName(name);
   if ((findByName(nodes, nameIndex) >= 0)      ||
       (findByName(objects, nameIndex) >= 0)    ||
       (findByName(sequences, nameIndex) >= 0)  ||
       (findByName(details, nameIndex) >= 0))
       return false;

   // Remove the name, then update nameIndex for affected elements
   names.erase(nameIndex);

   adjustForNameRemoval(nodes, nameIndex);
   adjustForNameRemoval(objects, nameIndex);
   adjustForNameRemoval(sequences, nameIndex);
   adjustForNameRemoval(details, nameIndex);

   return true;
}

//-----------------------------------------------------------------------------

template<class T> bool doRename(TSShape* shape, Vector<T>& group, const String& oldName, const String& newName)
{
   // Find the element in the group with the oldName
   S32 index = findByName(group, shape->findName(oldName));
   if (index < 0)
   {
      Con::errorf("TSShape::rename: Could not find '%s'", oldName.c_str());
      return false;
   }

   // Ignore trivial renames
   if (oldName.equal(newName, String::NoCase))
      return true;

   // Check that this name is not already in use
   if (findByName(group, shape->findName(newName)) >= 0)
   {
      Con::errorf("TSShape::rename: '%s' is already in use", newName.c_str());
      return false;
   }

   // Do the rename (the old name will be removed if it is no longer in use)
   group[index].nameIndex = shape->addName(newName);
   shape->removeName(oldName);
   return true;
}

bool TSShape::renameNode(const String& oldName, const String& newName)
{
   return doRename(this, nodes, oldName, newName);
}

bool TSShape::renameObject(const String& oldName, const String& newName)
{
   return doRename(this, objects, oldName, newName);
}

bool TSShape::renameDetail(const String& oldName, const String& newName)
{
   return doRename(this, details, oldName, newName);
}

bool TSShape::renameSequence(const String& oldName, const String& newName)
{
   return doRename(this, sequences, oldName, newName);
}

//-----------------------------------------------------------------------------

bool TSShape::addNode(const String& name, const String& parentName, const Point3F& pos, const QuatF& rot)
{
   // Check that adding this node would not exceed the maximum count
   if (nodes.size() >= MAX_TS_SET_SIZE)
   {
      Con::errorf("TSShape::addNode: Cannot add node, shape already has maximum (%d) nodes", MAX_TS_SET_SIZE);
      return false;
   }

   // Check that there is not already a node with this name
   if (findNode(name) >= 0)
   {
      Con::errorf("TSShape::addNode: %s already exists!", name.c_str());
      return false;
   }

   // Find the parent node (OK for name to be empty => node is at root level)
   S32 parentIndex = -1;
   if (dStrcmp(parentName, ""))
   {
      parentIndex = findNode(parentName);
      if (parentIndex < 0)
      {
         Con::errorf("TSShape::addNode: Could not find parent node '%s'", parentName.c_str());
         return false;
      }
   }

   // Insert node at the end of the subshape
   S32 subShapeIndex = (parentIndex >= 0) ? getSubShapeForNode(parentIndex) : 0;
   S32 nodeIndex = subShapeNumNodes[subShapeIndex];

   // Adjust subshape node indices
   subShapeNumNodes[subShapeIndex]++;
   for (S32 i = subShapeIndex + 1; i < subShapeFirstNode.size(); i++)
      subShapeFirstNode[i]++;

   // Update animation sequences
   for (S32 iSeq = 0; iSeq < sequences.size(); iSeq++)
   {
      // Update animation matters arrays (new node is not animated)
      TSShape::Sequence& seq = sequences[iSeq];
      seq.translationMatters.insert(nodeIndex, false);
      seq.rotationMatters.insert(nodeIndex, false);
      seq.scaleMatters.insert(nodeIndex, false);
   }

   // Insert the new node
   TSShape::Node node;
   node.nameIndex = addName(name);
   node.parentIndex = parentIndex;
   node.firstChild = -1;
   node.firstObject = -1;
   node.nextSibling = -1;
   nodes.insert(nodeIndex, node);

   // Insert node default translation and rotation
   Quat16 rot16;
   rot16.set(rot);
   defaultTranslations.insert(nodeIndex, pos);
   defaultRotations.insert(nodeIndex, rot16);

   // Fixup node indices
   for (S32 i = 0; i < nodes.size(); i++)
   {
      if (nodes[i].parentIndex >= nodeIndex)
         nodes[i].parentIndex++;
   }
   for (S32 i = 0; i < objects.size(); i++)
   {
      if (objects[i].nodeIndex >= nodeIndex)
         objects[i].nodeIndex++;
   }
   for (S32 i = 0; i < meshes.size(); i++)
   {
      if (meshes[i] && (meshes[i]->getMeshType() == TSMesh::SkinMeshType))
      {
         TSSkinMesh* skin = dynamic_cast<TSSkinMesh*>(meshes[i]);
         for (S32 j = 0; j < skin->batchData.nodeIndex.size(); j++)
         {
            if (skin->batchData.nodeIndex[j] >= nodeIndex)
               skin->batchData.nodeIndex[j]++;
         }
      }
   }

   // Re-initialise the shape
   init();

   return true;
}

/// Erase animation keyframes (translation, rotation etc)
template<class T> S32 eraseStates(Vector<T>& vec, const TSIntegerSet& matters, S32 base, S32 numKeyframes, S32 index=-1)
{
   S32 dest, count;
   if (index == -1)
   {
      // Erase for all nodes/objects
      dest = base;
      count = numKeyframes * matters.count();
   }
   else
   {
      // Erase for the indexed node/object only
      dest = base + matters.count(index)*numKeyframes;
      count = numKeyframes;
   }

   // Erase the values
   if (count)
   {
      if ((dest + count) < vec.size())
         dCopyArray(&vec[dest], &vec[dest + count], vec.size() - (dest + count));
      vec.decrement(count);
   }
   return count;
}

bool TSShape::removeNode(const String& name)
{
   // Find the node to be removed
   S32 nodeIndex = findNode(name);
   if (nodeIndex < 0)
   {
      Con::errorf("TSShape::removeNode: Could not find node '%s'", name.c_str());
      return false;
   }

   S32 nodeParentIndex = nodes[nodeIndex].parentIndex;

   // Warn if there are objects attached to this node
   Vector<S32> nodeObjects;
   getNodeObjects(nodeIndex, nodeObjects);
   if (nodeObjects.size())
   {
      Con::warnf("TSShape::removeNode: Node '%s' has %d objects attached, these "
         "will be reassigned to the node's parent ('%s')", name.c_str(), nodeObjects.size(),
         ((nodeParentIndex >= 0) ? getName(nodes[nodeParentIndex].nameIndex).c_str() : "null"));
   }

   // Update animation sequences
   for (S32 iSeq = 0; iSeq < sequences.size(); iSeq++)
   {
      TSShape::Sequence& seq = sequences[iSeq];

      // Remove animated node transforms
      if (seq.translationMatters.test(nodeIndex))
         eraseStates(nodeTranslations, seq.translationMatters, seq.baseTranslation, seq.numKeyframes, nodeIndex);
      if (seq.rotationMatters.test(nodeIndex))
         eraseStates(nodeRotations, seq.rotationMatters, seq.baseRotation, seq.numKeyframes, nodeIndex);
      if (seq.scaleMatters.test(nodeIndex))
      {
         if (seq.flags & TSShape::ArbitraryScale)
         {
            eraseStates(nodeArbitraryScaleRots, seq.scaleMatters, seq.baseScale, seq.numKeyframes, nodeIndex);
            eraseStates(nodeArbitraryScaleFactors, seq.scaleMatters, seq.baseScale, seq.numKeyframes, nodeIndex);
         }
         else if (seq.flags & TSShape::AlignedScale)
            eraseStates(nodeAlignedScales, seq.scaleMatters, seq.baseScale, seq.numKeyframes, nodeIndex);
         else
            eraseStates(nodeUniformScales, seq.scaleMatters, seq.baseScale, seq.numKeyframes, nodeIndex);
      }

      seq.translationMatters.erase(nodeIndex);
      seq.rotationMatters.erase(nodeIndex);
      seq.scaleMatters.erase(nodeIndex);
   }

   // Remove the node
   nodes.erase(nodeIndex);
   defaultTranslations.erase(nodeIndex);
   defaultRotations.erase(nodeIndex);

   // Adjust subshape node indices
   S32 subShapeIndex = getSubShapeForNode(nodeIndex);
   subShapeNumNodes[subShapeIndex]--;
   for (S32 i = subShapeIndex + 1; i < subShapeFirstNode.size(); i++)
      subShapeFirstNode[i]--;

   // Fixup node parent indices
   for (S32 i = 0; i < nodes.size(); i++)
   {
      if (nodes[i].parentIndex == nodeIndex)
         nodes[i].parentIndex = -1;
      else if (nodes[i].parentIndex > nodeIndex)
         nodes[i].parentIndex--;
   }
   if (nodeParentIndex > nodeIndex)
      nodeParentIndex--;

   // Fixup object node indices, and re-assign attached objects to node's parent
   for (S32 i = 0; i < objects.size(); i++)
   {
      if (objects[i].nodeIndex == nodeIndex)
         objects[i].nodeIndex = nodeParentIndex;
      if (objects[i].nodeIndex > nodeIndex)
         objects[i].nodeIndex--;
   }

   // Fixup skin weight node indices, and re-assign weights for deleted node to its parent
   for (S32 i = 0; i < meshes.size(); i++)
   {
      if (meshes[i] && (meshes[i]->getMeshType() == TSMesh::SkinMeshType))
      {
         TSSkinMesh* skin = dynamic_cast<TSSkinMesh*>(meshes[i]);
         for (S32 j = 0; j < skin->batchData.nodeIndex.size(); j++)
         {
            if (skin->batchData.nodeIndex[j] == nodeIndex)
               skin->batchData.nodeIndex[j] = nodeParentIndex;
            if (skin->batchData.nodeIndex[j] > nodeIndex)
               skin->batchData.nodeIndex[j]--;
         }
      }
   }

   // Remove the sequence name if it is no longer in use
   removeName(name);

   // Re-initialise the shape
   init();

   return true;
}

//-----------------------------------------------------------------------------

bool TSShape::setNodeTransform(const String& name, const Point3F& pos, const QuatF& rot)
{
   // Find the node to be transformed
   S32 nodeIndex = findNode(name);
   if (nodeIndex < 0)
   {
      Con::errorf("TSShape::setNodeTransform: Could not find node '%s'", name.c_str());
      return false;
   }

   // Update initial node position and rotation
   defaultTranslations[nodeIndex] = pos;
   defaultRotations[nodeIndex].set(rot);

   return true;
}

//-----------------------------------------------------------------------------

S32 TSShape::addObject(const String& objName, S32 subShapeIndex)
{
   S32 objIndex = subShapeNumObjects[subShapeIndex];

   // Add object to subshape
   subShapeNumObjects[subShapeIndex]++;
   for (S32 i = subShapeIndex + 1; i < subShapeFirstObject.size(); i++)
      subShapeFirstObject[i]++;

   TSShape::Object obj;
   obj.nameIndex = addName(objName);
   obj.nodeIndex = 0;
   obj.numMeshes = 0;
   obj.startMeshIndex = (objIndex == 0) ? 0 : objects[objIndex-1].startMeshIndex + objects[objIndex-1].numMeshes;
   obj.firstDecal = 0;
   obj.nextSibling = 0;
   objects.insert(objIndex, obj);

   // Add default object state
   TSShape::ObjectState state;
   state.frameIndex = 0;
   state.matFrameIndex = 0;
   state.vis = 1.0f;
   objectStates.insert(objIndex, state);

   // Fixup sequences
   for (S32 i = 0; i < sequences.size(); i++)
      sequences[i].baseObjectState++;

   return objIndex;
}

void TSShape::addMeshToObject(S32 objIndex, S32 meshIndex, TSMesh* mesh)
{
   TSShape::Object& obj = objects[objIndex];

   // Pad with NULLs if required
   S32 oldNumMeshes = obj.numMeshes;
   if (mesh)
   {
      for (S32 i = obj.numMeshes; i < meshIndex; i++)
      {
         meshes.insert(obj.startMeshIndex + i, NULL);
         obj.numMeshes++;
      }
   }

   // Insert the new mesh
   meshes.insert(obj.startMeshIndex + meshIndex, mesh);
   obj.numMeshes++;

   // Skinned meshes are not attached to any node
   if (mesh && (mesh->getMeshType() == TSMesh::SkinMeshType))
      obj.nodeIndex = -1;

   // Fixup mesh indices for other objects
   for (S32 i = 0; i < objects.size(); i++)
   {
      if ((i != objIndex) && (objects[i].startMeshIndex >= obj.startMeshIndex))
         objects[i].startMeshIndex += (obj.numMeshes - oldNumMeshes);
   }
}

void TSShape::removeMeshFromObject(S32 objIndex, S32 meshIndex)
{
   TSShape::Object& obj = objects[objIndex];

   // Remove the mesh, but do not destroy it (this must be done by the caller)
   meshes[obj.startMeshIndex + meshIndex] = NULL;

   // Check if there are any objects remaining that have a valid mesh at this
   // detail size
   bool removeDetail = true;
   for (S32 i = 0; i < objects.size(); i++)
   {
      if ((meshIndex < objects[i].numMeshes) && meshes[objects[i].startMeshIndex + meshIndex])
      {
         removeDetail = false;
         break;
      }
   }

   // Remove detail level if possible
   if (removeDetail)
   {
      for (S32 i = 0; i < objects.size(); i++)
      {
         if (meshIndex < objects[i].numMeshes)
         {
            meshes.erase(objects[i].startMeshIndex + meshIndex);
            objects[i].numMeshes--;

            for (S32 j = 0; j < objects.size(); j++)
            {
               if (objects[j].startMeshIndex > objects[i].startMeshIndex)
                  objects[j].startMeshIndex--;
            }
         }
      }

      Vector<S32> validDetails;
      getSubShapeDetails(getSubShapeForObject(objIndex), validDetails);

      for (S32 i = 0; i < validDetails.size(); i++)
      {
         TSShape::Detail& detail = details[validDetails[i]];
         if (detail.objectDetailNum > meshIndex)
            detail.objectDetailNum--;
      }

      details.erase(validDetails[meshIndex]);
   }

   // Remove trailing NULL meshes from the object
   S32 oldNumMeshes = obj.numMeshes;
   while (obj.numMeshes && !meshes[obj.startMeshIndex + obj.numMeshes - 1])
   {
      meshes.erase(obj.startMeshIndex + obj.numMeshes - 1);
      obj.numMeshes--;
   }

   // Fixup mesh indices for other objects
   for (S32 i = 0; i < objects.size(); i++)
   {
      if (objects[i].startMeshIndex > obj.startMeshIndex)
         objects[i].startMeshIndex -= (oldNumMeshes - obj.numMeshes);
   }
}

bool TSShape::setObjectNode(const String& objName, const String& nodeName)
{
   // Find the object and node
   S32 objIndex = findObject(objName);
   if (objIndex < 0)
   {
      Con::errorf("TSShape::setObjectNode: Could not find object '%s'", objName.c_str());
      return false;
   }

   S32 nodeIndex;
   if (nodeName.isEmpty())
      nodeIndex = -1;
   else
   {
      nodeIndex = findNode(nodeName);
      if (nodeIndex < 0)
      {
         Con::errorf("TSShape::setObjectNode: Could not find node '%s'", nodeName.c_str());
         return false;
      }
   }

   objects[objIndex].nodeIndex = nodeIndex;

   return true;
}

bool TSShape::removeObject(const String& name)
{
   // Find the object
   S32 objIndex = findObject(name);
   if (objIndex < 0)
   {
      Con::errorf("TSShape::removeObject: Could not find object '%s'", name.c_str());
      return false;
   }

   // Destroy all meshes in the object
   TSShape::Object& obj = objects[objIndex];
   while ( obj.numMeshes )
   {
      destructInPlace(meshes[obj.startMeshIndex + obj.numMeshes - 1]);
      removeMeshFromObject(objIndex, obj.numMeshes - 1);
   }

   // Remove the object from the shape
   objects.erase(objIndex);
   S32 subShapeIndex = getSubShapeForObject(objIndex);
   subShapeNumObjects[subShapeIndex]--;
   for (S32 i = subShapeIndex + 1; i < subShapeFirstObject.size(); i++)
      subShapeFirstObject[i]--;

   // Remove the object from all sequences
   for (S32 i = 0; i < sequences.size(); i++)
   {
      TSShape::Sequence& seq = sequences[i];

      TSIntegerSet objMatters(seq.frameMatters);
      objMatters.overlap(seq.matFrameMatters);
      objMatters.overlap(seq.visMatters);

      if (objMatters.test(objIndex))
         eraseStates(objectStates, objMatters, seq.baseObjectState, seq.numKeyframes, objIndex);

      seq.frameMatters.erase(objIndex);
      seq.matFrameMatters.erase(objIndex);
      seq.visMatters.erase(objIndex);
   }

   // Remove the object name if it is no longer in use
   removeName(name);

   // Update smallest visible detail
   updateSmallestVisibleDL();

   // Re-initialise the shape
   init();

   return true;
}

//-----------------------------------------------------------------------------

// Helper to copy a TSMesh ready for adding to this TSShape (with the right vertex format etc)
TSMesh* TSShape::copyMesh( const TSMesh* srcMesh ) const
{
   TSMesh * mesh = 0;
   if ( srcMesh && ( srcMesh->getMeshType() == TSMesh::SkinMeshType ) )
   {
      TSSkinMesh* skin = new TSSkinMesh;

      // Copy skin elements
      const TSSkinMesh *srcSkin = dynamic_cast<const TSSkinMesh*>(srcMesh);
      skin->weight = srcSkin->weight;
      skin->vertexIndex = srcSkin->vertexIndex;
      skin->boneIndex = srcSkin->boneIndex;

      skin->batchData.nodeIndex = srcSkin->batchData.nodeIndex;
      skin->batchData.initialTransforms = srcSkin->batchData.initialTransforms;
      skin->batchData.initialVerts = srcSkin->batchData.initialVerts;
      skin->batchData.initialNorms = srcSkin->batchData.initialNorms;

      mesh = static_cast<TSMesh*>(skin);
   }
   else
   {
      mesh = new TSMesh;
   }

   // Set vertex format (all meshes in this shape must share the same format)
   mesh->mVertSize = mVertSize;
   mesh->mVertexFormat = &mVertexFormat;

   if ( !srcMesh )
      return mesh;      // return an empty mesh

   // Copy mesh elements
   mesh->indices = srcMesh->indices;
   mesh->primitives = srcMesh->primitives;
   mesh->numFrames = srcMesh->numFrames;
   mesh->numMatFrames = srcMesh->numMatFrames;
   mesh->vertsPerFrame = srcMesh->vertsPerFrame;
   mesh->setFlags(srcMesh->getFlags());
   mesh->mHasColor = srcMesh->mHasColor;
   mesh->mHasTVert2 = srcMesh->mHasTVert2;
   mesh->mNumVerts = srcMesh->mNumVerts;

   if ( srcMesh->mVertexData.isReady() )
   {
      mesh->mVertexData.set( NULL, 0, 0, false );
      void *aligned_mem = dMalloc_aligned( mVertSize * srcMesh->mVertexData.size(), 16 );

      // Copy the source data (note that the destination shape may have different vertex size)
      if ( mVertSize == srcMesh->mVertexData.size() )
      {
         dMemcpy( aligned_mem, srcMesh->mVertexData.address(), srcMesh->mVertexData.mem_size() );
      }
      else
      {
         U8* src = (U8*)srcMesh->mVertexData.address();
         U8* dest = (U8*)aligned_mem;
         for ( S32 i = 0; i < srcMesh->mVertexData.size(); i++ )
         {
            dMemcpy( dest, src, srcMesh->mVertexData.vertSize() );
            src += srcMesh->mVertexData.vertSize();
            dest += mVertSize;
         }
      }
      mesh->mVertexData.set( aligned_mem, mVertSize, srcMesh->mVertexData.size() );
      mesh->mVertexData.setReady( true );
   }
   else
   {
      mesh->verts = srcMesh->verts;
      mesh->tverts = srcMesh->tverts;
      mesh->tverts2 = srcMesh->tverts2;
      mesh->colors = srcMesh->colors;
      mesh->norms = srcMesh->norms;

      mesh->createTangents(mesh->verts, mesh->norms);
      mesh->encodedNorms.set(NULL,0);

      mesh->convertToAlignedMeshData();
   }

   mesh->computeBounds();

   if ( mesh->getMeshType() != TSMesh::SkinMeshType )
      mesh->createVBIB();

   return mesh;
}

bool TSShape::addMesh(TSMesh* mesh, const String& meshName)
{
   // Determine the object name and detail size from the mesh name
   S32 detailSize = 999;
   String objName(String::GetTrailingNumber(meshName, detailSize));

   // Find the destination object (create one if it does not exist)
   S32 objIndex = findObject(objName);
   if (objIndex < 0)
      objIndex = addObject(objName, 0);
   AssertFatal(objIndex >= 0 && objIndex < objects.size(), "Invalid object index!");

   // Determine the subshape this object belongs to
   S32 subShapeIndex = getSubShapeForObject(objIndex);
   AssertFatal(subShapeIndex < subShapeFirstObject.size(), "Could not find subshape for object!");

   // Get the existing detail levels for the subshape
   Vector<S32> validDetails;
   getSubShapeDetails(subShapeIndex, validDetails);

   // Determine where to add the new mesh, and whether this is a new detail
   S32 detIndex;
   bool newDetail = true;
   for (detIndex = 0; detIndex < validDetails.size(); detIndex++)
   {
      const TSShape::Detail& det = details[validDetails[detIndex]];
      if (detailSize >= det.size)
      {
         newDetail = (det.size != detailSize);
         break;
      }
   }

   // Insert the new detail level if required
   if (newDetail)
   {
      // Determine a name for the detail level
      const char* detailName;
      if (dStrStartsWith(objName, "Col"))
         detailName = "collision";
      else if (dStrStartsWith(objName, "loscol"))
         detailName = "los";
      else
         detailName = "detail";

      S32 index = addDetail(detailName, detailSize, subShapeIndex);
      details[index].objectDetailNum = detIndex;
   }

   // Adding a new mesh or detail level is a bit tricky, since each
   // object potentially stores a different number of meshes, including
   // NULL meshes for higher detail levels where required.
   // For example, the following table shows 3 objects. Note how NULLs
   // must be inserted for detail levels higher than the first valid
   // mesh, but details after the the last valid mesh are left empty.
   //
   // Detail   |  Object1  |  Object2  |  Object3
   // ---------+-----------+-----------+---------
   // 128      |  128      |  NULL     |  NULL
   // 64       |           |  NULL     |  64
   // 32       |           |  32       |  NULL
   // 2        |           |           |  2

   // Add meshes as required for each object
   for (S32 i = 0; i < subShapeNumObjects[subShapeIndex]; i++)
   {
      S32 index = subShapeFirstObject[subShapeIndex] + i;
      const TSShape::Object& obj = objects[index];

      if (index == objIndex)
      {
         // The target object: replace the existing mesh (if any) or add a new one
         // if required.
         if (!newDetail && (detIndex < obj.numMeshes))
         {
            if ( meshes[obj.startMeshIndex + detIndex] )
               destructInPlace(meshes[obj.startMeshIndex + detIndex]);
            meshes[obj.startMeshIndex + detIndex] = mesh;
         }
         else
            addMeshToObject(index, detIndex, mesh);
      }
      else
      {
         // Other objects: add a NULL mesh only if inserting before a valid mesh
         if (newDetail && (detIndex < obj.numMeshes))
            addMeshToObject(index, detIndex, NULL);
      }
   }

   // Re-initialise the shape
   init();

   return true;
}

bool TSShape::addMesh(TSShape* srcShape, const String& srcMeshName, const String& meshName)
{
   // Find the mesh in the source shape
   TSMesh* srcMesh = srcShape->findMesh(srcMeshName);
   if (!srcMesh)
   {
      Con::errorf("TSShape::addMesh: Could not find mesh '%s' in shape", srcMeshName.c_str());
      return false;
   }

   // Copy the source mesh
   TSMesh *mesh = copyMesh( srcMesh );
   if (srcMesh->getMeshType() == TSMesh::SkinMeshType)
   {
      TSSkinMesh *srcSkin = dynamic_cast<TSSkinMesh*>(srcMesh);

      // Check that the source skin is compatible with our skeleton
      Vector<S32> nodeMap(srcShape->nodes.size());
      for (S32 i = 0; i < srcShape->nodes.size(); i++)
         nodeMap.push_back( findNode( srcShape->getName(srcShape->nodes[i].nameIndex) ) );

      for (S32 i = 0; i < srcSkin->boneIndex.size(); i++)
      {
         S32 srcNode = srcSkin->boneIndex[i];
         if (nodeMap[srcNode] == -1)
         {
            const char* name = srcShape->getName(srcShape->nodes[srcNode].nameIndex).c_str();
            Con::errorf("TSShape::addMesh: Skin is weighted to node (%s) that "
               "does not exist in this shape", name);
            return false;
         }
      }

      TSSkinMesh *skin = dynamic_cast<TSSkinMesh*>(mesh);

      // Remap node indices
      skin->batchData.nodeIndex = srcSkin->batchData.nodeIndex;
      for (S32 i = 0; i < skin->batchData.nodeIndex.size(); i++)
         skin->batchData.nodeIndex[i] = nodeMap[skin->batchData.nodeIndex[i]];
   }

   // Add the copied mesh to the shape
   if (!addMesh(mesh, meshName))
   {
      delete mesh;
      return false;
   }

   // Copy materials used by the source mesh (only if from a different shape)
   if (srcShape != this)
   {
      for (S32 i = 0; i < mesh->primitives.size(); i++)
      {
         if (!(mesh->primitives[i].matIndex & TSDrawPrimitive::NoMaterial))
         {
            S32 drawType = (mesh->primitives[i].matIndex & (~TSDrawPrimitive::MaterialMask));
            S32 srcMatIndex = mesh->primitives[i].matIndex & TSDrawPrimitive::MaterialMask;
            const String& matName = srcShape->materialList->getMaterialName(srcMatIndex);

            // Add the material if it does not already exist
            S32 destMatIndex = materialList->getMaterialNameList().find_next(matName);
            if (destMatIndex < 0)
            {
               destMatIndex = materialList->size();
               materialList->push_back(matName, srcShape->materialList->getFlags(srcMatIndex));
            }

            mesh->primitives[i].matIndex = drawType | destMatIndex;
         }
      }
   }

   return true;
}

bool TSShape::setMeshSize(const String& meshName, S32 size)
{
   S32 objIndex, meshIndex;
   if (!findMeshIndex(meshName, objIndex, meshIndex) ||
      !meshes[objects[objIndex].startMeshIndex + meshIndex])
   {
      Con::errorf("TSShape::setMeshSize: Could not find mesh '%s'", meshName.c_str());
      return false;
   }

   // Remove the mesh from the object, but don't destroy it
   TSShape::Object& obj = objects[objIndex];
   TSMesh* mesh = meshes[obj.startMeshIndex + meshIndex];
   removeMeshFromObject(objIndex, meshIndex);

   // Add the mesh back at the new position
   addMesh(mesh, avar("%s %d", getName(obj.nameIndex).c_str(), size));

   // Update smallest visible detail
   updateSmallestVisibleDL();

   // Re-initialise the shape
   init();

   return true;
}

bool TSShape::removeMesh(const String& meshName)
{
   S32 objIndex, meshIndex;
   if (!findMeshIndex(meshName, objIndex, meshIndex) ||
      !meshes[objects[objIndex].startMeshIndex + meshIndex])
   {
      Con::errorf("TSShape::removeMesh: Could not find mesh '%s'", meshName.c_str());
      return false;
   }

   // Destroy and remove the mesh
   TSShape::Object& obj = objects[objIndex];
   destructInPlace(meshes[obj.startMeshIndex + meshIndex]);
   removeMeshFromObject(objIndex, meshIndex);

   // Remove the object if there are no meshes left
   if (!obj.numMeshes)
      removeObject(getName(obj.nameIndex));

   // Update smallest visible detail
   updateSmallestVisibleDL();

   // Re-initialise the shape
   init();

   return true;
}

//-----------------------------------------------------------------------------

// Helper function for dealing with some of the Vectors used in a TSShape. 'meshes'
// for example contains a TSMesh* per-object, per-detail-level, with NULLs for
// undefined details for each object. Trailing NULLs are not added to the Vector,
// so you end up with a different number of pointers for each object, depending
// on which detail levels it defines. This makes it tricky to move meshes around
// since you have to know how many elements belong to each object.
// To simplify things, this function pads the Vector up to a certain length (so
// all objects can appear to have the same number of meshes), the moves a single
// element to a new index, then trims trailing NULLs again.
template<class T>
static void _PadMoveAndTrim(Vector<T*>& vec, S32 offset, S32 count,
                              S32 padLength, S32 oldIndex, S32 newIndex)
{
   // Pad the array with NULLs
   for ( S32 i = count; i < padLength; ++i )
      vec.insert( offset + count, NULL );

   // Move the element from the old to the new index
   T* tmp = vec[offset + oldIndex];
   vec.erase( offset + oldIndex );
   vec.insert( offset + newIndex, tmp );

   // Trim trailing NULLs from the vector
   for ( S32 i = padLength - 1; i >= 0; --i )
   {
      if ( vec[offset + i] )
         break;
      else
         vec.erase( offset + i );
   }
}

S32 TSShape::setDetailSize(S32 oldSize, S32 newSize)
{
   S32 oldIndex = findDetailBySize( oldSize );
   if ( oldIndex < 0 )
   {
      Con::errorf( "TSShape::setDetailSize: Cannot find detail with size %d", oldSize );
      return -1;
   }

   // Remove this detail from the list
   TSShape::Detail tmpDetail = details[oldIndex];
   tmpDetail.size = newSize;
   details.erase(oldIndex);

   // Determine the new position for the detail (details are sorted by size)
   S32 newIndex = 0;
   for ( newIndex = 0; newIndex < details.size(); ++newIndex )
   {
      if ( newSize > details[newIndex].size )
         break;
   }

   // Add the detail at its new position
   details.insert( newIndex, tmpDetail );

   // Rename the detail so its trailing size value is correct
   {
      S32 tmp;
      String oldName( getName( tmpDetail.nameIndex ) );
      String newName( String::GetTrailingNumber( oldName, tmp ) );
      newName += String::ToString( "%d", newSize );
      renameDetail(oldName, newName);
   }

   if ( newIndex != oldIndex )
   {
      // Fixup details
      for ( S32 iDet = 0; iDet < details.size(); iDet++ )
      {
         if ( details[iDet].subShapeNum < 0 )
         {
            if ( details[iDet].bbDetailLevel == oldIndex )
               details[iDet].bbDetailLevel = newIndex;
         }
         else
         {
            details[iDet].objectDetailNum = iDet;
         }
      }

      // Fixup Billboard details
      _PadMoveAndTrim( billboardDetails, 0, billboardDetails.size(),
                        details.size(), oldIndex, newIndex );

      // Now move the mesh for each object in the subshape (adding and removing
      // NULLs as appropriate)
      for ( S32 iObj = 0; iObj < objects.size(); ++iObj )
      {
         TSShape::Object& obj = objects[iObj];
         S32 oldMeshCount = meshes.size();

         _PadMoveAndTrim( meshes, obj.startMeshIndex, obj.numMeshes,
                           details.size(), oldIndex, newIndex );

         obj.numMeshes += ( meshes.size() - oldMeshCount );

         // Fixup startMeshIndex for remaining objects
         for ( S32 j = iObj + 1; j < objects.size(); ++j )
            objects[j].startMeshIndex += ( meshes.size() - oldMeshCount );
      }
   }

   // Update smallest visible detail
   updateSmallestVisibleDL();

   // Re-initialise the shape
   init();

   return newIndex;
}

bool TSShape::removeDetail( S32 size )
{
   S32 dl = findDetailBySize( size );

   if ( ( dl < 0 ) || ( dl >= details.size() ) )
   {
      Con::errorf( "TSShape::removeDetail: Invalid detail index (%d)", dl );
      return false;
   }

   // Destroy and remove each mesh in the detail level
   for ( S32 objIndex = objects.size()-1; objIndex >= 0; objIndex-- )
   {
      TSShape::Object& obj = objects[objIndex];
      if ( dl < obj.numMeshes )
      {
         if ( meshes[obj.startMeshIndex + dl] )
            destructInPlace( meshes[obj.startMeshIndex + dl] );
         removeMeshFromObject(objIndex, dl);

         // Remove the object if there are no meshes left
         if (!obj.numMeshes)
            removeObject( getName( obj.nameIndex ) );
      }
   }

   // Destroy billboard detail level
   if ( dl < billboardDetails.size() )
   {
      if ( billboardDetails[dl] )
      {
         // Delete old textures
         billboardDetails[dl]->deleteImposterCacheTextures();
         delete billboardDetails[dl];
     }
     
      billboardDetails.erase( dl );
   }

   // Update smallest visible detail
   updateSmallestVisibleDL();

   // Re-initialise the shape
   init();

   return true;
}

//-----------------------------------------------------------------------------
bool TSShape::addSequence(const Torque::Path& path, const String& fromSeq,
                          const String& name, S32 startFrame, S32 endFrame,
                          bool padRotKeys, bool padTransKeys)
{
   String oldName(fromSeq);

   if (path.getExtension().equal("dsq", String::NoCase))
   {
      S32 oldSeqCount = sequences.size();

      // DSQ source file
      char filenameBuf[1024];
      Con::expandScriptFilename(filenameBuf, sizeof(filenameBuf), path.getFullPath().c_str());

      FileStream *f;
      if((f = FileStream::createAndOpen( filenameBuf, Torque::FS::File::Read )) == NULL)
      {
         Con::errorf("TSShape::addSequence: Could not load DSQ file '%s'", filenameBuf);
         return false;
      }
      if (!importSequences(f, filenameBuf) || (f->getStatus() != Stream::Ok))
      {
         delete f;
         Con::errorf("TSShape::addSequence: Load sequence file '%s' failed", filenameBuf);
         return false;
      }
      delete f;

      // Rename the new sequence if required (avoid rename if name is not
      // unique (this will be fixed up later, and we don't need 2 errors about it!)
      if (oldName.isEmpty())
         oldName = getName(sequences.last().nameIndex);
      if (!oldName.equal(name))
      {
         if (findSequence(name) == -1)
         {
            // Use a dummy intermediate name since we might be renaming from an
            // existing name (and we want to rename the right sequence!)
            sequences.last().nameIndex = addName("__dummy__");
            renameSequence("__dummy__", name);
         }
      }

      // Check that sequences have unique names
      bool lastSequenceRejected = false;
      for (S32 i = sequences.size()-1; i >= oldSeqCount; i--)
      {
         S32 nameIndex = (i == sequences.size()-1) ? findName(name) : sequences[i].nameIndex;
         S32 seqIndex = findSequence(nameIndex);
         if ((seqIndex != -1) && (seqIndex != i))
         {
            Con::errorf("TSShape::addSequence: Failed to add sequence '%s' "
               "(name already exists)", getName(nameIndex).c_str());
            sequences[i].nameIndex = addName("__dummy__");
            removeSequence("__dummy__");
            if (i == sequences.size())
               lastSequenceRejected = true;
         }
      }

      // @todo:Need to remove keyframes if start!=0 and end!=-1
      TSShape::Sequence& seq = sequences.last();

      // Store information about how this sequence was created
      seq.sourceData.from = String::ToString("%s\t%s", filenameBuf, name.c_str());
      seq.sourceData.total = seq.numKeyframes;
      seq.sourceData.start = ((startFrame < 0) || (startFrame >= seq.numKeyframes)) ? 0 : startFrame;
      seq.sourceData.end = ((endFrame < 0) || (endFrame >= seq.numKeyframes)) ? seq.numKeyframes-1 : endFrame;

      return (sequences.size() != oldSeqCount);
   }

   /* Check that sequence to be added does not already exist */
   if (findSequence(name) != -1)
   {
      Con::errorf("TSShape::addSequence: Cannot add sequence '%s' (name already exists)", name.c_str());
      return false;
   }

   Resource<TSShape> hSrcShape;
   TSShape* srcShape = this;        // Assume we are copying an existing sequence

   if (path.getExtension().equal("dts", String::NoCase) ||
       path.getExtension().equal("dae", String::NoCase))
   {
      // DTS or DAE source file
      char filenameBuf[1024];
      Con::expandScriptFilename(filenameBuf, sizeof(filenameBuf), path.getFullPath().c_str());

      hSrcShape = ResourceManager::get().load(filenameBuf);
      if (!bool(hSrcShape))
      {
         Con::errorf("TSShape::addSequence: Could not load source shape '%s'", path.getFullPath().c_str());
         return false;
      }
      srcShape = const_cast<TSShape*>((const TSShape*)hSrcShape);
      if (!srcShape->sequences.size())
      {
         Con::errorf("TSShape::addSequence: Source shape '%s' does not contain any sequences", path.getFullPath().c_str());
         return false;
      }

      // If no sequence name is specified, just use the first one
      if (oldName.isEmpty())
         oldName = srcShape->getName(srcShape->sequences[0].nameIndex);
   }
   else
   {
      // Source is an existing sequence
      oldName = path.getFullPath();
   }

   // Find the sequence
   S32 seqIndex = srcShape->findSequence(oldName);
   if (seqIndex < 0)
   {
      Con::errorf("TSShape::addSequence: Could not find sequence named '%s'", oldName.c_str());
      return false;
   }

   // Check keyframe range
   const TSShape::Sequence* srcSeq = &srcShape->sequences[seqIndex];
   if ((startFrame < 0) || (startFrame >= srcSeq->numKeyframes))
   {
      Con::warnf("TSShape::addSequence: Start keyframe (%d) out of range (0-%d) for sequence '%s'",
         startFrame, srcSeq->numKeyframes-1, oldName.c_str());
      startFrame = 0;
   }
   if (endFrame < 0)
      endFrame = srcSeq->numKeyframes - 1;
   else if (endFrame >= srcSeq->numKeyframes)
   {
      Con::warnf("TSShape::addSequence: End keyframe (%d) out of range (0-%d) for sequence '%s'",
         endFrame, srcSeq->numKeyframes-1, oldName.c_str());
      endFrame = srcSeq->numKeyframes - 1;
   }

   // Create array to map source nodes to our nodes
   Vector<S32> nodeMap(srcShape->nodes.size());
   for (S32 i = 0; i < srcShape->nodes.size(); i++)
      nodeMap.push_back(findNode(srcShape->getName(srcShape->nodes[i].nameIndex)));

   // Create array to map source objects to our objects
   Vector<S32> objectMap(srcShape->objects.size());
   for (S32 i = 0; i < srcShape->objects.size(); i++)
      objectMap.push_back(findObject(srcShape->getName(srcShape->objects[i].nameIndex)));

   // Copy the source sequence (need to do it this ugly way instead of just
   // using push_back since srcSeq pointer may change if copying a sequence
   // from inside the shape itself
   sequences.increment();
   TSShape::Sequence& seq = sequences.last();
   srcSeq = &srcShape->sequences[seqIndex]; // update pointer as it may have changed!
   seq = *srcSeq;

   seq.nameIndex = addName(name);
   seq.numKeyframes = endFrame - startFrame + 1;
   if (seq.duration > 0)
      seq.duration *= ((F32)seq.numKeyframes / srcSeq->numKeyframes);

   // Add object states
   // Note: only visibility animation is supported
   seq.frameMatters.clearAll();
   seq.matFrameMatters.clearAll();
   seq.visMatters.clearAll();
   for (S32 i = 0; i < objectMap.size(); i++)
   {
      if (objectMap[i] < 0)
         continue;

      if (srcSeq->visMatters.test(i))
      {
         // Check if visibility is animated within the frames to be copied
         const F32 defaultVis = srcShape->objectStates[i].vis;
         S32 objNum = srcSeq->visMatters.count(i);
         for (int iFrame = startFrame; iFrame <= endFrame; iFrame++)
         {
            if (srcShape->getObjectState(*srcSeq, iFrame, objNum).vis != defaultVis)
            {
               seq.visMatters.set(objectMap[i]);
               break;
            }
         }
      }
   }

   TSIntegerSet srcObjectStateSet(srcSeq->frameMatters);
   srcObjectStateSet.overlap(srcSeq->matFrameMatters);
   srcObjectStateSet.overlap(srcSeq->visMatters);

   TSIntegerSet objectStateSet(seq.frameMatters);
   objectStateSet.overlap(seq.matFrameMatters);
   objectStateSet.overlap(seq.visMatters);

   seq.baseObjectState = objectStates.size();
   objectStates.increment(objectStateSet.count()*seq.numKeyframes);
   for (S32 i = 0; i < objectMap.size(); i++)
   {
      if (objectMap[i] < 0)
         continue;

      // Note: only visibility animation is supported
      if (objectStateSet.test(objectMap[i]))
      {
         S32 src = srcSeq->baseObjectState + srcSeq->numKeyframes * srcObjectStateSet.count(i) + startFrame;
         S32 dest = seq.baseObjectState + seq.numKeyframes * objectStateSet.count(objectMap[i]);
         dCopyArray(&objectStates[dest], &srcShape->objectStates[src], seq.numKeyframes);
      }
   }

   // Add ground frames
   F32 ratio = (F32)seq.numKeyframes / srcSeq->numKeyframes;
   S32 groundBase = srcSeq->firstGroundFrame + startFrame*ratio;

   seq.numGroundFrames *= ratio;
   seq.firstGroundFrame = groundTranslations.size();
   groundTranslations.reserve(groundTranslations.size() + seq.numGroundFrames);
   groundRotations.reserve(groundRotations.size() + seq.numGroundFrames);
   for (S32 i = 0; i < seq.numGroundFrames; i++)
   {
      groundTranslations.push_back(srcShape->groundTranslations[groundBase + i]);
      groundRotations.push_back(srcShape->groundRotations[groundBase + i]);
   }

   // Add triggers
   seq.numTriggers = 0;
   seq.firstTrigger = triggers.size();
   F32 seqStartPos = (F32)startFrame / seq.numKeyframes;
   F32 seqEndPos = (F32)endFrame / seq.numKeyframes;
   for (S32 i = 0; i < srcSeq->numTriggers; i++)
   {
      const TSShape::Trigger& srcTrig = srcShape->triggers[srcSeq->firstTrigger + i];
      if ((srcTrig.pos >= seqStartPos) && (srcTrig.pos <= seqEndPos))
      {
         triggers.push_back(srcTrig);
         triggers.last().pos -= seqStartPos;
         seq.numTriggers++;
      }
   }

   // Fixup node matters arrays
   seq.translationMatters.clearAll();
   seq.rotationMatters.clearAll();
   seq.scaleMatters.clearAll();
   for (S32 i = 0; i < nodeMap.size(); i++)
   {
      if (nodeMap[i] < 0)
         continue;

      if (srcSeq->translationMatters.test(i))
      {
         // Check if node position is animated within the frames to be copied
         const Point3F& defaultTrans = srcShape->defaultTranslations[i];
         S32 tranNum = srcSeq->translationMatters.count(i);
         for (int iFrame = startFrame; iFrame <= endFrame; iFrame++)
         {
            if (srcShape->getTranslation(*srcSeq, iFrame, tranNum) != defaultTrans)
            {
               seq.translationMatters.set(nodeMap[i]);
               break;
            }
         }
      }

      if (srcSeq->rotationMatters.test(i))
      {
         // Check if node rotation is animated within the frames to be copied
         const QuatF defaultRot = srcShape->defaultRotations[i].getQuatF();
         S32 rotNum = srcSeq->rotationMatters.count(i);
         for (int iFrame = startFrame; iFrame <= endFrame; iFrame++)
         {
            QuatF temp;
            if (srcShape->getRotation(*srcSeq, iFrame, rotNum, &temp) != defaultRot)
            {
               seq.rotationMatters.set(nodeMap[i]);
               break;
            }
         }
      }
      if (srcSeq->scaleMatters.test(i))
      {
         S32 scaleNum = srcSeq->scaleMatters.count(i);

         // Check if node scale is animated within the frames to be copied
         if (srcSeq->animatesArbitraryScale())
         {
            TSScale defaultScale;
            defaultScale.identity();
            for (int iFrame = startFrame; iFrame <= endFrame; iFrame++)
            {
               TSScale temp;
               if (!(srcShape->getArbitraryScale(*srcSeq, iFrame, scaleNum, &temp) == defaultScale))
               {
                  seq.scaleMatters.set(nodeMap[i]);
                  break;
               }
            }
         }
         else if (srcSeq->animatesAlignedScale())
         {
            const Point3F defaultScale(Point3F::One);
            for (int iFrame = startFrame; iFrame <= endFrame; iFrame++)
            {
               if (srcShape->getAlignedScale(*srcSeq, iFrame, scaleNum) != defaultScale)
               {
                  seq.scaleMatters.set(nodeMap[i]);
                  break;
               }
            }
         }
         else if (srcSeq->animatesUniformScale())
         {
            const F32 defaultScale = 1.0f;
            for (int iFrame = startFrame; iFrame <= endFrame; iFrame++)
            {
               if (srcShape->getUniformScale(*srcSeq, iFrame, scaleNum) != defaultScale)
               {
                  seq.scaleMatters.set(nodeMap[i]);
                  break;
               }
            }
         }
      }
   }

   // Resize the node transform arrays
   seq.baseTranslation = nodeTranslations.size();
   nodeTranslations.increment(seq.translationMatters.count()*seq.numKeyframes);
   seq.baseRotation = nodeRotations.size();
   nodeRotations.increment(seq.rotationMatters.count()*seq.numKeyframes);
   if (seq.flags & TSShape::ArbitraryScale)
   {
      S32 scaleCount = seq.scaleMatters.count();
      seq.baseScale = nodeArbitraryScaleRots.size();
      nodeArbitraryScaleRots.increment(scaleCount*seq.numKeyframes);
      nodeArbitraryScaleFactors.increment(scaleCount*seq.numKeyframes);
   }
   else if (seq.flags & TSShape::AlignedScale)
   {
      seq.baseScale = nodeAlignedScales.size();
      nodeAlignedScales.increment(seq.scaleMatters.count()*seq.numKeyframes);
   }
   else
   {
      seq.baseScale = nodeUniformScales.size();
      nodeUniformScales.increment(seq.scaleMatters.count()*seq.numKeyframes);
   }

   // Add node transforms (remap from source node indices to our node indices). As
   // well as copying animated node translations and rotations, also handle when the
   // default translation and rotation are different between the source and
   // destination shapes.
   for (S32 i = 0; i < nodeMap.size(); i++)
   {
      if (nodeMap[i] < 0)
         continue;

      if (seq.translationMatters.test(nodeMap[i]))
      {
         S32 src = srcSeq->baseTranslation + srcSeq->numKeyframes * srcSeq->translationMatters.count(i) + startFrame;
         S32 dest = seq.baseTranslation + seq.numKeyframes * seq.translationMatters.count(nodeMap[i]);
         dCopyArray(&nodeTranslations[dest], &srcShape->nodeTranslations[src], seq.numKeyframes);
      }
      else if (padTransKeys && (defaultTranslations[nodeMap[i]] != srcShape->defaultTranslations[i]))
      {
         seq.translationMatters.set(nodeMap[i]);
         S32 dest = seq.baseTranslation + seq.numKeyframes * seq.translationMatters.count(nodeMap[i]);
         for (S32 j = 0; j < seq.numKeyframes; j++)
            nodeTranslations.insert(dest, srcShape->defaultTranslations[i]);
      }

      if (seq.rotationMatters.test(nodeMap[i]))
      {
         S32 src = srcSeq->baseRotation + srcSeq->numKeyframes * srcSeq->rotationMatters.count(i) + startFrame;
         S32 dest = seq.baseRotation + seq.numKeyframes * seq.rotationMatters.count(nodeMap[i]);
         dCopyArray(&nodeRotations[dest], &srcShape->nodeRotations[src], seq.numKeyframes);
      }
      else if (padRotKeys && (defaultRotations[nodeMap[i]] != srcShape->defaultRotations[i]))
      {
         seq.rotationMatters.set(nodeMap[i]);
         S32 dest = seq.baseRotation + seq.numKeyframes * seq.rotationMatters.count(nodeMap[i]);
         for (S32 j = 0; j < seq.numKeyframes; j++)
            nodeRotations.insert(dest, srcShape->defaultRotations[i]);
      }

      if (seq.scaleMatters.test(nodeMap[i]))
      {
         S32 src = srcSeq->baseScale + srcSeq->numKeyframes * srcSeq->scaleMatters.count(i)+ startFrame;
         S32 dest = seq.baseScale + seq.numKeyframes * seq.scaleMatters.count(nodeMap[i]);
         if (seq.flags & TSShape::ArbitraryScale)
         {
            dCopyArray(&nodeArbitraryScaleRots[dest], &srcShape->nodeArbitraryScaleRots[src], seq.numKeyframes);
            dCopyArray(&nodeArbitraryScaleFactors[dest], &srcShape->nodeArbitraryScaleFactors[src], seq.numKeyframes);
         }
         else if (seq.flags & TSShape::AlignedScale)
            dCopyArray(&nodeAlignedScales[dest], &srcShape->nodeAlignedScales[src], seq.numKeyframes);
         else
            dCopyArray(&nodeUniformScales[dest], &srcShape->nodeUniformScales[src], seq.numKeyframes);
      }
   }

   // Set shape flags (only the most significant scale type)
   U32 curVal = mFlags & AnyScale;
   mFlags &= ~(AnyScale);
   mFlags |= getMax(curVal, seq.flags & AnyScale);    // take the larger value (can only convert upwards)

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

   // Store information about how this sequence was created
   seq.sourceData.from = String::ToString("%s\t%s", path.getFullPath().c_str(), oldName.c_str());
   seq.sourceData.total = srcSeq->numKeyframes;
   seq.sourceData.start = startFrame;
   seq.sourceData.end = endFrame;

   return true;
}

bool TSShape::removeSequence(const String& name)
{
   // Find the sequence to be removed
   S32 seqIndex = findSequence(name);
   if (seqIndex < 0)
   {
      Con::errorf("TSShape::removeSequence: Could not find sequence '%s'", name.c_str());
      return false;
   }

   TSShape::Sequence& seq = sequences[seqIndex];

   // Remove the node transforms for this sequence
   S32 transCount = eraseStates(nodeTranslations, seq.translationMatters, seq.baseTranslation, seq.numKeyframes);
   S32 rotCount = eraseStates(nodeRotations, seq.rotationMatters, seq.baseRotation, seq.numKeyframes);
   S32 scaleCount = 0;
   if (seq.flags & TSShape::ArbitraryScale)
   {
      scaleCount = eraseStates(nodeArbitraryScaleRots, seq.scaleMatters, seq.baseScale, seq.numKeyframes);
      eraseStates(nodeArbitraryScaleFactors, seq.scaleMatters, seq.baseScale, seq.numKeyframes);
   }
   else if (seq.flags & TSShape::AlignedScale)
      scaleCount = eraseStates(nodeAlignedScales, seq.scaleMatters, seq.baseScale, seq.numKeyframes);
   else
      scaleCount = eraseStates(nodeUniformScales, seq.scaleMatters, seq.baseScale, seq.numKeyframes);

   // Remove the object states for this sequence
   TSIntegerSet objMatters(seq.frameMatters);
   objMatters.overlap(seq.matFrameMatters);
   objMatters.overlap(seq.visMatters);
   S32 objCount = eraseStates(objectStates, objMatters, seq.baseObjectState, seq.numKeyframes);

   // Remove groundframes and triggers
   TSIntegerSet dummy;
   eraseStates(groundTranslations, dummy, seq.firstGroundFrame, seq.numGroundFrames, 0);
   eraseStates(groundRotations, dummy, seq.firstGroundFrame, seq.numGroundFrames, 0);
   eraseStates(triggers, dummy, seq.firstTrigger, seq.numTriggers, 0);

   // Fixup the base indices of the other sequences
   for (S32 i = seqIndex + 1; i < sequences.size(); i++)
   {
      sequences[i].baseTranslation -= transCount;
      sequences[i].baseRotation -= rotCount;
      sequences[i].baseScale -= scaleCount;
      sequences[i].baseObjectState -= objCount;
      sequences[i].firstGroundFrame -= seq.numGroundFrames;
      sequences[i].firstTrigger -= seq.numTriggers;
   }

   // Remove the sequence itself
   sequences.erase(seqIndex);

   // Remove the sequence name if it is no longer in use
   removeName(name);

   return true;
}

//-----------------------------------------------------------------------------

bool TSShape::addTrigger(const String& seqName, S32 keyframe, S32 state)
{
   // Find the sequence
   S32 seqIndex = findSequence(seqName);
   if (seqIndex < 0)
   {
      Con::errorf("TSShape::addTrigger: Could not find sequence '%s'", seqName.c_str());
      return false;
   }

   TSShape::Sequence& seq = sequences[seqIndex];
   if (keyframe >= seq.numKeyframes)
   {
      Con::errorf("TSShape::addTrigger: Keyframe out of range (0-%d for sequence '%s')",
         seq.numKeyframes-1, seqName.c_str());
      return false;
   }

   // Encode the trigger state
   if (state < 0)
      state = 1 << (-state-1);
   else if (state > 0)
      state = (1 << (state-1)) | TSShape::Trigger::StateOn;

   // Fixup seq.firstTrigger if this sequence does not have any triggers yet
   if (seq.numTriggers == 0)
   {
      seq.firstTrigger = 0;
      for (S32 i = 0; i < seqIndex; i++)
         seq.firstTrigger += sequences[i].numTriggers;
   }

   // Find where to insert the trigger (sorted by keyframe)
   S32 trigIndex;
   for (trigIndex = seq.firstTrigger; trigIndex < (seq.firstTrigger + seq.numTriggers); trigIndex++)
   {
      const TSShape::Trigger& trig = triggers[trigIndex];
      if ((S32)(trig.pos * seq.numKeyframes) > keyframe)
         break;
   }

   // Create the new trigger
   TSShape::Trigger trig;
   trig.pos = (F32)keyframe / getMax(1, seq.numKeyframes-1);
   trig.state = state;
   triggers.insert(trigIndex, trig);
   seq.numTriggers++;

   // set invert for other triggers if needed
   if ((trig.state & TSShape::Trigger::StateOn) == 0)
   {
      U32 offTrigger = (trig.state & TSShape::Trigger::StateMask);
      for (S32 i = 0; i < seq.numTriggers; i++)
      {
         if (triggers[seq.firstTrigger + i].state & offTrigger)
            triggers[seq.firstTrigger + i].state |= TSShape::Trigger::InvertOnReverse;
      }
   }

   // fixup firstTrigger index for other sequences
   for (S32 i = seqIndex + 1; i < sequences.size(); i++)
   {
      if (sequences[i].numTriggers > 0)
         sequences[i].firstTrigger++;
   }

   // set MakePath flag so triggers will be animated
   seq.flags |= TSShape::MakePath;

   return true;
}

bool TSShape::removeTrigger(const String& seqName, S32 keyframe, S32 state)
{
   // Find the sequence
   S32 seqIndex = findSequence(seqName);
   if (seqIndex < 0)
   {
      Con::errorf("TSShape::removeTrigger: Could not find sequence '%s'", seqName.c_str());
      return false;
   }

   TSShape::Sequence& seq = sequences[seqIndex];
   if (keyframe >= seq.numKeyframes)
   {
      Con::errorf("TSShape::removeTrigger: Keyframe out of range (0-%d for sequence '%s')",
         seq.numKeyframes-1, seqName.c_str());
      return false;
   }

   // Encode the trigger state
   if (state < 0)
      state = 1 << (-state-1);
   else if (state > 0)
      state = (1 << (state-1)) | TSShape::Trigger::StateOn;

   // Find and remove the trigger
   for (S32 trigIndex = seq.firstTrigger; trigIndex < (seq.firstTrigger + seq.numTriggers); trigIndex++)
   {
      TSShape::Trigger& trig = triggers[trigIndex];
      S32 cmpFrame = (S32)(trig.pos * (seq.numKeyframes-1) + 0.5f); 
      S32 cmpState = trig.state & (~TSShape::Trigger::InvertOnReverse);

      if ((cmpFrame == keyframe) && (cmpState == state))
      {
         triggers.erase(trigIndex);
         seq.numTriggers--;

         // Fix up firstTrigger for other sequences
         for (S32 i = seqIndex + 1; i < sequences.size(); i++)
         {
            if (sequences[i].numTriggers > 0)
               sequences[i].firstTrigger--;
         }

         // Clear MakePath flag if no more triggers
         if ( seq.numTriggers == 0 )
            seq.flags &= (~TSShape::MakePath);

         return true;
      }
   }

   Con::errorf("TSShape::removeTrigger: Could not find trigger (%d, %d) for sequence '%s'",
      keyframe, state, seqName.c_str());

   return false;
}

void TSShape::getNodeKeyframe(S32 nodeIndex, const TSShape::Sequence& seq, S32 keyframe, MatrixF* mat) const
{
   // Get the node rotation and translation
   QuatF rot;
   if (seq.rotationMatters.test(nodeIndex))
   {
      S32 index = seq.rotationMatters.count(nodeIndex) * seq.numKeyframes + keyframe;
      nodeRotations[seq.baseRotation + index].getQuatF(&rot);
   }   
   else
      defaultRotations[nodeIndex].getQuatF(&rot);

   Point3F trans;
   if (seq.translationMatters.test(nodeIndex))
   {
      S32 index = seq.translationMatters.count(nodeIndex) * seq.numKeyframes + keyframe;
      trans = nodeTranslations[seq.baseTranslation + index];
   }
   else
      trans = defaultTranslations[nodeIndex];

   // Set the keyframe matrix
   rot.setMatrix(mat);
   mat->setPosition(trans);
}

bool TSShape::setSequenceBlend(const String& seqName, bool blend, const String& blendRefSeqName, S32 blendRefFrame)
{
   // Find the target sequence
   S32 seqIndex = findSequence(seqName);
   if (seqIndex < 0)
   {
      Con::errorf("TSShape::setSequenceBlend: Could not find sequence named '%s'", seqName.c_str());
      return false;
   }
   TSShape::Sequence& seq = sequences[seqIndex];

   // Ignore if blend flag is already correct
   if (seq.isBlend() == blend)
      return true;

   // Find the sequence containing the reference frame
   S32 blendRefSeqIndex = findSequence(blendRefSeqName);
   if (blendRefSeqIndex < 0)
   {
      Con::errorf("TSShape::setSequenceBlend: Could not find reference sequence named '%s'", blendRefSeqName.c_str());
      return false;
   }
   TSShape::Sequence& blendRefSeq = sequences[blendRefSeqIndex];

   if ((blendRefFrame < 0) || (blendRefFrame >= blendRefSeq.numKeyframes))
   {
      Con::errorf("TSShape::setSequenceBlend: Reference frame out of range (0-%d)", blendRefSeq.numKeyframes-1);
      return false;
   }

   // Set the new flag
   if (blend)
      seq.flags |= TSShape::Blend;
   else
      seq.flags &= (~TSShape::Blend);

   // For each animated node in the target sequence, need to add or subtract the
   // reference keyframe from each frame
   TSIntegerSet nodeMatters(seq.rotationMatters);
   nodeMatters.overlap(seq.translationMatters);

   S32 end = nodeMatters.end();
   for (S32 nodeIndex = nodeMatters.start(); nodeIndex < end; nodeMatters.next(nodeIndex))
   {
      MatrixF refMat;
      getNodeKeyframe(nodeIndex, blendRefSeq, blendRefFrame, &refMat);

      // Add or subtract the reference?
      if (blend)
         refMat.inverse();

      bool updateRot(false), updateTrans(false);
      S32 rotOffset(0), transOffset(0);
      if (seq.rotationMatters.test(nodeIndex))
      {
         updateRot = true;
         rotOffset = seq.baseRotation + seq.rotationMatters.count(nodeIndex) * seq.numKeyframes;
      }
      if (seq.translationMatters.test(nodeIndex))
      {
         updateTrans = true;
         transOffset = seq.baseTranslation + seq.translationMatters.count(nodeIndex) * seq.numKeyframes;
      }

      for (S32 frame = 0; frame < seq.numKeyframes; frame++)
      {
         MatrixF oldMat;
         getNodeKeyframe(nodeIndex, seq, frame, &oldMat);

         MatrixF newMat;
         newMat.mul(refMat, oldMat);

         if (updateRot)
            nodeRotations[rotOffset + frame].set(QuatF(newMat));
         if (updateTrans)
            nodeTranslations[transOffset + frame] = newMat.getPosition();
      }
   }

   // Update sequence blend information
   seq.sourceData.blendSeq = blendRefSeqName;
   seq.sourceData.blendFrame = blendRefFrame;

   return true;
}

bool TSShape::setSequenceGroundSpeed(const String& seqName, const Point3F& trans, const Point3F& rot)
{
   // Find the sequence
   S32 seqIndex = findSequence(seqName);
   if (seqIndex < 0)
   {
      Con::errorf("setSequenceGroundSpeed: Could not find sequence named '%s'", seqName.c_str());
      return false;
   }
   TSShape::Sequence& seq = sequences[seqIndex];

   // Determine how many ground-frames to generate (FPS=10, at least 1 frame)
   const F32 groundFrameRate = 10.0f;
   S32 numFrames = getMax(1, (S32)(seq.duration * groundFrameRate));

   // Allocate space for the frames (add/delete frames as required)
   S32 frameAdjust = numFrames - seq.numGroundFrames;
   for (S32 i = 0; i < mAbs(frameAdjust); i++)
   {
      if (frameAdjust > 0)
      {
         groundTranslations.insert(seq.firstGroundFrame);
         groundRotations.insert(seq.firstGroundFrame);
      }
      else
      {
         groundTranslations.erase(seq.firstGroundFrame);
         groundRotations.erase(seq.firstGroundFrame);
      }
   }

   // Fixup ground frame indices
   seq.numGroundFrames += frameAdjust;
   for (S32 i = seqIndex+1; i < sequences.size(); i++)
      sequences[i].firstGroundFrame += frameAdjust;

   // Generate the ground-frames
   Point3F adjTrans = trans;
   Point3F adjRot = rot;
   if (seq.numGroundFrames > 0)
   {
      adjTrans /= seq.numGroundFrames;
      adjRot /= seq.numGroundFrames;
   }
   QuatF rotSpeed(adjRot);
   QuatF groundRot(rotSpeed);
   for (S32 i = 0; i < seq.numGroundFrames; i++)
   {
      groundTranslations[seq.firstGroundFrame + i] = adjTrans * (i + 1);
      groundRotations[seq.firstGroundFrame + i].set(groundRot);
      groundRot *= rotSpeed;
   }

   // set MakePath flag so ground frames will be animated
   seq.flags |= TSShape::MakePath;

   return true;
}
