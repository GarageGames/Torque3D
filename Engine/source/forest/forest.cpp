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
#include "forest/forest.h"

#include "forest/forestCell.h"
#include "forest/forestCollision.h"
#include "forest/forestDataFile.h"
#include "forest/forestWindMgr.h"
#include "forest/forestWindAccumulator.h"

#include "core/resourceManager.h"
#include "core/volume.h"
#include "T3D/gameBase/gameConnection.h"
#include "console/consoleInternal.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "environment/sun.h"
#include "scene/sceneManager.h"
#include "math/mathUtils.h"
#include "T3D/physics/physicsBody.h"
#include "forest/editor/forestBrushElement.h"

/// For frame signal
#include "gui/core/guiCanvas.h"


extern bool gEditingMission;

ForestCreatedSignal Forest::smCreatedSignal;
ForestCreatedSignal Forest::smDestroyedSignal;

bool Forest::smForceImposters = false;
bool Forest::smDisableImposters = false;
bool Forest::smDrawCells = false;
bool Forest::smDrawBounds = false;


IMPLEMENT_CO_NETOBJECT_V1(Forest);

ConsoleDocClass( Forest,
   
   "@brief %Forest is a global-bounds scene object provides collision and rendering for a "
   "(.forest) data file.\n\n"
   
   "%Forest is designed to efficiently render a large number of static meshes: trees, rocks "
   "plants, etc. These cannot be moved at game-time or play animations but do support wind "
   "effects using vertex shader transformations guided by vertex color in the asset and "
   "user placed wind emitters ( or weapon explosions ).\n\n"
   
   "Script level manipulation of forest data is not possible through %Forest, it is only "
   "the rendering/collision. All editing is done through the world editor.\n\n"
   
   "@see TSForestItemData Defines a tree type.\n"
   "@see GuiForestEditorCtrl Used by the world editor to provide manipulation of forest data.\n" 
   "@ingroup Forest"
);


Forest::Forest()
   :  mDataFileName( NULL ),
      mReflectionLodScalar( 2.0f ),
      mConvexList( new Convex() ),
      mZoningDirty( false )
{
   mTypeMask |= EnvironmentObjectType | StaticShapeObjectType | StaticObjectType;
   mNetFlags.set(Ghostable | ScopeAlways);
}

Forest::~Forest()
{
   delete mConvexList;
   mConvexList = NULL;
}


void Forest::initPersistFields()
{
   Parent::initPersistFields();

   addField( "dataFile",  TypeFilename, Offset( mDataFileName, Forest ),
      "The source forest data file." );

   addGroup( "Lod" );
      
      addField( "lodReflectScalar",      TypeF32,       Offset( mReflectionLodScalar, Forest ),
         "Scalar applied to the farclip distance when Forest renders into a reflection." );

   endGroup( "Lod" );
}

void Forest::consoleInit()
{
   // Some stats exposed to the console.
   Con::addVariable("$Forest::totalCells", TypeS32, &Forest::smTotalCells, "@internal" );
   Con::addVariable("$Forest::cellsRendered", TypeS32, &Forest::smCellsRendered, "@internal" );
   Con::addVariable("$Forest::cellItemsRendered", TypeS32, &Forest::smCellItemsRendered, "@internal" );
   Con::addVariable("$Forest::cellsBatched", TypeS32, &Forest::smCellsBatched, "@internal" );
   Con::addVariable("$Forest::cellItemsBatched", TypeS32, &Forest::smCellItemsBatched, "@internal" );
   Con::addVariable("$Forest::averageCellItems", TypeF32, &Forest::smAverageItemsPerCell, "@internal" );

   // Some debug flags.
   Con::addVariable("$Forest::forceImposters", TypeBool, &Forest::smForceImposters,
      "A debugging aid which will force all forest items to be rendered as imposters.\n"
      "@ingroup Forest\n" );
   Con::addVariable("$Forest::disableImposters", TypeBool, &Forest::smDisableImposters,
      "A debugging aid which will disable rendering of all imposters in the forest.\n"
      "@ingroup Forest\n" );
   Con::addVariable("$Forest::drawCells", TypeBool, &Forest::smDrawCells,
      "A debugging aid which renders the forest cell bounds.\n"
      "@ingroup Forest\n" );
   Con::addVariable("$Forest::drawBounds", TypeBool, &Forest::smDrawBounds,
      "A debugging aid which renders the forest bounds.\n"
      "@ingroup Forest\n" );

   // The canvas signal lets us know to clear the rendering stats.
   GuiCanvas::getGuiCanvasFrameSignal().notify( &Forest::_clearStats );
}

bool Forest::onAdd()
{
   if (!Parent::onAdd())
      return false;

   const char *name = getName();
   if(name && name[0] && getClassRep())
   {
      Namespace *parent = getClassRep()->getNameSpace();
      Con::linkNamespaces(parent->mName, name);
      mNameSpace = Con::lookupNamespace(name);
   }

   setGlobalBounds();
   resetWorldBox();

   // TODO: Make sure this calls the script "onAdd" which will
   // populate the object with forest entries before creation.
   addToScene();

   // If we don't have a file name and the editor is 
   // enabled then create an empty forest data file.
   if ( isServerObject() && ( !mDataFileName || !mDataFileName[0] ) )
      createNewFile();
   else
   {
      // Try to load the forest file.
      mData = ResourceManager::get().load( mDataFileName );
      if ( !mData )
      {
         if ( isClientObject() )
            NetConnection::setLastError( "You are missing a file needed to play this mission: %s", mDataFileName );

         return false;
      }
   }

   updateCollision();

   smCreatedSignal.trigger( this );

   if ( isClientObject() )
   {
      mZoningDirty = true;
      SceneZoneSpaceManager::getZoningChangedSignal().notify( this, &Forest::_onZoningChanged );

      ForestWindMgr::getAdvanceSignal().notify( this, &Forest::getLocalWindTrees );
   }

   return true;
}

