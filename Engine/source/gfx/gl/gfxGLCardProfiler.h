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

#ifndef _GFXGLCARDPROFILE_H
#define _GFXGLCARDPROFILE_H

#include "gfx/gfxCardProfile.h"

/// A note on workarounds - The following settings are used exclusively to work around driver bugs:
/// GL::Workaround::needsExplicitGenerateMipmap
/// GL::Workaround::X1600DepthBufferCopy
/// GL::Workaround::HD2600DepthBufferCopy
/// If you find that you need to work around additional driver bugs, add a new setting
/// of the form GL::Workaround::<name> and set it to false in GFXGLCardProfiler::setupCardCapabilities()
/// Enclose your workaround code in if(GFX->getCardProfiler()->queryProfile("GL::Workaround::<name>")) {
/// <workaround> }
/// Remember to set the work around to true in the card profile script!

class GFXGLCardProfiler : public GFXCardProfiler
{
public:
   void init();

protected:
   virtual const String& getRendererString() const { return mRendererString; }
   virtual void setupCardCapabilities();
   virtual bool _queryCardCap(const String& query, U32& foundResult);
   virtual bool _queryFormat(const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips);

private:
   String mRendererString;
   typedef GFXCardProfiler Parent;
};

#endif
