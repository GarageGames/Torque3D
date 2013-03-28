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

#include "core/tokenizer.h"
#include "platform/platform.h"
#include "core/stream/fileStream.h"
#include "core/strings/stringFunctions.h"
#include "core/util/safeDelete.h"

Tokenizer::Tokenizer()
{
   dMemset(mFileName, 0, sizeof(mFileName));

   mpBuffer    = NULL;
   mBufferSize = 0;

   mStartPos   = 0;
   mCurrPos    = 0;

   mTokenIsQuoted = false;

   dMemset(mCurrTokenBuffer, 0, sizeof(mCurrTokenBuffer));
   mTokenIsCurrent = false;

   mSingleTokens = NULL;

   VECTOR_SET_ASSOCIATION(mLinePositions);
}

Tokenizer::~Tokenizer()
{
   clear();
}

bool Tokenizer::openFile(const char* pFileName)
{
   AssertFatal(mFileName[0] == '\0', "Reuse of Tokenizers not allowed!");

   FileStream* pStream = new FileStream;
   if (pStream->open(pFileName, Torque::FS::File::Read) == false)
   {
      delete pStream;
      return false;
   }
   dStrcpy(mFileName, pFileName);

   mBufferSize = pStream->getStreamSize();
   mpBuffer    = new char[mBufferSize];
   pStream->read(mBufferSize, mpBuffer);
   pStream->close();
   delete pStream;

   reset();

   buildLinePositions();

   return true;
}

bool Tokenizer::openFile(Stream* pStream)
{
   mBufferSize = pStream->getStreamSize();
   mpBuffer    = new char[mBufferSize];
   pStream->read(mBufferSize, mpBuffer);

   reset();

   buildLinePositions();

   return true;
}

void Tokenizer::setBuffer(const char* buffer, U32 bufferSize)
{
   if (mpBuffer)
   {
      SAFE_DELETE_ARRAY(mpBuffer);
      mBufferSize = 0;
   }

   mBufferSize = bufferSize;
   mpBuffer    = new char[mBufferSize + 1];
   dStrcpy(mpBuffer, buffer);

   reset();

   buildLinePositions();
}

void Tokenizer::setSingleTokens(const char* singleTokens)
{
   if (mSingleTokens)
      SAFE_DELETE(mSingleTokens);

   if (singleTokens)
      mSingleTokens = dStrdup(singleTokens);
}

bool Tokenizer::reset()
{
   mStartPos   = 0;
   mCurrPos    = 0;

   mTokenIsQuoted = false;

   dMemset(mCurrTokenBuffer, 0, sizeof(mCurrTokenBuffer));
   mTokenIsCurrent = false;

   return true;
}

bool Tokenizer::clear()
{
   // Delete our buffer
   if (mpBuffer)
      SAFE_DELETE_ARRAY(mpBuffer);

   // Reset the buffer size
   mBufferSize = 0;

   // Reset our active data
   reset();

   // Clear our line positions
   mLinePositions.clear();

   // Reset our file name
   dMemset(mFileName, 0, 1024);

   // Wipe the single tokens
   setSingleTokens(NULL);

   return true;
}

bool Tokenizer::setCurrentPos(U32 pos)
{
   mCurrPos    = pos;
   mTokenIsCurrent = false;

   return advanceToken(true);
}

void Tokenizer::buildLinePositions()
{
   if (mBufferSize == 0)
      return;

   // We can safely assume that the first line is at position 0
   mLinePositions.push_back(0);

   U32 currPos = 0;
   while (currPos + 1 < mBufferSize)
   {
      // Windows line ending
      if (mpBuffer[currPos] == '\r' && mpBuffer[currPos + 1] == '\n')
      {
         currPos += 2;

         mLinePositions.push_back(currPos);
      }
      // Not sure if this ever happens but just in case
      else if (mpBuffer[currPos] == '\n' && mpBuffer[currPos + 1] == '\r')
      {
         currPos += 2;

         mLinePositions.push_back(currPos);
      }
      // Unix line endings should only have a single line break character
      else if (mpBuffer[currPos] == '\n' || mpBuffer[currPos] == '\r')
      {
         currPos++;

         mLinePositions.push_back(currPos);
      }
      else
         currPos++;
   }
}

U32 Tokenizer::getLinePosition(const U32 pos, U32 lowIndex, S32 highIndex)
{
   // If we have one or less lines then
   // the result is easy
   if (mLinePositions.size() <= 1)
      return 0;

   // Now that we know we have at least one position
   // we can do a quick test against the last line
   if (pos >= mLinePositions.last())
      return mLinePositions.size() - 1;

   // If this is the beginning of the search
   // set a good starting point (the middle)
   if (highIndex < 0)
      highIndex = mLinePositions.size() - 1;

   // Just in case bad values got handed in
   if (lowIndex > highIndex)
      lowIndex = highIndex;

   // Compute our test index (middle)
   U32 testIndex = (lowIndex + highIndex) / 2;

   // Make sure that our test indices are valid
   if (testIndex >= mLinePositions.size() ||
       testIndex + 1 >= mLinePositions.size())
      return mLinePositions.size() - 1;

   // See if we are already at the right line
   if (pos >= mLinePositions[testIndex] && pos < mLinePositions[testIndex + 1])
      return testIndex;

   if (pos < mLinePositions[testIndex])
      highIndex = testIndex;
   else
      lowIndex = testIndex;

   return getLinePosition(pos, lowIndex, highIndex);
}

