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

#pragma once
#include "platform/platformDlibrary.h"
#include "console/engineAPI.h"
#include "console/simBase.h"

#define CALL_CINTERFACE_FUNCTION(name, ...){const char *v[] = { __VA_ARGS__ }; CInterface::CallFunction(name, v, sizeof(v) / sizeof(*v));}

class CInterface {
   typedef bool(*IsMethodCallback)(const char* className, const char* methodName);
   typedef void(*CallMainCallback)();
   typedef const char* (*CallFunctionCallback)(const char* nameSpace, const char* name, const char **argv, int argc, bool *result);
   typedef const char* (*CallMethodCallback)(const char* className, const char* classNamespace, U32 object, const char* name, const char **argv, int argc, bool *result);
   IsMethodCallback mIsMethodCallback;
   CallFunctionCallback mFunctionCallback;
   CallMethodCallback mMethodCallback;
   CallMainCallback mMainCallback;
   const char* _CallFunction(const char* nameSpace, const char* name, const char **argv, int argc, bool *result) const;
   const char* _CallMethod(const char* className, const char* classNamespace, U32 object, const char* name, const char **argv, int argc, bool *res) const;
   void _CallMain(bool *res) const;
   bool _isMethod(const char* className, const char* methodName) const;
public:
   CInterface()
   {
      mFunctionCallback = NULL;
      mMethodCallback = NULL;
      mIsMethodCallback = NULL;
      mMainCallback = NULL;
   }

   static const char* CallFunction(const char* nameSpace, const char* name, const char **argv, int argc, bool *result);
   static const char* CallMethod(SimObject* obj, const char* name, const char **argv, int argc, bool *res);
   static void CallMain(bool *res);
   static bool isMethod(const char* className, const char* methodName);
   static CInterface& GetCInterface();
   void SetCallFunctionCallback(void* ptr) { mFunctionCallback = (CallFunctionCallback)ptr; };
   void SetCallMethodCallback(void* ptr) { mMethodCallback = (CallMethodCallback)ptr; };
   void SetCallIsMethodCallback(void* ptr) { mIsMethodCallback = (IsMethodCallback)ptr; };
   void SetMainCallback(void* ptr) { mMainCallback = (CallMainCallback)ptr; };
};