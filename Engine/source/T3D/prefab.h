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

#ifndef _PREFAB_H_
#define _PREFAB_H_

#ifndef _SCENEOBJECT_H_
   #include "scene/sceneObject.h"
#endif
#ifndef _PATH_H_
   #include "core/util/path.h"
#endif
#ifndef _UNDO_H_
   #include "util/undo.h"
#endif
#ifndef _TDICTIONARY_H_
   #include "core/util/tDictionary.h"
#endif


class BaseMatInstance;


class Prefab : public SceneObject
{
   typedef SceneObject Parent;
   
   enum MaskBits 
   {
      TransformMask = Parent::NextFreeMask << 0,
      FileMask = Parent::NextFreeMask << 1,
      NextFreeMask  = Parent::NextFreeMask << 2
   };   

public:

   Prefab();
   virtual ~Prefab();

   DECLARE_CONOBJECT(Prefab);
  
   static void initPersistFields();

   // SimObject
   virtual bool onAdd();
   virtual void onRemove();
   virtual void onEditorEnable();
   virtual void onEditorDisable();
   virtual void inspectPostApply();

   // NetObject
   U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn, BitStream *stream );

   // SceneObject
   virtual void setTransform( const MatrixF &mat );
   virtual void setScale(const VectorF & scale);

   // Prefab

   /// If the passed object is a child of any Prefab return that Prefab.
   /// Note that this call is only valid if the editor is open and when 
   /// passed server-side objects.
   static Prefab* getPrefabByChild( SimObject *child );

   /// Returns false if the passed object is of a type that is not allowed
   /// as a child within a Prefab.
   static bool isValidChild( SimObject *child, bool logWarnings );

   ///
   void render( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

   ///
   void setFile( String file );

   /// Removes all children from this Prefab and puts them into a SimGroup
   /// which is added to the MissionGroup and returned to the caller.
   SimGroup* explode();

protected:

   void _closeFile( bool removeFileNotify );
   void _loadFile( bool addFileNotify );
   void _updateChildTransform( SceneObject* child );
   void _updateChildren();
   void _onFileChanged( const Torque::Path &path );

   static bool protectedSetFile( void *object, const char *index, const char *data );

   /// @name Callbacks
   /// @{

   DECLARE_CALLBACK( void, onLoad, ( SimGroup *children ) );

   /// @}

protected:

   /// Prefab file which defines our children objects.
   String mFilename;

   /// Group which holds all children objects.
   SimObjectPtr<SimGroup> mChildGroup;

   /// Structure to keep track of child object initial transform and scale
   struct Transform
   {
      MatrixF mat;
      VectorF scale;
      Transform() : mat(true), scale(Point3F::One) { }
      Transform( const MatrixF& m, const VectorF& s ) : mat(m), scale(s) { }
   };
   typedef Map<SimObjectId,Transform> ChildToMatMap;

   /// Lookup from a child object's id to its transform in 
   /// this Prefab's object space.
   ChildToMatMap mChildMap;

   typedef Map<SimObjectId,SimObjectId> ChildToPrefabMap;

   /// Lookup from a SimObject to its parent Prefab if it has one.
   static ChildToPrefabMap smChildToPrefabMap;
};


class ExplodePrefabUndoAction : public UndoAction
{
   typedef UndoAction Parent;
   friend class WorldEditor;

public:

   ExplodePrefabUndoAction( Prefab *prefab );   

   // UndoAction
   virtual void undo();
   virtual void redo();

protected:

   SimGroup *mGroup;
   SimObjectId mPrefabId;
};


#endif // _PREFAB_H_