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
#ifndef _GFXDEVICESTATISTICS_H_
#define _GFXDEVICESTATISTICS_H_

#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif

// A class that hold a simple set of device stats.
class GFXDeviceStatistics
{
public:
   // Actual stats
   S32 mPolyCount;
   S32 mDrawCalls;
   S32 mRenderTargetChanges;

   GFXDeviceStatistics();

   void setPrefix(const String& prefix);

   /// Clear stats
   void clear();

   /// Copy from source (should just be a memcpy, but that may change later) used in 
   /// conjunction with end to get a subset of statistics.  For example, statistics
   /// for a particular render bin.
   void start(GFXDeviceStatistics * source);

   /// Used with start to get a subset of stats on a device.  Basically will do
   /// this->mPolyCount = source->mPolyCount - this->mPolyCount.  (Fancy!)
   void end(GFXDeviceStatistics * source);

   /// Exports the stats to the console
   void exportToConsole();
private:
   String vnPolyCount;
   String vnDrawCalls;
   String vnRenderTargetChanges;
};

#endif