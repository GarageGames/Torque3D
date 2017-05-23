%{

// bison --defines=cmdgram.h --verbose -o cmdgram.cpp -p CMD CMDgram.y

// Make sure we don't get gram.h twice.
#define _CMDGRAM_H_

#include <stdlib.h>
#include <stdio.h>
#include "console/console.h"
#include "console/compiler.h"
#include "console/consoleInternal.h"
#include "core/strings/stringFunctions.h"

#ifndef YYDEBUG
#define YYDEBUG 0
#endif

#define YYSSIZE 350

int outtext(char *fmt, ...);
extern int serrors;

#define nil 0
#undef YY_ARGS
#define YY_ARGS(x)   x

int CMDlex();
void CMDerror(char *, ...);

#ifdef alloca
#undef alloca
#endif
#define alloca dMalloc

template< typename T >
struct Token
{
   T value;
   U32 lineNumber;
};

%}
%{
        /* Reserved Word Definitions */
%}
%token <i> rwDEFINE rwENDDEF rwDECLARE rwDECLARESINGLETON
%token <i> rwBREAK rwELSE rwCONTINUE rwGLOBAL
%token <i> rwIF rwNIL rwRETURN rwWHILE rwDO
%token <i> rwENDIF rwENDWHILE rwENDFOR rwDEFAULT
%token <i> rwFOR rwFOREACH rwFOREACHSTR rwIN rwDATABLOCK rwSWITCH rwCASE rwSWITCHSTR
%token <i> rwCASEOR rwPACKAGE rwNAMESPACE rwCLASS
%token <i> rwASSERT
%token ILLEGAL_TOKEN
%{
        /* Constants and Identifier Definitions */
%}
%token <c>   CHRCONST
%token <i>   INTCONST
%token <s>   TTAG
%token <s>   VAR
%token <s>   IDENT
%token <i>   TYPEIDENT
%token <str> DOCBLOCK
%token <str> STRATOM
%token <str> TAGATOM
%token <f>   FLTCONST

%{
        /* Operator Definitions */
%}
%token <i> '+' '-' '*' '/' '<' '>' '=' '.' '|' '&' '%'
%token <i> '(' ')' ',' ':' ';' '{' '}' '^' '~' '!' '@'
%token <i> opINTNAME opINTNAMER
%token <i> opMINUSMINUS opPLUSPLUS
%token <i> STMT_SEP
%token <i> opSHL opSHR opPLASN opMIASN opMLASN opDVASN opMODASN opANDASN
%token <i> opXORASN opORASN opSLASN opSRASN opCAT
%token <i> opEQ opNE opGE opLE opAND opOR opSTREQ
%token <i> opCOLONCOLON

%union {
   Token< char >           c;
   Token< int >            i;
   Token< const char* >    s;
   Token< char* >          str;
   Token< double >         f;
   StmtNode*               stmt;
   ExprNode*               expr;
   SlotAssignNode*         slist;
   VarNode*                var;
   SlotDecl                slot;
   InternalSlotDecl        intslot;
   ObjectBlockDecl         odcl;
   ObjectDeclNode*         od;
   AssignDecl              asn;
   IfStmtNode*             ifnode;
}

%type <s>      parent_block
%type <ifnode> case_block
%type <stmt>   switch_stmt
%type <stmt>   decl
%type <stmt>   decl_list
%type <stmt>   package_decl
%type <stmt>   fn_decl_stmt
%type <stmt>   fn_decl_list
%type <stmt>   statement_list
%type <stmt>   stmt
%type <expr>   expr_list
%type <expr>   expr_list_decl
%type <expr>   aidx_expr
%type <expr>   funcall_expr
%type <expr>   assert_expr
%type <expr>   object_name
%type <expr>   object_args
%type <expr>   stmt_expr
%type <expr>   case_expr
%type <expr>   class_name_expr
%type <stmt>   if_stmt
%type <stmt>   while_stmt
%type <stmt>   for_stmt
%type <stmt>   foreach_stmt
%type <stmt>   stmt_block
%type <stmt>   datablock_decl
%type <od>     object_decl
%type <od>     object_decl_list
%type <odcl>   object_declare_block
%type <expr>   expr
%type <slist>  slot_assign_list_opt
%type <slist>  slot_assign_list
%type <slist>  slot_assign
%type <slot>   slot_acc
%type <intslot>   intslot_acc
%type <stmt>   expression_stmt
%type <var>    var_list
%type <var>    var_list_decl
%type <asn>    assign_op_struct

