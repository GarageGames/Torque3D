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
// Debug Shaders.
new ShaderData( AL_ColorBufferShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/postFX/postFxV.hlsl";
   DXPixelShaderFile  = "./shaders/dbgColorBufferP.hlsl";
   
   OGLVertexShaderFile = $Core::CommonShaderPath @ "/postFX/gl/postFxV.glsl";
   OGLPixelShaderFile  = "./shaders/dbgColorBufferP.glsl";

   samplerNames[0] = "colorBufferTex";
   pixVersion = 2.0;
};

singleton PostEffect( AL_ColorBufferVisualize )
{   
   shader = AL_ColorBufferShader;
   stateBlock = AL_DefaultVisualizeState;
   texture[0] = "#color";
   target = "$backBuffer";
   renderPriority = 9999;
};

/// Toggles the visualization of the AL lighting specular power buffer.
function toggleColorBufferViz( %enable )
{   
   if ( %enable $= "" )
   {
      $AL_ColorBufferShaderVar = AL_ColorBufferVisualize.isEnabled() ? false : true;
      AL_ColorBufferVisualize.toggle();
   }
   else if ( %enable )
   {
      AL_DeferredShading.disable();
      AL_ColorBufferVisualize.enable();
   }
   else if ( !%enable )
   {
      AL_ColorBufferVisualize.disable();    
      AL_DeferredShading.enable();
   }
}

new ShaderData( AL_SpecMapShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/postFX/postFxV.hlsl";
   DXPixelShaderFile  = "./shaders/dbgSpecMapVisualizeP.hlsl";

   OGLVertexShaderFile = $Core::CommonShaderPath @ "/postFX/gl/postFxV.glsl";
   OGLPixelShaderFile  = "./shaders/dbgSpecMapVisualizeP.glsl";

   samplerNames[0] = "matinfoTex";
   pixVersion = 2.0;
};

singleton PostEffect( AL_SpecMapVisualize )
{   
   shader = AL_SpecMapShader;
   stateBlock = AL_DefaultVisualizeState;
   texture[0] = "#matinfo";
   target = "$backBuffer";
   renderPriority = 9999;
};

/// Toggles the visualization of the AL lighting specular power buffer.
function toggleSpecMapViz( %enable )
{   
   if ( %enable $= "" )
   {
      $AL_SpecMapShaderVar = AL_SpecMapVisualize.isEnabled() ? false : true;
      AL_SpecMapVisualize.toggle();
   }
   else if ( %enable )
      AL_SpecMapVisualize.enable();
   else if ( !%enable )
      AL_SpecMapVisualize.disable();    
}

new GFXStateBlockData( AL_DepthVisualizeState )
{
   zDefined = true;
   zEnable = false;
   zWriteEnable = false;

   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint; // depth    
   samplerStates[1] = SamplerClampLinear; // viz color lookup
};

new GFXStateBlockData( AL_DefaultVisualizeState )
{
   blendDefined = true;
   blendEnable = true;
   blendSrc = GFXBlendSrcAlpha;
   blendDest = GFXBlendInvSrcAlpha;
   
   zDefined = true;
   zEnable = false;
   zWriteEnable = false;

   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint;   // #deferred
   samplerStates[1] = SamplerClampLinear;  // depthviz
};

new ShaderData( AL_DepthVisualizeShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/postFX/postFxV.hlsl";
   DXPixelShaderFile  = "./shaders/dbgDepthVisualizeP.hlsl";

   OGLVertexShaderFile = $Core::CommonShaderPath @ "/postFX/gl/postFxV.glsl";
   OGLPixelShaderFile  = "./shaders/dbgDepthVisualizeP.glsl";

   samplerNames[0] = "deferredTex";
   samplerNames[1] = "depthViz";

   pixVersion = 2.0;
};

singleton PostEffect( AL_DepthVisualize )
{   
   shader = AL_DepthVisualizeShader;
   stateBlock = AL_DefaultVisualizeState;
   texture[0] = "#deferred";
   texture[1] = "tools/worldEditor/images/depthviz";   
   target = "$backBuffer";
   renderPriority = 9999;
};

function AL_DepthVisualize::onEnabled( %this )
{
   AL_NormalsVisualize.disable();
   AL_LightColorVisualize.disable();
   AL_LightSpecularVisualize.disable();
   $AL_NormalsVisualizeVar = false;
   $AL_LightColorVisualizeVar = false;
   $AL_LightSpecularVisualizeVar = false;
   
   return true;
}

