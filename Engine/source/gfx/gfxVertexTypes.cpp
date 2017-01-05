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
#include "gfx/gfxVertexTypes.h"


GFXImplementVertexFormat( GFXVertexP )
{
   addElement( "POSITION", GFXDeclType_Float3 );
}

GFXImplementVertexFormat( GFXVertexPad )
{
   addElement("PADDING", GFXDeclType_UByte4);
}

GFXImplementVertexFormat( GFXVertexPT )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
}

GFXImplementVertexFormat( GFXVertexPTT )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 1 );
}

GFXImplementVertexFormat( GFXVertexPTTT )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 1 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 2 );
}

GFXImplementVertexFormat( GFXVertexPC )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "COLOR", GFXDeclType_Color );
}

GFXImplementVertexFormat( GFXVertexPCN )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "COLOR", GFXDeclType_Color );
}

GFXImplementVertexFormat( GFXVertexPCT )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "COLOR", GFXDeclType_Color );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
}

GFXImplementVertexFormat( GFXVertexPCTT )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "COLOR", GFXDeclType_Color );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 1 );
}

GFXImplementVertexFormat( GFXVertexPN )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "NORMAL", GFXDeclType_Float3 );
}

GFXImplementVertexFormat( GFXVertexPNT )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
}

GFXImplementVertexFormat( GFXVertexPNTT )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "TANGENT", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
}

GFXImplementVertexFormat( GFXVertexPCNTT )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "COLOR", GFXDeclType_Color );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 1 );
}

GFXImplementVertexFormat( GFXVertexPNTBT )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "TANGENT", GFXDeclType_Float3 );
   addElement( "BINORMAL", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
}

GFXImplementVertexFormat( GFXVertexPNTTB )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "TANGENT", GFXDeclType_Float3 );
   addElement( "BINORMAL", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 1 );
}
