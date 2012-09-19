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

#ifndef _GUISCRIPTNOTIFYCTRL_H_
#define _GUISCRIPTNOTIFYCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

class GuiScriptNotifyCtrl : public GuiControl
{
private:
   typedef GuiControl Parent;
public:

    /// @name Event Callbacks
    /// @{ 
   bool mOnChildAdded;         ///< Script Notify : onAddObject(%object)  
   bool mOnChildRemoved;       ///< Script Notify : onRemoveObject(%object)
   bool mOnResize;             ///< Script Notify : onResize()
   bool mOnChildResized;       ///< Script Notify : onChildResized(%child)
   bool mOnParentResized;      ///< Script Notify : onParentResized()
   bool mOnLoseFirstResponder; ///< Script Notify : onLoseFirstResponder()
   bool mOnGainFirstResponder; ///< Script Notify : onGainFirstResponder()
    /// @}

public:
    /// @name Initialization
    /// @{
    DECLARE_CONOBJECT(GuiScriptNotifyCtrl);
    DECLARE_CATEGORY( "Gui Other Script" );
    DECLARE_DESCRIPTION( "A control that implements various script callbacks for\n"
                         "certain GUI events." );
    
    GuiScriptNotifyCtrl();
    virtual ~GuiScriptNotifyCtrl();
    static void initPersistFields();

    virtual bool resize(const Point2I &newPosition, const Point2I &newExtent);
    virtual void childResized(GuiScriptNotifyCtrl *child);
    virtual void parentResized(const RectI &oldParentRect, const RectI &newParentRect);
    virtual void onChildRemoved( GuiControl *child );
    virtual void onChildAdded( GuiControl *child );

	DECLARE_CALLBACK(void, onResize, (SimObjectId ID) );
	DECLARE_CALLBACK(void, onChildAdded, (SimObjectId ID, SimObjectId childID));
	DECLARE_CALLBACK(void, onChildRemoved, (SimObjectId ID, SimObjectId childID));
	DECLARE_CALLBACK(void, onChildResized, (SimObjectId ID, SimObjectId childID));
	DECLARE_CALLBACK(void, onParentResized, (SimObjectId ID));
	DECLARE_CALLBACK(void, onLoseFirstResponder, (SimObjectId ID));
	DECLARE_CALLBACK(void, onGainFirstResponder, (SimObjectId ID));
	
	
	
	
    //virtual void onMouseUp(const GuiEvent &event);
    //virtual void onMouseDown(const GuiEvent &event);
    //virtual void onMouseMove(const GuiEvent &event);
    //virtual void onMouseDragged(const GuiEvent &event);
    //virtual void onMouseEnter(const GuiEvent &event);
    //virtual void onMouseLeave(const GuiEvent &event);

    //virtual bool onMouseWheelUp(const GuiEvent &event);
    //virtual bool onMouseWheelDown(const GuiEvent &event);

    //virtual void onRightMouseDown(const GuiEvent &event);
    //virtual void onRightMouseUp(const GuiEvent &event);
    //virtual void onRightMouseDragged(const GuiEvent &event);

    //virtual void onMiddleMouseDown(const GuiEvent &event);
    //virtual void onMiddleMouseUp(const GuiEvent &event);
    //virtual void onMiddleMouseDragged(const GuiEvent &event);

    //virtual void onMouseDownEditor(const GuiEvent &event, Point2I offset);
    //virtual void onRightMouseDownEditor(const GuiEvent &event, Point2I offset);

    virtual void setFirstResponder(GuiControl *firstResponder);
    virtual void setFirstResponder();
    void clearFirstResponder();
    virtual void onLoseFirstResponder();

    //virtual void acceleratorKeyPress(U32 index);
    //virtual void acceleratorKeyRelease(U32 index);
    //virtual bool onKeyDown(const GuiEvent &event);
    //virtual bool onKeyUp(const GuiEvent &event);
    //virtual bool onKeyRepeat(const GuiEvent &event);

    virtual void onMessage(GuiScriptNotifyCtrl *sender, S32 msg);    ///< Receive a message from another control

    virtual void onDialogPush();
    virtual void onDialogPop();

};

#endif
