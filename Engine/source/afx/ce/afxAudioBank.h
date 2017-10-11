
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#ifndef _AFX_AUDIO_BANK_H_
#define _AFX_AUDIO_BANK_H_

class SFXDescription;

class afxAudioBank: public SimDataBlock
{
private:
   typedef SimDataBlock Parent;

public:
   SFXDescription*    mDescriptionObject;
   U32                mDescriptionObjectID;
   StringTableEntry   mPath;
   StringTableEntry   mFilenames[32];
   bool               mPreload;
   S32                play_index;

public:
   /*C*/              afxAudioBank();
   /*C*/              afxAudioBank(const afxAudioBank&, bool = false);
   /*D*/              ~afxAudioBank();

   static void        initPersistFields();

   virtual bool       onAdd();
   virtual void       packData(BitStream* stream);
   virtual void       unpackData(BitStream* stream);

   bool               preload(bool server, String &errorStr);

   afxAudioBank*      cloneAndPerformSubstitutions(const SimObject*, S32 index=0);
   virtual void       onPerformSubstitutions();
   virtual bool       allowSubstitutions() const { return true; }

   DECLARE_CONOBJECT(afxAudioBank);
   DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif  // _AFX_AUDIO_BANK_H_
