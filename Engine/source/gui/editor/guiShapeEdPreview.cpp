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

#include "console/consoleTypes.h"
#include "console/console.h"
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "gui/editor/guiShapeEdPreview.h"
#include "renderInstance/renderPassManager.h"
#include "lighting/lightManager.h"
#include "lighting/lightInfo.h"
#include "core/resourceManager.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "collision/concretePolyList.h"

#ifdef TORQUE_COLLADA
   #include "collision/optimizedPolyList.h"
   #include "ts/collada/colladaUtils.h"
#endif

static const F32 sMoveScaler = 50.0f;
static const F32 sZoomScaler = 200.0f;
static const int sNodeRectSize = 16;

IMPLEMENT_CONOBJECT( GuiShapeEdPreview );

ConsoleDocClass( GuiShapeEdPreview,
   "@brief This control provides the 3D view for the Shape Editor tool, and is "
   "not intended for general purpose use.\n"

   "@ingroup GuiControls\n"
   "@internal"
);

IMPLEMENT_CALLBACK( GuiShapeEdPreview, onThreadPosChanged, void, ( F32 pos, bool inTransition ), ( pos, inTransition),
   "Called when the position of the active thread has changed, such as during "
   "playback." );


GuiShapeEdPreview::GuiShapeEdPreview()
:  mOrbitDist( 5.0f ),
   mMoveSpeed ( 1.0f ),
   mZoomSpeed ( 1.0f ),
   mModel( NULL ),
   mGridDimension( 30, 30 ),
   mRenderGhost( false ),
   mRenderNodes( false ),
   mRenderBounds( false ),
   mRenderObjBox( false ),
   mRenderMounts( true ),
   mSunDiffuseColor( 255, 255, 255, 255 ),
   mSunAmbientColor( 140, 140, 140, 255 ),
   mSelectedNode( -1 ),
   mHoverNode( -1 ),
   mSelectedObject( -1 ),
   mSelectedObjDetail( 0 ),
   mUsingAxisGizmo( false ),
   mGizmoDragID( 0 ),
   mEditingSun( false ),
   mTimeScale( 1.0f ),
   mActiveThread( -1 ),
   mLastRenderTime( 0 ),
   mFakeSun( NULL ),
   mSunRot( 45.0f, 0, 135.0f ),
   mCameraRot( 0, 0, 3.9f ),
   mRenderCameraAxes( false ),
   mOrbitPos( 0, 0, 0 ),
   mFixedDetail( true ),
   mCurrentDL( 0 ),
   mDetailSize( 0 ),
   mDetailPolys( 0 ),
   mPixelSize( 0 ),
   mNumMaterials( 0 ),
   mNumDrawCalls( 0 ),
   mNumBones( 0 ),
   mNumWeights( 0 ),
   mColMeshes( 0 ),
   mColPolys( 0 )
{
   mActive = true;

   // By default don't do dynamic reflection
   // updates for this viewport.
   mReflectPriority = 0.0f;
}

GuiShapeEdPreview::~GuiShapeEdPreview()
{
   SAFE_DELETE( mModel );
   SAFE_DELETE( mFakeSun );
}

void GuiShapeEdPreview::initPersistFields()
{
   addGroup( "Rendering" );
   addField( "editSun",        TypeBool,         Offset( mEditingSun, GuiShapeEdPreview ),
      "If true, dragging the gizmo will rotate the sun direction" );
   addField( "selectedNode",   TypeS32,          Offset( mSelectedNode, GuiShapeEdPreview ),
      "Index of the selected node, or -1 if none" );
   addField( "selectedObject", TypeS32,          Offset( mSelectedObject, GuiShapeEdPreview ),
      "Index of the selected object, or -1 if none" );
   addField( "selectedObjDetail", TypeS32,       Offset( mSelectedObjDetail, GuiShapeEdPreview ),
      "Index of the selected object detail mesh, or 0 if none" );
   addField( "gridDimension",  TypePoint2I,      Offset( mGridDimension, GuiShapeEdPreview ),
      "Grid dimensions (number of rows and columns) in the form \"rows cols\"" );
   addField( "renderGrid",     TypeBool,         Offset( mRenderGridPlane, EditTSCtrl ),
      "Flag indicating whether to draw the grid" );
   addField( "renderNodes",    TypeBool,         Offset( mRenderNodes, GuiShapeEdPreview ),
      "Flag indicating whether to render the shape nodes" );
   addField( "renderGhost",    TypeBool,         Offset( mRenderGhost, GuiShapeEdPreview ),
      "Flag indicating whether to render the shape in 'ghost' mode (transparent)" );
   addField( "renderBounds",   TypeBool,         Offset( mRenderBounds, GuiShapeEdPreview ),
      "Flag indicating whether to render the shape bounding box" );
   addField( "renderObjBox",   TypeBool,         Offset( mRenderObjBox, GuiShapeEdPreview ),
      "Flag indicating whether to render the selected object's bounding box" );
   addField( "renderColMeshes", TypeBool,        Offset( mRenderColMeshes, GuiShapeEdPreview ),
      "Flag indicating whether to render the shape's collision geometry" );
   addField( "renderMounts",   TypeBool,         Offset( mRenderMounts, GuiShapeEdPreview ),
      "Flag indicating whether to render mounted objects" );
   endGroup( "Rendering" );

   addGroup( "Sun" );
   addProtectedField( "sunDiffuse", TypeColorI, Offset( mSunDiffuseColor, GuiShapeEdPreview ), &setFieldSunDiffuse, &defaultProtectedGetFn,
      "Ambient color for the sun" );
   addProtectedField( "sunAmbient", TypeColorI, Offset( mSunAmbientColor, GuiShapeEdPreview ), &setFieldSunAmbient, &defaultProtectedGetFn,
      "Diffuse color for the sun" );
   addProtectedField( "sunAngleX",  TypeF32,    Offset( mSunRot.x, GuiShapeEdPreview ), &setFieldSunAngleX, &defaultProtectedGetFn,
      "X-axis rotation angle for the sun" );
   addProtectedField( "sunAngleZ",  TypeF32,    Offset( mSunRot.z, GuiShapeEdPreview ), &setFieldSunAngleZ, &defaultProtectedGetFn,
      "Z-axis rotation angle for the sun" );
   endGroup( "Sun" );

   addGroup( "Animation" );
   addField( "activeThread",              TypeS32,    Offset( mActiveThread, GuiShapeEdPreview ),
      "Index of the active thread, or -1 if none" );
   addProtectedField( "threadPos",        TypeF32,    NULL, &setFieldThreadPos, &getFieldThreadPos,
      "Current position of the active thread (0-1)" );
   addProtectedField( "threadDirection",  TypeS32,    NULL, &setFieldThreadDir, &getFieldThreadDir,
      "Playback direction of the active thread" );
   addProtectedField( "threadPingPong",   TypeBool,   NULL, &setFieldThreadPingPong, &getFieldThreadPingPong,
      "'PingPong' mode of the active thread" );
   endGroup( "Animation" );

   addGroup( "Detail Stats" );
   addField( "fixedDetail",            TypeBool, Offset( mFixedDetail, GuiShapeEdPreview ),
      "If false, the current detail is selected based on camera distance" );
   addField( "orbitDist",              TypeF32, Offset( mOrbitDist, GuiShapeEdPreview ),
      "The current distance from the camera to the model" );
   addProtectedField( "currentDL",     TypeS32, Offset( mCurrentDL, GuiShapeEdPreview ),     &setFieldCurrentDL,     &defaultProtectedGetFn,
      "The current detail level" );
   addProtectedField( "detailSize",    TypeS32, Offset( mDetailSize, GuiShapeEdPreview ),    &defaultProtectedSetFn, &defaultProtectedGetFn,
      "The size of the current detail" );
   addProtectedField( "detailPolys",   TypeS32, Offset( mDetailPolys, GuiShapeEdPreview ),   &defaultProtectedSetFn, &defaultProtectedGetFn,
      "Number of polygons in the current detail" );
   addProtectedField( "pixelSize",     TypeF32, Offset( mPixelSize, GuiShapeEdPreview ),     &defaultProtectedSetFn, &defaultProtectedGetFn,
      "The current pixel size of the model" );
   addProtectedField( "numMaterials",  TypeS32, Offset( mNumMaterials, GuiShapeEdPreview ),  &defaultProtectedSetFn, &defaultProtectedGetFn,
      "The number of materials in the current detail level" );
   addProtectedField( "numDrawCalls",  TypeS32, Offset( mNumDrawCalls, GuiShapeEdPreview ),  &defaultProtectedSetFn, &defaultProtectedGetFn,
      "The number of draw calls in the current detail level" );
   addProtectedField( "numBones",      TypeS32, Offset( mNumBones, GuiShapeEdPreview ),      &defaultProtectedSetFn, &defaultProtectedGetFn,
      "The number of bones in the current detail level (skins only)" );
   addProtectedField( "numWeights",    TypeS32, Offset( mNumWeights, GuiShapeEdPreview ),    &defaultProtectedSetFn, &defaultProtectedGetFn,
      "The number of vertex weights in the current detail level (skins only)" );
   addProtectedField( "colMeshes",     TypeS32, Offset( mColMeshes, GuiShapeEdPreview ),     &defaultProtectedSetFn, &defaultProtectedGetFn,
      "The number of collision meshes in the shape" );
   addProtectedField( "colPolys",      TypeS32, Offset( mColPolys, GuiShapeEdPreview ),      &defaultProtectedSetFn, &defaultProtectedGetFn,
      "The total number of collision polygons (all meshes) in the shape" );
   endGroup( "Detail Stats" );

   Parent::initPersistFields();
}

