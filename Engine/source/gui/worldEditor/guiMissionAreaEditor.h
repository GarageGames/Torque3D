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

#ifndef _GUIMISSIONAREAEDITORCTRL_H_
#define _GUIMISSIONAREAEDITORCTRL_H_

#ifndef _EDITTSCTRL_H_
#include "gui/worldEditor/editTSCtrl.h"
#endif
#ifndef _MISSIONAREA_H_
#include "T3D/missionArea.h"
#endif

class GuiMissionAreaEditorCtrl : public EditTSCtrl
{
   typedef EditTSCtrl Parent;

protected:
   SimObjectPtr<MissionArea>  mSelMissionArea;

public:
   GuiMissionAreaEditorCtrl();
   virtual ~GuiMissionAreaEditorCtrl();
   
   DECLARE_CONOBJECT(GuiMissionAreaEditorCtrl);

   // SimObject
   bool onAdd();
   static void initPersistFields();

   // EditTSCtrl      
   void get3DCursor( GuiCursor *&cursor, bool &visible, const Gui3DMouseEvent &event_ );

   void setSelectedMissionArea( MissionArea *missionArea );
   MissionArea* getSelectedMissionArea() { return mSelMissionArea; };
};

#endif // _GUIMISSIONAREAEDITORCTRL_H_
