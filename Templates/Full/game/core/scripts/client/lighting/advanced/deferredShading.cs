singleton ShaderData( ClearGBufferShader )
{
   DXVertexShaderFile = "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile  = "shaders/common/lighting/advanced/deferredClearGBufferP.hlsl";

   pixVersion = 2.0;   
};

singleton ShaderData( DeferredColorShader )
{
   DXVertexShaderFile = "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile  = "shaders/common/lighting/advanced/deferredColorShaderP.hlsl";

   pixVersion = 2.0;   
};

// Primary Deferred Shader
new GFXStateBlockData( AL_DeferredShadingState : PFX_DefaultStateBlock )
{  
   samplersDefined = true;
   samplerStates[0] = SamplerClampLinear;
   samplerStates[1] = SamplerWrapLinear;
   samplerStates[2] = SamplerWrapLinear;
};

new ShaderData( AL_DeferredShader )
{
   DXVertexShaderFile = "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile  = "shaders/common/lighting/advanced/deferredShadingP.hlsl";

   samplerNames[0] = "color";
   samplerNames[1] = "lightInfoBuffer";
   samplerNames[2] = "matInfoTex";
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
   target = "$backBuffer";
   renderPriority = 10000;
   allowReflectPass = true;
};

// Debug Shaders.
new ShaderData( AL_ColorBufferShader )
{
   DXVertexShaderFile = "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile  = "shaders/common/lighting/advanced/dbgColorBufferP.hlsl";

   samplerNames[0] = "color";
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
      AL_ColorBufferVisualize.enable();
   else if ( !%enable )
      AL_ColorBufferVisualize.disable();    
}

new ShaderData( AL_SpecMapShader )
{
   DXVertexShaderFile = "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile  = "shaders/common/lighting/advanced/dbgSpecMapVisualizeP.hlsl";

   samplerNames[0] = "color";
   pixVersion = 2.0;
};

singleton PostEffect( AL_SpecMapVisualize )
{   
   shader = AL_SpecMapShader;
   stateBlock = AL_DefaultVisualizeState;
   texture[0] = "#color";
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