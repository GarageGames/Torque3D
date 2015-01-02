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
#include "console/consoleLogger.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"

Vector<ConsoleLogger *> ConsoleLogger::mActiveLoggers;
bool ConsoleLogger::smInitialized = false;

IMPLEMENT_CONOBJECT( ConsoleLogger );
ConsoleDocClass( ConsoleLogger,
   "A class designed to be used as a console consumer and log the data it receives to a file.\n\n"
   
   "@see dumpConsoleFunctions\n"
   "@see dumpConsoleClasses\n"
   "@ingroup Logging\n"
);
//-----------------------------------------------------------------------------

ConsoleLogger::ConsoleLogger()
{
   mFilename = NULL;
   mLogging = false;
   mAppend = false;

   mLevel = ConsoleLogEntry::Normal;
}

//-----------------------------------------------------------------------------

ConsoleLogger::ConsoleLogger( const char *fileName, bool append )
{
   mLogging = false;

   mLevel = ConsoleLogEntry::Normal;
   mFilename = StringTable->insert( fileName );
   mAppend = append;

   init();
}

//-----------------------------------------------------------------------------

ImplementEnumType( LogLevel,
   "@brief Priority levels for logging entries\n\n" 
   "@ingroup Logging")
   { ConsoleLogEntry::Normal,     "normal", "Lowest priority level, no highlighting."  },
   { ConsoleLogEntry::Warning,    "warning", "Mid level priority, tags and highlights possible issues in blue." },
   { ConsoleLogEntry::Error,      "error",  "Highest priority level, extreme emphasis on this entry. Highlighted in red." },
EndImplementEnumType;

void ConsoleLogger::initPersistFields()
{
   addGroup( "Logging" );
   addField( "level",   TYPEID< ConsoleLogEntry::Level >(),     Offset( mLevel,    ConsoleLogger ), "Determines the priority level and attention the logged entry gets when recorded\n\n" );
   endGroup( "Logging" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool ConsoleLogger::processArguments( S32 argc, ConsoleValueRef *argv )
{
   if( argc == 0 )
      return false;

   bool append = false;

   if( argc == 2 )
      append = dAtob( argv[1] );

   mAppend = append;
   mFilename = StringTable->insert( argv[0] );

   if( init() )
   {
      attach();
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------

ConsoleLogger::~ConsoleLogger()
{
   detach();
}

//-----------------------------------------------------------------------------

bool ConsoleLogger::init()
{
   if( smInitialized )
      return true;

   Con::addConsumer( ConsoleLogger::logCallback );
   smInitialized = true;

   return true;
}

//-----------------------------------------------------------------------------

bool ConsoleLogger::attach()
{
   if( mFilename == NULL )
   {
      Con::errorf( "ConsoleLogger failed to attach: no filename supplied." );
      return false;
   }

   // Check to see if this is initialized before using it
   if( !smInitialized )
   {
      if( !init() )
      {
         Con::errorf( "ConsoleLogger failed to initalize." );
         return false;
      }
   }

   if( mLogging )
      return false;

   // Open the filestream
   mStream.open( mFilename, ( mAppend ? Torque::FS::File::WriteAppend : Torque::FS::File::Write ) );

   // Add this to list of active loggers
   mActiveLoggers.push_back( this );
   mLogging = true;

   return true;
}

//-----------------------------------------------------------------------------

bool ConsoleLogger::detach()
{

   // Make sure this is valid before messing with it
   if( !smInitialized ) 
   {
      if( !init() ) 
      {
         return false;
      }
   }

   if( !mLogging )
      return false;

   // Close filestream
   mStream.close();

   // Remove this object from the list of active loggers
   for( S32 i = 0; i < mActiveLoggers.size(); i++ ) 
   {
      if( mActiveLoggers[i] == this ) 
      {
         mActiveLoggers.erase( i );
         mLogging = false;
         return true;
      }
   }

   return false; // If this happens, it's bad...
}

//-----------------------------------------------------------------------------

void ConsoleLogger::logCallback( U32 level, const char *consoleLine )
{

   ConsoleLogger *curr;

   // Loop through active consumers and send them the message
   for( int i = 0; i < mActiveLoggers.size(); i++ ) 
   {
      curr = mActiveLoggers[i];

      // If the log level is within the log threshhold, log it
      if( curr->mLevel <= level )
         curr->log( consoleLine );
   }
}

//-----------------------------------------------------------------------------

void ConsoleLogger::log( const char *consoleLine )
{
   // Check to see if this is intalized before using it
   if( !smInitialized ) 
   {
      if( !init() ) 
      {
         Con::errorf( "I don't know how this happened, but log called on this without it being initialized" );
         return;
      }
   }

   mStream.writeLine( (U8 *)consoleLine );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( ConsoleLogger, attach, bool, (), , "() Attaches the logger to the console and begins writing to file"
			  "@tsexample\n"
			  "// Create the logger\n"
			  "// Will automatically start writing to testLogging.txt with normal priority\n"
			  "new ConsoleLogger(logger, \"testLogging.txt\", false);\n\n"
			  "// Send something to the console, with the logger consumes and writes to file\n"
			  "echo(\"This is logged to the file\");\n\n"
			  "// Stop logging, but do not delete the logger\n"
			  "logger.detach();\n\n"
			  "echo(\"This is not logged to the file\");\n\n"
			  "// Attach the logger to the console again\n"
			  "logger.attach();\n\n"
			  "// Logging has resumed\n"
			  "echo(\"Logging has resumed\");"
			  "@endtsexample\n\n")
{
   ConsoleLogger *logger = static_cast<ConsoleLogger *>( object );
   return logger->attach();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( ConsoleLogger, detach, bool, (), , "() Detaches the logger from the console and stops writing to file"
			  "@tsexample\n"
			  "// Create the logger\n"
			  "// Will automatically start writing to testLogging.txt with normal priority\n"
			  "new ConsoleLogger(logger, \"testLogging.txt\", false);\n\n"
			  "// Send something to the console, with the logger consumes and writes to file\n"
			  "echo(\"This is logged to the file\");\n\n"
			  "// Stop logging, but do not delete the logger\n"
			  "logger.detach();\n\n"
			  "echo(\"This is not logged to the file\");\n\n"
			  "// Attach the logger to the console again\n"
			  "logger.attach();\n\n"
			  "// Logging has resumed\n"
			  "echo(\"Logging has resumed\");"
			  "@endtsexample\n\n")
{
   ConsoleLogger *logger = static_cast<ConsoleLogger *>( object );
   return logger->detach();
}
