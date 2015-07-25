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

exec( "./shadowFilter.cs" );

singleton GFXStateBlockData( BL_ProjectedShadowSBData )
{
   blendDefined = true;
   blendEnable = true;
   blendSrc = GFXBlendDestColor;
   blendDest = GFXBlendZero;
         
   zDefined = true;
   zEnable = true;
   zWriteEnable = false;
               
   samplersDefined = true;
   samplerStates[0] = SamplerClampLinear;   
   vertexColorEnable = true;
};

singleton ShaderData( BL_ProjectedShadowShaderData )
{
   DXVertexShaderFile     = "shaders/common/projectedShadowV.hlsl";
   DXPixelShaderFile      = "shaders/common/projectedShadowP.hlsl";   
   
   OGLVertexShaderFile     = "shaders/common/gl/projectedShadowV.glsl";
   OGLPixelShaderFile      = "shaders/common/gl/projectedShadowP.glsl";   
      
   samplerNames[0] = "inputTex";
   
   pixVersion = 2.0;
};

singleton CustomMaterial( BL_ProjectedShadowMaterial )
{
   sampler["inputTex"] = "$miscbuff";
 
   shader = BL_ProjectedShadowShaderData;
   stateBlock = BL_ProjectedShadowSBData;
   version = 2.0;
   forwardLit = true;
};

function onActivateBasicLM()
{
   // If HDR is enabled... enable the special format token.
   if ( $platform !$= "macos" && HDRPostFx.isEnabled )
      AL_FormatToken.enable();
      
   // Create render pass for projected shadow.
   new RenderPassManager( BL_ProjectedShadowRPM );

   // Create the mesh bin and add it to the manager.
   %meshBin = new RenderMeshMgr();
   BL_ProjectedShadowRPM.addManager( %meshBin );
   
   // Add both to the root group so that it doesn't
   // end up in the MissionCleanup instant group.
   RootGroup.add( BL_ProjectedShadowRPM );
   RootGroup.add( %meshBin );      
}

function onDeactivateBasicLM()
{
   // Delete the pass manager which also deletes the bin.
   BL_ProjectedShadowRPM.delete();
}

function setBasicLighting()
{
   setLightManager( "Basic Lighting" );   
}
