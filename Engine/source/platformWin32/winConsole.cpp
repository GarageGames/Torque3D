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

#include "core/util/rawData.h"
#include "core/strings/stringFunctions.h"
#include "core/strings/unicode.h"

#include "platformWin32/platformWin32.h"
#include "platformWin32/winConsole.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "core/util/journal/process.h"


WinConsole *WindowsConsole = NULL;

namespace Con
{
   extern bool alwaysUseDebugOutput;
}

DefineConsoleFunction( enableWinConsole, void, (bool flag), , "enableWinConsole(bool);")
{
   WindowsConsole->enable(flag);
}

void WinConsole::create()
{
   if( !WindowsConsole )
      WindowsConsole = new WinConsole();
}

void WinConsole::destroy()
{
   if( WindowsConsole )
      delete WindowsConsole;
   WindowsConsole = NULL;
}

void WinConsole::enable(bool enabled)
{
   winConsoleEnabled = enabled;
   if(winConsoleEnabled)
   {
      AllocConsole();
      const char *title = Con::getVariable("Con::WindowTitle");
      if (title && *title)
      {
#ifdef UNICODE
         UTF16 buf[512];
         convertUTF8toUTF16((UTF8 *)title, buf);
         SetConsoleTitle(buf);
#else
         SetConsoleTitle(title);
#endif
      }
      stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
      stdIn  = GetStdHandle(STD_INPUT_HANDLE);
      stdErr = GetStdHandle(STD_ERROR_HANDLE);

      printf("%s", Con::getVariable("Con::Prompt"));
   }
}

bool WinConsole::isEnabled()
{
   if ( WindowsConsole )
      return WindowsConsole->winConsoleEnabled;

   return false;
}

static void winConsoleConsumer(U32 level, const char *line)
{
   if (WindowsConsole)
   {
      WindowsConsole->processConsoleLine(line);
#ifndef TORQUE_SHIPPING
      // see console.cpp for a description of Con::alwaysUseDebugOutput
      if( level == ConsoleLogEntry::Error  || Con::alwaysUseDebugOutput)
      {
         // [rene, 04/05/2008] This is incorrect.  Should do conversion from UTF8 here.
         //    Skipping for the sake of speed.  Not meant to be seen by user anyway.
         OutputDebugStringA( line );
         OutputDebugStringA( "\n" );
      }
#endif
   }
}

WinConsole::WinConsole()
{
   for (S32 iIndex = 0; iIndex < MAX_CMDS; iIndex ++)
      rgCmds[iIndex][0] = '\0';

   iCmdIndex = 0;
   winConsoleEnabled = false;
   Con::addConsumer(winConsoleConsumer);
   inpos = 0;
   lineOutput = false;

   Process::notify(this, &WinConsole::process, PROCESS_LAST_ORDER);
}

WinConsole::~WinConsole()
{
   Process::remove(this, &WinConsole::process);
   Con::removeConsumer(winConsoleConsumer);
}

void WinConsole::printf(const char *s, ...)
{
   // Get the line into a buffer.
   static const S32 BufSize = 4096;
   static char buffer[4096];
   DWORD bytes;
   va_list args;
   va_start(args, s);
   _vsnprintf(buffer, BufSize, s, args);
   // Replace tabs with carats, like the "real" console does.
   char *pos = buffer;
   while (*pos) {
      if (*pos == '\t') {
         *pos = '^';
      }
      pos++;
   }
   // Axe the color characters.
   Con::stripColorChars(buffer);
   // Print it.
   WriteFile(stdOut, buffer, strlen(buffer), &bytes, NULL);
   FlushFileBuffers( stdOut );
}

void WinConsole::processConsoleLine(const char *consoleLine)
{
   if(winConsoleEnabled)
   {
      inbuf[inpos] = 0;
      if(lineOutput)
         printf("%s\n", consoleLine);
      else
         printf("%c%s\n%s%s", '\r', consoleLine, Con::getVariable("Con::Prompt"), inbuf);
   }
}