%left '['
%right opMODASN opANDASN opXORASN opPLASN opMIASN opMLASN opDVASN opMDASN opNDASN opNTASN opORASN opSLASN opSRASN '='
%left '?' ':'
%left opOR
%left opAND
%left '|'
%left '^'
%left '&'
%left opEQ opNE
%left '<' opLE '>' opGE
%left '@' opCAT opSTREQ opSTRNE
%left opSHL opSHR
%left '+' '-'
%left '*' '/' '%'
%right '!' '~' opPLUSPLUS opMINUSMINUS UNARY
%left '.'
%left opINTNAME opINTNAMER

%%

start
   : decl_list
      { }
   ;

decl_list
   :
      { $$ = nil; }
   | decl_list decl
      { if(!gStatementList) { gStatementList = $2; } else { gStatementList->append($2); } }
   ;

decl
   : stmt
      { $$ = $1; }
   | fn_decl_stmt
      { $$ = $1; }
   | package_decl
     { $$ = $1; }
   ;

package_decl
   : rwPACKAGE IDENT '{' fn_decl_list '}' ';'
      { $$ = $4; for(StmtNode *walk = ($4);walk;walk = walk->getNext() ) walk->setPackage($2.value); }
   ;

fn_decl_list
   : fn_decl_stmt
      { $$ = $1; }
   | fn_decl_list fn_decl_stmt
      { $$ = $1; ($1)->append($2);  }
   ;

statement_list
   :
      { $$ = nil; }
   | statement_list stmt
      { if(!$1) { $$ = $2; } else { ($1)->append($2); $$ = $1; } }
   ;

stmt
   : if_stmt
   | while_stmt
   | for_stmt
   | foreach_stmt
   | datablock_decl
   | switch_stmt
   | rwBREAK ';'
      { $$ = BreakStmtNode::alloc( $1.lineNumber ); }
   | rwCONTINUE ';'
      { $$ = ContinueStmtNode::alloc( $1.lineNumber ); }
   | rwRETURN ';'
      { $$ = ReturnStmtNode::alloc( $1.lineNumber, NULL ); }
   | rwRETURN expr ';'
      { $$ = ReturnStmtNode::alloc( $1.lineNumber, $2 ); }
   | expression_stmt ';'
      { $$ = $1; }
   | TTAG '=' expr ';'
      { $$ = TTagSetStmtNode::alloc( $1.lineNumber, $1.value, $3, NULL ); }
   | TTAG '=' expr ',' expr ';'
      { $$ = TTagSetStmtNode::alloc( $1.lineNumber, $1.value, $3, $5 ); }
   | DOCBLOCK
      { $$ = StrConstNode::alloc( $1.lineNumber, $1.value, false, true ); }
   ;

fn_decl_stmt
   : rwDEFINE IDENT '(' var_list_decl ')' '{' statement_list '}'
      { $$ = FunctionDeclStmtNode::alloc( $1.lineNumber, $2.value, NULL, $4, $7 ); }
    | rwDEFINE IDENT opCOLONCOLON IDENT '(' var_list_decl ')' '{' statement_list '}'
     { $$ = FunctionDeclStmtNode::alloc( $1.lineNumber, $4.value, $2.value, $6, $9 ); }
   ;

var_list_decl
   :
     { $$ = NULL; }
   | var_list
     { $$ = $1; }
   ;

var_list
   : VAR
      { $$ = VarNode::alloc( $1.lineNumber, $1.value, NULL ); }
   | var_list ',' VAR
      { $$ = $1; ((StmtNode*)($1))->append((StmtNode*)VarNode::alloc( $3.lineNumber, $3.value, NULL ) ); }
   ;

datablock_decl
   : rwDATABLOCK class_name_expr '(' expr parent_block ')'  '{' slot_assign_list_opt '}' ';'
      { $$ = ObjectDeclNode::alloc( $1.lineNumber, $2, $4, NULL, $5.value, $8, NULL, true, false, false); }
   ;

