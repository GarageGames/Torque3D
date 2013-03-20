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
#include "console/telnetDebugger.h"

#include "console/ast.h"
#include "core/tAlgorithm.h"

#include "core/strings/findMatch.h"
#include "console/consoleInternal.h"
#include "core/stream/fileStream.h"
#include "console/compiler.h"

#include "console/simBase.h"

template< typename T >
struct Token
{
   T value;
   S32 lineNumber;
};
#include "console/cmdgram.h"


namespace Compiler
{
   U32 precompileBlock(StmtNode *block, U32 loopCount)
   {
      U32 sum = 0;
      for(StmtNode *walk = block; walk; walk = walk->getNext())
      {
         const U32 temp = walk->precompileStmt(loopCount);
#ifdef DEBUG_AST_NODES
         if(temp > 1000)
            Con::printf("suspect %s '%s' line %d", walk->dbgStmtType().c_str(), walk->dbgFileName, walk->dbgLineNumber);
#endif
         sum += temp;
      }
      return sum;
   }

   U32 compileBlock(StmtNode *block, U32 *codeStream, U32 ip, U32 continuePoint, U32 breakPoint)
   {
      for(StmtNode *walk = block; walk; walk = walk->getNext())
         ip = walk->compileStmt(codeStream, ip, continuePoint, breakPoint);
      return ip;
   }
}

using namespace Compiler;

//-----------------------------------------------------------------------------

void StmtNode::addBreakCount()
{
   CodeBlock::smBreakLineCount++;
}

void StmtNode::addBreakLine(U32 ip)
{
   U32 line = CodeBlock::smBreakLineCount * 2;
   CodeBlock::smBreakLineCount++;

   if(getBreakCodeBlock()->lineBreakPairs)
   {
      getBreakCodeBlock()->lineBreakPairs[line] = dbgLineNumber;
      getBreakCodeBlock()->lineBreakPairs[line+1] = ip;
   }
}

//------------------------------------------------------------

StmtNode::StmtNode()
{
   next = NULL;
   dbgFileName = CodeBlock::smCurrentParser->getCurrentFile();
}

void StmtNode::setPackage(StringTableEntry)
{
}

void StmtNode::append(StmtNode *next)
{
   StmtNode *walk = this;
   while(walk->next)
      walk = walk->next;
   walk->next = next;
}


void FunctionDeclStmtNode::setPackage(StringTableEntry packageName)
{
   package = packageName;
}

//------------------------------------------------------------
//
// Console language compilers
//
//------------------------------------------------------------

static U32 conversionOp(TypeReq src, TypeReq dst)
{
   if(src == TypeReqString)
   {
      switch(dst)
      {
      case TypeReqUInt:
         return OP_STR_TO_UINT;
      case TypeReqFloat:
         return OP_STR_TO_FLT;
      case TypeReqNone:
         return OP_STR_TO_NONE;
      default:
         break;
      }
   }
   else if(src == TypeReqFloat)
   {
      switch(dst)
      {
      case TypeReqUInt:
         return OP_FLT_TO_UINT;
      case TypeReqString:
         return OP_FLT_TO_STR;
      case TypeReqNone:
         return OP_FLT_TO_NONE;
      default:
         break;
      }
   }
   else if(src == TypeReqUInt)
   {
      switch(dst)
      {
      case TypeReqFloat:
         return OP_UINT_TO_FLT;
      case TypeReqString:
         return OP_UINT_TO_STR;
      case TypeReqNone:
         return OP_UINT_TO_NONE;
      default:
         break;
      }
   }
   return OP_INVALID;
}

//------------------------------------------------------------

U32 BreakStmtNode::precompileStmt(U32 loopCount)
{
   if(loopCount)
   {
      addBreakCount();
      return 2;
   }
   Con::warnf(ConsoleLogEntry::General, "%s (%d): break outside of loop... ignoring.", dbgFileName, dbgLineNumber);
   return 0;
}

U32 BreakStmtNode::compileStmt(U32 *codeStream, U32 ip, U32, U32 breakPoint)
{
   if(breakPoint)
   {
      addBreakLine(ip);
      codeStream[ip++] = OP_JMP;
      codeStream[ip++] = breakPoint;
   }
   return ip;
}

//------------------------------------------------------------

U32 ContinueStmtNode::precompileStmt(U32 loopCount)
{
   if(loopCount)
   {
      addBreakCount();
      return 2;
   }
   Con::warnf(ConsoleLogEntry::General, "%s (%d): continue outside of loop... ignoring.", dbgFileName, dbgLineNumber);
   return 0;
}

U32 ContinueStmtNode::compileStmt(U32 *codeStream, U32 ip, U32 continuePoint, U32)
{
   if(continuePoint)
   {
      addBreakLine(ip);
      codeStream[ip++] = OP_JMP;
      codeStream[ip++] = continuePoint;
   }
   return ip;
}

//------------------------------------------------------------

U32 ExprNode::precompileStmt(U32)
{
   addBreakCount();
   return precompile(TypeReqNone);
}

U32 ExprNode::compileStmt(U32 *codeStream, U32 ip, U32, U32)
{
   addBreakLine(ip);
   return compile(codeStream, ip, TypeReqNone);
}

//------------------------------------------------------------

U32 ReturnStmtNode::precompileStmt(U32)
{
   addBreakCount();
   return 1 + (expr ? expr->precompile(TypeReqString) : 0);
}

U32 ReturnStmtNode::compileStmt(U32 *codeStream, U32 ip, U32, U32)
{
   addBreakLine(ip);
   if(!expr)
      codeStream[ip++] = OP_RETURN_VOID;
   else
   {
      ip = expr->compile(codeStream, ip, TypeReqString);
      codeStream[ip++] = OP_RETURN;
   }
   return ip;
}

//------------------------------------------------------------

ExprNode *IfStmtNode::getSwitchOR(ExprNode *left, ExprNode *list, bool string)
{
   ExprNode *nextExpr = (ExprNode *) list->getNext();
   ExprNode *test;
   if(string)
      test = StreqExprNode::alloc( left->dbgLineNumber, left, list, true );
   else
      test = IntBinaryExprNode::alloc( left->dbgLineNumber, opEQ, left, list );
   if(!nextExpr)
      return test;
   return IntBinaryExprNode::alloc( test->dbgLineNumber, opOR, test, getSwitchOR( left, nextExpr, string ) );
}

void IfStmtNode::propagateSwitchExpr(ExprNode *left, bool string)
{
   testExpr = getSwitchOR(left, testExpr, string);
   if(propagate && elseBlock)
      ((IfStmtNode *) elseBlock)->propagateSwitchExpr(left, string);
}

