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
#include "ts/tsShapeConstruct.h"

#include "ts/tsShapeInstance.h"
#include "ts/tsMaterialList.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "core/resourceManager.h"
#include "core/stream/bitStream.h"
#include "core/stream/fileStream.h"
#include "core/stream/memStream.h"
#include "core/fileObject.h"

#define MAX_PATH_LENGTH 256


//#define DEBUG_SPEW

ConsoleDocClass( TSShapeConstructor,
   "@brief An object used to modify a DTS or COLLADA shape model after it has "
   "been loaded by Torque\n\n"
   "@ingroup gameObjects\n"
);

IMPLEMENT_CALLBACK( TSShapeConstructor, onLoad, void, (), (),
   "Called immediately after the DTS or DAE file has been loaded; before the "
   "shape data is available to any other object (StaticShape, Player etc). This "
   "is where you should put any post-load commands to modify the shape in-memory "
   "such as addNode, renameSequence etc." )

IMPLEMENT_CALLBACK( TSShapeConstructor, onUnload, void, (), (),
   "Called when the DTS or DAE resource is flushed from memory. Not normally "
   "required, but may be useful to perform cleanup." )

ImplementEnumType( TSShapeConstructorUpAxis,
   "Axis to use for upwards direction when importing from Collada.\n\n"
   "@ingroup TSShapeConstructor" )
   { UPAXISTYPE_X_UP,   "X_AXIS" },
   { UPAXISTYPE_Y_UP,   "Y_AXIS" },
   { UPAXISTYPE_Z_UP,   "Z_AXIS" },
   { UPAXISTYPE_COUNT,  "DEFAULT" }
EndImplementEnumType;

ImplementEnumType( TSShapeConstructorLodType,
   "\n\n"
   "@ingroup TSShapeConstructor" )
   { ColladaUtils::ImportOptions::DetectDTS,       "DetectDTS" },
   { ColladaUtils::ImportOptions::SingleSize,      "SingleSize" },
   { ColladaUtils::ImportOptions::TrailingNumber,  "TrailingNumber" },
EndImplementEnumType;


//-----------------------------------------------------------------------------

String TSShapeConstructor::smCapsuleShapePath("core/art/shapes/unit_capsule.dts");
String TSShapeConstructor::smCubeShapePath("core/art/shapes/unit_cube.dts");
String TSShapeConstructor::smSphereShapePath("core/art/shapes/unit_sphere.dts");

ResourceRegisterPostLoadSignal< TSShape > TSShapeConstructor::_smAutoLoad( &TSShapeConstructor::_onTSShapeLoaded );
ResourceRegisterUnloadSignal< TSShape > TSShapeConstructor::_smAutoUnload( &TSShapeConstructor::_onTSShapeUnloaded );

void TSShapeConstructor::_onTSShapeLoaded( Resource< TSShape >& resource )
{
   TSShapeConstructor* ctor = findShapeConstructor( resource.getPath().getFullPath() );
   if( ctor )
      ctor->_onLoad( resource );

   if (ctor && ctor->mShape && ctor->mShape->needsReinit())
   {
      ctor->mShape->init();
   }
}

void TSShapeConstructor::_onTSShapeUnloaded( const Torque::Path& path, TSShape* shape )
{
   TSShapeConstructor* ctor = findShapeConstructor( path.getFullPath() );
   if( ctor && ( ctor->getShape() == shape ) )
      ctor->_onUnload();
}

// TSShape names are case insensitive
static inline bool namesEqual( const String& nameA, const String& nameB )
{
   return nameA.equal( nameB, String::NoCase );
}

static void SplitSequencePathAndName( String& srcPath, String& srcName )
{
   srcName = "";

   // Determine if there is a sequence name at the end of the source string, and
   // if so, split the filename from the sequence name
   S32 split = srcPath.find(' ', 0, String::Right);
   S32 split2 = srcPath.find('\t', 0, String::Right);
   if ((split == String::NPos) || (split2 > split))
      split = split2;
   if (split != String::NPos)
   {
      split2 = split + 1;
      while ((srcPath[split2] != '\0') && dIsspace(srcPath[split2]))
         split2++;

      // now 'split' is at the end of the path, and 'split2' is at the start of the sequence name
      srcName = srcPath.substr(split2);
      srcPath = srcPath.erase(split, srcPath.length()-split);
   }
}

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(TSShapeConstructor);

TSShapeConstructor::TSShapeConstructor()
 : mShapePath(""), mLoadingShape(false)
{
   mShape = NULL;
}

TSShapeConstructor::~TSShapeConstructor()
{
}

bool TSShapeConstructor::addSequenceFromField( void *obj, const char *index, const char *data )
{
   TSShapeConstructor *pObj = static_cast<TSShapeConstructor*>( obj );

   if ( data && data[0] )
      pObj->mSequences.push_back( FileName(data) );

   return false;
}

void TSShapeConstructor::initPersistFields()
{
   addGroup( "Media" );
   addField( "baseShape", TypeStringFilename, Offset(mShapePath, TSShapeConstructor),
      "Specifies the path to the DTS or DAE file to be operated on by this object.\n"
      "Since the TSShapeConstructor script must be in the same folder as the DTS or "
      "DAE file, it is recommended to use a relative path so that the shape and "
      "script files can be copied to another location without having to modify the "
      "path." );
   endGroup( "Media" );

   addGroup( "Collada" );
   addField( "upAxis", TYPEID< domUpAxisType >(), Offset(mOptions.upAxis, TSShapeConstructor),
      "Override the <up_axis> element in the COLLADA (.dae) file. No effect for DTS files.\n"
      "Set to one of the following values:\n"
      "<dl><dt>X_AXIS</dt><dd>Positive X points up. Model will be rotated into Torque's coordinate system (Z up).</dd>"
      "<dt>Y_AXIS</dt><dd>Positive Y points up. Model will be rotated into Torque's coordinate system (Z up).</dd>"
      "<dt>Z_AXIS</dt><dd>Positive Z points up. No rotation will be applied to the model.</dd>"
      "<dt>DEFAULT</dt><dd>The default value. Use the value in the .dae file (defaults to Z_AXIS if the <up_axis> element is not present).</dd></dl>" );

   addField( "unit", TypeF32, Offset(mOptions.unit, TSShapeConstructor),
      "Override the <unit> element in the COLLADA (.dae) file. No effect for DTS files.\n"
      "COLLADA (.dae) files usually contain a <unit> element that indicates the "
      "'real world' units that the model is described in. It means you can work "
      "in sensible and meaningful units in your modeling app.<br>\n"
      "For example, if you were modeling a small object like a cup, it might make "
      "sense to work in inches (1 MAX unit = 1 inch), but if you were modeling a "
      "building, it might make more sense to work in feet (1 MAX unit = 1 foot). "
      "If you export both models to COLLADA, T3D will automatically scale them "
      "appropriately. 1 T3D unit = 1 meter, so the cup would be scaled down by 0.0254, "
      "and the building scaled down by 0.3048, given them both the correct scale "
      "relative to each other.<br>\n"
      "Omit the field or set to -1 to use the value in the .dae file (1.0 if the "
      "<unit> element is not present)" );

   addField( "lodType", TYPEID< ColladaUtils::ImportOptions::eLodType >(), Offset(mOptions.lodType, TSShapeConstructor),
      "Control how the COLLADA (.dae) importer interprets LOD in the model. No effect for DTS files.\n"
      "Set to one of the following values:\n"
      "<dl><dt>DetectDTS</dt><dd>The default value. Instructs the importer to search for a 'baseXXX->startXXX' node hierarchy at the root level. If found, the importer acts as if ''TrailingNumber'' was set. Otherwise, all geometry is imported at a single detail size.</dd>"
      "<dt>SingleSize</dt><dd>All geometry is imported at a fixed detail size. Numbers at the end of geometry node's are ignored.</dd>"
      "<dt>TrailingNumber</dt><dd>Numbers at the end of geometry node's name are interpreted as the detail size (similar to DTS exporting). Geometry instances with the same base name but different trailing number are grouped into the same object.</dd>"
      "<dt>DEFAULT</dt><dd>The default value. Use the value in the .dae file (defaults to Z_AXIS if the <up_axis> element is not present).</dd></dl>" );

   addField( "singleDetailSize", TypeS32, Offset(mOptions.singleDetailSize, TSShapeConstructor),
      "Sets the detail size when lodType is set to SingleSize. No effect otherwise, and no effect for DTS files.\n"
      "@see lodType" );

   addField( "matNamePrefix", TypeRealString, Offset(mOptions.matNamePrefix, TSShapeConstructor),
      "Prefix to apply to all material map names in the COLLADA (.dae) file. No effect for DTS files.\n"
      "This field is useful to avoid material name clashes for exporters that generate generic material "
      "names like \"texture0\" or \"material1\"." );

   addField( "alwaysImport", TypeRealString, Offset(mOptions.alwaysImport, TSShapeConstructor),
      "TAB separated patterns of nodes to import even if in neverImport list. No effect for DTS files.\n"
      "Torque allows unwanted nodes in COLLADA (.dae) files to to be ignored "
      "during import. This field contains a TAB separated list of patterns to "
      "match node names. Any node that matches one of the patterns in the list "
      "will <b>always</b> be imported, even if it also matches the neverImport list\n"
      "@see neverImport\n\n"
      "@tsexample\n"
      "singleton TSShapeConstructor(MyShapeDae)\n"
      "{\n"
      "   baseShape = \"./myShape.dae\";\n"
      "   alwaysImport = \"mount*\" TAB \"eye\";\n"
      "   neverImport = \"*-PIVOT\";\n"
      "}\n"
      "@endtsexample" );

   addField( "neverImport", TypeRealString, Offset(mOptions.neverImport, TSShapeConstructor),
      "TAB separated patterns of nodes to ignore on loading. No effect for DTS files.\n"
      "Torque allows unwanted nodes in COLLADA (.dae) files to to be ignored "
      "during import. This field contains a TAB separated list of patterns to "
      "match node names. Any node that matches one of the patterns in the list will "
      "not be imported (unless it matches the alwaysImport list.\n"
      "@see alwaysImport" );

   addField( "alwaysImportMesh", TypeRealString, Offset(mOptions.alwaysImportMesh, TSShapeConstructor),
      "TAB separated patterns of meshes to import even if in neverImportMesh list. No effect for DTS files.\n"
      "Torque allows unwanted meshes in COLLADA (.dae) files to to be ignored "
      "during import. This field contains a TAB separated list of patterns to "
      "match mesh names. Any mesh that matches one of the patterns in the list "
      "will <b>always</b> be imported, even if it also matches the neverImportMesh list\n"
      "@see neverImportMesh\n\n"
      "@tsexample\n"
      "singleton TSShapeConstructor(MyShapeDae)\n"
      "{\n"
      "   baseShape = \"./myShape.dae\";\n"
      "   alwaysImportMesh = \"body*\" TAB \"armor\" TAB \"bounds\";\n"
      "   neverImportMesh = \"*-dummy\";\n"
      "}\n"
      "@endtsexample" );

   addField( "neverImportMesh", TypeRealString, Offset(mOptions.neverImportMesh, TSShapeConstructor),
      "TAB separated patterns of meshes to ignore on loading. No effect for DTS files.\n"
      "Torque allows unwanted meshes in COLLADA (.dae) files to to be ignored "
      "during import. This field contains a TAB separated list of patterns to "
      "match mesh names. Any mesh that matches one of the patterns in the list will "
      "not be imported (unless it matches the alwaysImportMesh list.\n"
      "@see alwaysImportMesh" );

   addField( "ignoreNodeScale", TypeBool, Offset(mOptions.ignoreNodeScale, TSShapeConstructor),
      "Ignore <scale> elements inside COLLADA <node>s. No effect for DTS files.\n"
      "This field is a workaround for certain exporters that generate bad node "
      "scaling, and is not usually required." );

   addField( "adjustCenter", TypeBool, Offset(mOptions.adjustCenter, TSShapeConstructor),
      "Translate COLLADA model on import so the origin is at the center. No effect for DTS files." );

   addField( "adjustFloor", TypeBool, Offset(mOptions.adjustFloor, TSShapeConstructor),
      "Translate COLLADA model on import so origin is at the (Z axis) bottom of the model. No effect for DTS files.\n"
      "This can be used along with adjustCenter to have the origin at the "
      "center of the bottom of the model.\n"
      "@see adjustCenter" );

   addField( "forceUpdateMaterials", TypeBool, Offset(mOptions.forceUpdateMaterials, TSShapeConstructor),
      "Forces update of the materials.cs file in the same folder as the COLLADA "
      "(.dae) file, even if Materials already exist. No effect for DTS files.\n"
      "Normally only Materials that are not already defined are written to materials.cs." );
   endGroup( "Collada" );

   addGroup( "Sequences" );
   addProtectedField( "sequence", TypeStringFilename, NULL, &addSequenceFromField, &emptyStringProtectedGetFn,
      "Legacy method of adding sequences to a DTS or DAE shape after loading.\n\n"
      "@tsexample\n"
      "singleton TSShapeConstructor(MyShapeDae)\n"
      "{\n"
      "   baseShape = \"./myShape.dae\";\n"
      "   sequence = \"../anims/root.dae root\";\n"
      "   sequence = \"../anims/walk.dae walk\";\n"
      "   sequence = \"../anims/jump.dsq jump\";\n"
      "}\n"
      "@endtsexample" );
   endGroup( "Sequences" );

   Parent::initPersistFields();
}

