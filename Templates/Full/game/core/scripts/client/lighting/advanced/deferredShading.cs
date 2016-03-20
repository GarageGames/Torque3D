singleton ShaderData( ClearGBufferShader )
{
   DXVertexShaderFile = "shaders/common/lighting/advanced/deferredClearGBufferV.hlsl";
   DXPixelShaderFile  = "shaders/common/lighting/advanced/deferredClearGBufferP.hlsl";

   OGLVertexShaderFile = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile  = "shaders/common/lighting/advanced/gl/deferredClearGBufferP.glsl";

   pixVersion = 2.0;   
};

singleton ShaderData( DeferredColorShader )
{
   DXVertexShaderFile = "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile  = "shaders/common/lighting/advanced/deferredColorShaderP.hlsl";
   
   OGLVertexShaderFile = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile  = "shaders/common/lighting/advanced/gl/deferredColorShaderP.glsl";

   pixVersion = 2.0;   
};

// Primary Deferred Shader
new GFXStateBlockData( AL_DeferredShadingState : PFX_DefaultStateBlock )
{  
   cullMode = GFXCullNone;
   
   blendDefined = true;
   blendEnable = true; 
   blendSrc = GFXBlendSrcAlpha;
   blendDest = GFXBlendInvSrcAlpha;
   
   samplersDefined = true;
   samplerStates[0] = SamplerWrapLinear;
   samplerStates[1] = SamplerWrapLinear;
   samplerStates[2] = SamplerWrapLinear;
   samplerStates[3] = SamplerWrapLinear;
};

new ShaderData( AL_DeferredShader )
{
   DXVertexShaderFile = "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile  = "shaders/common/lighting/advanced/deferredShadingP.hlsl";
   
   OGLVertexShaderFile = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile  = "shaders/common/lighting/advanced/gl/deferredShadingP.glsl";

   samplerNames[0] = "colorBufferTex";
   samplerNames[1] = "lightPrePassTex";
   samplerNames[2] = "matInfoTex";
   samplerNames[3] = "prepassTex";
   
   pixVersion = 2.0;
};

singleton PostEffect( AL_DeferredShading )
{
   renderTime = "PFXBeforeBin";
   renderBin = "SkyBin";
   shader = AL_DeferredShader;
   stateBlock = AL_DeferredShadingState;
   texture[0] = "#color";
   texture[1] = "#lightinfo";
   texture[2] = "#matinfo";
   texture[3] = "#prepass";
   
   target = "$backBuffer";
   renderPriority = 10000;
   allowReflectPass = true;
};

// Debug Shaders.
new ShaderData( AL_ColorBufferShader )
{
   DXVertexShaderFile = "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile  = "shaders/common/lighting/advanced/dbgColorBufferP.hlsl";
   
   OGLVertexShaderFile = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile  = "shaders/common/lighting/advanced/gl/dbgColorBufferP.glsl";

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
   DXVertexShaderFile = "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile  = "shaders/common/lighting/advanced/dbgSpecMapVisualizeP.hlsl";

   OGLVertexShaderFile = "shaders/common/postFx/gl/postFxV.glsl";
   OGLPixelShaderFile  = "shaders/common/lighting/advanced/gl/dbgSpecMapVisualizeP.glsl";

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