U32 IfStmtNode::precompileStmt(U32 loopCount)
{
   U32 exprSize;
   addBreakCount();

   if(testExpr->getPreferredType() == TypeReqUInt)
   {
      exprSize = testExpr->precompile(TypeReqUInt);
      integer = true;
   }
   else
   {
      exprSize = testExpr->precompile(TypeReqFloat);
      integer = false;
   }
   // next is the JMPIFNOT or JMPIFFNOT - size of 2
   U32 ifSize = precompileBlock(ifBlock, loopCount);
   if(!elseBlock)
      endifOffset = ifSize + 2 + exprSize;
   else
   {
      elseOffset = exprSize + 2 + ifSize + 2;
      U32 elseSize = precompileBlock(elseBlock, loopCount);
      endifOffset = elseOffset + elseSize;
   }
   return endifOffset;
}

U32 IfStmtNode::compileStmt(U32 *codeStream, U32 ip, U32 continuePoint, U32 breakPoint)
{
   U32 start = ip;
   addBreakLine(ip);

   ip = testExpr->compile(codeStream, ip, integer ? TypeReqUInt : TypeReqFloat);
   codeStream[ip++] = integer ? OP_JMPIFNOT : OP_JMPIFFNOT;

   if(elseBlock)
   {
      codeStream[ip++] = start + elseOffset;
      ip = compileBlock(ifBlock, codeStream, ip, continuePoint, breakPoint);
      codeStream[ip++] = OP_JMP;
      codeStream[ip++] = start + endifOffset;
      ip = compileBlock(elseBlock, codeStream, ip, continuePoint, breakPoint);
   }
   else
   {
      codeStream[ip++] = start + endifOffset;
      ip = compileBlock(ifBlock, codeStream, ip, continuePoint, breakPoint);
   }
   return ip;
}

//------------------------------------------------------------

U32 LoopStmtNode::precompileStmt(U32 loopCount)
{
   U32 initSize = 0;
   addBreakCount();

   if(initExpr)
      initSize = initExpr->precompile(TypeReqNone);

   U32 testSize;

   if(testExpr->getPreferredType() == TypeReqUInt)
   {
      integer = true;
      testSize = testExpr->precompile(TypeReqUInt);
   }
   else
   {
      integer = false;
      testSize = testExpr->precompile(TypeReqFloat);
   }

   U32 blockSize = precompileBlock(loopBlock, loopCount + 1);

   U32 endLoopSize = 0;
   if(endLoopExpr)
      endLoopSize = endLoopExpr->precompile(TypeReqNone);

   // if it's a for loop or a while loop it goes:
   // initExpr
   // testExpr
   // OP_JMPIFNOT to break point
   // loopStartPoint:
   // loopBlock
   // continuePoint:
   // endLoopExpr
   // testExpr
   // OP_JMPIF loopStartPoint
   // breakPoint:

   // otherwise if it's a do ... while() it goes:
   // initExpr
   // loopStartPoint:
   // loopBlock
   // continuePoint:
   // endLoopExpr
   // testExpr
   // OP_JMPIF loopStartPoint
   // breakPoint:

   if(!isDoLoop)
   {
      loopBlockStartOffset = initSize + testSize + 2;
      continueOffset       = loopBlockStartOffset + blockSize;
      breakOffset          = continueOffset + endLoopSize + testSize + 2;
   }
   else
   {
      loopBlockStartOffset = initSize;
      continueOffset       = initSize + blockSize;
      breakOffset          = continueOffset + endLoopSize + testSize + 2;
   }
   return breakOffset;
}

U32 LoopStmtNode::compileStmt(U32 *codeStream, U32 ip, U32, U32)
{
   addBreakLine(ip);
   U32 start = ip;
   if(initExpr)
      ip = initExpr->compile(codeStream, ip, TypeReqNone);

   if(!isDoLoop)
   {
      ip = testExpr->compile(codeStream, ip, integer ? TypeReqUInt : TypeReqFloat);
      codeStream[ip++] = integer ? OP_JMPIFNOT : OP_JMPIFFNOT;
      codeStream[ip++] = start + breakOffset;
   }

   // Compile internals of loop.
   ip = compileBlock(loopBlock, codeStream, ip, start + continueOffset, start + breakOffset);

   if(endLoopExpr)
      ip = endLoopExpr->compile(codeStream, ip, TypeReqNone);

   ip = testExpr->compile(codeStream, ip, integer ? TypeReqUInt : TypeReqFloat);

   codeStream[ip++] = integer ? OP_JMPIF : OP_JMPIFF;
   codeStream[ip++] = start + loopBlockStartOffset;

   return ip;
}

//------------------------------------------------------------

U32 IterStmtNode::precompileStmt( U32 loopCount )
{
   addBreakCount();
   
   // Instruction sequence:
   //
   //   containerExpr
   //   OP_ITER_BEGIN varName .fail
   // .continue:
   //   OP_ITER .break
   //   body
   //   OP_JMP .continue
   // .break:
   //   OP_ITER_END
   // .fail:

   U32 exprSize = containerExpr->precompile( TypeReqString );
   bodySize = precompileBlock( body, loopCount + 1 );
   
   return 
        exprSize
      + 3 // OP_ITER_BEGIN
      + 2 // OP_ITER
      + bodySize
      + 2 // OP_JMP
      + 1 // OP_ITER_END
   ;
}

U32 IterStmtNode::compileStmt( U32* codeStream, U32 ip, U32 continuePoint, U32 breakPoint )
{
   addBreakLine( ip );
   
   const U32 startIp = ip;
   const U32 iterBeginIp = containerExpr->compile( codeStream, startIp, TypeReqString );
      
   const U32 continueIp = iterBeginIp + 3;
   const U32 bodyIp = continueIp + 2;
   const U32 jmpIp = bodyIp + bodySize;
   const U32 breakIp = jmpIp + 2;
   const U32 finalIp = breakIp + 1;
   
   codeStream[ iterBeginIp ] = isStringIter ? OP_ITER_BEGIN_STR : OP_ITER_BEGIN;
   codeStream[ iterBeginIp + 1 ] = STEtoU32( varName, iterBeginIp + 1 );
   codeStream[ iterBeginIp + 2 ] = finalIp;
   codeStream[ continueIp ] = OP_ITER;
   codeStream[ continueIp + 1 ] = breakIp;
      
   compileBlock( body, codeStream, bodyIp, continueIp, breakIp );
   
   codeStream[ jmpIp ] = OP_JMP;
   codeStream[ jmpIp + 1 ] = continueIp;
   codeStream[ breakIp ] = OP_ITER_END;
   
   return finalIp;
}

