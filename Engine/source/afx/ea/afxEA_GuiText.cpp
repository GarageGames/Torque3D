
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
#include "afx/afxChoreographer.h"
#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/ce/afxGuiText.h"
#include "afx/ui/afxGuiTextHud.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_GuiText 

class afxEA_GuiText : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  enum {
    UNDEFINED,
    USER_TEXT,
    SHAPE_NAME
  };

  afxGuiTextData* text_data;
  S32             text_src;
  ColorF          text_clr;

  void            do_runtime_substitutions();

public:
  /*C*/           afxEA_GuiText();

  virtual void    ea_set_datablock(SimDataBlock*);
  virtual bool    ea_start();
  virtual bool    ea_update(F32 dt);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_GuiText::afxEA_GuiText()
{
  text_data = 0;
  text_src = UNDEFINED;
  text_clr.set(1,1,1,1);
}

void afxEA_GuiText::ea_set_datablock(SimDataBlock* db)
{
  text_data = dynamic_cast<afxGuiTextData*>(db);
}

bool afxEA_GuiText::ea_start()
{
  if (!text_data)
  {
    Con::errorf("afxEA_GuiText::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  do_runtime_substitutions();

  if (text_data->text_str == ST_NULLSTRING || text_data->text_str[0] == '\0')
  {
    Con::errorf("afxEA_GuiText::ea_start() -- empty text string.");
    return false;
  }

  text_clr = text_data->text_clr;

  if (dStricmp("#shapeName", text_data->text_str) == 0)
    text_src = SHAPE_NAME;
  else
    text_src = USER_TEXT;

  return true;
}

bool afxEA_GuiText::ea_update(F32 dt)
{
  switch (text_src)
  {
  case USER_TEXT:
    {
      ColorF temp_clr = text_clr;
      if (do_fades)
        temp_clr.alpha = fade_value;
      afxGuiTextHud::addTextItem(updated_pos, text_data->text_str, temp_clr);
    }
    break;
  case SHAPE_NAME:
    {
      const char* name = 0;
      SceneObject* cons_obj = 0;
      afxConstraint* pos_cons = getPosConstraint();
      if (pos_cons)
      {
        ShapeBase* shape = dynamic_cast<ShapeBase*>(pos_cons->getSceneObject());
        if (shape)
        {
          name = shape->getShapeName();
          cons_obj = shape;
        }
      }
      if (name && name[0] != '\0')
      {
        ColorF temp_clr = text_clr;
        if (do_fades)
          temp_clr.alpha = fade_value;
        afxGuiTextHud::addTextItem(updated_pos, name, temp_clr, cons_obj);
      }
    }
    break;
  }

  return true;
}

void afxEA_GuiText::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (text_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxGuiTextData* orig_db = text_data;
    text_data = new afxGuiTextData(*orig_db, true);
    orig_db->performSubstitutions(text_data, choreographer, group_index);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_GuiTextDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_GuiTextDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_GuiText; }
};

afxEA_GuiTextDesc afxEA_GuiTextDesc::desc;

bool afxEA_GuiTextDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxGuiTextData) == typeid(*db));
}

bool afxEA_GuiTextDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//