%{

// flex --nounput -o CMDscan.cpp -P CMD CMDscan.l

#define YYLMAX 4096
#define YY_NO_UNISTD_H

#include <stdio.h>
#include "platform/platform.h"
#include "core/stringTable.h"
#include "console/console.h"
#include "console/compiler.h"
#include "console/dynamicTypes.h"
#include "core/strings/stringFunctions.h"

template< typename T >
struct Token
{
   T value;
   S32 lineNumber;
};

// Can't have ctors in structs used in unions, so we have this.
template< typename T >
inline Token< T > MakeToken( T value, U32 lineNumber )
{
   Token< T > result;
   result.value = value;
   result.lineNumber = lineNumber;
   return result;
}

#include "console/cmdgram.h"

using namespace Compiler;

#define YY_NEVER_INTERACTIVE 1

// Some basic parsing primitives...
static int Sc_ScanDocBlock();
static int Sc_ScanString(int ret);
static int Sc_ScanNum();
static int Sc_ScanVar();
static int Sc_ScanHex();
static int Sc_ScanIdent();

// Deal with debuggability of FLEX.
#ifdef TORQUE_DEBUG
#define FLEX_DEBUG 1
#else
#define FLEX_DEBUG 0
#endif

// Install our own input code...
#undef CMDgetc
int CMDgetc();

// Hack to make windows lex happy.
#ifndef isatty
inline int isatty(int) { return 0; }
#endif

// Wrap our getc, so that lex doesn't try to do its own buffering/file IO.
#define YY_INPUT(buf,result,max_size) \
   { \
      int c = '*', n; \
      for ( n = 0; n < max_size && \
            (c = CMDgetc()) != EOF && c != '\n'; ++n ) \
         buf[n] = (char) c; \
      if ( c == '\n' ) \
         buf[n++] = (char) c; \
      result = n; \
   }
   
// General helper stuff.
static int lineIndex;

// File state
void CMDSetScanBuffer(const char *sb, const char *fn);
const char * CMDgetFileLine(int &lineNumber);

// Error reporting
void CMDerror(char * s, ...);

// Reset the parser.
void CMDrestart(FILE *in);

%}

DIGIT    [0-9]
INTEGER  {DIGIT}+
FLOAT    ({INTEGER}?\.{INTEGER})|({INTEGER}(\.{INTEGER})?[eE][+-]?{INTEGER})
LETTER   [A-Za-z_]
FILECHAR [A-Za-z_\.]
VARMID   [:A-Za-z0-9_]
IDTAIL   [A-Za-z0-9_]
VARTAIL  {VARMID}*{IDTAIL}
VAR      [$%]{LETTER}{VARTAIL}*
ID       {LETTER}{IDTAIL}*
ILID     [$%]{DIGIT}+{LETTER}{VARTAIL}*
FILENAME {FILECHAR}+
SPACE    [ \t\v\f]
HEXDIGIT [a-fA-F0-9]

