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

/*

================================================================================
 The DOFPostEffect API
================================================================================

DOFPostEffect::setFocalDist( %dist )

@summary
This method is for manually controlling the focus distance. It will have no 
effect if auto focus is currently enabled. Makes use of the parameters set by 
setFocusParams.

@param dist 
float distance in meters

--------------------------------------------------------------------------------

DOFPostEffect::setAutoFocus( %enabled )

@summary
This method sets auto focus enabled or disabled. Makes use of the parameters set 
by setFocusParams. When auto focus is enabled it determines the focal depth
by performing a raycast at the screen-center.

@param enabled
bool

--------------------------------------------------------------------------------

DOFPostEffect::setFocusParams( %nearBlurMax, %farBlurMax, %minRange, %maxRange, %nearSlope, %farSlope )

Set the parameters that control how the near and far equations are calculated
from the focal distance. If you are not using auto focus you will need to call
setFocusParams PRIOR to calling setFocalDist.

@param nearBlurMax   
float between 0.0 and 1.0
The max allowed value of near blur.

@param farBlurMax    
float between 0.0 and 1.0
The max allowed value of far blur.

@param minRange/maxRange 
float in meters
The distance range around the focal distance that remains in focus is a lerp 
between the min/maxRange using the normalized focal distance as the parameter. 
The point is to allow the focal range to expand as you focus farther away since this is 
visually appealing.

Note: since min/maxRange are lerped by the "normalized" focal distance it is
dependant on the visible distance set in your level.

@param nearSlope
float less than zero
The slope of the near equation. A small number causes bluriness to increase gradually
at distances closer than the focal distance. A large number causes bluriness to 
increase quickly.

@param farSlope
float greater than zero
The slope of the far equation. A small number causes bluriness to increase gradually
at distances farther than the focal distance. A large number causes bluriness to
increase quickly.

Note: To rephrase, the min/maxRange parameters control how much area around the
focal distance is completely in focus where the near/farSlope parameters control
how quickly or slowly bluriness increases at distances outside of that range.

================================================================================
 Examples
================================================================================

Example1: Turn on DOF while zoomed in with a weapon.

NOTE: These are not real callbacks! Hook these up to your code where appropriate!

function onSniperZoom()
{
   // Parameterize how you want DOF to look.
   DOFPostEffect.setFocusParams( 0.3, 0.3, 50, 500, -5, 5 );
   
   // Turn on auto focus
   DOFPostEffect.setAutoFocus( true );
   
   // Turn on the PostEffect
   DOFPostEffect.enable();
}

function onSniperUnzoom()
{
   // Turn off the PostEffect
   DOFPostEffect.disable();
}

Example2: Manually control DOF with the mouse wheel.

// Somewhere on startup...

// Parameterize how you want DOF to look.
DOFPostEffect.setFocusParams( 0.3, 0.3, 50, 500, -5, 5 );

// Turn off auto focus
DOFPostEffect.setAutoFocus( false );

// Turn on the PostEffect
DOFPostEffect.enable();


NOTE: These are not real callbacks! Hook these up to your code where appropriate!

function onMouseWheelUp()
{
   // Since setFocalDist is really just a wrapper to assign to the focalDist
   // dynamic field we can shortcut and increment it directly.
   DOFPostEffect.focalDist += 8;
}

function onMouseWheelDown()
{
   DOFPostEffect.focalDist -= 8;
}
*/

/// This method is for manually controlling the focal distance. It will have no 
/// effect if auto focus is currently enabled. Makes use of the parameters set by 
/// setFocusParams.
function DOFPostEffect::setFocalDist( %this, %dist )
{    
   %this.focalDist = %dist;
}

/// This method sets auto focus enabled or disabled. Makes use of the parameters set 
/// by setFocusParams. When auto focus is enabled it determine the focal depth
/// by performing a raycast at the screen-center.
function DOFPostEffect::setAutoFocus( %this, %enabled )
{
   %this.autoFocusEnabled = %enabled;
}

/// Set the parameters that control how the near and far equations are calculated
/// from the focal distance. If you are not using auto focus you will need to call
/// setFocusParams PRIOR to calling setFocalDist.
function DOFPostEffect::setFocusParams( %this, %nearBlurMax, %farBlurMax, %minRange, %maxRange, %nearSlope, %farSlope )
{
   %this.nearBlurMax = %nearBlurMax;
   %this.farBlurMax = %farBlurMax;
   %this.minRange = %minRange;
   %this.maxRange = %maxRange;
   %this.nearSlope = %nearSlope;
   %this.farSlope = %farSlope;
}

