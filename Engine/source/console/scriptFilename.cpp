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
#include "console/scriptFilename.h"

#include "core/frameAllocator.h"
#include "core/tSimpleHashTable.h"
#include "core/strings/stringFunctions.h"
#include "core/stringTable.h"
#include "console/console.h"
#include "console/compiler.h"


namespace Con
{
//-----------------------------------------------------------------------------
// Local Globals
//-----------------------------------------------------------------------------

struct PathExpando
{
   StringTableEntry mPath;
   bool mIsToolsOnly;
};

static SimpleHashTable<PathExpando> sgPathExpandos(64, false);

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------

void setScriptPathExpando(const char *expando, const char *path, bool toolsOnly /*= false*/)
{
   PathExpando *exp = sgPathExpandos.retreive(expando);
   if(exp)
   {
      exp->mPath = StringTable->insert(path);
      exp->mIsToolsOnly = toolsOnly;
   }
   else
   {
      exp = new PathExpando;
      exp->mPath = StringTable->insert(path);
      exp->mIsToolsOnly = toolsOnly;
      sgPathExpandos.insert(exp, expando);
   }
}

void removeScriptPathExpando(const char *expando)
{
   PathExpando *exp = sgPathExpandos.remove(expando);
   if(exp)
      delete exp;
}

bool isScriptPathExpando(const char *expando)
{
   PathExpando *exp = sgPathExpandos.retreive(expando);
   return ( exp != NULL);
}

//-----------------------------------------------------------------------------

// [tom, 5/18/2006] FIXME: This needs some bounds checking
bool expandToolScriptFilename(char *filename, U32 size, const char *src)
{
   // [tom, 10/16/2006] Note: I am purposefully not early-outing here in the
   // same way the old code did as it is now possible that something could
   // be expanded if the name or mod is NULL. This was previously not the case.

   const StringTableEntry cbMod = CodeBlock::getCurrentCodeBlockModName();
   const StringTableEntry cbFullPath = CodeBlock::getCurrentCodeBlockFullPath();

   char varBuf[1024], modBuf[1024];
   const char *ptr = src;
   char *retPtr = filename;
   char *slash;

   const char *catPath = NULL;

#ifndef TORQUE_DEBUG
   bool isTools = Con::isCurrentScriptToolScript();
#endif

   // Check leading character
   switch(*ptr)
   {
      case '^':
         {
            // Variable
            const char *varPtr = ptr+1;
            char *insertPtr = varBuf;
            bool valid = true;
            while(*varPtr != '/')
            {
               if(*varPtr == 0)
               {
                  valid = false;
                  break;
               }
               *insertPtr++ = *varPtr++;
            }

            if(valid)
            {
               // Got a valid variable
               *insertPtr = 0;

               PathExpando *exp = sgPathExpandos.retreive(varBuf);
               
               if(exp == NULL)
               {
                  Con::errorf("expandScriptFilename - Ignoring invalid path expando \"%s\"", varBuf);
                  break;
               }

#ifndef TORQUE_DEBUG
               // [tom, 12/13/2006] This stops tools expandos from working in the console,
               // which is useful behavior when debugging so I'm ifdefing this out for debug builds.

               if(! isTools && exp->mIsToolsOnly)
               {
                  Con::errorf("expandScriptFilename - attempting to use tools only expando \"%s\" from outside of tools", varBuf);

                  *filename = 0;
                  return false;
               }
#endif

               catPath = exp ? exp->mPath : "";
               // swallow the expando and the slash after the expando
               ptr += dStrlen(varBuf) + 1;
               if(*ptr == '/')
                  ptr++;
            }
         }
         break;

      case '~':
         // Relative to mod
         if(cbMod && cbFullPath)
         {
            Platform::makeFullPathName(cbMod, modBuf, sizeof(modBuf));
            catPath = modBuf;
         }
         else
         {
            // Probably not a mod, so we'll use the ^game expando
            PathExpando *exp = sgPathExpandos.retreive("game");

            if(exp == NULL)
            {
               Con::errorf("expandScriptFilename - ~ expansion failed for mod and ^game when processing '%s'", src);
               break;
            }

            catPath = exp ? exp->mPath : "";
         }
         // swallow ~ and optional slash
         switch(ptr[1])
         {
            case '/': ptr += 2; break;
            default: ptr++;
         }
         
         break;

      case '.':
         // Relative to script directory
         if(cbFullPath)
         {
            dStrcpy(varBuf, cbFullPath);
            slash = dStrrchr(varBuf, '/');
            if(slash) *slash = 0;

            catPath = varBuf;
            
            // swallow dot and optional slash, but dont swallow .. relative path token
            switch(ptr[1])
            {
               case '.': break;
               case '/': ptr += 2; break;
               default: ptr++;
            }
         }
         break;
   }

   // [tom, 11/20/2006] Handing off to makeFullPathName() allows us to process .. correctly.
   Platform::makeFullPathName(ptr, retPtr, size, catPath);

   return true;
}

//-----------------------------------------------------------------------------

bool expandOldScriptFilename(char *filename, U32 size, const char *src)
{
   const StringTableEntry cbName = CodeBlock::getCurrentCodeBlockName();
   if (!cbName)
   {
      dStrcpy(filename, src);
      return true;
   }

   const char *slash;
   if (dStrncmp(src, "~/", 2) == 0)
      // tilde path means load from current codeblock/mod root
      slash = dStrchr(cbName, '/');
   else if (dStrncmp(src, "./", 2) == 0)
      // dot path means load from current codeblock/mod path
      slash = dStrrchr(cbName, '/');
   else if (dStrncmp(src, "^", 1) == 0)
   {
      Platform::makeFullPathName(src + 1, filename, size);
      return true;
   }
   else
   {
      // otherwise path must be fully specified
      if (dStrlen(src) > size)
      {
         Con::errorf("Buffer overflow attempting to expand filename: %s", src);
         *filename = 0;
         return false;
      }
      dStrcpy(filename, src);
      return true;
   }

   if (slash == NULL)
   {
      Con::errorf("Illegal CodeBlock path detected (no mod directory): %s", cbName);
      *filename = 0;
      return false;
   }

   U32 length = slash-cbName;
   if ((length+dStrlen(src)) > size)
   {
      Con::errorf("Buffer overflow attempting to expand filename: %s", src);
      *filename = 0;
      return false;
   }

   dStrncpy(filename, cbName, length);
   dStrcpy(filename+length, src+1);
   return true;
}

//-----------------------------------------------------------------------------

bool expandScriptFilename(char *filename, U32 size, const char *src)
{
#ifndef TORQUE2D_TOOLS_FIXME
   return expandOldScriptFilename(filename, size, src);
#else
   return expandToolScriptFilename(filename, size, src);
#endif
}

//-----------------------------------------------------------------------------

static StringTableEntry tryGetBasePath(const char *path, const char *base)
{
   U32 len = dStrlen(base);
   if(dStrnicmp(path, base, len) == 0)
   {
      if(*(path + len) == '/') --len;
      return StringTable->insertn(path, len, true);
   }
   return NULL;
}

struct CollapseTest
{
   const char *path;
   const char *replace;
};

bool collapseScriptFilename(char *filename, U32 size, const char *src)
{
   PathExpando *tools = sgPathExpandos.retreive("tools");
   PathExpando *game = sgPathExpandos.retreive("game");

   CollapseTest test[]=
   {
      { game ? game->mPath : NULL, "~" },
      { tools ? tools->mPath : NULL, "^tools" },
      { Platform::getCurrentDirectory(), "" },
      { Platform::getMainDotCsDir(), "" },
      { NULL, NULL }
   };

   for(S32 i = 0;!(test[i].path == NULL && test[i].replace == NULL);++i)
   {
      if(test[i].path == NULL) continue;

      StringTableEntry base = tryGetBasePath(src, test[i].path);
      if(base == NULL)
         continue;

      StringTableEntry rel = Platform::makeRelativePathName(src, test[i].path);
      
      *filename = 0;
      if(*test[i].replace)
         dSprintf(filename, size, "%s/", test[i].replace);
      dStrcat(filename, rel);
      return true;
   }

   dStrncpy(filename, src, size - 1);
   filename[size-1] = 0;
   return true;
}

//-----------------------------------------------------------------------------

} // end namespace Con