%%
         ;
{SPACE}+ { }
("///"([^/\n\r][^\n\r]*)?[\n\r]+)+ { return(Sc_ScanDocBlock()); }
"//"[^\n\r]*   ;
[\r]        ;
[\n]        {lineIndex++;}
\"(\\.|[^\\"\n\r])*\"      { return(Sc_ScanString(STRATOM)); }
\'(\\.|[^\\'\n\r])*\'      { return(Sc_ScanString(TAGATOM)); }
"=="        { CMDlval.i = MakeToken< int >( opEQ, lineIndex ); return opEQ; }
"!="        { CMDlval.i = MakeToken< int >( opNE, lineIndex ); return opNE; }
">="        { CMDlval.i = MakeToken< int >( opGE, lineIndex ); return opGE; }
"<="        { CMDlval.i = MakeToken< int >( opLE, lineIndex ); return opLE; }
"&&"        { CMDlval.i = MakeToken< int >( opAND, lineIndex ); return opAND; }
"||"        { CMDlval.i = MakeToken< int >( opOR, lineIndex ); return opOR; }
"::"        { CMDlval.i = MakeToken< int >( opCOLONCOLON, lineIndex ); return opCOLONCOLON; }
"--"        { CMDlval.i = MakeToken< int >( opMINUSMINUS, lineIndex ); return opMINUSMINUS; }
"++"        { CMDlval.i = MakeToken< int >( opPLUSPLUS, lineIndex ); return opPLUSPLUS; }
"$="        { CMDlval.i = MakeToken< int >( opSTREQ, lineIndex ); return opSTREQ; }
"!$="       { CMDlval.i = MakeToken< int >( opSTRNE, lineIndex ); return opSTRNE; }
"<<"        { CMDlval.i = MakeToken< int >( opSHL, lineIndex ); return opSHL; }
">>"        { CMDlval.i = MakeToken< int >( opSHR, lineIndex ); return opSHR; }
"+="        { CMDlval.i = MakeToken< int >( opPLASN, lineIndex ); return opPLASN; }
"-="        { CMDlval.i = MakeToken< int >( opMIASN, lineIndex ); return opMIASN; }
"*="        { CMDlval.i = MakeToken< int >( opMLASN, lineIndex ); return opMLASN; }
"/="        { CMDlval.i = MakeToken< int >( opDVASN, lineIndex ); return opDVASN; }
"%="        { CMDlval.i = MakeToken< int >( opMODASN, lineIndex ); return opMODASN; }
"&="        { CMDlval.i = MakeToken< int >( opANDASN, lineIndex ); return opANDASN; }
"^="        { CMDlval.i = MakeToken< int >( opXORASN, lineIndex ); return opXORASN; }
"|="        { CMDlval.i = MakeToken< int >( opORASN, lineIndex ); return opORASN; }
"<<="       { CMDlval.i = MakeToken< int >( opSLASN, lineIndex ); return opSLASN; }
">>="       { CMDlval.i = MakeToken< int >( opSRASN, lineIndex ); return opSRASN; }
"->"        { CMDlval.i = MakeToken< int >( opINTNAME, lineIndex ); return opINTNAME; }
"-->"       { CMDlval.i = MakeToken< int >( opINTNAMER, lineIndex ); return opINTNAMER; }
"NL"        { CMDlval.i = MakeToken< int >( '\n', lineIndex ); return '@'; }
"TAB"       { CMDlval.i = MakeToken< int >( '\t', lineIndex ); return '@'; }
"SPC"       { CMDlval.i = MakeToken< int >( ' ', lineIndex ); return '@'; }
"@"         { CMDlval.i = MakeToken< int >( 0, lineIndex ); return '@'; }
"/*" { /* this comment stops syntax highlighting from getting messed up when editing the lexer in TextPad */
         int c = 0, l;
         for ( ; ; )
         {
            l = c;
            c = yyinput();

            // Is this an open comment?
            if ( c == EOF )
            {
               CMDerror( "unexpected end of file found in comment" );
               break;
            }

            // Increment line numbers.
            else if ( c == '\n' )
               lineIndex++;

            // Did we find the end of the comment?
            else if ( l == '*' && c == '/' )
               break;
         }
      }
"?" |
"[" |
"]" |
"(" |
")" |
"+" |
"-" |
"*" |
"/" |
"<" |
">" |
"|" |
"." |
"!" |
":" |
";" |
"{" |
"}" |
"," |
"&" |
"%" |
"^" |
"~" |
"="         { CMDlval.i = MakeToken< int >( CMDtext[ 0 ], lineIndex ); return CMDtext[ 0 ]; }
"in"        { CMDlval.i = MakeToken< int >( rwIN, lineIndex ); return(rwIN); }
"or"        { CMDlval.i = MakeToken< int >( rwCASEOR, lineIndex ); return(rwCASEOR); }
"break"     { CMDlval.i = MakeToken< int >( rwBREAK, lineIndex ); return(rwBREAK); }
"return"    { CMDlval.i = MakeToken< int >( rwRETURN, lineIndex ); return(rwRETURN); }
"else"      { CMDlval.i = MakeToken< int >( rwELSE, lineIndex ); return(rwELSE); }
"assert"    { CMDlval.i = MakeToken< int >( rwASSERT, lineIndex ); return(rwASSERT); }
"while"     { CMDlval.i = MakeToken< int >( rwWHILE, lineIndex ); return(rwWHILE); }
"do"        { CMDlval.i = MakeToken< int >( rwDO, lineIndex ); return(rwDO); }
"if"        { CMDlval.i = MakeToken< int >( rwIF, lineIndex ); return(rwIF); }
"foreach$"  { CMDlval.i = MakeToken< int >( rwFOREACHSTR, lineIndex ); return(rwFOREACHSTR); }
"foreach"   { CMDlval.i = MakeToken< int >( rwFOREACH, lineIndex ); return(rwFOREACH); }
"for"       { CMDlval.i = MakeToken< int >( rwFOR, lineIndex ); return(rwFOR); }
"continue"  { CMDlval.i = MakeToken< int >( rwCONTINUE, lineIndex ); return(rwCONTINUE); }
"function"  { CMDlval.i = MakeToken< int >( rwDEFINE, lineIndex ); return(rwDEFINE); }
"new"       { CMDlval.i = MakeToken< int >( rwDECLARE, lineIndex ); return(rwDECLARE); }
"singleton" { CMDlval.i = MakeToken< int >( rwDECLARESINGLETON, lineIndex ); return(rwDECLARESINGLETON); }
"datablock" { CMDlval.i = MakeToken< int >( rwDATABLOCK, lineIndex ); return(rwDATABLOCK); }
"case"      { CMDlval.i = MakeToken< int >( rwCASE, lineIndex ); return(rwCASE); }
"switch$"   { CMDlval.i = MakeToken< int >( rwSWITCHSTR, lineIndex ); return(rwSWITCHSTR); }
"switch"    { CMDlval.i = MakeToken< int >( rwSWITCH, lineIndex ); return(rwSWITCH); }
"default"   { CMDlval.i = MakeToken< int >( rwDEFAULT, lineIndex ); return(rwDEFAULT); }
"package"   { CMDlval.i = MakeToken< int >( rwPACKAGE, lineIndex ); return(rwPACKAGE); }
"namespace" { CMDlval.i = MakeToken< int >( rwNAMESPACE, lineIndex ); return(rwNAMESPACE); }
"true"      { CMDlval.i = MakeToken< int >( 1, lineIndex ); return INTCONST; }
"false"     { CMDlval.i = MakeToken< int >( 0, lineIndex ); return INTCONST; }
{VAR}       { return(Sc_ScanVar()); }
{ID}        { return Sc_ScanIdent(); }
0[xX]{HEXDIGIT}+ return(Sc_ScanHex());
{INTEGER}   { CMDtext[CMDleng] = 0; CMDlval.i = MakeToken< int >( dAtoi(CMDtext), lineIndex ); return INTCONST; }
{FLOAT}     return Sc_ScanNum();
{ILID}      return(ILLEGAL_TOKEN);
.           return(ILLEGAL_TOKEN);
%%

