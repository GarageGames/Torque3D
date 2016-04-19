#include "../../gl/hlslCompat.glsl"
 
in float4 _hpos;
in float2 _texCoord;
in float2 _shiftdata;
in float4 _color;
 
#define IN_hpos _hpos
#define IN_texCoord _texCoord
#define IN_shiftdata _shiftdata
#define IN_color _color
 
out float4 OUT_col;
 
void main()
{
   float fade = 1.0 - abs(IN_shiftdata.y - 0.5) * 2.0;
   OUT_col.xyz = IN_color.xyz + pow(fade, 4) / 10;
   OUT_col.a = IN_color.a * fade;
}