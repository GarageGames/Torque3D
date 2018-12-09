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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
#include "platform/platform.h"
#include "console/simDatablock.h"

#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnectionEvents.h"
#include "T3D/gameBase/gameConnection.h"

#include "core/stream/bitStream.h"
#include "console/compiler.h"

IMPLEMENT_CO_DATABLOCK_V1(SimDataBlock);
SimObjectId SimDataBlock::sNextObjectId = DataBlockObjectIdFirst;
S32 SimDataBlock::sNextModifiedKey = 0;

ConsoleDocClass( SimDataBlock,
   "@brief \n"
   "@ingroup \n"
   
   "@section Datablock_Networking Datablocks and Networking\n"
   
   "@section Datablock_ClientSide Client-Side Datablocks\n"
);



//-----------------------------------------------------------------------------

SimDataBlock::SimDataBlock()
{
   setModDynamicFields(true);
   setModStaticFields(true);
}
// this implements a simple structure for managing substitution statements.

SimDataBlock::SubstitutionStatement::SubstitutionStatement(StringTableEntry slot, S32 idx, const char* value)
{
   this->mSlot = slot;
   this->mIdx = idx;
   this->mValue = dStrdup(value);
}

SimDataBlock::SubstitutionStatement::~SubstitutionStatement()
{
   dFree(mValue);
}

void SimDataBlock::SubstitutionStatement::replaceValue(const char* value)
{
   dFree(this->mValue);
   this->mValue = dStrdup(value);
}

// this is the copy-constructor for creating temp-clones.
SimDataBlock::SimDataBlock(const SimDataBlock& other, bool temp_clone) : SimObject(other, temp_clone)
{
   modifiedKey = other.modifiedKey;
}

// a destructor is added to SimDataBlock so that we can delete any substitutions.
SimDataBlock::~SimDataBlock()
{
   clear_substitutions();  
}

void SimDataBlock::clear_substitutions()
{
   for (S32 i = 0; i < substitutions.size(); i++)
      delete substitutions[i];
   substitutions.clear();
} 

void SimDataBlock::addSubstitution(StringTableEntry slot, S32 idx, const char* subst)
{
   AssertFatal(subst != 0 && subst[0] == '$' && subst[1] == '$', "Bad substition statement string added");

   subst += 2;
   while (dIsspace(*subst))
      subst++;

   bool empty_subs = (*subst == '\0');

   for (S32 i = 0; i < substitutions.size(); i++)
   {
      if (substitutions[i] && substitutions[i]->mSlot == slot && substitutions[i]->mIdx == idx)
      {
         if (empty_subs)
         {
            delete substitutions[i];
            substitutions[i] = 0;
            onRemoveSubstitution(slot, idx);
         }
         else
         {
            substitutions[i]->replaceValue(subst);
            onAddSubstitution(slot, idx, subst);
         }
         return;
      }
   }

   if (!empty_subs)
   {
      substitutions.push_back(new SubstitutionStatement(slot, idx, subst));
      onAddSubstitution(slot, idx, subst);
   }
}

const char* SimDataBlock::getSubstitution(StringTableEntry slot, S32 idx)
{
   for (S32 i = 0; i < substitutions.size(); i++)
   {
      if (substitutions[i] && substitutions[i]->mSlot == slot && substitutions[i]->mIdx == idx)
         return substitutions[i]->mValue;
   }

   return 0;
}

bool SimDataBlock::fieldHasSubstitution(StringTableEntry slot)
{
   for (S32 i = 0; i < substitutions.size(); i++)
      if (substitutions[i] && substitutions[i]->mSlot == slot)
         return true;
   return false;
}

void SimDataBlock::printSubstitutions()
{
   for (S32 i = 0; i < substitutions.size(); i++)
      if (substitutions[i])
         Con::errorf("SubstitutionStatement[%s] = \"%s\" -- %d", substitutions[i]->mSlot, substitutions[i]->mValue, i);
}

void SimDataBlock::copySubstitutionsFrom(SimDataBlock* other)
{
   clear_substitutions();
   if (!other)
      return;

   for (S32 i = 0; i < other->substitutions.size(); i++)
   {
      if (other->substitutions[i])
      {
         SubstitutionStatement* subs = other->substitutions[i];
         substitutions.push_back(new SubstitutionStatement(subs->mSlot, subs->mIdx, subs->mValue));
      }
   }
}