static const char *scanBuffer;
static const char *fileName;
static int scanIndex;

const char * CMDGetCurrentFile()
{
   return fileName;
}

int CMDGetCurrentLine()
{
   return lineIndex;
}

extern bool gConsoleSyntaxError;

void CMDerror(char *format, ...)
{
   Compiler::gSyntaxError = true;

   const int BUFMAX = 1024;
   char tempBuf[BUFMAX];
   va_list args;
   va_start( args, format );
#ifdef TORQUE_OS_WIN
   _vsnprintf( tempBuf, BUFMAX, format, args );
#else
   vsnprintf( tempBuf, BUFMAX, format, args );
#endif
    va_end(args);

   if(fileName)
   {
      Con::errorf(ConsoleLogEntry::Script, "%s Line: %d - %s", fileName, lineIndex, tempBuf);

#ifndef NO_ADVANCED_ERROR_REPORT
      // dhc - lineIndex is bogus.  let's try to add some sanity back in.
      int i,j,n;
      char c;
      int linediv = 1;
      // first, walk the buffer, trying to detect line ending type.
      // this is imperfect, if inconsistant line endings exist...
      for (i=0; i<scanIndex; i++)
      {
         c = scanBuffer[i];
         if (c=='\r' && scanBuffer[i+1]=='\n') linediv = 2; // crlf detected
         if (c=='\r' || c=='\n' || c==0) break; // enough for us to stop.
      }
      // grab some of the chars starting at the error location - lineending.
      i = 1; j = 0; n = 1;
      // find prev lineending
      while (n<BUFMAX-8 && i<scanIndex) // cap at file start
      {
         c = scanBuffer[scanIndex-i];
         if ((c=='\r' || c=='\n') && i>BUFMAX>>2) break; // at least get a little data
         n++; i++;
      }
      // find next lineending
      while (n<BUFMAX-8 && j<BUFMAX>>1) // cap at half-buf-size forward
      {
         c = scanBuffer[scanIndex+j];
         if (c==0) break;
         if ((c=='\r' || c=='\n') && j>BUFMAX>>2) break; // at least get a little data
         n++; j++;
      }
      if (i) i--; // chop off extra linefeed.
      if (j) j--; // chop off extra linefeed.
      // build our little text block
      if (i) dStrncpy(tempBuf,scanBuffer+scanIndex-i,i);
      dStrncpy(tempBuf+i,"##", 2); // bracketing.
      tempBuf[i+2] = scanBuffer[scanIndex]; // copy the halt character.
      dStrncpy(tempBuf+i+3,"##", 2); // bracketing.
      if (j) dStrncpy(tempBuf+i+5,scanBuffer+scanIndex+1,j); // +1 to go past current char.
      tempBuf[i+j+5] = 0; // null terminate
      for(n=0; n<i+j+5; n++) // convert CR to LF if alone...
         if (tempBuf[n]=='\r' && tempBuf[n+1]!='\n') tempBuf[n] = '\n';
      // write out to console the advanced error report
      Con::warnf(ConsoleLogEntry::Script, ">>> Advanced script error report.  Line %d.", lineIndex);
      Con::warnf(ConsoleLogEntry::Script, ">>> Some error context, with ## on sides of error halt:");
      Con::errorf(ConsoleLogEntry::Script, "%s", tempBuf);
      Con::warnf(ConsoleLogEntry::Script, ">>> Error report complete.\n");
#endif

      // Update the script-visible error buffer.
      const char *prevStr = Con::getVariable("$ScriptError");
      if (prevStr[0])
         dSprintf(tempBuf, sizeof(tempBuf), "%s\n%s Line: %d - Syntax error.", prevStr, fileName, lineIndex);
      else
         dSprintf(tempBuf, sizeof(tempBuf), "%s Line: %d - Syntax error.", fileName, lineIndex);
      Con::setVariable("$ScriptError", tempBuf);

      // We also need to mark that we came up with a new error.
      static S32 sScriptErrorHash=1000;
      Con::setIntVariable("$ScriptErrorHash", sScriptErrorHash++);
   }
   else
      Con::errorf(ConsoleLogEntry::Script, tempBuf);
}

