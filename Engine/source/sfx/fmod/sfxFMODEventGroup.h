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

#ifndef _SFXFMODEVENTGROUP_H_
#define _SFXFMODEVENTGROUP_H_

#ifndef _SIMDATABLOCK_H_
   #include "console/simDatablock.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif
#ifndef _CONSOLETYPES_H_
   #include "console/consoleTypes.h"
#endif

#include "fmod_event.h"


class SFXFMODProject;
class SFXFMODEvent;


///
class SFXFMODEventGroup : public SimDataBlock
{
   public:
   
      typedef SimDataBlock Parent;
      friend class SFXFMODProject;
      friend class SFXFMODEvent; // mHandle
      friend class SFXFMODEventSource; // mHandle
      
   protected:
   
      ///
      String mName;
      
      ///
      U32 mNumEvents;
      
      ///
      U32 mNumGroups;
      
      ///
      SFXFMODProject* mProject;

      ///
      SFXFMODEventGroup* mParent;

      ///
      SFXFMODEventGroup* mChildren;
      
      ///
      SFXFMODEventGroup* mSibling;
   
      ///
      SFXFMODEvent* mEvents;
      
      ///
      FMOD_EVENTGROUP* mHandle;
      
      ///
      U32 mLoadCount;
      
      /// Project ID for client net sync.
      S32 mParentId;
      
      /// Project ID for client net sync.
      S32 mProjectId;
                  
      ///
      void _load();
      
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
      SFXFMODEventGroup();
   
      ///
      SFXFMODEventGroup( SFXFMODProject* project, const String& name, SFXFMODEventGroup* parent = NULL );

      ///
      SFXFMODEventGroup( SFXFMODProject* project, FMOD_EVENTGROUP* handle, SFXFMODEventGroup* parent = NULL );
      
      ~SFXFMODEventGroup();
      
      /// Create the event group object on the FMOD device.
      void acquire( bool recursive = false );
      
      /// Release the event group object on the FMOD device.
      void release();
      
      ///
      const String& getGroupName() const { return mName; }

      ///
      String getQualifiedName() const;
      
      ///
      SFXFMODProject* getProject() const { return mProject; }
      
      /// Return true if the event data for this group has been loaded.
      bool isDataLoaded() const;

      /// Load the event data for this group.
      ///
      /// @note Loading is reference-counted.
      bool loadData( bool samples = true, bool streams = true );
      
      ///
      void freeData( bool force = false );
      
      // SimDataBlock.
      virtual bool onAdd();
      virtual void onRemove();
      virtual bool preload( bool server, String& errorStr );
      virtual void packData( BitStream* stream );
      virtual void unpackData( BitStream* stream );
      
      static void initPersistFields();
      
      DECLARE_CONOBJECT( SFXFMODEventGroup );
      DECLARE_CATEGORY( "SFX FMOD" );
      DECLARE_DESCRIPTION( "An event group in an FMOD Designer project." );
};

#endif // !_SFXFMODEVENTGROUP_H_
