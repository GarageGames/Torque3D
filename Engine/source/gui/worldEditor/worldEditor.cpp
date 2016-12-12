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
#include "gui/worldEditor/worldEditor.h"

#include "gui/worldEditor/gizmo.h"
#include "gui/worldEditor/undoActions.h"
#include "gui/worldEditor/editorIconRegistry.h"
#include "gui/worldEditor/worldEditorSelection.h"
#include "core/stream/memStream.h"
#include "scene/simPath.h"
#include "scene/mixin/scenePolyhedralObject.h"
#include "gui/core/guiCanvas.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/groundPlane.h"
#include "collision/earlyOutPolyList.h"
#include "collision/concretePolyList.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "T3D/shapeBase.h"
#include "T3D/cameraSpline.h"
#include "T3D/convexShape.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxDebugEvent.h"
#include "platform/typetraits.h"
#include "T3D/prefab.h"
#include "math/mEase.h"
#include "T3D/tsStatic.h"


IMPLEMENT_CONOBJECT( WorldEditor );

ConsoleDocClass( WorldEditor,
   "@brief The main World Editor tool class\n\n"
   "Editor use only.\n\n"
   "@internal"
);

ImplementEnumType( WorldEditorDropType,
   "How to drop objects when placed or dropped in the world.\n"
   "@internal\n\n")
	{ WorldEditor::DropAtOrigin,           "atOrigin",      "Place at the scene origin (usually 0,0,0)\n"        },
   { WorldEditor::DropAtCamera,           "atCamera",       "Places at the same position as the camera, without rotation.\n"        },
   { WorldEditor::DropAtCameraWithRot,    "atCameraRot",    "Places at the same position as the camera, with the camera's rotation.\n"     },
   { WorldEditor::DropBelowCamera,        "belowCamera",    "Places below the camera.\n"    },
   { WorldEditor::DropAtScreenCenter,     "screenCenter",   "Places at a position projected outwards from the screen's center.\n"    },
   { WorldEditor::DropAtCentroid,         "atCentroid",     "Places at the center position of the current centroid.\n"      },
   { WorldEditor::DropToTerrain,          "toTerrain",      "Places on the terrain.\n"       },
   { WorldEditor::DropBelowSelection,     "belowSelection", "Places at a position below the selected object.\n"  }
EndImplementEnumType;

ImplementEnumType( WorldEditorAlignmentType,
   "How to snap when snapping is enabled.\n"
   "@internal\n\n")
	{ WorldEditor::AlignNone,  "None", "No alignement type.\n"   },
   { WorldEditor::AlignPosX,  "+X", "Snap towards the higher position on the X plane.\n"   },
   { WorldEditor::AlignPosY,  "+Y", "Snap towards the higher position on the Y plane.\n"   },
   { WorldEditor::AlignPosZ,  "+Z", "Snap towards the higher position on the Z plane.\n"   },
   { WorldEditor::AlignNegX,  "-X", "Snap towards the lower position on the X plane.\n"    },
   { WorldEditor::AlignNegY,  "-Y", "Snap towards the lower position on the Y plane.\n"    },
   { WorldEditor::AlignNegZ,  "-Z", "Snap towards the lower position on the Z plane.\n"    },
EndImplementEnumType;


// unnamed namespace for static data
namespace {

   static VectorF axisVector[3] = {
      VectorF(1.0f,0.0f,0.0f),
      VectorF(0.0f,1.0f,0.0f),
      VectorF(0.0f,0.0f,1.0f)
   };

   static Point3F BoxPnts[] = {
      Point3F(0.0f,0.0f,0.0f),
      Point3F(0.0f,0.0f,1.0f),
      Point3F(0.0f,1.0f,0.0f),
      Point3F(0.0f,1.0f,1.0f),
      Point3F(1.0f,0.0f,0.0f),
      Point3F(1.0f,0.0f,1.0f),
      Point3F(1.0f,1.0f,0.0f),
      Point3F(1.0f,1.0f,1.0f)
   };

   static U32 BoxVerts[][4] = {
      {0,2,3,1},     // -x
      {7,6,4,5},     // +x
      {0,1,5,4},     // -y
      {3,2,6,7},     // +y
      {0,4,6,2},     // -z
      {3,7,5,1}      // +z
   };

   //
   U32 getBoxNormalIndex(const VectorF & normal)
   {
      const F32 * pNormal = ((const F32 *)normal);

      F32 max = 0;
      S32 index = -1;

      for(U32 i = 0; i < 3; i++)
         if(mFabs(pNormal[i]) >= mFabs(max))
         {
            max = pNormal[i];
            index = i*2;
         }

      AssertFatal(index >= 0, "Failed to get best normal");
      if(max > 0.f)
         index++;

      return(index);
   }

   //
   Point3F getBoundingBoxCenter(SceneObject * obj)
   {
      Box3F box = obj->getObjBox();
      MatrixF mat = obj->getTransform();
      VectorF scale = obj->getScale();

      Point3F center(0,0,0);
      Point3F projPnts[8];

      for(U32 i = 0; i < 8; i++)
      {
         Point3F pnt;
         pnt.set(BoxPnts[i].x ? box.maxExtents.x : box.minExtents.x,
                 BoxPnts[i].y ? box.maxExtents.y : box.minExtents.y,
                 BoxPnts[i].z ? box.maxExtents.z : box.minExtents.z);

         // scale it
         pnt.convolve(scale);
         mat.mulP(pnt, &projPnts[i]);
         center += projPnts[i];
      }

      center /= 8;
      return(center);
   }

   //
   const char * parseObjectFormat(SimObject * obj, const char * format)
   {
      static char buf[1024];

      U32 curPos = 0;
      U32 len = dStrlen(format);

      for(U32 i = 0; i < len; i++)
      {
         if(format[i] == '$')
         {
            U32 j;
            for(j = i+1; j < len; j++)
               if(format[j] == '$')
                  break;

            if(j == len)
               break;

            char token[80];

            AssertFatal((j - i) < (sizeof(token) - 1), "token too long");
            dStrncpy(token, &format[i+1], (j - i - 1));
            token[j-i-1] = 0;

            U32 remaining = sizeof(buf) - curPos - 1;

            // look at the token
            if(!dStricmp(token, "id"))
               curPos += dSprintf(buf + curPos, remaining, "%d", obj->getId());
            else if( dStricmp( token, "name|class" ) == 0 )
            {
               const char* str;
               if( obj->getName() && obj->getName()[ 0 ] )
                  str = obj->getName();
               else
                  str = obj->getClassName();
                  
               curPos += dSprintf( buf + curPos, remaining, "%s", str );
            }
            else if(!dStricmp(token, "name|internal"))
            {
               if( obj->getName() || !obj->getInternalName() )
                  curPos += dSprintf(buf + curPos, remaining, "%s", obj->getName());
               else
                  curPos += dSprintf(buf + curPos, remaining, "[%s]", obj->getInternalName());
            }
            else if(!dStricmp(token, "name"))
               curPos += dSprintf(buf + curPos, remaining, "%s", obj->getName());
            else if(!dStricmp(token, "class"))
               curPos += dSprintf(buf + curPos, remaining, "%s", obj->getClassName());
            else if(!dStricmp(token, "namespace") && obj->getNamespace())
               curPos += dSprintf(buf + curPos, remaining, "%s", obj->getNamespace()->mName);

            //
            i = j;
         }
         else
            buf[curPos++] = format[i];
      }

      buf[curPos] = 0;
      return(buf);
   }

   //
   F32 snapFloat(F32 val, F32 snap)
   {
      if(snap == 0.f)
         return(val);

      F32 a = mFmod(val, snap);

      if(mFabs(a) > (snap / 2))
         val < 0.f ? val -= snap : val += snap;

      return(val - a);
   }

   //
   EulerF extractEuler(const MatrixF & matrix)
   {
      const F32 * mat = (const F32*)matrix;

      EulerF r;
      r.x = mAsin(mat[MatrixF::idx(2,1)]);

      if(mCos(r.x) != 0.f)
      {
         r.y = mAtan2(-mat[MatrixF::idx(2,0)], mat[MatrixF::idx(2,2)]);
         r.z = mAtan2(-mat[MatrixF::idx(0,1)], mat[MatrixF::idx(1,1)]);
      }
      else
      {
         r.y = 0.f;
         r.z = mAtan2(mat[MatrixF::idx(1,0)], mat[MatrixF::idx(0,0)]);
      }

      return(r);
   }
}

F32 WorldEditor::smProjectDistance = 20000.0f;

SceneObject* WorldEditor::getClientObj(SceneObject * obj)
{
   AssertFatal(obj->isServerObject(), "WorldEditor::getClientObj: not a server object!");

   NetConnection * toServer = NetConnection::getConnectionToServer();
   NetConnection * toClient = NetConnection::getLocalClientConnection();
   if (!toServer || !toClient)
      return NULL;

   S32 index = toClient->getGhostIndex(obj);
   if(index == -1)
      return(0);

   return(dynamic_cast<SceneObject*>(toServer->resolveGhost(index)));
}

void WorldEditor::markAsSelected( SimObject* object, bool state )
{
   object->setSelected( state );
   
   if( dynamic_cast< SceneObject* >( object ) )
   {
      SceneObject* clientObj = WorldEditor::getClientObj( static_cast< SceneObject* >( object ) );
      if( clientObj )
         clientObj->setSelected( state );
   }
}

void WorldEditor::setClientObjInfo(SceneObject * obj, const MatrixF & mat, const VectorF & scale)
{
   SceneObject * clientObj = getClientObj(obj);
   if(!clientObj)
      return;

   clientObj->setTransform(mat);
   clientObj->setScale(scale);
}

void WorldEditor::updateClientTransforms(Selection*  sel)
{
   for( U32 i = 0; i < sel->size(); ++ i )
   {
      SceneObject* serverObj = dynamic_cast< SceneObject* >( ( *sel )[ i ] );
      if( !serverObj )
         continue;
         
      SceneObject* clientObj = getClientObj( serverObj );
      if(!clientObj)
         continue;

      clientObj->setTransform(serverObj->getTransform());
      clientObj->setScale(serverObj->getScale());
   }
}

void WorldEditor::addUndoState()
{
   submitUndo( mSelected );
}

void WorldEditor::submitUndo( Selection* sel, const UTF8* label )
{
   // Grab the world editor undo manager.
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      Con::errorf( "WorldEditor::createUndo() - EUndoManager not found!" );
      return;           
   }

   // Setup the action.
   WorldEditorUndoAction *action = new WorldEditorUndoAction( label );
   for(U32 i = 0; i < sel->size(); i++)
   {
      SceneObject* object = dynamic_cast< SceneObject* >( ( *sel )[ i ] );
      if( !object )
         continue;
         
      WorldEditorUndoAction::Entry entry;

      entry.mMatrix = object->getTransform();
      entry.mScale = object->getScale();
      entry.mObjId = object->getId();
      action->mEntries.push_back( entry );
   }

   // Submit it.
   action->mWorldEditor = this;
   undoMan->addAction( action );
   
   // Mark the world editor as dirty!
   setDirty();
}

void WorldEditor::WorldEditorUndoAction::undo()
{
   // NOTE: This function also handles WorldEditorUndoAction::redo().

   MatrixF oldMatrix;
   VectorF oldScale;
   for( U32 i = 0; i < mEntries.size(); i++ )
   {
      SceneObject *obj;
      if ( !Sim::findObject( mEntries[i].mObjId, obj ) )
         continue;

      mWorldEditor->setClientObjInfo( obj, mEntries[i].mMatrix, mEntries[i].mScale );

      // Grab the current state.
      oldMatrix = obj->getTransform();
      oldScale = obj->getScale();

      // Restore the saved state.
      obj->setTransform( mEntries[i].mMatrix );
      obj->setScale( mEntries[i].mScale );

      // Store the previous state so the next time
      // we're called we can restore it.
      mEntries[i].mMatrix = oldMatrix;
      mEntries[i].mScale = oldScale;
   }

   // Mark the world editor as dirty!
   mWorldEditor->setDirty();
   mWorldEditor->mSelected->invalidateCentroid();

   // Let the script get a chance at it.
   Con::executef( mWorldEditor, "onWorldEditorUndo" );
}

//------------------------------------------------------------------------------
// edit stuff

bool WorldEditor::cutSelection(Selection*  sel)
{
   if ( !sel->size() )
      return false;

   // First copy the selection.
   copySelection( sel );

   // Grab the world editor undo manager.
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      Con::errorf( "WorldEditor::cutSelection() - EUndoManager not found!" );
      return false;           
   }

   // Setup the action.
   MEDeleteUndoAction *action = new MEDeleteUndoAction();
   while ( sel->size() )
      action->deleteObject( ( *sel )[0] );
   undoMan->addAction( action );

   // Mark the world editor as dirty!
   setDirty();

   return true;
}

bool WorldEditor::copySelection(Selection*  sel)
{
   mCopyBuffer.clear();

   for( U32 i = 0; i < sel->size(); i++ )
   {
      mCopyBuffer.increment();
      mCopyBuffer.last().save( ( *sel )[i] );
   }

   return true;
}

bool WorldEditor::pasteSelection( bool dropSel )
{
   clearSelection();

   // Grab the world editor undo manager.
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      Con::errorf( "WorldEditor::pasteSelection() - EUndoManager not found!" );
      return false;           
   }

   SimGroup *missionGroup = NULL;   
   if( isMethod( "getNewObjectGroup" ) )
   {
      const char* targetGroupName = Con::executef( this, "getNewObjectGroup" );
      if( targetGroupName && targetGroupName[ 0 ] && !Sim::findObject( targetGroupName, missionGroup ) )
         Con::errorf( "WorldEditor::pasteSelection() - no SimGroup called '%s'", targetGroupName );
   }

   if( !missionGroup )
   {
      if( !Sim::findObject( "MissionGroup", missionGroup ) )
      {
         Con::errorf( "WorldEditor::pasteSelection() - MissionGroup not found" );
         return false;
      }
   }

   // Setup the action.
   MECreateUndoAction *action = new MECreateUndoAction( "Paste" );

   for( U32 i = 0; i < mCopyBuffer.size(); i++ )
   {
      SimObject* obj = mCopyBuffer[i].restore();
      if ( !obj )
         continue;

      if ( missionGroup )
         missionGroup->addObject( obj );

      action->addObject( obj );

      if ( !mSelectionLocked )
      {         
         mSelected->addObject( obj );
         Con::executef( this, "onSelect", obj->getIdString() );
      }
   }

   // Its safe to submit the action before the selection
   // is dropped below because the state of the objects 
   // are not stored until they are first undone.
   undoMan->addAction( action );

   // drop it ...
   if ( dropSel )
      dropSelection( mSelected );

   if ( mSelected->size() )
   {      
      char buf[16];
      dSprintf( buf, sizeof(buf), "%d", ( *mSelected )[0]->getId() );

      SimObject *obj = NULL;
      if(mRedirectID)
         obj = Sim::findObject(mRedirectID);
      Con::executef(obj ? obj : this, "onClick", buf);
   }

   // Mark the world editor as dirty!
   setDirty();

   return true;
}

//------------------------------------------------------------------------------

WorldEditorSelection* WorldEditor::getActiveSelectionSet() const
{
   return mSelected;
}

void WorldEditor::makeActiveSelectionSet( WorldEditorSelection* selection )
{
   Selection* oldSelection = getActiveSelectionSet();
   Selection* newSelection = selection;
   
   if( oldSelection == newSelection )
      return;
      
   // Unset the selection set so that calling onSelect/onUnselect callbacks
   // on the editor object will not affect the sets we have.
      
   mSelected = NULL;
      
   // Go through all objects in the old selection and for each
   // one that is not also in the new selection, signal an
   // unselect.
      
   if( oldSelection )
   {
      for( Selection::iterator iter = oldSelection->begin(); iter != oldSelection->end(); ++ iter )
         if( !newSelection || !newSelection->objInSet( *iter ) )
         {
            Con::executef( this, "onUnselect", ( *iter )->getIdString() );
            markAsSelected( *iter, false );
         }
            
      oldSelection->setAutoSelect( false );
   }
            
   // Go through all objects in the new selection and for each
   // one that is not also in the old selection, signal a
   // select.
            
   if( newSelection )
   {
      for( Selection::iterator iter = newSelection->begin(); iter != newSelection->end(); ++ iter )
         if( !oldSelection || !oldSelection->objInSet( *iter ) )
         {
            markAsSelected( *iter, true );
            Con::executef( this, "onSelect", ( *iter )->getIdString() );
         }
            
      newSelection->setAutoSelect( true );
   }
            
   // Install the new selection set.
            
   mSelected = newSelection;
   
   if( isMethod( "onSelectionSetChanged" ) )
      Con::executef( this, "onSelectionSetChanged" );
}

