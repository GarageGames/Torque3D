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

static const Point3F BoxNormals[] =
{
	Point3F( 1, 0, 0),
		Point3F(-1, 0, 0),
		Point3F( 0, 1, 0),
		Point3F( 0,-1, 0),
		Point3F( 0, 0, 1),
		Point3F( 0, 0,-1)
};

static U32 BoxVerts[][4] = {
	{7,6,4,5},     // +x
	{0,2,3,1},     // -x
	{7,3,2,6},     // +y
	{0,1,5,4},     // -y
	{7,5,1,3},     // +z
	{0,4,6,2}      // -z
};

static U32 BoxSharedEdgeMask[][6] = {
	{0, 0, 1, 4, 8, 2},
	{0, 0, 2, 8, 4, 1},
	{8, 2, 0, 0, 1, 4},
	{4, 1, 0, 0, 2, 8},
	{1, 4, 8, 2, 0, 0},
	{2, 8, 4, 1, 0, 0}
};

static Point3F BoxPnts[] = {
	Point3F(0,0,0),
		Point3F(0,0,1),
		Point3F(0,1,0),
		Point3F(0,1,1),
		Point3F(1,0,0),
		Point3F(1,0,1),
		Point3F(1,1,0),
		Point3F(1,1,1)
};

extern SceneLighting *gLighting;
extern F32 gParellelVectorThresh;
extern F32 gPlaneNormThresh;
extern F32 gPlaneDistThresh;

