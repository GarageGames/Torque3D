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

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------
in vec4 vPosition;
in vec3 vNormal;
in vec4 vColor;
in vec2 vTexCoord0;
in vec2 vTexCoord1;
in vec2 vTexCoord2;

uniform mat4 projection, world;
uniform vec3 CameraPos;
uniform float GlobalSwayPhase, SwayMagnitudeSide, SwayMagnitudeFront,
              GlobalLightPhase, LuminanceMagnitude, LuminanceMidpoint, DistanceRange;
              
out vec4 color, groundAlphaCoeff;
out vec2 outTexCoord, alphaLookup;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
void main()
{
	// Init a transform matrix to be used in the following steps
	mat4 trans = mat4(0.0);
	trans[0][0] = 1.0;
	trans[1][1] = 1.0;
	trans[2][2] = 1.0;
	trans[3][3] = 1.0;
	trans[3][0] = vPosition.x;
	trans[3][1] = vPosition.y;
	trans[3][2] = vPosition.z;
	
	// Billboard transform * world matrix
	mat4 o = world;
	o = o * trans;
		
	// Keep only the up axis result and position transform.
	// This gives us "cheating" cylindrical billboarding.
	o[0][0] = 1.0;
	o[0][1] = 0.0;
	o[0][2] = 0.0;
	o[0][3] = 0.0;
	o[1][0] = 0.0;
	o[1][1] = 1.0;
	o[1][2] = 0.0;
	o[1][3] = 0.0;
	
	// Handle sway. Sway is stored in a texture coord. The x coordinate is the sway phase multiplier, 
	// the y coordinate determines if this vertex actually sways or not.
	float xSway, ySway;
	float wavePhase = GlobalSwayPhase * vTexCoord1.x;
	ySway = sin(wavePhase);
	xSway = cos(wavePhase);
	xSway = xSway * vTexCoord1.y * SwayMagnitudeSide;
	ySway = ySway * vTexCoord1.y * SwayMagnitudeFront;
	vec4 p;
	p = o * vec4(vNormal.x + xSway, ySway, vNormal.z, 1.0);
		
	// Project the point	
	gl_Position = projection * p;
	
	// Lighting 
	float Luminance = LuminanceMidpoint + LuminanceMagnitude * cos(GlobalLightPhase + vNormal.y);

	// Alpha
	vec3 worldPos = vec3(vPosition.x, vPosition.y, vPosition.z);
	float alpha = abs(distance(worldPos, CameraPos)) / DistanceRange;			
	alpha = clamp(alpha, 0.0, 1.0); //pass it through	

	alphaLookup = vec2(alpha, 0.0);
	bool alphaCoeff = bool(vNormal.z);
   groundAlphaCoeff = vec4(float(alphaCoeff));
	outTexCoord = vTexCoord0.st;	
	color = vec4(Luminance, Luminance, Luminance, 1.0);
   gl_Position.y *= -1;
}