new ShaderData( AL_GlowVisualizeShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/postFX/postFxV.hlsl";
   DXPixelShaderFile  = "./shaders/dbgGlowVisualizeP.hlsl";
   
   OGLVertexShaderFile = $Core::CommonShaderPath @ "/postFX/gl/postFxV.glsl";
   OGLPixelShaderFile  = "./shaders/dbgGlowVisualizeP.glsl";

   samplerNames[0] = "glowBuffer";
   pixVersion = 2.0;
};

singleton PostEffect( AL_GlowVisualize )
{   
   shader = AL_GlowVisualizeShader;
   stateBlock = AL_DefaultVisualizeState;
   texture[0] = "#glowbuffer";
   target = "$backBuffer";
   renderPriority = 9999;
};

new ShaderData( AL_NormalsVisualizeShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/postFX/postFxV.hlsl";
   DXPixelShaderFile  = "./shaders/dbgNormalVisualizeP.hlsl";

   OGLVertexShaderFile = $Core::CommonShaderPath @ "/postFX/gl/postFxV.glsl";
   OGLPixelShaderFile  = "./shaders/dbgNormalVisualizeP.glsl";
   
   samplerNames[0] = "deferredTex";
   
   pixVersion = 2.0;
};

singleton PostEffect( AL_NormalsVisualize )
{   
   shader = AL_NormalsVisualizeShader;
   stateBlock = AL_DefaultVisualizeState;
   texture[0] = "#deferred";
   target = "$backBuffer";
   renderPriority = 9999;
};

function AL_NormalsVisualize::onEnabled( %this )
{
   AL_DepthVisualize.disable();
   AL_LightColorVisualize.disable();
   AL_LightSpecularVisualize.disable();
   $AL_DepthVisualizeVar = false;
   $AL_LightColorVisualizeVar = false;
   $AL_LightSpecularVisualizeVar = false;
   
   return true;
}



new ShaderData( AL_LightColorVisualizeShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/postFX/postFxV.hlsl";
   DXPixelShaderFile  = "./shaders/dbgLightColorVisualizeP.hlsl";

   OGLVertexShaderFile = $Core::CommonShaderPath @ "/postFX/gl/postFxV.glsl";
   OGLPixelShaderFile  = "./shaders/dbgLightColorVisualizeP.glsl";
   
   samplerNames[0] = "lightDeferredTex";
   
   pixVersion = 2.0;
};

singleton PostEffect( AL_LightColorVisualize )
{   
   shader = AL_LightColorVisualizeShader;
   stateBlock = AL_DefaultVisualizeState;
   texture[0] = "#lightinfo";
   target = "$backBuffer";
   renderPriority = 9999;
};

function AL_LightColorVisualize::onEnabled( %this )
{
   AL_NormalsVisualize.disable();
   AL_DepthVisualize.disable();
   AL_LightSpecularVisualize.disable();
   $AL_NormalsVisualizeVar = false;
   $AL_DepthVisualizeVar = false;
   $AL_LightSpecularVisualizeVar = false;   
   
   return true;
}


new ShaderData( AL_LightSpecularVisualizeShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/postFX/postFxV.hlsl";
   DXPixelShaderFile  = "./shaders/dbgLightSpecularVisualizeP.hlsl";

   OGLVertexShaderFile = $Core::CommonShaderPath @ "/postFX/gl/postFxV.glsl";
   OGLPixelShaderFile  = "./shaders/dbgLightSpecularVisualizeP.glsl";
   
   samplerNames[0] = "lightDeferredTex";
   
   pixVersion = 2.0;
};

singleton PostEffect( AL_LightSpecularVisualize )
{   
   shader = AL_LightSpecularVisualizeShader;
   stateBlock = AL_DefaultVisualizeState;
   texture[0] = "#lightinfo";
   target = "$backBuffer";
   renderPriority = 9999;
};

function AL_LightSpecularVisualize::onEnabled( %this )
{
   AL_NormalsVisualize.disable();
   AL_DepthVisualize.disable();
   AL_LightColorVisualize.disable();
   $AL_NormalsVisualizeVar = false;
   $AL_DepthVisualizeVar = false;
   $AL_LightColorVisualizeVar = false;   
   
   return true;
}

/// Toggles the visualization of the AL depth buffer.
function toggleDepthViz( %enable )
{
   if ( %enable $= "" )
   {
      $AL_DepthVisualizeVar = AL_DepthVisualize.isEnabled() ? false : true;
      AL_DepthVisualize.toggle();
   }
   else if ( %enable )
      AL_DepthVisualize.enable();
   else if ( !%enable )
      AL_DepthVisualize.disable();
}