//------------------------------------------------------------

U32 ConditionalExprNode::precompile(TypeReq type)
{
   // code is testExpr
   // JMPIFNOT falseStart
   // trueExpr
   // JMP end
   // falseExpr
   U32 exprSize;

   if(testExpr->getPreferredType() == TypeReqUInt)
   {
      exprSize = testExpr->precompile(TypeReqUInt);
      integer = true;
   }
   else
   {
      exprSize = testExpr->precompile(TypeReqFloat);
      integer = false;
   }
   return exprSize + 
      trueExpr->precompile(type) +
      falseExpr->precompile(type) + 4;
}

U32 ConditionalExprNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   ip = testExpr->compile(codeStream, ip, integer ? TypeReqUInt : TypeReqFloat);
   codeStream[ip++] = integer ? OP_JMPIFNOT : OP_JMPIFFNOT;
   U32 jumpElseIp = ip++;
   ip = trueExpr->compile(codeStream, ip, type);
   codeStream[ip++] = OP_JMP;
   U32 jumpEndIp = ip++;
   codeStream[jumpElseIp] = ip;
   ip = falseExpr->compile(codeStream, ip, type);
   codeStream[jumpEndIp] = ip;
   return ip;
}

TypeReq ConditionalExprNode::getPreferredType()
{
   return trueExpr->getPreferredType();
}

//------------------------------------------------------------

U32 FloatBinaryExprNode::precompile(TypeReq type)
{
   U32 addSize = left->precompile(TypeReqFloat) + right->precompile(TypeReqFloat) + 1;
   if(type != TypeReqFloat)
      addSize++;

   return addSize;
}

U32 FloatBinaryExprNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   ip = right->compile(codeStream, ip, TypeReqFloat);
   ip = left->compile(codeStream, ip, TypeReqFloat);
   U32 operand = OP_INVALID;
   switch(op)
   {
   case '+':
      operand = OP_ADD;
      break;
   case '-':
      operand = OP_SUB;
      break;
   case '/':
      operand = OP_DIV;
      break;
   case '*':
      operand = OP_MUL;
      break;
   }
   codeStream[ip++] = operand;
   if(type != TypeReqFloat)
      codeStream[ip++] =conversionOp(TypeReqFloat, type);
   return ip;
}

TypeReq FloatBinaryExprNode::getPreferredType()
{
   return TypeReqFloat;
}

//------------------------------------------------------------

void IntBinaryExprNode::getSubTypeOperand()
{
   subType = TypeReqUInt;
   switch(op)
   {
   case '^':
      operand = OP_XOR;
      break;
   case '%':
      operand = OP_MOD;
      break;
   case '&':
      operand = OP_BITAND;
      break;
   case '|':
      operand = OP_BITOR;
      break;
   case '<':
      operand = OP_CMPLT;
      subType = TypeReqFloat;
      break;
   case '>':
      operand = OP_CMPGR;
      subType = TypeReqFloat;
      break;
   case opGE:
      operand = OP_CMPGE;
      subType = TypeReqFloat;
      break;
   case opLE:
      operand = OP_CMPLE;
      subType = TypeReqFloat;
      break;
   case opEQ:
      operand = OP_CMPEQ;
      subType = TypeReqFloat;
      break;
   case opNE:
      operand = OP_CMPNE;
      subType = TypeReqFloat;
      break;
   case opOR:
      operand = OP_OR;
      break;
   case opAND:
      operand = OP_AND;
      break;
   case opSHR:
      operand = OP_SHR;
      break;
   case opSHL:
      operand = OP_SHL;
      break;
   }
}

U32 IntBinaryExprNode::precompile(TypeReq type)
{
   getSubTypeOperand();
   U32 addSize = left->precompile(subType) + right->precompile(subType) + 1;
   if(operand == OP_OR || operand == OP_AND)
      addSize++;

   if(type != TypeReqUInt)
      addSize++;

   return addSize;
}

U32 IntBinaryExprNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   if(operand == OP_OR || operand == OP_AND)
   {
      ip = left->compile(codeStream, ip, subType);
      codeStream[ip++] = operand == OP_OR ? OP_JMPIF_NP : OP_JMPIFNOT_NP;
      U32 jmpIp = ip++;
      ip = right->compile(codeStream, ip, subType);
      codeStream[jmpIp] = ip;
   }
   else
   {
      ip = right->compile(codeStream, ip, subType);
      ip = left->compile(codeStream, ip, subType);
      codeStream[ip++] = operand;
   }
   if(type != TypeReqUInt)
      codeStream[ip++] =conversionOp(TypeReqUInt, type);
   return ip;
}

TypeReq IntBinaryExprNode::getPreferredType()
{
   return TypeReqUInt;
}

//------------------------------------------------------------

U32 StreqExprNode::precompile(TypeReq type)
{
   // eval str left
   // OP_ADVANCE_STR_NUL
   // eval str right
   // OP_COMPARE_STR
   // optional conversion
   U32 addSize = left->precompile(TypeReqString) + right->precompile(TypeReqString) + 2;
   if(!eq)
      addSize ++;
   if(type != TypeReqUInt)
      addSize ++;
   return addSize;
}

U32 StreqExprNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   ip = left->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_ADVANCE_STR_NUL;
   ip = right->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_COMPARE_STR;
   if(!eq)
      codeStream[ip++] = OP_NOT;
   if(type != TypeReqUInt)
      codeStream[ip++] = conversionOp(TypeReqUInt, type);
   return ip;
}

TypeReq StreqExprNode::getPreferredType()
{
   return TypeReqUInt;
}

//------------------------------------------------------------

U32 StrcatExprNode::precompile(TypeReq type)
{
   U32 addSize = left->precompile(TypeReqString) + right->precompile(TypeReqString) + 2;
   if(appendChar)
      addSize++;

   if(type != TypeReqString)
      addSize ++;
   return addSize;
}

U32 StrcatExprNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   ip = left->compile(codeStream, ip, TypeReqString);
   if(!appendChar)
      codeStream[ip++] = OP_ADVANCE_STR;
   else
   {
      codeStream[ip++] = OP_ADVANCE_STR_APPENDCHAR;
      codeStream[ip++] = appendChar;
   }
   ip = right->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_REWIND_STR;
   if(type == TypeReqUInt)
      codeStream[ip++] = OP_STR_TO_UINT;
   else if(type == TypeReqFloat)
      codeStream[ip++] = OP_STR_TO_FLT;
   return ip;
}