object_decl
   : rwDECLARE class_name_expr '(' object_name parent_block object_args ')' '{' object_declare_block '}'
      { $$ = ObjectDeclNode::alloc( $1.lineNumber, $2, $4, $6, $5.value, $9.slots, $9.decls, false, false, false); }
   | rwDECLARE class_name_expr '(' object_name parent_block object_args ')'
      { $$ = ObjectDeclNode::alloc( $1.lineNumber, $2, $4, $6, $5.value, NULL, NULL, false, false, false); }
   | rwDECLARE class_name_expr '(' '[' object_name ']' parent_block object_args ')' '{' object_declare_block '}'
      { $$ = ObjectDeclNode::alloc( $1.lineNumber, $2, $5, $8, $7.value, $11.slots, $11.decls, false, true, false); }
   | rwDECLARE class_name_expr '(' '[' object_name ']' parent_block object_args ')'
      { $$ = ObjectDeclNode::alloc( $1.lineNumber, $2, $5, $8, $7.value, NULL, NULL, false, true, false); }
   | rwDECLARESINGLETON class_name_expr '(' object_name parent_block object_args ')' '{' object_declare_block '}'
      { $$ = ObjectDeclNode::alloc( $1.lineNumber, $2, $4, $6, $5.value, $9.slots, $9.decls, false, false, true); }
   | rwDECLARESINGLETON class_name_expr '(' object_name parent_block object_args ')'
      { $$ = ObjectDeclNode::alloc( $1.lineNumber, $2, $4, $6, $5.value, NULL, NULL, false, false, true); }
   ;

parent_block
   :
      { $$.value = NULL; }
   | ':' IDENT
      { $$ = $2; }
   ;

object_name
   :
      { $$ = StrConstNode::alloc( CodeBlock::smCurrentParser->getCurrentLine(), "", false); }
   | expr
      { $$ = $1; }
   ;

object_args
   :
      { $$ = NULL; }
   | ',' expr_list
      { $$ = $2; }
   ;

object_declare_block
   :
      { $$.slots = NULL; $$.decls = NULL; }
   | slot_assign_list
      { $$.slots = $1; $$.decls = NULL; }
   | object_decl_list
      { $$.slots = NULL; $$.decls = $1; }
   | slot_assign_list object_decl_list
      { $$.slots = $1; $$.decls = $2; }
   ;

object_decl_list
   : object_decl ';'
      { $$ = $1; }
   | object_decl_list object_decl ';'
      { $1->append($2); $$ = $1; }
   ;

stmt_block
   : '{' statement_list '}'
      { $$ = $2; }
   | stmt
      { $$ = $1; }
   ;

switch_stmt
   : rwSWITCH '(' expr ')' '{' case_block '}'
      { $$ = $6; $6->propagateSwitchExpr($3, false); }
   | rwSWITCHSTR '(' expr ')' '{' case_block '}'
      { $$ = $6; $6->propagateSwitchExpr($3, true); }
   ;

case_block
   : rwCASE case_expr ':' statement_list
      { $$ = IfStmtNode::alloc( $1.lineNumber, $2, $4, NULL, false); }
   | rwCASE case_expr ':' statement_list rwDEFAULT ':' statement_list
      { $$ = IfStmtNode::alloc( $1.lineNumber, $2, $4, $7, false); }
   | rwCASE case_expr ':' statement_list case_block
      { $$ = IfStmtNode::alloc( $1.lineNumber, $2, $4, $5, true); }
   ;

case_expr
   : expr
      { $$ = $1;}
   | case_expr rwCASEOR expr
      { ($1)->append($3); $$=$1; }
   ;

if_stmt
   : rwIF '(' expr ')' stmt_block
      { $$ = IfStmtNode::alloc($1.lineNumber, $3, $5, NULL, false); }
   | rwIF '(' expr ')' stmt_block rwELSE stmt_block
      { $$ = IfStmtNode::alloc($1.lineNumber, $3, $5, $7, false); }
   ;