/// Toggles the visualization of the AL depth buffer.
function toggleGlowViz( %enable )
{
   if ( %enable $= "" )
   {
      $AL_GlowVisualizeVar = AL_GlowVisualize.isEnabled() ? false : true;
      AL_GlowVisualize.toggle();
   }
   else if ( %enable )
      AL_GlowVisualize.enable();
   else if ( !%enable )
      AL_GlowVisualize.disable();
}

/// Toggles the visualization of the AL normals buffer.
function toggleNormalsViz( %enable )
{
   if ( %enable $= "" )
   {
      $AL_NormalsVisualizeVar = AL_NormalsVisualize.isEnabled() ? false : true;
      AL_NormalsVisualize.toggle();
   }
   else if ( %enable )
      AL_NormalsVisualize.enable();
   else if ( !%enable )
      AL_NormalsVisualize.disable();   
}

/// Toggles the visualization of the AL lighting color buffer.
function toggleLightColorViz( %enable )
{   
   if ( %enable $= "" )
   {
      $AL_LightColorVisualizeVar = AL_LightColorVisualize.isEnabled() ? false : true;
      AL_LightColorVisualize.toggle();
   }
   else if ( %enable )
      AL_LightColorVisualize.enable();
   else if ( !%enable )
      AL_LightColorVisualize.disable();    
}

/// Toggles the visualization of the AL lighting specular power buffer.
function toggleLightSpecularViz( %enable )
{   
   if ( %enable $= "" )
   {
      $AL_LightSpecularVisualizeVar = AL_LightSpecularVisualize.isEnabled() ? false : true;
      AL_LightSpecularVisualize.toggle();
   }
   else if ( %enable )
      AL_LightSpecularVisualize.enable();
   else if ( !%enable )
      AL_LightSpecularVisualize.disable();    
}

function toggleBackbufferViz( %enable )
{   
   if ( %enable $= "" )
   {
      $AL_BackbufferVisualizeVar = AL_DeferredShading.isEnabled() ? true : false;
      AL_DeferredShading.toggle();
   }
   else if ( %enable )
      AL_DeferredShading.disable();
   else if ( !%enable )
      AL_DeferredShading.enable();    
}

function toggleColorBlindnessViz( %enable )
{   
   if ( %enable $= "" )
   {
      $CBV_Protanopia = ColorBlindnessVisualize.isEnabled() ? false : true;
      ColorBlindnessVisualize.toggle();
   }
   else if ( %enable )
      ColorBlindnessVisualize.enable();
   else if ( !%enable )
      ColorBlindnessVisualize.disable();    
}

new ShaderData( ColorBlindnessVisualizeShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/postFX/postFxV.hlsl";
   DXPixelShaderFile  = "tools/worldEditor/scripts/shaders/dbgColorBlindnessVisualizeP.hlsl";

   OGLVertexShaderFile = $Core::CommonShaderPath @ "/postFX/gl/postFxV.glsl";
   OGLPixelShaderFile  = "tools/worldEditor/scripts/shaders/dbgColorBlindnessVisualizeP.glsl";
   
   samplerNames[0] = "$backBuffer";
   
   pixVersion = 2.0;
};

singleton PostEffect( ColorBlindnessVisualize )
{   
   isEnabled         = false;
   allowReflectPass  = false;
   renderTime        = "PFXAfterBin";
   renderBin         = "GlowBin";
   
   shader = ColorBlindnessVisualizeShader;
   stateBlock = PFX_DefaultStateBlock;
   texture[0] = "$backBuffer";
   target = "$backBuffer";
   renderPriority    = 10;
};

function ColorBlindnessVisualize::setShaderConsts(%this)
{
   %mode = 0;
   
   if($CBV_Protanopia)
      %mode = 1;
   else if($CBV_Protanomaly)
      %mode = 2;
   else if($CBV_Deuteranopia)
      %mode = 3;
   else if($CBV_Deuteranomaly)
      %mode = 4;
   else if($CBV_Tritanopia)
      %mode = 5;
   else if($CBV_Tritanomaly)
      %mode = 6;
   else if($CBV_Achromatopsia)
      %mode = 7;
   else if($CBV_Achromatomaly)
      %mode = 8;
      
   %this.setShaderConst("$mode", %mode);
}

function ColorBlindnessVisualize::onEnabled( %this )
{
   AL_NormalsVisualize.disable();
   AL_DepthVisualize.disable();
   AL_LightSpecularVisualize.disable();
   AL_LightColorVisualize.disable();
   $AL_NormalsVisualizeVar = false;
   $AL_DepthVisualizeVar = false;
   $AL_LightSpecularVisualizeVar = false;   
   $AL_LightColorVisualizeVar = false;   
   
   return true;
}