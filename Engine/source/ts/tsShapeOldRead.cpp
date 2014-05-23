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

#include "core/strings/stringFunctions.h"
#include "core/util/endian.h"

#include "ts/tsShapeInstance.h"

//-------------------------------------------------
// put old skins into object list
//-------------------------------------------------

void TSShape::fixupOldSkins(S32 numMeshes, S32 numSkins, S32 numDetails, S32 * detFirstSkin, S32 * detailNumSkins)
{
#if !defined(TORQUE_MAX_LIB)
   // this method not necessary in exporter, and a couple lines won't compile for exporter
   if (!objects.address() || !meshes.address() || !numSkins)
      // not ready for this yet, will catch it on the next pass
      return;
   S32 numObjects = objects.size();
   TSObject * newObjects = objects.address() + objects.size();
   TSSkinMesh ** skins = (TSSkinMesh**)&meshes[numMeshes];
   Vector<TSSkinMesh*> skinsCopy;
   // Note: newObjects has as much free space as we need, so we just need to keep track of the
   //       number of objects we use and then update objects.size
   S32 numSkinObjects = 0;
   S32 skinsUsed = 0;
   S32 emptySkins = 0;
   S32 i;
   for (i=0; i<numSkins; i++)
      if (skins[i]==NULL)
         emptySkins++; // probably never, but just in case
   while (skinsUsed<numSkins-emptySkins)
   {
      TSObject & object = newObjects[numSkinObjects++];
      objects.increment();
      object.nameIndex = 0; // no name
      object.numMeshes = 0;
      object.startMeshIndex = numMeshes + skinsCopy.size();
      object.nodeIndex = -1;
      object.nextSibling = -1;
      for (S32 dl=0; dl<numDetails; dl++)
      {
         // find one mesh per detail to add to this object
         // don't really need to be versions of the same object
         i = 0;
         while (i<detFirstSkin[dl] || detFirstSkin[dl]<0)
            i++;
         for (; i<numSkins && i<detFirstSkin[dl]+detailNumSkins[dl]; i++)
         {
            if (skins[i])
            {
               // found an unused skin... copy it to skinsCopy and set to NULL
               skinsCopy.push_back(skins[i]);
               skins[i]=NULL;
               object.numMeshes++;
               skinsUsed++;
               break;
            }
         }
         if (i==numSkins || i==detFirstSkin[dl]+detailNumSkins[dl])
         {
            skinsCopy.push_back(NULL);
            object.numMeshes++;
         }
      }
      // exit above loop with one skin per detail...despose of trailing null meshes
      while (!skinsCopy.empty() && skinsCopy.last()==NULL)
      {
         skinsCopy.decrement();
         object.numMeshes--;
      }
      // if no meshes, don't need object
      if (!object.numMeshes)
      {
         objects.decrement();
         numSkinObjects--;
      }
   }
   dMemcpy(skins,skinsCopy.address(),skinsCopy.size()*sizeof(TSSkinMesh*));

   if (subShapeFirstObject.size()==1)
      // as long as only one subshape, we'll now be rendered
      subShapeNumObjects[0] += numSkinObjects;

   // now for something ugly -- we've added somoe objects to hold the skins...
   // now we have to add default states for those objects
   // we also have to increment base states on all the sequences that are loaded
   dMemmove(objectStates.address()+numObjects+numSkinObjects,objectStates.address()+numObjects,(objectStates.size()-numObjects)*sizeof(ObjectState));
   for (i=numObjects; i<numObjects+numSkinObjects; i++)
   {
      objectStates[i].vis=1.0f;
      objectStates[i].frameIndex=0;
      objectStates[i].matFrameIndex=0;
   }
   for (i=0;i<sequences.size();i++)
   {
      sequences[i].baseObjectState += numSkinObjects;
   }
#endif
}

//-------------------------------------------------
// some macros used for read/write
//-------------------------------------------------

// write a vector of structs (minus the first 'm')
#define writeVectorStructMinus(a,m) \
{\
   s->write(a.size() - m); \
   for (S32 i=m;i<a.size();i++) \
      a[i].write(s); \
}

