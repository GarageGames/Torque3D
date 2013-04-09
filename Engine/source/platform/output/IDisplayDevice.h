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

#ifndef _IDISPLAYDEVICE_H_
#define _IDISPLAYDEVICE_H_

#include "console/consoleTypes.h"

// Defines a custom display device that requires particular rendering settings
// in order for a scene to display correctly.

class IDisplayDevice
{
public:
   virtual bool providesYFOV() const = 0;
   virtual F32 getYFOV() const = 0;

   virtual bool providesEyeOffset() const = 0;
   virtual const Point3F& getEyeOffset() const = 0;

   virtual bool providesProjectionOffset() const = 0;
   virtual const Point2F& getProjectionOffset() const = 0;
};

#endif   // _IDISPLAYDEVICE_H_
