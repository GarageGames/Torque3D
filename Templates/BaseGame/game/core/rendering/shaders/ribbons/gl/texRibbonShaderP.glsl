#include "../../gl/hlslCompat.glsl"
#include "../../gl/torque.glsl"

in float4 _hpos;
in float2 _texCoord;
in float2 _shiftdata;
in float4 _color;
 
#define IN_hpos _hpos
#define IN_texCoord _texCoord
#define IN_shiftdata _shiftdata
#define IN_color _color
 
out float4 OUT_col;
uniform sampler2D ribTex;

void main()
{
    float4 Tex = tex2D(ribTex,IN_texCoord);
	Tex.a *= IN_color.a;
	OUT_col = hdrEncode(Tex);
}