//------------------------------------------------------------------------------

void WorldEditor::hideObject(SceneObject* serverObj, bool hide)
{
   // client
   SceneObject * clientObj = getClientObj( serverObj );
   if( clientObj )
      clientObj->setHidden( hide );

   // server
   serverObj->setHidden( hide );
}

void WorldEditor::hideSelection(bool hide)
{
   SimGroup* pGroup = dynamic_cast<SimGroup*>(Sim::findObject("MissionGroup"));

   // set server/client objects hide field
   for(U32 i = 0; i < mSelected->size(); i++)
   {
      SceneObject* serverObj = dynamic_cast< SceneObject* >( ( *mSelected )[ i ] );
      if( !serverObj )
         continue;

      // Prevent non-mission group objects (i.e. Player) from being hidden.
      // Otherwise it is difficult to show them again.
      if(!serverObj->isChildOfGroup(pGroup))
         continue;

      hideObject(serverObj, hide);
   }
}

void WorldEditor::lockSelection(bool lock)
{
   //
   for(U32 i = 0; i < mSelected->size(); i++)
      ( *mSelected )[i]->setLocked(lock);
}

//------------------------------------------------------------------------------
// the centroid get's moved to the drop point...
void WorldEditor::dropSelection(Selection*  sel)
{
   if(!sel->size())
      return;

   setDirty();

   Point3F centroid = mObjectsUseBoxCenter ? sel->getBoxCentroid() : sel->getCentroid();

   switch(mDropType)
   {
      case DropAtCentroid:
         // already there
         break;

      case DropAtOrigin:
      {
         if(mDropAtBounds && !sel->containsGlobalBounds())
         {
            const Point3F& boxCenter = sel->getBoxCentroid();
            const Box3F& bounds = sel->getBoxBounds();
            Point3F offset = -boxCenter;
            offset.z += bounds.len_z() * 0.5f;

            sel->offset( offset, mGridSnap ? mGridPlaneSize : 0.f );
         }
         else
            sel->offset( Point3F( -centroid ), mGridSnap ? mGridPlaneSize : 0.f );

         break;
      }

      case DropAtCameraWithRot:
      {
         Point3F center = centroid;
         if(mDropAtBounds && !sel->containsGlobalBounds())
            center = sel->getBoxBottomCenter();

         sel->offset( Point3F( smCamPos - center ), mGridSnap ? mGridPlaneSize : 0.f );
         sel->orient(smCamMatrix, center);
         break;
      }

      case DropAtCamera:
      {
         Point3F center = centroid;
         if(mDropAtBounds && !sel->containsGlobalBounds())
            sel->getBoxBottomCenter();

         sel->offset( Point3F( smCamPos - center ), mGridSnap ? mGridPlaneSize : 0.f );
         break;
      }

      case DropBelowCamera:
      {
         Point3F center = centroid;
         if(mDropAtBounds && !sel->containsGlobalBounds())
            center = sel->getBoxBottomCenter();

         Point3F offset = smCamPos - center;
         offset.z -= mDropBelowCameraOffset;
         sel->offset( offset, mGridSnap ? mGridPlaneSize : 0.f );
         break;
      }

      case DropAtScreenCenter:
      {
         // Use the center of the selection bounds
         Point3F center = sel->getBoxCentroid();

         Gui3DMouseEvent event;
         event.pos = smCamPos;

         // Calculate the center of the sceen (in global screen coordinates)
         Point2I offset = localToGlobalCoord(Point2I(0,0));
         Point3F sp(F32(offset.x + F32(getExtent().x / 2)), F32(offset.y + (getExtent().y / 2)), 1.0f);

         // Calculate the view distance to fit the selection
         // within the camera's view.
         const Box3F bounds = sel->getBoxBounds();
         F32 radius = bounds.len()*0.5f;
         F32 viewdist = calculateViewDistance(radius) * mDropAtScreenCenterScalar;

         // Be careful of infinite sized objects, or just large ones in general.
         if(viewdist > mDropAtScreenCenterMax)
            viewdist = mDropAtScreenCenterMax;

         // Position the selection
         Point3F wp;
         unproject(sp, &wp);
         event.vec = wp - smCamPos;
         event.vec.normalizeSafe();
         event.vec *= viewdist;
         sel->offset( Point3F( event.pos - center ) += event.vec, mGridSnap ? mGridPlaneSize : 0.f );

         break;
      }

      case DropToTerrain:
      {
         terrainSnapSelection(sel, 0, mGizmo->getPosition(), true);
         break;
      }

      case DropBelowSelection:
      {
         dropBelowSelection(sel, centroid, mDropAtBounds);
         break;
      }
   }

   //
   updateClientTransforms(sel);
}

void WorldEditor::dropBelowSelection(Selection*  sel, const Point3F & centroid, bool useBottomBounds)
{
   if(!sel->size())
      return;

   Point3F start;
   if(useBottomBounds && !sel->containsGlobalBounds())
      start = sel->getBoxBottomCenter();
   else
      start = centroid;

   Point3F end = start;
   end.z -= 4000.f;
      
   sel->disableCollision(); // Make sure we don't hit ourselves.

   RayInfo ri;
   bool hit = gServerContainer.castRay(start, end, STATIC_COLLISION_TYPEMASK, &ri);
      
   sel->enableCollision();

   if( hit )
      sel->offset( ri.point - start, mGridSnap ? mGridPlaneSize : 0.f );
}

//------------------------------------------------------------------------------

void WorldEditor::terrainSnapSelection(Selection* sel, U8 modifier, Point3F gizmoPos, bool forceStick)
{
   mStuckToGround = false;

   if ( !mStickToGround && !forceStick )
      return;

   if(!sel->size())
      return;

   if(sel->containsGlobalBounds())
      return;

   Point3F centroid;
   if(mDropAtBounds && !sel->containsGlobalBounds())
      centroid = sel->getBoxBottomCenter();
   else
      centroid = mObjectsUseBoxCenter ? sel->getBoxCentroid() : sel->getCentroid();

   Point3F start = centroid;
   Point3F end = start;
   start.z -= 2000;
   end.z += 2000.f;
      
   sel->disableCollision(); // Make sure we don't hit ourselves.

   RayInfo ri;
   bool hit;
   if(mBoundingBoxCollision)
      hit = gServerContainer.collideBox(start, end, TerrainObjectType, &ri);
   else
      hit = gServerContainer.castRay(start, end, TerrainObjectType, &ri);
      
   sel->enableCollision();

   if( hit )
   {
      mStuckToGround = true;

      sel->offset( ri.point - centroid, mGridSnap ? mGridPlaneSize : 0.f );

      if(mTerrainSnapAlignment != AlignNone)
      {
         EulerF rot(0.0f, 0.0f, 0.0f); // Equivalent to AlignPosY
         switch(mTerrainSnapAlignment)
         {
            case AlignPosX:
               rot.set(0.0f, 0.0f, mDegToRad(-90.0f));
               break;

            case AlignPosZ:
               rot.set(mDegToRad(90.0f), 0.0f, mDegToRad(180.0f));
               break;

            case AlignNegX:
               rot.set(0.0f, 0.0f, mDegToRad(90.0f));
               break;

            case AlignNegY:
               rot.set(0.0f, 0.0f, mDegToRad(180.0f));
               break;

            case AlignNegZ:
               rot.set(mDegToRad(-90.0f), 0.0f, mDegToRad(180.0f));
               break;
         }

         MatrixF mat = MathUtils::createOrientFromDir(ri.normal);
         MatrixF rotMat(rot);

         sel->orient(mat.mul(rotMat), Point3F::Zero);
      }
   }
}

void WorldEditor::softSnapSelection(Selection* sel, U8 modifier, Point3F gizmoPos)
{
   mSoftSnapIsStuck = false;
   mSoftSnapActivated = false;

   // If soft snap is activated, holding CTRL will temporarily deactivate it.
   // Conversely, if soft snapping is deactivated, holding CTRL will activate it.
   if( (mSoftSnap && (modifier & SI_PRIMARY_CTRL)) || (!mSoftSnap && !(modifier & SI_PRIMARY_CTRL)) )
      return;

   if(!sel->size())
      return;

   if(sel->containsGlobalBounds())
      return;

   mSoftSnapActivated = true;

   Point3F centroid = mObjectsUseBoxCenter ? sel->getBoxCentroid() : sel->getCentroid();

   // Find objects we may stick against
   Vector<SceneObject*> foundobjs;

   SceneObject *controlObj = getControlObject();
   if ( controlObj )
      controlObj->disableCollision();

   sel->disableCollision();

   if(mSoftSnapSizeByBounds)
   {
      mSoftSnapBounds = sel->getBoxBounds();
      mSoftSnapBounds.setCenter(centroid);
   }
   else
   {
      mSoftSnapBounds.set(Point3F(mSoftSnapSize, mSoftSnapSize, mSoftSnapSize));
      mSoftSnapBounds.setCenter(centroid);
   }

   mSoftSnapPreBounds = mSoftSnapBounds;
   mSoftSnapPreBounds.setCenter(gizmoPos);

   SphereF sphere(gizmoPos, mSoftSnapPreBounds.len()*0.5f);

   gServerContainer.findObjectList(mSoftSnapPreBounds, 0xFFFFFFFF, &foundobjs);

   sel->enableCollision();

   if ( controlObj )
      controlObj->enableCollision();

   ConcretePolyList polys;
   for(S32 i=0; i<foundobjs.size(); ++i)
   {
      SceneObject* so = foundobjs[i];
      polys.setTransform(&(so->getTransform()), so->getScale());
      polys.setObject(so);
      so->buildPolyList(PLC_Selection, &polys, mSoftSnapPreBounds, sphere);
   }

   // Calculate sticky point
   bool     found = false;
   F32      foundDist = mSoftSnapPreBounds.len();
   Point3F  foundPoint(0.0f, 0.0f, 0.0f);
   PlaneF   foundPlane;
   MathUtils::IntersectInfo info;

   if(mSoftSnapDebugRender)
   {
      mSoftSnapDebugPoint.set(0.0f, 0.0f, 0.0f);
      mSoftSnapDebugTriangles.clear();
   }

   F32 backfaceToleranceSize = mSoftSnapBackfaceTolerance*mSoftSnapSize;
   for(S32 i=0; i<polys.mPolyList.size(); ++i)
   {
      ConcretePolyList::Poly p = polys.mPolyList[i];

      if(p.vertexCount >= 3)
      {
         S32 vertind[3];
         vertind[0] = polys.mIndexList[p.vertexStart];
         vertind[1] = polys.mIndexList[p.vertexStart + 1];
         vertind[2] = polys.mIndexList[p.vertexStart + 2];

         // Distance to the triangle
         F32 d = MathUtils::mTriangleDistance(polys.mVertexList[vertind[0]], polys.mVertexList[vertind[1]], polys.mVertexList[vertind[2]], gizmoPos, &info);

         // Cull backface polys that are not within tolerance
         if(p.plane.whichSide(gizmoPos) == PlaneF::Back && d > backfaceToleranceSize)
            continue;

         bool changed = false;
         if(d < foundDist)
         {
            changed = true;
            found = true;
            foundDist = d;
            foundPoint = info.segment.p1;
            foundPlane = p.plane;

            if(mSoftSnapRenderTriangle)
            {
               mSoftSnapTriangle.p0 = polys.mVertexList[vertind[0]];
               mSoftSnapTriangle.p1 = polys.mVertexList[vertind[1]];
               mSoftSnapTriangle.p2 = polys.mVertexList[vertind[2]];
            }
         }

         if(mSoftSnapDebugRender)
         {
            Triangle debugTri;
            debugTri.p0 = polys.mVertexList[vertind[0]];
            debugTri.p1 = polys.mVertexList[vertind[1]];
            debugTri.p2 = polys.mVertexList[vertind[2]];
            mSoftSnapDebugTriangles.push_back(debugTri);

            if(changed)
            {
               mSoftSnapDebugSnapTri = debugTri;
               mSoftSnapDebugPoint = foundPoint;
            }
         }
      }
   }

   if(found)
   {
      // Align selection to foundPlane normal
      if(mSoftSnapAlignment != AlignNone)
      {
         EulerF rot(0.0f, 0.0f, 0.0f); // Equivalent to AlignPosY
         switch(mSoftSnapAlignment)
         {
            case AlignPosX:
               rot.set(0.0f, 0.0f, mDegToRad(-90.0f));
               break;

            case AlignPosZ:
               rot.set(mDegToRad(90.0f), 0.0f, mDegToRad(180.0f));
               break;

            case AlignNegX:
               rot.set(0.0f, 0.0f, mDegToRad(90.0f));
               break;

            case AlignNegY:
               rot.set(0.0f, 0.0f, mDegToRad(180.0f));
               break;

            case AlignNegZ:
               rot.set(mDegToRad(-90.0f), 0.0f, mDegToRad(180.0f));
               break;
         }

         MatrixF mat = MathUtils::createOrientFromDir(foundPlane.getNormal());
         MatrixF rotMat(rot);

         sel->orient(mat.mul(rotMat), Point3F::Zero);
      }

      // Cast ray towards the selection to find how close to move it to the foundPoint
      F32 rayLength = mSoftSnapBounds.len() * 2;
      Point3F start = sel->getCentroid() - foundPlane.getNormal() * rayLength / 2;
      Point3F end = start + foundPlane.getNormal() * rayLength;

      RayInfo ri;
      F32 minT = TypeTraits< F32 >::MAX;

      for( U32 i = 0; i < sel->size(); ++ i )
      {
         SceneObject* object = dynamic_cast< SceneObject* >( ( *sel )[ i ] );
         if( !object )
            continue;

         // Convert start and end points to object space
         Point3F s, e;
         MatrixF mat = object->getTransform();
         mat.inverse();
         mat.mulP( start, &s );
         mat.mulP( end, &e );

         if ( object->castRayRendered( s, e, &ri ) )
            minT = getMin( minT, ri.t );
      }

      if ( minT <= 1.0f )
         foundPoint += ( end - start ) * (0.5f - minT);

      sel->offset( foundPoint - sel->getCentroid(), mGridSnap ? mGridPlaneSize : 0.f );
   }

   mSoftSnapIsStuck = found;
}

//------------------------------------------------------------------------------

SceneObject * WorldEditor::getControlObject()
{
   GameConnection * connection = GameConnection::getLocalClientConnection();
   if(connection)
      return(dynamic_cast<SceneObject*>(connection->getControlObject()));
   return(0);
}