bool GuiShapeEdPreview::setFieldCurrentDL( void *object, const char *index, const char *data )
{
   GuiShapeEdPreview* gui = static_cast<GuiShapeEdPreview*>( object );
   if ( gui )
      gui->setCurrentDetail( mFloor( dAtof( data ) + 0.5f ) );
   return false;
}

bool GuiShapeEdPreview::setFieldSunDiffuse( void *object, const char *index, const char *data )
{
   GuiShapeEdPreview* gui = static_cast<GuiShapeEdPreview*>( object );
   if ( gui )
   {
      Con::setData( TypeColorI, &gui->mSunDiffuseColor, 0, 1, &data );
      gui->updateSun();
   }
   return false;
}

bool GuiShapeEdPreview::setFieldSunAmbient( void *object, const char *index, const char *data )
{
   GuiShapeEdPreview* gui = static_cast<GuiShapeEdPreview*>( object );
   if ( gui )
   {
      Con::setData( TypeColorI, &gui->mSunAmbientColor, 0, 1, &data );
      gui->updateSun();
   }
   return false;
}

bool GuiShapeEdPreview::setFieldSunAngleX( void *object, const char *index, const char *data )
{
   GuiShapeEdPreview* gui = static_cast<GuiShapeEdPreview*>( object );
   if ( gui )
   {
      Con::setData( TypeF32, &gui->mSunRot.x, 0, 1, &data );
      gui->updateSun();
   }
   return false;
}

bool GuiShapeEdPreview::setFieldSunAngleZ( void *object, const char *index, const char *data )
{
   GuiShapeEdPreview* gui = static_cast<GuiShapeEdPreview*>( object );
   if ( gui )
   {
      Con::setData( TypeF32, &gui->mSunRot.z, 0, 1, &data );
      gui->updateSun();
   }
   return false;
}

bool GuiShapeEdPreview::setFieldThreadPos( void *object, const char *index, const char *data )
{
   GuiShapeEdPreview* gui = static_cast<GuiShapeEdPreview*>( object );
   if ( gui && ( gui->mActiveThread >= 0 ) && gui->mThreads[gui->mActiveThread].key )
      gui->mModel->setPos( gui->mThreads[gui->mActiveThread].key, dAtof( data ) );
   return false;
}

const char *GuiShapeEdPreview::getFieldThreadPos( void *object, const char *data )
{
   GuiShapeEdPreview* gui = static_cast<GuiShapeEdPreview*>( object );
   if ( gui && ( gui->mActiveThread >= 0 ) && gui->mThreads[gui->mActiveThread].key )
      return Con::getFloatArg( gui->mModel->getPos( gui->mThreads[gui->mActiveThread].key ) );
   else
      return "0";
}

bool GuiShapeEdPreview::setFieldThreadDir( void *object, const char *index, const char *data )
{
   GuiShapeEdPreview* gui = static_cast<GuiShapeEdPreview*>( object );
   if ( gui && ( gui->mActiveThread >= 0 ) )
   {
      Thread& thread = gui->mThreads[gui->mActiveThread];
      Con::setData( TypeS32, &(thread.direction), 0, 1, &data );
      if ( thread.key )
         gui->mModel->setTimeScale( thread.key, gui->mTimeScale * thread.direction );
   }
   return false;
}

const char *GuiShapeEdPreview::getFieldThreadDir( void *object, const char *data )
{
   GuiShapeEdPreview* gui = static_cast<GuiShapeEdPreview*>( object );
   if ( gui && ( gui->mActiveThread >= 0 ) )
      return Con::getIntArg( gui->mThreads[gui->mActiveThread].direction );
   else
      return "0";
}

bool GuiShapeEdPreview::setFieldThreadPingPong( void *object, const char *index, const char *data )
{
   GuiShapeEdPreview* gui = static_cast<GuiShapeEdPreview*>( object );
   if ( gui && ( gui->mActiveThread >= 0 ) )
      Con::setData( TypeBool, &(gui->mThreads[gui->mActiveThread].pingpong), 0, 1, &data );
   return false;
}

const char *GuiShapeEdPreview::getFieldThreadPingPong( void *object, const char *data )
{
   GuiShapeEdPreview* gui = static_cast<GuiShapeEdPreview*>( object );
   if ( gui && ( gui->mActiveThread >= 0 ) )
      return Con::getIntArg( gui->mThreads[gui->mActiveThread].pingpong );
   else
      return "0";
}


bool GuiShapeEdPreview::onWake()
{
   if (!Parent::onWake())
      return false;

   if (!mFakeSun )
      mFakeSun = LIGHTMGR->createLightInfo();

   mFakeSun->setRange( 2000000.0f );
   updateSun();

   mGizmoProfile->mode = MoveMode;

   return( true );
}

void GuiShapeEdPreview::setDisplayType( S32 type )
{
   Parent::setDisplayType( type );
   mOrthoCamTrans.set( 0, 0, 0 );
}

//-----------------------------------------------------------------------------

void GuiShapeEdPreview::setCurrentDetail(S32 dl)
{
   if ( mModel )
   {
      S32 smallest = mModel->getShape()->mSmallestVisibleDL;
      mModel->getShape()->mSmallestVisibleDL = mModel->getShape()->details.size()-1;
      mModel->setCurrentDetail( dl );
      mModel->getShape()->mSmallestVisibleDL = smallest;

      // Match the camera distance to this detail if necessary
      //@todo if ( !gui->mFixedDetail )
   }
}

bool GuiShapeEdPreview::setObjectModel(const char* modelName)
{
   SAFE_DELETE( mModel );
   unmountAll();
   mThreads.clear();
   mActiveThread = -1;

   if (modelName && modelName[0])
   {
      Resource<TSShape> model = ResourceManager::get().load( modelName );
      if (! bool( model ))
      {
         Con::warnf( avar("GuiShapeEdPreview: Failed to load model %s. Please check your model name and load a valid model.", modelName ));
         return false;
      }

      mModel = new TSShapeInstance( model, true );
      AssertFatal( mModel, avar("GuiShapeEdPreview: Failed to load model %s. Please check your model name and load a valid model.", modelName ));

      // Initialize camera values:
      mOrbitPos = mModel->getShape()->center;

      // Set camera move and zoom speed according to model size
      mMoveSpeed = mModel->getShape()->radius / sMoveScaler;
      mZoomSpeed = mModel->getShape()->radius / sZoomScaler;

      // Reset node selection
      mHoverNode = -1;
      mSelectedNode = -1;
      mSelectedObject = -1;
      mSelectedObjDetail = 0;
      mProjectedNodes.setSize( mModel->getShape()->nodes.size() );

      // Reset detail stats
      mCurrentDL = 0;

      // the first time recording
      mLastRenderTime = Platform::getVirtualMilliseconds();
   }

   return true;
}

