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

#ifndef _SFXTRACK_H_
#define _SFXTRACK_H_

#ifndef _SIMDATABLOCK_H_
   #include "console/simDatablock.h"
#endif
#ifndef _CONSOLETYPES_H_
   #include "console/consoleTypes.h"
#endif


class SFXDescription;


/// A datablock that describes sound data for playback.
class SFXTrack : public SimDataBlock
{
   public:
      
      typedef SimDataBlock Parent;
      
      enum
      {
         /// Maximum numbers of parameters that can be pre-assigned to tracks.
         MaxNumParameters = 8
      };
      
   protected:
   
      /// The description which controls playback settings.
      SFXDescription *mDescription;

      /// Name of the parameters to which sources playing this track should
      /// connect.
      StringTableEntry mParameters[ MaxNumParameters ];
   
      /// Overload this to disable direct instantiation of this class via script 'new'.
      virtual bool processArguments( S32 argc, const char **argv );

   public:
         
      ///
      SFXTrack();
      
      ///
      SFXTrack( SFXDescription* description );
      
      /// Returns the description object for this sound profile.
      SFXDescription* getDescription() const { return mDescription; }

      ///
      StringTableEntry getParameter( U32 index ) const
      {
         AssertFatal( index < MaxNumParameters, "SFXTrack::getParameter() - index out of range" );
         return mParameters[ index ];
      }
      
      ///
      void setParameter( U32 index, const char* name );
      
      // SimDataBlock.
      virtual void packData( BitStream* stream );
      virtual void unpackData( BitStream* stream );
      virtual bool preload( bool server, String& errorStr );
      virtual bool onAdd();
      virtual void inspectPostApply();
      
      static void initPersistFields();
      
      DECLARE_CONOBJECT( SFXTrack );
      DECLARE_CATEGORY( "SFX" );
      DECLARE_DESCRIPTION( "Abstract base class for any kind of data that can be turned into SFXSources." );
};

#endif // !_SFXTRACK_H_