//-----------------------------------------------------------------------------
// Console Functions
//-----------------------------------------------------------------------------

ConsoleFunction(expandFilename, const char*, 2, 2, "(string filename)"
				"@brief Grabs the full path of a specified file\n\n"
				"@param filename Name of the local file to locate\n"
				"@return String containing the full filepath on disk\n"
				"@ingroup FileSystem")
{
   TORQUE_UNUSED(argc);
   static const U32 bufSize = 1024;
   char* ret = Con::getReturnBuffer( bufSize );
   Con::expandScriptFilename(ret, bufSize, argv[1]);
   return ret;
}

ConsoleFunction(expandOldFilename, const char*, 2, 2, "(string filename)"
				"@brief Retrofits a filepath that uses old Torque style\n\n"
				"@return String containing filepath with new formatting\n"
				"@ingroup FileSystem")
{
   TORQUE_UNUSED(argc);
   static const U32 bufSize = 1024;
   char* ret = Con::getReturnBuffer( bufSize );
   Con::expandOldScriptFilename(ret, bufSize, argv[1]);
   return ret;
}

//-----------------------------------------------------------------------------
// Tool Functions
//-----------------------------------------------------------------------------

ConsoleToolFunction(collapseFilename, const char*, 2, 2, "(string filename)"
					"@internal Editor use only")
{
   TORQUE_UNUSED(argc);
   static const U32 bufSize = 1024;
   char* ret = Con::getReturnBuffer( bufSize );
   Con::collapseScriptFilename(ret, bufSize, argv[1]);
   return ret;
}

ConsoleToolFunction(setScriptPathExpando, void, 3, 4, "(string expando, string path[, bool toolsOnly])"
					"@internal Editor use only")
{
   if(argc == 4)
      Con::setScriptPathExpando(argv[1], argv[2], dAtob(argv[3]));
   else
      Con::setScriptPathExpando(argv[1], argv[2]);
}

ConsoleToolFunction(removeScriptPathExpando, void, 2, 2, "(string expando)"
					"@internal Editor use only")
{
   Con::removeScriptPathExpando(argv[1]);
}

ConsoleToolFunction(isScriptPathExpando, bool, 2, 2, "(string expando)"
					"@internal Editor use only")
{
   return Con::isScriptPathExpando(argv[1]);
}
