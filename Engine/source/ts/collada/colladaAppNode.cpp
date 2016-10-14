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

#ifdef _MSC_VER
#pragma warning(disable : 4706)  // disable warning about assignment within conditional
#endif

#include "ts/loader/appSequence.h"
#include "ts/collada/colladaExtensions.h"
#include "ts/collada/colladaAppNode.h"
#include "ts/collada/colladaAppMesh.h"
#include "ts/collada/colladaAppMesh.h"

#include "core/stringTable.h"

// Trim leading and trailing whitespace from the first word in the string
// Note that the string is modified.
static char* TrimFirstWord(char* str)
{
   char* value = str;

   // Trim leading whitespace
   while ( value && *value && dIsspace( *value ) )
      value++;

   // Trim trailing whitespace
   if ( value && *value )
   {
      char* end = value + 1;
      while ( *end && !dIsspace( *end ) )
         end++;
      *end = '\0';
   }

   return value;
}

ColladaAppNode::ColladaAppNode(const domNode* node, ColladaAppNode* parent)
      : p_domNode(node), appParent(parent),
        nodeExt(new ColladaExtension_node(node)),
        invertMeshes(false),
        lastTransformTime(TSShapeLoader::DefaultTime-1),
        defaultTransformValid(false)
      
{
   mName = dStrdup(_GetNameOrId(node));
   mParentName = dStrdup(parent ? parent->getName() : "ROOT");

   // Extract user properties from the <node> extension as whitespace separated
   // "name=value" pairs
   char* properties = dStrdup(nodeExt->user_properties);
   char* pos = properties;
   char* end = properties + dStrlen( properties );
   while ( pos < end )
   {
      // Find the '=' character to separate the name and value pair
      char* split = dStrchr( pos, '=' );
      if ( !split )
         break;

      // Get the name (whitespace trimmed string up to the '=')
      // and value (whitespace trimmed string after the '=')
      *split = '\0';
      char* name = TrimFirstWord( pos );
      char* value = TrimFirstWord( split + 1 );

      mProps.insert(StringTable->insert(name), dAtof(value));

      pos = value + dStrlen( value ) + 1;
   }

   dFree( properties );

   // Create vector of transform elements
   for (S32 iChild = 0; iChild < node->getContents().getCount(); iChild++) {
      switch (node->getContents()[iChild]->getElementType()) {
         case COLLADA_TYPE::TRANSLATE:
         case COLLADA_TYPE::ROTATE:
         case COLLADA_TYPE::SCALE:
         case COLLADA_TYPE::SKEW:
         case COLLADA_TYPE::MATRIX:
         case COLLADA_TYPE::LOOKAT:
            nodeTransforms.increment();
            nodeTransforms.last().element = node->getContents()[iChild];
            break;
      }
   }
}

// Get all child nodes
void ColladaAppNode::buildChildList()
{
   // Process children: collect <node> and <instance_node> elements
   for (S32 iChild = 0; iChild < p_domNode->getContents().getCount(); iChild++) {

      daeElement* child = p_domNode->getContents()[iChild];
      switch (child->getElementType()) {

         case COLLADA_TYPE::NODE:
         {
            domNode* node = daeSafeCast<domNode>(child);
            mChildNodes.push_back(new ColladaAppNode(node, this));
            break;
         }

         case COLLADA_TYPE::INSTANCE_NODE:
         {
            domInstance_node* instanceNode = daeSafeCast<domInstance_node>(child);
            domNode* node = daeSafeCast<domNode>(instanceNode->getUrl().getElement());
            if (node)
               mChildNodes.push_back(new ColladaAppNode(node, this));
            else
               Con::warnf("Failed to resolve instance_node with url=%s", instanceNode->getUrl().originalStr().c_str());
            break;
         }
      }
   }
}

// Get all geometry attached to this node
void ColladaAppNode::buildMeshList()
{
   // Process children: collect <instance_geometry> and <instance_controller> elements
   for (S32 iChild = 0; iChild < p_domNode->getContents().getCount(); iChild++) {

      daeElement* child = p_domNode->getContents()[iChild];
      switch (child->getElementType()) {

         case COLLADA_TYPE::INSTANCE_GEOMETRY:
         {
            // Only <geometry>.<mesh> instances are supported
            domInstance_geometry* instanceGeom = daeSafeCast<domInstance_geometry>(child);
            if (instanceGeom) {
               domGeometry* geometry = daeSafeCast<domGeometry>(instanceGeom->getUrl().getElement());
               if (geometry && geometry->getMesh())
                  mMeshes.push_back(new ColladaAppMesh(instanceGeom, this));
            }
            break;
         }

         case COLLADA_TYPE::INSTANCE_CONTROLLER:
            mMeshes.push_back(new ColladaAppMesh(daeSafeCast<domInstance_controller>(child), this));
            break;
      }
   }
}