U32 Tokenizer::getCurrentLine()
{
   // Binary search for the line number whose
   // position is equal to or lower than the
   // current position
   return getLinePosition(mStartPos);
}

U32 Tokenizer::getTokenLineOffset()
{
   U32 lineNumber = getCurrentLine();

   if (lineNumber >= mLinePositions.size())
      return 0;

   U32 linePosition = mLinePositions[lineNumber];

   if (linePosition >= mStartPos)
      return 0;

   return mStartPos - linePosition;
}

bool Tokenizer::advanceToken(const bool crossLine, const bool assertAvail)
{
   if (mTokenIsCurrent == true)
   {
      AssertFatal(mCurrTokenBuffer[0] != '\0', "No token, but marked as current?");
      mTokenIsCurrent = false;
      return true;
   }

   U32 currPosition = 0;
   mCurrTokenBuffer[0] = '\0';

   mTokenIsQuoted = false;

   // Store the beginning of the previous advance
   // and the beginning of the current advance
   mStartPos = mCurrPos;

   while (mCurrPos < mBufferSize)
   {
      char c = mpBuffer[mCurrPos];

      bool cont = true;

      if (mSingleTokens && dStrchr(mSingleTokens, c))
      {
         if (currPosition == 0)
         {
            mCurrTokenBuffer[currPosition++] = c;
            mCurrPos++;
            cont = false;
            break;
         }
         else
         {
            // End of token
            cont = false;
         }
      }
      else
      {
         switch (c)
         {
           case ' ':
           case '\t':
            if (currPosition == 0)
            {
               // Token hasn't started yet...
               mCurrPos++;
            }
            else
            {
               // End of token
               mCurrPos++;
               cont = false;
            }
            break;

           case '\r':
           case '\n':
            if (crossLine == true)
            {
               // Windows line ending
               if (mpBuffer[mCurrPos] == '\r' && mpBuffer[mCurrPos + 1] == '\n')
                  mCurrPos += 2;
               // Not sure if this ever happens but just in case
               else if (mpBuffer[mCurrPos] == '\n' && mpBuffer[mCurrPos + 1] == '\r')
                  mCurrPos += 2;
               // Unix line endings should only have a single line break character
               else
                  mCurrPos++;
            }
            else
            {
               cont = false;
               break;
            }
            break;

           default:
            if (c == '\"' || c == '\'')
            {
               // Quoted token
               U32 startLine = getCurrentLine();
               mCurrPos++;

               // Store the beginning of the token
               mStartPos = mCurrPos;

               while (mpBuffer[mCurrPos] != c)
               {
                  AssertISV(mCurrPos < mBufferSize,
                            avar("End of file before quote closed.  Quote started: (%s: %d)",
                                 getFileName(), startLine));
                  AssertISV((mpBuffer[mCurrPos] != '\n' && mpBuffer[mCurrPos] != '\r'),
                            avar("End of line reached before end of quote.  Quote started: (%s: %d)",
                                 getFileName(), startLine));

                  mCurrTokenBuffer[currPosition++] = mpBuffer[mCurrPos++];
               }

               mTokenIsQuoted = true;

               mCurrPos++;
               cont = false;
            }
            else if (c == '/' && mpBuffer[mCurrPos+1] == '/')
            {
               // Line quote...
               if (currPosition == 0)
               {
                  // continue to end of line, then let crossLine determine on the next pass
                  while (mCurrPos < mBufferSize && (mpBuffer[mCurrPos] != '\n' && mpBuffer[mCurrPos] != '\r'))
                     mCurrPos++;
               }
               else
               {
                  // This is the end of the token.  Continue to EOL
                  while (mCurrPos < mBufferSize && (mpBuffer[mCurrPos] != '\n' && mpBuffer[mCurrPos] != '\r'))
                     mCurrPos++;
                  cont = false;
               }
            }
            else if (c == '/' && mpBuffer[mCurrPos+1] == '*')
            {
               // Block quote...
               if (currPosition == 0)
               {
                  // continue to end of block, then let crossLine determine on the next pass
                  while (mCurrPos < mBufferSize - 1 && (mpBuffer[mCurrPos] != '*' || mpBuffer[mCurrPos + 1] != '/'))
                     mCurrPos++;

                  if (mCurrPos < mBufferSize - 1)
                     mCurrPos += 2;
               }
               else
               {
                  // This is the end of the token.  Continue to EOL
                  while (mCurrPos < mBufferSize - 1 && (mpBuffer[mCurrPos] != '*' || mpBuffer[mCurrPos + 1] != '/'))
                     mCurrPos++;

                  if (mCurrPos < mBufferSize - 1)
                     mCurrPos += 2;

                  cont = false;
               }
            }
            else
            {
               // If this is the first non-token character then store the
               // beginning of the token
               if (currPosition == 0)
                  mStartPos = mCurrPos;

               mCurrTokenBuffer[currPosition++] = c;
               mCurrPos++;
            }
            break;
         }
      }

      if (cont == false)
         break;
   }

   mCurrTokenBuffer[currPosition] = '\0';

   if (assertAvail == true)
      AssertISV(currPosition != 0, avar("Error parsing: %s at or around line: %d", getFileName(), getCurrentLine()));

   if (mCurrPos == mBufferSize)
      return false;

   return true;
}