void GuiShapeEdPreview::addThread()
{
   if ( mModel )
   {
      mThreads.increment();
      if ( mActiveThread == -1 )
         mActiveThread = 0;
   }
}

void GuiShapeEdPreview::removeThread(S32 slot)
{
   if ( slot < mThreads.size() )
   {
      if ( mThreads[slot].key )
         mModel->destroyThread( mThreads[slot].key );
      mThreads.erase( slot );

      if ( mActiveThread >= mThreads.size() )
         mActiveThread = mThreads.size() - 1;
   }
}

void GuiShapeEdPreview::setTimeScale( F32 scale )
{
   // Update time scale for all threads
   mTimeScale = scale;
   for ( S32 i = 0; i < mThreads.size(); i++ )
   {
      if ( mThreads[i].key )
         mModel->setTimeScale( mThreads[i].key, mTimeScale * mThreads[i].direction );
   }
}

void GuiShapeEdPreview::setActiveThreadSequence(const char* seqName, F32 duration, F32 pos, bool play)
{
   if ( mActiveThread == -1 )
      return;

   setThreadSequence(mThreads[mActiveThread], mModel, seqName, duration, pos, play);
}

void GuiShapeEdPreview::setThreadSequence(GuiShapeEdPreview::Thread& thread, TSShapeInstance* shape, const char* seqName, F32 duration, F32 pos, bool play)
{
   thread.seqName = seqName;

   S32 seq = shape->getShape()->findSequence( thread.seqName );
   if ( thread.key && ( shape->getSequence(thread.key) == seq ) )
      return;

   if ( seq == -1 )
   {
      // This thread is now set to an invalid sequence, so the key must be
      // removed, but we keep the thread info around in case the user changes
      // back to a valid sequence
      if ( thread.key )
      {
         shape->destroyThread( thread.key );
         thread.key = NULL;
      }
   }
   else
   {
      // Add a TSThread key if one does not already exist
      if ( !thread.key )
      {
         thread.key = shape->addThread();
         shape->setTimeScale( thread.key, mTimeScale * thread.direction );
      }

      // Transition to slider or synched position?
      if ( pos == -1.0f )
         pos = shape->getPos( thread.key );

      if ( duration == 0.0f )
      {
         // No transition => go straight to new sequence
         shape->setSequence( thread.key, seq, pos );
      }
      else
      {
         // Get the current position if transitioning to the sync position
         shape->setTimeScale( thread.key, thread.direction >= 0 ? 1 : -1 );
         shape->transitionToSequence( thread.key, seq, pos, duration, play );
         shape->setTimeScale( thread.key, mTimeScale * thread.direction );
      }
   }
}

const char* GuiShapeEdPreview::getThreadSequence() const
{
   return ( mActiveThread >= 0 ) ? mThreads[mActiveThread].seqName : "";
}

void GuiShapeEdPreview::refreshThreadSequences()
{
   S32 oldActive = mActiveThread;

   for ( S32 i = 0; i < mThreads.size(); i++ )
   {
      Thread& thread = mThreads[i];
      if ( !thread.key )
         continue;

      // Detect changed (or removed) sequence indices
      if ( mModel->getSequence(thread.key) != mModel->getShape()->findSequence( thread.seqName ) )
      {
         mActiveThread = i;
         setThreadSequence( thread, mModel, thread.seqName, 0.0f, mModel->getPos( thread.key ), false );
      }
   }

   mActiveThread = oldActive;
}

//-----------------------------------------------------------------------------
// MOUNTING

bool GuiShapeEdPreview::mountShape(const char* modelName, const char* nodeName, const char* mountType, S32 slot)
{
   if ( !modelName || !modelName[0] )
      return false;

   Resource<TSShape> model = ResourceManager::get().load( modelName );
   if ( !bool( model ) )
      return false;

   TSShapeInstance* tsi = new TSShapeInstance( model, true );
   if ( !tsi )
      return false;

   if ( slot == -1 )
   {
      slot = mMounts.size();
      mMounts.push_back( new MountedShape );
   }
   else
   {
      // Check if we are switching shapes
      if ( mMounts[slot]->mShape->getShape() != tsi->getShape() )
      {
         delete mMounts[slot]->mShape;
         mMounts[slot]->mShape = NULL;
         mMounts[slot]->mThread.init();
      }
      else
      {
         // Keep using the existing shape
         delete tsi;
         tsi = mMounts[slot]->mShape;
      }
   }

   MountedShape* mount = mMounts[slot];
   mount->mShape = tsi;

   if ( dStrEqual( mountType, "Wheel" ) )
      mount->mType = MountedShape::Wheel;
   else if ( dStrEqual( mountType, "Image" ) )
      mount->mType = MountedShape::Image;
   else
      mount->mType = MountedShape::Object;

   setMountNode( slot, nodeName);

   return true;
}

void GuiShapeEdPreview::setMountNode(S32 mountSlot, const char* nodeName)
{
   if ( mountSlot < mMounts.size() )
   {
      MountedShape* mount = mMounts[mountSlot];

      mount->mNode = mModel ? mModel->getShape()->findNode( nodeName ) : -1;
      mount->mTransform.identity();

      switch ( mount->mType )
      {
      case MountedShape::Image:
         {
            // Mount point is either the node called 'mountPoint' or the origin
            S32 node = mount->mShape->getShape()->findNode( "mountPoint" );
            if ( node != -1 )
            {
               mount->mShape->getShape()->getNodeWorldTransform( node, &mount->mTransform );
               mount->mTransform.inverse();
            }
         }
         break;

      case MountedShape::Wheel:
         // Rotate shape according to node's x position (left or right)
         {
            F32 rotAngle = M_PI_F/2;
            if ( mount->mNode != -1 )
            {
               MatrixF hubMat;
               mModel->getShape()->getNodeWorldTransform( mount->mNode, &hubMat );
               if ( hubMat.getPosition().x < 0 )
                  rotAngle = -M_PI_F/2;
            }
            mount->mTransform.set( EulerF( 0, 0, rotAngle ) );
         }
         break;

      default:
         // No mount transform (use origin)
         break;
      }
   }
}

const char* GuiShapeEdPreview::getMountThreadSequence(S32 mountSlot) const
{
   if ( mountSlot < mMounts.size() )
   {
      MountedShape* mount = mMounts[mountSlot];
      return mount->mThread.seqName;
   }
   else
      return "";
}

void GuiShapeEdPreview::setMountThreadSequence(S32 mountSlot, const char* seqName)
{
   if ( mountSlot < mMounts.size() )
   {
      MountedShape* mount = mMounts[mountSlot];
      setThreadSequence( mount->mThread, mount->mShape, seqName );
   }
}

F32 GuiShapeEdPreview::getMountThreadPos(S32 mountSlot) const
{
   if ( mountSlot < mMounts.size() )
   {
      MountedShape* mount = mMounts[mountSlot];
      if ( mount->mThread.key )
         return mount->mShape->getPos( mount->mThread.key );
   }
   return 0;
}

void GuiShapeEdPreview::setMountThreadPos(S32 mountSlot, F32 pos)
{
   if ( mountSlot < mMounts.size() )
   {
      MountedShape* mount = mMounts[mountSlot];
      if ( mount->mThread.key )
         mount->mShape->setPos( mount->mThread.key, pos );
   }
}

F32 GuiShapeEdPreview::getMountThreadDir(S32 mountSlot) const
{
   if ( mountSlot < mMounts.size() )
   {
      MountedShape* mount = mMounts[mountSlot];
      return mount->mThread.direction;
   }
   return 0;
}

void GuiShapeEdPreview::setMountThreadDir(S32 mountSlot, F32 dir)
{
   if ( mountSlot < mMounts.size() )
   {
      MountedShape* mount = mMounts[mountSlot];
      mount->mThread.direction = dir;
      if ( mount->mThread.key )
         mount->mShape->setTimeScale( mount->mThread.key, mTimeScale * mount->mThread.direction );
   }
}

void GuiShapeEdPreview::unmountShape(S32 mountSlot)
{
   if ( mountSlot < mMounts.size() )
   {
      delete mMounts[mountSlot];
      mMounts.erase( mountSlot );
   }
}

void GuiShapeEdPreview::unmountAll()
{
   for ( S32 i = 0; i < mMounts.size(); i++)
      delete mMounts[i];
   mMounts.clear();
}

