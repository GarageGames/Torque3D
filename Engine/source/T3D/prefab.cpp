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
#include "T3D/prefab.h"

#include "math/mathIO.h"
#include "core/stream/bitStream.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"
#include "console/consoleTypes.h"
#include "core/volume.h"
#include "console/engineAPI.h"
#include "T3D/physics/physicsShape.h"
#include "core/util/path.h"

// We use this locally ( within this file ) to prevent infinite recursion
// while loading prefab files that contain other prefabs.
static Vector<String> sPrefabFileStack;

Map<SimObjectId,SimObjectId> Prefab::smChildToPrefabMap;

IMPLEMENT_CO_NETOBJECT_V1(Prefab);

ConsoleDocClass( Prefab,
   "@brief A collection of arbitrary objects which can be allocated and manipulated as a group.\n\n"
   
   "%Prefab always points to a (.prefab) file which defines its objects. In "
   "fact more than one %Prefab can reference this file and both will update "
   "if the file is modified.\n\n"
   
   "%Prefab is a very simple object and only exists on the server. When it is "
   "created it allocates children objects by reading the (.prefab) file like "
   "a list of instructions.  It then sets their transform relative to the %Prefab "
   "and Torque networking handles the rest by ghosting the new objects to clients. "
   "%Prefab itself is not ghosted.\n\n"
   
   "@ingroup enviroMisc"   
);

IMPLEMENT_CALLBACK( Prefab, onLoad, void, ( SimGroup *children ), ( children ),
   "Called when the prefab file is loaded and children objects are created.\n"
   "@param children SimGroup containing all children objects.\n"
);

Prefab::Prefab()
{
   // Not ghosted unless we're editing
   mNetFlags.clear(Ghostable);

   mTypeMask |= StaticObjectType;
}

Prefab::~Prefab()
{
}

void Prefab::initPersistFields()
{
   addGroup( "Prefab" );

      addProtectedField( "filename", TypePrefabFilename, Offset( mFilename, Prefab ),         
                         &protectedSetFile, &defaultProtectedGetFn,
                         "(.prefab) File describing objects within this prefab." );

   endGroup( "Prefab" );

   Parent::initPersistFields();
}

extern bool gEditingMission;

bool Prefab::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   mObjBox.set( Point3F( -0.5f, -0.5f, -0.5f ),
      Point3F(  0.5f,  0.5f,  0.5f ) );

   resetWorldBox();
   
   // Not added to the scene unless we are editing.
   if ( gEditingMission )
      onEditorEnable();

   // Only the server-side prefab needs to create/update child objects.
   // We rely on regular Torque ghosting of the individual child objects
   // to take care of the rest.
   if ( isServerObject() )
   {
      _loadFile( true );
      _updateChildren();
   }

   return true;
}

void Prefab::onRemove()
{
   if ( isServerObject() )
      _closeFile( true );

   removeFromScene();
   Parent::onRemove();
}

void Prefab::onEditorEnable()
{
   if ( isClientObject() )
      return;

   // Just in case we are already in the scene, lets not cause an assert.   
   if ( mContainer != NULL )
      return;

   // Enable ghosting so we can see this on the client.
   mNetFlags.set(Ghostable);
   setScopeAlways();
   addToScene();

   Parent::onEditorEnable();
}

void Prefab::onEditorDisable()
{   
   if ( isClientObject() )
      return;

   // Just in case we are not in the scene, lets not cause an assert.   
   if ( mContainer == NULL )
      return;

   // Do not need this on the client if we are not editing.
   removeFromScene();
   mNetFlags.clear(Ghostable);
   clearScopeAlways();

   Parent::onEditorDisable();
}

void Prefab::inspectPostApply()
{
   Parent::inspectPostApply();
}

void Prefab::setTransform(const MatrixF & mat)
{
   Parent::setTransform( mat ); 

   if ( isServerObject() )
   {
      setMaskBits( TransformMask );
      _updateChildren();
   }
}

void Prefab::setScale(const VectorF & scale)
{
   Parent::setScale( scale ); 

   if ( isServerObject() )
   {
      setMaskBits( TransformMask );
      _updateChildren();
   }
}

U32 Prefab::packUpdate( NetConnection *conn, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( conn, mask, stream );

   mathWrite(*stream,mObjBox);

   if ( stream->writeFlag( mask & FileMask ) )
   {
      stream->write( mFilename );
   }

   if ( stream->writeFlag( mask & TransformMask ) )
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   return retMask;
}

void Prefab::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Parent::unpackUpdate(conn, stream);

   mathRead(*stream, &mObjBox);
   resetWorldBox();

   // FileMask
   if ( stream->readFlag() ) 
   {
      stream->read( &mFilename );
   }

   // TransformMask
   if ( stream->readFlag() )  
   {
      mathRead(*stream, &mObjToWorld);
      mathRead(*stream, &mObjScale);

      setTransform( mObjToWorld );
   }
}

