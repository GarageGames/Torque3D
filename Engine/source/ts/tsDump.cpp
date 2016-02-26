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

#include "ts/tsShapeInstance.h"
#include "ts/tsMaterialList.h"
#include "core/strings/stringFunctions.h"


//-------------------------------------------------------------------------------------
// Dump shape structure:
//-------------------------------------------------------------------------------------

#define dumpLine(buffer) {str = buffer; stream.write((int)dStrlen(str),str);}

void TSShapeInstance::dumpNode(Stream & stream ,S32 level, S32 nodeIndex, Vector<S32> & detailSizes)
{
   if (nodeIndex < 0)
      return;

   // limit level to prevent overflow
   if (level > 160)
      level = 160;

   S32 i;
   const char * str;
   char space[512];
   for (i = 0; i < level*3; i++)
      space[i] = ' ';
   space[level*3] = '\0';

   const char *nodeName = "";
   const TSShape::Node & node = mShape->nodes[nodeIndex];
   if (node.nameIndex != -1)
     nodeName = mShape->getName(node.nameIndex);
   dumpLine(avar("%s%s", space, nodeName));

   // find all the objects that hang off this node...
   Vector<ObjectInstance*> objectList;
   for (i=0; i<mMeshObjects.size(); i++)
      if (mMeshObjects[i].nodeIndex == nodeIndex)
         objectList.push_back(&mMeshObjects[i]);

   if (objectList.size() == 0)
      dumpLine("\r\n");

   S32 spaceCount = -1;
   for (S32 j=0;j<objectList.size(); j++)
   {
      // should be a dynamic cast, but MSVC++ has problems with this...
      MeshObjectInstance * obj = (MeshObjectInstance *)(objectList[j]);
      if (!obj)
         continue;

      // object name
      const char *objectName = "";
      if (obj->object->nameIndex!=-1)
         objectName = mShape->getName(obj->object->nameIndex);

      // more spaces if this is the second object on this node
      if (spaceCount>0)
      {
         char buf[2048];
         dMemset(buf,' ',spaceCount);
         buf[spaceCount] = '\0';
         dumpLine(buf);
      }

      // dump object name
      dumpLine(avar(" --> Object %s with following details: ",objectName));

      // dump object detail levels
      for (S32 k=0; k<obj->object->numMeshes; k++)
      {
         S32 f = obj->object->startMeshIndex;
         if (mShape->meshes[f+k])
            dumpLine(avar(" %i",detailSizes[k]));
      }

      dumpLine("\r\n");

      // how many spaces should we prepend if we have another object on this node
      if (spaceCount<0)
         spaceCount = (S32)(dStrlen(space) + dStrlen(nodeName));

      if(spaceCount > 2000)
         spaceCount = 2000;
   }

   // search for children
   for (S32 k=nodeIndex+1; k<mShape->nodes.size(); k++)
   {
      if (mShape->nodes[k].parentIndex == nodeIndex)
         // this is our child
         dumpNode(stream, level+1, k, detailSizes);
   }
}

void TSShapeInstance::dump(Stream & stream)
{
   S32 i,j,ss,od,sz;
   const char * name;
   const char * str;

   dumpLine("\r\nShape Hierarchy:\r\n");

   dumpLine("\r\n   Details:\r\n");

   for (i=0; i<mShape->details.size(); i++)
   {
      const TSDetail & detail = mShape->details[i];
      name = mShape->getName(detail.nameIndex);
      ss = detail.subShapeNum;
      od = detail.objectDetailNum;
      sz = (S32)detail.size;
      if (ss >= 0)
      {
         dumpLine(avar("      %s, Subtree %i, objectDetail %i, size %i\r\n",name,ss,od,sz));
      }
      else
      {
         dumpLine(avar("      %s, AutoBillboard, size %i\r\n", name, sz));
      }
   }

   dumpLine("\r\n   Subtrees:\r\n");

   for (i=0; i<mShape->subShapeFirstNode.size(); i++)
   {
      S32 a = mShape->subShapeFirstNode[i];
      S32 b = a + mShape->subShapeNumNodes[i];
      dumpLine(avar("      Subtree %i\r\n",i));

      // compute detail sizes for each subshape
      Vector<S32> detailSizes;
      for (S32 l=0;l<mShape->details.size(); l++)
      {
          if ((mShape->details[l].subShapeNum==i) || (mShape->details[l].subShapeNum==-1))
              detailSizes.push_back((S32)mShape->details[l].size);
      }

      for (j=a; j<b; j++)
      {
          const TSNode & node = mShape->nodes[j];
          // if the node has a parent, it'll get dumped via the parent
          if (node.parentIndex<0)
              dumpNode(stream,3,j,detailSizes);
      }
   }

   bool foundSkin = false;
   for (i=0; i<mShape->objects.size(); i++)
   {
      TSShape::Object& currentObject = mShape->objects[i];

      if (currentObject.nodeIndex<0) // must be a skin
      {
         if (!foundSkin)
            dumpLine("\r\n   Skins:\r\n");
         foundSkin=true;
         const char * skinName = "";
         S32 nameIndex = currentObject.nameIndex;
         if (nameIndex>=0)
            skinName = mShape->getName(nameIndex);
         dumpLine(avar("      Skin %s with following details: ",skinName));
         for (S32 num=0; num<currentObject.numMeshes; num++)
         {
            if (mShape->meshes[currentObject.startMeshIndex + num])
               dumpLine(avar(" %i",(S32)mShape->details[num].size));
         }
         dumpLine("\r\n");
      }
   }
   if (foundSkin)
      dumpLine("\r\n");

   dumpLine("\r\n   Sequences:\r\n");
   for (i = 0; i < mShape->sequences.size(); i++)
   {
      const char *name = "(none)";
      if (mShape->sequences[i].nameIndex != -1)
         name = mShape->getName(mShape->sequences[i].nameIndex);
      dumpLine(avar("      %3d: %s%s%s\r\n", i, name,
         mShape->sequences[i].isCyclic() ? " (cyclic)" : "",
         mShape->sequences[i].isBlend() ? " (blend)" : ""));
   }

   if (mShape->materialList)
   {
      TSMaterialList * ml = mShape->materialList;
      dumpLine("\r\n   Material list:\r\n");
      for (i=0; i<(S32)ml->size(); i++)
      {
         U32 flags = ml->getFlags(i);
         const String& name = ml->getMaterialName(i);
         dumpLine(avar(
            "   material #%i: '%s'%s.", i, name.c_str(),
            flags & (TSMaterialList::S_Wrap|TSMaterialList::T_Wrap) ? "" : " not tiled")
         );
         if (flags & TSMaterialList::Translucent)
         {
            if (flags & TSMaterialList::Additive)
               dumpLine("  Additive-translucent.")
            else if (flags & TSMaterialList::Subtractive)
               dumpLine("  Subtractive-translucent.")
            else
               dumpLine("  Translucent.")
         }
         dumpLine("\r\n");
      }
   }
}

