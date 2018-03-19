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

#ifndef C_SIMOBJECTINTERFACE_H
#define C_SIMOBJECTINTERFACE_H
#include "platform/platformDlibrary.h"
#include "console/simObject.h"

extern "C" {
   DLL_DECL bool fnSimObject_registerObject(SimObject* pObject);
   DLL_DECL void fnSimObject_GetField(SimObject* obj, const char* fieldName, const char* arrayIndex);
   DLL_DECL void fnSimObject_SetField(SimObject* obj, const char* fieldName, const char* arrayIndex, const char* value);
   DLL_DECL void fnSimObject_CopyFrom(SimObject* obj, SimObject* parent);
   DLL_DECL void fnSimObject_SetMods(SimObject* obj, bool modStaticFields, bool modDynamicFields);
   DLL_DECL bool fnSimObject_IsLocked(SimObject *so);
   DLL_DECL void fnSimObject_InspectPreApply(SimObject *so);
   DLL_DECL void fnSimObject_InspectPostApply(SimObject *so);
}
#endif // C_SIMOBJECTINTERFACE_H
