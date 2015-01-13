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
// Structures                                                                  
//-----------------------------------------------------------------------------
struct VertData
{
   float2 texCoord   : TEXCOORD0;
   float2 waveScale	: TEXCOORD1;
   float3 normal     : NORMAL;
   float4 position   : POSITION;
};

struct ConnectData
{
   float4 hpos            : POSITION;
   float2 outTexCoord     : TEXCOORD0;
   float4 color			  : COLOR0;
   float4 groundAlphaCoeff : COLOR1;
	float2 alphaLookup	  : TEXCOORD1;   
};

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
ConnectData main( VertData IN,
                  uniform float4x4 projection     		: register(C0),
                  uniform float4x4 world	   					: register(C4),
                  uniform float    GlobalSwayPhase 		: register(C8),
                  uniform float 	 SwayMagnitudeSide 	: register(C9),
                  uniform float 	 SwayMagnitudeFront	: register(C10),
                  uniform float    GlobalLightPhase		: register(C11),
                  uniform float    LuminanceMagnitude : register(C12),
                  uniform float		 LuminanceMidpoint	: register(C13),
                  uniform float	DistanceRange : register(C14),
                  uniform float3 CameraPos : register(C15),
                  uniform float TrueBillboard : register(C16)
)
{
  ConnectData OUT;

	// Init a transform matrix to be used in the following steps
	float4x4 trans = 0;
	trans[0][0] = 1;
	trans[1][1] = 1;
	trans[2][2] = 1;
	trans[3][3] = 1;
	trans[0][3] = IN.position.x;
	trans[1][3] = IN.position.y;
	trans[2][3] = IN.position.z;
	
	// Billboard transform * world matrix
	float4x4 o = world;
	o = mul(o, trans);
		
	// Keep only the up axis result and position transform.
	// This gives us "cheating" cylindrical billboarding.     
   o[0][0] = 1;
   o[1][0] = 0;
   o[2][0] = 0;
   o[3][0] = 0;
   o[0][1] = 0;
   o[1][1] = 1;
   o[2][1] = 0;
   o[3][1] = 0;
   
   // Unless the user specified TrueBillboard,
   // in which case we want the z axis to also be camera facing.      
   
#ifdef TRUE_BILLBOARD

   o[0][2] = 0;
   o[1][2] = 0;
   o[2][2] = 1;
   o[3][2] = 0;
   
#endif
   
	// Handle sway. Sway is stored in a texture coord. The x coordinate is the sway phase multiplier, 
	// the y coordinate determines if this vertex actually sways or not.
	float xSway, ySway;
	float wavePhase = GlobalSwayPhase * IN.waveScale.x;
	sincos(wavePhase, ySway, xSway);
	xSway = xSway * IN.waveScale.y * SwayMagnitudeSide;
	ySway = ySway * IN.waveScale.y * SwayMagnitudeFront;
	float4 p;	
	p = mul(o, float4(IN.normal.x + xSway, ySway, IN.normal.z, 1));
		
	// Project the point	
	OUT.hpos = mul(projection, p);
	
	// Lighting 
	float Luminance = LuminanceMidpoint + LuminanceMagnitude * cos(GlobalLightPhase + IN.normal.y);

	// Alpha
	float3 worldPos = float3(IN.position.x, IN.position.y, IN.position.z);
	float alpha = abs(distance(worldPos, CameraPos)) / DistanceRange;			
	alpha = clamp(alpha, 0.0f, 1.0f); //pass it through	

	OUT.alphaLookup = float2(alpha, 0.0f);
	OUT.groundAlphaCoeff = all(IN.normal.z);
	OUT.outTexCoord = IN.texCoord;	
	OUT.color = float4(Luminance, Luminance, Luminance, 1.0f);

	return OUT;
}