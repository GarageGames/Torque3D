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

#include "console/consoleInternal.h"
#include "console/simDatablock.h"
#include "console/simSet.h"

// External scripting cinterface, suitable for import into any scripting system which support "C" interfaces (C#, Python, Lua, Java, etc)


extern "C" {

   SimObject* Sim_FindObjectById(U32 pId)
   {
      return Sim::findObject(pId);
   }

   SimObject* Sim_FindObjectByName(const char* pName)
   {
      return Sim::findObject(StringTable->insert(pName));
   }

   SimObject* Sim_FindDataBlockByName(const char* pName)
   {
      return Sim::getDataBlockGroup()->findObject(StringTable->insert(pName));
   }

   SimObjectPtr<SimObject>* Sim_WrapObject(SimObject* pObject)
   {
      return new SimObjectPtr<SimObject>(pObject);
   }

   void Sim_DeleteObjectPtr(SimObjectPtr<SimObject>* pObjectPtr)
   {
      delete pObjectPtr;
   }
};