bool WorldEditor::collide( const Gui3DMouseEvent &event, SceneObject **hitObj )
{
   // Collide against the screen-space class icons...

   S32 collidedIconIdx = -1;
   F32 collidedIconDist = F32_MAX;

   for ( U32 i = 0; i < mIcons.size(); i++ )      
   {
      const IconObject &icon = mIcons[i];      

      if ( icon.rect.pointInRect( event.mousePoint ) &&
           icon.dist < collidedIconDist )
      {
         collidedIconIdx = i;
         collidedIconDist = icon.dist;
      }
   }

   if ( collidedIconIdx != -1 )
   {
      *hitObj = mIcons[collidedIconIdx].object;
      return true;
   }

   if ( mBoundingBoxCollision )
   {
      // Raycast against sceneObject bounding boxes...

      SceneObject *controlObj = getControlObject();
      if ( controlObj )
         controlObj->disableCollision();

      Point3F startPnt = event.pos;
      Point3F endPnt = event.pos + event.vec * smProjectDistance;
      RayInfo ri;

      bool hit = gServerContainer.collideBox(startPnt, endPnt, 0xFFFFFFFF, &ri);

      if ( controlObj )
         controlObj->enableCollision();

      if ( hit )      
      {
         // If we hit an object that is in a Prefab...
         // we really want to hit / select that Prefab.         
         Prefab *prefab = Prefab::getPrefabByChild( ri.object );
         
         if ( prefab )
            *hitObj = prefab;
         else
            *hitObj = ri.object;

         return true;
      }
   }

   // No icon hit so check against the mesh
   if ( mObjectMeshCollision )
   {
      SceneObject *controlObj = getControlObject();
      if ( controlObj )
         controlObj->disableCollision();

      Point3F startPnt = event.pos;
      Point3F endPnt = event.pos + event.vec * smProjectDistance;
      RayInfo ri;

      bool hit = gServerContainer.castRayRendered(startPnt, endPnt, 0xFFFFFFFF, &ri);
      if(hit && ri.object && ( ri.object->getTypeMask() & (TerrainObjectType) || dynamic_cast< GroundPlane* >( ri.object )))
      {
         // We don't want to mesh select terrain
         hit = false;
      }

      if ( controlObj )
         controlObj->enableCollision();

      if ( hit )      
      {
         // If we hit an object that is in a Prefab...
         // we really want to hit / select that Prefab.         
         Prefab *prefab = Prefab::getPrefabByChild( ri.object );

         if ( prefab )
            *hitObj = prefab;
         else
            *hitObj = ri.object;

         return true;
      }
   }

   return false;   
}

//------------------------------------------------------------------------------
// main render functions

void WorldEditor::renderSelectionWorldBox(Selection*  sel)
{
   if( !mRenderSelectionBox || !sel->size() )
      return;
      
   // Compute the world bounds of the selection.

   Box3F selBox( TypeTraits< F32 >::MAX, TypeTraits< F32 >::MAX, TypeTraits< F32 >::MAX,
                 TypeTraits< F32 >::MIN, TypeTraits< F32 >::MIN, TypeTraits< F32 >::MIN );

   for( U32 i = 0; i < sel->size(); ++ i )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( ( *sel )[ i ] );
      if( !object )
         continue;
         
      const Box3F & wBox = object->getWorldBox();
      selBox.minExtents.setMin(wBox.minExtents);
      selBox.maxExtents.setMax(wBox.maxExtents);
   }
   
   // Set up the render state block, if we haven't done so
   // already.

   if ( mRenderSelectionBoxSB.isNull() )
   {
      GFXStateBlockDesc desc;
      
      desc.setCullMode( GFXCullNone );
      desc.alphaDefined = true;
      desc.alphaTestEnable = true;
      desc.setZReadWrite( true, true );
      mRenderSelectionBoxSB = GFX->createStateBlock( desc );
   }

   GFX->setStateBlock(mRenderSelectionBoxSB);

   PrimBuild::color( mSelectionBoxColor );

   // create the box points
   Point3F projPnts[8];
   for( U32 i = 0; i < 8; ++ i )
   {
      Point3F pnt;
      pnt.set(BoxPnts[i].x ? selBox.maxExtents.x : selBox.minExtents.x,
              BoxPnts[i].y ? selBox.maxExtents.y : selBox.minExtents.y,
              BoxPnts[i].z ? selBox.maxExtents.z : selBox.minExtents.z);
      projPnts[i] = pnt;
   }

   // do the box
   for(U32 j = 0; j < 6; j++)
   {
      PrimBuild::begin( GFXLineStrip, 4 );
      for(U32 k = 0; k < 4; k++)
      {
         PrimBuild::vertex3fv( projPnts[BoxVerts[j][k]] );
      }
      PrimBuild::end();
   }
}

void WorldEditor::renderObjectBox( SceneObject *obj, const ColorI &color )
{
   if ( mRenderObjectBoxSB.isNull() )
   {
      GFXStateBlockDesc desc;
      desc.setCullMode( GFXCullNone );
      desc.setZReadWrite( true, true );
      mRenderObjectBoxSB = GFX->createStateBlock( desc );
   }

   GFX->setStateBlock(mRenderObjectBoxSB);

   GFXTransformSaver saver;

   Box3F objBox = obj->getObjBox();
   Point3F objScale = obj->getScale();   
   Point3F boxScale = objBox.getExtents();
   Point3F boxCenter = obj->getWorldBox().getCenter();

   MatrixF objMat = obj->getTransform();
   objMat.scale(objScale);
   objMat.scale(boxScale);
   objMat.setPosition(boxCenter);

   //GFX->multWorld( objMat );

   PrimBuild::color( ColorI(255,255,255,255) );
   PrimBuild::begin( GFXLineList, 48 );

   //Box3F objBox = obj->getObjBox();
   //Point3F size = objBox.getExtents();
   //Point3F halfSize = size * 0.5f;

   static const Point3F cubePoints[8] = 
   {
      Point3F(-0.5, -0.5, -0.5), Point3F(-0.5, -0.5,  0.5), Point3F(-0.5,  0.5, -0.5), Point3F(-0.5,  0.5,  0.5),
      Point3F( 0.5, -0.5, -0.5), Point3F( 0.5, -0.5,  0.5), Point3F( 0.5,  0.5, -0.5), Point3F( 0.5,  0.5,  0.5)
   };

   // 8 corner points of the box   
   for ( U32 i = 0; i < 8; i++ )
   {
      //const Point3F &start = cubePoints[i];  

      // 3 lines per corner point
      for ( U32 j = 0; j < 3; j++ )
      {
         Point3F start = cubePoints[i];
         Point3F end = start;
         end[j] *= 0.8f;

         objMat.mulP(start);
         PrimBuild::vertex3fv(start);
         objMat.mulP(end);
         PrimBuild::vertex3fv(end);            
      }
   }

   PrimBuild::end();
}

void WorldEditor::renderObjectFace(SceneObject * obj, const VectorF & normal, const ColorI & col)
{
   if ( mRenderObjectFaceSB.isNull() )
   {
      GFXStateBlockDesc desc;
      desc.setCullMode( GFXCullNone );
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      desc.setZReadWrite( false );
      mRenderObjectFaceSB = GFX->createStateBlock( desc );
   }

   GFX->setStateBlock(mRenderObjectFaceSB);

   // get the normal index
   VectorF objNorm;
   obj->getWorldTransform().mulV(normal, &objNorm);

   U32 normI = getBoxNormalIndex(objNorm);

   //
   Box3F box = obj->getObjBox();
   MatrixF mat = obj->getTransform();
   VectorF scale = obj->getScale();

   Point3F projPnts[4];
   for(U32 i = 0; i < 4; i++)
   {
      Point3F pnt;
      pnt.set(BoxPnts[BoxVerts[normI][i]].x ? box.maxExtents.x : box.minExtents.x,
              BoxPnts[BoxVerts[normI][i]].y ? box.maxExtents.y : box.minExtents.y,
              BoxPnts[BoxVerts[normI][i]].z ? box.maxExtents.z : box.minExtents.z);

      // scale it
      pnt.convolve(scale);
      mat.mulP(pnt, &projPnts[i]);
   }

   PrimBuild::color( col );

   PrimBuild::begin( GFXTriangleStrip, 4 );
      for(U32 k = 0; k < 4; k++)
      {
         PrimBuild::vertex3f(projPnts[k].x, projPnts[k].y, projPnts[k].z);
      }
   PrimBuild::end();
}

void WorldEditor::renderMousePopupInfo()
{
   if ( !mMouseDragged )
      return;

      
   if ( mGizmoProfile->mode == NoneMode || !mGizmoProfile->renderInfoText )
      return;

   char buf[256];

   switch ( mGizmoProfile->mode )
   {
      case MoveMode:      
      {
         if ( !bool(mSelected)|| !mSelected->size() )
            return;

         Point3F pos = getSelectionCentroid();
         dSprintf(buf, sizeof(buf), "x: %0.3f, y: %0.3f, z: %0.3f", pos.x, pos.y, pos.z);

         break;
      }

      case RotateMode:
      {
         if ( !bool(mHitObject) || !bool(mSelected) || (mSelected->size() != 1) )
            return;

         // print out the angle-axis
         AngAxisF aa(mHitObject->getTransform());

         dSprintf(buf, sizeof(buf), "x: %0.3f, y: %0.3f, z: %0.3f, a: %0.3f",
            aa.axis.x, aa.axis.y, aa.axis.z, mRadToDeg(aa.angle));

         break;
      }

      case ScaleMode:
      {
         if ( !bool(mHitObject) || !bool(mSelected) || (mSelected->size() != 1) )
            return;

         VectorF scale = mHitObject->getScale();

         Box3F box = mHitObject->getObjBox();
         box.minExtents.convolve(scale);
         box.maxExtents.convolve(scale);

         box.maxExtents -= box.minExtents;
         dSprintf(buf, sizeof(buf), "w: %0.3f, h: %0.3f, d: %0.3f", box.maxExtents.x, box.maxExtents.y, box.maxExtents.z);

         break;
      }

      default:
         return;
   }

   U32 width = mProfile->mFont->getStrWidth((const UTF8 *)buf);
   Point2I posi( mLastMouseEvent.mousePoint.x, mLastMouseEvent.mousePoint.y + 12 );

   if ( mRenderPopupBackground )
   {
      Point2I minPt(posi.x - width / 2 - 2, posi.y - 1);
      Point2I maxPt(posi.x + width / 2 + 2, posi.y + mProfile->mFont->getHeight() + 1);

      GFX->getDrawUtil()->drawRectFill(minPt, maxPt, mPopupBackgroundColor);
   }

	GFX->getDrawUtil()->setBitmapModulation(mPopupTextColor);
   GFX->getDrawUtil()->drawText(mProfile->mFont, Point2I(posi.x - width / 2, posi.y), buf);
}

void WorldEditor::renderPaths(SimObject *obj)
{
   if (obj == NULL)
      return;
   bool selected = false;

   // Loop through subsets
   if (SimSet *set = dynamic_cast<SimSet*>(obj))
      for(SimSetIterator itr(set); *itr; ++itr) {
         renderPaths(*itr);
         if ((*itr)->isSelected())
            selected = true;
      }

   // Render the path if it, or any of it's immediate sub-objects, is selected.
   if (SimPath::Path *path = dynamic_cast<SimPath::Path*>(obj))
      if (selected || path->isSelected())
         renderSplinePath(path);
}


