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

#include <stdio.h>

#include "platform/platform.h"
#include "platform/platformVolume.h"
#include "app/mainLoop.h"
#include "T3D/gameFunctions.h"
#include "core/stream/fileStream.h"
#include "core/resourceManager.h"
#include "ts/tsShape.h"
#include "ts/tsShapeConstruct.h"

#ifdef TORQUE_OS_WIN
#include "platformWin32/platformWin32.h"
#include "platformWin32/winConsole.h"
#endif


extern TSShape* loadColladaShape( const Torque::Path &path );


/** Print the usage string */
void printUsage()
{
   Con::printf(
"DAE-2-DTS Converter v%s (c) GarageGames, LLC.\n\n"
"dae2dts [options] daeFilename\n\n"
"--config cfgFilename   Set the conversion configuration filename.\n"
"--output dtsFilename   Set the output DTS filename.\n"
"--dsq                  If set, all sequences in the shape will be saved\n"
"                       to DSQ files instead of being embedded in the DTS\n"
"                       file.\n"
"--dsq-only             Same as --dsq, but no DTS file will be saved (handy for\n"
"                       animation only input files).\n"
"--compat               If set, write to DTS v24 for compatibility with\n"
"                       ShowToolPro (no support for vertex colors, 2nd UV\n"
"                       set, autobillboards etc)\n"
"--diffuse              If set, the diffuse texture will be used as the\n"
"                       material name (instead of the COLLADA <material> name)\n"
"--materials            If set, generate a materials.cs script in the output \n"
"                       folder to define Materials used in the shape.\n"
"--verbose              If set, output progress information\n\n"
"Exits with zero on success, non-zero on failure\n\n",
      TORQUE_APP_VERSION_STRING );
}

Torque::Path makeFullPath( const char* path )
{
   char tempBuf[1024];
   Platform::makeFullPathName( path, tempBuf, sizeof(tempBuf), Platform::getCurrentDirectory() );
   return Torque::Path( String( tempBuf ) );
}

S32 TorqueMain( S32 argc, const char **argv )
{
   S32 failed = 0;

   // Initialize the subsystems.
   StandardMainLoop::init();
   Con::setVariable( "Con::Prompt", "" );
   WindowsConsole->enable( true );

   // install all drives for now until we have everything using the volume stuff
   Platform::FS::InstallFileSystems();
   Platform::FS::MountDefaults();

   bool compatMode = false;
   bool diffuseNames = false;
   bool verbose = false;
   bool saveDTS = true;
   bool saveDSQ = false;
   bool genMaterials = false;
   Torque::Path cfgPath, srcPath, destPath;

   // Parse arguments
   S32 i;
   for ( i = 1; i < argc-1; i++ )
   {
      if ( dStrEqual( argv[i], "--config" ) )
         cfgPath = makeFullPath( argv[++i] );
      else if ( dStrEqual( argv[i], "--output" ) )
         destPath = makeFullPath( argv[++i] );
      else if ( dStrEqual( argv[i], "--dsq" ) )
         saveDSQ = true;
      else if ( dStrEqual( argv[i], "--dsq-only" ) )
      {
         saveDTS = false;
         saveDSQ = true;
      }
      else if ( dStrEqual( argv[i], "--compat" ) )
         compatMode = true;
      else if ( dStrEqual( argv[i], "--diffuse" ) )
         diffuseNames = true;
      else if ( dStrEqual( argv[i], "--materials" ) )
         genMaterials = true;
      else if ( dStrEqual( argv[i], "--verbose" ) )
         verbose = true;
   }

   if ( ( i >= argc ) || ( !dStrEndsWith(argv[i], ".dae") && !dStrEndsWith(argv[i], ".kmz" ) ) )
   {
      Con::errorf( "Error: no DAE file specified.\n" );
      printUsage();
      return -1;
   }

   srcPath = makeFullPath( argv[i] );
   if ( destPath.isEmpty() )
   {
      destPath = srcPath;
      destPath.setExtension( "dts" );
   }

   if ( !cfgPath.isEmpty() )
      Con::printf( "Configuration files not yet supported.\n" );

   // Define script callbacks
   if ( verbose )
      Con::evaluate( "function UpdateTSShapeLoadProgress(%progress, %msg) { echo(%msg); }" );
   else
      Con::evaluate( "function UpdateTSShapeLoadProgress(%progress, %msg) { }" );

   if ( verbose )
      Con::printf( "Parsing configuration file...\n" );

   // Set import options
   ColladaUtils::getOptions().reset();
   ColladaUtils::getOptions().forceUpdateMaterials = genMaterials;
   ColladaUtils::getOptions().useDiffuseNames = diffuseNames;

   if ( verbose )
      Con::printf( "Reading dae file...\n" );

   // Attempt to load the DAE file
   Resource<TSShape> shape = ResourceManager::get().load( srcPath );
   if ( !shape )
   {
      Con::errorf( "Failed to convert DAE file: %s\n", srcPath.getFullPath() );
      failed = 1;
   }
   else
   {
      if ( compatMode && !shape->canWriteOldFormat() )
      {
         Con::errorf( "Warning: Attempting to save to DTS v24 but the shape "
                      "contains v26 features. Resulting DTS file may not be valid." );
      }

      FileStream outStream;

      if ( saveDSQ )
      {
         Torque::Path dsqPath( destPath );
         dsqPath.setExtension( "dsq" );

         for ( S32 i = 0; i < shape->sequences.size(); i++ )
         {
            const String& seqName = shape->getName( shape->sequences[i].nameIndex );
            if ( verbose )
               Con::printf( "Writing DSQ file for sequence '%s'...\n", seqName.c_str() );

            dsqPath.setFileName( destPath.getFileName() + "_" + seqName );

            if ( outStream.open( dsqPath, Torque::FS::File::Write ) )
            {
               shape->exportSequence( &outStream, shape->sequences[i], compatMode );
               outStream.close();
            }
            else
            {
               Con::errorf( "Failed to save sequence to %s\n", dsqPath.getFullPath().c_str() );
               failed = 1;
            }
         }
      }
      if ( saveDTS )
      {
         if ( verbose )
            Con::printf( "Writing DTS file...\n" );

         if ( outStream.open( destPath, Torque::FS::File::Write ) )
         {
            if ( saveDSQ )
               shape->sequences.setSize(0);

            shape->write( &outStream, compatMode );
            outStream.close();
         }
         else
         {
            Con::errorf( "Failed to save shape to %s\n", destPath.getFullPath().c_str() );
            failed = 1;
         }
      }
   }

   // Clean everything up.
   StandardMainLoop::shutdown();

   // Do we need to restart?
   if( StandardMainLoop::requiresRestart() )
      Platform::restartInstance();

   return failed;
}