bool Tokenizer::regressToken(const bool crossLine)
{
   if (mTokenIsCurrent == true)
   {
      AssertFatal(mCurrTokenBuffer[0] != '\0', "No token, but marked as current?");
      mTokenIsCurrent = false;
      return true;
   }

   U32 currPosition = 0;
   mCurrTokenBuffer[0] = '\0';

   mTokenIsQuoted = false;

   // Store the beginning of the previous advance
   // and the beginning of the current advance
   mCurrPos = mStartPos;

   // Back up to the first character of the previous token
   mStartPos--;

   while (mStartPos > 0)
   {
      char c = mpBuffer[mStartPos];

      bool cont = true;

      if (mSingleTokens && dStrchr(mSingleTokens, c))
      {
         if (currPosition == 0)
         {
            mCurrTokenBuffer[currPosition++] = c;
            mStartPos--;
            cont = false;
            break;
         }
         else
         {
            // End of token
            cont = false;
         }
      }
      else
      {
         switch (c)
         {
           case ' ':
           case '\t':
            if (currPosition == 0)
            {
               // Token hasn't started yet...
               mStartPos--;
            }
            else
            {
               // End of token
               mStartPos--;
               cont = false;
            }
            break;

           case '\r':
           case '\n':
            if (crossLine == true && currPosition == 0)
            {
               // Windows line ending
               if (mStartPos > 0 && mpBuffer[mStartPos] == '\r' && mpBuffer[mStartPos - 1] == '\n')
                  mStartPos -= 2;
               // Not sure if this ever happens but just in case
               else if (mStartPos > 0 && mpBuffer[mStartPos] == '\n' && mpBuffer[mStartPos - 1] == '\r')
                  mStartPos -= 2;
               // Unix line endings should only have a single line break character
               else
                  mStartPos--;
            }
            else
            {
               cont = false;
               break;
            }
            break;

           default:
            if (c == '\"' || c == '\'')
            {
               // Quoted token
               U32 endLine = getCurrentLine();
               mStartPos--;

               while (mpBuffer[mStartPos] != c)
               {
                  AssertISV(mStartPos < 0,
                            avar("Beginning of file reached before finding begin quote.  Quote ended: (%s: %d)",
                                 getFileName(), endLine));

                  mCurrTokenBuffer[currPosition++] = mpBuffer[mStartPos--];
               }

               mTokenIsQuoted = true;

               mStartPos--;
               cont = false;
            }
            else if (c == '/' && mStartPos > 0 && mpBuffer[mStartPos - 1] == '/')
            {
               // Line quote...
               // Clear out anything saved already
               currPosition = 0;

               mStartPos -= 2;
            }
            else
            {
               mCurrTokenBuffer[currPosition++] = c;
               mStartPos--;
            }
            break;
         }
      }

      if (cont == false)
         break;
   }

   mCurrTokenBuffer[currPosition] = '\0';

   // Reveres the token
   for (U32 i = 0; i < currPosition / 2; i++)
   {
      char c = mCurrTokenBuffer[i];
      mCurrTokenBuffer[i] = mCurrTokenBuffer[currPosition - i - 1];
      mCurrTokenBuffer[currPosition - i - 1] = c;
   }

   mStartPos++;

   if (mStartPos == mCurrPos)
      return false;

   return true;
}

bool Tokenizer::tokenAvailable()
{
   // Note: this implies that when advanceToken(false) fails, it must cap the
   //        token buffer.
   //
   return mCurrTokenBuffer[0] != '\0';
}

const char* Tokenizer::getToken() const
{
   return mCurrTokenBuffer;
}

const char* Tokenizer::getNextToken()
{
   advanceToken(true);

   return getToken();
}

bool Tokenizer::tokenICmp(const char* pCmp) const
{
   return dStricmp(mCurrTokenBuffer, pCmp) == 0;
}

bool Tokenizer::findToken(U32 start, const char* pCmp)
{
   // Move to the start
   setCurrentPos(start);

   // In case the first token is what we are looking for
   if (tokenICmp(pCmp))
      return true;

   // Loop through the file and see if the token exists
   while (advanceToken(true))
   {
      if (tokenICmp(pCmp))
         return true;
   }

   return false;
}

bool Tokenizer::findToken(const char* pCmp)
{
   return findToken(0, pCmp);
}

bool Tokenizer::endOfFile()
{
   if (mCurrPos < mBufferSize)
      return false;
   else
      return true;
}