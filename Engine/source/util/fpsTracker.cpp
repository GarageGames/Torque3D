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

#include "util/fpsTracker.h"
#include "console/console.h"

FPSTracker gFPS;

FPSTracker::FPSTracker()
{
   mUpdateInterval = 0.25f;
   reset();
}

void FPSTracker::reset()
{
   fpsNext         = (F32)Platform::getRealMilliseconds()/1000.0f + mUpdateInterval;

   fpsRealLast       = 0.0f;
   fpsReal           = 0.0f;
   fpsRealMin        = 0.000001f; // Avoid division by zero.
   fpsRealMax        = 1.0f;
   fpsVirtualLast    = 0.0f;
   fpsVirtual        = 0.0f;
   fpsFrames         = 0;
}

void FPSTracker::update()
{
   const float alpha  = 0.07f;
   F32 realSeconds    = (F32)Platform::getRealMilliseconds()/1000.0f;
   F32 virtualSeconds = (F32)Platform::getVirtualMilliseconds()/1000.0f;

   fpsFrames++;
   if (fpsFrames > 1)
   {
      fpsReal    = fpsReal*(1.0-alpha) + (realSeconds-fpsRealLast)*alpha;
      fpsVirtual = fpsVirtual*(1.0-alpha) + (virtualSeconds-fpsVirtualLast)*alpha;

      if( fpsFrames > 10 ) // Wait a few frames before updating these.
      {
         // Update min/max.  This is a bit counter-intuitive, as the comparisons are
         // inversed because these are all one-over-x values.

         if( fpsReal > fpsRealMin )
            fpsRealMin = fpsReal;
         if( fpsReal < fpsRealMax )
            fpsRealMax = fpsReal;
      }
   }

   fpsRealLast    = realSeconds;
   fpsVirtualLast = virtualSeconds;

   // update variables every few frames
   F32 update = fpsRealLast - fpsNext;
   if (update > 0.5f)
   {
      Con::setVariable( "fps::real",      avar( "%4.1f", 1.0f / fpsReal ) );
      Con::setVariable( "fps::realMin",   avar( "%4.1f", 1.0f / fpsRealMin ) );
      Con::setVariable( "fps::realMax",   avar( "%4.1f", 1.0f / fpsRealMax ) );
      Con::setVariable( "fps::virtual",   avar( "%4.1f", 1.0f / fpsVirtual ) );

      if (update > mUpdateInterval)
         fpsNext  = fpsRealLast + mUpdateInterval;
      else
         fpsNext += mUpdateInterval;
   }
}

ConsoleFunction( resetFPSTracker, void, 1, 1, "()"
   "@brief Reset FPS stats (fps::)\n\n"
   "@ingroup Game")
{
   gFPS.reset();
}
