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

new ShaderData( AL_ShadowVisualizeShader )
{
   DXVertexShaderFile = "shaders/common/guiMaterialV.hlsl";
   DXPixelShaderFile  = "shaders/common/lighting/advanced/dbgShadowVisualizeP.hlsl";
   
   OGLVertexShaderFile = "shaders/common/gl/guiMaterialV.glsl";
   OGLPixelShaderFile  = "shaders/common/lighting/advanced/gl/dbgShadowVisualizeP.glsl";
   
   pixVersion = 2.0;
};

new CustomMaterial( AL_ShadowVisualizeMaterial )
{
   shader = AL_ShadowVisualizeShader;
   stateBlock = AL_DepthVisualizeState;
   
   sampler["shadowMap"] = "#AL_ShadowVizTexture";
   sampler["depthViz"] = "depthviz";

   pixVersion = 2.0;
};

singleton GuiControlProfile( AL_ShadowLabelTextProfile )
{
   fontColor = "0 0 0";
   autoSizeWidth = true;
   autoSizeHeight = true;
   justify = "left";
   fontSize = 14;
};

/// Toggles the visualization of the pre-pass lighting buffer.
function toggleShadowViz()
{
   if ( AL_ShadowVizOverlayCtrl.isAwake() )
   {
      setShadowVizLight( 0 );
      Canvas.popDialog( AL_ShadowVizOverlayCtrl );
   }
   else
   {
      Canvas.pushDialog( AL_ShadowVizOverlayCtrl, 100 );
      _setShadowVizLight( EWorldEditor.getSelectedObject( 0 ) );
   }
}

/// Called from the WorldEditor when an object is selected.
function _setShadowVizLight( %light, %force )
{
   if ( !AL_ShadowVizOverlayCtrl.isAwake() )   
      return;
      
   if ( AL_ShadowVizOverlayCtrl.isLocked && !%force )
      return;
      
   // Resolve the object to the client side.
   if ( isObject( %light ) )
   {      
      %clientLight = serverToClientObject( %light );
      %sizeAndAspect = setShadowVizLight( %clientLight );
   }      
   
   AL_ShadowVizOverlayCtrl-->MatCtrl.setMaterial( "AL_ShadowVisualizeMaterial" );      
   
   %text = "ShadowViz";
   if ( isObject( %light ) )
      %text = %text @ " : " @ getWord( %sizeAndAspect, 0 ) @ " x " @ getWord( %sizeAndAspect, 1 );
      
   AL_ShadowVizOverlayCtrl-->WindowCtrl.text = %text;   
}

/// For convenience, push the viz dialog and set the light manually from the console.
function showShadowVizForLight( %light )
{   
   if ( !AL_ShadowVizOverlayCtrl.isAwake() )
      Canvas.pushDialog( AL_ShadowVizOverlayCtrl, 100 );
   _setShadowVizLight( %light, true );
}

// Prevent shadowViz from changing lights in response to editor selection
// events until unlock is called. The only way a vis light will change while locked
// is if showShadowVizForLight is explicitly called by the user.
function lockShadowViz()
{
   AL_ShadowVizOverlayCtrl.islocked = true;
}

function unlockShadowViz()
{
   AL_ShadowVizOverlayCtrl.islocked = false;
}