bool Prefab::protectedSetFile( void *object, const char *index, const char *data )
{
   Prefab *prefab = static_cast<Prefab*>(object);
   
   String file = String( Platform::makeRelativePathName(data, Platform::getMainDotCsDir()) );

   prefab->setFile( file );

   return false;
}

void Prefab::setFile( String file )
{  
   AssertFatal( isServerObject(), "Prefab-bad" );

   if ( !isProperlyAdded() )
   {
      mFilename = file;
      return;
   }
   
   // Client-side Prefab(s) do not create/update/reference children, everything
   // is handled on the server-side. In normal usage this will never actually
   // be called for the client-side prefab but maybe the user did so accidentally.
   if ( isClientObject() )
   {
      Con::errorf( "Prefab::setFile( %s ) - Should not be called on a client-side Prefab.", file.c_str() );
      return;
   }

   _closeFile( true );

   mFilename = file;

   if ( isProperlyAdded() )
      _loadFile( true );
}

SimGroup* Prefab::explode()
{
   SimGroup *missionGroup;

   if ( !Sim::findObject( "MissionGroup", missionGroup ) )
   {
      Con::errorf( "Prefab::explode, MissionGroup was not found." );
      return NULL;
   }

   if ( !mChildGroup )
      return NULL;

   SimGroup *group = mChildGroup;
   Vector<SceneObject*> foundObjects;

   group->findObjectByType( foundObjects );

   if ( foundObjects.empty() )
      return NULL;

   for ( S32 i = 0; i < foundObjects.size(); i++ )
   {
      SceneObject *child = foundObjects[i];
      _updateChildTransform( child );
      smChildToPrefabMap.erase( child->getId() );
   }
   
   missionGroup->addObject(group);
   mChildGroup = NULL;
   mChildMap.clear();

   return group;
}

void Prefab::_closeFile( bool removeFileNotify )
{
   AssertFatal( isServerObject(), "Prefab-bad" );

   mChildMap.clear();

   if ( mChildGroup )
   {
      // Get a flat vector of all our children.
      Vector<SceneObject*> foundObjects;
      mChildGroup->findObjectByType( foundObjects );

      // Remove them all from the ChildToPrefabMap.
      for ( S32 i = 0; i < foundObjects.size(); i++ )
         smChildToPrefabMap.erase( foundObjects[i]->getId() );

      mChildGroup->deleteObject();
      mChildGroup = NULL;
   }

   if ( removeFileNotify )
      Torque::FS::RemoveChangeNotification( mFilename, this, &Prefab::_onFileChanged );

   // Back to a default bounding box size.
   mObjBox.set( Point3F( -0.5f, -0.5f, -0.5f ), Point3F(  0.5f,  0.5f,  0.5f ) );
   resetWorldBox();
}

void Prefab::_loadFile( bool addFileNotify )
{
   AssertFatal( isServerObject(), "Prefab-bad" );

   if ( mFilename.isEmpty() )
      return;

   if ( !Platform::isFile( mFilename ) )
   {
      Con::errorf( "Prefab::_loadFile() - file %s was not found.", mFilename.c_str() );
      return;
   }

   if ( sPrefabFileStack.contains(mFilename) )
   {
      Con::errorf( 
         "Prefab::_loadFile - failed loading prefab file (%s). \n"
         "File was referenced recursively by both a Parent and Child prefab.", mFilename.c_str() );
      return;
   }

   sPrefabFileStack.push_back(mFilename);

   String command = String::ToString( "exec( \"%s\" );", mFilename.c_str() );
   Con::evaluate( command );

   SimGroup *group;
   if ( !Sim::findObject( Con::getVariable( "$ThisPrefab" ), group ) )
   {
      Con::errorf( "Prefab::_loadFile() - file %s did not create $ThisPrefab.", mFilename.c_str() );
      return;
   }

   if ( addFileNotify )
      Torque::FS::AddChangeNotification( mFilename, this, &Prefab::_onFileChanged );

   mChildGroup = group;

   Vector<SceneObject*> foundObjects;
   mChildGroup->findObjectByType( foundObjects );

   if ( !foundObjects.empty() )
   {
      mWorldBox = Box3F::Invalid;

      for ( S32 i = 0; i < foundObjects.size(); i++ )
      {
         SceneObject *child = foundObjects[i];
         mChildMap.insert( child->getId(), Transform( child->getTransform(), child->getScale() ) );
         smChildToPrefabMap.insert( child->getId(), getId() );

         _updateChildTransform( child );

         mWorldBox.intersect( child->getWorldBox() );
      }

      resetObjectBox();
   }

   sPrefabFileStack.pop_back();

   onLoad_callback( mChildGroup );
}

void Prefab::_updateChildTransform( SceneObject* child )
{
   ChildToMatMap::Iterator itr = mChildMap.find(child->getId());
   AssertFatal( itr != mChildMap.end(), "Prefab, mChildMap out of synch with mChildGroup." );

   MatrixF mat( itr->value.mat );
   Point3F pos = mat.getPosition();
   pos.convolve( mObjScale );
   mat.setPosition( pos );
   mat.mulL( mObjToWorld );

   child->setTransform( mat );
   child->setScale( itr->value.scale * mObjScale );

   // Hack for PhysicsShape... need to store the "editor" position to return to
   // when a physics reset event occurs. Normally this would be where it is 
   // during onAdd, but in this case it is not because the prefab stores its
   // child objects in object space...

   PhysicsShape *childPS = dynamic_cast<PhysicsShape*>( child );
   if ( childPS )
      childPS->storeRestorePos();

}

