#define IN_HLSL
#include "../shdrConsts.h"
 
struct a2v
{
        float2 texCoord        : TEXCOORD0;
        float2 shiftdata       : TEXCOORD1;
        float3 normal          : NORMAL;
        float4 position        : POSITION;
        float4 color           : COLOR0;
};
 
struct v2f
{
        float4 hpos            : POSITION;
        float2 texCoord        : TEXCOORD0;
        float2 shiftdata       : TEXCOORD1;
        float4 color           : COLOR0;
};
 
uniform float4x4 modelview;
uniform float3   eyePos;
 
v2f main(a2v IN)
{
    v2f OUT;
 
    OUT.hpos = mul(modelview, IN.position);
    OUT.color = IN.color;
    OUT.texCoord = IN.texCoord;
    OUT.shiftdata = IN.shiftdata;
   
    return OUT;
}