TypeReq StrcatExprNode::getPreferredType()
{
   return TypeReqString;
}

//------------------------------------------------------------

U32 CommaCatExprNode::precompile(TypeReq type)
{
   U32 addSize = left->precompile(TypeReqString) + right->precompile(TypeReqString) + 2;
   if(type != TypeReqString)
      addSize ++;
   return addSize;
}

U32 CommaCatExprNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   ip = left->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_ADVANCE_STR_COMMA;
   ip = right->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_REWIND_STR;

   // At this point the stack has the concatenated string.

   // But we're paranoid, so accept (but whine) if we get an oddity...
   if(type == TypeReqUInt || type == TypeReqFloat)
      Con::warnf(ConsoleLogEntry::General, "%s (%d): converting comma string to a number... probably wrong.", dbgFileName, dbgLineNumber);
   if(type == TypeReqUInt)
      codeStream[ip++] = OP_STR_TO_UINT;
   else if(type == TypeReqFloat)
      codeStream[ip++] = OP_STR_TO_FLT;
   return ip;
}

TypeReq CommaCatExprNode::getPreferredType()
{
   return TypeReqString;
}

//------------------------------------------------------------

U32 IntUnaryExprNode::precompile(TypeReq type)
{
   integer = true;
   TypeReq prefType = expr->getPreferredType();
   if(op == '!' && (prefType == TypeReqFloat || prefType == TypeReqString))
      integer = false;

   U32 exprSize = expr->precompile(integer ? TypeReqUInt : TypeReqFloat);
   if(type != TypeReqUInt)
      return exprSize + 2;
   else
      return exprSize + 1;
}

U32 IntUnaryExprNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   ip = expr->compile(codeStream, ip, integer ? TypeReqUInt : TypeReqFloat);
   if(op == '!')
      codeStream[ip++] = integer ? OP_NOT : OP_NOTF;
   else if(op == '~')
      codeStream[ip++] = OP_ONESCOMPLEMENT;
   if(type != TypeReqUInt)
      codeStream[ip++] =conversionOp(TypeReqUInt, type);
   return ip;
}

TypeReq IntUnaryExprNode::getPreferredType()
{
   return TypeReqUInt;
}

//------------------------------------------------------------

U32 FloatUnaryExprNode::precompile(TypeReq type)
{
   U32 exprSize = expr->precompile(TypeReqFloat);
   if(type != TypeReqFloat)
      return exprSize + 2;
   else
      return exprSize + 1;
}

U32 FloatUnaryExprNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   ip = expr->compile(codeStream, ip, TypeReqFloat);
   codeStream[ip++] = OP_NEG;
   if(type != TypeReqFloat)
      codeStream[ip++] =conversionOp(TypeReqFloat, type);
   return ip;
}

TypeReq FloatUnaryExprNode::getPreferredType()
{
   return TypeReqFloat;
}

//------------------------------------------------------------

U32 VarNode::precompile(TypeReq type)
{
   // if this has an arrayIndex...
   // OP_LOADIMMED_IDENT
   // varName
   // OP_ADVANCE_STR
   // evaluate arrayIndex TypeReqString
   // OP_REWIND_STR
   // OP_SETCURVAR_ARRAY
   // OP_LOADVAR (type)

   // else
   // OP_SETCURVAR
   // varName
   // OP_LOADVAR (type)
   if(type == TypeReqNone)
      return 0;

   precompileIdent(varName);
   return (arrayIndex ? arrayIndex->precompile(TypeReqString) + 6 : 3);
}

U32 VarNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   if(type == TypeReqNone)
      return ip;

   codeStream[ip++] = arrayIndex ? OP_LOADIMMED_IDENT : OP_SETCURVAR;
   codeStream[ip] = STEtoU32(varName, ip);
   ip++;
   if(arrayIndex)
   {
      codeStream[ip++] = OP_ADVANCE_STR;
      ip = arrayIndex->compile(codeStream, ip, TypeReqString);
      codeStream[ip++] = OP_REWIND_STR;
      codeStream[ip++] = OP_SETCURVAR_ARRAY;
   }
   switch(type)
   {
   case TypeReqUInt:
      codeStream[ip++] = OP_LOADVAR_UINT;
      break;
   case TypeReqFloat:
      codeStream[ip++] = OP_LOADVAR_FLT;
      break;
   case TypeReqString:
      codeStream[ip++] = OP_LOADVAR_STR;
      break;
   case TypeReqNone:
      break;
   }
   return ip;
}

TypeReq VarNode::getPreferredType()
{
   return TypeReqNone; // no preferred type
}

//------------------------------------------------------------

U32 IntNode::precompile(TypeReq type)
{
   if(type == TypeReqNone)
      return 0;
   if(type == TypeReqString)
      index = getCurrentStringTable()->addIntString(value);
   else if(type == TypeReqFloat)
      index = getCurrentFloatTable()->add(value);
   return 2;
}

U32 IntNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   switch(type)
   {
   case TypeReqUInt:
      codeStream[ip++] = OP_LOADIMMED_UINT;
      codeStream[ip++] = value;
      break;
   case TypeReqString:
      codeStream[ip++] = OP_LOADIMMED_STR;
      codeStream[ip++] = index;
      break;
   case TypeReqFloat:
      codeStream[ip++] = OP_LOADIMMED_FLT;
      codeStream[ip++] = index;
      break;
   case TypeReqNone:
      break;
   }
   return ip;
}

TypeReq IntNode::getPreferredType()
{
   return TypeReqUInt;
}

//------------------------------------------------------------

U32 FloatNode::precompile(TypeReq type)
{
   if(type == TypeReqNone)
      return 0;
   if(type == TypeReqString)
      index = getCurrentStringTable()->addFloatString(value);
   else if(type == TypeReqFloat)
      index = getCurrentFloatTable()->add(value);
   return 2;
}

U32 FloatNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   switch(type)
   {
   case TypeReqUInt:
      codeStream[ip++] = OP_LOADIMMED_UINT;
      codeStream[ip++] = U32(value);
      break;
   case TypeReqString:
      codeStream[ip++] = OP_LOADIMMED_STR;
      codeStream[ip++] = index;
      break;
   case TypeReqFloat:
      codeStream[ip++] = OP_LOADIMMED_FLT;
      codeStream[ip++] = index;
      break;
   case TypeReqNone:
      break;
   }
   return ip;
}

TypeReq FloatNode::getPreferredType()
{
   return TypeReqFloat;
}

//------------------------------------------------------------

