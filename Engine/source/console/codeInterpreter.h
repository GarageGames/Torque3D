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

#ifndef _CODEINTERPRETER_H_
#define _CODEINTERPRETER_H_

#include "console/codeBlock.h"
#include "console/console.h"
#include "console/consoleInternal.h"

/// Frame data for a foreach/foreach$ loop.
struct IterStackRecord
{
   /// If true, this is a foreach$ loop; if not, it's a foreach loop.
   bool mIsStringIter;

   /// The iterator variable.
   Dictionary::Entry* mVariable;

   /// Information for an object iterator loop.
   struct ObjectPos
   {
      /// The set being iterated over.
      SimSet* mSet;

      /// Current index in the set.
      U32 mIndex;
   };

   /// Information for a string iterator loop.
   struct StringPos
   {
      /// The raw string data on the string stack.
      StringStackPtr mString;

      /// Current parsing position.
      U32 mIndex;
   };

   union
   {
      ObjectPos mObj;
      StringPos mStr;
   } mData;
};

enum OPCodeReturn
{
   exitCode = -1,
   success = 0,
   breakContinue = 1
};

class CodeInterpreter
{
public:
   CodeInterpreter(CodeBlock *cb);
   ~CodeInterpreter();

   ConsoleValueRef exec(U32 ip, 
                        StringTableEntry functionName,
                        Namespace *thisNamespace,
                        U32 argc, 
                        ConsoleValueRef *argv, 
                        bool noCalls, 
                        StringTableEntry packageName, 
                        S32 setFrame);

   static void init();

   // Methods
private:
   void parseArgs(U32 &ip);

   /// Group op codes
   /// @{