// write a vector of simple types (minus the first 'm')
#define writeVectorSimpleMinus(a,m) \
{\
   s->write(a.size() - m); \
   for (S32 i=m;i<a.size();i++) \
      s->write(a[i]); \
}

// same as above with m=0
#define writeVectorStruct(a) writeVectorStructMinus(a,0)
#define writeVectorSimple(a) writeVectorSimpleMinus(a,0)

// read a vector of structs -- over-writing any existing data
#define readVectorStruct(a) \
{ \
   S32 sz; \
   s->read(&sz); \
   a.setSize(sz); \
   for (S32 i=0;i<sz;i++) \
      a[i].read(s); \
}

// read a vector of simple types -- over-writing any existing data
#define readVectorSimple(a) \
{ \
   S32 sz; \
   s->read(&sz); \
   a.setSize(sz); \
   for (S32 i=0;i<sz;i++) \
      s->read(&a[i]); \
}

// read a vector of structs -- append to any existing data
#define appendVectorStruct(a) \
{ \
   S32 sz; \
   S32 oldSz = a.size(); \
   s->read(&sz); \
   a.setSize(oldSz + sz); \
   for (S32 i=0;i<sz;i++) \
      a[i + oldSz].read(s); \
}

// read a vector of simple types -- append to any existing data
#define appendVectorSimple(a) \
{ \
   S32 sz; \
   S32 oldSz = a.size(); \
   s->read(&sz); \
   a.setSize(oldSz + sz); \
   for (S32 i=0;i<sz;i++) \
      s->read(&a[i + oldSz]); \
}

//-------------------------------------------------
// export all sequences
//-------------------------------------------------
void TSShape::exportSequences(Stream * s)
{
   // write version
   s->write(smVersion);

   S32 i,sz;

   // write node names
   // -- this is how we will map imported sequence nodes to shape nodes
   sz = nodes.size();
   s->write(sz);
   for (i=0;i<nodes.size();i++)
      writeName(s,nodes[i].nameIndex);

   // legacy write -- write zero objects, don't pretend to support object export anymore
   s->write(0);

   // on import, we will need to adjust keyframe data based on number of
   // nodes/objects in this shape...number of nodes can be inferred from
   // above, but number of objects cannot be.  Write that quantity here:
   s->write(objects.size());

   // write node states -- skip default node states
   s->write(nodeRotations.size());
   for (i=0;i<nodeRotations.size();i++)
   {
      s->write(nodeRotations[i].x);
      s->write(nodeRotations[i].y);
      s->write(nodeRotations[i].z);
      s->write(nodeRotations[i].w);
   }
   s->write(nodeTranslations.size());
   for (i=0;i<nodeTranslations.size(); i++)
   {
      s->write(nodeTranslations[i].x);
      s->write(nodeTranslations[i].y);
      s->write(nodeTranslations[i].z);
   }
   s->write(nodeUniformScales.size());
   for (i=0;i<nodeUniformScales.size();i++)
      s->write(nodeUniformScales[i]);
   s->write(nodeAlignedScales.size());
   for (i=0;i<nodeAlignedScales.size();i++)
   {
      s->write(nodeAlignedScales[i].x);
      s->write(nodeAlignedScales[i].y);
      s->write(nodeAlignedScales[i].z);
   }
   s->write(nodeArbitraryScaleRots.size());
   for (i=0;i<nodeArbitraryScaleRots.size();i++)
   {
      s->write(nodeArbitraryScaleRots[i].x);
      s->write(nodeArbitraryScaleRots[i].y);
      s->write(nodeArbitraryScaleRots[i].z);
      s->write(nodeArbitraryScaleRots[i].w);
   }
   for (i=0;i<nodeArbitraryScaleFactors.size();i++)
   {
      s->write(nodeArbitraryScaleFactors[i].x);
      s->write(nodeArbitraryScaleFactors[i].y);
      s->write(nodeArbitraryScaleFactors[i].z);
   }
   s->write(groundTranslations.size());
   for (i=0;i<groundTranslations.size();i++)
   {
      s->write(groundTranslations[i].x);
      s->write(groundTranslations[i].y);
      s->write(groundTranslations[i].z);
   }
   for (i=0;i<groundRotations.size();i++)
   {
      s->write(groundRotations[i].x);
      s->write(groundRotations[i].y);
      s->write(groundRotations[i].z);
      s->write(groundRotations[i].w);
   }

   // write object states -- legacy..no object states
   s->write((S32)0);

   // write sequences
   s->write(sequences.size());
   for (i=0;i<sequences.size();i++)
   {
      Sequence & seq = sequences[i];

      // first write sequence name
      writeName(s,seq.nameIndex);

      // now write the sequence itself
      seq.write(s,false); // false --> don't write name index
   }

   // write out all the triggers...
   s->write(triggers.size());
   for (i=0; i<triggers.size(); i++)
   {
      s->write(triggers[i].state);
      s->write(triggers[i].pos);
   }
}

