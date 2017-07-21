
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

#include "afx/arcaneFX.h"

#include "console/consoleTypes.h"
#include "sim/netConnection.h"
#include "sfx/sfxDescription.h"

#include "afx/ce/afxAudioBank.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxAudioBank);

ConsoleDocClass( afxAudioBank,
   "@brief A datablock that specifies an Audio Bank effect.\n\n"

   "afxAudioBank is very similar to the stock Torque SFXProfile datablock but it allows specification of up to 32 different sound "
   "files. The sound that actually plays is determined by the playIndex field."
   "\n\n"

   "afxAudioBank is most useful when used in combination with field substitutions, whereby a substitution statement "
   "assigned to playIndex selects a different sound (perhaps randomly) each time the effect is used."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxAudioBank::afxAudioBank()
{
  mPath = ST_NULLSTRING;
  mDescriptionObjectID = 0;
  mDescriptionObject = NULL;
  mPreload = false;
  play_index = -1;

  for (S32 i = 0; i < 32; i++)
    mFilenames[i] = ST_NULLSTRING;
}

afxAudioBank::afxAudioBank(const afxAudioBank& other, bool temp_clone) : SimDataBlock(other, temp_clone)
{
  mPath = other.mPath;
  mDescriptionObject = other.mDescriptionObject;
  mDescriptionObjectID = other.mDescriptionObjectID; // -- for pack/unpack of mDescriptionObject ptr
  mPreload = other.mPreload;
  play_index = other.play_index;

  for (S32 i = 0; i < 32; i++)
    mFilenames[i] = other.mFilenames[i];
}

afxAudioBank::~afxAudioBank()
{
  if (!isTempClone())
    return;

  if (mDescriptionObject && mDescriptionObject->isTempClone())
  {
    delete mDescriptionObject;
    mDescriptionObject = 0;
  }
}

afxAudioBank* afxAudioBank::cloneAndPerformSubstitutions(const SimObject* owner, S32 index)
{
  if (!owner)
    return this;

  afxAudioBank* sub_profile_db = this;

  SFXDescription* desc_db;
  if (mDescriptionObject && mDescriptionObject->getSubstitutionCount() > 0)
  {
    SFXDescription* orig_db = mDescriptionObject;
    desc_db = new SFXDescription(*orig_db, true);
    orig_db->performSubstitutions(desc_db, owner, index);
  }
  else
    desc_db = 0;

  if (this->getSubstitutionCount() > 0 || desc_db)
  {
    sub_profile_db = new afxAudioBank(*this, true);
    performSubstitutions(sub_profile_db, owner, index);
    if (desc_db)
      sub_profile_db->mDescriptionObject = desc_db;
  }

  return sub_profile_db;
}

void afxAudioBank::onPerformSubstitutions() 
{ 
}

void afxAudioBank::initPersistFields()
{
  addField("path",        TypeFilename,             Offset(mPath, afxAudioBank),
    "A filesystem path to the folder containing the sound files specified by the "
    "filenames[] field. All sound files used in a single AudioBank must be located in "
    "the same folder.");
  addField("filenames",   TypeString,               Offset(mFilenames, afxAudioBank), 32,
    "Up to 32 names of sound files found in the path folder. The sound that is actually "
    "played by an Audio Bank effect is determined by the playIndex field.");
  addField("description", TYPEID<SFXDescription>(), Offset(mDescriptionObject, afxAudioBank),
    "SFXDescription datablock to use with this set of sounds.");
  addField("preload",     TypeBool,                 Offset(mPreload, afxAudioBank),
    "If set to true, file is pre-loaded, otherwise it is loaded on-demand.");
  addField("playIndex",   TypeS32,                  Offset(play_index, afxAudioBank),
    "An array index that selects a sound to play from the filenames[] field. Values "
    "outside of the range of assigned filename[] entries will not play any sound.");

  Parent::initPersistFields();
}

bool afxAudioBank::preload(bool server, String &errorStr)
{
  if(!Parent::preload(server, errorStr))
    return false;

  return true;
}

bool afxAudioBank::onAdd()
{
  if (!Parent::onAdd())
    return false;

  if (!mDescriptionObject && mDescriptionObjectID)
    Sim::findObject(mDescriptionObjectID , mDescriptionObject);

  // if this is client side, make sure that description is as well
  if(mDescriptionObject)
  {  // client side dataBlock id's are not in the dataBlock id range
    if (getId() >= DataBlockObjectIdFirst && getId() <= DataBlockObjectIdLast)
    {
      SimObjectId pid = mDescriptionObject->getId();
      if (pid < DataBlockObjectIdFirst || pid > DataBlockObjectIdLast)
      {
        Con::errorf(ConsoleLogEntry::General,"afxAudioBank: data dataBlock not networkable (use datablock to create).");
        return false;
      }
    }
  }

  return(true);
}

void afxAudioBank::packData(BitStream* stream)
{
  Parent::packData(stream);

  if (stream->writeFlag(mDescriptionObject))
    stream->writeRangedU32(mDescriptionObject->getId(), DataBlockObjectIdFirst, DataBlockObjectIdLast);

  /*
  char buffer[256];
  if(!mFilename)
    buffer[0] = 0;
  else
    dStrcpy(buffer, mFilename);
  stream->writeString(buffer);
  */

  stream->writeString(mPath);

  for (S32 i = 0; i < 32; i++)
  {
    stream->writeString(mFilenames[i]);
    if (mFilenames[i] == ST_NULLSTRING)
      break;
  }

  stream->writeFlag(mPreload);

  if (stream->writeFlag(play_index >= 0 && play_index < 32))
    stream->writeInt(play_index, 5);
}

void afxAudioBank::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  if (stream->readFlag()) // AudioDescription
  {
    SimObjectId id = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
    mDescriptionObjectID = id;
    Sim::findObject(id, mDescriptionObject);
  }

  // Filename
  /*
  char buffer[256];
  stream->readString(buffer);
  mFilename = StringTable->insert(buffer);
  */

  char buffer[256]; 

  stream->readString(buffer);
  mPath = StringTable->insert(buffer);

  for (S32 i = 0; i < 32; i++)
  {
    stream->readString(buffer);
    mFilenames[i] = StringTable->insert(buffer);
    if (mFilenames[i] == ST_NULLSTRING)
      break;
  }

  mPreload = stream->readFlag(); // Preload

  if (stream->readFlag())
    play_index = stream->readInt(5);
  else
    play_index = -1;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