// This is the method that evaluates any substitution statements on a datablock and does the
// actual replacement of substituted datablock fields. 
//
// Much of the work is done by passing the statement to Con::evaluate() but first there are
// some key operations performed on the statement. 
//   -- Instances of "%%" in the statement are replaced with the id of the <obj> object.
//   -- Instances of "##" are replaced with the value of <index>.
// 
// There are also some return values that get special treatment.
//   -- An empty result will produce a realtime error message.
//   -- A result of "~~" will leave the original field value unchanged.
//   -- A result of "~0" will clear the original field to "" without producing an error message.
//
void SimDataBlock::performSubstitutions(SimDataBlock* dblock, const SimObject* obj, S32 index)
{
   if (!dblock || !dblock->getClassRep())
   {
      // error message
      return;
   }

   char obj_str[32];
   dStrcpy(obj_str, Con::getIntArg(obj->getId()), 32);

   char index_str[32];
   dStrcpy(index_str, Con::getIntArg(index), 32);

   for (S32 i = 0; i < substitutions.size(); i++)
   {
      if (substitutions[i])
      {
         static char buffer[1024];
         static char* b_oob = &buffer[1024];
         char* b = buffer;

         // perform special token expansion (%% and ##)
         const char* v = substitutions[i]->mValue;
         while (*v != '\0')
         {
            // identify "%%" tokens and replace with <obj> id
            if (v[0] == '%' && v[1] == '%')
            {
               const char* s = obj_str;
               while (*s != '\0')
               {
                  b[0] = s[0];
                  b++;
                  s++;
               }
               v += 2;
            }
            // identify "##" tokens and replace with <index> value
            else if (v[0] == '#' && v[1] == '#')
            {
               const char* s = index_str;
               while (*s != '\0')
               {
                  b[0] = s[0];
                  b++;
                  s++;
               }
               v += 2;
            }
            else
            {
               b[0] = v[0];
               b++; 
               v++;
            }
         }

         AssertFatal((uintptr_t)b < (uintptr_t)b_oob, "Substitution buffer overflowed");

         b[0] = '\0';

         // perform the statement evaluation
         Compiler::gSyntaxError = false;
         //Con::errorf("EVAL [%s]", avar("return %s;", buffer));
         const char *result = Con::evaluate(avar("return %s;", buffer), false, 0);
         if (Compiler::gSyntaxError)
         {
            Con::errorf("Field Substitution Failed: field=\"%s\" substitution=\"%s\" -- syntax error", 
               substitutions[i]->mSlot, substitutions[i]->mValue);
            Compiler::gSyntaxError = false;
            return;
         }

         // output a runtime console error when a substitution produces and empty result.
         if (result == 0 || result[0] == '\0')
         {
            Con::errorf("Field Substitution Failed: field=\"%s\" substitution=\"%s\" -- empty result", 
               substitutions[i]->mSlot, substitutions[i]->mValue);
            return;
         }

         // handle special return values
         if (result[0] == '~')
         {
            // if value is "~~" then keep the existing value
            if (result[1] == '~' && result[2] == '\0')
               continue;
            // if "~0" then clear it
            if (result[1] == '0' && result[2] == '\0')
               result = "";
         }

         const AbstractClassRep::Field* field = dblock->getClassRep()->findField(substitutions[i]->mSlot);
         if (!field)
         {
            // this should be very unlikely...
            Con::errorf("Field Substitution Failed: unknown field, \"%s\".", substitutions[i]->mSlot);
            continue;
         }

         if (field->keepClearSubsOnly && result[0] != '\0')
         {
            Con::errorf("Field Substitution Failed: field \"%s\" of datablock %s only allows \"$$ ~~\" (keep) and \"$$ ~0\" (clear) field substitutions. [%s]", 
               substitutions[i]->mSlot, this->getClassName(), this->getName());
            continue;
         }

         // substitute the field value with its replacement
         Con::setData(field->type, (void*)(((const char*)(dblock)) + field->offset), substitutions[i]->mIdx, 1, &result, field->table, field->flag);

         //dStrncpy(buffer, result, 255);
         //Con::errorf("SUBSTITUTION %s.%s[%d] = %s idx=%s", Con::getIntArg(getId()), substitutions[i]->slot, substitutions[i]->idx, buffer, index_str);

         // notify subclasses of a field modification
         dblock->onStaticModified(substitutions[i]->mSlot);
      }
   }

   // notify subclasses of substitution operation
   if (substitutions.size() > 0)
      dblock->onPerformSubstitutions();
}

//-----------------------------------------------------------------------------

bool SimDataBlock::onAdd()
{
   Parent::onAdd();

   // This initialization is done here, and not in the constructor,
   // because some jokers like to construct and destruct objects
   // (without adding them to the manager) to check what class
   // they are.
   modifiedKey = ++sNextModifiedKey;
   AssertFatal(sNextObjectId <= DataBlockObjectIdLast,
      "Exceeded maximum number of data blocks");

   // add DataBlock to the DataBlockGroup unless it is client side ONLY DataBlock
   if ( !isClientOnly() )
      if (SimGroup* grp = Sim::getDataBlockGroup())
         grp->addObject(this);
         
   Sim::getDataBlockSet()->addObject( this );

   return true;
}