while_stmt
   : rwWHILE '(' expr ')' stmt_block
      { $$ = LoopStmtNode::alloc($1.lineNumber, nil, $3, nil, $5, false); }
   | rwDO stmt_block rwWHILE '(' expr ')'
      { $$ = LoopStmtNode::alloc($3.lineNumber, nil, $5, nil, $2, true); }
   ;

for_stmt
   : rwFOR '(' expr ';' expr ';' expr ')' stmt_block
      { $$ = LoopStmtNode::alloc($1.lineNumber, $3, $5, $7, $9, false); }
   | rwFOR '(' expr ';' expr ';'      ')' stmt_block
      { $$ = LoopStmtNode::alloc($1.lineNumber, $3, $5, NULL, $8, false); }
   | rwFOR '(' expr ';'      ';' expr ')' stmt_block
      { $$ = LoopStmtNode::alloc($1.lineNumber, $3, NULL, $6, $8, false); }
   | rwFOR '(' expr ';'      ';'      ')' stmt_block
      { $$ = LoopStmtNode::alloc($1.lineNumber, $3, NULL, NULL, $7, false); }
   | rwFOR '('      ';' expr ';' expr ')' stmt_block
      { $$ = LoopStmtNode::alloc($1.lineNumber, NULL, $4, $6, $8, false); }
   | rwFOR '('      ';' expr ';'      ')' stmt_block
      { $$ = LoopStmtNode::alloc($1.lineNumber, NULL, $4, NULL, $7, false); }
   | rwFOR '('      ';'      ';' expr ')' stmt_block
      { $$ = LoopStmtNode::alloc($1.lineNumber, NULL, NULL, $5, $7, false); }
   | rwFOR '('      ';'      ';'      ')' stmt_block
      { $$ = LoopStmtNode::alloc($1.lineNumber, NULL, NULL, NULL, $6, false); }
   ;
   
foreach_stmt
   : rwFOREACH '(' VAR rwIN expr ')' stmt_block
      { $$ = IterStmtNode::alloc( $1.lineNumber, $3.value, $5, $7, false ); }
   | rwFOREACHSTR '(' VAR rwIN expr ')' stmt_block
      { $$ = IterStmtNode::alloc( $1.lineNumber, $3.value, $5, $7, true ); }
   ;

expression_stmt
   : stmt_expr
      { $$ = $1; }
   ;

