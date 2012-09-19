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


singleton ShaderData( BL_ShadowFilterShaderV )
{   
   DXVertexShaderFile 	= "shaders/common/lighting/basic/shadowFilterV.hlsl";
   DXPixelShaderFile 	= "shaders/common/lighting/basic/shadowFilterP.hlsl";
   
   OGLVertexShaderFile 	= "shaders/common/lighting/basic/gl/shadowFilterV.glsl";
   OGLPixelShaderFile 	= "shaders/common/lighting/basic/gl/shadowFilterP.glsl";

   samplerNames[0] = "$diffuseMap";

   defines = "BLUR_DIR=float2(1.0,0.0)";

   pixVersion = 2.0;     
};

singleton ShaderData( BL_ShadowFilterShaderH : BL_ShadowFilterShaderV )
{
    defines = "BLUR_DIR=float2(0.0,1.0)";
};


singleton GFXStateBlockData( BL_ShadowFilterSB : PFX_DefaultStateBlock )
{
   colorWriteDefined=true;
   colorWriteRed=false;
   colorWriteGreen=false;
   colorWriteBlue=false;
   blendDefined = true;
   blendEnable = true;
};

// NOTE: This is ONLY used in Basic Lighting, and 
// only directly by the ProjectedShadow.  It is not 
// meant to be manually enabled like other PostEffects.
singleton PostEffect( BL_ShadowFilterPostFx )
{
    // Blur vertically
   shader = BL_ShadowFilterShaderV;
   stateBlock = PFX_DefaultStateBlock;
   targetClear = "PFXTargetClear_OnDraw";
   targetClearColor = "0 0 0 0";
   texture[0] = "$inTex";
   target = "$outTex";   

   // Blur horizontal
   new PostEffect()
   {
      shader = BL_ShadowFilterShaderH;
      stateBlock = PFX_DefaultStateBlock;
      texture[0] = "$inTex";
      target = "$outTex";
   };
};
