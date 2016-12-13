#ifndef GFX_GL_VERTEX_ATTRIB_LOCATION_H
#define GFX_GL_VERTEX_ATTRIB_LOCATION_H

namespace Torque
{
   enum GL_AttributeLocation
   {
      GL_VertexAttrib_Position = 0,
      GL_VertexAttrib_Normal,
      GL_VertexAttrib_Color,
      GL_VertexAttrib_Tangent,
      GL_VertexAttrib_TangentW,
      GL_VertexAttrib_Binormal,
      GL_VertexAttrib_TexCoord0,
      GL_VertexAttrib_TexCoord1,
      GL_VertexAttrib_TexCoord2,
      GL_VertexAttrib_TexCoord3,
      GL_VertexAttrib_TexCoord4,
      GL_VertexAttrib_TexCoord5,
      GL_VertexAttrib_TexCoord6,
      GL_VertexAttrib_TexCoord7,
      GL_VertexAttrib_TexCoord8,
      GL_VertexAttrib_TexCoord9,
      GL_VertexAttrib_COUNT,

      GL_VertexAttrib_LAST = GL_VertexAttrib_TexCoord9,
      GL_VertexAttrib_BlendWeight0 = GL_VertexAttrib_TexCoord6,
      GL_VertexAttrib_BlendIndex0 = GL_VertexAttrib_TexCoord2,
   };
}


#endif //GFX_GL_VERTEX_ATTRIB_LOCATION_H