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

#ifndef _THEORATEXTUREOBJECT_H_
#define _THEORATEXTUREOBJECT_H_

#ifdef TORQUE_OGGTHEORA

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#ifndef _THEORATEXTURE_H_
#include "gfx/video/theoraTexture.h"
#endif

#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif


class SFXDescription;


class TheoraTextureObject : public SimObject
{
   typedef SimObject Parent;

protected:

   /// Is the video currently playing
   bool mIsPlaying;

   /// Should the video loop
   bool mLoop;

   /// The Theora file we should play.
   String mFilename;

   /// Name for the NamedTexTarget.
   String mTexTargetName;

   /// Theora video player backend.
   TheoraTexture mTheoraTexture;

   /// The texture target which can be referenced in materials.
   NamedTexTarget mTexTarget;

   /// Sound description to use for the video's audio channel.
   SFXDescription* mSFXDescription;

   /// Method that is hooked up with the texture target's delegate.
   GFXTextureObject* _texDelegate( U32 index );

public:

   TheoraTextureObject();

   void play();
   void stop() { mTheoraTexture.stop(); mIsPlaying = false; }
   void pause() { mTheoraTexture.pause(); mIsPlaying = false; }

   // SimObject.
   DECLARE_CONOBJECT( TheoraTextureObject );

   virtual bool onAdd();
   virtual void onRemove();

   static void initPersistFields();
};

#endif // TORQUE_OGGTHEORA
#endif // _THEORATEXTUREOBJECT_H_