//-----------------------------------------------------------------------------

void SimDataBlock::assignId()
{
   // We don't want the id assigned by the manager, but it may have
   // already been assigned a correct data block id.
   if ( isClientOnly() )
      setId(sNextObjectId++);
}

//-----------------------------------------------------------------------------

void SimDataBlock::onStaticModified(const char* slotName, const char* newValue)
{
   modifiedKey = sNextModifiedKey++;
}

//-----------------------------------------------------------------------------

// packData() and unpackData() do nothing in the stock implementation, but here
// they've been modified to pack and unpack any substitution statements.
//
void SimDataBlock::packData(BitStream* stream)
{
   for (S32 i = 0; i < substitutions.size(); i++)
   {
      if (substitutions[i])
      {
         stream->writeFlag(true);
         stream->writeString(substitutions[i]->mSlot);
         stream->write(substitutions[i]->mIdx);
         stream->writeString(substitutions[i]->mValue);
      }
   }
   stream->writeFlag(false);
}

void SimDataBlock::unpackData(BitStream* stream)
{
   clear_substitutions();
   while(stream->readFlag())
   {
      char slotName[256];
      S32 idx;
      char value[256];
      stream->readString(slotName);
      stream->read(&idx);
      stream->readString(value);
      substitutions.push_back(new SubstitutionStatement(StringTable->insert(slotName), idx, value));
   }
}

//-----------------------------------------------------------------------------

bool SimDataBlock::preload(bool, String&)
{
   return true;
}

//-----------------------------------------------------------------------------

void SimDataBlock::write(Stream &stream, U32 tabStop, U32 flags)
{
   // Only output selected objects if they want that.
   if((flags & SelectedOnly) && !isSelected())
      return;

   stream.writeTabs(tabStop);
   char buffer[1024];

   // Client side datablocks are created with 'new' while
   // regular server datablocks use the 'datablock' keyword.
   if ( isClientOnly() )
      dSprintf(buffer, sizeof(buffer), "new %s(%s) {\r\n", getClassName(), getName() ? getName() : "");
   else
      dSprintf(buffer, sizeof(buffer), "datablock %s(%s) {\r\n", getClassName(), getName() ? getName() : "");

   stream.write(dStrlen(buffer), buffer);
   writeFields(stream, tabStop + 1);

   stream.writeTabs(tabStop);
   stream.write(4, "};\r\n");
}

//=============================================================================
//    API.
//=============================================================================
// MARK: ---- API ----

//-----------------------------------------------------------------------------

DefineEngineMethod( SimDataBlock, reloadOnLocalClient, void, (),,
   "Reload the datablock.  This can only be used with a local client configuration." )
{
   // Make sure we're running a local client.

   GameConnection* localClient = GameConnection::getLocalClientConnection();
   if( !localClient )
      return;

   // Do an in-place pack/unpack/preload.

   if( !object->preload( true, NetConnection::getErrorBuffer() ) )
   {
      Con::errorf( NetConnection::getErrorBuffer() );
      return;
   }

   U8 buffer[ 16384 ];
   BitStream stream( buffer, 16384 );

   object->packData( &stream );
   stream.setPosition(0);
   object->unpackData( &stream );

   if( !object->preload( false, NetConnection::getErrorBuffer() ) )
   {
      Con::errorf( NetConnection::getErrorBuffer() );
      return;
   }

   // Trigger a post-apply so that change notifications respond.
   object->inspectPostApply();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( preloadClientDataBlocks, void, (),,
   "Preload all datablocks in client mode.\n\n"
   "(Server parameter is set to false).  This will take some time to complete.")
{
   // we go from last to first because we cut 'n pasted the loop from deleteDataBlocks
   SimGroup *grp = Sim::getDataBlockGroup();
   String errorStr;
   for(S32 i = grp->size() - 1; i >= 0; i--)
   {
      AssertFatal(dynamic_cast<SimDataBlock*>((*grp)[i]), "Doh! non-datablock in datablock group!");
      SimDataBlock *obj = (SimDataBlock*)(*grp)[i];
      if (!obj->preload(false, errorStr))
         Con::errorf("Failed to preload client datablock, %s: %s", obj->getName(), errorStr.c_str());
   }
}

//-----------------------------------------------------------------------------

DefineEngineFunction( deleteDataBlocks, void, (),,
   "Delete all the datablocks we've downloaded.\n\n"
   "This is usually done in preparation of downloading a new set of datablocks, "
   "such as occurs on a mission change, but it's also good post-mission cleanup." )
{
   // delete from last to first:
   SimGroup *grp = Sim::getDataBlockGroup();
   grp->deleteAllObjects();
   SimDataBlock::sNextObjectId = DataBlockObjectIdFirst;
   SimDataBlock::sNextModifiedKey = 0;
}
