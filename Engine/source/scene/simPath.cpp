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
#include "scene/simPath.h"

#include "gfx/gfxDevice.h"
#include "gfx/gfxVertexBuffer.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "gfx/gfxTransformSaver.h"
#include "console/consoleTypes.h"
#include "scene/pathManager.h"
#include "scene/sceneRenderState.h"
#include "math/mathIO.h"
#include "core/stream/bitStream.h"
#include "renderInstance/renderPassManager.h"
#include "console/engineAPI.h"

extern bool gEditingMission;

//--------------------------------------------------------------------------
//-------------------------------------- Console functions and cmp funcs
//
DefineEngineFunction(pathOnMissionLoadDone, void, (),,
   "@brief Load all Path information from the mission.\n\n"

   "This function is usually called from the loadMissionStage2() server-side function "
   "after the mission file has loaded.  Internally it places all Paths into the server's "
   "PathManager.  From this point the Paths are ready for transmission to the clients.\n\n"

   "@tsexample\n"
      "// Inform the engine to load all Path information from the mission.\n"
      "pathOnMissionLoadDone();\n\n"
   "@endtsexample\n"

   "@see NetConnection::transmitPaths()\n"
   "@see NetConnection::clearPaths()\n"
   "@see Path\n"

   "@ingroup Networking")
{
   // Need to load subobjects for all loaded interiors...
   SimGroup* pMissionGroup = dynamic_cast<SimGroup*>(Sim::findObject("MissionGroup"));
   AssertFatal(pMissionGroup != NULL, "Error, mission done loading and no mission group?");

   U32 currStart = 0;
   U32 currEnd   = 1;
   Vector<SimGroup*> groups;
   groups.push_back(pMissionGroup);

   while (true) {
      for (U32 i = currStart; i < currEnd; i++) {
         for (SimGroup::iterator itr = groups[i]->begin(); itr != groups[i]->end(); itr++) {
            if (dynamic_cast<SimGroup*>(*itr) != NULL)
               groups.push_back(static_cast<SimGroup*>(*itr));
         }
      }

      if (groups.size() == currEnd) {
         break;
      } else {
         currStart = currEnd;
         currEnd   = groups.size();
      }
   }

   for (U32 i = 0; i < groups.size(); i++) {
      SimPath::Path* pPath = dynamic_cast<SimPath::Path*>(groups[i]);
      if (pPath)
         pPath->updatePath();
   }
}

S32 FN_CDECL cmpPathObject(const void* p1, const void* p2)
{
   SimObject* o1 = *((SimObject**)p1);
   SimObject* o2 = *((SimObject**)p2);

   Marker* m1 = dynamic_cast<Marker*>(o1);
   Marker* m2 = dynamic_cast<Marker*>(o2);

   if (m1 == NULL && m2 == NULL)
      return 0;
   else if (m1 != NULL && m2 == NULL)
      return 1;
   else if (m1 == NULL && m2 != NULL)
      return -1;
   else {
      // Both markers...
      return S32(m1->mSeqNum) - S32(m2->mSeqNum);
   }
}

ConsoleDocClass(SimPath::Path,
   "@brief A spline along which various objects can move along. The spline object acts like a container for Marker objects, which make\n"
   "up the joints, or knots, along the path. Paths can be assigned a speed, can be looping or non-looping. Each of a path's markers can be\n"
   "one of three primary movement types: \"normal\", \"Position Only\", or \"Kink\". \n"

   "@tsexample\n"
	   "new path()\n"
	   "	{\n"
       "     isLooping = \"1\";\n"
       "\n"
       "     new Marker()\n"
       "		{\n"
       "			seqNum = \"0\";\n"
       "			type = \"Normal\";\n"
       "			msToNext = \"1000\";\n"
       "			smoothingType = \"Spline\";\n"
       "			position = \"-0.054708 -35.0612 234.802\";\n"
       "			rotation = \"1 0 0 0\";\n"
	   "      };\n"
	   "\n"
	   "	};\n"
   "@endtsexample\n"

   "@see Marker\n"
   "@see NetConnection::transmitPaths()\n"
   "@see NetConnection::clearPaths()\n"
   "@see Path\n"

   "@ingroup enviroMisc\n"
);