//-------------------------------------------------
// export a single sequence
//-------------------------------------------------
void TSShape::exportSequence(Stream * s, const TSShape::Sequence& seq, bool saveOldFormat)
{
   S32 currentVersion = smVersion;
   if ( saveOldFormat )
      smVersion = 24;

   // write version
   s->write(smVersion);

   // write node names
   s->write( nodes.size() );
   for ( S32 i = 0; i < nodes.size(); i++ )
      writeName( s, nodes[i].nameIndex );

   // legacy write -- write zero objects, don't pretend to support object export anymore
   s->write( (S32)0 );

   // on import, we will need to adjust keyframe data based on number of
   // nodes/objects in this shape...number of nodes can be inferred from
   // above, but number of objects cannot be.  Write that quantity here:
   s->write( objects.size() );

   // write node states -- skip default node states
   S32 count = seq.rotationMatters.count() * seq.numKeyframes;
   s->write( count );
   for ( S32 i = seq.baseRotation; i < seq.baseRotation + count; i++ )
   {
      s->write( nodeRotations[i].x );
      s->write( nodeRotations[i].y );
      s->write( nodeRotations[i].z );
      s->write( nodeRotations[i].w );
   }

   count = seq.translationMatters.count() * seq.numKeyframes;
   s->write( count );
   for ( S32 i = seq.baseTranslation; i < seq.baseTranslation + count; i++ )
   {
      s->write( nodeTranslations[i].x );
      s->write( nodeTranslations[i].y );
      s->write( nodeTranslations[i].z );
   }

   count = seq.scaleMatters.count() * seq.numKeyframes;
   if ( seq.animatesUniformScale() )
   {
      s->write( count );
      for ( S32 i = seq.baseScale; i < seq.baseScale + count; i++ )
         s->write( nodeUniformScales[i] );
   }
   else
      s->write( (S32)0 );

   if ( seq.animatesAlignedScale() )
   {
      s->write( count );
      for ( S32 i = seq.baseScale; i < seq.baseScale + count; i++ )
      {
         s->write( nodeAlignedScales[i].x );
         s->write( nodeAlignedScales[i].y );
         s->write( nodeAlignedScales[i].z );
      }
   }
   else
      s->write( (S32)0 );

   if ( seq.animatesArbitraryScale() )
   {
      s->write( count );
      for ( S32 i = seq.baseScale; i < seq.baseScale + count; i++ )
      {
         s->write( nodeArbitraryScaleRots[i].x );
         s->write( nodeArbitraryScaleRots[i].y );
         s->write( nodeArbitraryScaleRots[i].z );
         s->write( nodeArbitraryScaleRots[i].w );
      }
      for ( S32 i = seq.baseScale; i < seq.baseScale + count; i++ )
      {
         s->write( nodeArbitraryScaleFactors[i].x );
         s->write( nodeArbitraryScaleFactors[i].y );
         s->write( nodeArbitraryScaleFactors[i].z );
      }
   }
   else
      s->write( (S32)0 );

   s->write( seq.numGroundFrames );
   for ( S32 i = seq.firstGroundFrame; i < seq.firstGroundFrame + seq.numGroundFrames; i++ )
   {
      s->write( groundTranslations[i].x );
      s->write( groundTranslations[i].y );
      s->write( groundTranslations[i].z );
   }
   for ( S32 i = seq.firstGroundFrame; i < seq.firstGroundFrame + seq.numGroundFrames; i++ )
   {
      s->write( groundRotations[i].x );
      s->write( groundRotations[i].y );
      s->write( groundRotations[i].z );
      s->write( groundRotations[i].w );
   }

   // write object states -- legacy..no object states
   s->write( (S32)0 );

   // write the sequence
   s->write( (S32)1 );
   writeName( s, seq.nameIndex );
   {
      // Write a copy of the sequence with all offsets set to 0
      TSShape::Sequence tmpSeq(seq);
      tmpSeq.baseDecalState = 0;
      tmpSeq.baseObjectState = 0;
      tmpSeq.baseTranslation = 0;
      tmpSeq.baseRotation = 0;
      tmpSeq.baseScale = 0;
      tmpSeq.firstGroundFrame = 0;
      tmpSeq.firstTrigger = 0;

      tmpSeq.write( s, false );
   }

   // write the sequence triggers
   s->write( seq.numTriggers );
   for ( S32 i = seq.firstTrigger; i < seq.firstTrigger + seq.numTriggers; i++ )
   {
      s->write( triggers[i].state );
      s->write( triggers[i].pos );
   }

   smVersion = currentVersion;
}