/*

More information...

This DOF technique is based on this paper:
http://http.developer.nvidia.com/GPUGems3/gpugems3_ch28.html

================================================================================
1. Overview of how we represent "Depth of Field"
================================================================================

DOF is expressed as an amount of bluriness per pixel according to its depth.
We represented this by a piecewise linear curve depicted below.

Note: we also refer to "bluriness" as CoC ( circle of confusion ) which is the term
used in the basis paper and in photography.
   

X-axis (depth) 
x = 0.0----------------------------------------------x = 1.0       

Y-axis (bluriness)
y = 1.0   
  |
  |   ____(x1,y1)                         (x4,y4)____
  |       (ns,nb)\  <--Line1  line2--->  /(fe,fb)
  |               \                     /
  |                \(x2,y2)     (x3,y3)/
  |                 (ne,0)------(fs,0)  
y = 0.0
                 

I have labeled the "corners" of this graph with (Xn,Yn) to illustrate that
this is in fact a collection of line segments where the x/y of each point
corresponds to the key below.

key:
ns - (n)ear blur (s)tart distance
nb - (n)ear (b)lur amount (max value)
ne - (n)ear blur (e)nd distance
fs - (f)ar blur (s)tart distance
fe - (f)ar blur (e)nd distance
fb - (f)ar (b)lur amount (max value)

Of greatest importance in this graph is Line1 and Line2. Where...
L1 { (x1,y1), (x2,y2) }
L2 { (x3,y3), (x4,y4) }

Line one represents the amount of "near" blur given a pixels depth and line two
represents the amount of "far" blur at that depth.

Both these equations are evaluated for each pixel and then the larger of the two
is kept. Also the output blur (for each equation) is clamped between 0 and its
maximum allowable value.

Therefore, to specify a DOF "qualify" you need to specify the near-blur-line, 
far-blur-line, and maximum near and far blur value.

================================================================================
2. Abstracting a "focal depth"
================================================================================

Although the shader(s) work in terms of a near and far equation it is more
useful to express DOF as an adjustable focal depth and derive the other parameters
"under the hood".

Given a maximum near/far blur amount and a near/far slope we can calculate the
near/far equations for any focal depth. We extend this to also support a range
of depth around the focal depth that is also in focus and for that range to
shrink or grow as the focal depth moves closer or farther.

Keep in mind this is only one implementation and depending on the effect you
desire you may which to express the relationship between focal depth and 
the shader paramaters different. 

*/

//-----------------------------------------------------------------------------
// GFXStateBlockData / ShaderData
//-----------------------------------------------------------------------------

singleton GFXStateBlockData( PFX_DefaultDOFStateBlock )
{
   zDefined = true;
   zEnable = false;
   zWriteEnable = false;
      
   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint;
   samplerStates[1] = SamplerClampPoint;
};

singleton GFXStateBlockData( PFX_DOFCalcCoCStateBlock )
{
   zDefined = true;
   zEnable = false;
   zWriteEnable = false;
      
   samplersDefined = true;
   samplerStates[0] = SamplerClampLinear;
   samplerStates[1] = SamplerClampLinear;
};

singleton GFXStateBlockData( PFX_DOFDownSampleStateBlock )
{
   zDefined = true;
   zEnable = false;
   zWriteEnable = false;
      
   samplersDefined = true;
   samplerStates[0] = SamplerClampLinear;
   samplerStates[1] = SamplerClampPoint;
};

singleton GFXStateBlockData( PFX_DOFBlurStateBlock )
{
   zDefined = true;
   zEnable = false;
   zWriteEnable = false;
      
   samplersDefined = true;
   samplerStates[0] = SamplerClampLinear;   
};

singleton GFXStateBlockData( PFX_DOFFinalStateBlock )
{
   zDefined = true;
   zEnable = false;
   zWriteEnable = false;
      
   samplersDefined = true;
   samplerStates[0] = SamplerClampLinear;
   samplerStates[1] = SamplerClampLinear;
   samplerStates[2] = SamplerClampLinear;
   samplerStates[3] = SamplerClampPoint;
   
   blendDefined = true;
   blendEnable = true;
   blendDest = GFXBlendInvSrcAlpha;
   blendSrc = GFXBlendOne;
};

singleton ShaderData( PFX_DOFDownSampleShader )
{      
   DXVertexShaderFile 	= "shaders/common/postFx/dof/DOF_DownSample_V.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/dof/DOF_DownSample_P.hlsl";
   
   OGLVertexShaderFile  = "shaders/common/postFx/dof/gl/DOF_DownSample_V.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/dof/gl/DOF_DownSample_P.glsl";
   
   samplerNames[0] = "$colorSampler";
   samplerNames[1] = "$depthSampler";
   
   pixVersion = 3.0;
};