U32 StrConstNode::precompile(TypeReq type)
{
   // Early out for documentation block.
   if( doc )
   {
      index = getCurrentStringTable()->add(str, true, tag);
      return 2;
   }

   if(type == TypeReqString)
   {
      index = getCurrentStringTable()->add(str, true, tag);
      return 2;
   }
   else if(type == TypeReqNone)
   {
      return 0;
   }

   fVal = consoleStringToNumber(str, dbgFileName, dbgLineNumber);
   if(type == TypeReqFloat)
      index = getCurrentFloatTable()->add(fVal);
   return 2;
}

U32 StrConstNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   // If this is a DOCBLOCK, then process w/ appropriate op...
   if( doc )
   {
      codeStream[ip++] = OP_DOCBLOCK_STR;
      codeStream[ip++] = index;
      return ip;
   }

   // Otherwise, deal with it normally as a string literal case.
   switch(type)
   {
   case TypeReqString:
      codeStream[ip++] = tag ? OP_TAG_TO_STR : OP_LOADIMMED_STR;
      codeStream[ip++] = index;
      break;
   case TypeReqUInt:
      codeStream[ip++] = OP_LOADIMMED_UINT;
      codeStream[ip++] = U32(fVal);
      break;
   case TypeReqFloat:
      codeStream[ip++] = OP_LOADIMMED_FLT;
      codeStream[ip++] = index;
      break;         
   case TypeReqNone:
      break;
   }
   return ip;
}

TypeReq StrConstNode::getPreferredType()
{
   return TypeReqString;
}

//------------------------------------------------------------

U32 ConstantNode::precompile(TypeReq type)
{
   if(type == TypeReqString)
   {
      precompileIdent(value);
      return 2;
   }
   else if(type == TypeReqNone)
      return 0;

   fVal = consoleStringToNumber(value, dbgFileName, dbgLineNumber);
   if(type == TypeReqFloat)
      index = getCurrentFloatTable()->add(fVal);
   return 2;
}

U32 ConstantNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   switch(type)
   {
   case TypeReqString:
      codeStream[ip++] = OP_LOADIMMED_IDENT;
      codeStream[ip] = STEtoU32(value, ip);
      ip++;
      break;
   case TypeReqUInt:
      codeStream[ip++] = OP_LOADIMMED_UINT;
      codeStream[ip++] = U32(fVal);
      break;
   case TypeReqFloat:
      codeStream[ip++] = OP_LOADIMMED_FLT;
      codeStream[ip++] = index;
      break;
   case TypeReqNone:
      break;
   }
   return ip;
}

TypeReq ConstantNode::getPreferredType()
{
   return TypeReqString;
}

//------------------------------------------------------------

U32 AssignExprNode::precompile(TypeReq type)
{
   subType = expr->getPreferredType();
   if(subType == TypeReqNone)
      subType = type;
   if(subType == TypeReqNone)
      subType = TypeReqString;
   // if it's an array expr, the formula is:
   // eval expr
   // (push and pop if it's TypeReqString) OP_ADVANCE_STR
   // OP_LOADIMMED_IDENT
   // varName
   // OP_ADVANCE_STR
   // eval array
   // OP_REWIND_STR
   // OP_SETCURVAR_ARRAY_CREATE
   // OP_TERMINATE_REWIND_STR
   // OP_SAVEVAR

   //else
   // eval expr
   // OP_SETCURVAR_CREATE
   // varname
   // OP_SAVEVAR
   const U32 addSize = (type != subType ? 1 : 0);
   const U32 retSize = expr->precompile(subType);

#ifdef DEBUG_AST_NODES
   if(retSize > 1000)
      Con::printf("Bad expr %s", expr->dbgStmtType().c_str());
#endif

   precompileIdent(varName);

   return retSize + addSize + (arrayIndex ? arrayIndex->precompile(TypeReqString) + (subType == TypeReqString ? 8 : 6 ) : 3);
}

U32 AssignExprNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   ip = expr->compile(codeStream, ip, subType);
   if(arrayIndex)
   {
      if(subType == TypeReqString)
         codeStream[ip++] = OP_ADVANCE_STR;

      codeStream[ip++] = OP_LOADIMMED_IDENT;
      codeStream[ip] = STEtoU32(varName, ip);
      ip++;
      codeStream[ip++] = OP_ADVANCE_STR;
      ip = arrayIndex->compile(codeStream, ip, TypeReqString);
      codeStream[ip++] = OP_REWIND_STR;
      codeStream[ip++] = OP_SETCURVAR_ARRAY_CREATE;
      if(subType == TypeReqString)
         codeStream[ip++] = OP_TERMINATE_REWIND_STR;
   }
   else
   {
      codeStream[ip++] = OP_SETCURVAR_CREATE;
      codeStream[ip] = STEtoU32(varName, ip);
      ip++;
   }
   switch(subType)
   {
   case TypeReqString:
      codeStream[ip++] = OP_SAVEVAR_STR;
      break;
   case TypeReqUInt:
      codeStream[ip++] = OP_SAVEVAR_UINT;
      break;
   case TypeReqFloat:
      codeStream[ip++] = OP_SAVEVAR_FLT;
      break;
   case TypeReqNone:
      break;
   }
   if(type != subType)
      codeStream[ip++] = conversionOp(subType, type);
   return ip;
}

TypeReq AssignExprNode::getPreferredType()
{
   return expr->getPreferredType();
}

//------------------------------------------------------------

static void getAssignOpTypeOp(S32 op, TypeReq &type, U32 &operand)
{
   switch(op)
   {
   case '+':
      type = TypeReqFloat;
      operand = OP_ADD;
      break;
   case '-':
      type = TypeReqFloat;
      operand = OP_SUB;
      break;
   case '*':
      type = TypeReqFloat;
      operand = OP_MUL;
      break;
   case '/':
      type = TypeReqFloat;
      operand = OP_DIV;
      break;
   case '%':
      type = TypeReqUInt;
      operand = OP_MOD;
      break;
   case '&':
      type = TypeReqUInt;
      operand = OP_BITAND;
      break;
   case '^':
      type = TypeReqUInt;
      operand = OP_XOR;
      break;
   case '|':
      type = TypeReqUInt;
      operand = OP_BITOR;
      break;
   case opSHL:
      type = TypeReqUInt;
      operand = OP_SHL;
      break;
   case opSHR:
      type = TypeReqUInt;
      operand = OP_SHR;
      break;
   }   
}