bool ColladaAppNode::animatesTransform(const AppSequence* appSeq)
{
   // Check if any of this node's transform elements are animated during the
   // sequence interval
   for (S32 iTxfm = 0; iTxfm < nodeTransforms.size(); iTxfm++) {
      if (nodeTransforms[iTxfm].isAnimated(appSeq->getStart(), appSeq->getEnd()))
         return true;
   }
   return false;
}

/// Get the world transform of the node at the specified time
MatrixF ColladaAppNode::getNodeTransform(F32 time)
{
   // Avoid re-computing the default transform if possible
   if (defaultTransformValid && time == TSShapeLoader::DefaultTime)
   {
      return defaultNodeTransform;
   }
   else
   {
      MatrixF nodeTransform = getTransform(time);

      // Check for inverted node coordinate spaces => can happen when modelers
      // use the 'mirror' tool in their 3d app. Shows up as negative <scale>
      // transforms in the collada model.
      if (m_matF_determinant(nodeTransform) < 0.0f)
      {
         // Mark this node as inverted so we can mirror mesh geometry, then
         // de-invert the transform matrix
         invertMeshes = true;
         nodeTransform.scale(Point3F(1, 1, -1));
      }

      // Cache the default transform
      if (time == TSShapeLoader::DefaultTime)
      {
         defaultTransformValid = true;
         defaultNodeTransform = nodeTransform;
      }

      return nodeTransform;
   }
}

MatrixF ColladaAppNode::getTransform(F32 time)
{
   // Check if we can use the last computed transform
   if (time == lastTransformTime)
      return lastTransform;

   if (appParent) {
      // Get parent node's transform
      lastTransform = appParent->getTransform(time);
   }
   else {
      // no parent (ie. root level) => scale by global shape <unit>
      lastTransform.identity();
      lastTransform.scale(ColladaUtils::getOptions().unit);
      if (!isBounds())
         ColladaUtils::convertTransform(lastTransform);     // don't convert bounds node transform (or upAxis won't work!)
   }

   // Multiply by local node transform elements
   for (S32 iTxfm = 0; iTxfm < nodeTransforms.size(); iTxfm++) {

      MatrixF mat(true);

      // Convert the transform element to a MatrixF
      switch (nodeTransforms[iTxfm].element->getElementType()) {
         case COLLADA_TYPE::TRANSLATE: mat = vecToMatrixF<domTranslate>(nodeTransforms[iTxfm].getValue(time));  break;
         case COLLADA_TYPE::SCALE:     mat = vecToMatrixF<domScale>(nodeTransforms[iTxfm].getValue(time));      break;
         case COLLADA_TYPE::ROTATE:    mat = vecToMatrixF<domRotate>(nodeTransforms[iTxfm].getValue(time));     break;
         case COLLADA_TYPE::MATRIX:    mat = vecToMatrixF<domMatrix>(nodeTransforms[iTxfm].getValue(time));     break;
         case COLLADA_TYPE::SKEW:      mat = vecToMatrixF<domSkew>(nodeTransforms[iTxfm].getValue(time));       break;
         case COLLADA_TYPE::LOOKAT:    mat = vecToMatrixF<domLookat>(nodeTransforms[iTxfm].getValue(time));     break;
      }

      // Remove node scaling (but keep reflections) if desired
      if (ColladaUtils::getOptions().ignoreNodeScale)
      {
         Point3F invScale = mat.getScale();
         invScale.x = invScale.x ? (1.0f / invScale.x) : 0;
         invScale.y = invScale.y ? (1.0f / invScale.y) : 0;
         invScale.z = invScale.z ? (1.0f / invScale.z) : 0;
         mat.scale(invScale);
      }

      // Post multiply the animated transform
      lastTransform.mul(mat);
   }

   lastTransformTime = time;
   return lastTransform;
}
