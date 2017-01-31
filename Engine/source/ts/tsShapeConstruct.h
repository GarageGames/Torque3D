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

#ifndef _TSSHAPECONSTRUCT_H_
#define _TSSHAPECONSTRUCT_H_

#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _MTRANSFORM_H_
#include "math/mTransform.h"
#endif
#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _COLLADA_UTILS_H_
#include "ts/collada/colladaUtils.h"
#endif
#ifndef _ENGINEAPI_H_
#include "console/engineAPI.h"
#endif

/// This class allows an artist to export their animations for the model
/// into the .dsq format.  This class in particular matches up the model
/// with the .dsqs to create a nice animated model.
class TSShapeConstructor : public SimObject
{
   typedef SimObject Parent;

public:
   struct ChangeSet
   {
      enum eCommandType
      {
         CmdAddNode,
         CmdRemoveNode,
         CmdRenameNode,
         CmdSetNodeTransform,
         CmdSetNodeParent,

         CmdAddMesh,
         CmdAddPrimitive,
         CmdRemoveMesh,
         CmdSetMeshSize,
         CmdSetMeshType,
         CmdSetMeshMaterial,

         CmdRemoveObject,
         CmdRenameObject,
         CmdSetObjectNode,
         CmdSetBounds,

         CmdRenameDetailLevel,
         CmdRemoveDetailLevel,
         CmdSetDetailLevelSize,
         CmdAddImposter,
         CmdRemoveImposter,
         CmdAddCollisionDetail,

         CmdAddSequence,
         CmdRemoveSequence,
         CmdRenameSequence,
         CmdSetSequenceCyclic,
         CmdSetSequenceBlend,
         CmdSetSequencePriority,
         CmdSetSequenceGroundSpeed,

         CmdAddTrigger,
         CmdRemoveTrigger,

         CmdInvalid
      };

      struct Command
      {
         eCommandType      type;       // Command type
         StringTableEntry  name;       // Command name
         static const U32 MAX_ARGS = 10;
         String            argv[MAX_ARGS];   // Command arguments
         S32               argc;       // Number of arguments
         Command() : type(CmdInvalid), name(0), argc(0) { }
         Command( const char* _name )
            : type(CmdInvalid), argc(0)
         {
            name = StringTable->insert( _name );
         }
        
        // Helper functions to fill in the command arguments
        template<typename ...ArgTs> inline void addArgs(ArgTs ...args){
           using Helper = engineAPI::detail::MarshallHelpers<String>;
           Helper::marshallEach(argc, argv, args...);
        }
      };

      Vector<Command>   mCommands;

      eCommandType getCmdType(const char* name);
      void clear() { mCommands.clear(); }
      bool empty() { return mCommands.empty(); }

      void add( Command& cmd );

      // These methods handle change set optimisation based on the newly added command
      bool addCmd_setNodeParent( const Command& newCmd );
      bool addCmd_setNodeTransform( const Command& newCmd );
      bool addCmd_renameNode( const Command& newCmd );
      bool addCmd_removeNode( const Command& newCmd );

      bool addCmd_setMeshSize( const Command& newCmd );
      bool addCmd_setMeshType( const Command& newCmd );
      bool addCmd_setMeshMaterial( const Command& newCmd );
      bool addCmd_removeMesh( const Command& newCmd );

      bool addCmd_setObjectNode( const Command& newCmd );
      bool addCmd_renameObject( const Command& newCmd );
      bool addCmd_removeObject( const Command& newCmd );
      bool addCmd_setBounds( const Command& newCmd );

      bool addCmd_renameDetailLevel( const Command& newCmd );
      bool addCmd_removeDetailLevel( const Command& newCmd );
      bool addCmd_setDetailSize( const Command& newCmd );
      bool addCmd_addImposter( const Command& newCmd );
      bool addCmd_removeImposter( const Command& newCmd );

      bool addCmd_addSequence( Command& newCmd );
      bool addCmd_setSequencePriority( const Command& newCmd );
      bool addCmd_setSequenceGroundSpeed( const Command& newCmd );
      bool addCmd_setSequenceCyclic( const Command& newCmd );
      bool addCmd_setSequenceBlend( const Command& newCmd );
      bool addCmd_renameSequence( const Command& newCmd );
      bool addCmd_removeSequence( const Command& newCmd );

      bool addCmd_addTrigger( const Command& newCmd );
      bool addCmd_removeTrigger( const Command& newCmd );

      void write(TSShape* shape, Stream& stream, const String& savePath);
   };