U32 AssignOpExprNode::precompile(TypeReq type)
{
   // goes like this...
   // eval expr as float or int
   // if there's an arrayIndex

   // OP_LOADIMMED_IDENT
   // varName
   // OP_ADVANCE_STR
   // eval arrayIndex stringwise
   // OP_REWIND_STR
   // OP_SETCURVAR_ARRAY_CREATE

   // else
   // OP_SETCURVAR_CREATE
   // varName

   // OP_LOADVAR_FLT or UINT
   // operand
   // OP_SAVEVAR_FLT or UINT

   // conversion OP if necessary.
   getAssignOpTypeOp(op, subType, operand);
   precompileIdent(varName);
   U32 size = expr->precompile(subType);
   if(type != subType)
      size++;
   if(!arrayIndex)
      return size + 5;
   else
   {
      size += arrayIndex->precompile(TypeReqString);
      return size + 8;
   }
}

U32 AssignOpExprNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   ip = expr->compile(codeStream, ip, subType);
   if(!arrayIndex)
   {
      codeStream[ip++] = OP_SETCURVAR_CREATE;
      codeStream[ip] = STEtoU32(varName, ip);
      ip++;
   }
   else
   {
      codeStream[ip++] = OP_LOADIMMED_IDENT;
      codeStream[ip] = STEtoU32(varName, ip);
      ip++;
      codeStream[ip++] = OP_ADVANCE_STR;
      ip = arrayIndex->compile(codeStream, ip, TypeReqString);
      codeStream[ip++] = OP_REWIND_STR;
      codeStream[ip++] = OP_SETCURVAR_ARRAY_CREATE;
   }
   codeStream[ip++] = (subType == TypeReqFloat) ? OP_LOADVAR_FLT : OP_LOADVAR_UINT;
   codeStream[ip++] = operand;
   codeStream[ip++] = (subType == TypeReqFloat) ? OP_SAVEVAR_FLT : OP_SAVEVAR_UINT;
   if(subType != type)
      codeStream[ip++] = conversionOp(subType, type);
   return ip;
}

TypeReq AssignOpExprNode::getPreferredType()
{
   getAssignOpTypeOp(op, subType, operand);
   return subType;
}

//------------------------------------------------------------

U32 TTagSetStmtNode::precompileStmt(U32 loopCount)
{
   TORQUE_UNUSED(loopCount);
   return 0;
}

U32 TTagSetStmtNode::compileStmt(U32*, U32 ip, U32, U32)
{
   return ip;
}

//------------------------------------------------------------

U32 TTagDerefNode::precompile(TypeReq)
{
   return 0;
}

U32 TTagDerefNode::compile(U32*, U32 ip, TypeReq)
{
   return ip;
}

TypeReq TTagDerefNode::getPreferredType()
{
   return TypeReqNone;
}

//------------------------------------------------------------

U32 TTagExprNode::precompile(TypeReq)
{
   return 0;
}

U32 TTagExprNode::compile(U32*, U32 ip, TypeReq)
{
   return ip;
}

TypeReq TTagExprNode::getPreferredType()
{
   return TypeReqNone;
}

//------------------------------------------------------------

U32 FuncCallExprNode::precompile(TypeReq type)
{
   // OP_PUSH_FRAME
   // arg OP_PUSH arg OP_PUSH arg OP_PUSH
   // eval all the args, then call the function.

   // OP_CALLFUNC
   // function
   // namespace
   // isDot

   U32 size = 0;
   if(type != TypeReqString)
      size++;
   precompileIdent(funcName);
   precompileIdent(nameSpace);
   for(ExprNode *walk = args; walk; walk = (ExprNode *) walk->getNext())
      size += walk->precompile(TypeReqString) + 1;
   return size + 5;
}

U32 FuncCallExprNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   codeStream[ip++] = OP_PUSH_FRAME;
   for(ExprNode *walk = args; walk; walk = (ExprNode *) walk->getNext())
   {
      ip = walk->compile(codeStream, ip, TypeReqString);
      codeStream[ip++] = OP_PUSH;
   }
   if(callType == MethodCall || callType == ParentCall)
      codeStream[ip++] = OP_CALLFUNC;
   else
      codeStream[ip++] = OP_CALLFUNC_RESOLVE;

   codeStream[ip] = STEtoU32(funcName, ip);
   ip++;
   codeStream[ip] = STEtoU32(nameSpace, ip);
   ip++;
   codeStream[ip++] = callType;
   if(type != TypeReqString)
      codeStream[ip++] = conversionOp(TypeReqString, type);
   return ip;
}

TypeReq FuncCallExprNode::getPreferredType()
{
   return TypeReqString;
}


//------------------------------------------------------------

U32 AssertCallExprNode::precompile( TypeReq type )
{
   #ifdef TORQUE_ENABLE_SCRIPTASSERTS

      messageIndex = getCurrentStringTable()->add( message, true, false );

      U32 exprSize = testExpr->precompile(TypeReqUInt);
      return exprSize + 2;

   #else

      return 0;

   #endif
}

U32 AssertCallExprNode::compile( U32 *codeStream, U32 ip, TypeReq type )
{
   #ifdef TORQUE_ENABLE_SCRIPTASSERTS

      ip = testExpr->compile( codeStream, ip, TypeReqUInt );
      codeStream[ip++] = OP_ASSERT;
      codeStream[ip++] = messageIndex;

   #endif

   return ip;
}

TypeReq AssertCallExprNode::getPreferredType()
{
   return TypeReqNone;
}

//------------------------------------------------------------

U32 SlotAccessNode::precompile(TypeReq type)
{
   if(type == TypeReqNone)
      return 0;
   U32 size = 0;
   precompileIdent(slotName);
   if(arrayExpr)
   {
      // eval array
      // OP_ADVANCE_STR
      // evaluate object expression sub (OP_SETCURFIELD)
      // OP_TERMINATE_REWIND_STR
      // OP_SETCURFIELDARRAY
      // total add of 4 + array precomp
      size += 3 + arrayExpr->precompile(TypeReqString);
   }
   // eval object expression sub + 3 (op_setCurField + OP_SETCUROBJECT)
   size += objectExpr->precompile(TypeReqString) + 3;

   // get field in desired type:
   return size + 1;
}

U32 SlotAccessNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   if(type == TypeReqNone)
      return ip;

   if(arrayExpr)
   {
      ip = arrayExpr->compile(codeStream, ip, TypeReqString);
      codeStream[ip++] = OP_ADVANCE_STR;
   }
   ip = objectExpr->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_SETCUROBJECT;
   
   codeStream[ip++] = OP_SETCURFIELD;
   
   codeStream[ip] = STEtoU32(slotName, ip);
   ip++;

   if(arrayExpr)
   {
      codeStream[ip++] = OP_TERMINATE_REWIND_STR;
      codeStream[ip++] = OP_SETCURFIELD_ARRAY;
   }
   
   switch(type)
   {
      case TypeReqUInt:
         codeStream[ip++] = OP_LOADFIELD_UINT;
         break;
      case TypeReqFloat:
         codeStream[ip++] = OP_LOADFIELD_FLT;
         break;
      case TypeReqString:
         codeStream[ip++] = OP_LOADFIELD_STR;
         break;
      case TypeReqNone:
         break;
   }
   return ip;
}