void CMDSetScanBuffer(const char *sb, const char *fn)
{
   scanBuffer = sb;
   fileName = fn;
   scanIndex = 0;
   lineIndex = 1;
}

int CMDgetc()
{
   int ret = scanBuffer[scanIndex];
   if(ret)
      scanIndex++;
   else
      ret = -1;
   return ret;
}

int CMDwrap()
{
   return 1;
}

static int Sc_ScanVar()
{
   // Truncate the temp buffer...
   CMDtext[CMDleng] = 0;

   // Make it a stringtable string!
   CMDlval.s = MakeToken< StringTableEntry >( StringTable->insert(CMDtext), lineIndex );
   return(VAR);
}

static int charConv(int in)
{
   switch(in)
   {
      case 'r':
         return '\r';
      case 'n':
         return '\n';
      case 't':
         return '\t';
      default:
         return in;
   }
}

static int getHexDigit(char c)
{
   if(c >= '0' && c <= '9')
      return c - '0';
   if(c >= 'A' && c <= 'F')
      return c - 'A' + 10;
   if(c >= 'a' && c <= 'f')
      return c - 'a' + 10;
   return -1;
}

static int Sc_ScanDocBlock()
{
   S32 len = dStrlen(CMDtext);
   char* text = (char *) consoleAlloc(len + 1);
   S32 line = lineIndex;

   for( S32 i = 0, j = 0; j <= len; j++ )
   {
      if( ( j <= (len - 2) ) && ( CMDtext[j] == '/' ) && ( CMDtext[j + 1] == '/' ) && ( CMDtext[j + 2] == '/' ) )
      {
         j += 2;
         continue;
      }

      if( CMDtext[j] == '\r' )
         continue;

      if( CMDtext[j] == '\n' )
         lineIndex++;

      text[i++] = CMDtext[j];
   }

   CMDlval.str = MakeToken< char* >( text, line );
   return(DOCBLOCK);
}

static int Sc_ScanString(int ret)
{
   CMDtext[CMDleng - 1] = 0;
   if(!collapseEscape(CMDtext+1))
      return -1;
      
   char* buffer = ( char* ) consoleAlloc( dStrlen( CMDtext ) );
   dStrcpy( buffer, CMDtext + 1 );
   
   CMDlval.str = MakeToken< char* >( buffer, lineIndex );
   return ret;
}

static int Sc_ScanIdent()
{
   ConsoleBaseType *type;

   CMDtext[CMDleng] = 0;

   if((type = ConsoleBaseType::getTypeByName(CMDtext)) != NULL)
   {
      /* It's a type */
      CMDlval.i = MakeToken< int >( type->getTypeID(), lineIndex );
      return TYPEIDENT;
   }

   /* It's an identifier */
   CMDlval.s = MakeToken< StringTableEntry >( StringTable->insert(CMDtext), lineIndex );
   return IDENT;
}

