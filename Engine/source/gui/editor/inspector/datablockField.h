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

#ifndef _GUI_INSPECTOR_DATABLOCKFIELD_H_
#define _GUI_INSPECTOR_DATABLOCKFIELD_H_

#include "gui/editor/guiInspectorTypes.h"


//-----------------------------------------------------------------------------
// GuiInspectorDatablockField - custom field type for datablock enumeration
//-----------------------------------------------------------------------------
class GuiInspectorDatablockField : public GuiInspectorTypeMenuBase
{
   public:
      
      typedef GuiInspectorTypeMenuBase Parent;
      
   protected:

      AbstractClassRep *mDesiredClass;

      virtual SimSet* _getDatablockSet() const { return Sim::getDataBlockGroup(); }
      virtual void _populateMenu( GuiPopUpMenuCtrl* menu );
      
   public:
      
      DECLARE_CONOBJECT(GuiInspectorDatablockField);
      
      GuiInspectorDatablockField( StringTableEntry className );
      GuiInspectorDatablockField() { mDesiredClass = NULL; };

      void setClassName( StringTableEntry className );
};

//-----------------------------------------------------------------------------
// TypeSFXDescriptionName GuiInspectorField Class
//-----------------------------------------------------------------------------
class GuiInspectorTypeSFXDescriptionName : public GuiInspectorDatablockField
{
   public:
   
      typedef GuiInspectorDatablockField Parent;
      
   protected:
   
      virtual SimSet* _getDatablockSet() const { return Sim::getSFXDescriptionSet(); }
   
   public:
      
      DECLARE_CONOBJECT(GuiInspectorTypeSFXDescriptionName);
      static void consoleInit();
};


//-----------------------------------------------------------------------------
// TypeSFXTrackName GuiInspectorField Class
//-----------------------------------------------------------------------------
class GuiInspectorTypeSFXTrackName : public GuiInspectorDatablockField
{
   public:
   
      typedef GuiInspectorDatablockField Parent;
      
   protected:
   
      virtual SimSet* _getDatablockSet() const { return Sim::getSFXTrackSet(); }
      
   public:

      DECLARE_CONOBJECT(GuiInspectorTypeSFXTrackName);
      static void consoleInit();
};


//-----------------------------------------------------------------------------
// TypeSFXEnvironmentName GuiInspectorField Class
//-----------------------------------------------------------------------------
class GuiInspectorTypeSFXEnvironmentName : public GuiInspectorDatablockField
{
   public:
   
      typedef GuiInspectorDatablockField Parent;
      
   protected:
   
      virtual SimSet* _getDatablockSet() const { return Sim::getSFXEnvironmentSet(); }
   
   public:
      DECLARE_CONOBJECT(GuiInspectorTypeSFXEnvironmentName);
      static void consoleInit();
};


//-----------------------------------------------------------------------------
// TypeSFXAmbienceName GuiInspectorField Class
//-----------------------------------------------------------------------------
class GuiInspectorTypeSFXAmbienceName : public GuiInspectorDatablockField
{
   public:
   
      typedef GuiInspectorDatablockField Parent;
      
   protected:
   
      virtual SimSet* _getDatablockSet() const { return Sim::getSFXAmbienceSet(); }
   
   public:
   
      DECLARE_CONOBJECT(GuiInspectorTypeSFXAmbienceName);
      static void consoleInit();
};


#endif