void GuiShapeEdPreview::refreshShape()
{
   if ( mModel )
   {
      // Nodes or details may have changed => refresh the shape instance
      mModel->setMaterialList( mModel->mMaterialList );
      mModel->initNodeTransforms();
      mModel->initMeshObjects();

      mProjectedNodes.setSize( mModel->getShape()->nodes.size() );

      if ( mSelectedObject >= mModel->getShape()->objects.size() )
      {
         mSelectedObject = -1;
         mSelectedObjDetail = 0;
      }

      // Re-compute the collision mesh stats
      mColMeshes = 0;
      mColPolys = 0;
      for ( S32 i = 0; i < mModel->getShape()->details.size(); i++ )
      {
         const TSShape::Detail& det = mModel->getShape()->details[i];
         const String& detName = mModel->getShape()->getName( det.nameIndex );
         if ( ( det.subShapeNum < 0 ) || !detName.startsWith( "collision-" ) )
            continue;

         mColPolys += det.polyCount;

         S32 od = det.objectDetailNum;
         S32 start = mModel->getShape()->subShapeFirstObject[det.subShapeNum];
         S32 end   = start + mModel->getShape()->subShapeNumObjects[det.subShapeNum];
         for ( S32 j = start; j < end; j++ )
         {
            const TSShape::Object &obj = mModel->getShape()->objects[j];
            const TSMesh* mesh = ( od < obj.numMeshes ) ? mModel->getShape()->meshes[obj.startMeshIndex + od] : NULL;
            if ( mesh )
               mColMeshes++;
         }
      }
   }
}

void GuiShapeEdPreview::updateSun()
{
   if ( mFakeSun )
   {
      // Update sun colors
      mFakeSun->setColor( mSunDiffuseColor );
      mFakeSun->setAmbient( mSunAmbientColor );

      // Determine the new sun direction and position
      Point3F vec;
      MatrixF xRot, zRot;
      xRot.set( EulerF( mDegToRad(mSunRot.x), 0.0f, 0.0f ));
      zRot.set( EulerF( 0.0f, 0.0f, mDegToRad(mSunRot.z) ));

      zRot.mul( xRot );
      zRot.getColumn( 1, &vec );

      mFakeSun->setDirection( vec );
      //mFakeSun->setPosition( vec * -10000.0f );
   }
}

void GuiShapeEdPreview::updateNodeTransforms()
{
   if ( mModel )
      mModel->mDirtyFlags[0] |= TSShapeInstance::TransformDirty;
}

bool GuiShapeEdPreview::getMeshHidden( const char* name ) const
{
   if ( mModel )
   {
      S32 objIndex = mModel->getShape()->findObject( name );
      if ( objIndex != -1 )
         return mModel->mMeshObjects[objIndex].forceHidden;
   }
   return false;
}

void GuiShapeEdPreview::setMeshHidden( const char* name, bool hidden )
{
   if ( mModel )
   {
      S32 objIndex = mModel->getShape()->findObject( name );
      if ( objIndex != -1 )
         mModel->setMeshForceHidden( objIndex, hidden );
   }
}

void GuiShapeEdPreview::setAllMeshesHidden( bool hidden )
{
   if ( mModel )
   {
      for ( S32 i = 0; i < mModel->mMeshObjects.size(); i++ )
         mModel->setMeshForceHidden( i, hidden );
   }
}

void GuiShapeEdPreview::get3DCursor( GuiCursor *&cursor, 
                                       bool &visible, 
                                       const Gui3DMouseEvent &event_ )
{
   cursor = NULL;
   visible = false;

   GuiCanvas *root = getRoot();
   if ( !root )
      return;

   S32 currCursor = PlatformCursorController::curArrow;

   if ( root->mCursorChanged == currCursor )
      return;

   PlatformWindow *window = root->getPlatformWindow();
   PlatformCursorController *controller = window->getCursorController();

   // We've already changed the cursor, 
   // so set it back before we change it again.
   if ( root->mCursorChanged != -1 )
      controller->popCursor();

   // Now change the cursor shape
   controller->pushCursor( currCursor );
   root->mCursorChanged = currCursor;
}

void GuiShapeEdPreview::fitToShape()
{
   if ( !mModel )
      return;

   // Determine the shape bounding box given the current camera rotation
   MatrixF camRotMatrix( smCamMatrix );
   camRotMatrix.setPosition( Point3F::Zero );
   camRotMatrix.inverse();

   Box3F bounds;
   computeSceneBounds( bounds );
   mOrbitPos = bounds.getCenter();

   camRotMatrix.mul( bounds );

   // Estimate the camera distance to fill the view by comparing the radii
   // of the box and the viewport
   F32 len_x = bounds.len_x();
   F32 len_z = bounds.len_z();
   F32 shapeRadius = mSqrt( len_x*len_x + len_z*len_z ) / 2;
   F32 viewRadius = 0.45f * getMin( getExtent().x, getExtent().y );

   // Set camera parameters
   if ( mDisplayType == DisplayTypePerspective )
   {
      mOrbitDist = ( shapeRadius / viewRadius ) * mSaveWorldToScreenScale.y;
   }
   else
   {
      mOrthoCamTrans.set( 0, 0, 0 );
      mOrthoFOV = shapeRadius * viewRadius / 320;
   }
}

void GuiShapeEdPreview::setOrbitPos( const Point3F& pos )
{
   mOrbitPos = pos;
}

void GuiShapeEdPreview::exportToCollada( const String& path )
{
#ifdef TORQUE_COLLADA
   if ( mModel )
   {
      MatrixF orientation( true );
      orientation.setPosition( mModel->getShape()->bounds.getCenter() );
      orientation.inverse();

      OptimizedPolyList polyList;
      polyList.setBaseTransform( orientation );

      mModel->buildPolyList( &polyList, mCurrentDL );
      for ( S32 i = 0; i < mMounts.size(); i++ )
      {
         MountedShape* mount = mMounts[i];

         MatrixF mat( true );
         if ( mount->mNode != -1 )
         {
            mat = mModel->mNodeTransforms[ mount->mNode ];
            mat *= mount->mTransform;
         }

         polyList.setTransform( &mat, Point3F::One );
         mount->mShape->buildPolyList( &polyList, 0 );
      }

      // Use a ColladaUtils function to do the actual export to a Collada file
      ColladaUtils::exportToCollada( path, polyList );
   }
#endif
}

//-----------------------------------------------------------------------------
// Camera control and Node editing
// - moving the mouse over a node will highlight (but not select) it
// - left clicking on a node will select it, the gizmo will appear
// - left clicking on no node will unselect the current node
// - left dragging the gizmo will translate/rotate the node
// - middle drag translates the view
// - right drag rotates the view
// - mouse wheel zooms the view
// - holding shift while changing the view speeds them up

void GuiShapeEdPreview::handleMouseDown(const GuiEvent& event, GizmoMode mode)
{
   if (!mActive || !mVisible || !mAwake )
      return;

   mouseLock();
   mLastMousePos = event.mousePoint;

   if ( mRenderNodes && ( mode == NoneMode ) )
   {
      mGizmoDragID++;
      make3DMouseEvent( mLastEvent, event );

      // Check gizmo first
      mUsingAxisGizmo = false;
      if ( mSelectedNode != -1 )
      {
         mGizmo->on3DMouseDown( mLastEvent );
         if ( mGizmo->getSelection() != Gizmo::None )
         {
            mUsingAxisGizmo = true;
            return;
         }
      }

      // Check if we have clicked on a node
      S32 selected = collideNode( mLastEvent );
      if ( selected != mSelectedNode )
      {
         mSelectedNode = selected;
         Con::executef( this, "onNodeSelected", Con::getIntArg( mSelectedNode ));
      }
   }

   if ( mode == RotateMode )
      mRenderCameraAxes = true;
}

void GuiShapeEdPreview::handleMouseUp(const GuiEvent& event, GizmoMode mode)
{
   mouseUnlock();
   mUsingAxisGizmo = false;

   if ( mRenderNodes && ( mode == NoneMode ) )
   {
      make3DMouseEvent( mLastEvent, event );
      mGizmo->on3DMouseUp( mLastEvent );
   }

   if ( mode == RotateMode )
      mRenderCameraAxes = false;
}