void Forest::onRemove()
{
   Parent::onRemove();

   smDestroyedSignal.trigger( this );

   if ( mData )
      mData->clearPhysicsRep( this );

   mData = NULL;
   
   if ( isClientObject() )
   {
      SceneZoneSpaceManager::getZoningChangedSignal().remove( this, &Forest::_onZoningChanged );
      ForestWindMgr::getAdvanceSignal().remove( this, &Forest::getLocalWindTrees );
   }

   mConvexList->nukeList();

   removeFromScene();
}

U32 Forest::packUpdate( NetConnection *connection, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( connection, mask, stream );
   
   if ( stream->writeFlag( mask & MediaMask ) ) 
      stream->writeString( mDataFileName );

   if ( stream->writeFlag( mask & LodMask ) )
   {
      stream->write( mReflectionLodScalar );
   }

   return retMask;
}

void Forest::unpackUpdate(NetConnection *connection, BitStream *stream)
{
   Parent::unpackUpdate(connection,stream);
   
   if ( stream->readFlag() )
   {
      mDataFileName = stream->readSTString();
   }

   if ( stream->readFlag() ) // LodMask
   {
      stream->read( &mReflectionLodScalar );
   }
}

void Forest::inspectPostApply()
{
   Parent::inspectPostApply();

   // Update the client... note that this
   // doesn't cause a regen of the forest.
   setMaskBits( LodMask );
}

void Forest::setTransform( const MatrixF &mat )
{
   // Note: We do not use the position of the forest at all.
   Parent::setTransform( mat );
}

void Forest::_onZoningChanged( SceneZoneSpaceManager *zoneManager )
{
   if ( mData == NULL || zoneManager != getSceneManager()->getZoneManager() )
      return;

   mZoningDirty = true;
}

void Forest::getLocalWindTrees( const Point3F &camPos, F32 radius, Vector<TreePlacementInfo> *placementInfo )
{
   PROFILE_SCOPE( Forest_getLocalWindTrees );

   Vector<ForestItem> items;
   items.reserve( placementInfo->capacity() );
   mData->getItems( camPos, radius, &items );

   TreePlacementInfo treeInfo;
   dMemset( &treeInfo, 0, sizeof ( TreePlacementInfo ) );

   // Reserve some space in the output.
   placementInfo->reserve( items.size() );

   // Build an info struct for each returned item.
   Vector<ForestItem>::const_iterator iter = items.begin();
   for ( ; iter != items.end(); iter++ )
   {
      // Skip over any zero wind elements here and
      // just keep them out of the final list.
      treeInfo.dataBlock = iter->getData();
      if ( treeInfo.dataBlock->mWindScale < 0.001f )
         continue;

      treeInfo.pos = iter->getPosition();
      treeInfo.scale = iter->getScale();
      treeInfo.itemKey = iter->getKey();
      placementInfo->push_back( treeInfo );
   }
}

void Forest::applyRadialImpulse( const Point3F &origin, F32 radius, F32 magnitude )
{   
   if ( isServerObject() )
      return;   

   // Find all the trees in the radius
   // then get their accumulators and 
   // push our impulse into them.
   VectorF impulse( 0, 0, 0 );
   ForestWindAccumulator *accumulator = NULL;

   Vector<TreePlacementInfo> trees;
   getLocalWindTrees( origin, radius, &trees );
   for ( U32 i = 0; i < trees.size(); i++ )
   {
      const TreePlacementInfo &treeInfo = trees[i];
      accumulator = WINDMGR->getLocalWind( treeInfo.itemKey );
      if ( !accumulator )
         continue;

      impulse = treeInfo.pos - origin;
      impulse.normalize();
      impulse *= magnitude;

      accumulator->applyImpulse( impulse );
   }
}

void Forest::createNewFile()
{
   // Release the current file if we have one.
   mData = NULL;

   // We need to construct a default file name
   String missionName( Con::getVariable( "$Client::MissionFile" ) );
   String levelDirectory( Con::getVariable( "$pref::Directories::Level" ) );
   if ( levelDirectory.isEmpty() )
   {
      levelDirectory = "levels";
   }
   missionName.replace( "tools/levels", levelDirectory );
   missionName = Platform::makeRelativePathName(missionName, Platform::getMainDotCsDir());

   Torque::Path basePath( missionName );
   String fileName = Torque::FS::MakeUniquePath( basePath.getPath(), basePath.getFileName(), "forest" );
   mDataFileName = StringTable->insert( fileName );

   ForestData *file = new ForestData;
   file->write( mDataFileName );
   delete file;

   mData = ResourceManager::get().load( mDataFileName );
   mZoningDirty = true;
}

void Forest::saveDataFile( const char *path )
{
   if ( path )
      mDataFileName = StringTable->insert( path );

   if ( mData )
      mData->write( mDataFileName );
}

ConsoleMethod( Forest, saveDataFile, bool, 2, 3, "saveDataFile( [path] )" )
{   
   object->saveDataFile( argc == 3 ? argv[2] : NULL );
   return true;
}

ConsoleMethod(Forest, isDirty, bool, 2, 2, "()")
{
   return object->getData() && object->getData()->isDirty();
}

ConsoleMethod(Forest, regenCells, void, 2, 2, "()")
{
   object->getData()->regenCells();
}

ConsoleMethod(Forest, clear, void, 2, 2, "()" )
{
   object->clear();
}