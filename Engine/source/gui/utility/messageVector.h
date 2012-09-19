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

#ifndef _MESSAGEVECTOR_H_
#define _MESSAGEVECTOR_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

/// Store a list of chat messages.
///
/// This is responsible for managing messages which appear in the chat HUD.
///
/// @see GuiMessageVectorCtrl for more details on how this is used.
class MessageVector : public SimObject
{
   typedef SimObject Parent;

   //-------------------------------------- Public interface...
  public:
   MessageVector();
   ~MessageVector();

  public:
   struct MessageLine {
      char* message;
      S32   messageTag;
   };


   // Spectator registration...
  public:
   enum MessageCode {
      LineInserted   = 0,
      LineDeleted    = 1,

      VectorDeletion = 2
   };

   typedef void (*SpectatorCallback)(void *            spectatorKey,
                                     const MessageCode code,
                                     const U32         argument);

   void registerSpectator(SpectatorCallback, void * spectatorKey);
   void unregisterSpectator(void *spectatorKey);

   // Query functions
  public:
   U32                getNumLines() const;
   const MessageLine& getLine(const U32 line) const;

   // Mutators
  public:
   void pushBackLine(const char*, const S32);
   void popBackLine();
   void pushFrontLine(const char*, const S32);
   void popFrontLine();
   void clear();

   virtual void insertLine(const U32 position, const char*, const S32);
   virtual void deleteLine(const U32);

   bool dump( const char* filename, const char* header = NULL );


   //-------------------------------------- Internal interface
  protected:
   bool onAdd();
   void onRemove();

  private:
   struct SpectatorRef {
      SpectatorCallback callback;
      void *               key;
   };

   Vector<MessageLine>  mMessageLines;

   Vector<SpectatorRef> mSpectators;
   void spectatorMessage(MessageCode, const U32 arg);

  public:
   DECLARE_CONOBJECT(MessageVector);
   static void initPersistFields();
};


//--------------------------------------------------------------------------
inline U32 MessageVector::getNumLines() const
{
   return mMessageLines.size();
}

//--------------------------------------------------------------------------
inline const MessageVector::MessageLine& MessageVector::getLine(const U32 line) const
{
   AssertFatal(line < mMessageLines.size(), "MessageVector::getLine: out of bounds line index");
   return mMessageLines[line];
}

#endif  // _H_GUICHATVECTOR_
