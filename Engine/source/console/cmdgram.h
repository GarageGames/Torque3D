/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     rwDEFINE = 258,
     rwENDDEF = 259,
     rwDECLARE = 260,
     rwDECLARESINGLETON = 261,
     rwBREAK = 262,
     rwELSE = 263,
     rwCONTINUE = 264,
     rwGLOBAL = 265,
     rwIF = 266,
     rwNIL = 267,
     rwRETURN = 268,
     rwWHILE = 269,
     rwDO = 270,
     rwENDIF = 271,
     rwENDWHILE = 272,
     rwENDFOR = 273,
     rwDEFAULT = 274,
     rwFOR = 275,
     rwFOREACH = 276,
     rwFOREACHSTR = 277,
     rwIN = 278,
     rwDATABLOCK = 279,
     rwSWITCH = 280,
     rwCASE = 281,
     rwSWITCHSTR = 282,
     rwCASEOR = 283,
     rwPACKAGE = 284,
     rwNAMESPACE = 285,
     rwCLASS = 286,
     rwASSERT = 287,
     ILLEGAL_TOKEN = 288,
     CHRCONST = 289,
     INTCONST = 290,
     TTAG = 291,
     VAR = 292,
     IDENT = 293,
     TYPEIDENT = 294,
     DOCBLOCK = 295,
     STRATOM = 296,
     TAGATOM = 297,
     FLTCONST = 298,
     opINTNAME = 299,
     opINTNAMER = 300,
     opMINUSMINUS = 301,
     opPLUSPLUS = 302,
     STMT_SEP = 303,
     opSHL = 304,
     opSHR = 305,
     opPLASN = 306,
     opMIASN = 307,
     opMLASN = 308,
     opDVASN = 309,
     opMODASN = 310,
     opANDASN = 311,
     opXORASN = 312,
     opORASN = 313,
     opSLASN = 314,
     opSRASN = 315,
     opCAT = 316,
     opEQ = 317,
     opNE = 318,
     opGE = 319,
     opLE = 320,
     opAND = 321,
     opOR = 322,
     opSTREQ = 323,
     opCOLONCOLON = 324,
     opNTASN = 325,
     opNDASN = 326,
     opMDASN = 327,
     opSTRNE = 328,
     UNARY = 329
   };
#endif
/* Tokens.  */
#define rwDEFINE 258
#define rwENDDEF 259
#define rwDECLARE 260
#define rwDECLARESINGLETON 261
#define rwBREAK 262
#define rwELSE 263
#define rwCONTINUE 264
#define rwGLOBAL 265
#define rwIF 266
#define rwNIL 267
#define rwRETURN 268
#define rwWHILE 269
#define rwDO 270
#define rwENDIF 271
#define rwENDWHILE 272
#define rwENDFOR 273
#define rwDEFAULT 274
#define rwFOR 275
#define rwFOREACH 276
#define rwFOREACHSTR 277
#define rwIN 278
#define rwDATABLOCK 279
#define rwSWITCH 280
#define rwCASE 281
#define rwSWITCHSTR 282
#define rwCASEOR 283
#define rwPACKAGE 284
#define rwNAMESPACE 285
#define rwCLASS 286
#define rwASSERT 287
#define ILLEGAL_TOKEN 288
#define CHRCONST 289
#define INTCONST 290
#define TTAG 291
#define VAR 292
#define IDENT 293
#define TYPEIDENT 294
#define DOCBLOCK 295
#define STRATOM 296
#define TAGATOM 297
#define FLTCONST 298
#define opINTNAME 299
#define opINTNAMER 300
#define opMINUSMINUS 301
#define opPLUSPLUS 302
#define STMT_SEP 303
#define opSHL 304
#define opSHR 305
#define opPLASN 306
#define opMIASN 307
#define opMLASN 308
#define opDVASN 309
#define opMODASN 310
#define opANDASN 311
#define opXORASN 312
#define opORASN 313
#define opSLASN 314
#define opSRASN 315
#define opCAT 316
#define opEQ 317
#define opNE 318
#define opGE 319
#define opLE 320
#define opAND 321
#define opOR 322
#define opSTREQ 323
#define opCOLONCOLON 324
#define opNTASN 325
#define opNDASN 326
#define opMDASN 327
#define opSTRNE 328
#define UNARY 329




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 82 "CMDgram.y"
{
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
/* Line 1529 of yacc.c.  */
#line 215 "cmdgram.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE CMDlval;