void WorldEditor::renderSplinePath(SimPath::Path *path)
{
   // at the time of writing the path properties are not part of the path object
   // so we don't know to render it looping, splined, linear etc.
   // for now we render all paths splined+looping

   Vector<Point3F> positions;
   Vector<QuatF>   rotations;

   path->sortMarkers();
   CameraSpline spline;

   for(SimSetIterator itr(path); *itr; ++itr)
   {
      Marker *pathmarker = dynamic_cast<Marker*>(*itr);
      if (!pathmarker)
         continue;
      Point3F pos;
      pathmarker->getTransform().getColumn(3, &pos);

      QuatF rot;
      rot.set(pathmarker->getTransform());
      CameraSpline::Knot::Type type;
      switch (pathmarker->mKnotType)
      {
         case Marker::KnotTypePositionOnly:  type = CameraSpline::Knot::POSITION_ONLY; break;
         case Marker::KnotTypeKink:          type = CameraSpline::Knot::KINK; break;
         case Marker::KnotTypeNormal:
         default:                            type = CameraSpline::Knot::NORMAL; break;

      }

      CameraSpline::Knot::Path path;
      switch (pathmarker->mSmoothingType)
      {
         case Marker::SmoothingTypeLinear:   path = CameraSpline::Knot::LINEAR; break;
         case Marker::SmoothingTypeSpline:
         default:                            path = CameraSpline::Knot::SPLINE; break;

      }

      spline.push_back(new CameraSpline::Knot(pos, rot, 1.0f, type, path));
   }

   F32 t = 0.0f;
   S32 size = spline.size();
   if (size <= 1)
      return;

   // DEBUG
   //spline.renderTimeMap();

   if (mSplineSB.isNull())
   {
      GFXStateBlockDesc desc;

      desc.setCullMode( GFXCullNone );
      desc.setBlend( true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      desc.samplersDefined = true;
      desc.samplers[0].textureColorOp = GFXTOPDisable;

      mSplineSB = GFX->createStateBlock( desc );
   }

   GFX->setStateBlock(mSplineSB);
   GFX->setupGenericShaders();

   if (path->isLooping())
   {
      CameraSpline::Knot *front = new CameraSpline::Knot(*spline.front());
      CameraSpline::Knot *back  = new CameraSpline::Knot(*spline.back());
      spline.push_back(front);
      spline.push_front(back);
      t = 1.0f;
      size += 2;
   }

   VectorF a(-0.45f, -0.55f, 0.0f);
   VectorF b( 0.0f,  0.55f, 0.0f);
   VectorF c( 0.45f, -0.55f, 0.0f);

   U32 vCount=0;

   F32 tmpT = t;
   while (tmpT < size - 1)
   {
      tmpT = spline.advanceDist(tmpT, 1.0f);
      vCount++;
   }

   // Build vertex buffer
 
   U32 batchSize = vCount;

   if(vCount > 4000)
      batchSize = 4000;

   GFXVertexBufferHandle<GFXVertexPCT> vb;
   vb.set(GFX, 3*batchSize, GFXBufferTypeVolatile);
   void *lockPtr = vb.lock();
   if(!lockPtr) return;

   U32 vIdx=0;

   while (t < size - 1)
   {
      CameraSpline::Knot k;
      spline.value(t, &k);
      t = spline.advanceDist(t, 1.0f);

      k.mRotation.mulP(a, &vb[vIdx+0].point);
      k.mRotation.mulP(b, &vb[vIdx+1].point);
      k.mRotation.mulP(c, &vb[vIdx+2].point);

      vb[vIdx+0].point += k.mPosition;
      vb[vIdx+1].point += k.mPosition;
      vb[vIdx+2].point += k.mPosition;

      vb[vIdx+0].color.set(0, 255, 0, 0);
      vb[vIdx+1].color.set(0, 255, 0, 255);
      vb[vIdx+2].color.set(0, 255, 0, 0);

      // vb[vIdx+3] = vb[vIdx+1];

      vIdx+=3;

      // Do we have to knock it out?
      if(vIdx > 3 * batchSize - 10)
      {
         vb.unlock();

         // Render the buffer
         GFX->setVertexBuffer(vb);
         GFX->drawPrimitive(GFXTriangleList,0,vIdx/3);

         // Reset for next pass...
         vIdx = 0;
         void *lockPtr = vb.lock();
         if(!lockPtr) return;
      }
   }

   vb.unlock();

   // Render the buffer
   GFX->setVertexBuffer(vb);
   //GFX->drawPrimitive(GFXLineStrip,0,3);

   if(vIdx)
      GFX->drawPrimitive(GFXTriangleList,0,vIdx/3);
}

void WorldEditor::renderScreenObj( SceneObject *obj, const Point3F& projPos, const Point3F& wPos )
{
   // Do not render control objects, hidden objects,
   // or objects that are within a prefab.
   if(obj == getControlObject() || obj->isHidden() || Prefab::getPrefabByChild(obj))
      return;

   GFXDrawUtil *drawer = GFX->getDrawUtil();
   
   // Lookup the ClassIcon - TextureHandle
   GFXTexHandle classIcon = gEditorIcons.findIcon( obj );

   if ( classIcon.isNull() )
      classIcon = mDefaultClassEntry.mDefaultHandle;

   U32 iconWidth = classIcon->getWidth();
   U32 iconHeight = classIcon->getHeight();

   bool isHighlight = ( obj == mHitObject || mDragSelected->objInSet(obj) );

   if ( isHighlight )
   {
      iconWidth += 0;
      iconHeight += 0;
   }

   Point2I sPos( (S32)projPos.x, (S32)projPos.y );
   //if ( obj->isSelected() )
   //   sPos.y += 4;
   Point2I renderPos = sPos;
   renderPos.x -= iconWidth / 2;
   renderPos.y -= iconHeight / 2;  

   Point2I iconSize( iconWidth, iconHeight );

   RectI renderRect( renderPos, iconSize );
   
   // Render object icon, except if the object is the
   // only selected object.  Do render the icon when there are
   // multiple selection as otherwise, objects like lights are
   // difficult to place.
   
   if( mRenderObjHandle && ( !obj->isSelected() || getSelectionSize() > 1 ) )
   {
      // Compute icon fade.

      S32 iconAlpha = 255;
      if( mFadeIcons && getDisplayType() == DisplayTypePerspective )
      {
         Point3F objDist = smCamPos - wPos;
         F32 dist = objDist.len();
         
         if( dist > mFadeIconsDist )
         {
            F32 iconDist = dist - mFadeIconsDist;
            iconAlpha = mClampF( 255 - ( 255 * ( iconDist / 10.f ) ), 0.f, 255.f );
         }
      }

      if ( isHighlight )      
         drawer->setBitmapModulation( ColorI(255,255,255, mClamp(iconAlpha + 50, 0, 255)) );               
      else      
         drawer->setBitmapModulation( ColorI(255,255,255,iconAlpha) );         

      drawer->drawBitmapStretch( classIcon, renderRect );      
      drawer->clearBitmapModulation();

      if ( obj->isLocked() )      
         drawer->drawBitmap( mDefaultClassEntry.mLockedHandle, renderPos );      

      // Save an IconObject for performing icon-click testing later.

      mIcons.increment();
      IconObject& lastIcon = mIcons.last();
      lastIcon.object = obj;
      lastIcon.rect = renderRect;
      lastIcon.dist = projPos.z;
      lastIcon.alpha = iconAlpha;
   }

   //
   if ( mRenderObjText && ( obj == mHitObject || obj->isSelected() ) )
   {      
      const char * str = parseObjectFormat(obj, mObjTextFormat);

      Point2I extent(mProfile->mFont->getStrWidth((const UTF8 *)str), mProfile->mFont->getHeight());

      Point2I pos(sPos);

      if(mRenderObjHandle)
      {
         pos.x += (classIcon->getWidth() / 2) - (extent.x / 2);
         pos.y += (classIcon->getHeight() / 2) + 3;
      }
	  
      
	  if(mGizmoProfile->mode == NoneMode){
		  
		 drawer->drawBitmapStretch( classIcon, renderRect );
		 drawer->setBitmapModulation( ColorI(255,255,255,255) ); 
		 drawer->drawText(mProfile->mFont, pos, str); 
		 if ( obj->isLocked() )      
			drawer->drawBitmap( mDefaultClassEntry.mLockedHandle, renderPos );      

			// Save an IconObject for performing icon-click testing later.
		{
			IconObject icon;
			icon.object = obj;
			icon.rect = renderRect;
			icon.dist = projPos.z;             
			mIcons.push_back( icon );
		}
	  }else{
		  drawer->setBitmapModulation(mObjectTextColor);
		  drawer->drawText(mProfile->mFont, pos, str);
	  };
   }
}

//------------------------------------------------------------------------------
// ClassInfo stuff

WorldEditor::ClassInfo::~ClassInfo()
{
   for(U32 i = 0; i < mEntries.size(); i++)
      delete mEntries[i];
}

bool WorldEditor::objClassIgnored(const SimObject * obj)
{
   ClassInfo::Entry * entry = getClassEntry(obj);
   if(mToggleIgnoreList)
      return(!(entry ? entry->mIgnoreCollision : false));
   else
      return(entry ? entry->mIgnoreCollision : false);
}

WorldEditor::ClassInfo::Entry * WorldEditor::getClassEntry(StringTableEntry name)
{
   AssertFatal(name, "WorldEditor::getClassEntry - invalid args");
   for(U32 i = 0; i < mClassInfo.mEntries.size(); i++)
      if(!dStricmp(name, mClassInfo.mEntries[i]->mName))
         return(mClassInfo.mEntries[i]);
   return(0);
}

WorldEditor::ClassInfo::Entry * WorldEditor::getClassEntry(const SimObject * obj)
{
   AssertFatal(obj, "WorldEditor::getClassEntry - invalid args");
   return(getClassEntry(obj->getClassName()));
}

bool WorldEditor::addClassEntry(ClassInfo::Entry * entry)
{
   AssertFatal(entry, "WorldEditor::addClassEntry - invalid args");
   if(getClassEntry(entry->mName))
      return(false);

   mClassInfo.mEntries.push_back(entry);
   return(true);
}

//------------------------------------------------------------------------------
// Mouse cursor stuff
void WorldEditor::setCursor(U32 cursor)
{
   mCurrentCursor = cursor;
}

//------------------------------------------------------------------------------
Signal<void(WorldEditor*)> WorldEditor::smRenderSceneSignal;

WorldEditor::WorldEditor()
   : mCurrentCursor(PlatformCursorController::curArrow)
{
   VECTOR_SET_ASSOCIATION( mIcons );
   
   // init the field data
   mDropType = DropAtScreenCenter;   
   mBoundingBoxCollision = true;   
   mObjectMeshCollision = true;
   mRenderPopupBackground = true;
   mPopupBackgroundColor.set(100,100,100);
   mPopupTextColor.set(255,255,0);
   mSelectHandle = StringTable->insert("tools/worldEditor/images/SelectHandle");
   mDefaultHandle = StringTable->insert("tools/worldEditor/images/DefaultHandle");
   mLockedHandle = StringTable->insert("tools/worldEditor/images/LockedHandle");
   mObjectTextColor.set(255,255,255);
   mObjectsUseBoxCenter = true;
   
   mObjSelectColor.set(255,0,0,200);
   mObjMultiSelectColor.set(128,0,0,200);
   mObjMouseOverSelectColor.set(0,0,255);
   mObjMouseOverColor.set(0,255,0);
   mShowMousePopupInfo = true;
   mDragRectColor.set(255,255,0);
   mRenderObjText = true;
   mRenderObjHandle = true;
   mObjTextFormat = StringTable->insert("$id$: $name|internal$");
   mFaceSelectColor.set(0,0,100,100);
   mRenderSelectionBox = true;
   mSelectionBoxColor.set(255,255,0,100);
   mSelectionLocked = false;

   mToggleIgnoreList = false;

   mIsDirty = false;

   mRedirectID = 0;

   //
   mHitObject = NULL;

   //
   //mDefaultMode = mCurrentMode = Move;
   mMouseDown = false;
   mDragSelect = false;

   mStickToGround = false;
   mStuckToGround = false;
   mTerrainSnapAlignment = AlignNone;
   mDropAtBounds = false;
   mDropBelowCameraOffset = 15.0f;
   mDropAtScreenCenterScalar = 1.0f;
   mDropAtScreenCenterMax = 100.0f;
   
   // Create the drag selection set.
   
   mDragSelected = new Selection();
   mDragSelected->registerObject( "EWorldEditorDragSelection" );
   Sim::getRootGroup()->addObject( mDragSelected );
   mDragSelected->setAutoSelect(false);

   //
   mSoftSnap = false;
   mSoftSnapActivated = false;
   mSoftSnapIsStuck = false;
   mSoftSnapAlignment = AlignNone;
   mSoftSnapRender = true;
   mSoftSnapRenderTriangle = false;
   mSoftSnapSizeByBounds = false;
   mSoftSnapSize = 2.0f;
   mSoftSnapBackfaceTolerance = 0.5f;
   mSoftSnapDebugRender = false;
   mSoftSnapDebugPoint.set(0.0f, 0.0f, 0.0f);
   
   mGridSnap = false;
   
   mFadeIcons = true;
   mFadeIconsDist = 8.f;
}

WorldEditor::~WorldEditor()
{
}

//------------------------------------------------------------------------------

bool WorldEditor::onAdd()
{
   if(!Parent::onAdd())
      return(false);

   // create the default class entry
   mDefaultClassEntry.mName = 0;
   mDefaultClassEntry.mIgnoreCollision = false;
   mDefaultClassEntry.mDefaultHandle   = GFXTexHandle(mDefaultHandle,   &GFXDefaultStaticDiffuseProfile, avar("%s() - mDefaultClassEntry.mDefaultHandle (line %d)", __FUNCTION__, __LINE__));
   mDefaultClassEntry.mSelectHandle    = GFXTexHandle(mSelectHandle,    &GFXDefaultStaticDiffuseProfile, avar("%s() - mDefaultClassEntry.mSelectHandle (line %d)", __FUNCTION__, __LINE__));
   mDefaultClassEntry.mLockedHandle    = GFXTexHandle(mLockedHandle,    &GFXDefaultStaticDiffuseProfile, avar("%s() - mDefaultClassEntry.mLockedHandle (line %d)", __FUNCTION__, __LINE__));

   if(!(mDefaultClassEntry.mDefaultHandle && mDefaultClassEntry.mSelectHandle && mDefaultClassEntry.mLockedHandle))
      return false;

   //mGizmo = new Gizmo();
   //mGizmo->registerObject("WorldEditorGizmo");   
   mGizmo->assignName("WorldEditorGizmo");   

   return true;
}

//------------------------------------------------------------------------------

void WorldEditor::onEditorEnable()
{
   // go through and copy the hidden field to the client objects...
   for(SimSetIterator itr(Sim::getRootGroup());  *itr; ++itr)
   {
      SceneObject * obj = dynamic_cast<SceneObject *>(*itr);
      if(!obj)
         continue;

      // only work with a server obj...
      if(obj->isClientObject())
         continue;

      // grab the client object
      SceneObject * clientObj = getClientObj(obj);
      if(!clientObj)
         continue;

      //
      clientObj->setHidden(obj->isHidden());
   }
}

//------------------------------------------------------------------------------

void WorldEditor::get3DCursor(GuiCursor *&cursor, bool &visible, const Gui3DMouseEvent &event)
{
   TORQUE_UNUSED(event);
   cursor = NULL;
   visible = false;

   GuiCanvas *pRoot = getRoot();
   if( !pRoot )
      return Parent::get3DCursor(cursor,visible,event);

   if(pRoot->mCursorChanged != mCurrentCursor)
   {
      PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
      AssertFatal(pWindow != NULL,"GuiControl without owning platform window!  This should not be possible.");
      PlatformCursorController *pController = pWindow->getCursorController();
      AssertFatal(pController != NULL,"PlatformWindow without an owned CursorController!");

      // We've already changed the cursor, 
      // so set it back before we change it again.
      if(pRoot->mCursorChanged != -1)
         pController->popCursor();

      // Now change the cursor shape
      pController->pushCursor(mCurrentCursor);
      pRoot->mCursorChanged = mCurrentCursor;
   }
}

//TODO: [rene 03/10 -- The entire event handling code here needs cleanup]

void WorldEditor::on3DMouseMove(const Gui3DMouseEvent & event)
{   
   setCursor(PlatformCursorController::curArrow);
   mHitObject = NULL;

   //
   mUsingAxisGizmo = false;

   if ( bool(mSelected) && mSelected->size() > 0 )
   {
      mGizmo->on3DMouseMove( event );

      if ( mGizmo->getSelection() != Gizmo::None )
      {
         mUsingAxisGizmo = true;
         mHitObject = dynamic_cast< SceneObject* >( ( *mSelected )[0] );
      }
   }

   if ( !mHitObject )
   {
      SceneObject *hitObj = NULL;
      if ( collide(event, &hitObj) && hitObj->isSelectionEnabled() && !objClassIgnored(hitObj) )
      {
         mHitObject = hitObj;
      }
   }
   
   mLastMouseEvent = event;
}

void WorldEditor::on3DMouseDown(const Gui3DMouseEvent & event)
{
   mMouseDown = true;
   mMouseDragged = false;
   mPerformedDragCopy = false;
   mDragGridSnapToggle = false;
   mLastMouseDownEvent = event;

   mouseLock();

   // check gizmo first
   mUsingAxisGizmo = false;
   mNoMouseDrag = false;

   if ( bool(mSelected) && mSelected->size() > 0 )
   {
      // Update the move grid settings depending on the
      // bounds of the current selection.

      const Box3F& selBounds = getActiveSelectionSet()->getBoxBounds();
      const F32 maxDim = getMax( selBounds.len_x(), getMax( selBounds.len_y(), selBounds.len_z() ) );
      const F32 size = mCeil( maxDim + 10.f );
      const F32 spacing = mCeil( size / 20.f );

     if( dynamic_cast< SceneObject* >( ( *mSelected )[0] ))
	  {
     
         if (size > 0)
         {
            mGizmo->setMoveGridSize( size );
            mGizmo->setMoveGridSpacing( spacing );
         }

         // Let the gizmo handle the event.

         mGizmo->on3DMouseDown( event );
      }

      if ( mGizmo->getSelection() != Gizmo::None )
      {
         mUsingAxisGizmo = true;         
         mHitObject = dynamic_cast< SceneObject* >( ( *mSelected )[0] );

         return;
      }
   }   

   SceneObject *hitObj = NULL;
   if ( collide( event, &hitObj ) && hitObj->isSelectionEnabled() && !objClassIgnored( hitObj ) )
   {
      mPossibleHitObject = hitObj;
      mNoMouseDrag = true;
   }
   else if ( !mSelectionLocked )
   {
      if ( !(event.modifier & ( SI_RANGESELECT | SI_MULTISELECT ) ) )
         clearSelection();

      mDragSelect = true;
      mDragSelected->clear();
      mDragRect.set( Point2I(event.mousePoint), Point2I(0,0) );
      mDragStart = event.mousePoint;
   }

   mLastMouseEvent = event;
}

void WorldEditor::on3DMouseUp( const Gui3DMouseEvent &event )
{
   const bool wasUsingAxisGizmo = mUsingAxisGizmo;
   
   mMouseDown = false;
   mStuckToGround = false;
   mSoftSnapIsStuck = false;
   mSoftSnapActivated = false;
   mUsingAxisGizmo = false;
   mGizmo->on3DMouseUp(event);
   
   // Restore grid snap if we temporarily toggled it.
   
   if( mDragGridSnapToggle )
   {
      mDragGridSnapToggle = false;
      const bool snapToGrid = !mGridSnap;
      mGridSnap = snapToGrid;
      mGizmo->getProfile()->snapToGrid = snapToGrid;
   }

   // check if selecting objects....
   if ( mDragSelect )
   {
      mDragSelect = false;
      mPossibleHitObject = NULL;
      
      const bool addToSelection = ( event.modifier & ( SI_RANGESELECT | SI_MULTISELECT ) );

      // add all the objects from the drag selection into the normal selection
      if( !addToSelection )
         clearSelection();

      if ( mDragSelected->size() > 1 )
      {
         for ( U32 i = 0; i < mDragSelected->size(); i++ )                     
            mSelected->addObject( ( *mDragSelected )[i] );                       
                  
         Con::executef( this, "onMultiSelect", mDragSelected->getIdString(), addToSelection ? "1" : "0" );
         mDragSelected->clear();

         SimObject *obj = NULL;
         if ( mRedirectID )
            obj = Sim::findObject( mRedirectID );
         Con::executef( obj ? obj : this, "onClick", ( *mSelected )[ 0 ]->getIdString() );
      }
      else if ( mDragSelected->size() == 1 )
      {         
         mSelected->addObject( ( *mDragSelected )[0] );    
         Con::executef( this, "onSelect", ( *mDragSelected )[ 0 ]->getIdString() );
         mDragSelected->clear();
         
         SimObject *obj = NULL;
         if ( mRedirectID )
            obj = Sim::findObject( mRedirectID );
         Con::executef( obj ? obj : this, "onClick", ( *mSelected )[ 0 ]->getIdString() );
      }

      mouseUnlock();
      return;
   }
   else if( mPossibleHitObject.isValid() && !wasUsingAxisGizmo )
   {
      if ( !mSelectionLocked )
      {
         if ( event.modifier & ( SI_RANGESELECT | SI_MULTISELECT ) )
         {
            mNoMouseDrag = true;
            if ( mSelected->objInSet( mPossibleHitObject ) )
            {
               mSelected->removeObject( mPossibleHitObject );
               mSelected->storeCurrentCentroid();
               Con::executef( this, "onUnSelect", mPossibleHitObject->getIdString() );
            }
            else
            {
               mSelected->addObject( mPossibleHitObject );
               mSelected->storeCurrentCentroid();
               Con::executef( this, "onSelect", mPossibleHitObject->getIdString() );
            }
         }
         else
         {
            if ( bool(mSelected) && !mSelected->objInSet( mPossibleHitObject ) )
            {
               mNoMouseDrag = true;

               // Call onUnSelect.  Because of the whole treeview<->selection synchronization,
               // this may actually cause things to disappear from mSelected so do the loop
               // in reverse.  This will make the loop work even if items are removed as
               // we go along.
               for( S32 i = mSelected->size() - 1; i >= 0; -- i )
                  Con::executef( this, "onUnSelect", ( *mSelected )[ i ]->getIdString() );
               
               mSelected->clear();
               mSelected->addObject( mPossibleHitObject );
               mSelected->storeCurrentCentroid();
               Con::executef( this, "onSelect", mPossibleHitObject->getIdString() );
            }
         }
      }

      if ( event.mouseClickCount > 1 )
      {
         //
         char buf[16];
         dSprintf(buf, sizeof(buf), "%d", mPossibleHitObject->getId());

         SimObject *obj = NULL;
         if ( mRedirectID )
            obj = Sim::findObject( mRedirectID );
         Con::executef( obj ? obj : this, "onDblClick", buf );
      }
      else 
      {
         char buf[16];
         dSprintf( buf, sizeof(buf), "%d", mPossibleHitObject->getId() );

         SimObject *obj = NULL;
         if ( mRedirectID )
            obj = Sim::findObject( mRedirectID );
         Con::executef( obj ? obj : this, "onClick", buf );
      }

      mHitObject = mPossibleHitObject;
   }

   if ( bool(mSelected) && mSelected->hasCentroidChanged() )
   {
      Con::executef( this, "onSelectionCentroidChanged");
   }

   if ( mMouseDragged && bool(mSelected) && mSelected->size() )
   {
      if ( mSelected->size() )
      {
         if ( isMethod("onEndDrag") )
         {
            SimObject * obj = 0;
            if ( mRedirectID )
               obj = Sim::findObject( mRedirectID );
            Con::executef( obj ? obj : this, "onEndDrag", ( *mSelected )[ 0 ]->getIdString() );
         }
      }
   }

   //if ( mHitObject )
   //   mHitObject->inspectPostApply();
   //mHitObject = NULL;  

   //
   //mHitObject = hitObj;
   mouseUnlock();
}

void WorldEditor::on3DMouseDragged(const Gui3DMouseEvent & event)
{
   if ( !mMouseDown )
      return;

   if ( mNoMouseDrag && !mUsingAxisGizmo )
   {
      // Perhaps we should start the drag after all
      if( mAbs(mLastMouseDownEvent.mousePoint.x - event.mousePoint.x) > 2 || mAbs(mLastMouseDownEvent.mousePoint.y - event.mousePoint.y) > 2 )
      {
         if ( !(event.modifier & ( SI_RANGESELECT | SI_MULTISELECT ) ) )
            clearSelection();

         mDragSelect = true;
         mDragSelected->clear();
         mDragRect.set( Point2I(mLastMouseDownEvent.mousePoint), Point2I(0,0) );
         mDragStart = mLastMouseDownEvent.mousePoint;

         mNoMouseDrag = false;
         mHitObject = NULL;
      }
      else
      {
         return;
      }
   }

   //
   if ( !mMouseDragged )
   {
      if ( !mUsingAxisGizmo )
      {
         // vert drag on new object.. reset hit offset
         if ( mHitObject && bool(mSelected) &&!mSelected->objInSet( mHitObject ) )
         {
            if ( !mSelectionLocked )
               mSelected->addObject( mHitObject );
         }
      }

      // create and add an undo state
      if ( !mDragSelect )
        submitUndo( mSelected );

      mMouseDragged = true;
   }

   // update the drag selection
   if ( mDragSelect )
   {
      // build the drag selection on the renderScene method - make sure no neg extent!
      mDragRect.point.x = (event.mousePoint.x < mDragStart.x) ? event.mousePoint.x : mDragStart.x;
      mDragRect.extent.x = (event.mousePoint.x > mDragStart.x) ? event.mousePoint.x - mDragStart.x : mDragStart.x - event.mousePoint.x;
      mDragRect.point.y = (event.mousePoint.y < mDragStart.y) ? event.mousePoint.y : mDragStart.y;
      mDragRect.extent.y = (event.mousePoint.y > mDragStart.y) ? event.mousePoint.y - mDragStart.y : mDragStart.y - event.mousePoint.y;
      return;
   }

   if ( !mUsingAxisGizmo && ( !mHitObject || !mSelected->objInSet( mHitObject ) ) )
      return;

   // anything locked?
   for ( U32 i = 0; i < mSelected->size(); i++ )
      if ( ( *mSelected )[i]->isLocked() )
         return;

   if ( mUsingAxisGizmo )
      mGizmo->on3DMouseDragged( event );

   switch ( mGizmoProfile->mode )
   {
   case MoveMode:

      // grabbed axis gizmo?
      if ( mUsingAxisGizmo )
      {
         // Check if a copy should be made
         if ( event.modifier & SI_SHIFT && !mPerformedDragCopy )
         {
            mPerformedDragCopy = true;
            mPossibleHitObject = NULL;
            
            copySelection( mSelected );
            pasteSelection( false );
         }
         
         // Check for grid snap toggle with ALT.
         
         if( event.modifier & SI_PRIMARY_ALT )
         {
            if( !mDragGridSnapToggle )
            {
               mDragGridSnapToggle = true;
               const bool snapToGrid = !mGridSnap;
               mGridSnap = snapToGrid;
               mGizmo->getProfile()->snapToGrid = snapToGrid;
            }
         }
         else if( mDragGridSnapToggle )
         {
            mDragGridSnapToggle = false;
            const bool snapToGrid = !mGridSnap;
            mGridSnap = snapToGrid;
            mGizmo->getProfile()->snapToGrid = snapToGrid;
         }

         mSelected->offset( mGizmo->getOffset() );

         // Handle various sticking
         terrainSnapSelection( mSelected, event.modifier, mGizmo->getPosition() );
         softSnapSelection( mSelected, event.modifier, mGizmo->getPosition() );

         updateClientTransforms( mSelected );
      }     
      break;

   case ScaleMode:
      if ( mUsingAxisGizmo )
      {
         Point3F scale = mGizmo->getScale() / (mGizmo->getScale() - mGizmo->getDeltaScale());

         // Can scale each object independently around its own origin, or scale
         // the selection as a group around the centroid
         if ( mObjectsUseBoxCenter )
            mSelected->scale( scale, getSelectionCentroid() );
         else
            mSelected->scale( scale );

         updateClientTransforms(mSelected);
      }

      break;

   case RotateMode:
   {
      Point3F centroid = getSelectionCentroid();
      EulerF rot = mGizmo->getDeltaRot();

      mSelected->rotate(rot, centroid);
      updateClientTransforms(mSelected);

      break;  
   }
      
   default:
      break;
   }

   mLastMouseEvent = event;
}

void WorldEditor::on3DMouseEnter(const Gui3DMouseEvent &)
{
}

void WorldEditor::on3DMouseLeave(const Gui3DMouseEvent &)
{
}

void WorldEditor::on3DRightMouseDown(const Gui3DMouseEvent & event)
{
}

void WorldEditor::on3DRightMouseUp(const Gui3DMouseEvent & event)
{
}

//------------------------------------------------------------------------------

void WorldEditor::updateGuiInfo()
{
   SimObject * obj = 0;
   if ( mRedirectID )
      obj = Sim::findObject( mRedirectID );

   char buf[] = "";
   Con::executef( obj ? obj : this, "onGuiUpdate", buf );
}

//------------------------------------------------------------------------------

static void findObjectsCallback( SceneObject *obj, void *val )
{
   Vector<SceneObject*> * list = (Vector<SceneObject*>*)val;
   list->push_back(obj);
}

struct DragMeshCallbackData {
   WorldEditor*            mWorldEditor;
   Box3F                   mBounds;
   SphereF                 mSphereBounds;
   Vector<SceneObject *>   mObjects;
   EarlyOutPolyList        mPolyList;
   MatrixF                 mStandardMat;
   Point3F                 mStandardScale;

   DragMeshCallbackData(WorldEditor* we, Box3F &bounds, SphereF &sphereBounds)
   {
      mWorldEditor = we;
      mBounds = bounds;
      mSphereBounds = sphereBounds;
      mStandardMat.identity();
      mStandardScale.set(1.0f, 1.0f, 1.0f);
   }
};

static Frustum gDragFrustum;
static void findDragMeshCallback( SceneObject *obj, void *data )
{
   DragMeshCallbackData* dragData = reinterpret_cast<DragMeshCallbackData*>(data);

   if ( dragData->mWorldEditor->objClassIgnored( obj ) ||
        !obj->isSelectionEnabled() ||
        obj->getTypeMask() & ( TerrainObjectType | ProjectileObjectType ) ||
        dynamic_cast< GroundPlane* >( obj ) ||
        Prefab::getPrefabByChild( obj ) )
   {
      return;
   }

   // Reset the poly list for us
   dragData->mPolyList.clear();
   dragData->mPolyList.setTransform(&(dragData->mStandardMat), dragData->mStandardScale);

   // Do the work
   obj->buildPolyList(PLC_Selection, &(dragData->mPolyList), dragData->mBounds, dragData->mSphereBounds);
   if (!dragData->mPolyList.isEmpty())
   {
      dragData->mObjects.push_back(obj);
   }
}

void WorldEditor::renderScene( const RectI &updateRect )
{
   GFXDEBUGEVENT_SCOPE( Editor_renderScene, ColorI::RED );

   smRenderSceneSignal.trigger(this);
	
   // Grab this before anything here changes it.
   Frustum frustum;
   {
      F32 left, right, top, bottom, nearPlane, farPlane;
      bool isOrtho = false;   
      GFX->getFrustum( &left, &right, &bottom, &top, &nearPlane, &farPlane, &isOrtho );

      MatrixF cameraMat = GFX->getWorldMatrix();
      cameraMat.inverse();

      frustum.set( isOrtho, left, right, top, bottom, nearPlane, farPlane, cameraMat );
   }

   // Render the paths
   renderPaths(Sim::findObject("MissionGroup"));

   // walk selected
   Selection* selection = getActiveSelectionSet();
   if( selection )
   {
      bool isMultiSelection = mSelected->size() > 1;
      for( U32 i = 0; i < mSelected->size(); i++ )
      {
         if ( (const SceneObject *)mHitObject == ( *mSelected )[i] )
            continue;
         SceneObject* object = dynamic_cast< SceneObject* >( ( *mSelected )[ i ] );
         if( object && mRenderSelectionBox )
            renderObjectBox( object, isMultiSelection ? mObjMultiSelectColor : mObjSelectColor );
      }
   }

   // do the drag selection
   for ( U32 i = 0; i < mDragSelected->size(); i++ )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( ( *mDragSelected )[ i ] );
      if( object && mRenderSelectionBox )
         renderObjectBox(object, mObjSelectColor);
   }

   if( selection )
   {
      // draw the mouse over obj
      if ( bool(mHitObject) && mRenderSelectionBox )
      {
         ColorI & col = selection->objInSet(mHitObject) ? mObjMouseOverSelectColor : mObjMouseOverColor;
         renderObjectBox(mHitObject, col);
      }

      // stuff to do if there is a selection
      if ( selection->size() )
      {   
         if ( mRenderSelectionBox && selection->size() > 1 )
            renderSelectionWorldBox(selection);
            
         SceneObject* singleSelectedSceneObject = NULL;
         if ( selection->size() == 1 )
            singleSelectedSceneObject = dynamic_cast< SceneObject* >( ( *selection )[ 0 ] );

         MatrixF objMat(true);
         if( singleSelectedSceneObject )
            objMat = singleSelectedSceneObject->getTransform();

         Point3F worldPos = getSelectionCentroid();

         Point3F objScale = singleSelectedSceneObject ? singleSelectedSceneObject->getScale() : Point3F(1,1,1);
         
         mGizmo->set( objMat, worldPos, objScale );

         // Change the gizmo's centroid highlight based on soft sticking state
         if( mSoftSnapIsStuck || mStuckToGround )
            mGizmo->setCentroidHandleHighlight( true );

         mGizmo->renderGizmo( mLastCameraQuery.cameraMatrix, mLastCameraQuery.fov );

         // Reset any highlighting
         if( mSoftSnapIsStuck || mStuckToGround )
            mGizmo->setCentroidHandleHighlight( false );

         // Soft snap box rendering
         if( (mSoftSnapRender || mSoftSnapRenderTriangle) && mSoftSnapActivated)
         {
            GFXDrawUtil *drawUtil = GFX->getDrawUtil();
            ColorI color;

            GFXStateBlockDesc desc;

            if(mSoftSnapRenderTriangle && mSoftSnapIsStuck)
            {
               desc.setBlend( false );
               desc.setZReadWrite( false, false );
               desc.fillMode = GFXFillWireframe;
               desc.cullMode = GFXCullNone;

               color.set( 255, 255, 128, 255 );
               drawUtil->drawTriangle( desc, mSoftSnapTriangle.p0, mSoftSnapTriangle.p1, mSoftSnapTriangle.p2, color );
            }

            if(mSoftSnapRender)
            {
               desc.setBlend( true );
               desc.blendSrc = GFXBlendOne;
               desc.blendDest = GFXBlendOne;
               desc.blendOp = GFXBlendOpAdd;
               desc.setZReadWrite( true, false );
               desc.cullMode = GFXCullCCW;

               color.set( 64, 64, 0, 255 );

               desc.fillMode = GFXFillWireframe;
               drawUtil->drawCube(desc, mSoftSnapPreBounds, color);

               desc.fillMode = GFXFillSolid;
               drawUtil->drawSphere(desc, mSoftSnapPreBounds.len()*0.05f, mSoftSnapPreBounds.getCenter(), color);
            }
         }
      }
   }

   // Debug rendering of the soft stick
   if(mSoftSnapDebugRender)
   {
      GFXDrawUtil *drawUtil = GFX->getDrawUtil();
      ColorI color( 255, 0, 0, 255 );

      GFXStateBlockDesc desc;
      desc.setBlend( false );
      desc.setZReadWrite( false, false );

      if(mSoftSnapIsStuck)
      {
         drawUtil->drawArrow( desc, getSelectionCentroid(), mSoftSnapDebugPoint, color );

         color.set(255, 255, 255);
         desc.fillMode = GFXFillWireframe;
         for(S32 i=0; i<mSoftSnapDebugTriangles.size(); ++i)
         {
            drawUtil->drawTriangle( desc, mSoftSnapDebugTriangles[i].p0, mSoftSnapDebugTriangles[i].p1, mSoftSnapDebugTriangles[i].p2, color );
         }

         color.set(255, 255, 0);
         desc.fillMode = GFXFillSolid;
         desc.cullMode = GFXCullNone;
         drawUtil->drawTriangle( desc, mSoftSnapDebugSnapTri.p0, mSoftSnapDebugSnapTri.p1, mSoftSnapDebugSnapTri.p2, color );
      }

   }

   // Now do the 2D stuff...
   // icons and text
   GFX->setClipRect(updateRect);

   // update what is in the selection
   if ( mDragSelect )
      mDragSelected->clear();

   // Determine selected objects based on the drag box touching
   // a mesh if a drag operation has begun.
   if( mDragSelect && mDragRect.extent.x > 1 && mDragRect.extent.y > 1 )
   {
      // Build the drag frustum based on the rect
      F32 wwidth;
      F32 wheight;
      F32 aspectRatio = F32(getWidth()) / F32(getHeight());
      
      if(!mLastCameraQuery.ortho)
      {
         wheight = mLastCameraQuery.nearPlane * mTan(mLastCameraQuery.fov / 2);
         wwidth = aspectRatio * wheight;
      }
      else
      {
         wheight = mLastCameraQuery.fov;
         wwidth = aspectRatio * wheight;
      }

      F32 hscale = wwidth * 2 / F32(getWidth());
      F32 vscale = wheight * 2 / F32(getHeight());

      F32 left = (mDragRect.point.x - getPosition().x) * hscale - wwidth;
      F32 right = (mDragRect.point.x - getPosition().x + mDragRect.extent.x) * hscale - wwidth;
      F32 top = wheight - vscale * (mDragRect.point.y - getPosition().y);
      F32 bottom = wheight - vscale * (mDragRect.point.y - getPosition().y + mDragRect.extent.y);
      gDragFrustum.set(mLastCameraQuery.ortho, left, right, top, bottom, mLastCameraQuery.nearPlane, mLastCameraQuery.farPlane, mLastCameraQuery.cameraMatrix );

      // Create the search bounds and callback data
      Box3F bounds = gDragFrustum.getBounds();
      SphereF sphere;
      sphere.center = bounds.getCenter();
      sphere.radius = (bounds.maxExtents - sphere.center).len();
      DragMeshCallbackData data(this, bounds, sphere);

      // Set up the search normal and planes
      Point3F vec;
      mLastCameraQuery.cameraMatrix.getColumn(1,&vec);
      vec.neg();
      data.mPolyList.mNormal.set(vec);
      const PlaneF* planes = gDragFrustum.getPlanes();
      for( U32 i=0; i<Frustum::PlaneCount; ++i)
      {
         data.mPolyList.mPlaneList.push_back(planes[i]);

         // Invert the planes as the poly list routines require a different
         // facing from gServerContainer.findObjects().
         data.mPolyList.mPlaneList.last().invert();
      }
      
      // If we're in first-person view currently, disable
      // hitting the control object.
      
      const bool isFirstPerson = GameConnection::getLocalClientConnection() ? GameConnection::getLocalClientConnection()->isFirstPerson() : false;
      if( isFirstPerson )
         GameConnection::getLocalClientConnection()->getControlObject()->disableCollision();
         
      // Find objects in the region.

      gServerContainer.findObjects( gDragFrustum, 0xFFFFFFFF, findDragMeshCallback, &data);
      for ( U32 i = 0; i < data.mObjects.size(); i++ )
      {
         SceneObject *obj = data.mObjects[i];
         
         // Filter out unwanted objects.
         
         if(    objClassIgnored( obj )
             || !obj->isSelectionEnabled()
             || ( obj->getTypeMask() & ( TerrainObjectType | ProjectileObjectType ) )
             || ( obj->getTypeMask() & StaticShapeObjectType && dynamic_cast< GroundPlane* >( obj ) ) )
            continue;
            
         // Add the object to the drag selection.

         mDragSelected->addObject(obj);
      }
      
      // Re-enable collision on control object when in first-person view.
      
      if( isFirstPerson )
         GameConnection::getLocalClientConnection()->getControlObject()->enableCollision();
   }
   
   // Clear the vector of onscreen icons, will populate this below
   // Necessary for performing click testing efficiently
   mIcons.clear();

   // Cull Objects and perform icon rendering
   Vector<SceneObject *> objects;
   gServerContainer.findObjects( frustum, 0xFFFFFFFF, findObjectsCallback, &objects);
   for ( U32 i = 0; i < objects.size(); i++ )
   {
      SceneObject *obj = objects[i];
      if( objClassIgnored(obj) || !obj->isSelectionEnabled() )
         continue;

      Point3F wPos;
      if ( obj->isGlobalBounds() || !mObjectsUseBoxCenter )
         obj->getTransform().getColumn(3, &wPos);
      else      
         wPos = getBoundingBoxCenter(obj);      

      Point3F sPos;
      if ( project(wPos, &sPos) )
      {
         Point2I sPosI( (S32)sPos.x,(S32)sPos.y );
         if ( !updateRect.pointInRect(sPosI) )
            continue;

         // check if object needs to be added into the regions select

         // Probably should test the entire icon screen-rect instead of just the centerpoint
         // but would need to move some code from renderScreenObj to here.
         if (mDragSelect && selection)
            if ( mDragRect.pointInRect(sPosI) && !selection->objInSet(obj) )
               mDragSelected->addObject(obj);

         //
         renderScreenObj( obj, sPos, wPos );
      }
   }

   //// Debug render rect around icons
   //for ( U32 i = 0; i < mIcons.size(); i++ )
   //   GFX->getDrawUtil()->drawRect( mIcons[i].rect, ColorI(255,255,255,255) );

   if ( mShowMousePopupInfo && mMouseDown )
      renderMousePopupInfo();

   if ( mDragSelect && mDragRect.extent.x > 1 && mDragRect.extent.y > 1 )
      GFX->getDrawUtil()->drawRect( mDragRect, mDragRectColor );

   if ( selection && selection->size() )
      mGizmo->renderText( mSaveViewport, mSaveModelview, mSaveProjection );
}

