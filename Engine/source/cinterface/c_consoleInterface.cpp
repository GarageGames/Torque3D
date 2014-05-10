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

#include "platform/platform.h"
#include "console/compiler.h"
#include "console/consoleInternal.h"
#include "console/simSet.h"

extern "C" {

   // SimObject C interface
   const char *SimObject_GetName(SimObject *so)
   {
      return so->getName();
   }

   U32 SimObject_GetId(SimObject *so)
   {
      return so->getId();
   }

   const char *SimObject_GetClassName(SimObject *so)
   {
      return so->getClassName();
   }

   void *SimObject_GetFieldList(SimObject *so, S32 &outNumFields)
   {
      const AbstractClassRep::FieldList &fl = so->getFieldList();
      outNumFields = fl.size();
      return fl.address();
   }

   bool SimObject_IsLocked(SimObject *so)
   {
      return so->isLocked();
   }

   void SimObject_SetDataField(SimObject *so, const char *fieldName, const char *arr, const char *val)
   {
      so->setDataField(StringTable->insert(fieldName), arr, val);
   }

   const char *SimObject_GetDataField(SimObject *so, const char *fieldName, const char *arr)
   {
      return so->getDataField(StringTable->insert(fieldName), arr);
   }

   void SimObject_InspectPreApply(SimObject *so)
   {
      so->inspectPreApply();
   }

   void SimObject_InspectPostApply(SimObject *so)
   {
      so->inspectPostApply();
   }

   // Con C interface
   void Con_AddConsumer(ConsumerCallback cb)
   {
      Con::addConsumer(cb);
   }

   void Con_RemoveConsumer(ConsumerCallback cb)
   {
      Con::removeConsumer(cb);
   }

   void Con_AddCommand_String(StringCallback cb, const char *nameSpace, const char *funcName, const char* usage,  S32 minArgs, S32 maxArgs)
   {
      if (!nameSpace || !dStrlen(nameSpace))
         Con::addCommand(funcName, cb, usage, minArgs + 1, maxArgs + 1);
      else
         Con::addCommand(nameSpace, funcName, cb, usage, minArgs + 1, maxArgs + 1);
   }

   // ConsoleBaseType C interface
   ConsoleBaseType *ConsoleBaseType_GetTypeById(const S32 typeId)
   {
      return ConsoleBaseType::getType(typeId);
   }

   S32 ConsoleBaseType_GetTypeId(ConsoleBaseType *cbt)
   {
      return cbt->getTypeID();
   }

   S32 ConsoleBaseType_GetTypeSize(ConsoleBaseType *cbt)
   {
      return cbt->getTypeSize();
   }

   const char *ConsoleBaseType_GetTypeName(ConsoleBaseType *cbt)
   {
      return cbt->getTypeName();
   }

   const char *ConsoleBaseType_GetInspectorFieldType(ConsoleBaseType *cbt)
   {
      return cbt->getInspectorFieldType();
   }

   void ConsoleBaseType_SetData(ConsoleBaseType *cbt, void *dptr, S32 argc, const char **argv, const EnumTable *tbl, BitSet32 flag)
   {
      return cbt->setData(dptr, argc, argv, tbl, flag);
   }

   const char *ConsoleBaseType_GetData(ConsoleBaseType *cbt, void *dptr, const EnumTable *tbl, BitSet32 flag)
   {
      return cbt->getData(dptr, tbl, flag);
   }

   // Abstract Class Rep
   AbstractClassRep *AbstractClassRep_GetCommonParent(AbstractClassRep *acr, AbstractClassRep *otheracr)
   {
      return acr->getCommonParent(otheracr);
   }

   AbstractClassRep *AbstractClassRep_FindClassRep(const char* in_pClassName)
   {
      return AbstractClassRep::findClassRep(in_pClassName);
   }

   U32 AbstractClassRep_GetFieldStructSize()
   {
      return sizeof(AbstractClassRep::Field);
   }

   // Sim C interface
   SimObject *Sim_FindObjectByString(const char *param)
   {
      return Sim::findObject(param);
   }
   
   SimObject *Sim_FindObjectById(S32 param)
   {
      return Sim::findObject(param);
   }

   // Sim Set
   SimObject **SimSet_Begin(SimObject *simObject)
   {
      return dynamic_cast<SimSet *>(simObject)->begin();
   }

   SimObject **SimSet_End(SimObject *simObject)
   {
      return dynamic_cast<SimSet *>(simObject)->end();
   }

};


