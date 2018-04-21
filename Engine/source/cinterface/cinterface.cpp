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
#include "cinterface.h"

#include "console/compiler.h"
#include "windowManager/platformWindow.h"

CInterface& CInterface::GetCInterface()
{
   static CInterface INSTANCE;
   return INSTANCE;
}

bool CInterface::isMethod(const char* className, const char* methodName)
{
   return GetCInterface()._isMethod(className, methodName);
}

const char* CInterface::CallFunction(const char* nameSpace, const char* name, const char **argv, int argc, bool *result)
{
   return GetCInterface()._CallFunction(nameSpace, name, argv, argc, result);
}

const char* CInterface::CallMethod(SimObject* obj, const char* name, const char **argv, int argc, bool *res)
{
   return GetCInterface()._CallMethod(obj->getClassName(), obj->getClassNamespace(), obj->getId(), name, argv, argc, res);
}

void CInterface::CallMain(bool *res)
{
   GetCInterface()._CallMain(res);
}

bool CInterface::_isMethod(const char* className, const char* methodName) const
{
   if (mIsMethodCallback)
      return mIsMethodCallback(className, methodName);

   return NULL;
}

const char* CInterface::_CallFunction(const char* nameSpace, const char* name, const char **argv, int argc, bool *result) const
{
   if (mFunctionCallback)
      return mFunctionCallback(nameSpace, name, argv, argc, result);

   *result = false;
   return NULL;
}

const char* CInterface::_CallMethod(const char* className, const char* classNamespace, U32 object, const char* name, const char **argv, int argc, bool *res) const
{
   if (mMethodCallback)
      return mMethodCallback(className, classNamespace, object, name, argv, argc, res);

   *res = false;
   return NULL;
}

void CInterface::_CallMain(bool *res) const
{
   if (mMainCallback)
   {
      *res = true;
      mMainCallback();
      return;
   }

   *res = false;
}

TORQUE_API void SetCallbacks(void* ptr, void* methodPtr, void* isMethodPtr, void* mainPtr) {
   CInterface::GetCInterface().SetCallFunctionCallback(ptr);
   CInterface::GetCInterface().SetCallMethodCallback(methodPtr);
   CInterface::GetCInterface().SetCallIsMethodCallback(isMethodPtr);
   CInterface::GetCInterface().SetMainCallback(mainPtr);
}