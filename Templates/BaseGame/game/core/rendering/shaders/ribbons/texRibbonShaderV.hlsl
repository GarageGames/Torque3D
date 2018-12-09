#define IN_HLSL
#include "../shdrConsts.h"
#include "../shaderModel.hlsl"

struct a2v
{
   float3 position        : POSITION;
   float4 color           : COLOR0;
   float3 normal          : NORMAL;   
   float2 texCoord        : TEXCOORD0;
   float2 shiftdata       : TEXCOORD1;
};
 
struct v2f
{
   float4 hpos            : TORQUE_POSITION;
   float4 color           : COLOR0;
   float2 texCoord        : TEXCOORD0;
   float2 shiftdata       : TEXCOORD1;   
};
 
uniform float4x4 modelview;
uniform float3   eyePos;
 
v2f main(a2v IN)
{
    v2f OUT;
 
    OUT.hpos = mul(modelview, float4(IN.position,1.0));
    OUT.color = IN.color;
    OUT.texCoord = IN.texCoord;
    OUT.shiftdata = IN.shiftdata;
   
    return OUT;
}