void WinConsole::process()
{
   if(winConsoleEnabled)
   {
      DWORD numEvents;
      GetNumberOfConsoleInputEvents(stdIn, &numEvents);
      if(numEvents)
      {
         INPUT_RECORD rec[20];
         char outbuf[512];
         S32 outpos = 0;

         ReadConsoleInput(stdIn, rec, 20, &numEvents);
         DWORD i;
         for(i = 0; i < numEvents; i++)
         {
            if(rec[i].EventType == KEY_EVENT)
            {
               KEY_EVENT_RECORD *ke = &(rec[i].Event.KeyEvent);
               if(ke->bKeyDown)
               {
                  switch (ke->uChar.AsciiChar)
                  {
                     // If no ASCII char, check if it's a handled virtual key
                     case 0:
                        switch (ke->wVirtualKeyCode)
                        {
                           // UP ARROW
                           case 0x26 :
                              // Go to the previous command in the cyclic array
                              if ((-- iCmdIndex) < 0)
                                 iCmdIndex = MAX_CMDS - 1;

                              // If this command isn't empty ...
                              if (rgCmds[iCmdIndex][0] != '\0')
                              {
                                 // Obliterate current displayed text
                                 for (S32 i = outpos = 0; i < inpos; i ++)
                                 {
                                    outbuf[outpos ++] = '\b';
                                    outbuf[outpos ++] = ' ';
                                    outbuf[outpos ++] = '\b';
                                 }

                                 // Copy command into command and display buffers
                                 for (inpos = 0; inpos < (S32)strlen(rgCmds[iCmdIndex]); inpos ++, outpos ++)
                                 {
                                    outbuf[outpos] = rgCmds[iCmdIndex][inpos];
                                    inbuf [inpos ] = rgCmds[iCmdIndex][inpos];
                                 }
                              }
                              // If previous is empty, stay on current command
                              else if ((++ iCmdIndex) >= MAX_CMDS)
                              {
                                 iCmdIndex = 0;
                              }

                              break;

                           // DOWN ARROW
                           case 0x28 : {
                              // Go to the next command in the command array, if
                              // it isn't empty
                              if (rgCmds[iCmdIndex][0] != '\0' && (++ iCmdIndex) >= MAX_CMDS)
                                  iCmdIndex = 0;

                              // Obliterate current displayed text
                              for (S32 i = outpos = 0; i < inpos; i ++)
                              {
                                 outbuf[outpos ++] = '\b';
                                 outbuf[outpos ++] = ' ';
                                 outbuf[outpos ++] = '\b';
                              }

                              // Copy command into command and display buffers
                              for (inpos = 0; inpos < (S32)strlen(rgCmds[iCmdIndex]); inpos ++, outpos ++)
                              {
                                 outbuf[outpos] = rgCmds[iCmdIndex][inpos];
                                 inbuf [inpos ] = rgCmds[iCmdIndex][inpos];
                              }
                              }
                              break;

                           // LEFT ARROW
                           case 0x25 :
                              break;

                           // RIGHT ARROW
                           case 0x27 :
                              break;

                           default :
                              break;
                        }
                        break;
                     case '\b':
                        if(inpos > 0)
                        {
                           outbuf[outpos++] = '\b';
                           outbuf[outpos++] = ' ';
                           outbuf[outpos++] = '\b';
                           inpos--;
                        }
                        break;
                     case '\t':
                        // In the output buffer, we're going to have to erase the current line (in case
                        // we're cycling through various completions) and write out the whole input
                        // buffer, so (inpos * 3) + complen <= 512.  Should be OK.  The input buffer is
                        // also 512 chars long so that constraint will also be fine for the input buffer.
                        {
                           // Erase the current line.
                           U32 i;
                           for (i = 0; i < inpos; i++) {
                              outbuf[outpos++] = '\b';
                              outbuf[outpos++] = ' ';
                              outbuf[outpos++] = '\b';
                           }
                           // Modify the input buffer with the completion.
                           U32 maxlen = 512 - (inpos * 3);
                           if (ke->dwControlKeyState & SHIFT_PRESSED) {
                              inpos = Con::tabComplete(inbuf, inpos, maxlen, false);
                           }
                           else {
                              inpos = Con::tabComplete(inbuf, inpos, maxlen, true);
                           }
                           // Copy the input buffer to the output.
                           for (i = 0; i < inpos; i++) {
                              outbuf[outpos++] = inbuf[i];
                           }
                        }
                        break;
                     case '\n':
                     case '\r':
                        outbuf[outpos++] = '\r';
                        outbuf[outpos++] = '\n';

                        inbuf[inpos] = 0;
                        outbuf[outpos] = 0;
                        printf("%s", outbuf);

                        // Pass the line along to the console for execution.
                        {
                           RawData rd;
                           rd.size = inpos + 1;
                           rd.data = ( S8* ) inbuf;

                           Con::smConsoleInput.trigger(rd);
                        }

                        // If we've gone off the end of our array, wrap
                        // back to the beginning
                        if (iCmdIndex >= MAX_CMDS)
                            iCmdIndex %= MAX_CMDS;

                        // Put the new command into the array
                        strcpy(rgCmds[iCmdIndex ++], inbuf);

                        printf("%s", Con::getVariable("Con::Prompt"));
                        inpos = outpos = 0;
                        break;
                     default:
                        inbuf[inpos++] = ke->uChar.AsciiChar;
                        outbuf[outpos++] = ke->uChar.AsciiChar;
                        break;
                  }
               }
            }
         }
         if(outpos)
         {
            outbuf[outpos] = 0;
            printf("%s", outbuf);
         }
      }
   }
}