//-------------------------------------------------
// import sequences into existing shape
//-------------------------------------------------
bool TSShape::importSequences(Stream * s, const String& sequencePath)
{
   // write version
   s->read(&smReadVersion);
   if (smReadVersion>smVersion)
   {
      // error -- don't support future version yet :>
      Con::errorf(ConsoleLogEntry::General,
                  "Sequence import failed:  shape exporter newer than running executable.");
      return false;
   }
   if (smReadVersion<19)
   {
      // error -- don't support future version yet :>
      Con::errorf(ConsoleLogEntry::General,
         "Sequence import failed:  deprecated version (%i).",smReadVersion);
      return false;
   }

   Vector<S32> nodeMap;   // node index of each node from imported sequences
   Vector<S32> objectMap; // object index of objects from imported sequences
   VECTOR_SET_ASSOCIATION(nodeMap);
   VECTOR_SET_ASSOCIATION(objectMap);

   S32 i,sz;

   // read node names
   // -- this is how we will map imported sequence nodes to our nodes
   s->read(&sz);
   nodeMap.setSize(sz);

   for (i=0;i<sz;i++)
   {
      U32 startSize = names.size();
      S32 nameIndex = readName(s,true);
      
      nodeMap[i] = findNode(nameIndex);

      if (nodeMap[i] < 0)
      {
         // node found in sequence but not shape => remove the added node name
         if (names.size() != startSize)
         {
            names.decrement();

            if (names.size() != startSize)
               Con::errorf(ConsoleLogEntry::General, "TSShape::importSequence: failed to remove unused node correctly for dsq %s.", names[nameIndex].c_str(), sequencePath.c_str());
         }
      }
   }

   // read the following size, but won't do anything with it...legacy:  was going to support
   // import of sequences that animate objects...we don't...
   s->read(&sz);

   // before reading keyframes, take note of a couple numbers
   S32 oldShapeNumObjects;
   s->read(&oldShapeNumObjects);

   // adjust all the new keyframes
   S32 adjNodeRots = smReadVersion<22 ? nodeRotations.size() - nodeMap.size() : nodeRotations.size();
   S32 adjNodeTrans = smReadVersion<22 ? nodeTranslations.size() - nodeMap.size() : nodeTranslations.size();
   S32 adjGroundStates = smReadVersion<22 ? 0 : groundTranslations.size(); // groundTrans==groundRot

   // Read the node states into temporary vectors, then use the
   // nodeMap to discard unused transforms and map others to our nodes
   Vector<Quat16>    seqRotations;
   Vector<Point3F>   seqTranslations;
   Vector<F32>       seqUniformScales;
   Vector<Point3F>   seqAlignedScales;
   Vector<Quat16>    seqArbitraryScaleRots;
   Vector<Point3F>   seqArbitraryScaleFactors;

   if (smReadVersion>21)
   {
      s->read(&sz);
      seqRotations.setSize(sz);
      for (i=0; i < sz; i++)
      {
         s->read(&seqRotations[i].x);
         s->read(&seqRotations[i].y);
         s->read(&seqRotations[i].z);
         s->read(&seqRotations[i].w);
      }
      s->read(&sz);
      seqTranslations.setSize(sz);
      for (i=0; i <sz; i++)
      {
         s->read(&seqTranslations[i].x);
         s->read(&seqTranslations[i].y);
         s->read(&seqTranslations[i].z);
      }
      s->read(&sz);
      seqUniformScales.setSize(sz);
      for (i = 0; i < sz; i++)
         s->read(&seqUniformScales[i]);
      s->read(&sz);
      seqAlignedScales.setSize(sz);
      for (i = 0; i < sz; i++)
      {
         s->read(&seqAlignedScales[i].x);
         s->read(&seqAlignedScales[i].y);
         s->read(&seqAlignedScales[i].z);
      }
      s->read(&sz);
      seqArbitraryScaleRots.setSize(sz);
      for (i = 0; i <sz; i++)
      {
         s->read(&seqArbitraryScaleRots[i].x);
         s->read(&seqArbitraryScaleRots[i].y);
         s->read(&seqArbitraryScaleRots[i].z);
         s->read(&seqArbitraryScaleRots[i].w);
      }
      seqArbitraryScaleFactors.setSize(sz);
      for (i = 0; i < sz; i++)
      {
         s->read(&seqArbitraryScaleFactors[i].x);
         s->read(&seqArbitraryScaleFactors[i].y);
         s->read(&seqArbitraryScaleFactors[i].z);
      }

      // ground transforms can be read directly into the shape (none will be
      // discarded)
      s->read(&sz);
      S32 oldSz = groundTranslations.size();
      groundTranslations.setSize(sz+oldSz);
      for (i=oldSz;i<sz+oldSz;i++)
      {
         s->read(&groundTranslations[i].x);
         s->read(&groundTranslations[i].y);
         s->read(&groundTranslations[i].z);
      }
      groundRotations.setSize(sz+oldSz);
      for (i=oldSz;i<sz+oldSz;i++)
      {
         s->read(&groundRotations[i].x);
         s->read(&groundRotations[i].y);
         s->read(&groundRotations[i].z);
         s->read(&groundRotations[i].w);
      }
   }
   else
   {
      s->read(&sz);
      seqRotations.setSize(sz);
      seqTranslations.setSize(sz);
      for (i = 0; i < sz; i++)
      {
         s->read(&seqRotations[i].x);
         s->read(&seqRotations[i].y);
         s->read(&seqRotations[i].z);
         s->read(&seqRotations[i].w);
         s->read(&seqTranslations[i].x);
         s->read(&seqTranslations[i].y);
         s->read(&seqTranslations[i].z);
      }
   }

   // add these object states to our own -- shouldn't be any...assume it
   s->read(&sz);

   // read sequences
   s->read(&sz);
   S32 startSeqNum = sequences.size();
   for (i=0;i<sz;i++)
   {
      sequences.increment();
      Sequence & seq = sequences.last();

      // read name
      seq.nameIndex = readName(s,true);

      // read the rest of the sequence
      seq.read(s,false);
      seq.baseRotation = nodeRotations.size();
      seq.baseTranslation = nodeTranslations.size();

      if (smReadVersion > 21)
      {
         if (seq.animatesUniformScale())
            seq.baseScale = nodeUniformScales.size();
         else if (seq.animatesAlignedScale())
            seq.baseScale = nodeAlignedScales.size();
         else if (seq.animatesArbitraryScale())
            seq.baseScale = nodeArbitraryScaleFactors.size();
      }

      // remap the node matters arrays
      S32 j;
      TSIntegerSet newTransMembership;
      TSIntegerSet newRotMembership;
      TSIntegerSet newScaleMembership;
      for (j = 0; j < (S32)nodeMap.size(); j++)
      {
         if (nodeMap[j] < 0)
            continue;

         if (seq.translationMatters.test(j))
            newTransMembership.set(nodeMap[j]);
         if (seq.rotationMatters.test(j))
            newRotMembership.set(nodeMap[j]);
         if (seq.scaleMatters.test(j))
            newScaleMembership.set(nodeMap[j]);
      }

      // resize node transform arrays
      nodeTranslations.increment(newTransMembership.count() * seq.numKeyframes);
      nodeRotations.increment(newRotMembership.count() * seq.numKeyframes);
      if (seq.flags & TSShape::ArbitraryScale)
      {
         S32 scaleCount = newScaleMembership.count() * seq.numKeyframes;
         nodeArbitraryScaleRots.increment(scaleCount);
         nodeArbitraryScaleFactors.increment(scaleCount);
      }
      else if (seq.flags & TSShape::AlignedScale)
         nodeAlignedScales.increment(newScaleMembership.count() * seq.numKeyframes);
      else
         nodeUniformScales.increment(newScaleMembership.count() * seq.numKeyframes);

      // remap node transforms from temporary arrays
      for (S32 j = 0; j < nodeMap.size(); j++)
      {
         if (nodeMap[j] < 0)
            continue;

         if (newTransMembership.test(nodeMap[j]))
         {
            S32 src = seq.numKeyframes * seq.translationMatters.count(j);
            S32 dest = seq.baseTranslation + seq.numKeyframes * newTransMembership.count(nodeMap[j]);
            dCopyArray(&nodeTranslations[dest], &seqTranslations[src], seq.numKeyframes);
         }
         if (newRotMembership.test(nodeMap[j]))
         {
            S32 src = seq.numKeyframes * seq.rotationMatters.count(j);
            S32 dest = seq.baseRotation + seq.numKeyframes * newRotMembership.count(nodeMap[j]);
            dCopyArray(&nodeRotations[dest], &seqRotations[src], seq.numKeyframes);
         }
         if (newScaleMembership.test(nodeMap[j]))
         {
            S32 src = seq.numKeyframes * seq.scaleMatters.count(j);
            S32 dest = seq.baseScale + seq.numKeyframes * newScaleMembership.count(nodeMap[j]);
            if (seq.flags & TSShape::ArbitraryScale)
            {
               dCopyArray(&nodeArbitraryScaleRots[dest], &seqArbitraryScaleRots[src], seq.numKeyframes);
               dCopyArray(&nodeArbitraryScaleFactors[dest], &seqArbitraryScaleFactors[src], seq.numKeyframes);
            }
            else if (seq.flags & TSShape::AlignedScale)
               dCopyArray(&nodeAlignedScales[dest], &seqAlignedScales[src], seq.numKeyframes);
            else
               dCopyArray(&nodeUniformScales[dest], &seqUniformScales[src], seq.numKeyframes);
         }
      }

      seq.translationMatters = newTransMembership;
      seq.rotationMatters = newRotMembership;
      seq.scaleMatters = newScaleMembership;

      // adjust trigger numbers...we'll read triggers after sequences...
      seq.firstTrigger += triggers.size();

      // finally, adjust ground transform's nodes states
      seq.firstGroundFrame += adjGroundStates;
   }

   if (smReadVersion<22)
   {
      for (i=startSeqNum; i<sequences.size(); i++)
      {
         // move ground transform data to ground vectors
         Sequence & seq = sequences[i];
         S32 oldSz = groundTranslations.size();
         groundTranslations.setSize(oldSz+seq.numGroundFrames);
         groundRotations.setSize(oldSz+seq.numGroundFrames);
         for (S32 j=0;j<seq.numGroundFrames;j++)
         {
            groundTranslations[j+oldSz] = nodeTranslations[seq.firstGroundFrame+adjNodeTrans+j];
            groundRotations[j+oldSz] = nodeRotations[seq.firstGroundFrame+adjNodeRots+j];
         }
         seq.firstGroundFrame = oldSz;
      }
   }

   // add the new triggers
   S32 oldSz = triggers.size();
   s->read(&sz);
   triggers.setSize(oldSz+sz);
   for (S32 i=0; i<sz;i++)
   {
      s->read(&triggers[i+oldSz].state);
      s->read(&triggers[i+oldSz].pos);
   }

   if (smInitOnRead)
      init();

   return true;
}