namespace SimPath
{

//--------------------------------------------------------------------------
//-------------------------------------- Implementation
//
IMPLEMENT_CONOBJECT(Path);

Path::Path()
{
   mPathIndex = NoPathIndex;
   mIsLooping = true;
}

Path::~Path()
{
   //
}

//--------------------------------------------------------------------------
void Path::initPersistFields()
{
   addField("isLooping",   TypeBool, Offset(mIsLooping, Path), "If this is true, the loop is closed, otherwise it is open.\n");

   Parent::initPersistFields();
   //
}



//--------------------------------------------------------------------------
bool Path::onAdd()
{
   if(!Parent::onAdd())
      return false;

   return true;
}


void Path::onRemove()
{
   //

   Parent::onRemove();
}



//--------------------------------------------------------------------------
/// Sort the markers objects into sequence order
void Path::sortMarkers()
{
   dQsort(objectList.address(), objectList.size(), sizeof(SimObject*), cmpPathObject);
}

void Path::updatePath()
{
   // If we need to, allocate a path index from the manager
   if (mPathIndex == NoPathIndex)
      mPathIndex = gServerPathManager->allocatePathId();

   sortMarkers();

   Vector<Point3F> positions;
   Vector<QuatF>   rotations;
   Vector<U32>     times;
   Vector<U32>     smoothingTypes;

   for (iterator itr = begin(); itr != end(); itr++)
   {
      Marker* pMarker = dynamic_cast<Marker*>(*itr);
      if (pMarker != NULL)
      {
         Point3F pos;
         pMarker->getTransform().getColumn(3, &pos);
         positions.push_back(pos);

         QuatF rot;
         rot.set(pMarker->getTransform());
         rotations.push_back(rot);

         times.push_back(pMarker->mMSToNext);
         smoothingTypes.push_back(pMarker->mSmoothingType);
      }
   }

   // DMMTODO: Looping paths.
   gServerPathManager->updatePath(mPathIndex, positions, rotations, times, smoothingTypes);
}

void Path::addObject(SimObject* obj)
{
   Parent::addObject(obj);

   if (mPathIndex != NoPathIndex) {
      // If we're already finished, and this object is a marker, then we need to
      //  update our path information...
      if (dynamic_cast<Marker*>(obj) != NULL)
         updatePath();
   }
}

void Path::removeObject(SimObject* obj)
{
   bool recalc = dynamic_cast<Marker*>(obj) != NULL;

   Parent::removeObject(obj);

   if (mPathIndex != NoPathIndex && recalc == true)
      updatePath();
}

DefineEngineMethod( Path, getPathId, S32, (),,
   "@brief Returns the PathID (not the object ID) of this path.\n\n"
   "@return PathID (not the object ID) of this path.\n"
   "@tsexample\n"
	   "// Acquire the PathID of this path object.\n"
	   "%pathID = %thisPath.getPathId();\n\n"
   "@endtsexample\n\n"
   )
{
   Path *path = static_cast<Path *>(object);
   return path->getPathIndex();
}

} // Namespace

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

GFXStateBlockRef Marker::smStateBlock;
GFXVertexBufferHandle<GFXVertexPCT> Marker::smVertexBuffer;
GFXPrimitiveBufferHandle Marker::smPrimitiveBuffer;

static Point3F wedgePoints[4] = {
   Point3F(-1, -1,  0),
   Point3F( 0,  1,  0),
   Point3F( 1, -1,  0),
   Point3F( 0,-.75, .5),
};

void Marker::initGFXResources()
{
   if(smVertexBuffer != NULL)
      return;
      
   GFXStateBlockDesc d;
   d.cullDefined = true;
   d.cullMode = GFXCullNone;
   
   smStateBlock = GFX->createStateBlock(d);
   
   smVertexBuffer.set(GFX, 4, GFXBufferTypeStatic);
   GFXVertexPCT* verts = smVertexBuffer.lock();
   verts[0].point = wedgePoints[0] * 1.25f;
   verts[1].point = wedgePoints[1] * 1.25f;
   verts[2].point = wedgePoints[2] * 1.25f;
   verts[3].point = wedgePoints[3] * 1.25f;
   verts[1].color = GFXVertexColor(ColorI(255, 0, 0, 255));
   verts[0].color = verts[2].color = verts[3].color = GFXVertexColor(ColorI(0, 0, 255, 255));
   smVertexBuffer.unlock();
   
   smPrimitiveBuffer.set(GFX, 24, 12, GFXBufferTypeStatic);
   U16* prims;
   smPrimitiveBuffer.lock(&prims);
   prims[0] = 0;
   prims[1] = 3;
   prims[2] = 3;
   prims[3] = 1;
   prims[4] = 1;
   prims[5] = 0;
   
   prims[6] = 3;
   prims[7] = 1;
   prims[8] = 1;
   prims[9] = 2;
   prims[10] = 2;
   prims[11] = 3;
   
   prims[12] = 0;
   prims[13] = 3;
   prims[14] = 3;
   prims[15] = 2;
   prims[16] = 2;
   prims[17] = 0;
   
   prims[18] = 0;
   prims[19] = 2;
   prims[20] = 2;
   prims[21] = 1;
   prims[22] = 1;
   prims[23] = 0;
   smPrimitiveBuffer.unlock();
}

IMPLEMENT_CO_NETOBJECT_V1(Marker);

ConsoleDocClass( Marker,
   "@brief A single joint, or knot, along a path. Should be stored inside a Path container object. A path markers can be\n"
   "one of three primary movement types: \"normal\", \"Position Only\", or \"Kink\". \n"

   "@tsexample\n"
	"new path()\n"
	"	{\n"
    "     isLooping = \"1\";\n"
    "\n"
    "     new Marker()\n"
    "		{\n"
    "			seqNum = \"0\";\n"
    "			type = \"Normal\";\n"
    "			msToNext = \"1000\";\n"
    "			smoothingType = \"Spline\";\n"
    "			position = \"-0.054708 -35.0612 234.802\";\n"
    "			rotation = \"1 0 0 0\";\n"
	"      };\n"
	"\n"
	"	};\n"
   "@endtsexample\n"
   "@see Path\n"
   "@ingroup enviroMisc\n"
);