TypeReq SlotAccessNode::getPreferredType()
{
   return TypeReqNone;
}

//-----------------------------------------------------------------------------

U32 InternalSlotAccessNode::precompile(TypeReq type)
{
   if(type == TypeReqNone)
      return 0;

   U32 size = 3;
   
   // eval object expression sub + 3 (op_setCurField + OP_SETCUROBJECT)
   size += objectExpr->precompile(TypeReqString);
   size += slotExpr->precompile(TypeReqString);
   if(type != TypeReqUInt)
      size++;

   // get field in desired type:
   return size;
}

U32 InternalSlotAccessNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   if(type == TypeReqNone)
      return ip;

   ip = objectExpr->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_SETCUROBJECT;

   ip = slotExpr->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_SETCUROBJECT_INTERNAL;
   codeStream[ip++] = recurse;

   if(type != TypeReqUInt)
      codeStream[ip++] = conversionOp(TypeReqUInt, type);
   return ip;
}

TypeReq InternalSlotAccessNode::getPreferredType()
{
   return TypeReqUInt;
}

//-----------------------------------------------------------------------------

//------------------------------------------------------------

U32 SlotAssignNode::precompile(TypeReq type)
{
   // first eval the expression TypeReqString

   // if it's an array:

   // if OP_ADVANCE_STR 1
   // eval array

   // OP_ADVANCE_STR 1
   // evaluate object expr
   // OP_SETCUROBJECT 1
   // OP_SETCURFIELD 1
   // fieldName 1
   // OP_TERMINATE_REWIND_STR 1

   // OP_SETCURFIELDARRAY 1
   // OP_TERMINATE_REWIND_STR 1

   // else
   // OP_ADVANCE_STR
   // evaluate object expr
   // OP_SETCUROBJECT
   // OP_SETCURFIELD
   // fieldName
   // OP_TERMINATE_REWIND_STR

   // OP_SAVEFIELD
   // convert to return type if necessary.

   U32 size = 0;
   if(type != TypeReqString)
      size++;

   precompileIdent(slotName);

   size += valueExpr->precompile(TypeReqString);

   if(objectExpr)
      size += objectExpr->precompile(TypeReqString) + 5;
   else
      size += 5;

   if(arrayExpr)
      size += arrayExpr->precompile(TypeReqString) + 3;

   if(typeID != -1)
      size += 2;

   return size + 1;
}

U32 SlotAssignNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   ip = valueExpr->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_ADVANCE_STR;
   if(arrayExpr)
   {
      ip = arrayExpr->compile(codeStream, ip, TypeReqString);
      codeStream[ip++] = OP_ADVANCE_STR;
   }
   if(objectExpr)
   {
      ip = objectExpr->compile(codeStream, ip, TypeReqString);
      codeStream[ip++] = OP_SETCUROBJECT;
   }
   else
      codeStream[ip++] = OP_SETCUROBJECT_NEW;
   codeStream[ip++] = OP_SETCURFIELD;
   codeStream[ip] = STEtoU32(slotName, ip);
   ip++;
   if(arrayExpr)
   {
      codeStream[ip++] = OP_TERMINATE_REWIND_STR;
      codeStream[ip++] = OP_SETCURFIELD_ARRAY;
   }

   codeStream[ip++] = OP_TERMINATE_REWIND_STR;
   codeStream[ip++] = OP_SAVEFIELD_STR;

   if(typeID != -1)
   {
      codeStream[ip++] = OP_SETCURFIELD_TYPE;
      codeStream[ip++] = typeID;
   }

   if(type != TypeReqString)
      codeStream[ip++] = conversionOp(TypeReqString, type);
   return ip;
}

TypeReq SlotAssignNode::getPreferredType()
{
   return TypeReqString;
}

//------------------------------------------------------------

U32 SlotAssignOpNode::precompile(TypeReq type)
{
   // first eval the expression as its type

   // if it's an array:
   // eval array
   // OP_ADVANCE_STR
   // evaluate object expr
   // OP_SETCUROBJECT
   // OP_SETCURFIELD
   // fieldName
   // OP_TERMINATE_REWIND_STR
   // OP_SETCURFIELDARRAY

   // else
   // evaluate object expr
   // OP_SETCUROBJECT
   // OP_SETCURFIELD
   // fieldName

   // OP_LOADFIELD of appropriate type
   // operand
   // OP_SAVEFIELD of appropriate type
   // convert to return type if necessary.

   getAssignOpTypeOp(op, subType, operand);
   precompileIdent(slotName);
   U32 size = valueExpr->precompile(subType);
   if(type != subType)
      size++;
   if(arrayExpr)
      return size + 9 + arrayExpr->precompile(TypeReqString) + objectExpr->precompile(TypeReqString);
   else
      return size + 6 + objectExpr->precompile(TypeReqString);
}

U32 SlotAssignOpNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   ip = valueExpr->compile(codeStream, ip, subType);
   if(arrayExpr)
   {
      ip = arrayExpr->compile(codeStream, ip, TypeReqString);
      codeStream[ip++] = OP_ADVANCE_STR;
   }
   ip = objectExpr->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_SETCUROBJECT;
   codeStream[ip++] = OP_SETCURFIELD;
   codeStream[ip] = STEtoU32(slotName, ip);
   ip++;
   if(arrayExpr)
   {
      codeStream[ip++] = OP_TERMINATE_REWIND_STR;
      codeStream[ip++] = OP_SETCURFIELD_ARRAY;
   }
   codeStream[ip++] = (subType == TypeReqFloat) ? OP_LOADFIELD_FLT : OP_LOADFIELD_UINT;
   codeStream[ip++] = operand;
   codeStream[ip++] = (subType == TypeReqFloat) ? OP_SAVEFIELD_FLT : OP_SAVEFIELD_UINT;
   if(subType != type)
      codeStream[ip++] = conversionOp(subType, type);
   return ip;
}

TypeReq SlotAssignOpNode::getPreferredType()
{
   getAssignOpTypeOp(op, subType, operand);
   return subType;
}

//------------------------------------------------------------