expr
   : stmt_expr
      { $$ = $1; }
   | '(' expr ')'
      { $$ = $2; }
   | expr '^' expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr '%' expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr '&' expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr '|' expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr '+' expr
      { $$ = FloatBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr '-' expr
      { $$ = FloatBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr '*' expr
      { $$ = FloatBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr '/' expr
      { $$ = FloatBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | '-' expr  %prec UNARY
      { $$ = FloatUnaryExprNode::alloc( $1.lineNumber, $1.value, $2); }
   | '*' expr %prec UNARY
      { $$ = TTagDerefNode::alloc( $1.lineNumber, $2 ); }
   | TTAG
      { $$ = TTagExprNode::alloc( $1.lineNumber, $1.value ); }
   | expr '?' expr ':' expr
      { $$ = ConditionalExprNode::alloc( $1->dbgLineNumber, $1, $3, $5); }
   | expr '<' expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr '>' expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr opGE expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr opLE expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr opEQ expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr opNE expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr opOR expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr opSHL expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr opSHR expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr opAND expr
      { $$ = IntBinaryExprNode::alloc( $1->dbgLineNumber, $2.value, $1, $3); }
   | expr opSTREQ expr
      { $$ = StreqExprNode::alloc( $1->dbgLineNumber, $1, $3, true); }
   | expr opSTRNE expr
      { $$ = StreqExprNode::alloc( $1->dbgLineNumber, $1, $3, false); }
   | expr '@' expr
      { $$ = StrcatExprNode::alloc( $1->dbgLineNumber, $1, $3, $2.value); }
   | '!' expr
      { $$ = IntUnaryExprNode::alloc($1.lineNumber, $1.value, $2); }
   | '~' expr
      { $$ = IntUnaryExprNode::alloc($1.lineNumber, $1.value, $2); }
   | TAGATOM
      { $$ = StrConstNode::alloc( $1.lineNumber, $1.value, true); }
   | FLTCONST
      { $$ = FloatNode::alloc( $1.lineNumber, $1.value ); }
   | INTCONST
      { $$ = IntNode::alloc( $1.lineNumber, $1.value ); }
   | rwBREAK
      { $$ = ConstantNode::alloc( $1.lineNumber, StringTable->insert("break")); }
   | slot_acc
      { $$ = SlotAccessNode::alloc( $1.lineNumber, $1.object, $1.array, $1.slotName ); }
   | intslot_acc
      { $$ = InternalSlotAccessNode::alloc( $1.lineNumber, $1.object, $1.slotExpr, $1.recurse); }
   | IDENT
      { $$ = ConstantNode::alloc( $1.lineNumber, $1.value ); }
   | STRATOM
      { $$ = StrConstNode::alloc( $1.lineNumber, $1.value, false); }
   | VAR
      { $$ = (ExprNode*)VarNode::alloc( $1.lineNumber, $1.value, NULL); }
   | VAR '[' aidx_expr ']'
      { $$ = (ExprNode*)VarNode::alloc( $1.lineNumber, $1.value, $3 ); }
   | rwDEFINE '(' var_list_decl ')' '{' statement_list '}'
      {
         const U32 bufLen = 64;
         UTF8 buffer[bufLen];
         dSprintf(buffer, bufLen, "__anonymous_function%d", gAnonFunctionID++);
         StringTableEntry fName = StringTable->insert(buffer);
         StmtNode *fndef = FunctionDeclStmtNode::alloc($1.lineNumber, fName, NULL, $3, $6);

         if(!gAnonFunctionList)
            gAnonFunctionList = fndef;
         else
            gAnonFunctionList->append(fndef);

         $$ = StrConstNode::alloc( $1.lineNumber, (UTF8*)fName, false );
      }
   ;

slot_acc
   : expr '.' IDENT
      { $$.lineNumber = $1->dbgLineNumber; $$.object = $1; $$.slotName = $3.value; $$.array = NULL; }
   | expr '.' IDENT '[' aidx_expr ']'
      { $$.lineNumber = $1->dbgLineNumber; $$.object = $1; $$.slotName = $3.value; $$.array = $5; }
   ;

intslot_acc
   : expr opINTNAME class_name_expr
     { $$.lineNumber = $1->dbgLineNumber; $$.object = $1; $$.slotExpr = $3; $$.recurse = false; }
   | expr opINTNAMER class_name_expr
     { $$.lineNumber = $1->dbgLineNumber; $$.object = $1; $$.slotExpr = $3; $$.recurse = true; }
   ;

class_name_expr
   : IDENT
      { $$ = ConstantNode::alloc( $1.lineNumber, $1.value ); }
   | '(' expr ')'
      { $$ = $2; }
   ;

assign_op_struct
   : opPLUSPLUS
      { $$.lineNumber = $1.lineNumber; $$.token = '+'; $$.expr = FloatNode::alloc( $1.lineNumber, 1 ); }
   | opMINUSMINUS
      { $$.lineNumber = $1.lineNumber; $$.token = '-'; $$.expr = FloatNode::alloc( $1.lineNumber, 1 ); }
   | opPLASN expr
      { $$.lineNumber = $1.lineNumber; $$.token = '+'; $$.expr = $2; }
   | opMIASN expr
      { $$.lineNumber = $1.lineNumber; $$.token = '-'; $$.expr = $2; }
   | opMLASN expr
      { $$.lineNumber = $1.lineNumber; $$.token = '*'; $$.expr = $2; }
   | opDVASN expr
      { $$.lineNumber = $1.lineNumber; $$.token = '/'; $$.expr = $2; }
   | opMODASN expr
      { $$.lineNumber = $1.lineNumber; $$.token = '%'; $$.expr = $2; }
   | opANDASN expr
      { $$.lineNumber = $1.lineNumber; $$.token = '&'; $$.expr = $2; }
   | opXORASN expr
      { $$.lineNumber = $1.lineNumber; $$.token = '^'; $$.expr = $2; }
   | opORASN expr
      { $$.lineNumber = $1.lineNumber; $$.token = '|'; $$.expr = $2; }
   | opSLASN expr
      { $$.lineNumber = $1.lineNumber; $$.token = opSHL; $$.expr = $2; }
   | opSRASN expr
      { $$.lineNumber = $1.lineNumber; $$.token = opSHR; $$.expr = $2; }
   ;

stmt_expr
   : funcall_expr
      { $$ = $1; }
   | assert_expr
      { $$ = $1; }      
   | object_decl
      { $$ = $1; }
   | VAR '=' expr
      { $$ = AssignExprNode::alloc( $1.lineNumber, $1.value, NULL, $3); }
   | VAR '[' aidx_expr ']' '=' expr
      { $$ = AssignExprNode::alloc( $1.lineNumber, $1.value, $3, $6); }
   | VAR assign_op_struct
      { $$ = AssignOpExprNode::alloc( $1.lineNumber, $1.value, NULL, $2.expr, $2.token); }
   | VAR '[' aidx_expr ']' assign_op_struct
      { $$ = AssignOpExprNode::alloc( $1.lineNumber, $1.value, $3, $5.expr, $5.token); }
   | slot_acc assign_op_struct
      { $$ = SlotAssignOpNode::alloc( $1.lineNumber, $1.object, $1.slotName, $1.array, $2.token, $2.expr); }
   | slot_acc '=' expr
      { $$ = SlotAssignNode::alloc( $1.lineNumber, $1.object, $1.array, $1.slotName, $3); }
   | slot_acc '=' '{' expr_list '}'
      { $$ = SlotAssignNode::alloc( $1.lineNumber, $1.object, $1.array, $1.slotName, $4); }
   ;

funcall_expr
   : IDENT '(' expr_list_decl ')'
     { $$ = FuncCallExprNode::alloc( $1.lineNumber, $1.value, NULL, $3, false); }
   | IDENT opCOLONCOLON IDENT '(' expr_list_decl ')'
     { $$ = FuncCallExprNode::alloc( $1.lineNumber, $3.value, $1.value, $5, false); }
   | expr '.' IDENT '(' expr_list_decl ')'
      { $1->append($5); $$ = FuncCallExprNode::alloc( $1->dbgLineNumber, $3.value, NULL, $1, true); }
   ;

assert_expr
   : rwASSERT '(' expr ')'
      { $$ = AssertCallExprNode::alloc( $1.lineNumber, $3, NULL ); }
   | rwASSERT '(' expr ',' STRATOM ')'
      { $$ = AssertCallExprNode::alloc( $1.lineNumber, $3, $5.value ); }
   ;
                  
expr_list_decl
   :
      { $$ = NULL; }
   | expr_list
      { $$ = $1; }
   ;

expr_list
   : expr
      { $$ = $1; }
   | expr_list ',' expr
      { ($1)->append($3); $$ = $1; }
   ;
   
slot_assign_list_opt
   :
      { $$ = NULL; }
   | slot_assign_list
      { $$ = $1; }
   ;

slot_assign_list
   : slot_assign
      { $$ = $1; }
   | slot_assign_list slot_assign
      { $1->append($2); $$ = $1; }
   ;

slot_assign
   : IDENT '=' expr ';'
      { $$ = SlotAssignNode::alloc( $1.lineNumber, NULL, NULL, $1.value, $3); }
   | TYPEIDENT IDENT '=' expr ';'
      { $$ = SlotAssignNode::alloc( $1.lineNumber, NULL, NULL, $2.value, $4, $1.value); }
   | rwDATABLOCK '=' expr ';'
      { $$ = SlotAssignNode::alloc( $1.lineNumber, NULL, NULL, StringTable->insert("datablock"), $3); }
   | IDENT '[' aidx_expr ']' '=' expr ';'
      { $$ = SlotAssignNode::alloc( $1.lineNumber, NULL, $3, $1.value, $6); }
   | TYPEIDENT IDENT '[' aidx_expr ']' '=' expr ';'
      { $$ = SlotAssignNode::alloc( $1.lineNumber, NULL, $4, $2.value, $7, $1.value); }
   ;

aidx_expr
   : expr
      { $$ = $1; }
   | aidx_expr ',' expr
      { $$ = CommaCatExprNode::alloc( $1->dbgLineNumber, $1, $3); }
   ;
%%

