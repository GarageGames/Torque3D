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


float attenuate( vec4 lightColor, vec2 attParams, float dist )
{
	// We're summing the results of a scaled constant,
	// linear, and quadratic attenuation.

	#ifdef ACCUMULATE_LUV
		return lightColor.w * ( 1.0 - dot( attParams, vec2( dist, dist * dist ) ) );
	#else
		return 1.0 - dot( attParams, vec2( dist, dist * dist ) );
	#endif
}

// Calculate the specular coefficent
//
//	pxlToLight - Normalized vector representing direction from the pixel being lit, to the light source, in world space
//	normal - Normalized surface normal
//	pxlToEye - Normalized vector representing direction from pixel being lit, to the camera, in world space
//	specPwr - Specular exponent
//	specularScale - A scalar on the specular output used in RGB accumulation.
//
float calcSpecular( vec3 pxlToLight, vec3 normal, vec3 pxlToEye, float specPwr, float specularScale )
{
#ifdef PHONG_SPECULAR 
   // (R.V)^c
   float specVal = dot( normalize( -reflect( pxlToLight, normal ) ), pxlToEye );
#else
   // (N.H)^c   [Blinn-Phong, TGEA style, default]
   float specVal = dot( normal, normalize( pxlToLight + pxlToEye ) );
#endif

#ifdef ACCUMULATE_LUV
   return pow( max( specVal, 0.00001f ), specPwr );
#else
   // If this is RGB accumulation, than there is no facility for the luminance
   // of the light to play in to the specular intensity. In LUV, the luminance
   // of the light color gets rolled into N.L * Attenuation
   return specularScale * pow( max( specVal, 0.00001f ), specPwr );
#endif
}

vec3 getDistanceVectorToPlane( vec3 origin, vec3 direction, vec4 plane )
{
   float denum = dot( plane.xyz, direction.xyz );
   float num = dot( plane, vec4( origin, 1.0 ) );
   float t = -num / denum;

   return direction.xyz * t;
}

vec3 getDistanceVectorToPlane( float negFarPlaneDotEye, vec3 direction, vec4 plane )
{
   float denum = dot( plane.xyz, direction.xyz );
   float t = negFarPlaneDotEye / denum;

   return direction.xyz * t;
}