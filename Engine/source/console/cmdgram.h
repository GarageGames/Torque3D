typedef union {
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
} YYSTYPE;
#define	rwDEFINE	258
#define	rwENDDEF	259
#define	rwDECLARE	260
#define	rwDECLARESINGLETON	261
#define	rwBREAK	262
#define	rwELSE	263
#define	rwCONTINUE	264
#define	rwGLOBAL	265
#define	rwIF	266
#define	rwNIL	267
#define	rwRETURN	268
#define	rwWHILE	269
#define	rwDO	270
#define	rwENDIF	271
#define	rwENDWHILE	272
#define	rwENDFOR	273
#define	rwDEFAULT	274
#define	rwFOR	275
#define	rwFOREACH	276
#define	rwFOREACHSTR	277
#define	rwIN	278
#define	rwDATABLOCK	279
#define	rwSWITCH	280
#define	rwCASE	281
#define	rwSWITCHSTR	282
#define	rwCASEOR	283
#define	rwPACKAGE	284
#define	rwNAMESPACE	285
#define	rwCLASS	286
#define	rwASSERT	287
#define	ILLEGAL_TOKEN	288
#define	CHRCONST	289
#define	INTCONST	290
#define	TTAG	291
#define	VAR	292
#define	IDENT	293
#define	TYPEIDENT	294
#define	DOCBLOCK	295
#define	STRATOM	296
#define	TAGATOM	297
#define	FLTCONST	298
#define	opINTNAME	299
#define	opINTNAMER	300
#define	opMINUSMINUS	301
#define	opPLUSPLUS	302
#define	STMT_SEP	303
#define	opSHL	304
#define	opSHR	305
#define	opPLASN	306
#define	opMIASN	307
#define	opMLASN	308
#define	opDVASN	309
#define	opMODASN	310
#define	opANDASN	311
#define	opXORASN	312
#define	opORASN	313
#define	opSLASN	314
#define	opSRASN	315
#define	opCAT	316
#define	opEQ	317
#define	opNE	318
#define	opGE	319
#define	opLE	320
#define	opAND	321
#define	opOR	322
#define	opSTREQ	323
#define	opCOLONCOLON	324
#define	opMDASN	325
#define	opNDASN	326
#define	opNTASN	327
#define	opSTRNE	328
#define	UNARY	329


extern YYSTYPE CMDlval;
