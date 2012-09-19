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

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

//Includes
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _STREAM_H_
#include "core/stream/stream.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class SizedStream;

class Tokenizer
{
  public:
   enum
   {
      MaxTokenSize = 1023
   };

  private:
   char   mFileName[1024];

   char* mpBuffer;
   U32   mBufferSize;

   S32 mCurrPos;
   S32 mStartPos;

   bool mTokenIsQuoted;

   char   mCurrTokenBuffer[MaxTokenSize + 1];
   bool   mTokenIsCurrent;

   char*  mSingleTokens;

   Vector<U32> mLinePositions;

  public:
   Tokenizer();
   ~Tokenizer();

   bool openFile(const char* pFileName);
   bool openFile(Stream* pStream);
   void setBuffer(const char* buffer, U32 bufferSize);

   void setSingleTokens(const char* singleTokens);

   void buildLinePositions();

   bool        advanceToken(const bool crossLine, const bool assertAvailable = false);
   bool        regressToken(const bool crossLine);
   bool        tokenAvailable();

   const char* getToken() const;
   const char* getNextToken();
   bool tokenICmp(const char* pCmp) const;
   bool tokenIsQuoted() const { return mTokenIsQuoted; }

   bool findToken(const char* pCmp);
   bool findToken(U32 start, const char* pCmp);

   const char* getFileName() const     { return mFileName; }

   U32   getLinePosition(const U32 pos, U32 lowIndex = 0, S32 highIndex = -1);
   U32   getCurrentLine();
   U32   getTokenLineOffset();
   U32   getCurrentPos() const { return mCurrPos; }

   bool  setCurrentPos(U32 pos);

   // Resets our active data but leaves the buffer intact
   bool  reset();
   // Clears the buffer and resets the active data
   bool  clear();

   bool  endOfFile();
};


#endif //_TOKENIZER_H_
