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

function initRenderManager()
{
   assert( !isObject( DiffuseRenderPassManager ), "initRenderManager() - DiffuseRenderPassManager already initialized!" );
        
	new RenderPassManager( DiffuseRenderPassManager );
	
	// This token, and the associated render managers, ensure that driver MSAA 
	// does not get used for Advanced Lighting renders.  The 'AL_FormatResolve' 
	// PostEffect copies the result to the backbuffer.	
   new RenderFormatToken(AL_FormatToken)
   {
      enabled = "false";
      
      format = "GFXFormatR8G8B8A8";
      depthFormat = "GFXFormatD24S8";
      aaLevel = 0; // -1 = match backbuffer
      
      // The contents of the back buffer before this format token is executed
      // is provided in $inTex
      copyEffect = "AL_FormatCopy";
      
      // The contents of the render target created by this format token is
      // provided in $inTex
      resolveEffect = "AL_FormatCopy";
   };
   DiffuseRenderPassManager.addManager( new RenderPassStateBin() { renderOrder = 0.001; stateToken = AL_FormatToken; } );
     
   // We really need to fix the sky to render after all the 
   // meshes... but that causes issues in reflections.
   DiffuseRenderPassManager.addManager( new RenderObjectMgr() { bintype = "Sky"; renderOrder = 0.1; processAddOrder = 0.1; } );
   
   //DiffuseRenderPassManager.addManager( new RenderVistaMgr()               { bintype = "Vista"; renderOrder = 0.15; processAddOrder = 0.15; } );
   
   DiffuseRenderPassManager.addManager( new RenderObjectMgr()              { bintype = "Begin"; renderOrder = 0.2; processAddOrder = 0.2; } );
   // Normal mesh rendering.
   DiffuseRenderPassManager.addManager( new RenderTerrainMgr()             { renderOrder = 0.4; processAddOrder = 0.4; } );
   DiffuseRenderPassManager.addManager( new RenderMeshMgr()                { bintype = "Mesh"; renderOrder = 0.5; processAddOrder = 0.5; } );
   DiffuseRenderPassManager.addManager( new RenderImposterMgr()            { renderOrder = 0.56; processAddOrder = 0.56; } );
   DiffuseRenderPassManager.addManager( new RenderObjectMgr()              { bintype = "Object"; renderOrder = 0.6; processAddOrder = 0.6; } );
     
   DiffuseRenderPassManager.addManager( new RenderObjectMgr()              { bintype = "Shadow"; renderOrder = 0.7; processAddOrder = 0.7; } );
   DiffuseRenderPassManager.addManager( new RenderMeshMgr()                { bintype = "Decal"; renderOrder = 0.8; processAddOrder = 0.8; } );
   DiffuseRenderPassManager.addManager( new RenderOcclusionMgr()           { bintype = "Occluder"; renderOrder = 0.9; processAddOrder = 0.9; } );
     
   // We now render translucent objects that should handle
   // their own fogging and lighting.
   
   // Note that the fog effect is triggered before this bin.
   DiffuseRenderPassManager.addManager( new RenderObjectMgr(ObjTranslucentBin) { bintype = "ObjectTranslucent"; renderOrder = 1.0; processAddOrder = 1.0; } );
         
   DiffuseRenderPassManager.addManager( new RenderObjectMgr()              { bintype = "Water"; renderOrder = 1.2; processAddOrder = 1.2; } );
   DiffuseRenderPassManager.addManager( new RenderObjectMgr()              { bintype = "Foliage"; renderOrder = 1.3; processAddOrder = 1.3; } );
	DiffuseRenderPassManager.addManager( new RenderParticleMgr()            { renderOrder = 1.35; processAddOrder = 1.35; } );
   DiffuseRenderPassManager.addManager( new RenderTranslucentMgr()         { renderOrder = 1.4; processAddOrder = 1.4; } );
   
   DiffuseRenderPassManager.addManager(new RenderObjectMgr(FogBin){ bintype = "ObjectVolumetricFog"; renderOrder = 1.45; processAddOrder = 1.45; } );
   
   // Note that the GlowPostFx is triggered after this bin.
   DiffuseRenderPassManager.addManager( new RenderGlowMgr(GlowBin) { renderOrder = 1.5; processAddOrder = 1.5; } );
   
   // We render any editor stuff from this bin.  Note that the HDR is
   // completed before this bin to keep editor elements from tone mapping.   
   DiffuseRenderPassManager.addManager( new RenderObjectMgr(EditorBin) { bintype = "Editor"; renderOrder = 1.6; processAddOrder = 1.6; } );
               
   // Resolve format change token last.
   DiffuseRenderPassManager.addManager( new RenderPassStateBin() { renderOrder = 1.7; stateToken = AL_FormatToken; } );
}

/// This post effect is used to copy data from the non-MSAA back-buffer to the
/// device back buffer (which could be MSAA). It must be declared here so that
/// it is initialized when 'AL_FormatToken' is initialzed.
singleton GFXStateBlockData( AL_FormatTokenState : PFX_DefaultStateBlock )
{
   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint;
};

singleton PostEffect( AL_FormatCopy )
{
   // This PostEffect is used by 'AL_FormatToken' directly. It is never added to
   // the PostEffectManager. Do not call enable() on it.
   isEnabled = false;
   allowReflectPass = true;
   
   shader = PFX_PassthruShader;
   stateBlock = AL_FormatTokenState;
   
   texture[0] = "$inTex";
   target = "$backbuffer";
};