void GuiShapeEdPreview::handleMouseMove(const GuiEvent& event, GizmoMode mode)
{
   if ( mRenderNodes && ( mode == NoneMode ) )
   {
      make3DMouseEvent( mLastEvent, event );
      if ( mSelectedNode != -1 )
      {
         // Check if the mouse is hovering over an axis
         mGizmo->on3DMouseMove( mLastEvent );
         if ( mGizmo->getSelection() != Gizmo::None )
            return;
      }

      // Check if we are over another node
      mHoverNode = collideNode( mLastEvent );
   }
}

void GuiShapeEdPreview::handleMouseDragged(const GuiEvent& event, GizmoMode mode)
{
   // For non-perspective views, ignore rotation, and let EditTSCtrl handle
   // translation
   if ( mDisplayType != DisplayTypePerspective )
   {
      if ( mode == MoveMode )
      {
         Parent::onRightMouseDragged( event );
         return;
      }
      else if ( mode == RotateMode )
         return;
   }

   Point2F delta( event.mousePoint.x - mLastMousePos.x, event.mousePoint.y - mLastMousePos.y );
   mLastMousePos = event.mousePoint;

   // Use shift to increase speed
   delta.x *= ( event.modifier & SI_SHIFT ) ? 0.05f : 0.01f;
   delta.y *= ( event.modifier & SI_SHIFT ) ? 0.05f : 0.01f;

   if ( mode == NoneMode )
   {
      if ( mEditingSun )
      {
         mSunRot.x += mRadToDeg( delta.y );
         mSunRot.z += mRadToDeg( delta.x );
         updateSun();
      }
      else if ( mRenderNodes )
      {
         make3DMouseEvent( mLastEvent, event );

         if ( mUsingAxisGizmo )
         {
            // Use gizmo to modify the transform of the selected node
            mGizmo->on3DMouseDragged( mLastEvent );
            switch ( mGizmoProfile->mode )
            {
            case MoveMode:
               // Update node transform
               if ( mSelectedNode != -1 )
               {
                  Point3F pos = mModel->mNodeTransforms[mSelectedNode].getPosition() + mGizmo->getOffset();
                  mModel->mNodeTransforms[mSelectedNode].setPosition( pos );
               }
               break;

            case RotateMode:
               // Update node transform
               if ( mSelectedNode != -1 )
               {
                  EulerF rot = mGizmo->getDeltaRot();
                  mModel->mNodeTransforms[mSelectedNode].mul( MatrixF( rot ) );
               }
               break;
            default:
               break;
            }

            // Notify the change in node transform
            const char* name = mModel->getShape()->getNodeName(mSelectedNode).c_str();
            const Point3F pos = mModel->mNodeTransforms[mSelectedNode].getPosition();
            AngAxisF aa(mModel->mNodeTransforms[mSelectedNode]);
            char buffer[256];
            dSprintf(buffer, sizeof(buffer), "%g %g %g %g %g %g %g",
               pos.x, pos.y, pos.z, aa.axis.x, aa.axis.y, aa.axis.z, aa.angle);

            Con::executef(this, "onEditNodeTransform", name, buffer, Con::getIntArg(mGizmoDragID));
         }
      }
   }
   else
   {
      switch ( mode )
      {
      case MoveMode:
         {
            VectorF offset(-delta.x, 0, delta.y );
            smCamMatrix.mulV( offset );
            mOrbitPos += offset * mMoveSpeed;
         }
         break;

      case RotateMode:
         mCameraRot.x += delta.y;
         mCameraRot.z += delta.x;
         break;

      default:
         break;
      }
   }
}

void GuiShapeEdPreview::on3DMouseWheelUp(const Gui3DMouseEvent& event)
{
   if ( mDisplayType == DisplayTypePerspective )
   {
      // Use shift and ctrl to increase speed
      F32 mod = ( event.modifier & SI_SHIFT ) ? ( ( event.modifier & SI_CTRL ) ? 4.0 : 1.0 ) : 0.25f;
      mOrbitDist -= mFabs(event.fval) * mZoomSpeed * mod;
   }
}

void GuiShapeEdPreview::on3DMouseWheelDown(const Gui3DMouseEvent& event)
{
   if ( mDisplayType == DisplayTypePerspective )
   {
      // Use shift and ctrl to increase speed
      F32 mod = ( event.modifier & SI_SHIFT ) ? ( ( event.modifier & SI_CTRL ) ? 4.0 : 1.0 ) : 0.25f;
      mOrbitDist += mFabs(event.fval) * mZoomSpeed * mod;
   }
}

//-----------------------------------------------------------------------------
// NODE PICKING
void GuiShapeEdPreview::updateProjectedNodePoints()
{
   if ( mModel )
   {
      // Project the 3D node position to get the 2D screen coordinates
      for ( S32 i = 0; i < mModel->mNodeTransforms.size(); i++)
         project( mModel->mNodeTransforms[i].getPosition(), &mProjectedNodes[i] );
   }
}

S32 GuiShapeEdPreview::collideNode(const Gui3DMouseEvent& event) const
{
   // Check if the given position is inside the screen rectangle of
   // any shape node
   S32 nodeIndex = -1;
   F32 minZ = 0;
   for ( S32 i = 0; i < mProjectedNodes.size(); i++)
   {
      const Point3F& pt = mProjectedNodes[i];
      if ( pt.z > 1.0f )
         continue;

      RectI rect( pt.x - sNodeRectSize/2, pt.y - sNodeRectSize/2, sNodeRectSize, sNodeRectSize );
      if ( rect.pointInRect( event.mousePoint ) )
      {
         if ( ( nodeIndex == -1 ) || ( pt.z < minZ ) )
         {
            nodeIndex = i;
            minZ = pt.z;
         }
      }
   }

   return nodeIndex;
}

//-----------------------------------------------------------------------------
// RENDERING
bool GuiShapeEdPreview::getCameraTransform(MatrixF* cameraMatrix)
{
   // Adjust the camera so that we are still facing the model
   if ( mDisplayType == DisplayTypePerspective )
   {
      Point3F vec;
      MatrixF xRot, zRot;
      xRot.set( EulerF( mCameraRot.x, 0.0f, 0.0f ));
      zRot.set( EulerF( 0.0f, 0.0f, mCameraRot.z ));

      cameraMatrix->mul( zRot, xRot );
      cameraMatrix->getColumn( 1, &vec );
      cameraMatrix->setColumn( 3, mOrbitPos - vec*mOrbitDist );
   }
   else
   {
      cameraMatrix->identity();
      if ( mModel )
      {
         Point3F camPos = mModel->getShape()->bounds.getCenter();
         F32 offset = mModel->getShape()->bounds.len();

         switch (mDisplayType)
         {
            case DisplayTypeTop:       camPos.z += offset;    break;
            case DisplayTypeBottom:    camPos.z -= offset;    break;
            case DisplayTypeFront:     camPos.y += offset;    break;
            case DisplayTypeBack:      camPos.y -= offset;    break;
            case DisplayTypeRight:     camPos.x += offset;    break;
            case DisplayTypeLeft:      camPos.x -= offset;    break;
            default:
               break;
         }

         cameraMatrix->setColumn( 3, camPos );
      }
   }

   return true;
}

void GuiShapeEdPreview::computeSceneBounds(Box3F& bounds)
{
   if ( mModel )
      mModel->computeBounds( mCurrentDL, bounds );
}

