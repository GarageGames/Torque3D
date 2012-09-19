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
#include "console/console.h"
#include "console/compiler.h"
#include "console/consoleInternal.h"

using namespace Compiler;

/// @file
///
/// TorqueScript AST node allocators.
///
/// These static methods exist to allocate new AST node for the compiler. They
/// all allocate memory from the consoleAllocator for efficiency, and often take
/// arguments relating to the state of the nodes. They are called from gram.y
/// (really gram.c) as the lexer analyzes the script code.

//------------------------------------------------------------

BreakStmtNode *BreakStmtNode::alloc( S32 lineNumber )
{
   BreakStmtNode *ret = (BreakStmtNode *) consoleAlloc(sizeof(BreakStmtNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   return ret;
}

ContinueStmtNode *ContinueStmtNode::alloc( S32 lineNumber )
{
   ContinueStmtNode *ret = (ContinueStmtNode *) consoleAlloc(sizeof(ContinueStmtNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   return ret;
}

ReturnStmtNode *ReturnStmtNode::alloc( S32 lineNumber, ExprNode *expr)
{
   ReturnStmtNode *ret = (ReturnStmtNode *) consoleAlloc(sizeof(ReturnStmtNode));
   constructInPlace(ret);
   ret->expr = expr;
   ret->dbgLineNumber = lineNumber;

   return ret;
}

IfStmtNode *IfStmtNode::alloc( S32 lineNumber, ExprNode *testExpr, StmtNode *ifBlock, StmtNode *elseBlock, bool propagate )
{
   IfStmtNode *ret = (IfStmtNode *) consoleAlloc(sizeof(IfStmtNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;

   ret->testExpr = testExpr;
   ret->ifBlock = ifBlock;
   ret->elseBlock = elseBlock;
   ret->propagate = propagate;

   return ret;
}

LoopStmtNode *LoopStmtNode::alloc( S32 lineNumber, ExprNode *initExpr, ExprNode *testExpr, ExprNode *endLoopExpr, StmtNode *loopBlock, bool isDoLoop )
{
   LoopStmtNode *ret = (LoopStmtNode *) consoleAlloc(sizeof(LoopStmtNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->testExpr = testExpr;
   ret->initExpr = initExpr;
   ret->endLoopExpr = endLoopExpr;
   ret->loopBlock = loopBlock;
   ret->isDoLoop = isDoLoop;

   // Deal with setting some dummy constant nodes if we weren't provided with
   // info... This allows us to play nice with missing parts of for(;;) for
   // instance.
   if(!ret->testExpr) ret->testExpr = IntNode::alloc( lineNumber, 1 );

   return ret;
}

IterStmtNode* IterStmtNode::alloc( S32 lineNumber, StringTableEntry varName, ExprNode* containerExpr, StmtNode* body, bool isStringIter )
{
   IterStmtNode* ret = ( IterStmtNode* ) consoleAlloc( sizeof( IterStmtNode ) );
   constructInPlace( ret );
   
   ret->dbgLineNumber = lineNumber;
   ret->varName = varName;
   ret->containerExpr = containerExpr;
   ret->body = body;
   ret->isStringIter = isStringIter;
   
   return ret;
}

FloatBinaryExprNode *FloatBinaryExprNode::alloc( S32 lineNumber, S32 op, ExprNode *left, ExprNode *right )
{
   FloatBinaryExprNode *ret = (FloatBinaryExprNode *) consoleAlloc(sizeof(FloatBinaryExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;

   ret->op = op;
   ret->left = left;
   ret->right = right;

   return ret;
}

IntBinaryExprNode *IntBinaryExprNode::alloc( S32 lineNumber, S32 op, ExprNode *left, ExprNode *right )
{
   IntBinaryExprNode *ret = (IntBinaryExprNode *) consoleAlloc(sizeof(IntBinaryExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;

   ret->op = op;
   ret->left = left;
   ret->right = right;

   return ret;
}

StreqExprNode *StreqExprNode::alloc( S32 lineNumber, ExprNode *left, ExprNode *right, bool eq )
{
   StreqExprNode *ret = (StreqExprNode *) consoleAlloc(sizeof(StreqExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->left = left;
   ret->right = right;
   ret->eq = eq;

   return ret;
}

StrcatExprNode *StrcatExprNode::alloc( S32 lineNumber, ExprNode *left, ExprNode *right, int appendChar )
{
   StrcatExprNode *ret = (StrcatExprNode *) consoleAlloc(sizeof(StrcatExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->left = left;
   ret->right = right;
   ret->appendChar = appendChar;

   return ret;
}

CommaCatExprNode *CommaCatExprNode::alloc( S32 lineNumber, ExprNode *left, ExprNode *right )
{
   CommaCatExprNode *ret = (CommaCatExprNode *) consoleAlloc(sizeof(CommaCatExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->left = left;
   ret->right = right;

   return ret;
}

IntUnaryExprNode *IntUnaryExprNode::alloc( S32 lineNumber, S32 op, ExprNode *expr )
{
   IntUnaryExprNode *ret = (IntUnaryExprNode *) consoleAlloc(sizeof(IntUnaryExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->op = op;
   ret->expr = expr;
   return ret;
}

FloatUnaryExprNode *FloatUnaryExprNode::alloc( S32 lineNumber, S32 op, ExprNode *expr )
{
   FloatUnaryExprNode *ret = (FloatUnaryExprNode *) consoleAlloc(sizeof(FloatUnaryExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->op = op;
   ret->expr = expr;
   return ret;
}

VarNode *VarNode::alloc( S32 lineNumber, StringTableEntry varName, ExprNode *arrayIndex )
{
   VarNode *ret = (VarNode *) consoleAlloc(sizeof(VarNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->varName = varName;
   ret->arrayIndex = arrayIndex;
   return ret;
}

IntNode *IntNode::alloc( S32 lineNumber, S32 value )
{
   IntNode *ret = (IntNode *) consoleAlloc(sizeof(IntNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->value = value;
   return ret;
}

ConditionalExprNode *ConditionalExprNode::alloc( S32 lineNumber, ExprNode *testExpr, ExprNode *trueExpr, ExprNode *falseExpr )
{
   ConditionalExprNode *ret = (ConditionalExprNode *) consoleAlloc(sizeof(ConditionalExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->testExpr = testExpr;
   ret->trueExpr = trueExpr;
   ret->falseExpr = falseExpr;
   ret->integer = false;
   return ret;
}

FloatNode *FloatNode::alloc( S32 lineNumber, F64 value )
{
   FloatNode *ret = (FloatNode *) consoleAlloc(sizeof(FloatNode));
   constructInPlace(ret);
   
   ret->dbgLineNumber = lineNumber;
   ret->value = value;
   return ret;
}

StrConstNode *StrConstNode::alloc( S32 lineNumber, char *str, bool tag, bool doc )
{
   StrConstNode *ret = (StrConstNode *) consoleAlloc(sizeof(StrConstNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->str = (char *) consoleAlloc(dStrlen(str) + 1);
   ret->tag = tag;
   ret->doc = doc;
   dStrcpy(ret->str, str);

   return ret;
}

ConstantNode *ConstantNode::alloc( S32 lineNumber, StringTableEntry value )
{
   ConstantNode *ret = (ConstantNode *) consoleAlloc(sizeof(ConstantNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->value = value;
   return ret;
}

AssignExprNode *AssignExprNode::alloc( S32 lineNumber, StringTableEntry varName, ExprNode *arrayIndex, ExprNode *expr )
{
   AssignExprNode *ret = (AssignExprNode *) consoleAlloc(sizeof(AssignExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->varName = varName;
   ret->expr = expr;
   ret->arrayIndex = arrayIndex;

   return ret;
}

AssignOpExprNode *AssignOpExprNode::alloc( S32 lineNumber, StringTableEntry varName, ExprNode *arrayIndex, ExprNode *expr, S32 op )
{
   AssignOpExprNode *ret = (AssignOpExprNode *) consoleAlloc(sizeof(AssignOpExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->varName = varName;
   ret->expr = expr;
   ret->arrayIndex = arrayIndex;
   ret->op = op;
   return ret;
}

TTagSetStmtNode *TTagSetStmtNode::alloc( S32 lineNumber, StringTableEntry tag, ExprNode *valueExpr, ExprNode *stringExpr )
{
   TTagSetStmtNode *ret = (TTagSetStmtNode *) consoleAlloc(sizeof(TTagSetStmtNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->tag = tag;
   ret->valueExpr = valueExpr;
   ret->stringExpr = stringExpr;
   return ret;
}

TTagDerefNode *TTagDerefNode::alloc( S32 lineNumber, ExprNode *expr )
{
   TTagDerefNode *ret = (TTagDerefNode *) consoleAlloc(sizeof(TTagDerefNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->expr = expr;
   return ret;
}

TTagExprNode *TTagExprNode::alloc( S32 lineNumber, StringTableEntry tag )
{
   TTagExprNode *ret = (TTagExprNode *) consoleAlloc(sizeof(TTagExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->tag = tag;
   return ret;
}

FuncCallExprNode *FuncCallExprNode::alloc( S32 lineNumber, StringTableEntry funcName, StringTableEntry nameSpace, ExprNode *args, bool dot )
{
   FuncCallExprNode *ret = (FuncCallExprNode *) consoleAlloc(sizeof(FuncCallExprNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->funcName = funcName;
   ret->nameSpace = nameSpace;
   ret->args = args;
   if(dot)
      ret->callType = MethodCall;
   else
   {
      if(nameSpace && !dStricmp(nameSpace, "Parent"))
         ret->callType = ParentCall;
      else
         ret->callType = FunctionCall;
   }
   return ret;
}

AssertCallExprNode *AssertCallExprNode::alloc( S32 lineNumber,  ExprNode *testExpr, const char *message )
{
   #ifdef TORQUE_ENABLE_SCRIPTASSERTS      

      AssertCallExprNode *ret = (AssertCallExprNode *) consoleAlloc(sizeof(FuncCallExprNode));
      constructInPlace(ret);
      ret->dbgLineNumber = lineNumber;
      ret->testExpr = testExpr;
      ret->message = message ? message : "TorqueScript assert!";   
      return ret;
   
   #else

      return NULL;

   #endif
}

SlotAccessNode *SlotAccessNode::alloc( S32 lineNumber, ExprNode *objectExpr, ExprNode *arrayExpr, StringTableEntry slotName )
{
   SlotAccessNode *ret = (SlotAccessNode *) consoleAlloc(sizeof(SlotAccessNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->objectExpr = objectExpr;
   ret->arrayExpr = arrayExpr;
   ret->slotName = slotName;
   return ret;
}

InternalSlotAccessNode *InternalSlotAccessNode::alloc( S32 lineNumber, ExprNode *objectExpr, ExprNode *slotExpr, bool recurse )
{
   InternalSlotAccessNode *ret = (InternalSlotAccessNode *) consoleAlloc(sizeof(InternalSlotAccessNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->objectExpr = objectExpr;
   ret->slotExpr = slotExpr;
   ret->recurse = recurse;
   return ret;
}

SlotAssignNode *SlotAssignNode::alloc( S32 lineNumber, ExprNode *objectExpr, ExprNode *arrayExpr, StringTableEntry slotName, ExprNode *valueExpr, U32 typeID /* = -1 */ )
{
   SlotAssignNode *ret = (SlotAssignNode *) consoleAlloc(sizeof(SlotAssignNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->objectExpr = objectExpr;
   ret->arrayExpr = arrayExpr;
   ret->slotName = slotName;
   ret->valueExpr = valueExpr;
   ret->typeID = typeID;
   return ret;
}

SlotAssignOpNode *SlotAssignOpNode::alloc( S32 lineNumber, ExprNode *objectExpr, StringTableEntry slotName, ExprNode *arrayExpr, S32 op, ExprNode *valueExpr )
{
   SlotAssignOpNode *ret = (SlotAssignOpNode *) consoleAlloc(sizeof(SlotAssignOpNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->objectExpr = objectExpr;
   ret->arrayExpr = arrayExpr;
   ret->slotName = slotName;
   ret->op = op;
   ret->valueExpr = valueExpr;
   return ret;
}

ObjectDeclNode *ObjectDeclNode::alloc( S32 lineNumber, ExprNode *classNameExpr, ExprNode *objectNameExpr, ExprNode *argList, StringTableEntry parentObject, SlotAssignNode *slotDecls, ObjectDeclNode *subObjects, bool isDatablock, bool classNameInternal, bool isSingleton )
{
   ObjectDeclNode *ret = (ObjectDeclNode *) consoleAlloc(sizeof(ObjectDeclNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->classNameExpr = classNameExpr;
   ret->objectNameExpr = objectNameExpr;
   ret->argList = argList;
   ret->slotDecls = slotDecls;
   ret->subObjects = subObjects;
   ret->isDatablock = isDatablock;
   ret->isClassNameInternal = classNameInternal;
   ret->isSingleton = isSingleton;
   ret->failOffset = 0;
   if(parentObject)
      ret->parentObject = parentObject;
   else
      ret->parentObject = StringTable->insert("");
   return ret;
}

FunctionDeclStmtNode *FunctionDeclStmtNode::alloc( S32 lineNumber, StringTableEntry fnName, StringTableEntry nameSpace, VarNode *args, StmtNode *stmts )
{
   FunctionDeclStmtNode *ret = (FunctionDeclStmtNode *) consoleAlloc(sizeof(FunctionDeclStmtNode));
   constructInPlace(ret);
   ret->dbgLineNumber = lineNumber;
   ret->fnName = fnName;
   ret->args = args;
   ret->stmts = stmts;
   ret->nameSpace = nameSpace;
   ret->package = NULL;
   return ret;
}