   OPCodeReturn op_func_decl(U32 &ip);
   OPCodeReturn op_create_object(U32 &ip);
   OPCodeReturn op_add_object(U32 &ip);
   OPCodeReturn op_end_object(U32 &ip);
   OPCodeReturn op_finish_object(U32 &ip);
   OPCodeReturn op_jmpiffnot(U32 &ip);
   OPCodeReturn op_jmpifnot(U32 &ip);
   OPCodeReturn op_jmpiff(U32 &ip);
   OPCodeReturn op_jmpif(U32 &ip);
   OPCodeReturn op_jmpifnot_np(U32 &ip);
   OPCodeReturn op_jmpif_np(U32 &ip);
   OPCodeReturn op_jmp(U32 &ip);
   OPCodeReturn op_return_void(U32 &ip);
   OPCodeReturn op_return(U32 &ip);
   OPCodeReturn op_return_flt(U32 &ip);
   OPCodeReturn op_return_uint(U32 &ip);
   OPCodeReturn op_cmpeq(U32 &ip);
   OPCodeReturn op_cmpgr(U32 &ip);
   OPCodeReturn op_cmpge(U32 &ip);
   OPCodeReturn op_cmplt(U32 &ip);
   OPCodeReturn op_cmple(U32 &ip);
   OPCodeReturn op_cmpne(U32 &ip);
   OPCodeReturn op_xor(U32 &ip);
   OPCodeReturn op_mod(U32 &ip);
   OPCodeReturn op_bitand(U32 &ip);
   OPCodeReturn op_bitor(U32 &ip);
   OPCodeReturn op_not(U32 &ip);
   OPCodeReturn op_notf(U32 &ip);
   OPCodeReturn op_onescomplement(U32 &ip);
   OPCodeReturn op_shr(U32 &ip);
   OPCodeReturn op_shl(U32 &ip);
   OPCodeReturn op_and(U32 &ip);
   OPCodeReturn op_or(U32 &ip);
   OPCodeReturn op_add(U32 &ip);
   OPCodeReturn op_sub(U32 &ip);
   OPCodeReturn op_mul(U32 &ip);
   OPCodeReturn op_div(U32 &ip);
   OPCodeReturn op_neg(U32 &ip);
   OPCodeReturn op_inc(U32 &ip);
   OPCodeReturn op_dec(U32 &ip);
   OPCodeReturn op_setcurvar(U32 &ip);
   OPCodeReturn op_setcurvar_create(U32 &ip);
   OPCodeReturn op_setcurvar_array(U32 &ip);
   OPCodeReturn op_setcurvar_array_varlookup(U32 &ip);
   OPCodeReturn op_setcurvar_array_create(U32 &ip);
   OPCodeReturn op_setcurvar_array_create_varlookup(U32 &ip);
   OPCodeReturn op_loadvar_uint(U32 &ip);
   OPCodeReturn op_loadvar_flt(U32 &ip);
   OPCodeReturn op_loadvar_str(U32 &ip);
   OPCodeReturn op_loadvar_var(U32 &ip);
   OPCodeReturn op_savevar_uint(U32 &ip);
   OPCodeReturn op_savevar_flt(U32 &ip);
   OPCodeReturn op_savevar_str(U32 &ip);
   OPCodeReturn op_savevar_var(U32 &ip);
   OPCodeReturn op_setcurobject(U32 &ip);
   OPCodeReturn op_setcurobject_internal(U32 &ip);
   OPCodeReturn op_setcurobject_new(U32 &ip);
   OPCodeReturn op_setcurfield(U32 &ip);
   OPCodeReturn op_setcurfield_array(U32 &ip);
   OPCodeReturn op_setcurfield_type(U32 &ip);
   OPCodeReturn op_setcurfield_this(U32 &ip);
   OPCodeReturn op_setcurfield_array_var(U32 &ip);
   OPCodeReturn op_loadfield_uint(U32 &ip);
   OPCodeReturn op_loadfield_flt(U32 &ip);
   OPCodeReturn op_loadfield_str(U32 &ip);
   OPCodeReturn op_savefield_uint(U32 &ip);
   OPCodeReturn op_savefield_flt(U32 &ip);
   OPCodeReturn op_savefield_str(U32 &ip);
   OPCodeReturn op_str_to_uint(U32 &ip);
   OPCodeReturn op_str_to_flt(U32 &ip);
   OPCodeReturn op_str_to_none(U32 &ip);
   OPCodeReturn op_flt_to_uint(U32 &ip);
   OPCodeReturn op_flt_to_str(U32 &ip);
   OPCodeReturn op_flt_to_none(U32 &ip);
   OPCodeReturn op_uint_to_flt(U32 &ip);
   OPCodeReturn op_uint_to_str(U32 &ip);
   OPCodeReturn op_uint_to_none(U32 &ip);
   OPCodeReturn op_copyvar_to_none(U32 &ip);
   OPCodeReturn op_loadimmed_uint(U32 &ip);
   OPCodeReturn op_loadimmed_flt(U32 &ip);
   OPCodeReturn op_tag_to_str(U32 &ip);
   OPCodeReturn op_loadimmed_str(U32 &ip);
   OPCodeReturn op_docblock_str(U32 &ip);
   OPCodeReturn op_loadimmed_ident(U32 &ip);
   OPCodeReturn op_callfunc_resolve(U32 &ip);
   OPCodeReturn op_callfunc(U32 &ip);
   OPCodeReturn op_callfunc_pointer(U32 &ip);
   OPCodeReturn op_callfunc_this(U32 &ip);
   OPCodeReturn op_advance_str(U32 &ip);
   OPCodeReturn op_advance_str_appendchar(U32 &ip);
   OPCodeReturn op_advance_str_comma(U32 &ip);
   OPCodeReturn op_advance_str_nul(U32 &ip);
   OPCodeReturn op_rewind_str(U32 &ip);
   OPCodeReturn op_terminate_rewind_str(U32 &ip);
   OPCodeReturn op_compare_str(U32 &ip);
   OPCodeReturn op_push(U32 &ip);
   OPCodeReturn op_push_uint(U32 &ip);
   OPCodeReturn op_push_flt(U32 &ip);
   OPCodeReturn op_push_var(U32 &ip);
   OPCodeReturn op_push_this(U32 &ip);
   OPCodeReturn op_push_frame(U32 &ip);
   OPCodeReturn op_assert(U32 &ip);
   OPCodeReturn op_break(U32 &ip);
   OPCodeReturn op_iter_begin_str(U32 &ip);
   OPCodeReturn op_iter_begin(U32 &ip);
   OPCodeReturn op_iter(U32 &ip);
   OPCodeReturn op_iter_end(U32 &ip);
   OPCodeReturn op_invalid(U32 &ip);

   /// @}

private:
   CodeBlock *mCodeBlock;

   /// Group exec arguments.
   struct
   {
      StringTableEntry functionName;
      Namespace *thisNamespace;
      U32 argc;
      ConsoleValueRef *argv;
      bool noCalls;
      StringTableEntry packageName;
      S32 setFrame;
   } mExec;

   U32 mIterDepth;
   F64 *mCurFloatTable;
   char *mCurStringTable;
   StringTableEntry mThisFunctionName;
   bool mPopFrame;

   // Add local object creation stack [7/9/2007 Black]
   static const U32 objectCreationStackSize = 32;
   U32 mObjectCreationStackIndex;
   struct 
   {
      SimObject *newObject;
      U32 failJump;
   } mObjectCreationStack[objectCreationStackSize];

   SimObject *mCurrentNewObject;
   U32 mFailJump;
   StringTableEntry mPrevField;
   StringTableEntry mCurField;
   SimObject *mPrevObject;
   SimObject *mCurObject;
   SimObject *mSaveObject;
   SimObject *mThisObject;
   Namespace::Entry *mNSEntry;
   StringTableEntry mCurFNDocBlock;
   StringTableEntry mCurNSDocBlock;
   U32 mCallArgc;
   ConsoleValueRef *mCallArgv;
   CodeBlock *mSaveCodeBlock;

   // note: anything returned is pushed to CSTK and will be invalidated on the next exec()
   ConsoleValueRef mReturnValue;

   U32 mCurrentInstruction;

   static const S32 nsDocLength = 128;
   char mNSDocBlockClass[nsDocLength];
};

#endif