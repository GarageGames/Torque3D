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

#include "../../../gl/hlslCompat.glsl"
#include "../../gl/postFX.glsl"

uniform sampler2D frameSampler;
uniform sampler2D backBuffer;

uniform vec3 camForward;
uniform vec3 lightDirection;
uniform vec2 screenSunPos;
uniform vec2 oneOverTargetSize;
uniform int numSamples;
uniform float density;
uniform float weight;
uniform float decay;
uniform float exposure;

out vec4 OUT_col;

void main()
{  
    vec4 texCoord = vec4( IN_uv0.xy, 0, 0 );        
    
    // Store initial sample.
    half3 color = half3(texture( frameSampler, texCoord.xy ).rgb);  
	
	// Store original bb color.
	vec4 bbCol = texture( backBuffer, IN_uv1 );

    // Set up illumination decay factor.
    half illuminationDecay = 1.0;  		
	
	float amount = saturate( dot( -lightDirection, camForward ) );
		
	int samples = int(numSamples * amount);
	
	if ( samples <= 0 )
   {
	   OUT_col = bbCol;
      return;
   }

	// Calculate vector from pixel to light source in screen space.
    half2 deltaTexCoord = half2( texCoord.xy - screenSunPos );  

    // Divide by number of samples and scale by control factor.  
    deltaTexCoord *= 1.0 / half(samples * density); 
	
    // Evaluate summation from Equation 3 NUM_SAMPLES iterations.  
    for ( int i = 0; i < samples; i++ )  
    {  
        // Step sample location along ray.
        texCoord.xy -= deltaTexCoord;  

        // Retrieve sample at new location.
        half3 sample_ = half3(tex2Dlod( frameSampler, texCoord ));  

        // Apply sample attenuation scale/decay factors.
        sample_ *= illuminationDecay * weight;

        // Accumulate combined color.
        color += sample_;

        // Update exponential decay factor.
        illuminationDecay *= decay;
    }       
   
    //return saturate( amount ) * color * Exposure;
	//return bbCol * decay;
	
    // Output final color with a further scale control factor.      
    OUT_col = saturate( amount ) * vec4( color * exposure, 1 ) + bbCol;
}  