Marker::Marker()
{
   // Not ghostable unless we're editing...
   mNetFlags.clear(Ghostable);

   mTypeMask |= MarkerObjectType;

   mSeqNum   = 0;
   mMSToNext = 1000;
   mSmoothingType = SmoothingTypeSpline;
   mKnotType = KnotTypeNormal;
}

Marker::~Marker()
{
   //
}

//--------------------------------------------------------------------------

ImplementEnumType( MarkerSmoothingType,
   "The type of smoothing this marker will have for pathed objects.\n"
   "@ingroup enviroMisc\n\n")
   { Marker::SmoothingTypeSpline , "Spline", "Marker will cause the movements of the pathed object to be smooth.\n" },
   { Marker::SmoothingTypeLinear , "Linear", "Marker will have no smoothing effect.\n" },
   //{ Marker::SmoothingTypeAccelerate , "Accelerate" },
EndImplementEnumType;

ImplementEnumType( MarkerKnotType,
   "The type of knot that this marker will be.\n"
   "@ingroup enviroMisc\n\n")
   { Marker::KnotTypeNormal ,       "Normal", "Knot will have a smooth camera translation/rotation effect.\n" },
   { Marker::KnotTypePositionOnly,  "Position Only", "Will do the same for translations, leaving rotation un-touched.\n" },
   { Marker::KnotTypeKink,          "Kink", "The rotation will take effect immediately for an abrupt rotation change.\n" },
EndImplementEnumType;

void Marker::initPersistFields()
{
   addGroup( "Misc" );
   addField("seqNum",   TypeS32, Offset(mSeqNum,   Marker), "Marker position in sequence of markers on this path.\n");
   addField("type", TYPEID< KnotType >(), Offset(mKnotType, Marker), "Type of this marker/knot. A \"normal\" knot will have a smooth camera translation/rotation effect.\n\"Position Only\" will do the same for translations, leaving rotation un-touched.\nLastly, a \"Kink\" means the rotation will take effect immediately for an abrupt rotation change.\n");
   addField("msToNext", TypeS32, Offset(mMSToNext, Marker), "Milliseconds to next marker in sequence.\n");
   addField("smoothingType", TYPEID< SmoothingType >(), Offset(mSmoothingType, Marker), "Path smoothing at this marker/knot. \"Linear\" means no smoothing, while \"Spline\" means to smooth.\n");
   endGroup("Misc");

   Parent::initPersistFields();
}

//--------------------------------------------------------------------------
bool Marker::onAdd()
{
   if(!Parent::onAdd())
      return false;

   mObjBox = Box3F(Point3F(-1.25, -1.25, -1.25), Point3F(1.25, 1.25, 1.25));
   resetWorldBox();

   if(gEditingMission)
      onEditorEnable();

   return true;
}


void Marker::onRemove()
{
   if(gEditingMission)
      onEditorDisable();

   Parent::onRemove();

   smVertexBuffer = NULL;
   smPrimitiveBuffer = NULL;
}

void Marker::onGroupAdd()
{
   mSeqNum = getGroup()->size() - 1;
}


/// Enable scoping so we can see this thing on the client.
void Marker::onEditorEnable()
{
   mNetFlags.set(Ghostable);
   setScopeAlways();
   addToScene();
}

/// Disable scoping so we can see this thing on the client
void Marker::onEditorDisable()
{
   removeFromScene();
   mNetFlags.clear(Ghostable);
   clearScopeAlways();
}


/// Tell our parent that this Path has been modified
void Marker::inspectPostApply()
{
   SimPath::Path *path = dynamic_cast<SimPath::Path*>(getGroup());
   if (path)
      path->updatePath();
}


//--------------------------------------------------------------------------
void Marker::prepRenderImage( SceneRenderState* state )
{
   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &Marker::renderObject );
   ri->type = RenderPassManager::RIT_Editor;
   state->getRenderPass()->addInst(ri);
}


void Marker::renderObject(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* overrideMat)
{
   initGFXResources();
   
   for(U32 i = 0; i < GFX->getNumSamplers(); i++)
      GFX->setTexture(i, NULL);
   GFXTransformSaver saver;
   MatrixF mat = getRenderTransform();
   mat.scale(mObjScale);
   GFX->multWorld(mat);
   
   GFX->setStateBlock(smStateBlock);
   GFX->setVertexBuffer(smVertexBuffer);
   GFX->setPrimitiveBuffer(smPrimitiveBuffer);
   GFX->setupGenericShaders();
   GFX->drawIndexedPrimitive(GFXLineList, 0, 0, 4, 0, 12);
}


//--------------------------------------------------------------------------
U32 Marker::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   // Note that we don't really care about efficiency here, since this is an
   //  edit-only ghost...
   stream->writeAffineTransform(mObjToWorld);

   return retMask;
}

void Marker::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   // Transform
   MatrixF otow;
   stream->readAffineTransform(&otow);

   setTransform(otow);
}