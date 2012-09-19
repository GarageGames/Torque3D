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

#ifndef _PLATFORMFONT_H_
#include "platform/platformFont.h"
#endif

#ifndef _WINFONT_H_
#define _WINFONT_H_

class WinFont : public PlatformFont
{
private:
    HFONT mFont;
    TEXTMETRIC mTextMetric;
    
public:
    WinFont();
    virtual ~WinFont();
    
    // PlatformFont virtual methods
    virtual bool isValidChar(const UTF16 ch) const;
    virtual bool isValidChar(const UTF8 *str) const;

    inline virtual U32 getFontHeight() const
    {
        return mTextMetric.tmHeight;
    }

    inline virtual U32 getFontBaseLine() const
    {
        return mTextMetric.tmAscent;
    }

    virtual PlatformFont::CharInfo &getCharInfo(const UTF16 ch) const;
    virtual PlatformFont::CharInfo &getCharInfo(const UTF8 *str) const;

    virtual bool create(const char *name, dsize_t size, U32 charset = TGE_ANSI_CHARSET);
};

#endif // _WINFONT_H_
