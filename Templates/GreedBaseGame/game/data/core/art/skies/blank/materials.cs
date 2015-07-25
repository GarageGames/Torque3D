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

singleton CubemapData( BlackSkyCubemap )
{
   cubeFace[0] = "./solidsky_black";
   cubeFace[1] = "./solidsky_black";
   cubeFace[2] = "./solidsky_black";
   cubeFace[3] = "./solidsky_black";
   cubeFace[4] = "./solidsky_black";
   cubeFace[5] = "./solidsky_black";
};

singleton Material( BlackSkyMat )
{
   cubemap = BlackSkyCubemap;
   materialTag0 = "Skies";
};

singleton CubemapData( BlueSkyCubemap )
{
   cubeFace[0] = "./solidsky_blue";
   cubeFace[1] = "./solidsky_blue";
   cubeFace[2] = "./solidsky_blue";
   cubeFace[3] = "./solidsky_blue";
   cubeFace[4] = "./solidsky_blue";
   cubeFace[5] = "./solidsky_blue";
};

singleton Material( BlueSkyMat )
{
   cubemap = BlueSkyCubemap;
   materialTag0 = "Skies";
};

singleton CubemapData( GreySkyCubemap )
{
   cubeFace[0] = "./solidsky_grey";
   cubeFace[1] = "./solidsky_grey";
   cubeFace[2] = "./solidsky_grey";
   cubeFace[3] = "./solidsky_grey";
   cubeFace[4] = "./solidsky_grey";
   cubeFace[5] = "./solidsky_grey";
};

singleton Material( GreySkyMat )
{
   cubemap = GreySkyCubemap;
   materialTag0 = "Skies";
};
