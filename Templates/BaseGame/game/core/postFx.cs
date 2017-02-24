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

singleton ShaderData( PFX_PassthruShader )
{   
   DXVertexShaderFile 	= $Core::CommonShaderPath @ "/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= $Core::CommonShaderPath @ "/postFx/passthruP.hlsl";
         
//   OGLVertexShaderFile  = $Core::CommonShaderPath @ "/postFx/postFxV.glsl";
//   OGLPixelShaderFile   = $Core::CommonShaderPath @ "/postFx/gl/passthruP.glsl";
      
   samplerNames[0] = "$inputTex";
   
   pixVersion = 2.0;
};

function PostEffect::inspectVars( %this )
{
   %name = %this.getName(); 
   %globals = "$" @ %name @ "::*";
   inspectVars( %globals );   
}

function PostEffect::viewDisassembly( %this )
{
   %file = %this.dumpShaderDisassembly();  
   
   if ( %file $= "" )
   {
      echo( "PostEffect::viewDisassembly - no shader disassembly found." );  
   }
   else
   {
      echo( "PostEffect::viewDisassembly - shader disassembly file dumped ( " @ %file @ " )." );
      openFile( %file );
   }
}

// Return true if we really want the effect enabled.
// By default this is the case.
function PostEffect::onEnabled( %this )
{   
   return true;
}