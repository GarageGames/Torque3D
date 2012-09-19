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

#ifndef _SFXFMODPROJECT_H_
#define _SFXFMODPROJECT_H_

#ifndef _SIMDATABLOCK_H_
   #include "console/simDatablock.h"
#endif
#ifndef _CONSOLETYPES_H_
   #include "console/consoleTypes.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif
#ifndef _SFXSYSTEM_H_
   #include "sfx/sfxSystem.h"
#endif

#include "fmod_event.h"


class SFXFMODEvent;
class SFXFMODEventGroup;
class SimGroup;



/// Datablock that loads an FMOD Designer project.
///
/// All events in the project are automatically made available as SFXFMODEvent track
/// datablock instances.  Each event object is automatically named by substituting
/// the slashes in its fully qualified name with underscores and preprending the project
/// name to this; event 'group1/group2/event' in the SFXFMODProject instance called
/// 'project', for example, will be available as a TorqueScript object called
/// 'project_group1_group2_event'.
///
/// This class also works in a client-server environment where the server is
/// not running FMOD.  The event objects are cached in an auto-generated TorqueScript
/// file alongside the .fev project file (x/y.fev -> x/y.fev.cs) which, when available
/// and up-to-date, does not require FMOD for the server-side objects to correctly
/// initialize.
///
/// To establish good loading behavior and for good memory management, it is necessary to
/// wisely distribute events to groups and to manually pre-load groups.  The best solution
/// probably is to have one group of common events that is loaded during game startup and
/// then have one event group for each level in the game that is only loaded for the
/// duration of its particular level.
///
/// SFXFMODProject will propagate it's networking model to all its contents.  This means
/// that if the project is a non-networked datablock, then all event groups, events, and
/// descriptions contained in the project will also be non-networked datablocks.
///
/// It usually makes the most sense to use non-networked ("client-only") datablocks as
/// otherwise the FMOD datablocks will be purged on each mission load.
///
/// @note Only one project's music data can ever be loaded at any one time.
///   Usually you wouldn't want more than a single SFXFMODProject instance in your game
///   data.  Also, only a single media path can be set through the designer API so when
///   loading multiple projects, note that each project will set the media path to its
///   own directory.  For data loading to work, all project thus need to be placed in
///   the same directory.
///
class SFXFMODProject : public SimDataBlock
{
   public:
   
      typedef SimDataBlock Parent;
      friend class SFXFMODEventGroup; // _addGroup
      friend class SFXFMODEvent; // _addEvent
      
   protected:
   
      ///
      String mFileName;
      
      ///
      String mMediaPath;
      
      ///
      SFXFMODEventGroup* mRootGroups;
      
      /// A flat list of all the groups in this projet.
      Vector< SFXFMODEventGroup* > mGroups;

      /// A flat list of all the events in the project.
      Vector< SFXFMODEvent* > mEvents;
      
      ///
      FMOD_EVENTPROJECT* mHandle;
      
      ///
      void _onSystemEvent( SFXSystemEventType event );
      
      ///
      void _clear();
      
      ///
      bool _load();
      
      ///
      void _addEvent( SFXFMODEvent* event );
      
      ///
      void _addGroup( SFXFMODEventGroup* group );
      
      ///
      void _removeEvent( SFXFMODEvent* event );
      
      ///
      void _removeGroup( SFXFMODEventGroup* group );
   
   public:
   
      ///
      SFXFMODProject();
      
      virtual ~SFXFMODProject();
            
      ///
      void acquire( bool recursive = false );
      
      ///
      void release();
      
      ///
      const String& getFileName() const { return mFileName; }

      // SimDataBlock.
      virtual bool onAdd();
      virtual void onRemove();
      virtual bool preload( bool server, String& errorStr );
      virtual void packData( BitStream* stream );
      virtual void unpackData( BitStream* stream );
      
      static void initPersistFields();
   
      DECLARE_CONOBJECT( SFXFMODProject );
      DECLARE_CATEGORY( "SFX FMOD" );
      DECLARE_DESCRIPTION( "An FMOD Designer project." );
};

#endif // !_SFXFMODPROJECT_H_