//------------------------------------------------------------------------------
// Console stuff

void WorldEditor::initPersistFields()
{
   addGroup( "Grid" );
   
      addField( "gridSnap",               TypeBool,   Offset( mGridSnap, WorldEditor ),
         "If true, transform operations will snap to the grid." );
   
   endGroup( "Grid" );
   
   addGroup( "Dropping" );
   
      addField( "dropAtBounds",           TypeBool,   Offset(mDropAtBounds, WorldEditor) );
      addField( "dropBelowCameraOffset",  TypeF32,    Offset(mDropBelowCameraOffset, WorldEditor) );
      addField( "dropAtScreenCenterScalar",TypeF32,   Offset(mDropAtScreenCenterScalar, WorldEditor) );
      addField( "dropAtScreenCenterMax",  TypeF32,    Offset(mDropAtScreenCenterMax, WorldEditor) );
      addField( "dropType",               TYPEID< DropType >(),   Offset(mDropType, WorldEditor) );   

   endGroup( "Dropping" );
   
   addGroup( "Colors" );

      addField( "popupBackgroundColor",   TypeColorI, Offset(mPopupBackgroundColor, WorldEditor) );
      addField( "popupTextColor",         TypeColorI, Offset(mPopupTextColor, WorldEditor) );
      addField( "objectTextColor",        TypeColorI, Offset(mObjectTextColor, WorldEditor) );
      addField( "selectionBoxColor",      TypeColorI, Offset(mSelectionBoxColor, WorldEditor) );
      addField( "objSelectColor",         TypeColorI, Offset(mObjSelectColor, WorldEditor) );
      addField( "objMouseOverSelectColor",TypeColorI, Offset(mObjMouseOverSelectColor, WorldEditor) );
      addField( "objMouseOverColor",      TypeColorI, Offset(mObjMouseOverColor, WorldEditor) );
      addField( "dragRectColor",          TypeColorI, Offset(mDragRectColor, WorldEditor) );
      addField( "faceSelectColor",        TypeColorI, Offset(mFaceSelectColor, WorldEditor) );
   
   endGroup( "Colors" );
   
   addGroup( "Selections" );
   
      addField( "boundingBoxCollision",   TypeBool,   Offset(mBoundingBoxCollision, WorldEditor) );
      addField( "objectMeshCollision",    TypeBool,   Offset(mObjectMeshCollision, WorldEditor) );
      addField( "selectionLocked",        TypeBool,   Offset(mSelectionLocked, WorldEditor) );   
      addProtectedField( "objectsUseBoxCenter", TypeBool, Offset(mObjectsUseBoxCenter, WorldEditor), &setObjectsUseBoxCenter, &defaultProtectedGetFn, "" );

   endGroup( "Selections" );
   
   addGroup( "Rendering" );

      addField( "objTextFormat",          TypeString, Offset(mObjTextFormat, WorldEditor) );
      addField( "renderPopupBackground",  TypeBool,   Offset(mRenderPopupBackground, WorldEditor) );
      addField( "showMousePopupInfo",     TypeBool,   Offset(mShowMousePopupInfo, WorldEditor) );
      addField( "renderObjText",          TypeBool,   Offset(mRenderObjText, WorldEditor) );
      addField( "renderObjHandle",        TypeBool,   Offset(mRenderObjHandle, WorldEditor) );
      addField( "renderSelectionBox",     TypeBool,   Offset(mRenderSelectionBox, WorldEditor) );
      addField( "selectHandle",           TypeFilename, Offset(mSelectHandle, WorldEditor) );
      addField( "defaultHandle",          TypeFilename, Offset(mDefaultHandle, WorldEditor) );
      addField( "lockedHandle",           TypeFilename, Offset(mLockedHandle, WorldEditor) );
   
   endGroup( "Rendering" );
   
   addGroup( "Rendering: Icons" );
   
      addField( "fadeIcons", TypeBool, Offset( mFadeIcons, WorldEditor ),
         "Whether object icons should fade out with distance to camera pos." );
      addField( "fadeIconsDist", TypeF32, Offset( mFadeIconsDist, WorldEditor ),
         "Distance from camera pos at which to start fading out icons." );
   
   endGroup( "Rendering: Icons" );

   addGroup( "Misc" );	

      addField( "isDirty",                TypeBool,   Offset(mIsDirty, WorldEditor) );
      addField( "stickToGround",          TypeBool,   Offset(mStickToGround, WorldEditor) );
      //addField("sameScaleAllAxis", TypeBool, Offset(mSameScaleAllAxis, WorldEditor));
      addField( "toggleIgnoreList",       TypeBool,   Offset(mToggleIgnoreList, WorldEditor) );

   endGroup( "Misc" );

   Parent::initPersistFields();
}