void GuiShapeEdPreview::updateDetailLevel(const SceneRenderState* state)
{
   // Make sure current detail is valid
   if ( !mModel->getShape()->details.size() )
      return;

   if ( mModel->getCurrentDetail() >= mModel->getShape()->details.size() )
      setCurrentDetail( mModel->getShape()->details.size() - 1 );

   // Convert between FOV and distance so zoom is consistent between Perspective
   // and Orthographic views (conversion factor found by trial and error)
   const F32 fov2dist = 1.3f;
   if ( mDisplayType == DisplayTypePerspective )
      mOrthoFOV = mOrbitDist / fov2dist;
   else
      mOrbitDist = mOrthoFOV * fov2dist;

   // Use fixed distance in orthographic view (value found by trial + error)
   F32 dist = ( mDisplayType == DisplayTypePerspective ) ? mOrbitDist : 0.1f;

   // Select the appropriate detail level, and update the detail stats
   S32 currentDetail = mModel->getCurrentDetail();
   mModel->setDetailFromDistance( state, dist );   // need to call this to update smLastPixelSize
   if ( mFixedDetail )
      setCurrentDetail( currentDetail );

   if ( mModel->getCurrentDetail() < 0 )
      setCurrentDetail( 0 );

   currentDetail = mModel->getCurrentDetail();
   const TSShape::Detail& det = mModel->getShape()->details[ currentDetail ];

   mDetailPolys = det.polyCount;
   mDetailSize = det.size;
   mPixelSize = TSShapeInstance::smLastPixelSize;

   mNumMaterials = 0;
   mNumDrawCalls = 0;
   mNumBones = 0;
   mNumWeights = 0;

   if ( det.subShapeNum < 0 )
   {
      mNumMaterials = 1;
      mNumDrawCalls = 1;
   }
   else
   {
      Vector<U32> usedMaterials;

      S32 start = mModel->getShape()->subShapeFirstObject[det.subShapeNum];
      S32 end = start + mModel->getShape()->subShapeNumObjects[det.subShapeNum];

      for ( S32 iObj = start; iObj < end; iObj++ )
      {
         const TSShape::Object& obj = mModel->getShape()->objects[iObj];

         if ( obj.numMeshes <= currentDetail )
            continue;

         const TSMesh* mesh = mModel->getShape()->meshes[ obj.startMeshIndex + currentDetail ];
         if ( !mesh )
            continue;

         // Count the number of draw calls and materials
         mNumDrawCalls += mesh->primitives.size();
         for ( S32 iPrim = 0; iPrim < mesh->primitives.size(); iPrim++ )
            usedMaterials.push_back_unique( mesh->primitives[iPrim].matIndex & TSDrawPrimitive::MaterialMask );

         // For skinned meshes, count the number of bones and weights
         if ( mesh->getMeshType() == TSMesh::SkinMeshType )
         {
            const TSSkinMesh* skin = dynamic_cast<const TSSkinMesh*>(mesh);
            mNumBones += skin->batchData.initialTransforms.size();
            mNumWeights += skin->weight.size();
         }
      }

      mNumMaterials = usedMaterials.size();
   }

   // Detect changes in detail level
   if ( mCurrentDL != currentDetail )
   {
      mCurrentDL = currentDetail;
      Con::executef( this, "onDetailChanged");
   }
}

void GuiShapeEdPreview::updateThreads(F32 delta)
{
   // Advance time on all threads
   for ( S32 i = 0; i < mThreads.size(); i++ )
   {
      Thread& thread = mThreads[i];
      if ( !thread.key || !thread.direction )
         continue;

      // Make sure thread priority matches sequence priority (which may have changed)
      mModel->setPriority( thread.key, mModel->getShape()->sequences[mModel->getSequence( thread.key )].priority );

      // Handle ping-pong
      if ( thread.pingpong && !mModel->isInTransition( thread.key ) )
      {
         // Determine next position, then adjust if needed
         F32 threadPos = mModel->getPos( thread.key );
         F32 nextPos = threadPos + ( mModel->getTimeScale( thread.key ) * delta / mModel->getDuration( thread.key ) );

         if ( nextPos < 0 )
         {
            // Reflect position and swap playback direction
            nextPos = -nextPos;
            mModel->setTimeScale( thread.key, -mModel->getTimeScale( thread.key ) );
            mModel->setPos( thread.key, nextPos );
         }
         else if ( nextPos > 1.0f )
         {
            // Reflect position and swap playback direction
            nextPos = 2.0f - nextPos;
            mModel->setTimeScale( thread.key, -mModel->getTimeScale( thread.key ) );
            mModel->setPos( thread.key, nextPos );
         }
         else
         {
            // Advance time normally
            mModel->advanceTime( delta, thread.key );
         }
      }
      else
      {
         // Advance time normally
         mModel->advanceTime( delta, thread.key );
      }

      // Invoke script callback if active thread position has changed
      if ( i == mActiveThread )
      {
         F32 threadPos = mModel->getPos( thread.key );
         bool inTransition = mModel->isInTransition( thread.key );
         onThreadPosChanged_callback( threadPos, inTransition );
      }
   }

   // Mark threads as dirty so they will be re-sorted, in case the user changed
   // sequence priority or blend flags
   mModel->setDirty( TSShapeInstance::ThreadDirty );

   // Advance time on all mounted shape threads
   for ( S32 i = 0; i < mMounts.size(); i++ )
   {
      MountedShape* mount = mMounts[i];
      if ( mount->mThread.key )
         mount->mShape->advanceTime( delta, mount->mThread.key );
   }
}

void GuiShapeEdPreview::renderWorld(const RectI &updateRect)
{
   if ( !mModel )
      return;

   mSaveFrustum = GFX->getFrustum();
   mSaveFrustum.setFarDist( 100000.0f );
   GFX->setFrustum( mSaveFrustum );
   mSaveFrustum.setTransform( smCamMatrix );

   mSaveProjection = GFX->getProjectionMatrix();
   mSaveWorldToScreenScale = GFX->getWorldToScreenScale();

   FogData savedFogData = gClientSceneGraph->getFogData();
   gClientSceneGraph->setFogData( FogData() );  // no fog in preview window

   SceneRenderState state
   (
      gClientSceneGraph,
      SPT_Diffuse,
      SceneCameraState( GFX->getViewport(), mSaveFrustum,
                        GFX->getWorldMatrix(), GFX->getProjectionMatrix() )
   );

   // Set up pass transforms
   RenderPassManager *renderPass = state.getRenderPass();
   renderPass->assignSharedXform( RenderPassManager::View, GFX->getWorldMatrix() );
   renderPass->assignSharedXform( RenderPassManager::Projection, GFX->getProjectionMatrix() );

   // Set up our TS render state here.
   TSRenderState rdata;
   rdata.setSceneState(&state);

   LIGHTMGR->unregisterAllLights();
   LIGHTMGR->setSpecialLight( LightManager::slSunLightType, mFakeSun );

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init( SphereF( Point3F::Zero, 1 ) );
   rdata.setLightQuery( &query );

   // Update projected node points (for mouse picking)
   updateProjectedNodePoints();

   // Determine time elapsed since last render (for animation playback)
   S32 time = Platform::getVirtualMilliseconds();
   S32 dt = time - mLastRenderTime;
   mLastRenderTime = time;

   if ( mModel )
   {
      updateDetailLevel( &state );

      // Render the grid
      renderGrid();

      // Animate the model
      updateThreads( (F32)dt / 1000.f );
      mModel->animate();

      // Render the shape
      GFX->setStateBlock( mDefaultGuiSB );

      if ( mRenderGhost )
         rdata.setFadeOverride( 0.5f );

      GFX->pushWorldMatrix();
      GFX->setWorldMatrix( MatrixF::Identity );

      mModel->render( rdata );

      // Render mounted objects
      if ( mRenderMounts )
      {
         for ( S32 i = 0; i < mMounts.size(); i++ )
         {
            MountedShape* mount = mMounts[i];

            GFX->pushWorldMatrix();

            if ( mount->mNode != -1 )
            {
               GFX->multWorld( mModel->mNodeTransforms[ mount->mNode ] );
               GFX->multWorld( mount->mTransform );
            }

            mount->mShape->animate();
            mount->mShape->render( rdata );

            GFX->popWorldMatrix();
         }
      }

      GFX->popWorldMatrix();

      renderPass->renderPass( &state );

      // @todo: Model and other elements (bounds, grid etc) use different
      // zBuffers, so at the moment, draw order determines what is on top

      // Render collision volumes
      renderCollisionMeshes();

      // Render the shape bounding box
      if ( mRenderBounds )
      {
         Point3F boxSize = mModel->getShape()->bounds.maxExtents - mModel->getShape()->bounds.minExtents;

         GFXStateBlockDesc desc;
         desc.fillMode = GFXFillWireframe;
         GFX->getDrawUtil()->drawCube( desc, boxSize, mModel->getShape()->center, ColorF::WHITE );
      }

      // Render the selected object bounding box
      if ( mRenderObjBox && ( mSelectedObject != -1 ) )
      {
         const TSShape::Object& obj = mModel->getShape()->objects[mSelectedObject];
         const TSMesh* mesh = ( mCurrentDL < obj.numMeshes ) ? mModel->getShape()->meshes[obj.startMeshIndex + mSelectedObjDetail] : NULL;
         if ( mesh )
         {
            GFX->pushWorldMatrix();
            if ( obj.nodeIndex != -1 )
               GFX->multWorld( mModel->mNodeTransforms[ obj.nodeIndex ] );

            const Box3F& bounds = mesh->getBounds();
            GFXStateBlockDesc desc;
            desc.fillMode = GFXFillWireframe;
            GFX->getDrawUtil()->drawCube( desc, bounds.getExtents(), bounds.getCenter(), ColorF::RED );

            GFX->popWorldMatrix();
         }
      }

      // Render the sun direction if currently editing it
      renderSunDirection();

      // render the nodes in the model
      renderNodes();

      // use the gizmo to render the camera axes
      if ( mRenderCameraAxes )
      {
         GizmoMode savedMode = mGizmoProfile->mode;
         mGizmoProfile->mode = MoveMode;

         Point3F pos;
         Point2I screenCenter( updateRect.point + updateRect.extent/2 );
         unproject( Point3F( screenCenter.x, screenCenter.y, 0.5 ), &pos );

         mGizmo->set( MatrixF::Identity, pos, Point3F::One);
         mGizmo->renderGizmo( smCamMatrix );

         mGizmoProfile->mode = savedMode;
      }
   }

   gClientSceneGraph->setFogData( savedFogData );         // restore fog setting
}