void expandEscape(char *dest, const char *src)
{
   U8 c;
   while((c = (U8) *src++) != 0)
   {
      if(c == '\"')
      {
         *dest++ = '\\';
         *dest++ = '\"';
      }
      else if(c == '\\')
      {
         *dest++ = '\\';
         *dest++ = '\\';
      }
      else if(c == '\r')
      {
         *dest++ = '\\';
         *dest++ = 'r';
      }
      else if(c == '\n')
      {
         *dest++ = '\\';
         *dest++ = 'n';
      }
      else if(c == '\t')
      {
         *dest++ = '\\';
         *dest++ = 't';
      }
      else if(c == '\'')
      {
         *dest++ = '\\';
         *dest++ = '\'';
      }
      else if((c >= 1 && c <= 7) ||
              (c >= 11 && c <= 12) ||
              (c >= 14 && c <= 15))
      {
        /*  Remap around: \b = 0x8, \t = 0x9, \n = 0xa, \r = 0xd */
        static U8 expandRemap[15] = { 0x0,
                                        0x0,
                                        0x1,
                                        0x2,
                                        0x3,
                                        0x4,
                                        0x5,
                                        0x6,
                                        0x0,
                                        0x0,
                                        0x0,
                                        0x7,
                                        0x8,
                                        0x0,
                                        0x9 };

         *dest++ = '\\';
         *dest++ = 'c';
         if(c == 15)
            *dest++ = 'r';
         else if(c == 16)
            *dest++ = 'p';
         else if(c == 17)
            *dest++ = 'o';
         else
            *dest++ = expandRemap[c] + '0';
      }
      else if(c < 32)
      {
         *dest++ = '\\';
         *dest++ = 'x';
         S32 dig1 = c >> 4;
         S32 dig2 = c & 0xf;
         if(dig1 < 10)
            dig1 += '0';
         else
            dig1 += 'A' - 10;
         if(dig2 < 10)
            dig2 += '0';
         else
            dig2 += 'A' - 10;
         *dest++ = dig1;
         *dest++ = dig2;
      }
      else
         *dest++ = c;
   }
   *dest = '\0';
}

bool collapseEscape(char *buf)
{
   S32 len = dStrlen(buf) + 1;
   for(S32 i = 0; i < len;)
   {
      if(buf[i] == '\\')
      {
         if(buf[i+1] == 'x')
         {
            S32 dig1 = getHexDigit(buf[i+2]);
            if(dig1 == -1)
               return false;

            S32 dig2 = getHexDigit(buf[i+3]);
            if(dig2 == -1)
               return false;
            buf[i] = dig1 * 16 + dig2;
            dMemmove(buf + i + 1, buf + i + 4, len - i - 3);
            len -= 3;
            i++;
         }
         else if(buf[i+1] == 'c')
         {
            /*  Remap around: \b = 0x8, \t = 0x9, \n = 0xa, \r = 0xd */
            static U8 collapseRemap[10] = { 0x1,
                                              0x2,
                                              0x3,
                                              0x4,
                                              0x5,
                                              0x6,
                                              0x7,
                                              0xb,
                                              0xc,
                                              0xe };

            if(buf[i+2] == 'r')
                buf[i] = 15;
            else if(buf[i+2] == 'p')
               buf[i] = 16;
            else if(buf[i+2] == 'o')
               buf[i] = 17;
            else
            {
                int dig1 = buf[i+2] - '0';
                if(dig1 < 0 || dig1 > 9)
                   return false;
                buf[i] = collapseRemap[dig1];
            }
            // Make sure we don't put 0x1 at the beginning of the string.
            if ((buf[i] == 0x1) && (i == 0))
            {
               buf[i] = 0x2;
               buf[i+1] = 0x1;
               dMemmove(buf + i + 2, buf + i + 3, len - i - 1);
               len -= 1;
            }
            else
            {
               dMemmove(buf + i + 1, buf + i + 3, len - i - 2);
               len -= 2;
            }
            i++;
         }
         else
         {
            buf[i] = charConv(buf[i+1]);
            dMemmove(buf + i + 1, buf + i + 2, len - i - 1);
            len--;
            i++;
         }
      }
      else
         i++;
   }
   return true;
}

static int Sc_ScanNum()
{
   CMDtext[CMDleng] = 0;
   CMDlval.f = MakeToken< double >( dAtof(CMDtext), lineIndex );
   return(FLTCONST);
}

static int Sc_ScanHex()
{
   S32 val = 0;
   dSscanf(CMDtext, "%x", &val);
   CMDlval.i = MakeToken< int >( val, lineIndex );
   return INTCONST;
}

void CMD_reset()
{
   CMDrestart(NULL);
}