void Prefab::_updateChildren()
{
   if ( !mChildGroup )
      return;

   Vector<SceneObject*> foundObjects;
   mChildGroup->findObjectByType( foundObjects );

   for ( S32 i = 0; i < foundObjects.size(); i++ )
   {
      SceneObject *child = foundObjects[i];

      _updateChildTransform( child );

      if ( child->getClientObject() )
      {
         ((SceneObject*)child->getClientObject())->setTransform( child->getTransform() );
         ((SceneObject*)child->getClientObject())->setScale( child->getScale() );
      }
   }
}

void Prefab::_onFileChanged( const Torque::Path &path )
{
   AssertFatal( path == mFilename, "Prefab::_onFileChanged - path does not match filename." );

   _closeFile(false);
   _loadFile(false);
   setMaskBits(U32_MAX);
}

Prefab* Prefab::getPrefabByChild( SimObject *child )
{
   ChildToPrefabMap::Iterator itr = smChildToPrefabMap.find( child->getId() );
   if ( itr == smChildToPrefabMap.end() )
      return NULL;

   Prefab *prefab;
   if ( !Sim::findObject( itr->value, prefab ) )
   {
      Con::errorf( "Prefab::getPrefabByChild - child object mapped to a prefab that no longer exists." );
      return NULL;
   }

   return prefab;
}

bool Prefab::isValidChild( SimObject *simobj, bool logWarnings )
{
   if ( simobj->getName() && dStricmp(simobj->getName(),"MissionGroup") == 0 )
   {
      if ( logWarnings )
         Con::warnf( "MissionGroup is not valid within a Prefab." );
      return false;
   }

   if ( simobj->getClassRep()->isClass( AbstractClassRep::findClassRep("LevelInfo") ) )
   {
      if ( logWarnings )
         Con::warnf( "LevelInfo objects are not valid within a Prefab" );
      return false;
   }

   if ( simobj->getClassRep()->isClass( AbstractClassRep::findClassRep("TimeOfDay") ) )
   {
      if ( logWarnings )
         Con::warnf( "TimeOfDay objects are not valid within a Prefab" );
      return false;
   }

   SceneObject *sceneobj = dynamic_cast<SceneObject*>(simobj);

   if ( !sceneobj )
      return false;
   
   if ( sceneobj->isGlobalBounds() )
   {
      if ( logWarnings )
         Con::warnf( "SceneObject's with global bounds are not valid within a Prefab." );
      return false;
   }
   
   if ( sceneobj->getClassRep()->isClass( AbstractClassRep::findClassRep("TerrainBlock") ) )
   {
      if ( logWarnings )
         Con::warnf( "TerrainBlock objects are not valid within a Prefab" );
      return false;
   }

   if ( sceneobj->getClassRep()->isClass( AbstractClassRep::findClassRep("Player") ) )
   {
      if ( logWarnings )
         Con::warnf( "Player objects are not valid within a Prefab" );
      return false;
   }

   if ( sceneobj->getClassRep()->isClass( AbstractClassRep::findClassRep("DecalRoad") ) )
   {
      if ( logWarnings )
         Con::warnf( "DecalRoad objects are not valid within a Prefab" );
      return false;
   }

   return true;
}

bool Prefab::buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF& sphere)
{
   Vector<SceneObject*> foundObjects;
   mChildGroup->findObjectByType(foundObjects);

   for (S32 i = 0; i < foundObjects.size(); i++)
   {
      foundObjects[i]->buildPolyList(context, polyList, box, sphere);
   }

   return true;
}

ExplodePrefabUndoAction::ExplodePrefabUndoAction( Prefab *prefab )
: UndoAction( "Explode Prefab" )
{
   mPrefabId = prefab->getId();
   mGroup = NULL;

   // Do the action.
   redo();
}

void ExplodePrefabUndoAction::undo()
{
   if ( !mGroup )   
   {
      Con::errorf( "ExplodePrefabUndoAction::undo - NULL Group" );
      return;
   }

   mGroup->deleteObject();
   mGroup = NULL;   
}

void ExplodePrefabUndoAction::redo()
{
   Prefab *prefab;
   if ( !Sim::findObject( mPrefabId, prefab ) )
   {
      Con::errorf( "ExplodePrefabUndoAction::redo - Prefab (%i) not found.", mPrefabId );
      return;
   }

   mGroup = prefab->explode();

   String name;

   if ( prefab->getName() && prefab->getName()[0] != '\0' )   
      name = prefab->getName();   
   else
      name = "prefab";

   name += "_exploded";
   name = Sim::getUniqueName( name );
   mGroup->assignName( name );   
}