//------------------------------------------------------------------------------
// These methods are needed for the console interfaces.

void WorldEditor::ignoreObjClass( U32 argc, ConsoleValueRef *argv )
{
   for(S32 i = 2; i < argc; i++)
   {
      ClassInfo::Entry * entry = getClassEntry(argv[i]);
      if(entry)
         entry->mIgnoreCollision = true;
      else
      {
         entry = new ClassInfo::Entry;
         entry->mName = StringTable->insert(argv[i]);
         entry->mIgnoreCollision = true;
         if(!addClassEntry(entry))
            delete entry;
      }
   }	
}

void WorldEditor::clearIgnoreList()
{
   for(U32 i = 0; i < mClassInfo.mEntries.size(); i++)
      mClassInfo.mEntries[i]->mIgnoreCollision = false;	
}

void WorldEditor::setObjectsUseBoxCenter(bool state)
{
   mObjectsUseBoxCenter = state;
   if( getActiveSelectionSet() && isMethod( "onSelectionCentroidChanged" ) )
      Con::executef( this, "onSelectionCentroidChanged" );
}

void WorldEditor::clearSelection()
{
   if( mSelectionLocked || !mSelected )
      return;

   // Call onUnSelect.  Because of the whole treeview<->selection synchronization,
   // this may actually cause things to disappear from mSelected so do the loop
   // in reverse.  This will make the loop work even if items are removed as
   // we go along.
   for( S32 i = mSelected->size() - 1; i >= 0; -- i )
      Con::executef( this, "onUnSelect", ( *mSelected )[ i ]->getIdString() );

   Con::executef(this, "onClearSelection");
   mSelected->clear();
}

void WorldEditor::selectObject( SimObject *obj )
{
   if ( mSelectionLocked || !mSelected || !obj )
      return;

   // Don't check isSelectionEnabled of SceneObjects here as we
   // want to still allow manual selection in treeviews.

   if ( !objClassIgnored( obj ) && !mSelected->objInSet( obj ) )
   {
      mSelected->addObject( obj );	
      Con::executef( this, "onSelect", obj->getIdString() );
   }
}

void WorldEditor::selectObject( const char* obj )
{   
   SimObject *select;

   if ( Sim::findObject( obj, select ) )
      selectObject( select );
} 

void WorldEditor::unselectObject( SimObject *obj )
{
   if ( mSelectionLocked || !mSelected || !obj )
      return;

   if ( !objClassIgnored( obj ) && mSelected->objInSet( obj ) )
   {
      mSelected->removeObject( obj );	
      Con::executef( this, "onUnSelect", obj->getIdString() );
   }
}

void WorldEditor::unselectObject( const char *obj )
{
   SimObject *select;

   if ( Sim::findObject( obj, select ) )
      unselectObject( select );
}

S32 WorldEditor::getSelectionSize()
{
   if( !mSelected )
      return 0;
      
	return mSelected->size();
}

S32 WorldEditor::getSelectObject(S32 index)
{
   AssertFatal( mSelected != NULL, "WorldEditor::getSelectedObject - no active selection set!" );
   
	// Return the object's id
	return ( *mSelected )[index]->getId();	
}

const Point3F& WorldEditor::getSelectionCentroid()
{
   if( !mSelected )
      return Point3F::Zero;
      
   if( mSelected->containsGlobalBounds() )
   {
      return mSelected->getCentroid();
   }

   return mObjectsUseBoxCenter ? mSelected->getBoxCentroid() : mSelected->getCentroid();
}

const Box3F& WorldEditor::getSelectionBounds()
{
   return mSelected->getBoxBounds();
}

Point3F WorldEditor::getSelectionExtent()
{
   const Box3F& box = getSelectionBounds();
   return box.getExtents();
}

F32 WorldEditor::getSelectionRadius()
{
   const Box3F box = getSelectionBounds();
   return box.len() * 0.5f;
}

void WorldEditor::dropCurrentSelection( bool skipUndo )
{
   if ( !bool(mSelected) || !mSelected->size() )
      return;

   if ( !skipUndo )
      submitUndo( mSelected );

	dropSelection( mSelected );	

   if ( mSelected->hasCentroidChanged() )
      Con::executef( this, "onSelectionCentroidChanged" );
}

void WorldEditor::redirectConsole( S32 objID )
{
	mRedirectID = objID;		
}

//------------------------------------------------------------------------------

bool WorldEditor::alignByBounds( S32 boundsAxis )
{
   if(boundsAxis < 0 || boundsAxis > 5)
      return false;

   if(mSelected->size() < 2)
      return true;

   S32 axis = boundsAxis >= 3 ? boundsAxis-3 : boundsAxis;
   bool useMax = boundsAxis >= 3 ? false : true;
   
   // Find out which selected object has its bounds the farthest out
   F32 pos;
   S32 baseObj = 0;
   if(useMax)
      pos = TypeTraits< F32 >::MIN;
   else
      pos = TypeTraits< F32 >::MAX;

   for(S32 i=1; i<mSelected->size(); ++i)
   {
      SceneObject* object = dynamic_cast< SceneObject* >( ( *mSelected )[ i ] );
      if( !object )
         continue;
         
      const Box3F& bounds = object->getWorldBox();

      if(useMax)
      {
         if(bounds.maxExtents[axis] > pos)
         {
            pos = bounds.maxExtents[axis];
            baseObj = i;
         }
      }
      else
      {
         if(bounds.minExtents[axis] < pos)
         {
            pos = bounds.minExtents[axis];
            baseObj = i;
         }
      }
   }

   submitUndo( mSelected, "Align By Bounds" );

   // Move all selected objects to align with the calculated bounds
   for(S32 i=0; i<mSelected->size(); ++i)
   {
      if(i == baseObj)
         continue;
         
      SceneObject* object = dynamic_cast< SceneObject* >( ( *mSelected )[ i ] );
      if( !object )
         continue;

      const Box3F& bounds = object->getWorldBox();
      F32 delta;
      if(useMax)
         delta = pos - bounds.maxExtents[axis];
      else
         delta = pos - bounds.minExtents[axis];

      Point3F objPos = object->getPosition();
      objPos[axis] += delta;
      object->setPosition(objPos);
   }

   return true;
}

bool WorldEditor::alignByAxis( S32 axis )
{
   if(axis < 0 || axis > 2)
      return false;

   if(mSelected->size() < 2)
      return true;
      
   SceneObject* object = dynamic_cast< SceneObject* >( ( *mSelected )[ 0 ] );
   if( !object )
      return false;

   submitUndo( mSelected, "Align By Axis" );

   // All objects will be repositioned to line up with the
   // first selected object
   Point3F pos = object->getPosition();

   for(S32 i=0; i<mSelected->size(); ++i)
   {
      SceneObject* object = dynamic_cast< SceneObject* >( ( *mSelected )[ i ] );
      if( !object )
         continue;
         
      Point3F objPos = object->getPosition();
      objPos[axis] = pos[axis];
      object->setPosition(objPos);
   }

   return true;
}

//------------------------------------------------------------------------------