void GuiShapeEdPreview::renderGui(Point2I offset, const RectI& updateRect)
{
   // Render the 2D stuff here

   // Render the names of the hovered and selected nodes
   if ( mModel )
   {
      if ( mRenderNodes && mHoverNode != -1 )
         renderNodeName( mHoverNode, ColorF::WHITE );
      if ( mSelectedNode != -1 )
         renderNodeName( mSelectedNode, ColorF::WHITE );
   }
}

void GuiShapeEdPreview::renderGrid()
{
   if ( mRenderGridPlane )
   {
      // Use EditTSCtrl to render the grid in non-perspective views
      if ( mDisplayType != DisplayTypePerspective )
      {
         Parent::renderGrid();
         return;
      }

      // Round grid dimension up to a multiple of the minor ticks
      Point2I dim(mGridDimension.x + mGridPlaneMinorTicks, mGridDimension.y + mGridPlaneMinorTicks);
      dim /= ( mGridPlaneMinorTicks + 1 );
      dim *= ( mGridPlaneMinorTicks + 1 );

      Point2F minorStep( mGridPlaneSize, mGridPlaneSize );
      Point2F size( minorStep.x * dim.x, minorStep.y * dim.y );
      Point2F majorStep( minorStep * ( mGridPlaneMinorTicks + 1 ) );

      GFXStateBlockDesc desc;
      desc.setBlend( true );
      desc.setZReadWrite( true, false );

      GFX->getDrawUtil()->drawPlaneGrid( desc, Point3F::Zero, size, minorStep, mGridPlaneMinorTickColor );
      GFX->getDrawUtil()->drawPlaneGrid( desc, Point3F::Zero, size, majorStep, mGridPlaneColor );
   }
}

void GuiShapeEdPreview::renderSunDirection() const
{
   if ( mEditingSun )
   {
      // Render four arrows aiming in the direction of the sun's light
      ColorI color( mFakeSun->getColor() );
      F32 length = mModel->getShape()->bounds.len() * 0.8f;

      // Get the sun's vectors
      Point3F fwd = mFakeSun->getTransform().getForwardVector();
      Point3F up = mFakeSun->getTransform().getUpVector() * length / 8;
      Point3F right = mFakeSun->getTransform().getRightVector() * length / 8;

      // Calculate the start and end points of the first arrow (bottom left)
      Point3F start = mModel->getShape()->center - fwd * length - up/2 - right/2;
      Point3F end = mModel->getShape()->center - fwd * length / 3 - up/2 - right/2;

      GFXStateBlockDesc desc;
      desc.setZReadWrite( true, true );

      GFX->getDrawUtil()->drawArrow( desc, start, end, color );
      GFX->getDrawUtil()->drawArrow( desc, start + up, end + up, color );
      GFX->getDrawUtil()->drawArrow( desc, start + right, end + right, color );
      GFX->getDrawUtil()->drawArrow( desc, start + up + right, end + up + right, color );
   }
}

void GuiShapeEdPreview::renderNodes() const
{
   if ( mRenderNodes )
   {
      // Render links between nodes
      GFXStateBlockDesc desc;
      desc.setZReadWrite( false, true );
      desc.setCullMode( GFXCullNone );
      GFX->setStateBlockByDesc( desc );

      PrimBuild::color( ColorI::WHITE );
      PrimBuild::begin( GFXLineList, mModel->getShape()->nodes.size() * 2 );
      for ( S32 i = 0; i < mModel->getShape()->nodes.size(); i++)
      {
         const TSShape::Node& node = mModel->getShape()->nodes[i];
         if (node.parentIndex >= 0)
         {
            Point3F start(mModel->mNodeTransforms[i].getPosition());
            Point3F end(mModel->mNodeTransforms[node.parentIndex].getPosition());

            PrimBuild::vertex3f( start.x, start.y, start.z );
            PrimBuild::vertex3f( end.x, end.y, end.z );
         }
      }
      PrimBuild::end();

      // Render the node axes
      for ( S32 i = 0; i < mModel->getShape()->nodes.size(); i++)
      {
         // Render the selected and hover nodes last (so they are on top)
         if ( ( i == mSelectedNode ) || ( i == mHoverNode ) )
            continue;   

         renderNodeAxes( i, ColorF::WHITE );
      }

      // Render the hovered node
      if ( mHoverNode != -1 )
         renderNodeAxes( mHoverNode, ColorF::GREEN );
   }

   // Render the selected node (even if mRenderNodes is false)
   if ( mSelectedNode != -1 )
   {
      renderNodeAxes( mSelectedNode, ColorF::GREEN );

      const MatrixF& nodeMat = mModel->mNodeTransforms[mSelectedNode];
      mGizmo->set( nodeMat, nodeMat.getPosition(), Point3F::One);
      mGizmo->renderGizmo( smCamMatrix );
   }
}

void GuiShapeEdPreview::renderNodeAxes(S32 index, const ColorF& nodeColor) const
{
   const Point3F xAxis( 1.0f,  0.15f, 0.15f );
   const Point3F yAxis( 0.15f, 1.0f,  0.15f );
   const Point3F zAxis( 0.15f, 0.15f, 1.0f  );

   GFXStateBlockDesc desc;
   desc.setZReadWrite( false, true );
   desc.setCullMode( GFXCullNone );

   // Render nodes the same size regardless of zoom
   F32 scale = mOrbitDist / 60;

   GFX->pushWorldMatrix();
   GFX->multWorld( mModel->mNodeTransforms[index] );

   GFX->getDrawUtil()->drawCube( desc, xAxis * scale, Point3F::Zero, nodeColor );
   GFX->getDrawUtil()->drawCube( desc, yAxis * scale, Point3F::Zero, nodeColor );
   GFX->getDrawUtil()->drawCube( desc, zAxis * scale, Point3F::Zero, nodeColor );

   GFX->popWorldMatrix();
}

void GuiShapeEdPreview::renderNodeName(S32 index, const ColorF& textColor) const
{
   const TSShape::Node& node = mModel->getShape()->nodes[index];
   const String& nodeName = mModel->getShape()->getName( node.nameIndex );

   Point2I pos( mProjectedNodes[index].x, mProjectedNodes[index].y + sNodeRectSize + 6 );

   GFX->getDrawUtil()->setBitmapModulation( textColor );
   GFX->getDrawUtil()->drawText( mProfile->mFont, pos, nodeName.c_str() );
}

