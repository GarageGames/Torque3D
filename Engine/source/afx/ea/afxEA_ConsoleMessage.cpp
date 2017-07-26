
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

#include <typeinfo>
#include "afx/arcaneFX.h"
#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/ce/afxConsoleMessage.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_ConsoleMessage 

class afxEA_ConsoleMessage : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;
  
  afxConsoleMessageData*  message_data;
  bool              displayed;
  
  void              do_runtime_substitutions();

public:
  /*C*/             afxEA_ConsoleMessage();

  virtual bool      isDone() { return displayed; }

  virtual void      ea_set_datablock(SimDataBlock*);
  virtual bool      ea_start();
  virtual bool      ea_update(F32 dt);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_ConsoleMessage::afxEA_ConsoleMessage()
{
  message_data = 0;
  displayed = false;
}

void afxEA_ConsoleMessage::ea_set_datablock(SimDataBlock* db)
{
  message_data = dynamic_cast<afxConsoleMessageData*>(db);
}

bool afxEA_ConsoleMessage::ea_start()
{
  if (!message_data)
  {
    Con::errorf("afxEA_ConsoleMessage::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  return true;
}

bool afxEA_ConsoleMessage::ea_update(F32 dt)
{
  if (!displayed)
  {
    if (message_data->message_str != ST_NULLSTRING)
      Con::printf("ConsoleMessage: \"%s\"", message_data->message_str);
    displayed = true;
  }

  return true;
}

void afxEA_ConsoleMessage::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (message_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxConsoleMessageData* orig_db = message_data;
    message_data = new afxConsoleMessageData(*orig_db, true);
    orig_db->performSubstitutions(message_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_ConsoleMessageDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_ConsoleMessageDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const { return false; }
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return true; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }
  virtual bool  isPositional(const afxEffectWrapperData*) const { return false; }

  virtual afxEffectWrapper* create() const { return new afxEA_ConsoleMessage; }
};

afxEA_ConsoleMessageDesc afxEA_ConsoleMessageDesc::desc;

bool afxEA_ConsoleMessageDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxConsoleMessageData) == typeid(*db));
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//