   static const S32 MaxLegacySequences = 127;

protected:
   FileName          mShapePath;
   Vector<FileName>  mSequences;
   ChangeSet         mChangeSet;

   // Paths to shapes used by MeshFit
   static String smCapsuleShapePath;
   static String smCubeShapePath;
   static String smSphereShapePath;

   static bool addSequenceFromField( void *obj, const char *index, const char *data );
   
   static void       _onTSShapeLoaded( Resource< TSShape >& shape );
   static void       _onTSShapeUnloaded( const Torque::Path& path, TSShape* shape );
   
   static ResourceRegisterPostLoadSignal< TSShape > _smAutoLoad;
   static ResourceRegisterUnloadSignal< TSShape > _smAutoUnload;
   
   /// @name Callbacks
   ///@{
   DECLARE_CALLBACK( void, onLoad, () );
   DECLARE_CALLBACK( void, onUnload, () );
   ///@}

   virtual void      _onLoad( TSShape* shape );
   virtual void      _onUnload();

public:

   TSShape*                mShape;        // Edited shape; NULL while not loaded; not a Resource<TSShape> as we don't want it to prevent from unloading.
   ColladaUtils::ImportOptions   mOptions;
   bool mLoadingShape;

public:

   TSShapeConstructor();
   TSShapeConstructor(const String& path) : mShapePath(path) { }
   ~TSShapeConstructor();

   DECLARE_CONOBJECT(TSShapeConstructor);
   static void initPersistFields();
   static void consoleInit();
   static TSShapeConstructor* findShapeConstructor(const FileName& path);

   bool onAdd();

   void onScriptChanged(const Torque::Path& path);
   void onActionPerformed();

   bool writeField(StringTableEntry fieldname, const char *value);
   void writeChangeSet();

   void notifyShapeChanged();

   /// @name Shape paths for MeshFit
   ///@{
   static const String& getCapsuleShapePath() { return smCapsuleShapePath; }
   static const String& getCubeShapePath() { return smCubeShapePath; }
   static const String& getSphereShapePath() { return smSphereShapePath; }
   ///@}

   TSShape* getShape() const { return mShape; }
   const String& getShapePath() const { return mShapePath; }

   /// @name Dumping
   ///@{
   void dumpShape( const char* filename );
   void saveShape( const char* filename );
   ///@}

   /// @name Nodes
   ///@{
   S32 getNodeCount();
   S32 getNodeIndex( const char* name );
   const char* getNodeName( S32 index );
   const char* getNodeParentName( const char* name );
   bool setNodeParent( const char* name, const char* parentName );
   S32 getNodeChildCount( const char* name );
   const char* getNodeChildName( const char* name, S32 index );
   S32 getNodeObjectCount( const char* name );
   const char* getNodeObjectName( const char* name, S32 index );
   TransformF getNodeTransform( const char* name, bool isWorld=false );
   bool setNodeTransform( const char* name, TransformF txfm, bool isWorld=false );
   bool renameNode( const char* oldName, const char* newName );
   bool addNode( const char* name, const char* parentName, TransformF txfm=TransformF::Identity, bool isWorld=false);
   bool removeNode( const char* name );
   ///@}

   /// @name Materials
   ///@{
   S32 getTargetCount();
   const char* getTargetName( S32 index );
   ///@}

   ///@{
   S32 getObjectCount();
   const char* getObjectName( S32 index );
   S32 getObjectIndex( const char* name );
   const char* getObjectNode( const char* name );
   bool setObjectNode( const char* objName, const char* nodeName );
   bool renameObject( const char* oldName, const char* newName );
   bool removeObject( const char* name );
   ///@}

   /// @name Meshes
   ///@{
   S32 getMeshCount( const char* name );
   const char* getMeshName( const char* name, S32 index );
   S32 getMeshSize( const char* name, S32 index );
   bool setMeshSize( const char* name, S32 size );
   const char* getMeshType( const char* name );
   bool setMeshType( const char* name, const char* type );
   const char* getMeshMaterial( const char* name );
   bool setMeshMaterial( const char* meshName, const char* matName );
   bool addMesh( const char* meshName, const char* srcShape, const char* srcMesh );
   bool addPrimitive( const char* meshName, const char* type, const char* params, TransformF txfm, const char* nodeName );
   bool removeMesh( const char* name );
   ///@}

