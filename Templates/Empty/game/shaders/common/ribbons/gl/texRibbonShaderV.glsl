#include "../../gl/hlslCompat.glsl"
 
in float2 vTexCoord0;
in float2 vTexCoord1;
in float3 vNormal;
in float4 vPosition;
in float4 vColor;
 
#define IN_texCoord vTexCoord0
#define IN_shiftdata vTexCoord1
#define IN_normal vNormal
#define IN_position vPosition
#define IN_color vColor
 
out float4 _hpos;
out float2 _texCoord;
out float2 _shiftdata;
out float4 _color;
 
#define OUT_hpos _hpos
#define OUT_texCoord _texCoord
#define OUT_shiftdata _shiftdata
#define OUT_color _color
 
uniform float4x4 modelview;
uniform float3   eyePos;
 
void main()
{
    OUT_hpos = tMul(modelview, IN_position);
    OUT_color = IN_color;
    OUT_texCoord = IN_texCoord;
    OUT_shiftdata = IN_shiftdata;
   
    gl_Position = OUT_hpos;
    correctSSP(gl_Position);
}