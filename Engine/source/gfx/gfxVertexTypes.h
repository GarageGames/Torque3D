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

#ifndef _GFXVERTEXTYPES_H_
#define _GFXVERTEXTYPES_H_

#ifndef _GFXVERTEXFORMAT_H_
#include "gfx/gfxVertexFormat.h"
#endif
#ifndef _GFXVERTEXCOLOR_H_
#include "gfx/gfxVertexColor.h"
#endif
#ifndef _MPOINT2_H_
#include "math/mPoint2.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

GFXDeclareVertexFormat( GFXVertexPad )
{
   U32 data;
};

GFXDeclareVertexFormat( GFXVertexP )
{
   Point3F point;
};

GFXDeclareVertexFormat( GFXVertexPT )
{
   Point3F point;
   Point2F texCoord;
};

GFXDeclareVertexFormat( GFXVertexPTT )
{
   Point3F point;
   Point2F texCoord1;
   Point2F texCoord2;
};

GFXDeclareVertexFormat( GFXVertexPTTT )
{
   Point3F point;
   Point2F texCoord1;
   Point2F texCoord2;
   Point2F texCoord3;
};

GFXDeclareVertexFormat( GFXVertexPC )
{
   Point3F point;
   GFXVertexColor color;
};

GFXDeclareVertexFormat( GFXVertexPCN )
{
   Point3F point;
   Point3F normal;
   GFXVertexColor color;
};

GFXDeclareVertexFormat( GFXVertexPCT )
{
   Point3F point;
   GFXVertexColor color;
   Point2F texCoord;
};

GFXDeclareVertexFormat( GFXVertexPCTT )
{
   Point3F point;
   GFXVertexColor color;
   Point2F texCoord;
   Point2F texCoord2;
};

GFXDeclareVertexFormat( GFXVertexPN )
{
   Point3F point;
   Point3F normal;
};

GFXDeclareVertexFormat( GFXVertexPNT )
{
   Point3F point;
   Point3F normal;
   Point2F texCoord;
};

GFXDeclareVertexFormat( GFXVertexPNTT )
{
   Point3F point;
   Point3F normal;
   Point3F tangent;
   Point2F texCoord;
};

GFXDeclareVertexFormat( GFXVertexPCNTT )
{
   Point3F point;
   GFXVertexColor color;
   Point3F normal;
   Point2F texCoord[2];
};

GFXDeclareVertexFormat( GFXVertexPNTBT )
{
   Point3F point;
   Point3F normal;
   Point3F tangent;
   Point3F binormal;
   Point2F texCoord;
};

/*

DEFINE_VERT( GFXVertexPCNT, 
            GFXVertexFlagXYZ | GFXVertexFlagNormal | GFXVertexFlagDiffuse | GFXVertexFlagTextureCount1 | GFXVertexFlagUV0)
{
   Point3F point;
   Point3F normal;
   GFXVertexColor color;
   Point2F texCoord;
};

DEFINE_VERT( GFXVertexPCNTT, 
            GFXVertexFlagXYZ | GFXVertexFlagNormal | GFXVertexFlagDiffuse | GFXVertexFlagTextureCount2 | GFXVertexFlagUV0 | GFXVertexFlagUV1)
{
   Point3F point;
   Point3F normal;
   GFXVertexColor color;
   Point2F texCoord[2];
};
*/

GFXDeclareVertexFormat( GFXVertexPNTTB )
{
   Point3F point;
   Point3F normal;
   Point3F T;
   Point3F B;
   Point2F texCoord;
   Point2F texCoord2;
};

/*
DEFINE_VERT( GFXVertexPNTB,
            GFXVertexFlagXYZ | GFXVertexFlagNormal | GFXVertexFlagTextureCount2 | 
            GFXVertexFlagUV0 | GFXVertexFlagUVW1 )
{
   Point3F point;
   Point3F normal;
   Point2F texCoord;
   Point3F binormal;
};
*/

#endif // _GFXVERTEXTYPES_H_