   /// @name Detail Levels
   ///@{
   Box3F getBounds();
   bool setBounds( Box3F bbox );
   S32 getDetailLevelCount();
   const char* getDetailLevelName( S32 index );
   S32 getDetailLevelSize( S32 index);
   S32 getDetailLevelIndex( S32 size );
   bool renameDetailLevel( const char* oldName, const char* newName );
   bool removeDetailLevel( S32 index );
   S32 setDetailLevelSize( S32 index, S32 newSize );
   S32 getImposterDetailLevel();
   const char* getImposterSettings( S32 index );
   S32 addImposter( S32 size, S32 equatorSteps, S32 polarSteps, S32 dl, S32 dim, bool includePoles, F32 polarAngle );
   bool removeImposter();
   bool addCollisionDetail( S32 size, const char* type, const char* target, S32 depth=4, F32 merge=30.0f, F32 concavity=30.0f, S32 maxVerts=32, F32 boxMaxError=0, F32 sphereMaxError=0, F32 capsuleMaxError=0 );
   ///@}

   /// @name Sequences
   ///@{
   S32 getSequenceCount();
   S32 getSequenceIndex( const char* name);
   const char* getSequenceName( S32 index );
   const char* getSequenceSource( const char* name );
   S32 getSequenceFrameCount( const char* name );
   F32 getSequencePriority( const char* name );
   bool setSequencePriority( const char* name, F32 priority );
   const char* getSequenceGroundSpeed( const char* name );
   bool setSequenceGroundSpeed( const char* name, Point3F transSpeed, Point3F rotSpeed=Point3F::Zero );
   bool getSequenceCyclic( const char* name );
   bool setSequenceCyclic( const char* name, bool cyclic );
   const char* getSequenceBlend( const char* name );
   bool setSequenceBlend( const char* name, bool blend, const char* blendSeq, S32 blendFrame );
   bool renameSequence( const char* oldName, const char* newName );
   bool addSequence( const char* source, const char* name, S32 start=0, S32 end=-1, bool padRot=true, bool padTrans=false );
   bool removeSequence( const char* name );
   ///@}

   /// @name Triggers
   ///@{
   S32 getTriggerCount( const char* name );
   const char* getTrigger( const char* name, S32 index );
   bool addTrigger( const char* name, S32 keyframe, S32 state );
   bool removeTrigger( const char* name, S32 keyframe, S32 state );
   ///@}
};

typedef domUpAxisType TSShapeConstructorUpAxis;
typedef ColladaUtils::ImportOptions::eLodType TSShapeConstructorLodType;

DefineEnumType( TSShapeConstructorUpAxis );
DefineEnumType(TSShapeConstructorLodType);

class TSShapeConstructorMethodActionCallback
{
   TSShapeConstructor* mObject;

public:
   TSShapeConstructorMethodActionCallback(TSShapeConstructor *object) : mObject(object) { ; }
   ~TSShapeConstructorMethodActionCallback() { mObject->onActionPerformed(); }
};

/* This macro simplifies the definition of a TSShapeConstructor API method. It
   wraps the actual EngineMethod definition and automatically calls the real
   class method. It also creates a ChangeSet::Comand (with all arguments stored
   as strings). The one drawback is that it includes the open brace for the real
   class method, so to keep the code looking mostly normal, such methods start
   with another open brace, and end with a double closing brace. Not perfect,
   but a lot better than having to type out the argument list multiple times for
   the 50 odd API functions. */
#define DefineTSShapeConstructorMethod( name, retType, args, defArgs, rawArgs, defRet, usage )  \
   DefineEngineMethod( TSShapeConstructor, name, retType, args, defArgs, usage )                \
   {                                                                                            \
      /* Check that shape is loaded */                                                          \
      if( !object->getShape() )                                                                 \
      {                                                                                         \
         Con::errorf( "TSShapeConstructor::" #name " - shape not loaded" );                     \
         return defRet;                                                                         \
      }                                                                                         \
      TSShapeConstructorMethodActionCallback actionCallback(object);                            \
      return object->name rawArgs ;                                                             \
   }                                                                                            \
   /* Define the real TSShapeConstructor method */                                              \
   retType TSShapeConstructor::name args                                                        \
   {                                                                                            \
      /* Initialise change set command (may or may not be added) */                             \
      TSShapeConstructor::ChangeSet::Command newCmd( #name );                                   \
      newCmd.addArgs rawArgs ;                                                                  \
      TORQUE_UNUSED(newCmd);


/* This macro just hides the name of the auto-created ChangeSet::Command from
   above, so we are free to change the implementation later if needed */
#define ADD_TO_CHANGE_SET()   mChangeSet.add( newCmd );


#endif