void TSShapeConstructor::consoleInit()
{
   Parent::consoleInit();

   Con::addVariable( "$pref::TSShapeConstructor::CapsuleShapePath", TypeRealString, &TSShapeConstructor::smCapsuleShapePath, 
      "The file path to the capsule shape used by tsMeshFit.\n\n"
	   "@ingroup MeshFit\n" );

   Con::addVariable( "$pref::TSShapeConstructor::CubeShapePath", TypeRealString, &TSShapeConstructor::smCubeShapePath, 
      "The file path to the cube shape used by tsMeshFit.\n\n"
	   "@ingroup MeshFit\n" );

   Con::addVariable( "$pref::TSShapeConstructor::SphereShapePath", TypeRealString, &TSShapeConstructor::smSphereShapePath, 
      "The file path to the sphere shape used by tsMeshFit.\n\n"
	   "@ingroup MeshFit\n" );
}

TSShapeConstructor* TSShapeConstructor::findShapeConstructor(const FileName& path)
{
   SimGroup *group;
   if (Sim::findObject( "TSShapeConstructorGroup", group ))
   {
      // Find the TSShapeConstructor object for the given shape file
      for (S32 i = 0; i < group->size(); i++)
      {
         TSShapeConstructor* tss = dynamic_cast<TSShapeConstructor*>( group->at(i) );
         if ( tss->mShapePath.equal( path, String::NoCase ) )
            return tss;
      }
   }
   return NULL;
}

//-----------------------------------------------------------------------------
bool TSShapeConstructor::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   // Prevent multiple objects pointing at the same shape file
   TSShapeConstructor* tss = findShapeConstructor( mShapePath );
   if ( tss )
   {
      Con::errorf("TSShapeConstructor::onAdd failed: %s is already referenced by "
         "another TSShapeConstructor object (%s - %d)", mShapePath.c_str(),
         tss->getName(), tss->getId());
      return false;
   }

   // Add to the TSShapeConstructor group (for lookups)
   SimGroup *group;
   if ( !Sim::findObject( "TSShapeConstructorGroup", group ) )
   {
      group = new SimGroup();
      if ( !group->registerObject("TSShapeConstructorGroup") )
      {
         Con::errorf("TSShapeConstructor::onAdd failed: Could not register "
            "TSShapeConstructorGroup");
         return false;
      }
      Sim::getRootGroup()->addObject( group );
   }
   group->addObject( this );

   // This is only here for backwards compatibility!
   //
   // If we have no sequences, it may be using the older sequence# syntax.
   // Check for dynamic fields of that pattern and add them into the sequence vector.
   if ( mSequences.empty() )
   {
      for ( U32 idx = 0; idx < MaxLegacySequences; idx++ )
      {
         String field = String::ToString( "sequence%d", idx );
         const char *data = getDataField( StringTable->insert(field.c_str()), NULL );

         // Break at first field not used
         if ( !data || !data[0] )
            break;

         // By pushing the field thru Con::setData for TypeStringFilename
         // we get the default filename expansion.  If we didn't do this
         // then we would have unexpanded ~/ in the file paths.
         String expanded;
         Con::setData( TypeStringFilename, &expanded, 0, 1, &data );
         addSequenceFromField( this, NULL, expanded.c_str() );
      }
   }

   // If an instance of this shape has already been loaded, call onLoad now
   Resource<TSShape> shape = ResourceManager::get().find( mShapePath );

   if ( shape )
      _onLoad( shape );

   if (mShape && mShape->needsReinit())
   {
      mShape->init();
   }

   return true;
}

//-----------------------------------------------------------------------------

void TSShapeConstructor::_onLoad(TSShape* shape)
{
   // Check if we should unload first
   if ( mShape )
      _onUnload();

   #ifdef DEBUG_SPEW
   Con::printf( "[TSShapeConstructor] attaching to shape '%s'", mShapePath.c_str() );
   #endif

   mShape = shape;
   mChangeSet.clear();
   mLoadingShape = true;

   // Add sequences defined using field syntax
   for ( S32 i = 0; i < mSequences.size(); i++ )
   {
      if ( mSequences[i].isEmpty() )
         continue;

      // Split the sequence path from the target sequence name
      String destName;
      String srcPath( mSequences[i] );
      SplitSequencePathAndName( srcPath, destName );

      addSequence( srcPath, destName );
   }

   // Call script function
   onLoad_callback();
   mLoadingShape = false;
}

//-----------------------------------------------------------------------------

void TSShapeConstructor::_onUnload()
{
   #ifdef DEBUG_SPEW
   Con::printf( "[TSShapeConstructor] detaching from '%s'", mShapePath.c_str() );
   #endif

   onUnload_callback();

   mShape = NULL;
}

//-----------------------------------------------------------------------------
// Storage

bool TSShapeConstructor::writeField(StringTableEntry fieldname, const char *value)
{
   // Ignore the sequence fields (these are written as 'addSequence' commands instead)
   if ( dStrnicmp( fieldname, "sequence", 8 ) == 0 )
      return false;
   else if ( dStrnicmp( fieldname, "baseShape", 9 ) == 0 )
   {
      // Small hack to only write the base filename (no path) since the
      // TSShapeConstructor script must be in the same folder as the model, and
      // then we can easily copy both around without having to change the field
      const char* filename = dStrrchr( value, '/' );
      if ( filename > value )
      {
         S32 len = dStrlen( filename );
         dMemmove((void*)(value + 1), filename, len );
         ((char*)value)[0] = '.';
         ((char*)value)[len + 1] = '\0';
      }
      return true;
   }

   return Parent::writeField( fieldname, value );
}

//-----------------------------------------------------------------------------
// Console utility methods

// These macros take care of doing node, sequence and object lookups, including
// printing error messages to the console if the element cannot be found.

// Check that the given index is valid (0 - max-1). If not, generate an
// error and return.
#define CHECK_INDEX_IN_RANGE(func, index, maxIndex, ret)             \
   if ( ( index < 0 ) || ( index >= maxIndex ) )                     \
   {                                                                 \
      Con::errorf( "TSShapeConstructor::" #func ": index out of "    \
         "range (0-%d)", maxIndex-1);                                \
      return ret;                                                    \
   }

// Do a node lookup and allow the root node name ("")
#define GET_NODE_INDEX_ALLOW_ROOT(func, var, name, ret)              \
   S32 var##Index = -1;                                              \
   if (name[0])                                                      \
   {                                                                 \
      var##Index = mShape->findNode(name);                           \
      if (var##Index < 0)                                            \
      {                                                              \
         Con::errorf( "TSShapeConstructor::" #func ": Could not "    \
            "find node '%s'", name);                                 \
         return ret;                                                 \
      }                                                              \
   }                                                                 \
   TSShape::Node* var = var##Index < 0 ? NULL : &(mShape->nodes[var##Index]); \
   TORQUE_UNUSED(var##Index);                                        \
   TORQUE_UNUSED(var)

// Do a node lookup, root node ("") is not allowed
#define GET_NODE_INDEX_NO_ROOT(func, var, name, ret)                 \
   S32 var##Index = mShape->findNode(name);                          \
   if (var##Index < 0)                                               \
   {                                                                 \
      Con::errorf( "TSShapeConstructor::" #func ": Could not find "  \
         "node '%s'", name);                                         \
      return ret;                                                    \
   }                                                                 \
   TSShape::Node* var = &(mShape->nodes[var##Index]);                \
   TORQUE_UNUSED(var##Index);                                        \
   TORQUE_UNUSED(var)

// Do an object lookup
#define GET_OBJECT(func, var, name, ret)                             \
   S32 var##Index = mShape->findObject(name);                        \
   if (var##Index < 0)                                               \
   {                                                                 \
      Con::errorf( "TSShapeConstructor::" #func ": Could not find "  \
         "object '%s'", name);                                       \
      return ret;                                                    \
   }                                                                 \
   TSShape::Object* var = &(mShape->objects[var##Index]);            \
   TORQUE_UNUSED(var##Index);                                        \
   TORQUE_UNUSED(var)

// Do a mesh lookup
#define GET_MESH(func, var, name, ret)                               \
   TSMesh* var = mShape->findMesh(name);                             \
   if (!var)                                                         \
   {                                                                 \
      Con::errorf( "TSShapeConstructor::" #func ": Could not find "  \
         "mesh '%s'", name);                                         \
      return ret;                                                    \
   }

// Do a sequence lookup
#define GET_SEQUENCE(func, var, name, ret)                           \
   S32 var##Index = mShape->findSequence(name);                      \
   if (var##Index < 0)                                               \
   {                                                                 \
      Con::errorf( "TSShapeConstructor::" #func ": Could not find "  \
         "sequence named '%s'", name);                               \
      return ret;                                                    \
   }                                                                 \
   TSShape::Sequence* var = &(mShape->sequences[var##Index]);        \
   TORQUE_UNUSED(var##Index);                                        \
   TORQUE_UNUSED(var);


//-----------------------------------------------------------------------------
// DUMP
DefineTSShapeConstructorMethod( dumpShape, void, ( const char* filename ), ( "" ),
   ( filename ),,
   "Dump the shape hierarchy to the console or to a file. Useful for reviewing "
   "the result of a series of construction commands.\n"
   "@param filename Destination filename. If not specified, dump to console.\n\n"
   "@tsexample\n"
   "%this.dumpShape();               // dump to console\n"
   "%this.dumpShape( \"./dump.txt\" ); // dump to file\n"
   "@endtsexample\n" )
{
   TSShapeInstance* tsi = new TSShapeInstance( mShape, false );

   if ( dStrEqual( filename, "" ) )
   {
      // Dump the constructed shape to a memory stream
      MemStream* dumpStream = new MemStream( 8192 );
      tsi->dump( *dumpStream );

      // Write stream to the console
      U32 end = dumpStream->getPosition();
      dumpStream->setPosition( 0 );
      while ( dumpStream->getPosition() < end )
      {
         char line[1024];
         dumpStream->readLine( (U8*)line, sizeof(line) );
         Con::printf( line );
      }

      delete dumpStream;
   }
   else
   {
      // Dump constructed shape to file
      char filenameBuf[1024];
      Con::expandScriptFilename( filenameBuf, sizeof(filenameBuf), filename );

      FileStream* dumpStream = new FileStream;
      if ( dumpStream->open( filenameBuf, Torque::FS::File::Write ) )
      {
         tsi->dump( *dumpStream );
         dumpStream->close();
      }
      else
         Con::errorf( "dumpShape failed: Could not open file '%s' for writing", filenameBuf );

      delete dumpStream;
   }

   delete tsi;
}}

DefineTSShapeConstructorMethod( saveShape, void, ( const char* filename ),,
   ( filename ),,
   "Save the shape (with all current changes) to a new DTS file.\n"
   "@param filename Destination filename.\n\n"
   "@tsexample\n"
   "%this.saveShape( \"./myShape.dts\" );\n"
   "@endtsexample\n" )
{
   char filenameBuf[1024];
   Con::expandScriptFilename( filenameBuf, sizeof(filenameBuf), filename );

   FileStream* dtsStream = new FileStream;
   if ( dtsStream->open( filenameBuf, Torque::FS::File::Write ) )
   {
      mShape->write( dtsStream );
      dtsStream->close();
   }
   else
   {
      Con::errorf( "saveShape failed: Could not open '%s' for writing", filenameBuf );
   }
   delete dtsStream;
}}

DefineTSShapeConstructorMethod( writeChangeSet, void, (),,
   (),,
   "Write the current change set to a TSShapeConstructor script file. The "
   "name of the script file is the same as the model, but with .cs extension. "
   "eg. myShape.cs for myShape.dts or myShape.dae.\n" )
{
   Torque::Path scriptPath( mShapePath );
   scriptPath.setExtension( "cs" );

   // Read current file contents
   FileObject f;
   f.readMemory( scriptPath.getFullPath() );

   // Write new file
   FileStream *stream;
   if ((stream = FileStream::createAndOpen( scriptPath.getFullPath(), Torque::FS::File::Write )) == NULL)
   {
      Con::errorf( "Failed to write TSShapeConstructor change set to %s", scriptPath.getFullPath().c_str() );
      return;
   }

   // Write existing file contents up to the start of the onLoad function
   String beginMessage( avar( "function %s::onLoad(%%this)", getName() ) );
   String endMessage( "}" );

   while ( !f.isEOF() )
   {
      const char* buffer = (const char *) f.readLine();
      if ( !dStrcmp( buffer, beginMessage ))
         break;
      stream->writeText( buffer );
      stream->writeText( "\r\n" );
   }

   // Write the new contents
   if ( f.isEOF() )
      stream->writeText( "\r\n" );
   stream->writeText( beginMessage );
   stream->writeText( "\r\n{\r\n" );

   mChangeSet.write( mShape, *stream, scriptPath.getPath() );

   stream->writeText( endMessage );
   stream->writeText( "\r\n" );

   // Now skip the contents of the function
   while ( !f.isEOF() )
   {
      const char* buffer = (const char *) f.readLine();
      if ( !dStrcmp( buffer, endMessage ))
         break;
   }

   // Write the remainder of the existing file contents
   while( !f.isEOF() )
   {
      const char* buffer = (const char *) f.readLine();
      stream->writeText( buffer );
      stream->writeText( "\r\n" );
   }

   delete stream;
}}

DefineTSShapeConstructorMethod( notifyShapeChanged, void, (),,
   (),,
   "Notify game objects that this shape file has changed, allowing them to update "
   "internal data if needed." )
{
   ResourceManager::get().getChangedSignal().trigger( mShapePath );
}}

//-----------------------------------------------------------------------------
// NODES
DefineTSShapeConstructorMethod( getNodeCount, S32, (),,
   (), 0,
   "Get the total number of nodes in the shape.\n"
   "@return the number of nodes in the shape.\n\n"
   "@tsexample\n"
   "%count = %this.getNodeCount();\n"
   "@endtsexample\n" )
{
   return mShape->nodes.size();
}}

DefineTSShapeConstructorMethod( getNodeIndex, S32, ( const char* name ),,
   ( name ), -1,
   "Get the index of the node.\n"
   "@param name name of the node to lookup.\n"
   "@return the index of the named node, or -1 if no such node exists.\n\n"
   "@tsexample\n"
   "// get the index of Bip01 Pelvis node in the shape\n"
   "%index = %this.getNodeIndex( \"Bip01 Pelvis\" );\n"
   "@endtsexample\n" )
{
   return mShape->findNode( name );
}}

DefineTSShapeConstructorMethod( getNodeName, const char*, ( S32 index ),,
   ( index ), "",
   "Get the name of the indexed node.\n"
   "@param index index of the node to lookup (valid range is 0 - getNodeCount()-1).\n"
   "@return the name of the indexed node, or \"\" if no such node exists.\n\n"
   "@tsexample\n"
   "// print the names of all the nodes in the shape\n"
   "%count = %this.getNodeCount();\n"
   "for (%i = 0; %i < %count; %i++)\n"
   "   echo(%i SPC %this.getNodeName(%i));\n"
   "@endtsexample\n" )
{
   CHECK_INDEX_IN_RANGE( getNodeName, index, mShape->nodes.size(), "" );
   return mShape->getName( mShape->nodes[index].nameIndex );
}}

DefineTSShapeConstructorMethod( getNodeParentName, const char*, ( const char* name ),,
   ( name ), "",
   "Get the name of the node's parent. If the node has no parent (ie. it is at "
   "the root level), return an empty string.\n"
   "@param name name of the node to query.\n"
   "@return the name of the node's parent, or \"\" if the node is at the root level\n\n"
   "@tsexample\n"
   "echo( \"Bip01 Pelvis parent = \" @ %this.getNodeParentName( \"Bip01 Pelvis \") );\n"
   "@endtsexample\n" )
{
   GET_NODE_INDEX_NO_ROOT( getNodeParentName, node, name, "" );

   if ( node->parentIndex < 0 )
      return "";
   else
      return mShape->getName( mShape->nodes[node->parentIndex].nameIndex );
}}

DefineTSShapeConstructorMethod( setNodeParent, bool, ( const char* name, const char* parentName ),,
   ( name, parentName ), false,
  "Set the parent of a node.\n"
  "@param name name of the node to modify\n"
  "@param parentName name of the parent node to set (use \"\" to move the node to the root level)\n"
  "@return true if successful, false if failed\n\n"
  "@tsexample\n"
  "%this.setNodeParent( \"Bip01 Pelvis\", \"start01\" );\n"
  "@endtsexample\n" )
{
   GET_NODE_INDEX_NO_ROOT( setNodeParent, node, name, false );
   GET_NODE_INDEX_ALLOW_ROOT( setNodeParent, parent, parentName, false );

   node->parentIndex = parentIndex;
   ADD_TO_CHANGE_SET();

   return true;
}}

DefineTSShapeConstructorMethod( getNodeChildCount, S32, ( const char* name ),,
   ( name ), 0,
   "Get the number of children of this node.\n"
   "@param name name of the node to query.\n"
   "@return the number of child nodes.\n\n"
   "@tsexample\n"
   "%count = %this.getNodeChildCount( \"Bip01 Pelvis\" );\n"
   "@endtsexample\n" )
{
   GET_NODE_INDEX_ALLOW_ROOT( getNodeChildCount, node, name, 0 );

   Vector<S32> nodeChildren;
   mShape->getNodeChildren( nodeIndex, nodeChildren );
   return nodeChildren.size();
}}

DefineTSShapeConstructorMethod( getNodeChildName, const char*, ( const char* name, S32 index ),,
   ( name, index ), "",
   "Get the name of the indexed child node.\n"
   "@param name name of the parent node to query.\n"
   "@param index index of the child node (valid range is 0 - getNodeChildName()-1).\n"
   "@return the name of the indexed child node.\n\n"
   "@tsexample\n"
   "function dumpNode( %shape, %name, %indent )\n"
   "{\n"
   "   echo( %indent @ %name );\n"
   "   %count = %shape.getNodeChildCount( %name );\n"
   "   for ( %i = 0; %i < %count; %i++ )\n"
   "      dumpNode( %shape, %shape.getNodeChildName( %name, %i ), %indent @ \"   \" );\n"
   "}\n\n"
   "function dumpShape( %shape )\n"
   "{\n"
   "   // recursively dump node hierarchy\n"
   "   %count = %shape.getNodeCount();\n"
   "   for ( %i = 0; %i < %count; %i++ )\n"
   "   {\n"
   "      // dump top level nodes\n"
   "      %name = %shape.getNodeName( %i );\n"
   "      if ( %shape.getNodeParentName( %name ) $= "" )\n"
   "         dumpNode( %shape, %name, \"\" );\n"
   "   }\n"
   "}\n"
   "@endtsexample\n" )
{
   GET_NODE_INDEX_ALLOW_ROOT( getNodeChildName, node, name, "" );

   Vector<S32> nodeChildren;
   mShape->getNodeChildren( nodeIndex, nodeChildren );
   CHECK_INDEX_IN_RANGE( getNodeChildName, index, nodeChildren.size(), "" );

   return mShape->getName( mShape->nodes[nodeChildren[index]].nameIndex );
}}

DefineTSShapeConstructorMethod( getNodeObjectCount, S32, ( const char* name ),,
   ( name ), 0,
   "Get the number of geometry objects attached to this node.\n"
   "@param name name of the node to query.\n"
   "@return the number of attached objects.\n\n"
   "@tsexample\n"
   "%count = %this.getNodeObjectCount( \"Bip01 Head\" );\n"
   "@endtsexample\n" )
{
   GET_NODE_INDEX_ALLOW_ROOT( getNodeObjectCount, node, name, 0 );

   Vector<S32> nodeObjects;
   mShape->getNodeObjects( nodeIndex, nodeObjects );
   return nodeObjects.size();
}}

DefineTSShapeConstructorMethod( getNodeObjectName, const char*, ( const char* name, S32 index ),,
   ( name, index ), "",
   "Get the name of the indexed object.\n"
   "@param name name of the node to query.\n"
   "@param index index of the object (valid range is 0 - getNodeObjectCount()-1).\n"
   "@return the name of the indexed object.\n\n"
   "@tsexample\n"
   "// print the names of all objects attached to the node\n"
   "%count = %this.getNodeObjectCount( \"Bip01 Head\" );\n"
   "for ( %i = 0; %i < %count; %i++ )\n"
   "   echo( %this.getNodeObjectName( \"Bip01 Head\", %i ) );\n"
   "@endtsexample\n" )
{
   GET_NODE_INDEX_ALLOW_ROOT( getNodeObjectName, node, name, "" );

   Vector<S32> nodeObjects;
   mShape->getNodeObjects( nodeIndex, nodeObjects );
   CHECK_INDEX_IN_RANGE( getNodeObjectName, index, nodeObjects.size(), "" );

   return mShape->getName( mShape->objects[nodeObjects[index]].nameIndex );
}}

DefineTSShapeConstructorMethod( getNodeTransform, TransformF, ( const char* name, bool isWorld ), ( false ),
   ( name, isWorld ), TransformF::Identity,
   "Get the base (ie. not animated) transform of a node.\n"
   "@param name name of the node to query.\n"
   "@param isWorld true to get the global transform, false (or omitted) to get "
   "the local-to-parent transform.\n"
   "@return the node transform in the form \"pos.x pos.y pos.z rot.x rot.y rot.z rot.angle\".\n\n"
   "@tsexample\n"
   "%ret = %this.getNodeTransform( \"mount0\" );\n"
   "%this.setNodeTransform( \"mount4\", %ret );\n"
   "@endtsexample\n" )
{
   GET_NODE_INDEX_NO_ROOT( getNodeTransform, node, name, TransformF::Identity );

   // Get the node transform
   Point3F pos;
   AngAxisF aa;

   if ( isWorld )
   {
      // World transform
      MatrixF mat;
      mShape->getNodeWorldTransform( nodeIndex, &mat );
      pos = mat.getPosition();
      aa.set( mat );
   }
   else
   {
      // Local transform
      pos = mShape->defaultTranslations[nodeIndex];
      const Quat16& q16 = mShape->defaultRotations[nodeIndex];
      aa.set( q16.getQuatF() );
   }

   return TransformF( pos, aa );
}}

