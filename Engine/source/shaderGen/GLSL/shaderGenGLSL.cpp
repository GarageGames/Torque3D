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
#include "shaderGen/GLSL/shaderGenGLSL.h"
#include "shaderGen/GLSL/shaderCompGLSL.h"
#include "shaderGen/featureMgr.h"
#include "gfx/gl/tGL/tGL.h"


void ShaderGenPrinterGLSL::printShaderHeader( Stream& stream )
{
   const char *header1 = "//*****************************************************************************\r\n";
   const char *header2 = "// Torque -- GLSL procedural shader\r\n";

   stream.write( dStrlen(header1), header1 );
   stream.write( dStrlen(header2), header2 );
   stream.write( dStrlen(header1), header1 );

   // Cheap HLSL compatibility.
   const char* header3 = "#include \"shaders/common/gl/hlslCompat.glsl\"\r\n";      
   stream.write( dStrlen(header3), header3 );

   const char* header4 = "\r\n";      
   stream.write( dStrlen(header4), header4 );
}

void ShaderGenPrinterGLSL::printMainComment( Stream& stream )
{
   // Print out main function definition
   const char * header5 = "// Main                                                                        \r\n";
   const char * line    = "//-----------------------------------------------------------------------------\r\n";

   stream.write( dStrlen(line), line );
   stream.write( dStrlen(header5), header5 );
   stream.write( dStrlen(line), line );
}

void ShaderGenPrinterGLSL::printVertexShaderCloser( Stream& stream )
{
   // We are render OpenGL upside down for use DX9 texture coords.
   // Must be the last vertex feature.
   const char *closer = "   gl_Position.y *= -1;\r\n}\r\n";
   stream.write( dStrlen(closer), closer );
}

void ShaderGenPrinterGLSL::printPixelShaderOutputStruct( Stream& stream, const MaterialFeatureData &featureData )
{
    // Determine the number of output targets we need
    U32 numMRTs = 0;
    for (U32 i = 0; i < FEATUREMGR->getFeatureCount(); i++)
    {
        const FeatureInfo &info = FEATUREMGR->getAt(i);
        if (featureData.features.hasFeature(*info.type))
            numMRTs |= info.feature->getOutputTargets(featureData);
    }

    WRITESTR(avar("//Fragment shader OUT\r\n"));
    WRITESTR(avar("out vec4 OUT_col;\r\n"));
    for( U32 i = 1; i < 4; i++ )
    {
        if( numMRTs & 1 << i )
            WRITESTR(avar("out vec4 OUT_col%d;\r\n", i));
    }

    WRITESTR("\r\n");
    WRITESTR("\r\n");
}

void ShaderGenPrinterGLSL::printPixelShaderCloser( Stream& stream )
{
    const char *closer = "   \r\n}\r\n";
    stream.write( dStrlen(closer), closer );
}

void ShaderGenPrinterGLSL::printLine(Stream& stream, const String& line)
{
   stream.write(line.length(), line.c_str());
   const char* end = "\r\n";
   stream.write(dStrlen(end), end);
} 

const char* ShaderGenComponentFactoryGLSL::typeToString( GFXDeclType type )
{
   switch ( type )
   {
      default:
      case GFXDeclType_Float:
         return "float";

      case GFXDeclType_Float2:
         return "vec2";

      case GFXDeclType_Float3:
         return "vec3";

      case GFXDeclType_UByte4:
         return "vec4";

      case GFXDeclType_Float4:
      case GFXDeclType_Color:
         return "vec4";
   }
}

ShaderComponent* ShaderGenComponentFactoryGLSL::createVertexInputConnector( const GFXVertexFormat &vertexFormat )
{
   AppVertConnectorGLSL *vertComp = new AppVertConnectorGLSL;

   // Loop thru the vertex format elements.
   for ( U32 i=0; i < vertexFormat.getElementCount(); i++ )
   {
      const GFXVertexElement &element = vertexFormat.getElement( i );
      
      Var *var = NULL;

      if ( element.isSemantic( GFXSemantic::POSITION ) )
      {
         var = vertComp->getElement( RT_POSITION );
         var->setName( "position" );
      }
      else if ( element.isSemantic( GFXSemantic::NORMAL ) )
      {
         var = vertComp->getElement( RT_NORMAL );
         var->setName( "normal" );
      }
      else if ( element.isSemantic( GFXSemantic::TANGENT ) )
      {
         var = vertComp->getElement( RT_TANGENT );
         var->setName( "T" );
      }
      else if ( element.isSemantic( GFXSemantic::TANGENTW ) )
      {
         var = vertComp->getElement( RT_TANGENTW );
         var->setName( "tangentW" );
      }
      else if ( element.isSemantic( GFXSemantic::BINORMAL ) )
      {
         var = vertComp->getElement( RT_BINORMAL );
         var->setName( "B" );
      }
      else if ( element.isSemantic( GFXSemantic::COLOR ) )
      {
         var = vertComp->getElement( RT_COLOR );
         var->setName( "diffuse" );
      }
      else if (element.isSemantic(GFXSemantic::BLENDINDICES))
      {
         var = vertComp->getElement(RT_BLENDINDICES);
         var->setName(String::ToString("vBlendIndex%d", element.getSemanticIndex()));
      }
      else if (element.isSemantic(GFXSemantic::BLENDWEIGHT))
      {
         var = vertComp->getElement(RT_BLENDWEIGHT);
         var->setName(String::ToString("vBlendWeight%d", element.getSemanticIndex()));
      }
      else if ( element.isSemantic( GFXSemantic::TEXCOORD ) )
      {
         var = vertComp->getElement( RT_TEXCOORD );
         if ( element.getSemanticIndex() == 0 )
            var->setName( "texCoord" );
         else
            var->setName( String::ToString( "texCoord%d", element.getSemanticIndex() + 1 ) );
      }
      else
      {
         // Everything else is a texcoord!
         var = vertComp->getElement( RT_TEXCOORD );
         var->setName( "tc" + element.getSemantic() );
      }

      if ( !var )
         continue;

      var->setStructName( "IN" );
      var->setType( typeToString( element.getType() ) );
   }

   return vertComp;
}

ShaderComponent* ShaderGenComponentFactoryGLSL::createVertexPixelConnector()
{
   VertPixelConnectorGLSL* comp = new VertPixelConnectorGLSL;
   return comp;
}

ShaderComponent* ShaderGenComponentFactoryGLSL::createVertexParamsDef()
{
   VertexParamsDefGLSL* comp = new VertexParamsDefGLSL;
   return comp;
}

ShaderComponent* ShaderGenComponentFactoryGLSL::createPixelParamsDef()
{
   PixelParamsDefGLSL* comp = new PixelParamsDefGLSL;
   return comp;
}