void WorldEditor::transformSelection(bool position, Point3F& p, bool relativePos, bool rotate, EulerF& r, bool relativeRot, bool rotLocal, S32 scaleType, Point3F& s, bool sRelative, bool sLocal)
{
   if(mSelected->size() == 0)
      return;

   submitUndo( mSelected, "Transform Selection" );

   if( position )
   {
      if( relativePos )
      {
         mSelected->offset( p, mGridSnap ? mGridPlaneSize : 0.f );
      }
      else
      {
         mSelected->setCentroidPosition(mObjectsUseBoxCenter, p);
      }
   }

   if( rotate )
   {
      Point3F centroid;
      if( mSelected->containsGlobalBounds() )
      {
         centroid = mSelected->getCentroid();
      }
      else
      {
         centroid = mObjectsUseBoxCenter ? mSelected->getBoxCentroid() : mSelected->getCentroid();
      }

      if( relativeRot )
      {
         if( rotLocal )
         {
            mSelected->rotate(r);
         }
         else
         {
            mSelected->rotate(r, centroid);
         }
      }
      else if( rotLocal )
      {
         // Can only do absolute rotation for multiple objects about
         // object center
         mSelected->setRotate(r);
      }
   }

   if( scaleType == 1 )
   {
      // Scale

      Point3F centroid;
      if( mSelected->containsGlobalBounds() )
      {
         centroid = mSelected->getCentroid();
      }
      else
      {
         centroid = mObjectsUseBoxCenter ? mSelected->getBoxCentroid() : mSelected->getCentroid();
      }

      if( sRelative )
      {
         if( sLocal )
         {
            mSelected->scale(s);
         }
         else
         {
            mSelected->scale(s, centroid);
         }
      }
      else
      {
         if( sLocal )
         {
            mSelected->setScale(s);
         }
         else
         {
            mSelected->setScale(s, centroid);
         }
      }
   }
   else if( scaleType == 2 )
   {
      // Size

      if( mSelected->containsGlobalBounds() )
         return;

      if( sRelative )
      {
         // Size is always local/object based
         mSelected->addSize(s);
      }
      else
      {
         // Size is always local/object based
         mSelected->setSize(s);
      }
   }

   updateClientTransforms(mSelected);

   if(mSelected->hasCentroidChanged())
   {
      Con::executef( this, "onSelectionCentroidChanged");
   }

   if ( isMethod("onEndDrag") )
   {
      SimObject * obj = 0;
      if ( mRedirectID )
         obj = Sim::findObject( mRedirectID );
      Con::executef( obj ? obj : this, "onEndDrag", ( *mSelected )[ 0 ]->getIdString() );
   }
}

//------------------------------------------------------------------------------

void WorldEditor::resetSelectedRotation()
{
   for(S32 i=0; i<mSelected->size(); ++i)
   {
      SceneObject* object = dynamic_cast< SceneObject* >( ( *mSelected )[ i ] );
      if( !object )
         continue;
         
      MatrixF mat(true);
      mat.setPosition(object->getPosition());
      object->setTransform(mat);
   }
}

void WorldEditor::resetSelectedScale()
{
   for(S32 i=0; i<mSelected->size(); ++i)
   {
      SceneObject* object = dynamic_cast< SceneObject* >( ( *mSelected )[ i ] );
      if( object )
         object->setScale(Point3F(1,1,1));
   }
}

//------------------------------------------------------------------------------

ConsoleMethod( WorldEditor, ignoreObjClass, void, 3, 0, "(string class_name, ...)")
{
	object->ignoreObjClass(argc, argv);
}

DefineEngineMethod( WorldEditor, clearIgnoreList, void, (),,
   "Clear the ignore class list.\n")
{
	object->clearIgnoreList();
}

DefineEngineMethod( WorldEditor, clearSelection, void, (),,
   "Clear the selection.\n")
{
	object->clearSelection();
}

DefineEngineMethod( WorldEditor, getActiveSelection, S32, (),,
   "Return the currently active WorldEditorSelection object.\n"
   "@return currently active WorldEditorSelection object or 0 if no selection set is available.")
{
   if( !object->getActiveSelectionSet() )
      return 0;
      
   return object->getActiveSelectionSet()->getId();
}

DefineConsoleMethod( WorldEditor, setActiveSelection, void, ( WorldEditorSelection* selection), ,
   "Set the currently active WorldEditorSelection object.\n"
   "@param	selection A WorldEditorSelectionSet object to use for the selection container.")
{
	if (selection)
   object->makeActiveSelectionSet( selection );
}

DefineEngineMethod( WorldEditor, selectObject, void, (SimObject* obj),,
   "Selects a single object."
   "@param obj	Object to select.")
{
	object->selectObject(obj);
}

DefineEngineMethod( WorldEditor, unselectObject, void, (SimObject* obj),,
   "Unselects a single object."
   "@param obj	Object to unselect.")
{
	object->unselectObject(obj);
}

DefineEngineMethod( WorldEditor, invalidateSelectionCentroid, void, (),,
   "Invalidate the selection sets centroid.")
{
   WorldEditor::Selection* sel = object->getActiveSelectionSet();
   if(sel)
	   sel->invalidateCentroid();
}

DefineEngineMethod( WorldEditor, getSelectionSize, S32, (),,
	"Return the number of objects currently selected in the editor."
	"@return number of objects currently selected in the editor.")
{
	return object->getSelectionSize();
}

DefineEngineMethod( WorldEditor, getSelectedObject, S32, (S32 index),,
	"Return the selected object and the given index."
	"@param index Index of selected object to get."
	"@return selected object at given index or -1 if index is incorrect.")
{
   if(index < 0 || index >= object->getSelectionSize())
   {
      Con::errorf(ConsoleLogEntry::General, "WorldEditor::getSelectedObject: invalid object index");
      return(-1);
   }

   return(object->getSelectObject(index));
}

DefineEngineMethod( WorldEditor, getSelectionRadius, F32, (),,
	"Get the radius of the current selection."
	"@return radius of the current selection.")
{
	return object->getSelectionRadius();
}

DefineEngineMethod( WorldEditor, getSelectionCentroid, Point3F, (),,
	"Get centroid of the selection."
	"@return centroid of the selection.")
{
	return object->getSelectionCentroid();
}

DefineEngineMethod( WorldEditor, getSelectionExtent, Point3F, (),,
	"Get extent of the selection."
	"@return extent of the selection.")
{
   return object->getSelectionExtent();
}

DefineEngineMethod( WorldEditor, dropSelection, void, (bool skipUndo), (false),
	"Drop the current selection."
	"@param skipUndo True to skip creating undo's for this action, false to create an undo.")
{

	object->dropCurrentSelection( skipUndo );
}

void WorldEditor::cutCurrentSelection()
{
	cutSelection(mSelected);	
}

void WorldEditor::copyCurrentSelection()
{
	copySelection(mSelected);	
}

DefineEngineMethod( WorldEditor, cutSelection, void, (), ,
	"Cut the current selection to be pasted later.")
{
   object->cutCurrentSelection();
}

DefineEngineMethod( WorldEditor, copySelection, void, (), ,
	"Copy the current selection to be pasted later.")
{
   object->copyCurrentSelection();
}

DefineEngineMethod( WorldEditor, pasteSelection, void, (), ,
	"Paste the current selection.")
{
   object->pasteSelection();
}

bool WorldEditor::canPasteSelection()
{
	return mCopyBuffer.empty() != true;
}

DefineEngineMethod( WorldEditor, canPasteSelection, bool, (), ,
	"Check if we can paste the current selection."
	"@return True if we can paste the current selection, false if not.")
{
	return object->canPasteSelection();
}

DefineEngineMethod( WorldEditor, hideObject, void, (SceneObject* obj, bool hide), ,
	"Hide/show the given object."
	"@param obj	Object to hide/show."
	"@param hide True to hide the object, false to show it.")
{

	if (obj)
   object->hideObject(obj, hide);
}

DefineEngineMethod( WorldEditor, hideSelection, void, (bool hide), ,
	"Hide/show the selection."
	"@param hide True to hide the selection, false to show it.")
{
   object->hideSelection(hide);
}

DefineEngineMethod( WorldEditor, lockSelection, void, (bool lock), ,
	"Lock/unlock the selection."
	"@param lock True to lock the selection, false to unlock it.")
{
   object->lockSelection(lock);
}

//TODO: Put in the param possible options and what they mean
DefineEngineMethod( WorldEditor, alignByBounds, void, (S32 boundsAxis), ,
	"Align all selected objects against the given bounds axis."
	"@param boundsAxis Bounds axis to align all selected objects against.")
{
	if(!object->alignByBounds(boundsAxis))
		Con::warnf(ConsoleLogEntry::General, avar("worldEditor.alignByBounds: invalid bounds axis '%s'", boundsAxis));
}

//TODO: Put in the param possible options and what they mean (assuming x,y,z)
DefineEngineMethod( WorldEditor, alignByAxis, void, (S32 axis), ,
	"Align all selected objects along the given axis."
	"@param axis Axis to align all selected objects along.")
{
	if(!object->alignByAxis(axis))
		Con::warnf(ConsoleLogEntry::General, avar("worldEditor.alignByAxis: invalid axis '%s'", axis));
}

DefineEngineMethod( WorldEditor, resetSelectedRotation, void, (), ,
	"Reset the rotation of the selection.")
{
	object->resetSelectedRotation();
}

DefineEngineMethod( WorldEditor, resetSelectedScale, void, (), ,
	"Reset the scale of the selection.")
{
	object->resetSelectedScale();
}

//TODO: Better documentation on exactly what this does.
DefineEngineMethod( WorldEditor, redirectConsole, void, (S32 objID), ,
	"Redirect console."
	"@param objID Object id.")
{
   object->redirectConsole(objID);
}

DefineEngineMethod( WorldEditor, addUndoState, void, (), ,
	"Adds/Submits an undo state to the undo manager.")
{
	object->addUndoState();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( WorldEditor, getSoftSnap, bool, (), ,
	"Is soft snapping always on?"
	"@return True if soft snap is on, false if not.")
{
	return object->mSoftSnap;
}

DefineEngineMethod( WorldEditor, setSoftSnap, void, (bool softSnap), ,
	"Allow soft snapping all of the time."
	"@param softSnap True to turn soft snap on, false to turn it off.")
{
	object->mSoftSnap = softSnap;
}

DefineEngineMethod( WorldEditor, getSoftSnapSize, F32, (), ,
	"Get the absolute size to trigger a soft snap."
	"@return absolute size to trigger a soft snap.")
{
	return object->mSoftSnapSize;
}

DefineEngineMethod( WorldEditor, setSoftSnapSize, void, (F32 size), ,
	"Set the absolute size to trigger a soft snap."
	"@param size Absolute size to trigger a soft snap.")
{
	object->mSoftSnapSize = size;
}

DefineEngineMethod( WorldEditor, getSoftSnapAlignment, WorldEditor::AlignmentType, (),,
	"Get the soft snap alignment."
	"@return soft snap alignment.")
{
   return object->mSoftSnapAlignment;
}

DefineEngineMethod( WorldEditor, setSoftSnapAlignment, void, ( WorldEditor::AlignmentType type ),,
	"Set the soft snap alignment."
	"@param type Soft snap alignment type.")
{
   object->mSoftSnapAlignment = type;
}

DefineEngineMethod( WorldEditor, softSnapSizeByBounds, void, (bool useBounds), ,
	"Use selection bounds size as soft snap bounds."
	"@param useBounds True to use selection bounds size as soft snap bounds, false to not.")
{
	object->mSoftSnapSizeByBounds = useBounds;
}

DefineEngineMethod( WorldEditor, getSoftSnapBackfaceTolerance, F32, (),,
	"Get the fraction of the soft snap radius that backfaces may be included."
	"@return fraction of the soft snap radius that backfaces may be included.")
{
	return object->mSoftSnapBackfaceTolerance;
}

DefineEngineMethod( WorldEditor, setSoftSnapBackfaceTolerance, void, (F32 tolerance),,
	"Set the fraction of the soft snap radius that backfaces may be included."
	"@param tolerance Fraction of the soft snap radius that backfaces may be included (range of 0..1).")
{
	object->mSoftSnapBackfaceTolerance = tolerance;
}

DefineEngineMethod( WorldEditor, softSnapRender, void, (F32 render),,
	"Render the soft snapping bounds."
	"@param render True to render the soft snapping bounds, false to not.")
{
	object->mSoftSnapRender = render;
}

DefineEngineMethod( WorldEditor, softSnapRenderTriangle, void, (F32 renderTriangle),,
	"Render the soft snapped triangle."
	"@param renderTriangle True to render the soft snapped triangle, false to not.")
{
	object->mSoftSnapRenderTriangle = renderTriangle;
}

DefineEngineMethod( WorldEditor, softSnapDebugRender, void, (F32 debugRender),,
	"Toggle soft snapping debug rendering."
	"@param debugRender True to turn on soft snapping debug rendering, false to turn it off.")
{
	object->mSoftSnapDebugRender = debugRender;
}

DefineEngineMethod( WorldEditor, getTerrainSnapAlignment, WorldEditor::AlignmentType, (),,
   "Get the terrain snap alignment."
   "@return terrain snap alignment type.")
{
   return object->mTerrainSnapAlignment;
}

DefineEngineMethod( WorldEditor, setTerrainSnapAlignment, void, ( WorldEditor::AlignmentType alignment ),,
   "Set the terrain snap alignment."
   "@param alignment New terrain snap alignment type.")
{
   object->mTerrainSnapAlignment = alignment;
}

DefineEngineMethod( WorldEditor, transformSelection, void, 
                   ( bool position,
                     Point3F point,
                     bool relativePos,
                     bool rotate,
                     Point3F rotation,
                     bool relativeRot,
                     bool rotLocal,
                     S32 scaleType,
                     Point3F scale,
                     bool sRelative,
                     bool sLocal ), ,
   "Transform selection by given parameters."
   "@param position True to transform the selection's position."
   "@param point Position to transform by."
   "@param relativePos True to use relative position."
   "@param rotate True to transform the selection's rotation."
   "@param rotation Rotation to transform by."
   "@param relativeRot True to use the relative rotation."
   "@param rotLocal True to use the local rotation."
   "@param scaleType Scale type to use."
   "@param scale Scale to transform by."
   "@param sRelative True to use a relative scale."
   "@param sLocal True to use a local scale.")
{
   object->transformSelection(position, point, relativePos, rotate, rotation, relativeRot, rotLocal, scaleType, scale, sRelative, sLocal);
}

#include "core/strings/stringUnit.h"
#include "collision/optimizedPolyList.h"
#include "core/volume.h"
#ifdef TORQUE_COLLADA
   #include "ts/collada/colladaUtils.h"
#endif

void WorldEditor::colladaExportSelection( const String &path )
{
#ifdef TORQUE_COLLADA
   
   Vector< SceneObject* > objectList;

   for ( S32 i = 0; i < mSelected->size(); i++ )
   {
      SceneObject *pObj = dynamic_cast< SceneObject* >( ( *mSelected )[i] );
      if ( pObj )
         objectList.push_back( pObj );
   }

   if ( objectList.empty() )
      return;

   Point3F centroid;
   MatrixF orientation;

   if ( objectList.size() == 1 )
   {
      orientation = objectList[0]->getTransform();
      centroid = objectList[0]->getPosition();
   }
   else
   {
      orientation.identity();
      centroid.zero();

      S32 count = 0;

      for ( S32 i = 0; i < objectList.size(); i++ )      
      {
         SceneObject *pObj = objectList[i];
         if ( pObj->isGlobalBounds() )
            continue;

         centroid += pObj->getPosition();
         count++;
      }
            
      centroid /= count;
   }

   orientation.setPosition( centroid );
   orientation.inverse();

   OptimizedPolyList polyList;
   polyList.setBaseTransform( orientation );

   for ( S32 i = 0; i < objectList.size(); i++ )
   {
      SceneObject *pObj = objectList[i];
      if ( !pObj->buildPolyList( PLC_Export, &polyList, pObj->getWorldBox(), pObj->getWorldSphere() ) )      
         Con::warnf( "colladaExportObjectList() - object %i returned no geometry.", pObj->getId() );               
   }

   // Use a ColladaUtils function to do the actual export to a Collada file
   ColladaUtils::exportToCollada( path, polyList );

#endif
}

DefineEngineMethod( WorldEditor, colladaExportSelection, void, ( const char* path ),,
	"Export the combined geometry of all selected objects to the specified path in collada format."
	"@param path Path to export collada format to.")
{
   object->colladaExportSelection( path );
}

void WorldEditor::makeSelectionPrefab( const char *filename )
{
   if ( mSelected->size() == 0 )
   {
      Con::errorf( "WorldEditor::makeSelectionPrefab - Nothing selected." );
      return;
   }

   SimGroup *missionGroup;
   if ( !Sim::findObject( "MissionGroup", missionGroup ) )
   {
      Con::errorf( "WorldEditor::makeSelectionPrefab - Could not find MissionGroup." );
      return;
   }

   Vector< SimObject* > stack;
   Vector< SimObject* > found;

   for ( S32 i = 0; i < mSelected->size(); i++ )
   {
      SimObject *obj = ( *mSelected )[i];
      stack.push_back( obj );      
   }

   Vector< SimGroup* > cleanup;

   while ( !stack.empty() )
   {
      SimObject *obj = stack.last();
      SimGroup *grp = dynamic_cast< SimGroup* >( obj );      

      stack.pop_back();

      if ( grp )
      {
         for ( S32 i = 0; i < grp->size(); i++ )
            stack.push_back( grp->at(i) );
         
         SceneObject* scn = dynamic_cast< SceneObject* >(grp);
         if (scn)
         {
            if (Prefab::isValidChild(obj, true))
               found.push_back(obj);
         }
         else
         {
            //Only push the cleanup of the group if it's ONLY a SimGroup.
            cleanup.push_back(grp);
         }
      }
      else
      {
         if ( Prefab::isValidChild( obj, true ) )
            found.push_back( obj );
      }
   }

   if ( found.empty() )
   {
      Con::warnf( "WorldEditor::makeSelectionPrefab - No valid objects selected." );      
      return;
   }
   
   // SimGroup we collect prefab objects into.
   SimGroup *group = new SimGroup();
   group->registerObject();

   // Transform from World to Prefab space.
   MatrixF fabMat(true);
   fabMat.setPosition( mSelected->getCentroid() );
   fabMat.inverse();

   MatrixF objMat;
   SimObject *obj = NULL;
   SceneObject *sObj = NULL;

   for ( S32 i = 0; i < found.size(); i++ )
   {      
      obj = found[i];
      sObj = dynamic_cast< SceneObject* >( obj );

      obj->assignName( "" );

      if ( sObj )
      {         
         objMat.mul( fabMat, sObj->getTransform() );
         sObj->setTransform( objMat );
      }

      group->addObject( obj );         
   }
   
   // Save out .prefab file.
   group->save( filename, false, "$ThisPrefab = " ); 

   // Allocate Prefab object and add to level.
   Prefab *fab = new Prefab();
   fab->setFile( filename );
   fabMat.inverse();
   fab->setTransform( fabMat );
   fab->registerObject();
   missionGroup->addObject( fab );

   // Select it, mark level as dirty.
   clearSelection();
   selectObject( fab );
   setDirty();      

   // Delete original objects and temporary SimGroup.
   group->deleteObject();
   for ( S32 i = 0; i < cleanup.size(); i++ )
      cleanup[i]->deleteObject();
}

void WorldEditor::explodeSelectedPrefab()
{
   Vector<Prefab*> prefabList;

   for ( S32 i = 0; i < mSelected->size(); i++ )
   {
      Prefab *obj = dynamic_cast<Prefab*>( ( *mSelected )[i] );
      if ( obj )
         prefabList.push_back(obj);
   }

   if ( prefabList.empty() )
      return;

   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      Con::errorf( "WorldEditor::createUndo() - EUndoManager not found!" );
      return;           
   }

   CompoundUndoAction *action = new CompoundUndoAction("Explode Prefab");

   clearSelection();

   for ( S32 i = 0; i < prefabList.size(); i++ )   
   {
      Prefab *prefab = prefabList[i];      

      ExplodePrefabUndoAction *explodeAction = new ExplodePrefabUndoAction(prefab);
      action->addAction( explodeAction );      

      selectObject( explodeAction->mGroup );

      MEDeleteUndoAction *delAction = new MEDeleteUndoAction();
      delAction->deleteObject( prefab );

      action->addAction( delAction );
   }

   undoMan->addAction( action );   

   setDirty();
}