singleton ShaderData( PFX_DOFBlurYShader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/dof/DOF_Gausian_V.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/dof/DOF_Gausian_P.hlsl";
   
   OGLVertexShaderFile  = "shaders/common/postFx/dof/gl/DOF_Gausian_V.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/dof/gl/DOF_Gausian_P.glsl";
   
   samplerNames[0] = "$diffuseMap";
   
   pixVersion = 2.0;      
   defines = "BLUR_DIR=float2(0.0,1.0)";         
};

singleton ShaderData( PFX_DOFBlurXShader : PFX_DOFBlurYShader )
{
   defines = "BLUR_DIR=float2(1.0,0.0)";
};

singleton ShaderData( PFX_DOFCalcCoCShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/dof/DOF_CalcCoC_V.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/dof/DOF_CalcCoC_P.hlsl"; 
   
   OGLVertexShaderFile  = "shaders/common/postFx/dof/gl/DOF_CalcCoC_V.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/dof/gl/DOF_CalcCoC_P.glsl"; 

   samplerNames[0] = "$shrunkSampler";
   samplerNames[1] = "$blurredSampler";
   
   pixVersion = 3.0;
};

singleton ShaderData( PFX_DOFSmallBlurShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/dof/DOF_SmallBlur_V.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/dof/DOF_SmallBlur_P.hlsl";
   
   OGLVertexShaderFile  = "shaders/common/postFx/dof/gl/DOF_SmallBlur_V.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/dof/gl/DOF_SmallBlur_P.glsl";

   samplerNames[0] = "$colorSampler";
   
   pixVersion = 3.0;
};

singleton ShaderData( PFX_DOFFinalShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/dof/DOF_Final_V.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/dof/DOF_Final_P.hlsl";
   
   OGLVertexShaderFile  = "shaders/common/postFx/dof/gl/DOF_Final_V.glsl";
   OGLPixelShaderFile   = "shaders/common/postFx/dof/gl/DOF_Final_P.glsl";
   
   samplerNames[0] = "$colorSampler";
   samplerNames[1] = "$smallBlurSampler";
   samplerNames[2] = "$largeBlurSampler";
   samplerNames[3] = "$depthSampler";
   
   pixVersion = 3.0;
};

//-----------------------------------------------------------------------------
// PostEffects
//-----------------------------------------------------------------------------

function DOFPostEffect::onAdd( %this )
{
   // The weighted distribution of CoC value to the three blur textures
   // in the order small, medium, large. Most likely you will not need to
   // change this value.
   %this.setLerpDist( 0.2, 0.3, 0.5 );
   
   // Fill out some default values but DOF really should not be turned on
   // without actually specifying your own parameters!
   %this.autoFocusEnabled = false;
   %this.focalDist = 0.0;
   %this.nearBlurMax = 0.5;
   %this.farBlurMax = 0.5;
   %this.minRange = 50;
   %this.maxRange = 500;
   %this.nearSlope = -5.0;
   %this.farSlope = 5.0;
}

function DOFPostEffect::setLerpDist( %this, %d0, %d1, %d2 )
{
   %this.lerpScale = -1.0 / %d0 SPC -1.0 / %d1 SPC -1.0 / %d2 SPC 1.0 / %d2;
   %this.lerpBias = 1.0 SPC ( 1.0 - %d2 ) / %d1 SPC 1.0 / %d2 SPC ( %d2 - 1.0 ) / %d2;
}

singleton PostEffect( DOFPostEffect )
{
   renderTime = "PFXAfterBin";
   renderBin = "GlowBin";      
   renderPriority = 0.1;
      
   shader = PFX_DOFDownSampleShader;
   stateBlock = PFX_DOFDownSampleStateBlock;
   texture[0] = "$backBuffer";
   texture[1] = "#prepass";
   target = "#shrunk";
   targetScale = "0.25 0.25";   
   
   isEnabled = false;
};

singleton PostEffect( DOFBlurY )
{
   shader = PFX_DOFBlurYShader;
   stateBlock = PFX_DOFBlurStateBlock;
   texture[0] = "#shrunk";
   target = "$outTex";
};

DOFPostEffect.add( DOFBlurY );

singleton PostEffect( DOFBlurX )
{
   shader = PFX_DOFBlurXShader;
   stateBlock = PFX_DOFBlurStateBlock;
   texture[0] = "$inTex";  
   target = "#largeBlur";
};

