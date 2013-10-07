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

//------------------------------------------------------------------------------
// CloudLayer
//------------------------------------------------------------------------------

singleton ShaderData( CloudLayerShader )
{
   DXVertexShaderFile   = "shaders/common/cloudLayerV.hlsl";
   DXPixelShaderFile    = "shaders/common/cloudLayerP.hlsl";
   
   OGLVertexShaderFile = "shaders/common/gl/cloudLayerV.glsl";
   OGLPixelShaderFile = "shaders/common/gl/cloudLayerP.glsl";
      
   pixVersion = 2.0;   
};

//------------------------------------------------------------------------------
// BasicClouds
//------------------------------------------------------------------------------

singleton ShaderData( BasicCloudsShader )
{
   DXVertexShaderFile   = "shaders/common/basicCloudsV.hlsl";
   DXPixelShaderFile    = "shaders/common/basicCloudsP.hlsl";
   
   //OGLVertexShaderFile = "shaders/common/gl/basicCloudsV.glsl";
   //OGLPixelShaderFile = "shaders/common/gl/basicCloudsP.glsl";
      
   pixVersion = 2.0;   
};