//-------------------------------------------------
// read/write sequence
//-------------------------------------------------
void TSShape::Sequence::read(Stream * s, bool readNameIndex)
{
   AssertISV(smReadVersion>=19,"Reading old sequence");

   if (readNameIndex)
      s->read(&nameIndex);
   flags = 0;
   if (TSShape::smReadVersion>21)
      s->read(&flags);
   else
      flags=0;

   s->read(&numKeyframes);
   s->read(&duration);

   if (TSShape::smReadVersion<22)
   {
      bool tmp = false;
      s->read(&tmp);
      if (tmp)
         flags |= Blend;
      s->read(&tmp);
      if (tmp)
         flags |= Cyclic;
      s->read(&tmp);
      if (tmp)
         flags |= MakePath;
   }

   s->read(&priority);
   s->read(&firstGroundFrame);
   s->read(&numGroundFrames);
   if (TSShape::smReadVersion>21)
   {
      s->read(&baseRotation);
      s->read(&baseTranslation);
      s->read(&baseScale);
      s->read(&baseObjectState);
      s->read(&baseDecalState);
   }
   else
   {
      s->read(&baseRotation);
      baseTranslation=baseRotation;
      s->read(&baseObjectState);
      s->read(&baseDecalState);
   }

   s->read(&firstTrigger);
   s->read(&numTriggers);
   s->read(&toolBegin);

   // now the membership sets:
   rotationMatters.read(s);
   if (TSShape::smReadVersion<22)
      translationMatters=rotationMatters;
   else
   {
      translationMatters.read(s);
      scaleMatters.read(s);
   }

   TSIntegerSet dummy;
   dummy.read(s); // DEPRECIATED: Decals
   dummy.read(s); // DEPRECIATED: Ifl materials

   visMatters.read(s);
   frameMatters.read(s);
   matFrameMatters.read(s);

   dirtyFlags = 0;
   if (rotationMatters.testAll() || translationMatters.testAll() || scaleMatters.testAll())
      dirtyFlags |= TSShapeInstance::TransformDirty;
   if (visMatters.testAll())
      dirtyFlags |= TSShapeInstance::VisDirty;
   if (frameMatters.testAll())
      dirtyFlags |= TSShapeInstance::FrameDirty;
   if (matFrameMatters.testAll())
      dirtyFlags |= TSShapeInstance::MatFrameDirty;
}