DefineTSShapeConstructorMethod( setNodeTransform, bool, ( const char* name, TransformF txfm, bool isWorld ), ( false ),
   ( name, txfm, isWorld ), false,
   "Set the base transform of a node. That is, the transform of the node when "
   "in the root (not-animated) pose.\n"
   "@param name name of the node to modify\n"
   "@param txfm transform string of the form: \"pos.x pos.y pos.z rot.x rot.y rot.z rot.angle\"\n"
   "@param isworld (optional) flag to set the local-to-parent or the global "
   "transform. If false, or not specified, the position and orientation are "
   "treated as relative to the node's parent.\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.setNodeTransform( \"mount0\", \"0 0 1 0 0 1 0\" );\n"
   "%this.setNodeTransform( \"mount0\", \"0 0 0 0 0 1 1.57\" );\n"
   "%this.setNodeTransform( \"mount0\", \"1 0 0 0 0 1 0\", true );\n"
   "@endtsexample\n" )
{
   GET_NODE_INDEX_NO_ROOT( setNodeTransform, node, name, false );

   Point3F pos( txfm.getPosition() );
   QuatF rot( txfm.getOrientation() );

   if ( isWorld )
   {
      // World transform

      // Get the node's parent (if any)
      if ( node->parentIndex != -1 )
      {
         MatrixF mat;
         mShape->getNodeWorldTransform( node->parentIndex, &mat );

         // Pre-multiply by inverse of parent's world transform to get
         // local node transform
         mat.inverse();
         mat.mul( txfm.getMatrix() );

         rot.set( mat );
         pos = mat.getPosition();
      }
   }

   if ( !mShape->setNodeTransform( name, pos, rot) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( renameNode, bool, ( const char* oldName, const char* newName ),,
   ( oldName, newName ), false,
   "Rename a node.\n"
   "@note Note that node names must be unique, so this command will fail if "
   "there is already a node with the desired name\n"
   "@param oldName current name of the node\n"
   "@param newName new name of the node\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.renameNode( \"Bip01 L Hand\", \"mount5\" );\n"
   "@endtsexample\n" )
{
   GET_NODE_INDEX_NO_ROOT( renameNode, node, oldName, false );

   if ( !mShape->renameNode( oldName, newName ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( addNode, bool, ( const char* name, const char* parentName, TransformF txfm, bool isWorld ), ( TransformF::Identity, false ),
   ( name, parentName, txfm, isWorld ), false,
   "Add a new node.\n"
   "@param name name for the new node (must not already exist)\n"
   "@param parentName name of an existing node to be the parent of the new node. "
   "If empty (\"\"), the new node will be at the root level of the node hierarchy.\n"
   "@param txfm (optional) transform string of the form: \"pos.x pos.y pos.z rot.x rot.y rot.z rot.angle\"\n"
   "@param isworld (optional) flag to set the local-to-parent or the global "
   "transform. If false, or not specified, the position and orientation are "
   "treated as relative to the node's parent.\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.addNode( \"Nose\", \"Bip01 Head\", \"0 2 2 0 0 1 0\" );\n"
   "%this.addNode( \"myRoot\", \"\", \"0 0 4 0 0 1 1.57\" );\n"
   "%this.addNode( \"Nodes\", \"Bip01 Head\", \"0 2 0 0 0 1 0\", true );\n"
   "@endtsexample\n" )
{
   Point3F pos( txfm.getPosition() );
   QuatF rot( txfm.getOrientation() );

   if ( isWorld )
   {
      // World transform

      // Get the node's parent (if any)
      S32 parentIndex = mShape->findNode( parentName );
      if ( parentIndex != -1 )
      {
         MatrixF mat;
         mShape->getNodeWorldTransform( parentIndex, &mat );

         // Pre-multiply by inverse of parent's world transform to get
         // local node transform
         mat.inverse();
         mat.mul( txfm.getMatrix() );

         rot.set( mat );
         pos = mat.getPosition();
      }
   }

   if ( !mShape->addNode( name, parentName, pos, rot ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( removeNode, bool, ( const char* name ),,
   ( name ), false,
   "Remove a node from the shape.\n"
   "The named node is removed from the shape, including from any sequences that "
   "use the node. Child nodes and objects attached to the node are re-assigned "
   "to the node's parent.\n"
   "@param name name of the node to remove.\n"
   "@return true if successful, false otherwise.\n\n"
   "@tsexample\n"
   "%this.removeNode( \"Nose\" );\n"
   "@endtsexample\n" )
{
   GET_NODE_INDEX_NO_ROOT( removeNode, node, name, false );

   if ( !mShape->removeNode( name ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

//-----------------------------------------------------------------------------
// MATERIALS

DefineTSShapeConstructorMethod( getTargetCount, S32, (),, (), 0,
   "Get the number of materials in the shape.\n"
   "@return the number of materials in the shape.\n\n"
   "@tsexample\n"
   "%count = %this.getTargetCount();\n"
   "@endtsexample\n" )
{
   return mShape->getTargetCount();
}}

DefineTSShapeConstructorMethod( getTargetName, const char*, ( S32 index ),,
   ( index ), "",
   "Get the name of the indexed shape material.\n"
   "@param index index of the material to get (valid range is 0 - getTargetCount()-1).\n"
   "@return the name of the indexed material.\n\n"
   "@tsexample\n"
   "%count = %this.getTargetCount();\n"
   "for ( %i = 0; %i < %count; %i++ )\n"
   "   echo( \"Target \" @ %i @ \": \" @ %this.getTargetName( %i ) );\n"
   "@endtsexample\n" )
{
   return mShape->getTargetName( index );
}}

//-----------------------------------------------------------------------------
// OBJECTS

DefineTSShapeConstructorMethod( getObjectCount, S32, (),, (), 0,
   "Get the total number of objects in the shape.\n"
   "@return the number of objects in the shape.\n\n"
   "@tsexample\n"
   "%count = %this.getObjectCount();\n"
   "@endtsexample\n" )
{
   return mShape->objects.size();
}}

DefineTSShapeConstructorMethod( getObjectName, const char*, ( S32 index ),,
   ( index ), "",
   "Get the name of the indexed object.\n"
   "@param index index of the object to get (valid range is 0 - getObjectCount()-1).\n"
   "@return the name of the indexed object.\n\n"
   "@tsexample\n"
   "// print the names of all objects in the shape\n"
   "%count = %this.getObjectCount();\n"
   "for ( %i = 0; %i < %count; %i++ )\n"
   "   echo( %i SPC %this.getObjectName( %i ) );\n"
   "@endtsexample\n" )
{
   CHECK_INDEX_IN_RANGE( getObjectName, index, mShape->objects.size(), "" );

   return mShape->getName( mShape->objects[index].nameIndex );
}}

DefineTSShapeConstructorMethod( getObjectIndex, S32, ( const char* name ),,
   ( name ), -1,
   "Get the index of the first object with the given name.\n"
   "@param name name of the object to get.\n"
   "@return the index of the named object.\n\n"
   "@tsexample\n"
   "%index = %this.getObjectIndex( \"Head\" );\n"
   "@endtsexample\n" )
{
   return mShape->findObject( name );
}}

DefineTSShapeConstructorMethod( getObjectNode, const char*, ( const char* name ),,
   ( name ), "",
   "Get the name of the node this object is attached to.\n"
   "@param name name of the object to get.\n"
   "@return the name of the attached node, or an empty string if this "
   "object is not attached to a node (usually the case for skinned meshes).\n\n"
   "@tsexample\n"
   "echo( \"Hand is attached to \" @ %this.getObjectNode( \"Hand\" ) );\n"
   "@endtsexample\n" )
{
   GET_OBJECT( getObjectNode, obj, name, 0 );
   if ( obj->nodeIndex < 0 )
      return "";
   else
      return mShape->getName( mShape->nodes[obj->nodeIndex].nameIndex );
}}

DefineTSShapeConstructorMethod( setObjectNode, bool, ( const char* objName, const char* nodeName ),,
   ( objName, nodeName ), false,
   "Set the node an object is attached to.\n"
   "When the shape is rendered, the object geometry is rendered at the node's "
   "current transform.\n"
   "@param objName name of the object to modify\n"
   "@param nodeName name of the node to attach the object to\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.setObjectNode( \"Hand\", \"Bip01 LeftHand\" );\n"
   "@endtsexample\n" )
{
   if ( !mShape->setObjectNode( objName, nodeName ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( renameObject, bool, ( const char* oldName, const char* newName ),,
   ( oldName, newName ), false,
   "Rename an object.\n"
   "@note Note that object names must be unique, so this command will fail if "
   "there is already an object with the desired name\n"
   "@param oldName current name of the object\n"
   "@param newName new name of the object\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.renameObject( \"MyBox\", \"Box\" );\n"
   "@endtsexample\n" )
{
   if ( !mShape->renameObject( oldName, newName ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( removeObject, bool, ( const char* name ),,
   ( name ), false,
   "Remove an object (including all meshes for that object) from the shape.\n"
   "@param name name of the object to remove.\n"
   "@return true if successful, false otherwise.\n\n"
   "@tsexample\n"
   "// clear all objects in the shape\n"
   "%count = %this.getObjectCount();\n"
   "for ( %i = %count-1; %i >= 0; %i-- )\n"
   "   %this.removeObject( %this.getObjectName(%i) );\n"
   "@endtsexample\n" )
{
   if ( !mShape->removeObject( name ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

//-----------------------------------------------------------------------------
// MESHES
DefineTSShapeConstructorMethod( getMeshCount, S32, ( const char* name ),,
   ( name ), 0,
   "Get the number of meshes (detail levels) for the specified object.\n"
   "@param name name of the object to query\n"
   "@return the number of meshes for this object.\n\n"
   "@tsexample\n"
   "%count = %this.getMeshCount( \"SimpleShape\" );\n"
   "@endtsexample\n" )
{
   GET_OBJECT( getMeshCount, obj, name, 0 );

   Vector<S32> objectDetails;
   mShape->getObjectDetails( objIndex, objectDetails );
   return objectDetails.size();
}}

DefineTSShapeConstructorMethod( getMeshName, const char*, ( const char* name, S32 index ),,
   ( name, index ), "",
   "Get the name of the indexed mesh (detail level) for the specified object.\n"
   "@param name name of the object to query\n"
   "@param index index of the mesh (valid range is 0 - getMeshCount()-1)\n"
   "@return the mesh name.\n\n"
   "@tsexample\n"
   "// print the names of all meshes in the shape\n"
   "%objCount = %this.getObjectCount();\n"
   "for ( %i = 0; %i < %objCount; %i++ )\n"
   "{\n"
   "   %objName = %this.getObjectName( %i );\n"
   "   %meshCount = %this.getMeshCount( %objName );\n"
   "   for ( %j = 0; %j < %meshCount; %j++ )\n"
   "      echo( %this.getMeshName( %objName, %j ) );\n"
   "}\n"
   "@endtsexample\n" )
{
   GET_OBJECT( getMeshName, obj, name, "" );

   Vector<S32> objectDetails;
   mShape->getObjectDetails(objIndex, objectDetails);

   CHECK_INDEX_IN_RANGE( getMeshName, index, objectDetails.size(), "" );

   static const U32 bufSize = 256;
   char* returnBuffer = Con::getReturnBuffer(bufSize);
   dSprintf(returnBuffer, bufSize, "%s %d", name, (S32)mShape->details[objectDetails[index]].size);
   return returnBuffer;
}}

DefineTSShapeConstructorMethod( getMeshSize, S32, ( const char* name, S32 index ),,
   ( name, index ), -1,
   "Get the detail level size of the indexed mesh for the specified object.\n"
   "@param name name of the object to query\n"
   "@param index index of the mesh (valid range is 0 - getMeshCount()-1)\n"
   "@return the mesh detail level size.\n\n"
   "@tsexample\n"
   "// print sizes for all detail levels of this object\n"
   "%objName = \"trunk\";\n"
   "%count = %this.getMeshCount( %objName );\n"
   "for ( %i = 0; %i < %count; %i++ )\n"
   "   echo( %this.getMeshSize( %objName, %i ) );\n"
   "@endtsexample\n" )
{
   GET_OBJECT( getMeshName, obj, name, -1 );

   Vector<S32> objectDetails;
   mShape->getObjectDetails( objIndex, objectDetails );

   CHECK_INDEX_IN_RANGE( getMeshName, index, objectDetails.size(), -1 );

   return (S32)mShape->details[objectDetails[index]].size;
}}

DefineTSShapeConstructorMethod( setMeshSize, bool, ( const char* name, S32 size ),,
   ( name, size ), false,
   "Change the detail level size of the named mesh.\n"
   "@param name full name (object name + current size ) of the mesh to modify\n"
   "@param size new detail level size\n"
   "@return true if successful, false otherwise.\n\n"
   "@tsexample\n"
   "%this.setMeshSize( \"SimpleShape128\", 64 );\n"
   "@endtsexample\n" )
{
   if ( !mShape->setMeshSize( name, size ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( getMeshType, const char*, ( const char* name ),,
   ( name ), "",
   "Get the display type of the mesh.\n"
   "@param name name of the mesh to query\n"
   "@return the string returned is one of:"
   "<dl><dt>normal</dt><dd>a normal 3D mesh</dd>"
   "<dt>billboard</dt><dd>a mesh that always faces the camera</dd>"
   "<dt>billboardzaxis</dt><dd>a mesh that always faces the camera in the Z-axis</dd></dl>\n\n"
   "@tsexample\n"
   "echo( \"Mesh type is \" @ %this.getMeshType( \"SimpleShape128\" ) );\n"
   "@endtsexample\n" )
{
   GET_MESH( getMeshType, mesh, name, "normal" );

   if (mesh->getFlags(TSMesh::BillboardZAxis))
      return "billboardzaxis";
   else if (mesh->getFlags(TSMesh::Billboard))
      return "billboard";
   else
      return "normal";
}}

DefineTSShapeConstructorMethod( setMeshType, bool, ( const char* name, const char* type ),,
   ( name, type ), false,
   "Set the display type for the mesh.\n"
   "@param name full name (object name + detail size) of the mesh to modify\n"
   "@param type the new type for the mesh: \"normal\", \"billboard\" or \"billboardzaxis\"\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "// set the mesh to be a billboard\n"
   "%this.setMeshType( \"SimpleShape64\", \"billboard\" );\n"
   "@endtsexample\n" )
{
   GET_MESH( setMeshType, mesh, name, false );

   // Update the mesh flags
   mesh->clearFlags( TSMesh::Billboard | TSMesh::BillboardZAxis );
   if ( dStrEqual( type, "billboard" ) )
      mesh->setFlags( TSMesh::Billboard );
   else if ( dStrEqual( type, "billboardzaxis" ) )
      mesh->setFlags( TSMesh::Billboard | TSMesh::BillboardZAxis );
   else if ( !dStrEqual( type, "normal" ) )
   {
      Con::printf( "setMeshType: Unknown mesh type '%s'", type );
      return false;
   }

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( getMeshMaterial, const char*, ( const char* name ),,
   ( name ), "",
   "Get the name of the material attached to a mesh. Note that only the first "
   "material used by the mesh is returned.\n"
   "@param name full name (object name + detail size) of the mesh to query\n"
   "@return name of the material attached to the mesh (suitable for use with the Material mapTo field)\n\n"
   "@tsexample\n"
   "echo( \"Mesh material is \" @ %this.sgetMeshMaterial( \"SimpleShape128\" ) );\n"
   "@endtsexample\n" )
{
   GET_MESH( getMeshMaterial, mesh, name, "" );

   // Return the name of the first material attached to this mesh
   S32 matIndex = mesh->primitives[0].matIndex & TSDrawPrimitive::MaterialMask;
   if ((matIndex >= 0) && (matIndex < mShape->materialList->size()))
      return mShape->materialList->getMaterialName( matIndex );
   else
      return "";
}}

DefineTSShapeConstructorMethod( setMeshMaterial, bool, ( const char* meshName, const char* matName ),,
   ( meshName, matName ), false,
   "Set the name of the material attached to the mesh.\n"
   "@param meshName full name (object name + detail size) of the mesh to modify\n"
   "@param matName name of the material to attach. This could be the base name of "
   "the diffuse texture (eg. \"test_mat\" for \"test_mat.jpg\"), or the name of a "
   "Material object already defined in script.\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "// set the mesh material\n"
   "%this.setMeshMaterial( \"SimpleShape128\", \"test_mat\" );\n"
   "@endtsexample\n" )
{
   GET_MESH( setMeshMaterial, mesh, meshName, false );

   // Check if this material is already in the shape
   S32 matIndex;
   for ( matIndex = 0; matIndex < mShape->materialList->size(); matIndex++ )
   {
      if ( dStrEqual( matName, mShape->materialList->getMaterialName( matIndex ) ) )
         break;
   }
   if ( matIndex == mShape->materialList->size() )
   {
      // Add a new material to the shape
      U32 flags = TSMaterialList::S_Wrap | TSMaterialList::T_Wrap;
      mShape->materialList->push_back( matName, flags );
   }

   // Set this material for all primitives in the mesh
   for ( S32 i = 0; i < mesh->primitives.size(); i++ )
   {
      U32 matType = mesh->primitives[i].matIndex & ( TSDrawPrimitive::TypeMask | TSDrawPrimitive::Indexed );
      mesh->primitives[i].matIndex = ( matType | matIndex );
   }

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( addMesh, bool, ( const char* meshName, const char* srcShape, const char* srcMesh ),,
   ( meshName, srcShape, srcMesh ), false,
   "Add geometry from another DTS or DAE shape file into this shape.\n"
   "Any materials required by the source mesh are also copied into this shape.<br>\n"
   "@param meshName full name (object name + detail size) of the new mesh. If "
      "no detail size is present at the end of the name, a value of 2 is used.<br>"
      "An underscore before the number at the end of the name will be interpreted as "
      "a negative sign. eg. \"MyMesh_4\" will be interpreted as \"MyMesh-4\".\n"
   "@param srcShape name of a shape file (DTS or DAE) that contains the mesh\n"
   "@param srcMesh the full name (object name + detail size) of the mesh to "
      "copy from the DTS/DAE file into this shape</li>"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.addMesh( \"ColMesh-1\", \"./collision.dts\", \"ColMesh\", \"Col-1\" );\n"
   "%this.addMesh( \"SimpleShape10\", \"./testShape.dae\", \"MyMesh2\", "" );\n"
   "@endtsexample\n" )
{
   // Load the shape source file
   char filenameBuf[1024];
   Con::expandScriptFilename(filenameBuf, sizeof(filenameBuf), srcShape);

   Resource<TSShape> hSrcShape = ResourceManager::get().load( filenameBuf );
   if ( !bool(hSrcShape) )
   {
      Con::errorf( "addMesh failed: Could not load source shape: '%s'", filenameBuf );
      return false;
   }

   TSShape* shape = const_cast<TSShape*>( (const TSShape*)hSrcShape );
   if ( !mShape->addMesh( shape, srcMesh, meshName ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( removeMesh, bool, ( const char* name ),,
   ( name ), false,
   "Remove a mesh from the shape.\n"
   "If all geometry is removed from an object, the object is also removed.\n"
   "@param name full name (object name + detail size) of the mesh to remove\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.removeMesh( \"SimpleShape128\" );\n"
   "@endtsexample\n" )
{
   if ( !mShape->removeMesh( name ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( getBounds, Box3F, (),,
   (), Box3F::Invalid,
   "Get the bounding box for the shape.\n"
   "@return Bounding box \"minX minY minZ maxX maxY maxZ\"" )
{
   return mShape->bounds;
}}

DefineTSShapeConstructorMethod( setBounds, bool, ( Box3F bbox ),,
   ( bbox ), false,
   "Set the shape bounds to the given bounding box.\n"
   "@param Bounding box \"minX minY minZ maxX maxY maxZ\"\n"
   "@return true if successful, false otherwise\n" )
{
   // Set shape bounds
   TSShape* shape = mShape;

   shape->bounds = bbox;
   shape->bounds.getCenter( &shape->center );
   shape->radius = ( shape->bounds.maxExtents - shape->center ).len();
   shape->tubeRadius = shape->radius;

   ADD_TO_CHANGE_SET();
   return true;
}}

//-----------------------------------------------------------------------------
// DETAILS
DefineTSShapeConstructorMethod( getDetailLevelCount, S32, (),, (), 0,
   "Get the total number of detail levels in the shape.\n"
   "@return the number of detail levels in the shape\n" )
{
   return mShape->details.size();
}}

DefineTSShapeConstructorMethod( getDetailLevelName, const char*, ( S32 index ),,
   ( index ), "",
   "Get the name of the indexed detail level.\n"
   "@param index detail level index (valid range is 0 - getDetailLevelCount()-1)\n"
   "@return the detail level name\n\n"
   "@tsexample\n"
   "// print the names of all detail levels in the shape\n"
   "%count = %this.getDetailLevelCount();\n"
   "for ( %i = 0; %i < %count; %i++ )\n"
   "   echo( %i SPC %this.getDetailLevelName( %i ) );\n"
   "@endtsexample\n" )
{
   CHECK_INDEX_IN_RANGE( getDetailLevelName, index, mShape->details.size(), "" );

   return mShape->getName(mShape->details[index].nameIndex);
}}

DefineTSShapeConstructorMethod( getDetailLevelSize, S32, ( S32 index),,
   ( index ), 0,
   "Get the size of the indexed detail level.\n"
   "@param index detail level index (valid range is 0 - getDetailLevelCount()-1)\n"
   "@return the detail level size\n\n"
   "@tsexample\n"
   "// print the sizes of all detail levels in the shape\n"
   "%count = %this.getDetailLevelCount();\n"
   "for ( %i = 0; %i < %count; %i++ )\n"
   "   echo( \"Detail\" @ %i @ \" has size \" @ %this.getDetailLevelSize( %i ) );\n"
   "@endtsexample\n" )
{
   CHECK_INDEX_IN_RANGE( getDetailLevelSize, index, mShape->details.size(), 0 );

   return (S32)mShape->details[index].size;
}}

DefineTSShapeConstructorMethod( getDetailLevelIndex, S32, ( S32 size ),,
   ( size ), -1,
   "Get the index of the detail level with a given size.\n"
   "@param size size of the detail level to lookup\n"
   "@return index of the detail level with the desired size, or -1 if no such "
   "detail exists\n\n"
   "@tsexample\n"
   "if ( %this.getDetailLevelSize( 32 ) == -1 )\n"
   "   echo( \"Error: This shape does not have a detail level at size 32\" );\n"
   "@endtsexample\n" )
{
   return mShape->findDetailBySize( size );
}}

DefineTSShapeConstructorMethod( renameDetailLevel, bool, ( const char* oldName, const char* newName ),,
   ( oldName, newName ), false,
   "Rename a detail level.\n"
   "@note Note that detail level names must be unique, so this command will "
   "fail if there is already a detail level with the desired name\n"
   "@param oldName current name of the detail level\n"
   "@param newName new name of the detail level\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.renameDetailLevel( \"detail-1\", \"collision-1\" );\n"
   "@endtsexample\n" )
{
   if ( !mShape->renameDetail( oldName, newName ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( removeDetailLevel, bool, ( S32 index ),,
   ( index ), false,
   "Remove the detail level (including all meshes in the detail level)\n"
   "@param size size of the detail level to remove\n"
   "@return true if successful, false otherwise\n"
   "@tsexample\n"
   "%this.removeDetailLevel( 2 );\n"
   "@endtsexample\n" )
{
   if ( !mShape->removeDetail( index ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( setDetailLevelSize, S32, ( S32 index, S32 newSize ),,
   ( index, newSize ), index,
   "Change the size of a detail level."
   "@note Note that detail levels are always sorted in decreasing size order, "
   "so this command may cause detail level indices to change.\n"
   "@param index index of the detail level to modify\n"
   "@param newSize new size for the detail level\n"
   "@return new index for this detail level\n\n"
   "@tsexample\n"
   "%this.setDetailLevelSize( 2, 256 );\n"
   "@endtsexample\n" )
{
   S32 dl = mShape->setDetailSize( index, newSize );
   if ( dl >= 0 )
      ADD_TO_CHANGE_SET();
   return dl;
}}

DefineTSShapeConstructorMethod( getImposterDetailLevel, S32, (),, (), -1,
   "Get the index of the imposter (auto-billboard) detail level (if any).\n"
   "@return imposter detail level index, or -1 if the shape does not use "
   "imposters.\n\n" )
{
   for ( S32 i = 0; i < mShape->details.size(); i++ )
   {
      if ( mShape->details[i].subShapeNum < 0 )
         return i;
   }
   return -1;
}}

DefineTSShapeConstructorMethod( getImposterSettings, const char*, ( S32 index ),,
   ( index ), "",
   "Get the settings used to generate imposters for the indexed detail level.\n"
   "@param index index of the detail level to query (does not need to be an "
   "imposter detail level\n"
   "@return string of the form: \"valid eqSteps pSteps dl dim poles angle\", where:"
   "<dl>"
      "<dt>valid</dt><dd>1 if this detail level generates imposters, 0 otherwise</dd>"
      "<dt>eqSteps</dt><dd>number of steps around the equator</dd>"
      "<dt>pSteps</dt><dd>number of steps between the poles</dd>"
      "<dt>dl</dt><dd>index of the detail level used to generate imposters</dd>"
      "<dt>dim</dt><dd>size (in pixels) of each imposter image</dd>"
      "<dt>poles</dt><dd>1 to include pole images, 0 otherwise</dd>"
      "<dt>angle</dt><dd>angle at which to display pole images</dd>"
   "</dl>\n\n"
   "@tsexample\n"
   "// print the imposter detail level settings\n"
   "%index = %this.getImposterDetailLevel();\n"
   "if ( %index != -1 )\n"
   "   echo( \"Imposter settings: \" @ %this.getImposterSettings( %index ) );\n"
   "@endtsexample\n" )
{
   CHECK_INDEX_IN_RANGE( getImposterSettings, index, mShape->details.size(), "" );

   // Return information about the detail level
   const TSShape::Detail& det = mShape->details[index];

   static const U32 bufSize = 512;
   char* returnBuffer = Con::getReturnBuffer(bufSize);
   dSprintf(returnBuffer, bufSize, "%d\t%d\t%d\t%d\t%d\t%d\t%g",
      (S32)( det.subShapeNum < 0 ),          // isImposter
      det.bbEquatorSteps,
      det.bbPolarSteps,
      det.bbDetailLevel,
      det.bbDimension,
      det.bbIncludePoles,
      det.bbPolarAngle );

   return returnBuffer;
}}

DefineTSShapeConstructorMethod( addImposter, S32, ( S32 size, S32 equatorSteps, S32 polarSteps, S32 dl, S32 dim, bool includePoles, F32 polarAngle ),,
   ( size, equatorSteps, polarSteps, dl, dim, includePoles, polarAngle ), -1,
   "Add (or edit) an imposter detail level to the shape.\n"
   "If the shape already contains an imposter detail level, this command will "
   "simply change the imposter settings\n"
   "@param size size of the imposter detail level\n"
   "@param equatorSteps defines the number of snapshots to take around the "
   "equator. Imagine the object being rotated around the vertical axis, then "
   "a snapshot taken at regularly spaced intervals.\n"
   "@param polarSteps defines the number of snapshots taken between the poles "
   "(top and bottom), at each equator step. eg. At each equator snapshot, "
   "snapshots are taken at regular intervals between the poles.\n"
   "@param dl the detail level to use when generating the snapshots. Note that "
   "this is an array index rather than a detail size. So if an object has detail "
   "sizes of: 200, 150, and 40, then setting @a dl to 1 will generate the snapshots "
   "using detail size 150.\n"
   "@param dim defines the size of the imposter images in pixels. The larger the "
   "number, the more detailed the billboard will be.\n"
   "@param includePoles flag indicating whether to include the \"pole\" snapshots. "
   "ie. the views from the top and bottom of the object.\n"
   "@param polar_angle if pole snapshots are active (@a includePoles is true), this "
   "parameter defines the camera angle (in degrees) within which to render the "
   "pole snapshot. eg. if polar_angle is set to 25 degrees, then the snapshot "
   "taken at the pole (looking directly down or up at the object) will be rendered "
   "when the camera is within 25 degrees of the pole.\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.addImposter( 2, 4, 0, 0, 64, false, 0 );\n"
   "%this.addImposter( 2, 4, 2, 0, 64, true, 10 );   // this command would edit the existing imposter detail level\n"
   "@endtsexample\n" )
{
   // Add the imposter detail level
	dl = mShape->addImposter( getShapePath(), size, equatorSteps, polarSteps, dl, dim, includePoles, polarAngle);
   if ( dl != -1 )
      ADD_TO_CHANGE_SET();
   return dl;
}}

DefineTSShapeConstructorMethod( removeImposter, bool, (),, (), false,
   "() Remove the imposter detail level (if any) from the shape.\n"
   "@return true if successful, false otherwise\n\n" )
{
   if ( !mShape->removeImposter() )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

//-----------------------------------------------------------------------------
// SEQUENCES
DefineTSShapeConstructorMethod( getSequenceCount, S32, (),, (), 0,
   "Get the total number of sequences in the shape.\n"
   "@return the number of sequences in the shape\n\n" )
{
   return mShape->sequences.size();
}}

DefineTSShapeConstructorMethod( getSequenceIndex, S32, ( const char* name),,
   ( name ), -1,
   "Find the index of the sequence with the given name.\n"
   "@param name name of the sequence to lookup\n"
   "@return index of the sequence with matching name, or -1 if not found\n\n"
   "@tsexample\n"
   "// Check if a given sequence exists in the shape\n"
   "if ( %this.getSequenceIndex( \"walk\" ) == -1 )\n"
   "   echo( \"Could not find 'walk' sequence\" );\n"
   "@endtsexample\n" )
{
   return mShape->findSequence( name );
}}

DefineTSShapeConstructorMethod( getSequenceName, const char*, ( S32 index ),,
   ( index ), "",
   "Get the name of the indexed sequence.\n"
   "@param index index of the sequence to query (valid range is 0 - getSequenceCount()-1)\n"
   "@return the name of the sequence\n\n"
   "@tsexample\n"
   "// print the name of all sequences in the shape\n"
   "%count = %this.getSequenceCount();\n"
   "for ( %i = 0; %i < %count; %i++ )\n"
   "   echo( %i SPC %this.getSequenceName( %i ) );\n"
   "@endtsexample\n" )
{
   CHECK_INDEX_IN_RANGE( getSequenceName, index, mShape->sequences.size(), "" );

   return mShape->getName( mShape->sequences[index].nameIndex );
}}

DefineTSShapeConstructorMethod( getSequenceSource, const char*, ( const char* name ),,
   ( name ), "",
   "Get information about where the sequence data came from.\n"
   "For example, whether it was loaded from an external DSQ file.\n"
   "@param name name of the sequence to query\n"
   "@return TAB delimited string of the form: \"from reserved start end total\", where:"
   "<dl>"
      "<dt>from</dt><dd>the source of the animation data, such as the path to "
      "a DSQ file, or the name of an existing sequence in the shape. This field "
      "will be empty for sequences already embedded in the DTS or DAE file.</dd>"
      "<dt>reserved</dt><dd>reserved value</dd>"
      "<dt>start</dt><dd>the first frame in the source sequence used to create this sequence</dd>"
      "<dt>end</dt><dd>the last frame in the source sequence used to create this sequence</dd>"
      "<dt>total</dt><dd>the total number of frames in the source sequence</dd>"
   "</dl>\n\n"
   "@tsexample\n"
   "// print the source for the walk animation\n"
   "echo( \"walk source:\" SPC getField( %this.getSequenceSource( \"walk\" ), 0 ) );\n"
   "@endtsexample\n" )
{
   GET_SEQUENCE( getSequenceSource, seq, name, "" );

   // Return information about the source data for this sequence
   static const U32 bufSize = 512;
   char* returnBuffer = Con::getReturnBuffer(bufSize);
   dSprintf( returnBuffer, bufSize, "%s\t%d\t%d\t%d",
      seq->sourceData.from.c_str(), seq->sourceData.start,
      seq->sourceData.end, seq->sourceData.total );
   return returnBuffer;
}}

DefineTSShapeConstructorMethod( getSequenceFrameCount, S32, ( const char* name ),,
   ( name ), 0,
   "Get the number of keyframes in the sequence.\n"
   "@param name name of the sequence to query\n"
   "@return number of keyframes in the sequence\n\n"
   "@tsexample\n"
   "echo( \"Run has \" @ %this.getSequenceFrameCount( \"run\" ) @ \" keyframes\" );\n"
   "@endtsexample\n" )
{
   GET_SEQUENCE( getSequenceFrameCount, seq, name, 0 );
   return seq->numKeyframes;
}}

DefineTSShapeConstructorMethod( getSequencePriority, F32, ( const char* name ),,
   ( name ), -1.0f,
   "Get the priority setting of the sequence.\n"
   "@param name name of the sequence to query\n"
   "@return priority value of the sequence\n\n" )
{
   GET_SEQUENCE( getSequencePriority, seq, name, 0.0f );
   return seq->priority;
}}

DefineTSShapeConstructorMethod( setSequencePriority, bool, ( const char* name, F32 priority ),,
   ( name, priority ), false,
   "Set the sequence priority.\n"
   "@param name name of the sequence to modify\n"
   "@param priority new priority value\n"
   "@return true if successful, false otherwise\n\n" )
{
   GET_SEQUENCE( setSequencePriority, seq, name, false );

   seq->priority = priority;
   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( getSequenceGroundSpeed, const char*, ( const char* name ),,
   ( name ), "",
   "Get the ground speed of the sequence.\n"
   "@note Note that only the first 2 ground frames of the sequence are "
   "examined; the speed is assumed to be constant throughout the sequence.\n"
   "@param name name of the sequence to query\n"
   "@return string of the form: \"trans.x trans.y trans.z rot.x rot.y rot.z\"\n\n"
   "@tsexample\n"
   "%speed = VectorLen( getWords( %this.getSequenceGroundSpeed( \"run\" ), 0, 2 ) );\n"
   "   echo( \"Run moves at \" @ %speed @ \" units per frame\" );\n"
   "@endtsexample\n" )
{
   // Find the sequence and return the ground speed (assumed to be constant)
   GET_SEQUENCE( getSequenceGroundSpeed, seq, name, "" );

   Point3F trans(0,0,0), rot(0,0,0);
   if ( seq->numGroundFrames > 0 )
   {
      const Point3F& p1 = mShape->groundTranslations[seq->firstGroundFrame];
      const Point3F& p2 = mShape->groundTranslations[seq->firstGroundFrame + 1];
      trans = p2 - p1;

      QuatF r1 = mShape->groundRotations[seq->firstGroundFrame].getQuatF();
      QuatF r2 = mShape->groundRotations[seq->firstGroundFrame + 1].getQuatF();
      r2 -= r1;

      MatrixF mat;
      r2.setMatrix(&mat);
      rot = mat.toEuler();
   }

   static const U32 bufSize = 256;
   char* returnBuffer = Con::getReturnBuffer(bufSize);
   dSprintf( returnBuffer, bufSize, "%g %g %g %g %g %g",
      trans.x, trans.y, trans.z, rot.x, rot.y, rot.z );
   return returnBuffer;
}}

DefineTSShapeConstructorMethod( setSequenceGroundSpeed, bool, ( const char* name, Point3F transSpeed, Point3F rotSpeed ), ( Point3F::Zero ),
   ( name, transSpeed, rotSpeed ), false,
   "Set the translation and rotation ground speed of the sequence.\n"
   "The ground speed of the sequence is set by generating ground transform "
   "keyframes. The ground translational and rotational speed is assumed to "
   "be constant for the duration of the sequence. Existing ground frames for "
   "the sequence (if any) will be replaced.\n"
   "@param name name of the sequence to modify\n"
   "@param transSpeed translational speed (trans.x trans.y trans.z) in "
   "Torque units per frame\n"
   "@param rotSpeed (optional) rotational speed (rot.x rot.y rot.z) in "
   "radians per frame. Default is \"0 0 0\"\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.setSequenceGroundSpeed( \"run\", \"5 0 0\" );\n"
   "%this.setSequenceGroundSpeed( \"spin\", \"0 0 0\", \"4 0 0\" );\n"
   "@endtsexample\n" )
{
   if ( !mShape->setSequenceGroundSpeed( name, transSpeed, rotSpeed ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( getSequenceCyclic, bool, ( const char* name ),,
   ( name ), false,
   "Check if this sequence is cyclic (looping).\n"
   "@param name name of the sequence to query\n"
   "@return true if this sequence is cyclic, false if not\n\n"
   "@tsexample\n"
   "if ( !%this.getSequenceCyclic( \"ambient\" ) )\n"
   "   error( \"ambient sequence is not cyclic!\" );\n"
   "@endtsexample\n" )
{
   GET_SEQUENCE( getSequenceCyclic, seq, name, false );
   return seq->isCyclic();
}}

DefineTSShapeConstructorMethod( setSequenceCyclic, bool, ( const char* name, bool cyclic ),,
   ( name, cyclic ), false,
   "Mark a sequence as cyclic or non-cyclic.\n"
   "@param name name of the sequence to modify\n"
   "@param cyclic true to make the sequence cyclic, false for non-cyclic\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.setSequenceCyclic( \"ambient\", true );\n"
   "%this.setSequenceCyclic( \"shoot\", false );\n"
   "@endtsexample\n" )
{
   GET_SEQUENCE( setSequenceCyclic, seq, name, false );

   // update cyclic flag
   if (cyclic != seq->isCyclic())
   {
      if (cyclic && !seq->isCyclic())
         seq->flags |= TSShape::Cyclic;
      else if (!cyclic && seq->isCyclic())
         seq->flags &= (~(TSShape::Cyclic));

      ADD_TO_CHANGE_SET();
   }
   return true;
}}

DefineTSShapeConstructorMethod( getSequenceBlend, const char*, ( const char* name ),,
   ( name ), "",
   "Get information about blended sequences.\n"
   "@param name name of the sequence to query\n"
   "@return TAB delimited string of the form: \"isBlend blendSeq blendFrame\", where:"
   "<dl>"
   "<dt>blend_flag</dt><dd>a boolean flag indicating whether this sequence is a blend</dd>"
   "<dt>blend_seq_name</dt><dd>the name of the sequence that contains the reference "
   "frame (empty for blend sequences embedded in DTS files)</dd>"
   "<dt>blend_seq_frame</dt><dd>the blend reference frame (empty for blend sequences "
   "embedded in DTS files)</dd>"
   "</dl>\n"
   "@note Note that only sequences set to be blends using the setSequenceBlend "
   "command will contain the blendSeq and blendFrame information.\n\n"
   "@tsexample\n"
   "%blendData = %this.getSequenceBlend( \"look\" );\n"
   "if ( getField( %blendData, 0 ) )\n"
   "   echo( \"look is a blend, reference: \" @ getField( %blendData, 1 ) );\n"
   "@endtsexample\n" )
{
   GET_SEQUENCE( getSequenceBlend, seq, name, "0" );

   // Return the blend information (flag reference_sequence reference_frame)
   static const U32 bufSize = 512;
   char* returnBuffer = Con::getReturnBuffer(bufSize);
   dSprintf( returnBuffer, bufSize, "%d\t%s\t%d", (int)seq->isBlend(),
      seq->sourceData.blendSeq.c_str(), seq->sourceData.blendFrame );
   return returnBuffer;
}}

DefineTSShapeConstructorMethod( setSequenceBlend, bool, ( const char* name, bool blend, const char* blendSeq, S32 blendFrame ),,
   ( name, blend, blendSeq, blendFrame ), false,
   "Mark a sequence as a blend or non-blend.\n"
   "A blend sequence is one that will be added on top of any other playing "
   "sequences. This is done by storing the animated node transforms relative "
   "to a reference frame, rather than as absolute transforms.\n"
   "@param name name of the sequence to modify\n"
   "@param blend true to make the sequence a blend, false for a non-blend\n"
   "@param blendSeq the name of the sequence that contains the blend reference frame\n"
   "@param blendFrame the reference frame in the blendSeq sequence\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.setSequenceBlend( \"look\", true, \"root\", 0 );\n"
   "@endtsexample\n" )
{
   GET_SEQUENCE( setSequenceBlend, seq, name, false );

   if ( !mShape->setSequenceBlend( name, blend, blendSeq, blendFrame ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( renameSequence, bool, ( const char* oldName, const char* newName ),,
   ( oldName, newName ), false,
   "Rename a sequence.\n"
   "@note Note that sequence names must be unique, so this command will fail "
   "if there is already a sequence with the desired name\n"
   "@param oldName current name of the sequence\n"
   "@param newName new name of the sequence\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.renameSequence( \"walking\", \"walk\" );\n"
   "@endtsexample\n" )
{
   GET_SEQUENCE( renameSequence, seq, oldName, false );

   if ( !mShape->renameSequence( oldName, newName ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( addSequence, bool,
   ( const char* source, const char* name, S32 start, S32 end, bool padRot, bool padTrans ),
   ( 0, -1, true, false ), ( source, name, start, end, padRot, padTrans ), false,
   "Add a new sequence to the shape.\n"
   "@param source the name of an existing sequence, or the name of a DTS or DAE "
   "shape or DSQ sequence file. When the shape file contains more than one "
   "sequence, the desired sequence can be specified by appending the name to the "
   "end of the shape file. eg. \"myShape.dts run\" would select the \"run\" "
   "sequence from the \"myShape.dts\" file.\n\n"
   "@param name name of the new sequence\n"
   "@param start (optional) first frame to copy. Defaults to 0, the first frame in the sequence.\n"
   "@param end (optional) last frame to copy. Defaults to -1, the last frame in the sequence.\n"
   "@param padRot (optional) copy root-pose rotation keys for non-animated nodes. This is useful if "
   "the source sequence data has a different root-pose to the target shape, such as if one character was "
   "in the T pose, and the other had arms at the side. Normally only nodes that are actually rotated by "
   "the source sequence have keyframes added, but setting this flag will also add keyframes for nodes "
   "that are not animated, but have a different root-pose rotation to the target shape root pose.\n"
   "@param padTrans (optional) copy root-pose translation keys for non-animated nodes.  This is useful if "
   "the source sequence data has a different root-pose to the target shape, such as if one character was "
   "in the T pose, and the other had arms at the side. Normally only nodes that are actually moved by "
   "the source sequence have keyframes added, but setting this flag will also add keyframes for nodes "
   "that are not animated, but have a different root-pose position to the target shape root pose.\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.addSequence( \"./testShape.dts ambient\", \"ambient\" );\n"
   "%this.addSequence( \"./myPlayer.dae run\", \"run\" );\n"
   "%this.addSequence( \"./player_look.dsq\", \"look\", 0, -1 );     // start to end\n"
   "%this.addSequence( \"walk\", \"walk_shortA\", 0, 4 );            // start to frame 4\n"
   "%this.addSequence( \"walk\", \"walk_shortB\", 4, -1 );           // frame 4 to end\n"
   "@endtsexample\n" )
{
   String srcName;
   String srcPath( source );
   SplitSequencePathAndName( srcPath, srcName );

   if ( !mShape->addSequence( srcPath, srcName, name, start, end, padRot, padTrans ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( removeSequence, bool, ( const char* name ),,
   ( name ), false,
   "Remove the sequence from the shape.\n"
   "@param name name of the sequence to remove\n"
   "@return true if successful, false otherwise\n\n" )
{
   if ( !mShape->removeSequence( name ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

//-----------------------------------------------------------------------------
// TRIGGERS
DefineTSShapeConstructorMethod( getTriggerCount, S32, ( const char* name ),,
   ( name ), 0,
   "Get the number of triggers in the specified sequence.\n"
   "@param name name of the sequence to query\n"
   "@return number of triggers in the sequence\n\n" )
{
   GET_SEQUENCE( getTriggerCount, seq, name, 0 );
   return seq->numTriggers;
}}

DefineTSShapeConstructorMethod( getTrigger, const char*, ( const char* name, S32 index ),,
   ( name, index ), "",
   "Get information about the indexed trigger\n"
   "@param name name of the sequence to query\n"
   "@param index index of the trigger (valid range is 0 - getTriggerCount()-1)\n"
   "@return string of the form \"frame state\"\n\n"
   "@tsexample\n"
   "// print all triggers in the sequence\n"
   "%count = %this.getTriggerCount( \"back\" );\n"
   "for ( %i = 0; %i < %count; %i++ )\n"
   "   echo( %i SPC %this.getTrigger( \"back\", %i ) );\n"
   "@endtsexample\n" )
{  
   // Find the sequence and return the indexed trigger (frame and state)
   GET_SEQUENCE( getTrigger, seq, name, "" );

   CHECK_INDEX_IN_RANGE( getTrigger, index, seq->numTriggers, "" );

   const TSShape::Trigger& trig = mShape->triggers[seq->firstTrigger + index];
   S32 frame = trig.pos * seq->numKeyframes;
   S32 state = getBinLog2(trig.state & TSShape::Trigger::StateMask) + 1;
   if (!(trig.state & TSShape::Trigger::StateOn))
      state = -state;

   static const U32 bufSize = 32;
   char* returnBuffer = Con::getReturnBuffer(bufSize);
   dSprintf(returnBuffer, bufSize, "%d %d", frame, state);
   return returnBuffer;
}}

DefineTSShapeConstructorMethod( addTrigger, bool, ( const char* name, S32 keyframe, S32 state ),,
   ( name, keyframe, state ), false,
   "Add a new trigger to the sequence.\n"
   "@param name name of the sequence to modify\n"
   "@param keyframe keyframe of the new trigger\n"
   "@param state of the new trigger\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.addTrigger( \"walk\", 3, 1 );\n"
   "%this.addTrigger( \"walk\", 5, -1 );\n"
   "@endtsexample\n" )
{
   if ( !mShape->addTrigger( name, keyframe, state ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}

DefineTSShapeConstructorMethod( removeTrigger, bool, ( const char* name, S32 keyframe, S32 state ),,
   ( name, keyframe, state ), false,
   "Remove a trigger from the sequence.\n"
   "@param name name of the sequence to modify\n"
   "@param keyframe keyframe of the trigger to remove\n"
   "@param state of the trigger to remove\n"
   "@return true if successful, false otherwise\n\n"
   "@tsexample\n"
   "%this.removeTrigger( \"walk\", 3, 1 );\n"
   "@endtsexample\n" )
{
   if ( !mShape->removeTrigger( name, keyframe, state ) )
      return false;

   ADD_TO_CHANGE_SET();
   return true;
}}


//-----------------------------------------------------------------------------
// Change-Set manipulation
TSShapeConstructor::ChangeSet::eCommandType TSShapeConstructor::ChangeSet::getCmdType(const char* name)
{
   #define RETURN_IF_MATCH(type)   if (!dStricmp(name, #type)) return Cmd##type

   RETURN_IF_MATCH(AddNode);
   else RETURN_IF_MATCH(RemoveNode);
   else RETURN_IF_MATCH(RenameNode);
   else RETURN_IF_MATCH(SetNodeTransform);
   else RETURN_IF_MATCH(SetNodeParent);

   else RETURN_IF_MATCH(AddMesh);
   else RETURN_IF_MATCH(AddPrimitive);
   else RETURN_IF_MATCH(SetMeshSize);
   else RETURN_IF_MATCH(SetMeshType);
   else RETURN_IF_MATCH(SetMeshMaterial);
   else RETURN_IF_MATCH(RemoveMesh);

   else RETURN_IF_MATCH(SetObjectNode);
   else RETURN_IF_MATCH(RenameObject);
   else RETURN_IF_MATCH(RemoveObject);
   else RETURN_IF_MATCH(SetBounds);

   else RETURN_IF_MATCH(SetDetailLevelSize);
   else RETURN_IF_MATCH(RenameDetailLevel);
   else RETURN_IF_MATCH(RemoveDetailLevel);
   else RETURN_IF_MATCH(AddImposter);
   else RETURN_IF_MATCH(RemoveImposter);
   else RETURN_IF_MATCH(AddCollisionDetail);

   else RETURN_IF_MATCH(AddSequence);
   else RETURN_IF_MATCH(RemoveSequence);
   else RETURN_IF_MATCH(RenameSequence);
   else RETURN_IF_MATCH(SetSequenceCyclic);
   else RETURN_IF_MATCH(SetSequenceBlend);
   else RETURN_IF_MATCH(SetSequencePriority);
   else RETURN_IF_MATCH(SetSequenceGroundSpeed);

   else RETURN_IF_MATCH(AddTrigger);
   else RETURN_IF_MATCH(RemoveTrigger);

   else return CmdInvalid;

   #undef RETURN_IF_MATCH
}

void TSShapeConstructor::ChangeSet::write(TSShape* shape, Stream& stream, const String& savePath)
{
   // First make a copy of the change-set
   ChangeSet output;
   for ( S32 i = 0; i < mCommands.size(); i++ )
      output.add(mCommands[i]);

   // Remove all __backup__ sequences (used during Shape Editing)
   if (shape)
   {
      for (S32 i = 0; i < shape->sequences.size(); i++)
      {
         const char* seqName = shape->getName( shape->sequences[i].nameIndex );
         if ( dStrStartsWith( seqName, "__backup__" ) )
         {
            Command cmd( "removeSequence" );
            cmd.addArgs( seqName );
            output.add( cmd );
         }
      }
   }

   // Write the final change set to the stream
   for (U32 i = 0; i < output.mCommands.size(); i++)
   {
      const Command& cmd = output.mCommands[i];

      // Write the command
      stream.writeTabs( 1 );
      stream.writeText( "%this." );

      stream.writeText( cmd.name );
      stream.writeText( "(" );

      if ( cmd.argc > 0 )
      {
         // Use relative paths when possible
         String str( cmd.argv[0] );
         if ( str.startsWith( savePath ) )
         {
            // Need to add "./" to a local file for the script file system.  Otherwise
            // it will be assumed to be a full and complete path when it comes to loading.
            str = "./" + str.substr( savePath.length() + 1 );
         }

         stream.writeText( "\"" );
         stream.write( str.length(), str.c_str() );
         stream.writeText( "\"" );

         // Write remaining arguments and newline
         for (U32 j = 1; j < cmd.argc; j++)
         {
            // Use relative paths when possible
            String str( cmd.argv[j] );
            if ( str.startsWith( savePath ) )
               str = str.substr( savePath.length() + 1 );

            stream.writeText( ", \"" );
            stream.write( str.length(), str.c_str() );
            stream.writeText( "\"" );
         }
      }
      stream.writeText( ");\r\n" );
   }
}


TiXmlElement *createNodeWithText( const char* name, const char* text )
{
   TiXmlElement* node = new TiXmlElement( name );
   node->LinkEndChild( new TiXmlText( text ) );
   return node;
}

void TSShapeConstructor::ChangeSet::add( TSShapeConstructor::ChangeSet::Command& cmd )
{
   // Lookup the command type
   cmd.type = getCmdType( cmd.name );if ( cmd.type == CmdInvalid )
      return;

   // Ignore operations on __proxy__ sequences (they are only used by the shape editor)
   if ( cmd.argv[0].startsWith( "__proxy__" ) || ((cmd.type == CmdAddSequence) && cmd.argv[1].startsWith( "__proxy__") ) )
      return;

   // Add the command to the change set (apply command specific collapsing)
   bool addCommand = true;
   switch ( cmd.type )
   {

   // Node commands
   case CmdSetNodeParent:           addCommand = addCmd_setNodeParent( cmd );             break;
   case CmdSetNodeTransform:        addCommand = addCmd_setNodeTransform( cmd );          break;
   case CmdRenameNode:              addCommand = addCmd_renameNode( cmd );                break;
   case CmdRemoveNode:              addCommand = addCmd_removeNode( cmd );                break;

   // Mesh commands
   case CmdSetMeshSize:             addCommand = addCmd_setMeshSize( cmd );               break;
   case CmdSetMeshType:             addCommand = addCmd_setMeshType( cmd );               break;
   case CmdSetMeshMaterial:         addCommand = addCmd_setMeshMaterial( cmd );           break;
   case CmdRemoveMesh:              addCommand = addCmd_removeMesh( cmd );                break;

   // Object commands
   case CmdSetObjectNode:           addCommand = addCmd_setObjectNode( cmd );             break;
   case CmdRenameObject:            addCommand = addCmd_renameObject( cmd );              break;
   case CmdRemoveObject:            addCommand = addCmd_removeObject( cmd );              break;
   case CmdSetBounds:               addCommand = addCmd_setBounds( cmd );                 break;

   // Detail level commands
   case CmdRenameDetailLevel:       addCommand = addCmd_renameDetailLevel( cmd );         break;
   case CmdRemoveDetailLevel:       addCommand = addCmd_removeDetailLevel( cmd );         break;
   case CmdSetDetailLevelSize:      addCommand = addCmd_setDetailSize( cmd );             break;
   case CmdAddImposter:             addCommand = addCmd_addImposter( cmd );               break;
   case CmdRemoveImposter:          addCommand = addCmd_removeImposter( cmd );            break;

   // Sequence commands
   case CmdAddSequence:             addCommand = addCmd_addSequence( cmd );               break;
   case CmdSetSequencePriority:     addCommand = addCmd_setSequencePriority( cmd );       break;
   case CmdSetSequenceGroundSpeed:  addCommand = addCmd_setSequenceGroundSpeed( cmd );    break;
   case CmdSetSequenceCyclic:       addCommand = addCmd_setSequenceCyclic( cmd );         break;
   case CmdSetSequenceBlend:        addCommand = addCmd_setSequenceBlend( cmd );          break;
   case CmdRenameSequence:          addCommand = addCmd_renameSequence( cmd );            break;
   case CmdRemoveSequence:          addCommand = addCmd_removeSequence( cmd );            break;

   case CmdAddTrigger:              addCommand = addCmd_addTrigger( cmd );                break;
   case CmdRemoveTrigger:           addCommand = addCmd_removeTrigger( cmd );             break;

   // Other commands that do not have optimizations
   default:
      break;
   }

   if ( addCommand )
      mCommands.push_back( cmd );
}

//-----------------------------------------------------------------------------
// NODE COMMANDS

bool TSShapeConstructor::ChangeSet::addCmd_setNodeParent( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // No dependencies, replace the parent argument for any previous addNode or
   // setNodeParent.

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddNode:
      case CmdSetNodeParent:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
         {
            cmd.argv[1] = newCmd.argv[1];              // Replace parent argument
            return false;
         }
         break;

      default:
         break;
      }
   }

   return true;

}

bool TSShapeConstructor::ChangeSet::addCmd_setNodeTransform( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // No dependencies, replace the parent argument for any previous addNode or
   // setNodeParent.

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddNode:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
         {
            cmd.argc = newCmd.argc + 1;         // Replace transform argument
            cmd.argv[2] = newCmd.argv[1];
            cmd.argv[3] = newCmd.argv[2];
            return false;
         }
         break;

      case CmdSetNodeTransform:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
         {
            cmd = newCmd;                       // Collapse successive set transform commands
            return false;
         }
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_renameNode( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Replace name argument for previous addNode or renameNode, but stop
   // if the new name is already in use (can occur if 2 nodes switch names). eg.
   // A->C
   // B->A
   // C->B  (cannot replace the previous A->C with A->B as 'B' is in use)

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddNode:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
         {
            cmd.argv[0] = newCmd.argv[1];       // Replace initial name argument
            return false;
         }
         break;

      case CmdRenameNode:
         if ( namesEqual( cmd.argv[1], newCmd.argv[0] ) )
         {
            cmd.argv[1] = newCmd.argv[1];       // Collapse successive renames
            if ( namesEqual( cmd.argv[0], cmd.argv[1] ) )
               mCommands.erase(index);          // Ignore empty renames
            return false;
         }
         else if ( namesEqual( cmd.argv[0], newCmd.argv[1] ) )
            return true;                        // Name is in use, cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_removeNode( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // No dependencies. Remove any previous command that references the node

   String nodeName( newCmd.argv[0] );
   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddNode:
         if ( namesEqual( cmd.argv[0], nodeName ) )
         {
            mCommands.erase(index);             // Remove the added node
            return false;
         }
         break;

      case CmdSetNodeTransform:
      case CmdSetNodeParent:
         if ( namesEqual( cmd.argv[0], nodeName ) )
            mCommands.erase(index);             // Remove any commands that reference the removed node
         break;

      case CmdRenameNode:
         if ( namesEqual( cmd.argv[1], nodeName ) )
         {
            nodeName = cmd.argv[0];             // Node is renamed
            mCommands.erase(index);
         }
         break;

      default:
         break;
      }
   }

   return true;
}

//-----------------------------------------------------------------------------
// SEQUENCE COMMANDS

bool TSShapeConstructor::ChangeSet::addCmd_addSequence( TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // For sequences added from ShapeEditor __backup sequences, search backwards for
   // any changes made to the source of the __backup sequence. If none are found,
   // use the __backup source instead of the __backup.
   const char* backupPrefix = "__backup__";
   if ( !newCmd.argv[0].startsWith( backupPrefix ) )
      return true;

   S32 start = dStrlen( backupPrefix );
   S32 end = newCmd.argv[0].find( '_', 0, String::Right );
   String sourceName = newCmd.argv[0].substr( start, end - start );

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdSetSequencePriority:
      case CmdSetSequenceCyclic:
      case CmdSetSequenceBlend:
      case CmdSetSequenceGroundSpeed:
         // __backup sequence source has been modified => cannot go back further
         if ( namesEqual( cmd.argv[0], sourceName ) )
            return true;

      case CmdAddSequence:
         if ( namesEqual( cmd.argv[1], newCmd.argv[0] ) )
         {
            // No changes to the __backup sequence were found
            newCmd.argv[0] = sourceName;
            return true;
         }
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_setSequencePriority( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Replace any previous setSequencePriority command, but stop if the
   // sequence is used as a source for addSequence (since the priority is
   // copied).

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdSetSequencePriority:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
         {
            cmd.argv[1] = newCmd.argv[1];       // Collapse successive set priority commands
            return false;
         }
         break;

      case CmdAddSequence:
         if ( namesEqual( cmd.argv[1], newCmd.argv[0] ) )
            return true;                        // Sequence is used as source => cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_setSequenceGroundSpeed( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Replace any previous setSequenceGroundSpeed command, but stop if the
   // sequence is used as a source for addSequence (since the priority is
   // copied).

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdSetSequenceGroundSpeed:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
         {
            cmd.argv[1] = newCmd.argv[1];       // Collapse successive set ground speed commands
            return false;
         }
         break;

      case CmdAddSequence:
         if ( namesEqual( cmd.argv[1], newCmd.argv[0] ) )
            return true;                        // Sequence is used as source => cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_setSequenceCyclic( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Replace any previous setSequenceCyclic command, but stop if the
   // sequence is used as a source for addSequence (since the priority is
   // copied).

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdSetSequenceCyclic:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) &&
              dAtob( cmd.argv[1] ) != dAtob( newCmd.argv[1] ) )
         {
            mCommands.erase(index);             // ignore both setCyclic commands (1 undoes the other)
            return false;
         }
         break;

      case CmdAddSequence:
         if ( namesEqual( cmd.argv[1], newCmd.argv[0] ) )
            return true;                        // Sequence is used as source => cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_setSequenceBlend( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Replace any previous setSequenceBlend command, but stop if the
   // sequence is used as a source for addSequence (since the priority is
   // copied).

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdSetSequenceBlend:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) &&
              dAtob( cmd.argv[1] ) != dAtob( newCmd.argv[1] )     &&
              namesEqual( cmd.argv[2], newCmd.argv[2] ) &&
              dAtoi( cmd.argv[3] ) == dAtoi( newCmd.argv[3] ) )
         {
            mCommands.erase(index);             // Ignore both setBlend commands (1 undoes the other)
            return false;
         }
         break;

      case CmdAddSequence:
         if ( namesEqual( cmd.argv[1], newCmd.argv[0] ) )
            return true;                        // Sequence is used as source => cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_renameSequence( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Replace name argument for previous addSequence or renameSequence, but stop
   // if the new name is already in use (can occur if 2 nodes switch names). eg.
   // A->C
   // B->A
   // C->B  (cannot replace the previous A->C with A->B as 'B' is in use)
   //
   // Once a previous command is found, go forward through the command list and
   // update any references to the old name

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdRenameSequence:
         if ( namesEqual( cmd.argv[0], newCmd.argv[1] ) && !namesEqual( cmd.argv[1], newCmd.argv[0] ) )
            return true;                              // Name is in use => cannot go back further
         // fall through to common processing
      case CmdAddSequence:
         if ( namesEqual( cmd.argv[1], newCmd.argv[0] ) )
         {
            if ( cmd.type == CmdRenameSequence )
            {
               cmd.argv[1] = newCmd.argv[1];          // Collapse successive renames
               if ( namesEqual( cmd.argv[0], cmd.argv[1] ) )
                  mCommands.erase(index);             // Ignore empty renames
            }
            else if ( cmd.type == CmdAddSequence )
            {
               cmd.argv[1] = newCmd.argv[1];          // Replace initial name argument
            }

            // Update any references to the old name
            for ( S32 j = index + 1; j < mCommands.size(); j++ )
            {
               Command& cmd2 = mCommands[j];
               switch ( cmd2.type )
               {
               case CmdSetSequencePriority:
               case CmdSetSequenceCyclic:
               case CmdSetSequenceBlend:
               case CmdSetSequenceGroundSpeed:
                  if ( namesEqual( cmd2.argv[0], newCmd.argv[0] ) )
                     cmd2.argv[0] = newCmd.argv[1];
                  break;
               }
            }
            return false;
         }
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_removeSequence( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Remove any previous command that references the sequence, but stop if the
   // sequence is used as a source for addSequence

   String seqName( newCmd.argv[0] );
   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddSequence:
         if ( namesEqual( cmd.argv[1], seqName ) )
         {
            mCommands.erase( index );           // Remove the added sequence
            return false;
         }
         else if ( namesEqual( cmd.argv[0], seqName ) )
         {
            // Removed sequence is used as source for another sequence => can't
            // go back any further
            return true;
         }
         break;

      case CmdRenameSequence:
         if ( namesEqual( cmd.argv[1], seqName ) )
         {
            seqName = cmd.argv[0];              // Sequence is renamed
            mCommands.erase( index );
         }
         break;

      case CmdSetSequencePriority:
      case CmdSetSequenceGroundSpeed:
      case CmdSetSequenceCyclic:
      case CmdSetSequenceBlend:
      case CmdAddTrigger:
      case CmdRemoveTrigger:
         if ( namesEqual( cmd.argv[0], seqName ) )
            mCommands.erase( index );           // Remove any commands that reference the removed sequence
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_addTrigger( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Remove a matching removeTrigger command, but stop if the sequence is used as
   // a source for addSequence (since triggers are copied).

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdRemoveTrigger:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) &&
              cmd.argv[1] == newCmd.argv[1] &&
              cmd.argv[2] == newCmd.argv[2] )
         {
            mCommands.erase(index);             // Remove previous removeTrigger command
            return false;
         }
         break;

      case CmdAddSequence:
         if ( namesEqual( cmd.argv[1], newCmd.argv[0] ) )
            return true;                        // Sequence is used as a source => cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_removeTrigger( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Remove a matching addTrigger command, but stop if the sequence is used as
   // a source for addSequence (since triggers are copied).

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddTrigger:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) &&
              cmd.argv[1] == newCmd.argv[1] &&
              cmd.argv[2] == newCmd.argv[2] )
         {
            mCommands.erase(index);             // Remove previous addTrigger command
            return false;
         }
         break;

      case CmdAddSequence:
         if ( namesEqual( cmd.argv[1], newCmd.argv[0] ) )
            return true;                        // Sequence is used as a source => cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

//-----------------------------------------------------------------------------
// MESH COMMANDS

bool TSShapeConstructor::ChangeSet::addCmd_setMeshSize( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Replace size argument for previous addMesh or setMeshSize, but stop if the
   // new name is already in use (can occur if 2 nodes switch names). eg.
   // A->C
   // B->A
   // C->B  (cannot replace the previous A->C with A->B as 'B' is in use)

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddMesh:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
         {
            cmd.argv[0] = newCmd.argv[1];       // Replace initial size argument
            return false;
         }
         break;

      case CmdSetMeshSize:
         if ( cmd.argv[1] == newCmd.argv[0] )
         {
            cmd.argv[1] = newCmd.argv[1];       // Collapse successive size sets
            if ( cmd.argv[0] == cmd.argv[1] )
               mCommands.erase(index);          // Ignore empty resizes
            return false;
         }
         else if ( cmd.argv[0] == newCmd.argv[1] )
            return true;                        // Size is in use, cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_setMeshType( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Replace any previous setMeshType command, but stop if the mesh is used as
   // a source for addMesh (since the type is copied).

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdSetMeshType:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
         {
            cmd.argv[1] = newCmd.argv[1];       // Collapse successive set type commands
            return false;
         }
         break;

      case CmdAddMesh:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
            return true;                        // Mesh is used as source => cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_setMeshMaterial( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Replace any previous setMeshMaterial command, but stop if the mesh is used as
   // a source for addMesh (since the materials are copied).

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdSetMeshMaterial:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
         {
            cmd.argv[1] = newCmd.argv[1];       // Collapse successive set material commands
            return false;
         }
         break;

      case CmdAddMesh:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
            return true;                        // Mesh is used as source => cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_removeMesh( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Remove any previous command that references the mesh, but stop if the mesh
   // is used as a source for addMesh

   String meshName( newCmd.argv[0] );
   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddMesh:
         if ( namesEqual( cmd.argv[0], meshName ) )
         {
            mCommands.erase( index );           // Remove the added mesh
            return false;
         }
         else if ( namesEqual( cmd.argv[2], meshName ) )
         {
            // Removed mesh is used as source for another mesh => can't go back
            // any further
            return true;
         }
         break;

      case CmdAddPrimitive:
         if ( namesEqual( cmd.argv[0], meshName ) )
         {
            mCommands.erase( index );           // Remove the added primitive
            return false;
         }
         break;

      case CmdSetMeshSize:
      case CmdSetMeshType:
      case CmdSetMeshMaterial:
         if ( namesEqual( cmd.argv[0], meshName ) )
            mCommands.erase( index );           // Remove any commands that reference the removed mesh
         break;

      default:
         break;
      }
   }

   return true;
}

//-----------------------------------------------------------------------------
// OBJECT COMMANDS

bool TSShapeConstructor::ChangeSet::addCmd_setObjectNode( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // No dependencies, replace the node argument for any previous parent argument for any previous addNode or
   // setNodeParent.

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddMesh:
      {
         S32 dummy;
         if ( namesEqual( String::GetTrailingNumber(cmd.argv[0], dummy), newCmd.argv[0] ) )
         {
            cmd.argv[3] = newCmd.argv[1];    // Replace node argument
            return false;
         }
         break;
      }

      case CmdSetObjectNode:
         if ( namesEqual( cmd.argv[0], newCmd.argv[0] ) )
         {
            cmd.argv[1] = newCmd.argv[1];
            return false;
         }
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_renameObject( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Replace name argument for previous renameObject, but stop if the new name
   // is already in use (can occur if 2 objects switch names). eg.
   // A->C
   // B->A
   // C->B  (cannot replace the previous A->C with A->B as 'B' is in use)

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdRenameObject:
         if ( namesEqual( cmd.argv[1], newCmd.argv[0] ) )
         {
            cmd.argv[1] = newCmd.argv[1];       // Collapse successive renames
            if ( namesEqual( cmd.argv[0], cmd.argv[1] ) )
               mCommands.erase(index);          // Ignore empty renames
            return false;
         }
         else if ( namesEqual( cmd.argv[0], newCmd.argv[1] ) )
            return true;                        // Name is in use, cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_removeObject( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Remove any previous command that references the object, but stop if any
   // object mesh is used as a source for addMesh

   S32 dummy;
   String objName( newCmd.argv[0] );
   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddMesh:
         if ( namesEqual( String::GetTrailingNumber(cmd.argv[0], dummy), objName ) )
         {
            mCommands.erase( index );           // Remove the added mesh
            // Must still add the removeObject command as there could be multiple
            // meshes in the object
         }
         else if ( namesEqual( String::GetTrailingNumber(cmd.argv[2], dummy), objName ) )
         {
            // Removed mesh is used as source for another mesh => can't go back
            // any further
            return true;
         }
         break;

      case CmdRenameObject:
         if ( namesEqual( cmd.argv[1], objName ) )
         {
            objName = cmd.argv[0];              // Object is renamed
            mCommands.erase( index );
         }
         break;

      case CmdSetObjectNode:
         if ( namesEqual( cmd.argv[0], objName ) )
            mCommands.erase( index );           // Remove any commands that reference the removed object
         break;

      case CmdSetMeshSize:
      case CmdSetMeshType:
      case CmdSetMeshMaterial:
      case CmdRemoveMesh:
         if ( namesEqual( String::GetTrailingNumber(cmd.argv[0], dummy), objName ) )
            mCommands.erase( index );           // Remove comands that reference the removed object
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_setBounds( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Only the last bounds update applies, so replace any previous command.

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdSetBounds:
         mCommands.erase( index );
         break;

      default:
         break;
      }
   }

   return true;
}

//-----------------------------------------------------------------------------
// DETAIL COMMANDS

bool TSShapeConstructor::ChangeSet::addCmd_renameDetailLevel( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Replace name argument for previous renameDetailLevel, but stop if the new
   // name is already in use (can occur if 2 objects switch names). eg.
   // A->C
   // B->A
   // C->B  (cannot replace the previous A->C with A->B as 'B' is in use)

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdRenameDetailLevel:
         if ( namesEqual( cmd.argv[1], newCmd.argv[0] ) )
         {
            cmd.argv[1] = newCmd.argv[1];       // Collapse successive renames
            if ( namesEqual( cmd.argv[0], cmd.argv[1] ) )
               mCommands.erase(index);          // Ignore empty renames
            return false;
         }
         else if ( namesEqual( cmd.argv[0], newCmd.argv[1] ) )
            return true;                        // Name is in use, cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_removeDetailLevel( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Remove any previous command that references the detail, but stop if a mesh
   // is used as a source for addMesh

   S32 detSize = dAtoi( newCmd.argv[0] );
   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      S32 size;

      switch ( cmd.type )
      {
      case CmdAddMesh:
         String::GetTrailingNumber( cmd.argv[2], size );
         if ( size == detSize )
         {
            // Removed detail is used as source for another mesh => can't go back
            // any further
            return true;
         }
         // fall through

      case CmdAddPrimitive:
      case CmdSetMeshSize:
      case CmdSetMeshType:
      case CmdSetMeshMaterial:
      case CmdRemoveMesh:
         String::GetTrailingNumber( cmd.argv[0], size );
         if ( size == detSize )
            mCommands.erase( index );
         break;

      case CmdAddImposter:
      case CmdAddCollisionDetail:
         if ( dAtoi(cmd.argv[0]) == detSize )
         {
            mCommands.erase( index );
            return false;
         }
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_setDetailSize( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Similar to renameXXX. Replace size argument for previous addImposter or
   // setDetailLevelSize, but stop if the new size is already in use (can occur
   // if 2 details switch sizes). eg.
   // A->C
   // B->A
   // C->B  (cannot replace the previous A->C with A->B as 'B' is in use)

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddImposter:
         if ( cmd.argv[0] == newCmd.argv[0] )
         {
            cmd.argv[0] = newCmd.argv[1];       // Change detail size argument
            return false;
         }
         break;

      case CmdSetDetailLevelSize:
         if ( cmd.argv[1] == newCmd.argv[0] )
         {
            cmd.argv[1] = newCmd.argv[1];       // Collapse successive detail size changes
            if ( cmd.argv[0] == cmd.argv[1] )
               mCommands.erase(index);          // Ignore empty changes
            return false;
         }
         else if ( cmd.argv[0] == newCmd.argv[1] )
            return true;                        // Detail size already in use => cannot go back further
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_addImposter( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Remove previous removeImposter, and replace any previous addImposter. If
   // replacing, also remove any setDetailLevelSize for the old imposter

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddImposter:
         // Replace the AddImposter command, but first remove any reference to
         // the added detail level.
         for ( S32 j = index + 1; j < mCommands.size(); j++ )
         {
            Command& cmd2 = mCommands[j];
            if ( ( cmd2.type == CmdSetDetailLevelSize ) &&
                 cmd2.argv[0] == cmd.argv[0] )
            {
               mCommands.erase(j);
               break;
            }
         }
         // Replace previous addImposter command
         cmd = newCmd;
         return false;

      case CmdRemoveImposter:
         mCommands.erase(index);                // Remove previous removeImposter command
         break;

      default:
         break;
      }
   }

   return true;
}

bool TSShapeConstructor::ChangeSet::addCmd_removeImposter( const TSShapeConstructor::ChangeSet::Command& newCmd )
{
   // Remove any previous addImposter, and also remove any setDetailLevelSize
   // for that imposter.
   // Always need to return true, since we could be removing imposters already
   // present in the shape (not added with addImposter).

   for ( S32 index = mCommands.size()-1; index >= 0; index-- )
   {
      Command& cmd = mCommands[index];
      switch ( cmd.type )
      {
      case CmdAddImposter:
         // Remove the AddImposter command, but first remove any reference to
         // the added detail level.
         for ( S32 j = index + 1; j < mCommands.size(); j++ )
         {
            Command& cmd2 = mCommands[j];
            if ( ( cmd2.type == CmdSetDetailLevelSize ) &&
                 cmd2.argv[0] == cmd.argv[0] )
            {
               mCommands.erase(j);
               break;
            }
         }
         mCommands.erase(index);
         break;

      default:
         break;
      }
   }

   return true;
}

void TSShapeConstructor::onActionPerformed()
{
   // Reinit shape if we modify stuff in the shape editor, otherwise delay
   if (!mLoadingShape)
   {
      if (mShape && mShape->needsReinit())
      {
         mShape->init();
      }
   }
}
