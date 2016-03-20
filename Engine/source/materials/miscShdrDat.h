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
#ifndef _MISCSHDRDAT_H_
#define _MISCSHDRDAT_H_

//**************************************************************************
// This file is an attempt to keep certain classes from having to know about
//   the ShaderGen class
//**************************************************************************


//-----------------------------------------------------------------------
// Enums
//-----------------------------------------------------------------------
enum RegisterType
{
   RT_POSITION = 0,
   RT_NORMAL,
   RT_BINORMAL,
   RT_TANGENT,
   RT_TANGENTW,
   RT_COLOR,
   RT_TEXCOORD,
   RT_VPOS,
   RT_SVPOSITION
};

enum Components
{
   C_VERT_STRUCT = 0,
   C_CONNECTOR,
   C_VERT_MAIN,
   C_PIX_MAIN
};

#endif // _MISCSHDRDAT_H_