void WorldEditor::makeSelectionAMesh(const char *filename)
{
   if (mSelected->size() == 0)
   {
      Con::errorf("WorldEditor::makeSelectionAMesh - Nothing selected.");
      return;
   }

   SimGroup *missionGroup;
   if (!Sim::findObject("MissionGroup", missionGroup))
   {
      Con::errorf("WorldEditor::makeSelectionAMesh - Could not find MissionGroup.");
      return;
   }

   Vector< SimObject* > stack;
   Vector< SimObject* > found;

   for (S32 i = 0; i < mSelected->size(); i++)
   {
      SimObject *obj = (*mSelected)[i];
      stack.push_back(obj);
   }

   Vector< SimGroup* > cleanup;

   while (!stack.empty())
   {
      SimObject *obj = stack.last();
      SimGroup *grp = dynamic_cast< SimGroup* >(obj);

      stack.pop_back();

      if (grp)
      {
         for (S32 i = 0; i < grp->size(); i++)
            stack.push_back(grp->at(i));

         SceneObject* scn = dynamic_cast< SceneObject* >(grp);
         if (scn)
         {
            if (Prefab::isValidChild(obj, true))
               found.push_back(obj);
         }
         else
         {
            //Only push the cleanup of the group if it's ONLY a SimGroup.
            cleanup.push_back(grp);
         }
      }
      else
      {
         if (Prefab::isValidChild(obj, true))
            found.push_back(obj);
      }
   }

   if (found.empty())
   {
      Con::warnf("WorldEditor::makeSelectionPrefab - No valid objects selected.");
      return;
   }

   // SimGroup we collect prefab objects into.
   SimGroup *group = new SimGroup();
   group->registerObject();

   // Transform from World to Prefab space.
   MatrixF fabMat(true);
   fabMat.setPosition(mSelected->getCentroid());
   fabMat.inverse();

   MatrixF objMat;
   SimObject *obj = NULL;
   SceneObject *sObj = NULL;

   Vector< SceneObject* > objectList;

   for ( S32 i = 0; i < mSelected->size(); i++ )
   {
      SceneObject *pObj = dynamic_cast< SceneObject* >( ( *mSelected )[i] );
      if ( pObj )
         objectList.push_back( pObj );
   }

   if ( objectList.empty() )
      return;

   //
   Point3F centroid;
   MatrixF orientation;

   if (objectList.size() == 1)
   {
      orientation = objectList[0]->getTransform();
      centroid = objectList[0]->getPosition();
   }
   else
   {
      orientation.identity();
      centroid.zero();

      S32 count = 0;

      for (S32 i = 0; i < objectList.size(); i++)
      {
         SceneObject *pObj = objectList[i];
         if (pObj->isGlobalBounds())
            continue;

         centroid += pObj->getPosition();
         count++;
      }

      centroid /= count;
   }

   orientation.setPosition(centroid);
   orientation.inverse();

   OptimizedPolyList polyList;
   polyList.setBaseTransform(orientation);

   for (S32 i = 0; i < objectList.size(); i++)
   {
      SceneObject *pObj = objectList[i];
      if (!pObj->buildPolyList(PLC_Export, &polyList, pObj->getWorldBox(), pObj->getWorldSphere()))
         Con::warnf("colladaExportObjectList() - object %i returned no geometry.", pObj->getId());
   }

   // Use a ColladaUtils function to do the actual export to a Collada file
   ColladaUtils::exportToCollada(filename, polyList);
   //

   // Allocate TSStatic object and add to level.
   TSStatic *ts = new TSStatic();
   ts->setShapeFileName(StringTable->insert(filename));
   fabMat.inverse();
   ts->setTransform(fabMat);
   ts->registerObject();
   missionGroup->addObject(ts);

   // Select it, mark level as dirty.
   clearSelection();
   selectObject(ts);
   setDirty();

   // Delete original objects and temporary SimGroup.
   for (S32 i = 0; i < objectList.size(); i++)
      objectList[i]->deleteObject();
}

DefineEngineMethod( WorldEditor, makeSelectionPrefab, void, ( const char* filename ),,
	"Save selected objects to a .prefab file and replace them in the level with a Prefab object."
	"@param filename Prefab file to save the selected objects to.")
{
   object->makeSelectionPrefab( filename );
}

DefineEngineMethod( WorldEditor, explodeSelectedPrefab, void, (),,
	"Replace selected Prefab objects with a SimGroup containing all children objects defined in the .prefab.")
{
   object->explodeSelectedPrefab();
}

DefineEngineMethod(WorldEditor, makeSelectionAMesh, void, (const char* filename), ,
   "Save selected objects to a .dae collada file and replace them in the level with a TSStatic object."
   "@param filename collada file to save the selected objects to.")
{
   object->makeSelectionAMesh(filename);
}

DefineEngineMethod( WorldEditor, mountRelative, void, ( SceneObject *objA, SceneObject *objB ),,
	"Mount object B relatively to object A."
	"@param objA Object to mount to."
	"@param objB Object to mount.")
{
	if (!objA || !objB)
		return;

   MatrixF xfm = objB->getTransform();   
   MatrixF mat = objA->getWorldTransform();
   xfm.mul( mat );
   
   Point3F pos = objB->getPosition();
   MatrixF temp = objA->getTransform();
   temp.scale( objA->getScale() );
   temp.inverse();
   temp.mulP( pos );
   
   xfm.setPosition( pos );
   

   objA->mountObject( objB, -1, xfm );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( WorldEditor, createPolyhedralObject, SceneObject*, ( const char* className, SceneObject* geometryProvider ),,
   "Grab the geometry from @a geometryProvider, create a @a className object, and assign it the extracted geometry." )
{
   if( !geometryProvider )
   {
      Con::errorf( "WorldEditor::createPolyhedralObject - Invalid geometry provider!" );
      return NULL;
   }

   if( !className || !className[ 0 ] )
   {
      Con::errorf( "WorldEditor::createPolyhedralObject - Invalid class name" );
      return NULL;
   }

   AbstractClassRep* classRep = AbstractClassRep::findClassRep( className );
   if( !classRep )
   {
      Con::errorf( "WorldEditor::createPolyhedralObject - No such class: %s", className );
      return NULL;
   }

   // We don't want the extracted poly list to be affected by the object's
   // current transform and scale so temporarily reset them.

   MatrixF savedTransform = geometryProvider->getTransform();
   Point3F savedScale = geometryProvider->getScale();

   geometryProvider->setTransform( MatrixF::Identity );
   geometryProvider->setScale( Point3F( 1.f, 1.f, 1.f ) );

   // Extract the geometry.  Use the object-space bounding volumes
   // as we have moved the object to the origin for the moment.

   OptimizedPolyList polyList;
   if( !geometryProvider->buildPolyList( PLC_Export, &polyList, geometryProvider->getObjBox(), geometryProvider->getObjBox().getBoundingSphere() ) )
   {
      Con::errorf( "WorldEditor::createPolyhedralObject - Failed to extract geometry!" );
      return NULL;
   }

   // Restore the object's original transform.

   geometryProvider->setTransform( savedTransform );
   geometryProvider->setScale( savedScale );

   // Create the object.

   SceneObject* object = dynamic_cast< SceneObject* >( classRep->create() );
   if( !Object )
   {
      Con::errorf( "WorldEditor::createPolyhedralObject - Could not create SceneObject with class '%s'", className );
      return NULL;
   }

   // Convert the polylist to a polyhedron.

   Polyhedron polyhedron = polyList.toPolyhedron();

   // Add the vertex data.

   const U32 numPoints = polyhedron.getNumPoints();
   const Point3F* points = polyhedron.getPoints();

   for( U32 i = 0; i < numPoints; ++ i )
   {
      static StringTableEntry sPoint = StringTable->insert( "point" );
      object->setDataField( sPoint, NULL, EngineMarshallData( points[ i ] ) );
   }

   // Add the plane data.

   const U32 numPlanes = polyhedron.getNumPlanes();
   const PlaneF* planes = polyhedron.getPlanes();

   for( U32 i = 0; i < numPlanes; ++ i )
   {
      static StringTableEntry sPlane = StringTable->insert( "plane" );
      const PlaneF& plane = planes[ i ];

      char buffer[ 1024 ];
      dSprintf( buffer, sizeof( buffer ), "%g %g %g %g", plane.x, plane.y, plane.z, plane.d );

      object->setDataField( sPlane, NULL, buffer );
   }

   // Add the edge data.

   const U32 numEdges = polyhedron.getNumEdges();
   const Polyhedron::Edge* edges = polyhedron.getEdges();

   for( U32 i = 0; i < numEdges; ++ i )
   {
      static StringTableEntry sEdge = StringTable->insert( "edge" );
      const Polyhedron::Edge& edge = edges[ i ];

      char buffer[ 1024 ];
      dSprintf( buffer, sizeof( buffer ), "%i %i %i %i ",
         edge.face[ 0 ], edge.face[ 1 ],
         edge.vertex[ 0 ], edge.vertex[ 1 ]
      );

      object->setDataField( sEdge, NULL, buffer );
   }

   // Set the transform.

   object->setTransform( savedTransform );
   object->setScale( savedScale );

   // Register and return the object.

   if( !object->registerObject() )
   {
      Con::errorf( "WorldEditor::createPolyhedralObject - Failed to register object!" );
      delete object;
      return NULL;
   }

   return object;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( WorldEditor, createConvexShapeFrom, ConvexShape*, ( SceneObject* polyObject ),,
   "Create a ConvexShape from the given polyhedral object." )
{
   if( !polyObject )
   {
      Con::errorf( "WorldEditor::createConvexShapeFrom - Invalid object" );
      return NULL;
   }

   IScenePolyhedralObject* iPoly = dynamic_cast< IScenePolyhedralObject* >( polyObject );
   if( !iPoly )
   {
      Con::errorf( "WorldEditor::createConvexShapeFrom - Not a polyhedral object!" );
      return NULL;
   }

   // Get polyhedron.

   AnyPolyhedron polyhedron = iPoly->ToAnyPolyhedron();
   const U32 numPlanes = polyhedron.getNumPlanes();
   if( !numPlanes )
   {
      Con::errorf( "WorldEditor::createConvexShapeFrom - Object returned no valid polyhedron" );
      return NULL;
   }

   // Create a ConvexShape.

   ConvexShape* shape = new ConvexShape();

   // Add all planes.

   for( U32 i = 0; i < numPlanes; ++ i )
   {
      const PlaneF& plane = polyhedron.getPlanes()[ i ];

      // Polyhedron planes are facing inwards so we need to
      // invert the normal here.

      Point3F normal = plane.getNormal();
      normal.neg();

      // Turn the orientation of the plane into a quaternion.
      // The normal is our up vector (that's what's expected
      // by ConvexShape for the surface orientation).

      MatrixF orientation( true );
      MathUtils::getMatrixFromUpVector( normal, &orientation );
      const QuatF quat( orientation );

      // Get the plane position.

      const Point3F position = plane.getPosition();

      // Turn everything into a "surface" property for the ConvexShape.

      char buffer[ 1024 ];
      dSprintf( buffer, sizeof( buffer ), "%g %g %g %g %g %g %g",
         quat.x, quat.y, quat.z, quat.w,
         position.x, position.y, position.z
      );

      // Add the surface.

      static StringTableEntry sSurface = StringTable->insert( "surface" );
      shape->setDataField( sSurface, NULL, buffer );
   }

   // Copy the transform.

   shape->setTransform( polyObject->getTransform() );
   shape->setScale( polyObject->getScale() );

   // Register the shape.

   if( !shape->registerObject() )
   {
      Con::errorf( "WorldEditor::createConvexShapeFrom - Could not register ConvexShape!" );
      delete shape;
      return NULL;
   }

   return shape;
}
