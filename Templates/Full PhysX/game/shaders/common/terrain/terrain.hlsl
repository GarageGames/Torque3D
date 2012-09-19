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


float calcBlend( float texId, float2 layerCoord, float layerSize, float4 layerSample )
{
   // This is here to disable the blend if none of 
   // the neighbors equal the current id.
   //
   // We depend on the input layer samples being 
   // rounded to the correct integer ids.
   //
   float4 diff = saturate( abs( layerSample - texId ) );
   float noBlend = any( 1 - diff );

   // Use step to see if any of the layer samples 
   // match the current texture id.
   float4 factors = step( texId, layerSample );

   // This is a custom bilinear filter.

   float2 uv = layerCoord * layerSize;
   float2 xy = floor( uv );
   float2 ratio = uv - xy;
   float2 opposite = 1 - ratio;

   // NOTE: This will optimize down to two lerp operations.
   float blend = ( factors.b * opposite.x + factors.g * ratio.x ) * opposite.y +
                 ( factors.r * opposite.x + factors.a * ratio.x ) * ratio.y;

   return noBlend * blend;
}
