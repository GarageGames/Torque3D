singleton ShaderData( ClearGBufferShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/lighting/advanced/deferredClearGBufferV.hlsl";
   DXPixelShaderFile  = $Core::CommonShaderPath @ "/lighting/advanced/deferredClearGBufferP.hlsl";

   OGLVertexShaderFile = $Core::CommonShaderPath @ "/postFX/gl/postFxV.glsl";
   OGLPixelShaderFile  = $Core::CommonShaderPath @ "/lighting/advanced/gl/deferredClearGBufferP.glsl";

   pixVersion = 2.0;   
};

singleton ShaderData( DeferredColorShader )
{
   DXVertexShaderFile = $Core::CommonShaderPath @ "/postFx/postFxV.hlsl";
   DXPixelShaderFile  = $Core::CommonShaderPath @ "/lighting/advanced/deferredColorShaderP.hlsl";
   
   OGLVertexShaderFile = $Core::CommonShaderPath @ "/postFX/gl/postFxV.glsl";
   OGLPixelShaderFile  = $Core::CommonShaderPath @ "/lighting/advanced/gl/deferredColorShaderP.glsl";

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
   DXVertexShaderFile = $Core::CommonShaderPath @ "/postFX/postFxV.hlsl";
   DXPixelShaderFile  = $Core::CommonShaderPath @ "/lighting/advanced/deferredShadingP.hlsl";
   
   OGLVertexShaderFile = $Core::CommonShaderPath @ "/postFX/gl/postFxV.glsl";
   OGLPixelShaderFile  = $Core::CommonShaderPath @ "/lighting/advanced/gl/deferredShadingP.glsl";

   samplerNames[0] = "colorBufferTex";
   samplerNames[1] = "lightPrePassTex";
   samplerNames[2] = "matInfoTex";
   samplerNames[3] = "prepassTex";
   
   pixVersion = 2.0;
};

singleton PostEffect( AL_DeferredShading )
{
   renderTime = "PFXAfterBin";
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