DOFPostEffect.add( DOFBlurX );

singleton PostEffect( DOFCalcCoC )
{
   shader = PFX_DOFCalcCoCShader;
   stateBlock = PFX_DOFCalcCoCStateBlock;
   texture[0] = "#shrunk";
   texture[1] = "#largeBlur";
   target = "$outTex";
};

DOFPostEffect.add( DOFCalcCoc );
  
singleton PostEffect( DOFSmallBlur )
{
   shader = PFX_DOFSmallBlurShader;
   stateBlock = PFX_DefaultDOFStateBlock;
   texture[0] = "$inTex";
   target = "$outTex";
};

DOFPostEffect.add( DOFSmallBlur );
   
singleton PostEffect( DOFFinalPFX )
{
   shader = PFX_DOFFinalShader;
   stateBlock = PFX_DOFFinalStateBlock;
   texture[0] = "$backBuffer";
   texture[1] = "$inTex";
   texture[2] = "#largeBlur";   
   texture[3] = "#prepass";   
   target = "$backBuffer";
};

DOFPostEffect.add( DOFFinalPFX );


//-----------------------------------------------------------------------------
// Scripts
//-----------------------------------------------------------------------------

function DOFPostEffect::setShaderConsts( %this )
{
   if ( %this.autoFocusEnabled )
      %this.autoFocus();
   
   %fd = %this.focalDist / $Param::FarDist;
      
   %range = mLerp( %this.minRange, %this.maxRange, %fd ) / $Param::FarDist * 0.5;  
   
   // We work in "depth" space rather than real-world units for the
   // rest of this method...
   
   // Given the focal distance and the range around it we want in focus
   // we can determine the near-end-distance and far-start-distance
      
   %ned = getMax( %fd - %range, 0.0 );   
   %fsd = getMin( %fd + %range, 1.0 );         
   
   // near slope
   %nsl = %this.nearSlope;
   
   // Given slope of near blur equation and the near end dist and amount (x2,y2)
   // solve for the y-intercept
   // y = mx + b
   // so...
   // y - mx = b
   
   %b = 0.0 - %nsl * %ned;
   
   %eqNear = %nsl SPC %b SPC 0.0;
   
   // Do the same for the far blur equation...
   
   %fsl = %this.farSlope;
   
   %b = 0.0 - %fsl * %fsd;
   
   %eqFar = %fsl SPC %b SPC 1.0;
   
   %this.setShaderConst( "$dofEqWorld", %eqNear );           
   DOFFinalPFX.setShaderConst( "$dofEqFar", %eqFar );
      
   %this.setShaderConst( "$maxWorldCoC", %this.nearBlurMax );
   DOFFinalPFX.setShaderConst( "$maxFarCoC", %this.farBlurMax );      

   DOFFinalPFX.setShaderConst( "$dofLerpScale", %this.lerpScale );
   DOFFinalPFX.setShaderConst( "$dofLerpBias", %this.lerpBias );  
}

function DOFPostEffect::autoFocus( %this )
{      
   if ( !isObject( ServerConnection ) ||   
        !isObject( ServerConnection.getCameraObject() ) )
   {      
      return;
   }
   
   %mask = $TypeMasks::StaticObjectType | $TypeMasks::TerrainObjectType;               
   %control = ServerConnection.getCameraObject();

   %fvec = %control.getEyeVector();      
   %start = %control.getEyePoint();
      
   %end = VectorAdd( %start, VectorScale( %fvec, $Param::FarDist ) );
      
   // Use the client container for this ray cast.
   %result = containerRayCast( %start, %end, %mask, %control, true );
   
   %hitPos = getWords( %result, 1, 3 );
   
   if ( %hitPos $= "" )
      %focDist = $Param::FarDist;
   else
      %focDist = VectorDist( %hitPos, %start );  
      
   // For debuging   
   //$DOF::debug_dist = %focDist;
   //$DOF::debug_depth = %focDist / $Param::FarDist;      
   //echo( "F: " @ %focDist SPC "D: " @ %delta );
     
   %this.focalDist = %focDist;
}


// For debugging
/*
function reloadDOF()
{
   exec( "./dof.cs" );
   DOFPostEffect.reload();  
   DOFPostEffect.disable();
   DOFPostEffect.enable();
}

function dofMetricsCallback()
{
   return "  | DOF |" @
          "  Dist: " @ $DOF::debug_dist @
          "  Depth: " @ $DOF::debug_depth;               
}
*/