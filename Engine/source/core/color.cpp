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
#include "core/color.h"

const ColorF ColorF::ZERO( 0, 0, 0, 0 );
const ColorF ColorF::ONE( 1, 1, 1, 1 );
const ColorF ColorF::WHITE( 1, 1, 1 );
const ColorF ColorF::BLACK( 0, 0, 0 );
const ColorF ColorF::RED( 1, 0, 0 );
const ColorF ColorF::GREEN( 0, 1, 0 );
const ColorF ColorF::BLUE( 0, 0, 1 );

const ColorI ColorI::ZERO( 0, 0, 0, 0 );
const ColorI ColorI::ONE( 255, 255, 255, 255 );
const ColorI ColorI::WHITE( 255, 255, 255 );
const ColorI ColorI::BLACK( 0, 0, 0 );
const ColorI ColorI::RED( 255, 0, 0 );
const ColorI ColorI::GREEN( 0, 255, 0 );
const ColorI ColorI::BLUE( 0, 0, 255 );