void TSShape::Sequence::write(Stream * s, bool writeNameIndex) const
{
   if (writeNameIndex)
      s->write(nameIndex);
   s->write(flags);
   s->write(numKeyframes);
   s->write(duration);
   s->write(priority);
   s->write(firstGroundFrame);
   s->write(numGroundFrames);
   s->write(baseRotation);
   s->write(baseTranslation);
   s->write(baseScale);
   s->write(baseObjectState);
   s->write(baseDecalState);
   s->write(firstTrigger);
   s->write(numTriggers);
   s->write(toolBegin);

   // now the membership sets:
   rotationMatters.write(s);
   translationMatters.write(s);
   scaleMatters.write(s);

   TSIntegerSet dummy;
   dummy.write(s); // DEPRECIATED: Decals
   dummy.write(s); // DEPRECIATED: Ifl materials

   visMatters.write(s);
   frameMatters.write(s);
   matFrameMatters.write(s);
}

void TSShape::writeName(Stream * s, S32 nameIndex)
{
   const char * name = "";
   if (nameIndex>=0)
      name = names[nameIndex];
   S32 sz = (S32)dStrlen(name);
   s->write(sz);
   if (sz)
      s->write(sz*sizeof(char),name);
}

S32 TSShape::readName(Stream * s, bool addName)
{
   static char buffer[256];
   S32 sz;
   S32 nameIndex = -1;
   s->read(&sz);
   if (sz)
   {
      s->read(sz*sizeof(char),buffer);
      buffer[sz] = '\0';
      nameIndex = findName(buffer);

      // Many modeling apps don't support spaces in names, so if the lookup
      // failed, try the name again with spaces replaced by underscores
      if (nameIndex < 0)
      {
         while (char *p = dStrchr(buffer, ' '))
            *p = '_';
         nameIndex = findName(buffer);
      }

      if (nameIndex<0 && addName)
      {
         nameIndex = names.size();
         names.increment();
         names.last() = buffer;
      }
   }

   return nameIndex;
}