void GuiShapeEdPreview::renderCollisionMeshes() const
{
   if ( mRenderColMeshes )
   {
      ConcretePolyList polylist;
      polylist.setTransform( &MatrixF::Identity, Point3F::One );
      for ( S32 iDet = 0; iDet < mModel->getShape()->details.size(); iDet++ )
      {
         const TSShape::Detail& det = mModel->getShape()->details[iDet];
         const String& detName = mModel->getShape()->getName( det.nameIndex );

         // Ignore non-collision details
         if ( detName.startsWith( "Collision-" ) )
            mModel->buildPolyList( &polylist, iDet );
      }

      polylist.render();
   }
}

//-----------------------------------------------------------------------------
// Console methods (GuiShapeEdPreview)
//-----------------------------------------------------------------------------

DefineEngineMethod( GuiShapeEdPreview, setOrbitPos, void, ( Point3F pos ),,
   "Set the camera orbit position\n\n"
   "@param pos Position in the form \"x y z\"\n" )
{
   object->setOrbitPos( pos );
}

DefineEngineMethod( GuiShapeEdPreview, setModel, bool, ( const char* shapePath ),,
   "Sets the model to be displayed in this control\n\n"
   "@param shapeName Name of the model to display.\n"
   "@return True if the model was loaded successfully, false otherwise.\n" )
{
   return object->setObjectModel( shapePath );
}

DefineEngineMethod( GuiShapeEdPreview, fitToShape, void, (),,
   "Adjust the camera position and zoom to fit the shape within the view.\n\n" )
{
   object->fitToShape();
}

DefineEngineMethod( GuiShapeEdPreview, refreshShape, void, (),,
   "Refresh the shape (used when the shape meshes or nodes have been added or removed)\n\n" )
{
   object->refreshShape();
}

DefineEngineMethod( GuiShapeEdPreview, updateNodeTransforms, void, (),,
   "Refresh the shape node transforms (used when a node transform has been modified externally)\n\n" )
{
   object->updateNodeTransforms();
}

DefineEngineMethod( GuiShapeEdPreview, computeShapeBounds, Box3F, (),,
   "Compute the bounding box of the shape using the current detail and node transforms\n\n"
   "@return the bounding box \"min.x min.y min.z max.x max.y max.z\"" )
{
   Box3F bounds;
   object->computeSceneBounds(bounds);
   return bounds;
}

DefineEngineMethod( GuiShapeEdPreview, getMeshHidden, bool, ( const char* name ),,
   "Return whether the named object is currently hidden\n\n" )
{
   return object->getMeshHidden( name );
}

DefineEngineMethod( GuiShapeEdPreview, setMeshHidden, void, ( const char* name, bool hidden ),,
   "Show or hide the named object in the shape\n\n" )
{
   object->setMeshHidden( name, hidden );
}

DefineEngineMethod( GuiShapeEdPreview, setAllMeshesHidden, void, ( bool hidden ),,
   "Show or hide all objects in the shape\n\n" )
{
   object->setAllMeshesHidden( hidden );
}

DefineEngineMethod( GuiShapeEdPreview, exportToCollada, void, ( const char* path ),,
   "Export the current shape and all mounted objects to COLLADA (.dae).\n"
   "Note that animation is not exported, and all geometry is combined into a "
   "single mesh.\n\n"
   "@param path Destination filename\n" )
{
   object->exportToCollada( path );
}

//-----------------------------------------------------------------------------
// THREADS
DefineEngineMethod( GuiShapeEdPreview, addThread, void, (),,
   "Add a new thread (initially without any sequence set)\n\n" )
{
   object->addThread();
}

DefineEngineMethod( GuiShapeEdPreview, removeThread, void, ( S32 slot ),,
   "Removes the specifed thread\n\n"
   "@param slot index of the thread to remove\n" )
{
   object->removeThread( slot );
}

DefineEngineMethod( GuiShapeEdPreview, getThreadCount, S32, (),,
   "Get the number of threads\n\n"
   "@return the number of threads\n" )
{
   return object->getThreadCount();
}

DefineEngineMethod( GuiShapeEdPreview, setTimeScale, void, ( F32 scale ),,
   "Set the time scale of all threads\n\n"
   "@param scale new time scale value\n" )
{
   object->setTimeScale( scale );
}

DefineEngineMethod( GuiShapeEdPreview, setThreadSequence, void, ( const char* name, F32 duration, F32 pos, bool play ), ( 0, 0, false ),
   "Sets the sequence to play for the active thread.\n\n"
   "@param name name of the sequence to play\n"
   "@param duration transition duration (0 for no transition)\n"
   "@param pos position in the new sequence to transition to\n"
   "@param play if true, the new sequence will play during the transition\n" )
{
   object->setActiveThreadSequence( name, duration, pos, play );
}

DefineEngineMethod( GuiShapeEdPreview, getThreadSequence, const char*, (),,
   "Get the name of the sequence assigned to the active thread" )
{
   return object->getThreadSequence();
}

DefineEngineMethod( GuiShapeEdPreview, refreshThreadSequences, void, (),,
   "Refreshes thread sequences (in case of removed/renamed sequences" )
{
   object->refreshThreadSequences();
}

//-----------------------------------------------------------------------------
// Mounting
DefineEngineMethod( GuiShapeEdPreview, mountShape, bool, ( const char* shapePath, const char* nodeName, const char* type, S32 slot ),,
   "Mount a shape onto the main shape at the specified node\n\n"
   "@param shapePath path to the shape to mount\n"
   "@param nodeName name of the node on the main shape to mount to\n"
   "@param type type of mounting to use (Object, Image or Wheel)\n"
   "@param slot mount slot\n" )
{
   return object->mountShape( shapePath, nodeName, type, slot );
}

DefineEngineMethod( GuiShapeEdPreview, setMountNode, void, ( S32 slot, const char* nodeName ),,
   "Set the node a shape is mounted to.\n\n"
   "@param slot mounted shape slot\n"
   "@param nodename name of the node to mount to\n" )
{
   object->setMountNode( slot, nodeName );
}

DefineEngineMethod( GuiShapeEdPreview, getMountThreadSequence, const char*, ( S32 slot ),,
   "Get the name of the sequence playing on this mounted shape\n"
   "@param slot mounted shape slot\n"
   "@return name of the sequence (if any)\n" )
{
   return object->getMountThreadSequence( slot );
}

DefineEngineMethod( GuiShapeEdPreview, setMountThreadSequence, void, ( S32 slot, const char* name ),,
   "Set the sequence to play for the shape mounted in the specified slot\n"
   "@param slot mounted shape slot\n"
   "@param name name of the sequence to play\n" )
{
   object->setMountThreadSequence( slot, name );
}

DefineEngineMethod( GuiShapeEdPreview, getMountThreadPos, F32, ( S32 slot ),,
   "Get the playback position of the sequence playing on this mounted shape\n"
   "@param slot mounted shape slot\n"
   "@return playback position of the sequence (0-1)\n" )
{
   return object->getMountThreadPos( slot );
}

DefineEngineMethod( GuiShapeEdPreview, setMountThreadPos, void, ( S32 slot, F32 pos ),,
   "Set the sequence position of the shape mounted in the specified slot\n"
   "@param slot mounted shape slot\n"
   "@param pos sequence position (0-1)\n" )
{
   object->setMountThreadPos( slot, pos );
}

DefineEngineMethod( GuiShapeEdPreview, getMountThreadDir, F32, ( S32 slot ),,
   "Get the playback direction of the sequence playing on this mounted shape\n"
   "@param slot mounted shape slot\n"
   "@return direction of the sequence (-1=reverse, 0=paused, 1=forward)\n" )
{
   return object->getMountThreadDir( slot );
}

DefineEngineMethod( GuiShapeEdPreview, setMountThreadDir, void, ( S32 slot, F32 dir ),,
   "Set the playback direction of the shape mounted in the specified slot\n"
   "@param slot mounted shape slot\n"
   "@param dir playback direction (-1=backwards, 0=paused, 1=forwards)\n" )
{
   object->setMountThreadDir( slot, dir );
}

DefineEngineMethod( GuiShapeEdPreview, unmountShape, void, ( S32 slot ),,
   "Unmount the shape in the specified slot\n"
   "@param slot mounted shape slot\n" )
{
   return object->unmountShape( slot );
}

DefineEngineMethod( GuiShapeEdPreview, unmountAll, void, (),,
   "Unmount all shapes\n" )
{
   return object->unmountAll();
}
