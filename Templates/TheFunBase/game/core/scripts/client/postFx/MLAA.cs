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

// An implementation of "Practical Morphological Anti-Aliasing" from 
// GPU Pro 2 by Jorge Jimenez, Belen Masia, Jose I. Echevarria, 
// Fernando Navarro, and Diego Gutierrez.
//
// http://www.iryoku.com/mlaa/

// NOTE: This is currently disabled in favor of FXAA.  See 
// core\scripts\client\canvas.cs if you want to re-enable it.

singleton GFXStateBlockData( MLAA_EdgeDetectStateBlock : PFX_DefaultStateBlock )
{   
   // Mark the edge pixels in stencil.
   stencilDefined = true;
   stencilEnable = true;
   stencilPassOp = GFXStencilOpReplace;
   stencilFunc = GFXCmpAlways;
   stencilRef = 1;

   samplersDefined = true;   
   samplerStates[0] = SamplerClampLinear;
};

singleton ShaderData( MLAA_EdgeDetectionShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/mlaa/offsetV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/mlaa/edgeDetectionP.hlsl";
   
   OGLVertexShaderFile  = "shaders/common/postFx/mlaa/gl/offsetV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/mlaa/gl/edgeDetectionP.glsl";

   samplerNames[0] = "$colorMapG";
   samplerNames[1] = "$prepassMap";

   pixVersion = 3.0;
};

singleton GFXStateBlockData( MLAA_BlendWeightCalculationStateBlock : PFX_DefaultStateBlock )
{   
   // Here we want to process only marked pixels.
   stencilDefined = true;
   stencilEnable = true;
   stencilPassOp = GFXStencilOpKeep;
   stencilFunc = GFXCmpEqual;
   stencilRef = 1;

   samplersDefined = true;   
   samplerStates[0] = SamplerClampPoint;
   samplerStates[1] = SamplerClampLinear;
   samplerStates[2] = SamplerClampPoint;
};

singleton ShaderData( MLAA_BlendWeightCalculationShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/mlaa/passthruV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/mlaa/blendWeightCalculationP.hlsl";
   
   OGLVertexShaderFile  = "shaders/common/postFx/mlaa/gl/passthruV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/mlaa/gl/blendWeightCalculationP.glsl";

   samplerNames[0] = "$edgesMap";
   samplerNames[1] = "$edgesMapL";
   samplerNames[2] = "$areaMap";

   pixVersion = 3.0;
};

singleton GFXStateBlockData( MLAA_NeighborhoodBlendingStateBlock : PFX_DefaultStateBlock )
{   
   // Here we want to process only marked pixels too.
   stencilDefined = true;
   stencilEnable = true;
   stencilPassOp = GFXStencilOpKeep;
   stencilFunc = GFXCmpEqual;
   stencilRef = 1;

   samplersDefined = true;   
   samplerStates[0] = SamplerClampPoint;
   samplerStates[1] = SamplerClampLinear;
   samplerStates[2] = SamplerClampPoint;
};

singleton ShaderData( MLAA_NeighborhoodBlendingShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/mlaa/offsetV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/mlaa/neighborhoodBlendingP.hlsl";
   
   OGLVertexShaderFile  = "shaders/common/postFx/mlaa/gl/offsetV.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/mlaa/gl/neighborhoodBlendingP.glsl";

   samplerNames[0] = "$blendMap";
   samplerNames[1] = "$colorMapL";
   samplerNames[2] = "$colorMap";

   pixVersion = 3.0;
};


singleton PostEffect( MLAAFx )
{
   isEnabled = false;
   
   allowReflectPass = false;
   renderTime = "PFXAfterDiffuse";

   texture[0] = "$backBuffer"; //colorMapG      
   texture[1] = "#prepass"; // Used for depth detection

   target = "$outTex";
   targetClear = PFXTargetClear_OnDraw;
   targetClearColor = "0 0 0 0";

   stateBlock = MLAA_EdgeDetectStateBlock;
   shader = MLAA_EdgeDetectionShader;

   // The luma calculation weights which can be user adjustable
   // per-scene if nessasary.  The default value of...
   //
   //    0.2126 0.7152 0.0722 
   //
   // ... is the HDTV ITU-R Recommendation BT. 709.
   lumaCoefficients = "0.2126 0.7152 0.0722";

   // The tweakable color threshold used to select
   // the range of edge pixels to blend.
   threshold = 0.1;

   // The depth delta threshold used to select
   // the range of edge pixels to blend.
   depthThreshold = 0.01;

   new PostEffect()
   {
      internalName = "blendingWeightsCalculation";

      target = "$outTex";
      targetClear = PFXTargetClear_OnDraw;

      shader = MLAA_BlendWeightCalculationShader;
      stateBlock = MLAA_BlendWeightCalculationStateBlock;

      texture[0] = "$inTex"; // Edges mask    
      texture[1] = "$inTex"; // Edges mask 
      texture[2] = "AreaMap33.dds";
   };

   new PostEffect()
   {
      internalName = "neighborhoodBlending";

      shader = MLAA_NeighborhoodBlendingShader;
      stateBlock = MLAA_NeighborhoodBlendingStateBlock;

      texture[0] = "$inTex"; // Blend weights
      texture[1] = "$backBuffer";      
      texture[2] = "$backBuffer";      
   };
};

function MLAAFx::setShaderConsts(%this)
{
   %this.setShaderConst("$lumaCoefficients", %this.lumaCoefficients);
   %this.setShaderConst("$threshold", %this.threshold);
   %this.setShaderConst("$depthThreshold", %this.depthThreshold);
}