U32 ObjectDeclNode::precompileSubObject(bool)
{
   // goes

   // OP_PUSHFRAME 1
   // name expr
   // OP_PUSH 1
   // args... PUSH
   // OP_CREATE_OBJECT 1
   // parentObject 1
   // isDatablock 1
   // internalName 1
   // isSingleton 1
   // lineNumber 1
   // fail point 1

   // for each field, eval
   // OP_ADD_OBJECT (to UINT[0]) 1
   // root? 1

   // add all the sub objects.
   // OP_END_OBJECT 1
   // root? 1
   // To fix the stack issue [7/9/2007 Black]
   // OP_FINISH_OBJECT <-- fail point jumps to this opcode

   U32 argSize = 0;
   precompileIdent(parentObject);
   for(ExprNode *exprWalk = argList; exprWalk; exprWalk = (ExprNode *) exprWalk->getNext())
      argSize += exprWalk->precompile(TypeReqString) + 1;
   argSize += classNameExpr->precompile(TypeReqString) + 1;

   U32 nameSize = objectNameExpr->precompile(TypeReqString) + 1;

   U32 slotSize = 0;
   for(SlotAssignNode *slotWalk = slotDecls; slotWalk; slotWalk = (SlotAssignNode *) slotWalk->getNext())
      slotSize += slotWalk->precompile(TypeReqNone);

   // OP_ADD_OBJECT
   U32 subObjSize = 0;
   for(ObjectDeclNode *objectWalk = subObjects; objectWalk; objectWalk = (ObjectDeclNode *) objectWalk->getNext())
      subObjSize += objectWalk->precompileSubObject(false);

   failOffset = 12 + nameSize + argSize + slotSize + subObjSize;
   // +1 because the failOffset should jump to OP_FINISH_OBJECT [7/9/2007 Black]
   return failOffset + 1;
}

U32 ObjectDeclNode::precompile(TypeReq type)
{
   // root object decl does:

   // push 0 onto the UINT stack OP_LOADIMMED_UINT
   // precompiles the subObject(true)
   // UINT stack now has object id
   // type conv to type

   U32 ret = 2 + precompileSubObject(true);
   if(type != TypeReqUInt)
      return ret + 1;
   return ret;
}

U32 ObjectDeclNode::compileSubObject(U32 *codeStream, U32 ip, bool root)
{
   U32 start = ip;
   codeStream[ip++] = OP_PUSH_FRAME;
   ip = classNameExpr->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_PUSH;

   ip = objectNameExpr->compile(codeStream, ip, TypeReqString);
   codeStream[ip++] = OP_PUSH;
   for(ExprNode *exprWalk = argList; exprWalk; exprWalk = (ExprNode *) exprWalk->getNext())
   {
      ip = exprWalk->compile(codeStream, ip, TypeReqString);
      codeStream[ip++] = OP_PUSH;
   }
   codeStream[ip++] = OP_CREATE_OBJECT;
   codeStream[ip] = STEtoU32(parentObject, ip);
   ip++;
   codeStream[ip++] = isDatablock;
   codeStream[ip++] = isClassNameInternal;
   codeStream[ip++] = isSingleton;
   codeStream[ip++] = dbgLineNumber;
   codeStream[ip++] = start + failOffset;
   for(SlotAssignNode *slotWalk = slotDecls; slotWalk; slotWalk = (SlotAssignNode *) slotWalk->getNext())
      ip = slotWalk->compile(codeStream, ip, TypeReqNone);
   codeStream[ip++] = OP_ADD_OBJECT;
   codeStream[ip++] = root;
   for(ObjectDeclNode *objectWalk = subObjects; objectWalk; objectWalk = (ObjectDeclNode *) objectWalk->getNext())
      ip = objectWalk->compileSubObject(codeStream, ip, false);
   codeStream[ip++] = OP_END_OBJECT;
   codeStream[ip++] = root || isDatablock;
   // Added to fix the object creation issue [7/9/2007 Black]
   codeStream[ip++] = OP_FINISH_OBJECT;
   return ip;
}

U32 ObjectDeclNode::compile(U32 *codeStream, U32 ip, TypeReq type)
{
   codeStream[ip++] = OP_LOADIMMED_UINT;
   codeStream[ip++] = 0;
   ip = compileSubObject(codeStream, ip, true);
   if(type != TypeReqUInt)
      codeStream[ip++] = conversionOp(TypeReqUInt, type);
   return ip;
}   
TypeReq ObjectDeclNode::getPreferredType()
{
   return TypeReqUInt;
}

//------------------------------------------------------------

U32 FunctionDeclStmtNode::precompileStmt(U32)
{
   // OP_FUNC_DECL
   // func name
   // namespace
   // package
   // hasBody?
   // func end ip
   // argc
   // ident array[argc]
   // code
   // OP_RETURN_VOID
   setCurrentStringTable(&getFunctionStringTable());
   setCurrentFloatTable(&getFunctionFloatTable());

   argc = 0;
   for(VarNode *walk = args; walk; walk = (VarNode *)((StmtNode*)walk)->getNext())
      argc++;
   
   CodeBlock::smInFunction = true;
   
   precompileIdent(fnName);
   precompileIdent(nameSpace);
   precompileIdent(package);
   
   U32 subSize = precompileBlock(stmts, 0);
   CodeBlock::smInFunction = false;

   addBreakCount();

   setCurrentStringTable(&getGlobalStringTable());
   setCurrentFloatTable(&getGlobalFloatTable());

   endOffset = argc + subSize + 8;
   return endOffset;
}

U32 FunctionDeclStmtNode::compileStmt(U32 *codeStream, U32 ip, U32, U32)
{
   U32 start = ip;
   codeStream[ip++] = OP_FUNC_DECL;
   codeStream[ip] = STEtoU32(fnName, ip);
   ip++;
   codeStream[ip] = STEtoU32(nameSpace, ip);
   ip++;
   codeStream[ip] = STEtoU32(package, ip);
   ip++;
   codeStream[ip++] = U32( bool(stmts != NULL) ? 1 : 0 ) + U32( dbgLineNumber << 1 );
   codeStream[ip++] = start + endOffset;
   codeStream[ip++] = argc;
   for(VarNode *walk = args; walk; walk = (VarNode *)((StmtNode*)walk)->getNext())
   {
      codeStream[ip] = STEtoU32(walk->varName, ip);
      ip++;
   }
   CodeBlock::smInFunction = true;
   ip = compileBlock(stmts, codeStream, ip, 0, 0); 

   // Add break so breakpoint can be set at closing brace or
   // in empty function.
   addBreakLine( ip );

   CodeBlock::smInFunction = false;
   codeStream[ip++] = OP_RETURN_VOID;
   return ip;
}
