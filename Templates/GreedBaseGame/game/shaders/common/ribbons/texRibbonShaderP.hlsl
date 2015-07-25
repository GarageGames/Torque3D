#define IN_HLSL
#include "../shdrConsts.h"
#include "../torque.hlsl"
 
uniform sampler2D ribTex : register(S0);
 
struct v2f
{
           
   float2 texCoord        : TEXCOORD0;
   float2 shiftdata       : TEXCOORD1;
   float4 color           : COLOR0;
};
 
float4 main(v2f IN) : COLOR0
{
    float4 Tex = tex2D(ribTex,IN.texCoord);
	Tex.a *= IN.color.a;
	return hdrEncode(Tex);
}