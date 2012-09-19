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

#ifdef GL_VERSION_1_2
GL_GROUP_BEGIN(GL_VERSION_1_2)
GL_FUNCTION(glDrawRangeElements ,void, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices))
GL_FUNCTION(glTexImage3D, void, (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels))
GL_FUNCTION(glTexSubImage3D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels))
GL_FUNCTION(glCopyTexSubImage3D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height))
GL_GROUP_END()
#endif

#ifdef GL_VERSION_1_3
GL_GROUP_BEGIN(GL_VERSION_1_3)
GL_FUNCTION(glActiveTexture,void,(GLenum texture))
GL_FUNCTION(glClientActiveTexture,void,(GLenum texture))
GL_FUNCTION(glCompressedTexImage1D,void,(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data))
GL_FUNCTION(glCompressedTexImage2D,void,(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data))
GL_FUNCTION(glCompressedTexImage3D,void,(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data))
GL_FUNCTION(glCompressedTexSubImage1D,void,(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data))
GL_FUNCTION(glCompressedTexSubImage2D,void,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data))
GL_FUNCTION(glCompressedTexSubImage3D,void,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data))
GL_FUNCTION(glGetCompressedTexImage,void,(GLenum target, GLint lod, GLvoid *img))
GL_FUNCTION(glLoadTransposeMatrixd,void,(const GLdouble m[16]))
GL_FUNCTION(glLoadTransposeMatrixf,void,(const GLfloat m[16]))
GL_FUNCTION(glMultTransposeMatrixd,void,(const GLdouble m[16]))
GL_FUNCTION(glMultTransposeMatrixf,void,(const GLfloat m[16]))
GL_FUNCTION(glMultiTexCoord1d,void,(GLenum target, GLdouble s))
GL_FUNCTION(glMultiTexCoord1dv,void,(GLenum target, const GLdouble *v))
GL_FUNCTION(glMultiTexCoord1f,void,(GLenum target, GLfloat s))
GL_FUNCTION(glMultiTexCoord1fv,void,(GLenum target, const GLfloat *v))
GL_FUNCTION(glMultiTexCoord1i,void,(GLenum target, GLint s))
GL_FUNCTION(glMultiTexCoord1iv,void,(GLenum target, const GLint *v))
GL_FUNCTION(glMultiTexCoord1s,void,(GLenum target, GLshort s))
GL_FUNCTION(glMultiTexCoord1sv,void,(GLenum target, const GLshort *v))
GL_FUNCTION(glMultiTexCoord2d,void,(GLenum target, GLdouble s, GLdouble t))
GL_FUNCTION(glMultiTexCoord2dv,void,(GLenum target, const GLdouble *v))
GL_FUNCTION(glMultiTexCoord2f,void,(GLenum target, GLfloat s, GLfloat t))
GL_FUNCTION(glMultiTexCoord2fv,void,(GLenum target, const GLfloat *v))
GL_FUNCTION(glMultiTexCoord2i,void,(GLenum target, GLint s, GLint t))
GL_FUNCTION(glMultiTexCoord2iv,void,(GLenum target, const GLint *v))
GL_FUNCTION(glMultiTexCoord2s,void,(GLenum target, GLshort s, GLshort t))
GL_FUNCTION(glMultiTexCoord2sv,void,(GLenum target, const GLshort *v))
GL_FUNCTION(glMultiTexCoord3d,void,(GLenum target, GLdouble s, GLdouble t, GLdouble r))
GL_FUNCTION(glMultiTexCoord3dv,void,(GLenum target, const GLdouble *v))
GL_FUNCTION(glMultiTexCoord3f,void,(GLenum target, GLfloat s, GLfloat t, GLfloat r))
GL_FUNCTION(glMultiTexCoord3fv,void,(GLenum target, const GLfloat *v))
GL_FUNCTION(glMultiTexCoord3i,void,(GLenum target, GLint s, GLint t, GLint r))
GL_FUNCTION(glMultiTexCoord3iv,void,(GLenum target, const GLint *v))
GL_FUNCTION(glMultiTexCoord3s,void,(GLenum target, GLshort s, GLshort t, GLshort r))
GL_FUNCTION(glMultiTexCoord3sv,void,(GLenum target, const GLshort *v))
GL_FUNCTION(glMultiTexCoord4d,void,(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q))
GL_FUNCTION(glMultiTexCoord4dv,void,(GLenum target, const GLdouble *v))
GL_FUNCTION(glMultiTexCoord4f,void,(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q))
GL_FUNCTION(glMultiTexCoord4fv,void,(GLenum target, const GLfloat *v))
GL_FUNCTION(glMultiTexCoord4i,void,(GLenum target, GLint s, GLint t, GLint r, GLint q))
GL_FUNCTION(glMultiTexCoord4iv,void,(GLenum target, const GLint *v))
GL_FUNCTION(glMultiTexCoord4s,void,(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q))
GL_FUNCTION(glMultiTexCoord4sv,void,(GLenum target, const GLshort *v))
GL_FUNCTION(glSampleCoverage,void,(GLclampf value, GLboolean invert))
GL_GROUP_END()
#endif

#ifdef GL_VERSION_1_4
GL_GROUP_BEGIN(GL_VERSION_1_4)
GL_FUNCTION(glBlendEquation,void,(GLenum mode))
GL_FUNCTION(glBlendColor,void,(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha))
GL_FUNCTION(glFogCoordf,void,(GLfloat coord))
GL_FUNCTION(glFogCoordfv,void,(const GLfloat *coord))
GL_FUNCTION(glFogCoordd,void,(GLdouble coord))
GL_FUNCTION(glFogCoorddv,void,(const GLdouble *coord))
GL_FUNCTION(glFogCoordPointer,void,(GLenum type, GLsizei stride, const GLvoid *pointer))
GL_FUNCTION(glMultiDrawArrays,void,(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount))
GL_FUNCTION(glMultiDrawElements,void,(GLenum mode, GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount))
GL_FUNCTION(glPointParameterf,void,(GLenum pname, GLfloat param))
GL_FUNCTION(glPointParameterfv,void,(GLenum pname, GLfloat *params))
GL_FUNCTION(glSecondaryColor3b,void,(GLbyte red, GLbyte green, GLbyte blue))
GL_FUNCTION(glSecondaryColor3bv,void,(const GLbyte *v))
GL_FUNCTION(glSecondaryColor3d,void,(GLdouble red, GLdouble green, GLdouble blue))
GL_FUNCTION(glSecondaryColor3dv,void,(const GLdouble *v))
GL_FUNCTION(glSecondaryColor3f,void,(GLfloat red, GLfloat green, GLfloat blue))
GL_FUNCTION(glSecondaryColor3fv,void,(const GLfloat *v))
GL_FUNCTION(glSecondaryColor3i,void,(GLint red, GLint green, GLint blue))
GL_FUNCTION(glSecondaryColor3iv,void,(const GLint *v))
GL_FUNCTION(glSecondaryColor3s,void,(GLshort red, GLshort green, GLshort blue))
GL_FUNCTION(glSecondaryColor3sv,void,(const GLshort *v))
GL_FUNCTION(glSecondaryColor3ub,void,(GLubyte red, GLubyte green, GLubyte blue))
GL_FUNCTION(glSecondaryColor3ubv,void,(const GLubyte *v))
GL_FUNCTION(glSecondaryColor3ui,void,(GLuint red, GLuint green, GLuint blue))
GL_FUNCTION(glSecondaryColor3uiv,void,(const GLuint *v))
GL_FUNCTION(glSecondaryColor3us,void,(GLushort red, GLushort green, GLushort blue))
GL_FUNCTION(glSecondaryColor3usv,void,(const GLushort *v))
GL_FUNCTION(glSecondaryColorPointer,void,(GLint size, GLenum type, GLsizei stride, GLvoid *pointer))
GL_FUNCTION(glBlendFuncSeparate,void,(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha))
GL_FUNCTION(glWindowPos2d,void,(GLdouble x, GLdouble y))
GL_FUNCTION(glWindowPos2f,void,(GLfloat x, GLfloat y))
GL_FUNCTION(glWindowPos2i,void,(GLint x, GLint y))
GL_FUNCTION(glWindowPos2s,void,(GLshort x, GLshort y))
GL_FUNCTION(glWindowPos2dv,void,(const GLdouble *p))
GL_FUNCTION(glWindowPos2fv,void,(const GLfloat *p))
GL_FUNCTION(glWindowPos2iv,void,(const GLint *p))
GL_FUNCTION(glWindowPos2sv,void,(const GLshort *p))
GL_FUNCTION(glWindowPos3d,void,(GLdouble x, GLdouble y, GLdouble z))
GL_FUNCTION(glWindowPos3f,void,(GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glWindowPos3i,void,(GLint x, GLint y, GLint z))
GL_FUNCTION(glWindowPos3s,void,(GLshort x, GLshort y, GLshort z))
GL_FUNCTION(glWindowPos3dv,void,(const GLdouble *p))
GL_FUNCTION(glWindowPos3fv,void,(const GLfloat *p))
GL_FUNCTION(glWindowPos3iv,void,(const GLint *p))
GL_FUNCTION(glWindowPos3sv,void,(const GLshort *p))
GL_GROUP_END()
#endif

#ifdef GL_VERSION_1_5
GL_GROUP_BEGIN(GL_VERSION_1_5)
GL_FUNCTION(glGenQueries,void,(GLsizei n, GLuint* ids))
GL_FUNCTION(glDeleteQueries,void,(GLsizei n, const GLuint* ids))
GL_FUNCTION(glIsQuery,GLboolean,(GLuint id))
GL_FUNCTION(glBeginQuery,void,(GLenum target, GLuint id))
GL_FUNCTION(glEndQuery,void,(GLenum target))
GL_FUNCTION(glGetQueryiv,void,(GLenum target, GLenum pname, GLint* params))
GL_FUNCTION(glGetQueryObjectiv,void,(GLuint id, GLenum pname, GLint* params))
GL_FUNCTION(glGetQueryObjectuiv,void,(GLuint id, GLenum pname, GLuint* params))
GL_FUNCTION(glBindBuffer,void,(GLenum target, GLuint buffer))
GL_FUNCTION(glDeleteBuffers,void,(GLsizei n, const GLuint* buffers))
GL_FUNCTION(glGenBuffers,void,(GLsizei n, GLuint* buffers))
GL_FUNCTION(glIsBuffer,GLboolean,(GLuint buffer))
GL_FUNCTION(glBufferData,void,(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage))
GL_FUNCTION(glBufferSubData,void,(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data))
GL_FUNCTION(glGetBufferSubData,void,(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data))
GL_FUNCTION(glMapBuffer,GLvoid*,(GLenum target, GLenum access))
GL_FUNCTION(glUnmapBuffer,GLboolean,(GLenum target))
GL_FUNCTION(glGetBufferParameteriv,void,(GLenum target, GLenum pname, GLint* params))
GL_FUNCTION(glGetBufferPointerv,void,(GLenum target, GLenum pname, GLvoid** params))
GL_GROUP_END()
#endif

#ifdef GL_VERSION_2_0
GL_GROUP_BEGIN(GL_VERSION_2_0)
GL_FUNCTION(glBlendEquationSeparate,void,(GLenum, GLenum))
GL_FUNCTION(glDrawBuffers,void,(GLsizei n, const GLenum* bufs))
GL_FUNCTION(glStencilOpSeparate,void,(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass))
GL_FUNCTION(glStencilFuncSeparate,void,(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask))
GL_FUNCTION(glStencilMaskSeparate,void,(GLenum, GLuint))
GL_FUNCTION(glAttachShader,void,(GLuint program, GLuint shader))
GL_FUNCTION(glBindAttribLocation,void,(GLuint program, GLuint index, const GLchar* name))
GL_FUNCTION(glCompileShader,void,(GLuint shader))
GL_FUNCTION(glCreateProgram,GLuint,(void))
GL_FUNCTION(glCreateShader,GLuint,(GLenum type))
GL_FUNCTION(glDeleteProgram,void,(GLuint program))
GL_FUNCTION(glDeleteShader,void,(GLuint shader))
GL_FUNCTION(glDetachShader,void,(GLuint program, GLuint shader))
GL_FUNCTION(glDisableVertexAttribArray,void,(GLuint))
GL_FUNCTION(glEnableVertexAttribArray,void,(GLuint))
GL_FUNCTION(glGetActiveAttrib,void,(GLuint program, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLchar* name))
GL_FUNCTION(glGetActiveUniform,void,(GLuint program, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLchar* name))
GL_FUNCTION(glGetAttachedShaders,void,(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders))
GL_FUNCTION(glGetAttribLocation,GLint,(GLuint program, const GLchar* name))
GL_FUNCTION(glGetProgramiv,void,(GLuint program, GLenum pname, GLint* param))
GL_FUNCTION(glGetProgramInfoLog,void,(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog))
GL_FUNCTION(glGetShaderiv,void,(GLuint shader, GLenum pname, GLint* param))
GL_FUNCTION(glGetShaderInfoLog,void,(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog))
GL_FUNCTION(glShaderSource,void,(GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths))
GL_FUNCTION(glGetUniformLocation,GLint,(GLint programObj, const GLchar* name))
GL_FUNCTION(glGetUniformfv,void,(GLuint program, GLint location, GLfloat* params))
GL_FUNCTION(glGetUniformiv,void,(GLuint program, GLint location, GLint* params))
GL_FUNCTION(glGetVertexAttribdv,void,(GLuint, GLenum, GLdouble*))
GL_FUNCTION(glGetVertexAttribfv,void,(GLuint, GLenum, GLfloat*))
GL_FUNCTION(glGetVertexAttribiv,void,(GLuint, GLenum, GLint*))
GL_FUNCTION(glGetVertexAttribPointerv,void,(GLuint, GLenum, GLvoid*))
GL_FUNCTION(glIsProgram,GLboolean,(GLuint program))
GL_FUNCTION(glIsShader,GLboolean,(GLuint shader))
GL_FUNCTION(glLinkProgram,void,(GLuint program))
GL_FUNCTION(glGetShaderSource,void,(GLint obj, GLsizei maxLength, GLsizei* length, GLchar* source))
GL_FUNCTION(glUseProgram,void,(GLuint program))
GL_FUNCTION(glUniform1f,void,(GLint location, GLfloat v0))
GL_FUNCTION(glUniform1fv,void,(GLint location, GLsizei count, const GLfloat* value))
GL_FUNCTION(glUniform1i,void,(GLint location, GLint v0))
GL_FUNCTION(glUniform1iv,void,(GLint location, GLsizei count, const GLint* value))
GL_FUNCTION(glUniform2f,void,(GLint location, GLfloat v0, GLfloat v1))
GL_FUNCTION(glUniform2fv,void,(GLint location, GLsizei count, const GLfloat* value))
GL_FUNCTION(glUniform2i,void,(GLint location, GLint v0, GLint v1))
GL_FUNCTION(glUniform2iv,void,(GLint location, GLsizei count, const GLint* value))
GL_FUNCTION(glUniform3f,void,(GLint location, GLfloat v0, GLfloat v1, GLfloat v2))
GL_FUNCTION(glUniform3fv,void,(GLint location, GLsizei count, const GLfloat* value))
GL_FUNCTION(glUniform3i,void,(GLint location, GLint v0, GLint v1, GLint v2))
GL_FUNCTION(glUniform3iv,void,(GLint location, GLsizei count, const GLint* value))
GL_FUNCTION(glUniform4f,void,(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3))
GL_FUNCTION(glUniform4fv,void,(GLint location, GLsizei count, const GLfloat* value))
GL_FUNCTION(glUniform4i,void,(GLint location, GLint v0, GLint v1, GLint v2, GLint v3))
GL_FUNCTION(glUniform4iv,void,(GLint location, GLsizei count, const GLint* value))
GL_FUNCTION(glUniformMatrix2fv,void,(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value))
GL_FUNCTION(glUniformMatrix3fv,void,(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value))
GL_FUNCTION(glUniformMatrix4fv,void,(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value))
GL_FUNCTION(glValidateProgram,void,(GLuint program))
GL_FUNCTION(glVertexAttrib1d,void,(GLuint index, GLdouble x))
GL_FUNCTION(glVertexAttrib1dv,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib1f,void,(GLuint index, GLfloat x))
GL_FUNCTION(glVertexAttrib1fv,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib1s,void,(GLuint index, GLshort x))
GL_FUNCTION(glVertexAttrib1sv,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib2d,void,(GLuint index, GLdouble x, GLdouble y))
GL_FUNCTION(glVertexAttrib2dv,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib2f,void,(GLuint index, GLfloat x, GLfloat y))
GL_FUNCTION(glVertexAttrib2fv,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib2s,void,(GLuint index, GLshort x, GLshort y))
GL_FUNCTION(glVertexAttrib2sv,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib3d,void,(GLuint index, GLdouble x, GLdouble y, GLdouble z))
GL_FUNCTION(glVertexAttrib3dv,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib3f,void,(GLuint index, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glVertexAttrib3fv,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib3s,void,(GLuint index, GLshort x, GLshort y, GLshort z))
GL_FUNCTION(glVertexAttrib3sv,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib4Nbv,void,(GLuint index, const GLbyte* v))
GL_FUNCTION(glVertexAttrib4Niv,void,(GLuint index, const GLint* v))
GL_FUNCTION(glVertexAttrib4Nsv,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib4Nub,void,(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w))
GL_FUNCTION(glVertexAttrib4Nubv,void,(GLuint index, const GLubyte* v))
GL_FUNCTION(glVertexAttrib4Nuiv,void,(GLuint index, const GLuint* v))
GL_FUNCTION(glVertexAttrib4Nusv,void,(GLuint index, const GLushort* v))
GL_FUNCTION(glVertexAttrib4bv,void,(GLuint index, const GLbyte* v))
GL_FUNCTION(glVertexAttrib4d,void,(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_FUNCTION(glVertexAttrib4dv,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib4f,void,(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_FUNCTION(glVertexAttrib4fv,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib4iv,void,(GLuint index, const GLint* v))
GL_FUNCTION(glVertexAttrib4s,void,(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w))
GL_FUNCTION(glVertexAttrib4sv,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib4ubv,void,(GLuint index, const GLubyte* v))
GL_FUNCTION(glVertexAttrib4uiv,void,(GLuint index, const GLuint* v))
GL_FUNCTION(glVertexAttrib4usv,void,(GLuint index, const GLushort* v))
GL_FUNCTION(glVertexAttribPointer,void,(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer))
GL_GROUP_END()
#endif

#ifdef GL_3DFX_multisample
GL_GROUP_BEGIN(GL_3DFX_multisample)
GL_GROUP_END()
#endif

#ifdef GL_3DFX_tbuffer
GL_GROUP_BEGIN(GL_3DFX_tbuffer)
GL_FUNCTION(glTbufferMask3DFX,void,(GLuint mask))
GL_GROUP_END()
#endif

#ifdef GL_3DFX_texture_compression_FXT1
GL_GROUP_BEGIN(GL_3DFX_texture_compression_FXT1)
GL_GROUP_END()
#endif

#ifdef GL_APPLE_client_storage
GL_GROUP_BEGIN(GL_APPLE_client_storage)
GL_GROUP_END()
#endif

#ifdef GL_APPLE_element_array
GL_GROUP_BEGIN(GL_APPLE_element_array)
GL_FUNCTION(glDrawElementArrayAPPLE,void,(GLenum mode, GLint first, GLsizei count))
GL_FUNCTION(glDrawRangeElementArrayAPPLE,void,(GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count))
GL_FUNCTION(glElementPointerAPPLE,void,(GLenum type, const void* pointer))
GL_FUNCTION(glMultiDrawElementArrayAPPLE,void,(GLenum mode, const GLint* first, const GLsizei *count, GLsizei primcount))
GL_FUNCTION(glMultiDrawRangeElementArrayAPPLE,void,(GLenum mode, GLuint start, GLuint end, const GLint* first, const GLsizei *count, GLsizei primcount))
GL_GROUP_END()
#endif

#ifdef GL_APPLE_fence
GL_GROUP_BEGIN(GL_APPLE_fence)
GL_FUNCTION(glDeleteFencesAPPLE,void,(GLsizei n, const GLuint* fences))
GL_FUNCTION(glFinishFenceAPPLE,void,(GLuint fence))
GL_FUNCTION(glFinishObjectAPPLE,void,(GLenum object, GLint name))
GL_FUNCTION(glGenFencesAPPLE,void,(GLsizei n, GLuint* fences))
GL_FUNCTION(glIsFenceAPPLE,GLboolean,(GLuint fence))
GL_FUNCTION(glSetFenceAPPLE,void,(GLuint fence))
GL_FUNCTION(glTestFenceAPPLE,GLboolean,(GLuint fence))
GL_FUNCTION(glTestObjectAPPLE,GLboolean,(GLenum object, GLuint name))
GL_GROUP_END()
#endif

#ifdef GL_APPLE_float_pixels
GL_GROUP_BEGIN(GL_APPLE_float_pixels)
GL_GROUP_END()
#endif

#ifdef GL_APPLE_pixel_buffer
GL_GROUP_BEGIN(GL_APPLE_pixel_buffer)
GL_GROUP_END()
#endif

#ifdef GL_APPLE_specular_vector
GL_GROUP_BEGIN(GL_APPLE_specular_vector)
GL_GROUP_END()
#endif

#ifdef GL_APPLE_texture_range
GL_GROUP_BEGIN(GL_APPLE_texture_range)
GL_FUNCTION(glTextureRangeAPPLE,void,(GLenum target, GLsizei length, GLvoid *pointer))
GL_FUNCTION(glGetTexParameterPointervAPPLE,void,(GLenum target, GLenum pname, GLvoid **params))
GL_GROUP_END()
#endif

#ifdef GL_APPLE_transform_hint
GL_GROUP_BEGIN(GL_APPLE_transform_hint)
GL_GROUP_END()
#endif

#ifdef GL_APPLE_vertex_array_object
GL_GROUP_BEGIN(GL_APPLE_vertex_array_object)
GL_FUNCTION(glBindVertexArrayAPPLE,void,(GLuint array))
GL_FUNCTION(glDeleteVertexArraysAPPLE,void,(GLsizei n, const GLuint* arrays))
GL_FUNCTION(glGenVertexArraysAPPLE,void,(GLsizei n, const GLuint* arrays))
GL_FUNCTION(glIsVertexArrayAPPLE,GLboolean,(GLuint array))
GL_GROUP_END()
#endif

#ifdef GL_APPLE_vertex_array_range
GL_GROUP_BEGIN(GL_APPLE_vertex_array_range)
GL_FUNCTION(glFlushVertexArrayRangeAPPLE,void,(GLsizei length, void* pointer))
GL_FUNCTION(glVertexArrayParameteriAPPLE,void,(GLenum pname, GLint param))
GL_FUNCTION(glVertexArrayRangeAPPLE,void,(GLsizei length, void* pointer))
GL_GROUP_END()
#endif

#ifdef GL_APPLE_ycbcr_422
GL_GROUP_BEGIN(GL_APPLE_ycbcr_422)
GL_GROUP_END()
#endif

#ifdef GL_ARB_color_buffer_float
GL_GROUP_BEGIN(GL_ARB_color_buffer_float)
GL_FUNCTION(glClampColorARB,void,(GLenum target, GLenum clamp))
GL_GROUP_END()
#endif

#ifdef GL_ARB_depth_texture
GL_GROUP_BEGIN(GL_ARB_depth_texture)
GL_GROUP_END()
#endif

#ifdef GL_ARB_draw_buffers
GL_GROUP_BEGIN(GL_ARB_draw_buffers)
GL_FUNCTION(glDrawBuffersARB,void,(GLsizei n, const GLenum* bufs))
GL_GROUP_END()
#endif

#ifdef GL_ARB_fragment_program
GL_GROUP_BEGIN(GL_ARB_fragment_program)
GL_GROUP_END()
#endif

#ifdef GL_ARB_fragment_program_shadow
GL_GROUP_BEGIN(GL_ARB_fragment_program_shadow)
GL_GROUP_END()
#endif

#ifdef GL_ARB_fragment_shader
GL_GROUP_BEGIN(GL_ARB_fragment_shader)
GL_GROUP_END()
#endif

#ifdef GL_ARB_half_float_pixel
GL_GROUP_BEGIN(GL_ARB_half_float_pixel)
GL_GROUP_END()
#endif

#if defined(GL_ARB_imaging) && !defined(GL_VERSION_1_4)
GL_GROUP_BEGIN(GL_ARB_imaging)
GL_FUNCTION(glColorTable,void,(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table))
GL_FUNCTION(glColorSubTable,void,(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data))
GL_FUNCTION(glColorTableParameteriv,void,(GLenum target, GLenum pname, const GLint *params))
GL_FUNCTION(glColorTableParameterfv,void,(GLenum target, GLenum pname, const GLfloat *params))
GL_FUNCTION(glCopyColorSubTable,void,(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width))
GL_FUNCTION(glCopyColorTable,void,(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width))
GL_FUNCTION(glGetColorTable,void,(GLenum target, GLenum format, GLenum type, GLvoid *table))
GL_FUNCTION(glGetColorTableParameterfv,void,(GLenum target, GLenum pname, GLfloat *params))
GL_FUNCTION(glGetColorTableParameteriv,void,(GLenum target, GLenum pname, GLint *params))
GL_FUNCTION(glHistogram,void,(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink))
GL_FUNCTION(glResetHistogram,void,(GLenum target))
GL_FUNCTION(glGetHistogram,void,(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values))
GL_FUNCTION(glGetHistogramParameterfv,void,(GLenum target, GLenum pname, GLfloat *params))
GL_FUNCTION(glGetHistogramParameteriv,void,(GLenum target, GLenum pname, GLint *params))
GL_FUNCTION(glMinmax,void,(GLenum target, GLenum internalformat, GLboolean sink))
GL_FUNCTION(glResetMinmax,void,(GLenum target))
GL_FUNCTION(glGetMinmaxParameterfv,void,(GLenum target, GLenum pname, GLfloat *params))
GL_FUNCTION(glGetMinmaxParameteriv,void,(GLenum target, GLenum pname, GLint *params))
GL_FUNCTION(glConvolutionFilter1D,void,(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image))
GL_FUNCTION(glConvolutionFilter2D,void,(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image))
GL_FUNCTION(glConvolutionParameterf,void,(GLenum target, GLenum pname, GLfloat params))
GL_FUNCTION(glConvolutionParameterfv,void,(GLenum target, GLenum pname, const GLfloat *params))
GL_FUNCTION(glConvolutionParameteri,void,(GLenum target, GLenum pname, GLint params))
GL_FUNCTION(glConvolutionParameteriv,void,(GLenum target, GLenum pname, const GLint *params))
GL_FUNCTION(glCopyConvolutionFilter1D,void,(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width))
GL_FUNCTION(glCopyConvolutionFilter2D,void,(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height))
GL_FUNCTION(glGetConvolutionFilter,void,(GLenum target, GLenum format, GLenum type, GLvoid *image))
GL_FUNCTION(glGetConvolutionParameterfv,void,(GLenum target, GLenum pname, GLfloat *params))
GL_FUNCTION(glGetConvolutionParameteriv,void,(GLenum target, GLenum pname, GLint *params))
GL_FUNCTION(glSeparableFilter2D,void,(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column))
GL_FUNCTION(glGetSeparableFilter,void,(GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span))
GL_FUNCTION(glGetMinmax,void,(GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values))
GL_GROUP_END()
#endif

#ifdef GL_ARB_matrix_palette
GL_GROUP_BEGIN(GL_ARB_matrix_palette)
GL_FUNCTION(glCurrentPaletteMatrixARB,void,(GLint index))
GL_FUNCTION(glMatrixIndexPointerARB,void,(GLint size, GLenum type, GLsizei stride, GLvoid *pointer))
GL_FUNCTION(glMatrixIndexubvARB,void,(GLint size, GLubyte *indices))
GL_FUNCTION(glMatrixIndexusvARB,void,(GLint size, GLushort *indices))
GL_FUNCTION(glMatrixIndexuivARB,void,(GLint size, GLuint *indices))
GL_GROUP_END()
#endif

#ifdef GL_ARB_multisample
GL_GROUP_BEGIN(GL_ARB_multisample)
GL_FUNCTION(glSampleCoverageARB,void,(GLclampf value, GLboolean invert))
GL_GROUP_END()
#endif

#ifdef GL_ARB_multitexture
GL_GROUP_BEGIN(GL_ARB_multitexture)
GL_FUNCTION(glActiveTextureARB,void,(GLenum texture))
GL_FUNCTION(glClientActiveTextureARB,void,(GLenum texture))
GL_FUNCTION(glMultiTexCoord1dARB,void,(GLenum target, GLdouble s))
GL_FUNCTION(glMultiTexCoord1dvARB,void,(GLenum target, const GLdouble *v))
GL_FUNCTION(glMultiTexCoord1fARB,void,(GLenum target, GLfloat s))
GL_FUNCTION(glMultiTexCoord1fvARB,void,(GLenum target, const GLfloat *v))
GL_FUNCTION(glMultiTexCoord1iARB,void,(GLenum target, GLint s))
GL_FUNCTION(glMultiTexCoord1ivARB,void,(GLenum target, const GLint *v))
GL_FUNCTION(glMultiTexCoord1sARB,void,(GLenum target, GLshort s))
GL_FUNCTION(glMultiTexCoord1svARB,void,(GLenum target, const GLshort *v))
GL_FUNCTION(glMultiTexCoord2dARB,void,(GLenum target, GLdouble s, GLdouble t))
GL_FUNCTION(glMultiTexCoord2dvARB,void,(GLenum target, const GLdouble *v))
GL_FUNCTION(glMultiTexCoord2fARB,void,(GLenum target, GLfloat s, GLfloat t))
GL_FUNCTION(glMultiTexCoord2fvARB,void,(GLenum target, const GLfloat *v))
GL_FUNCTION(glMultiTexCoord2iARB,void,(GLenum target, GLint s, GLint t))
GL_FUNCTION(glMultiTexCoord2ivARB,void,(GLenum target, const GLint *v))
GL_FUNCTION(glMultiTexCoord2sARB,void,(GLenum target, GLshort s, GLshort t))
GL_FUNCTION(glMultiTexCoord2svARB,void,(GLenum target, const GLshort *v))
GL_FUNCTION(glMultiTexCoord3dARB,void,(GLenum target, GLdouble s, GLdouble t, GLdouble r))
GL_FUNCTION(glMultiTexCoord3dvARB,void,(GLenum target, const GLdouble *v))
GL_FUNCTION(glMultiTexCoord3fARB,void,(GLenum target, GLfloat s, GLfloat t, GLfloat r))
GL_FUNCTION(glMultiTexCoord3fvARB,void,(GLenum target, const GLfloat *v))
GL_FUNCTION(glMultiTexCoord3iARB,void,(GLenum target, GLint s, GLint t, GLint r))
GL_FUNCTION(glMultiTexCoord3ivARB,void,(GLenum target, const GLint *v))
GL_FUNCTION(glMultiTexCoord3sARB,void,(GLenum target, GLshort s, GLshort t, GLshort r))
GL_FUNCTION(glMultiTexCoord3svARB,void,(GLenum target, const GLshort *v))
GL_FUNCTION(glMultiTexCoord4dARB,void,(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q))
GL_FUNCTION(glMultiTexCoord4dvARB,void,(GLenum target, const GLdouble *v))
GL_FUNCTION(glMultiTexCoord4fARB,void,(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q))
GL_FUNCTION(glMultiTexCoord4fvARB,void,(GLenum target, const GLfloat *v))
GL_FUNCTION(glMultiTexCoord4iARB,void,(GLenum target, GLint s, GLint t, GLint r, GLint q))
GL_FUNCTION(glMultiTexCoord4ivARB,void,(GLenum target, const GLint *v))
GL_FUNCTION(glMultiTexCoord4sARB,void,(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q))
GL_FUNCTION(glMultiTexCoord4svARB,void,(GLenum target, const GLshort *v))
GL_GROUP_END()
#endif

#ifdef GL_ARB_occlusion_query
GL_GROUP_BEGIN(GL_ARB_occlusion_query)
GL_FUNCTION(glBeginQueryARB,void,(GLenum target, GLuint id))
GL_FUNCTION(glDeleteQueriesARB,void,(GLsizei n, const GLuint* ids))
GL_FUNCTION(glEndQueryARB,void,(GLenum target))
GL_FUNCTION(glGenQueriesARB,void,(GLsizei n, GLuint* ids))
GL_FUNCTION(glGetQueryObjectivARB,void,(GLuint id, GLenum pname, GLint* params))
GL_FUNCTION(glGetQueryObjectuivARB,void,(GLuint id, GLenum pname, GLuint* params))
GL_FUNCTION(glGetQueryivARB,void,(GLenum target, GLenum pname, GLint* params))
GL_FUNCTION(glIsQueryARB,GLboolean,(GLuint id))
GL_GROUP_END()
#endif

#ifdef GL_ARB_pixel_buffer_object
GL_GROUP_BEGIN(GL_ARB_pixel_buffer_object)
GL_GROUP_END()
#endif

#ifdef GL_ARB_point_parameters
GL_GROUP_BEGIN(GL_ARB_point_parameters)
GL_FUNCTION(glPointParameterfARB,void,(GLenum pname, GLfloat param))
GL_FUNCTION(glPointParameterfvARB,void,(GLenum pname, GLfloat* params))
GL_GROUP_END()
#endif

#ifdef GL_ARB_point_sprite
GL_GROUP_BEGIN(GL_ARB_point_sprite)
GL_GROUP_END()
#endif

#ifdef GL_ARB_shader_objects
GL_GROUP_BEGIN(GL_ARB_shader_objects)
GL_FUNCTION(glAttachObjectARB,void,(GLhandleARB containerObj, GLhandleARB obj))
GL_FUNCTION(glCompileShaderARB,void,(GLhandleARB shaderObj))
GL_FUNCTION(glCreateProgramObjectARB,GLhandleARB,(void))
GL_FUNCTION(glCreateShaderObjectARB,GLhandleARB,(GLenum shaderType))
GL_FUNCTION(glDeleteObjectARB,void,(GLhandleARB obj))
GL_FUNCTION(glDetachObjectARB,void,(GLhandleARB containerObj, GLhandleARB attachedObj))
GL_FUNCTION(glGetActiveUniformARB,void,(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint *size, GLenum *type, GLcharARB *name))
GL_FUNCTION(glGetAttachedObjectsARB,void,(GLhandleARB containerObj, GLsizei maxCount, GLsizei* count, GLhandleARB *obj))
GL_FUNCTION(glGetHandleARB,GLhandleARB,(GLenum pname))
GL_FUNCTION(glGetInfoLogARB,void,(GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB *infoLog))
GL_FUNCTION(glGetObjectParameterfvARB,void,(GLhandleARB obj, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetObjectParameterivARB,void,(GLhandleARB obj, GLenum pname, GLint* params))
GL_FUNCTION(glGetShaderSourceARB,void,(GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB *source))
GL_FUNCTION(glGetUniformLocationARB,GLint,(GLhandleARB programObj, const GLcharARB* name))
GL_FUNCTION(glGetUniformfvARB,void,(GLhandleARB programObj, GLint location, GLfloat* params))
GL_FUNCTION(glGetUniformivARB,void,(GLhandleARB programObj, GLint location, GLint* params))
GL_FUNCTION(glLinkProgramARB,void,(GLhandleARB programObj))
GL_FUNCTION(glShaderSourceARB,void,(GLhandleARB shaderObj, GLsizei count, const GLcharARB ** string, const GLint *length))
GL_FUNCTION(glUniform1fARB,void,(GLint location, GLfloat v0))
GL_FUNCTION(glUniform1fvARB,void,(GLint location, GLsizei count, const GLfloat* value))
GL_FUNCTION(glUniform1iARB,void,(GLint location, GLint v0))
GL_FUNCTION(glUniform1ivARB,void,(GLint location, GLsizei count, const GLint* value))
GL_FUNCTION(glUniform2fARB,void,(GLint location, GLfloat v0, GLfloat v1))
GL_FUNCTION(glUniform2fvARB,void,(GLint location, GLsizei count, const GLfloat* value))
GL_FUNCTION(glUniform2iARB,void,(GLint location, GLint v0, GLint v1))
GL_FUNCTION(glUniform2ivARB,void,(GLint location, GLsizei count, const GLint* value))
GL_FUNCTION(glUniform3fARB,void,(GLint location, GLfloat v0, GLfloat v1, GLfloat v2))
GL_FUNCTION(glUniform3fvARB,void,(GLint location, GLsizei count, const GLfloat* value))
GL_FUNCTION(glUniform3iARB,void,(GLint location, GLint v0, GLint v1, GLint v2))
GL_FUNCTION(glUniform3ivARB,void,(GLint location, GLsizei count, const GLint* value))
GL_FUNCTION(glUniform4fARB,void,(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3))
GL_FUNCTION(glUniform4fvARB,void,(GLint location, GLsizei count, const GLfloat* value))
GL_FUNCTION(glUniform4iARB,void,(GLint location, GLint v0, GLint v1, GLint v2, GLint v3))
GL_FUNCTION(glUniform4ivARB,void,(GLint location, GLsizei count, const GLint* value))
GL_FUNCTION(glUniformMatrix2fvARB,void,(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value))
GL_FUNCTION(glUniformMatrix3fvARB,void,(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value))
GL_FUNCTION(glUniformMatrix4fvARB,void,(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value))
GL_FUNCTION(glUseProgramObjectARB,void,(GLhandleARB programObj))
GL_FUNCTION(glValidateProgramARB,void,(GLhandleARB programObj))
GL_GROUP_END()
#endif

#ifdef GL_ARB_shading_language_100
GL_GROUP_BEGIN(GL_ARB_shading_language_100)
GL_GROUP_END()
#endif

#ifdef GL_ARB_shadow
GL_GROUP_BEGIN(GL_ARB_shadow)
GL_GROUP_END()
#endif

#ifdef GL_ARB_shadow_ambient
GL_GROUP_BEGIN(GL_ARB_shadow_ambient)
GL_GROUP_END()
#endif

#ifdef GL_ARB_texture_border_clamp
GL_GROUP_BEGIN(GL_ARB_texture_border_clamp)
GL_GROUP_END()
#endif

#ifdef GL_ARB_texture_compression
GL_GROUP_BEGIN(GL_ARB_texture_compression)
GL_FUNCTION(glCompressedTexImage1DARB,void,(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data))
GL_FUNCTION(glCompressedTexImage2DARB,void,(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data))
GL_FUNCTION(glCompressedTexImage3DARB,void,(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data))
GL_FUNCTION(glCompressedTexSubImage1DARB,void,(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data))
GL_FUNCTION(glCompressedTexSubImage2DARB,void,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data))
GL_FUNCTION(glCompressedTexSubImage3DARB,void,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data))
GL_FUNCTION(glGetCompressedTexImageARB,void,(GLenum target, GLint lod, void* img))
GL_GROUP_END()
#endif

#ifdef GL_ARB_texture_cube_map
GL_GROUP_BEGIN(GL_ARB_texture_cube_map)
GL_GROUP_END()
#endif

#ifdef GL_ARB_texture_env_add
GL_GROUP_BEGIN(GL_ARB_texture_env_add)
GL_GROUP_END()
#endif

#ifdef GL_ARB_texture_env_combine
GL_GROUP_BEGIN(GL_ARB_texture_env_combine)
GL_GROUP_END()
#endif

#ifdef GL_ARB_texture_env_crossbar
GL_GROUP_BEGIN(GL_ARB_texture_env_crossbar)
GL_GROUP_END()
#endif

#ifdef GL_ARB_texture_env_dot3
GL_GROUP_BEGIN(GL_ARB_texture_env_dot3)
GL_GROUP_END()
#endif

#ifdef GL_ARB_texture_float
GL_GROUP_BEGIN(GL_ARB_texture_float)
GL_GROUP_END()
#endif

#ifdef GL_ARB_texture_mirrored_repeat
GL_GROUP_BEGIN(GL_ARB_texture_mirrored_repeat)
GL_GROUP_END()
#endif

#ifdef GL_ARB_texture_non_power_of_two
GL_GROUP_BEGIN(GL_ARB_texture_non_power_of_two)
GL_GROUP_END()
#endif

#ifdef GL_ARB_texture_rectangle
GL_GROUP_BEGIN(GL_ARB_texture_rectangle)
GL_GROUP_END()
#endif

#ifdef GL_ARB_transpose_matrix
GL_GROUP_BEGIN(GL_ARB_transpose_matrix)
GL_FUNCTION(glLoadTransposeMatrixfARB,void,(GLfloat m[16]))
GL_FUNCTION(glLoadTransposeMatrixdARB,void,(GLdouble m[16]))
GL_FUNCTION(glMultTransposeMatrixfARB,void,(GLfloat m[16]))
GL_FUNCTION(glMultTransposeMatrixdARB,void,(GLdouble m[16]))
GL_GROUP_END()
#endif

#ifdef GL_ARB_vertex_blend
GL_GROUP_BEGIN(GL_ARB_vertex_blend)
GL_FUNCTION(glWeightbvARB,void,(GLint size, GLbyte *weights))
GL_FUNCTION(glWeightsvARB,void,(GLint size, GLshort *weights))
GL_FUNCTION(glWeightivARB,void,(GLint size, GLint *weights))
GL_FUNCTION(glWeightfvARB,void,(GLint size, GLfloat *weights))
GL_FUNCTION(glWeightdvARB,void,(GLint size, GLdouble *weights))
GL_FUNCTION(glWeightubvARB,void,(GLint size, GLubyte *weights))
GL_FUNCTION(glWeightusvARB,void,(GLint size, GLushort *weights))
GL_FUNCTION(glWeightuivARB,void,(GLint size, GLuint *weights))
GL_FUNCTION(glWeightPointerARB,void,(GLint size, GLenum type, GLsizei stride, GLvoid *pointer))
GL_FUNCTION(glVertexBlendARB,void,(GLint count))
GL_GROUP_END()
#endif

#ifdef GL_ARB_vertex_buffer_object
GL_GROUP_BEGIN(GL_ARB_vertex_buffer_object)
GL_FUNCTION(glBindBufferARB,void,(GLenum target, GLuint buffer))
GL_FUNCTION(glBufferDataARB,void,(GLenum target, GLsizeiptrARB size, const GLvoid* data, GLenum usage))
GL_FUNCTION(glBufferSubDataARB,void,(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid* data))
GL_FUNCTION(glDeleteBuffersARB,void,(GLsizei n, const GLuint* buffers))
GL_FUNCTION(glGenBuffersARB,void,(GLsizei n, GLuint* buffers))
GL_FUNCTION(glGetBufferParameterivARB,void,(GLenum target, GLenum pname, GLint* params))
GL_FUNCTION(glGetBufferPointervARB,void,(GLenum target, GLenum pname, GLvoid** params))
GL_FUNCTION(glGetBufferSubDataARB,void,(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid* data))
GL_FUNCTION(glIsBufferARB,GLboolean,(GLuint buffer))
GL_FUNCTION(glMapBufferARB,GLvoid *,(GLenum target, GLenum access))
GL_FUNCTION(glUnmapBufferARB,GLboolean,(GLenum target))
GL_GROUP_END()
#endif

#ifdef GL_ARB_vertex_program
GL_GROUP_BEGIN(GL_ARB_vertex_program)
GL_FUNCTION(glBindProgramARB,void,(GLenum target, GLuint program))
GL_FUNCTION(glDeleteProgramsARB,void,(GLsizei n, const GLuint* programs))
GL_FUNCTION(glDisableVertexAttribArrayARB,void,(GLuint index))
GL_FUNCTION(glEnableVertexAttribArrayARB,void,(GLuint index))
GL_FUNCTION(glGenProgramsARB,void,(GLsizei n, GLuint* programs))
GL_FUNCTION(glGetProgramEnvParameterdvARB,void,(GLenum target, GLuint index, GLdouble* params))
GL_FUNCTION(glGetProgramEnvParameterfvARB,void,(GLenum target, GLuint index, GLfloat* params))
GL_FUNCTION(glGetProgramLocalParameterdvARB,void,(GLenum target, GLuint index, GLdouble* params))
GL_FUNCTION(glGetProgramLocalParameterfvARB,void,(GLenum target, GLuint index, GLfloat* params))
GL_FUNCTION(glGetProgramStringARB,void,(GLenum target, GLenum pname, void* string))
GL_FUNCTION(glGetProgramivARB,void,(GLenum target, GLenum pname, GLint* params))
GL_FUNCTION(glGetVertexAttribPointervARB,void,(GLuint index, GLenum pname, GLvoid** pointer))
GL_FUNCTION(glGetVertexAttribdvARB,void,(GLuint index, GLenum pname, GLdouble* params))
GL_FUNCTION(glGetVertexAttribfvARB,void,(GLuint index, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetVertexAttribivARB,void,(GLuint index, GLenum pname, GLint* params))
GL_FUNCTION(glIsProgramARB,GLboolean,(GLuint program))
GL_FUNCTION(glProgramEnvParameter4dARB,void,(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_FUNCTION(glProgramEnvParameter4dvARB,void,(GLenum target, GLuint index, const GLdouble* params))
GL_FUNCTION(glProgramEnvParameter4fARB,void,(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_FUNCTION(glProgramEnvParameter4fvARB,void,(GLenum target, GLuint index, const GLfloat* params))
GL_FUNCTION(glProgramLocalParameter4dARB,void,(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_FUNCTION(glProgramLocalParameter4dvARB,void,(GLenum target, GLuint index, const GLdouble* params))
GL_FUNCTION(glProgramLocalParameter4fARB,void,(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_FUNCTION(glProgramLocalParameter4fvARB,void,(GLenum target, GLuint index, const GLfloat* params))
GL_FUNCTION(glProgramStringARB,void,(GLenum target, GLenum format, GLsizei len, const void* string))
GL_FUNCTION(glVertexAttrib1dARB,void,(GLuint index, GLdouble x))
GL_FUNCTION(glVertexAttrib1dvARB,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib1fARB,void,(GLuint index, GLfloat x))
GL_FUNCTION(glVertexAttrib1fvARB,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib1sARB,void,(GLuint index, GLshort x))
GL_FUNCTION(glVertexAttrib1svARB,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib2dARB,void,(GLuint index, GLdouble x, GLdouble y))
GL_FUNCTION(glVertexAttrib2dvARB,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib2fARB,void,(GLuint index, GLfloat x, GLfloat y))
GL_FUNCTION(glVertexAttrib2fvARB,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib2sARB,void,(GLuint index, GLshort x, GLshort y))
GL_FUNCTION(glVertexAttrib2svARB,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib3dARB,void,(GLuint index, GLdouble x, GLdouble y, GLdouble z))
GL_FUNCTION(glVertexAttrib3dvARB,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib3fARB,void,(GLuint index, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glVertexAttrib3fvARB,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib3sARB,void,(GLuint index, GLshort x, GLshort y, GLshort z))
GL_FUNCTION(glVertexAttrib3svARB,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib4NbvARB,void,(GLuint index, const GLbyte* v))
GL_FUNCTION(glVertexAttrib4NivARB,void,(GLuint index, const GLint* v))
GL_FUNCTION(glVertexAttrib4NsvARB,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib4NubARB,void,(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w))
GL_FUNCTION(glVertexAttrib4NubvARB,void,(GLuint index, const GLubyte* v))
GL_FUNCTION(glVertexAttrib4NuivARB,void,(GLuint index, const GLuint* v))
GL_FUNCTION(glVertexAttrib4NusvARB,void,(GLuint index, const GLushort* v))
GL_FUNCTION(glVertexAttrib4bvARB,void,(GLuint index, const GLbyte* v))
GL_FUNCTION(glVertexAttrib4dARB,void,(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_FUNCTION(glVertexAttrib4dvARB,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib4fARB,void,(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_FUNCTION(glVertexAttrib4fvARB,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib4ivARB,void,(GLuint index, const GLint* v))
GL_FUNCTION(glVertexAttrib4sARB,void,(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w))
GL_FUNCTION(glVertexAttrib4svARB,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib4ubvARB,void,(GLuint index, const GLubyte* v))
GL_FUNCTION(glVertexAttrib4uivARB,void,(GLuint index, const GLuint* v))
GL_FUNCTION(glVertexAttrib4usvARB,void,(GLuint index, const GLushort* v))
GL_FUNCTION(glVertexAttribPointerARB,void,(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer))
GL_GROUP_END()
#endif

#ifdef GL_ARB_vertex_shader
GL_GROUP_BEGIN(GL_ARB_vertex_shader)
GL_FUNCTION(glBindAttribLocationARB,void,(GLhandleARB programObj, GLuint index, const GLcharARB* name))
GL_FUNCTION(glGetActiveAttribARB,void,(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint *size, GLenum *type, GLcharARB *name))
GL_FUNCTION(glGetAttribLocationARB,GLint,(GLhandleARB programObj, const GLcharARB* name))
GL_GROUP_END()
#endif

#ifdef GL_ARB_window_pos
GL_GROUP_BEGIN(GL_ARB_window_pos)
GL_FUNCTION(glWindowPos2dARB,void,(GLdouble x, GLdouble y))
GL_FUNCTION(glWindowPos2dvARB,void,(const GLdouble* p))
GL_FUNCTION(glWindowPos2fARB,void,(GLfloat x, GLfloat y))
GL_FUNCTION(glWindowPos2fvARB,void,(const GLfloat* p))
GL_FUNCTION(glWindowPos2iARB,void,(GLint x, GLint y))
GL_FUNCTION(glWindowPos2ivARB,void,(const GLint* p))
GL_FUNCTION(glWindowPos2sARB,void,(GLshort x, GLshort y))
GL_FUNCTION(glWindowPos2svARB,void,(const GLshort* p))
GL_FUNCTION(glWindowPos3dARB,void,(GLdouble x, GLdouble y, GLdouble z))
GL_FUNCTION(glWindowPos3dvARB,void,(const GLdouble* p))
GL_FUNCTION(glWindowPos3fARB,void,(GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glWindowPos3fvARB,void,(const GLfloat* p))
GL_FUNCTION(glWindowPos3iARB,void,(GLint x, GLint y, GLint z))
GL_FUNCTION(glWindowPos3ivARB,void,(const GLint* p))
GL_FUNCTION(glWindowPos3sARB,void,(GLshort x, GLshort y, GLshort z))
GL_FUNCTION(glWindowPos3svARB,void,(const GLshort* p))
GL_GROUP_END()
#endif

#ifdef GL_ATIX_point_sprites
GL_GROUP_BEGIN(GL_ATIX_point_sprites)
GL_GROUP_END()
#endif

#ifdef GL_ATIX_texture_env_combine3
GL_GROUP_BEGIN(GL_ATIX_texture_env_combine3)
GL_GROUP_END()
#endif

#ifdef GL_ATIX_texture_env_route
GL_GROUP_BEGIN(GL_ATIX_texture_env_route)
GL_GROUP_END()
#endif

#ifdef GL_ATIX_vertex_shader_output_point_size
GL_GROUP_BEGIN(GL_ATIX_vertex_shader_output_point_size)
GL_GROUP_END()
#endif

#ifdef GL_ATI_draw_buffers
GL_GROUP_BEGIN(GL_ATI_draw_buffers)
GL_FUNCTION(glDrawBuffersATI,void,(GLsizei n, const GLenum* bufs))
GL_GROUP_END()
#endif

#ifdef GL_ATI_element_array
GL_GROUP_BEGIN(GL_ATI_element_array)
GL_FUNCTION(glDrawElementArrayATI,void,(GLenum mode, GLsizei count))
GL_FUNCTION(glDrawRangeElementArrayATI,void,(GLenum mode, GLuint start, GLuint end, GLsizei count))
GL_FUNCTION(glElementPointerATI,void,(GLenum type, const void* pointer))
GL_GROUP_END()
#endif

#ifdef GL_ATI_envmap_bumpmap
GL_GROUP_BEGIN(GL_ATI_envmap_bumpmap)
GL_FUNCTION(glTexBumpParameterivATI,void,(GLenum pname, GLint *param))
GL_FUNCTION(glTexBumpParameterfvATI,void,(GLenum pname, GLfloat *param))
GL_FUNCTION(glGetTexBumpParameterivATI,void,(GLenum pname, GLint *param))
GL_FUNCTION(glGetTexBumpParameterfvATI,void,(GLenum pname, GLfloat *param))
GL_GROUP_END()
#endif

#ifdef GL_ATI_fragment_shader
GL_GROUP_BEGIN(GL_ATI_fragment_shader)
GL_FUNCTION(glAlphaFragmentOp1ATI,void,(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod))
GL_FUNCTION(glAlphaFragmentOp2ATI,void,(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod))
GL_FUNCTION(glAlphaFragmentOp3ATI,void,(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod))
GL_FUNCTION(glBeginFragmentShaderATI,void,(void))
GL_FUNCTION(glBindFragmentShaderATI,void,(GLuint id))
GL_FUNCTION(glColorFragmentOp1ATI,void,(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod))
GL_FUNCTION(glColorFragmentOp2ATI,void,(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod))
GL_FUNCTION(glColorFragmentOp3ATI,void,(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod))
GL_FUNCTION(glDeleteFragmentShaderATI,void,(GLuint id))
GL_FUNCTION(glEndFragmentShaderATI,void,(void))
GL_FUNCTION(glGenFragmentShadersATI,GLuint,(GLuint range))
GL_FUNCTION(glPassTexCoordATI,void,(GLuint dst, GLuint coord, GLenum swizzle))
GL_FUNCTION(glSampleMapATI,void,(GLuint dst, GLuint interp, GLenum swizzle))
GL_FUNCTION(glSetFragmentShaderConstantATI,void,(GLuint dst, const GLfloat* value))
GL_GROUP_END()
#endif

#ifdef GL_ATI_map_object_buffer
GL_GROUP_BEGIN(GL_ATI_map_object_buffer)
GL_FUNCTION(glMapObjectBufferATI,void*,(GLuint buffer))
GL_FUNCTION(glUnmapObjectBufferATI,void,(GLuint buffer))
GL_GROUP_END()
#endif

#ifdef GL_ATI_pn_triangles
GL_GROUP_BEGIN(GL_ATI_pn_triangles)
GL_FUNCTION(glPNTrianglesiATI,void,(GLenum pname, GLint param))
GL_FUNCTION(glPNTrianglesfATI,void,(GLenum pname, GLfloat param))
GL_GROUP_END()
#endif

#ifdef GL_ATI_separate_stencil
GL_GROUP_BEGIN(GL_ATI_separate_stencil)
GL_FUNCTION(glStencilOpSeparateATI,void,(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass))
GL_FUNCTION(glStencilFuncSeparateATI,void,(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask))
GL_GROUP_END()
#endif

#ifdef GL_ATI_text_fragment_shader
GL_GROUP_BEGIN(GL_ATI_text_fragment_shader)
GL_GROUP_END()
#endif

#ifdef GL_ATI_texture_compression_3dc
GL_GROUP_BEGIN(GL_ATI_texture_compression_3dc)
GL_GROUP_END()
#endif

#ifdef GL_ATI_texture_env_combine3
GL_GROUP_BEGIN(GL_ATI_texture_env_combine3)
GL_GROUP_END()
#endif

#ifdef GL_ATI_texture_float
GL_GROUP_BEGIN(GL_ATI_texture_float)
GL_GROUP_END()
#endif

#ifdef GL_ATI_texture_mirror_once
GL_GROUP_BEGIN(GL_ATI_texture_mirror_once)
GL_GROUP_END()
#endif

#ifdef GL_ATI_vertex_array_object
GL_GROUP_BEGIN(GL_ATI_vertex_array_object)
GL_FUNCTION(glArrayObjectATI,void,(GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset))
GL_FUNCTION(glFreeObjectBufferATI,void,(GLuint buffer))
GL_FUNCTION(glGetArrayObjectfvATI,void,(GLenum array, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetArrayObjectivATI,void,(GLenum array, GLenum pname, GLint* params))
GL_FUNCTION(glGetObjectBufferfvATI,void,(GLuint buffer, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetObjectBufferivATI,void,(GLuint buffer, GLenum pname, GLint* params))
GL_FUNCTION(glGetVariantArrayObjectfvATI,void,(GLuint id, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetVariantArrayObjectivATI,void,(GLuint id, GLenum pname, GLint* params))
GL_FUNCTION(glIsObjectBufferATI,GLboolean,(GLuint buffer))
GL_FUNCTION(glNewObjectBufferATI,GLuint,(GLsizei size, const void* pointer, GLenum usage))
GL_FUNCTION(glUpdateObjectBufferATI,void,(GLuint buffer, GLuint offset, GLsizei size, const void* pointer, GLenum preserve))
GL_FUNCTION(glVariantArrayObjectATI,void,(GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset))
GL_GROUP_END()
#endif

#ifdef GL_ATI_vertex_attrib_array_object
GL_GROUP_BEGIN(GL_ATI_vertex_attrib_array_object)
GL_FUNCTION(glGetVertexAttribArrayObjectfvATI,void,(GLuint index, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetVertexAttribArrayObjectivATI,void,(GLuint index, GLenum pname, GLint* params))
GL_FUNCTION(glVertexAttribArrayObjectATI,void,(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset))
GL_GROUP_END()
#endif

#ifdef GL_ATI_vertex_streams
GL_GROUP_BEGIN(GL_ATI_vertex_streams)
GL_FUNCTION(glClientActiveVertexStreamATI,void,(GLenum stream))
GL_FUNCTION(glVertexBlendEnviATI,void,(GLenum pname, GLint param))
GL_FUNCTION(glVertexBlendEnvfATI,void,(GLenum pname, GLfloat param))
GL_FUNCTION(glVertexStream2sATI,void,(GLenum stream, GLshort x, GLshort y))
GL_FUNCTION(glVertexStream2svATI,void,(GLenum stream, const GLshort *v))
GL_FUNCTION(glVertexStream2iATI,void,(GLenum stream, GLint x, GLint y))
GL_FUNCTION(glVertexStream2ivATI,void,(GLenum stream, const GLint *v))
GL_FUNCTION(glVertexStream2fATI,void,(GLenum stream, GLfloat x, GLfloat y))
GL_FUNCTION(glVertexStream2fvATI,void,(GLenum stream, const GLfloat *v))
GL_FUNCTION(glVertexStream2dATI,void,(GLenum stream, GLdouble x, GLdouble y))
GL_FUNCTION(glVertexStream2dvATI,void,(GLenum stream, const GLdouble *v))
GL_FUNCTION(glVertexStream3sATI,void,(GLenum stream, GLshort x, GLshort y, GLshort z))
GL_FUNCTION(glVertexStream3svATI,void,(GLenum stream, const GLshort *v))
GL_FUNCTION(glVertexStream3iATI,void,(GLenum stream, GLint x, GLint y, GLint z))
GL_FUNCTION(glVertexStream3ivATI,void,(GLenum stream, const GLint *v))
GL_FUNCTION(glVertexStream3fATI,void,(GLenum stream, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glVertexStream3fvATI,void,(GLenum stream, const GLfloat *v))
GL_FUNCTION(glVertexStream3dATI,void,(GLenum stream, GLdouble x, GLdouble y, GLdouble z))
GL_FUNCTION(glVertexStream3dvATI,void,(GLenum stream, const GLdouble *v))
GL_FUNCTION(glVertexStream4sATI,void,(GLenum stream, GLshort x, GLshort y, GLshort z, GLshort w))
GL_FUNCTION(glVertexStream4svATI,void,(GLenum stream, const GLshort *v))
GL_FUNCTION(glVertexStream4iATI,void,(GLenum stream, GLint x, GLint y, GLint z, GLint w))
GL_FUNCTION(glVertexStream4ivATI,void,(GLenum stream, const GLint *v))
GL_FUNCTION(glVertexStream4fATI,void,(GLenum stream, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_FUNCTION(glVertexStream4fvATI,void,(GLenum stream, const GLfloat *v))
GL_FUNCTION(glVertexStream4dATI,void,(GLenum stream, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_FUNCTION(glVertexStream4dvATI,void,(GLenum stream, const GLdouble *v))
GL_FUNCTION(glNormalStream3bATI,void,(GLenum stream, GLbyte x, GLbyte y, GLbyte z))
GL_FUNCTION(glNormalStream3bvATI,void,(GLenum stream, const GLbyte *v))
GL_FUNCTION(glNormalStream3sATI,void,(GLenum stream, GLshort x, GLshort y, GLshort z))
GL_FUNCTION(glNormalStream3svATI,void,(GLenum stream, const GLshort *v))
GL_FUNCTION(glNormalStream3iATI,void,(GLenum stream, GLint x, GLint y, GLint z))
GL_FUNCTION(glNormalStream3ivATI,void,(GLenum stream, const GLint *v))
GL_FUNCTION(glNormalStream3fATI,void,(GLenum stream, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glNormalStream3fvATI,void,(GLenum stream, const GLfloat *v))
GL_FUNCTION(glNormalStream3dATI,void,(GLenum stream, GLdouble x, GLdouble y, GLdouble z))
GL_FUNCTION(glNormalStream3dvATI,void,(GLenum stream, const GLdouble *v))
GL_GROUP_END()
#endif

#ifdef GL_EXT_422_pixels
GL_GROUP_BEGIN(GL_EXT_422_pixels)
GL_GROUP_END()
#endif

#ifdef GL_EXT_Cg_shader
GL_GROUP_BEGIN(GL_EXT_Cg_shader)
GL_GROUP_END()
#endif

#ifdef GL_EXT_abgr
GL_GROUP_BEGIN(GL_EXT_abgr)
GL_GROUP_END()
#endif

#ifdef GL_EXT_bgra
GL_GROUP_BEGIN(GL_EXT_bgra)
GL_GROUP_END()
#endif

#ifdef GL_EXT_blend_color
GL_GROUP_BEGIN(GL_EXT_blend_color)
GL_FUNCTION(glBlendColorEXT,void,(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha))
GL_GROUP_END()
#endif

#ifdef GL_EXT_blend_equation_separate
GL_GROUP_BEGIN(GL_EXT_blend_equation_separate)
GL_FUNCTION(glBlendEquationSeparateEXT,void,(GLenum modeRGB, GLenum modeAlpha))
GL_GROUP_END()
#endif

#ifdef GL_EXT_blend_func_separate
GL_GROUP_BEGIN(GL_EXT_blend_func_separate)
GL_FUNCTION(glBlendFuncSeparateEXT,void,(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha))
GL_GROUP_END()
#endif

#ifdef GL_EXT_blend_logic_op
GL_GROUP_BEGIN(GL_EXT_blend_logic_op)
GL_GROUP_END()
#endif

#ifdef GL_EXT_blend_minmax
GL_GROUP_BEGIN(GL_EXT_blend_minmax)
GL_FUNCTION(glBlendEquationEXT,void,(GLenum mode))
GL_GROUP_END()
#endif

#ifdef GL_EXT_blend_subtract
GL_GROUP_BEGIN(GL_EXT_blend_subtract)
GL_GROUP_END()
#endif

#ifdef GL_EXT_clip_volume_hint
GL_GROUP_BEGIN(GL_EXT_clip_volume_hint)
GL_GROUP_END()
#endif

#ifdef GL_EXT_cmyka
GL_GROUP_BEGIN(GL_EXT_cmyka)
GL_GROUP_END()
#endif

#ifdef GL_EXT_color_subtable
GL_GROUP_BEGIN(GL_EXT_color_subtable)
GL_FUNCTION(glColorSubTableEXT,void,(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void* data))
GL_FUNCTION(glCopyColorSubTableEXT,void,(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width))
GL_GROUP_END()
#endif

#ifdef GL_EXT_compiled_vertex_array
GL_GROUP_BEGIN(GL_EXT_compiled_vertex_array)
GL_FUNCTION(glLockArraysEXT,void,(GLint first, GLsizei count))
GL_FUNCTION(glUnlockArraysEXT,void,(void))
GL_GROUP_END()
#endif

#ifdef GL_EXT_convolution
GL_GROUP_BEGIN(GL_EXT_convolution)
GL_FUNCTION(glConvolutionFilter1DEXT,void,(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void* image))
GL_FUNCTION(glConvolutionFilter2DEXT,void,(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* image))
GL_FUNCTION(glConvolutionParameterfEXT,void,(GLenum target, GLenum pname, GLfloat param))
GL_FUNCTION(glConvolutionParameterfvEXT,void,(GLenum target, GLenum pname, const GLfloat* params))
GL_FUNCTION(glConvolutionParameteriEXT,void,(GLenum target, GLenum pname, GLint param))
GL_FUNCTION(glConvolutionParameterivEXT,void,(GLenum target, GLenum pname, const GLint* params))
GL_FUNCTION(glCopyConvolutionFilter1DEXT,void,(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width))
GL_FUNCTION(glCopyConvolutionFilter2DEXT,void,(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height))
GL_FUNCTION(glGetConvolutionFilterEXT,void,(GLenum target, GLenum format, GLenum type, void* image))
GL_FUNCTION(glGetConvolutionParameterfvEXT,void,(GLenum target, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetConvolutionParameterivEXT,void,(GLenum target, GLenum pname, GLint* params))
GL_FUNCTION(glGetSeparableFilterEXT,void,(GLenum target, GLenum format, GLenum type, void* row, void* column, void* span))
GL_FUNCTION(glSeparableFilter2DEXT,void,(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* row, const void* column))
GL_GROUP_END()
#endif

#ifdef GL_EXT_coordinate_frame
GL_GROUP_BEGIN(GL_EXT_coordinate_frame)
GL_FUNCTION(glBinormalPointerEXT,void,(GLenum type, GLsizei stride, void* pointer))
GL_FUNCTION(glTangentPointerEXT,void,(GLenum type, GLsizei stride, void* pointer))
GL_GROUP_END()
#endif

#ifdef GL_EXT_copy_texture
GL_GROUP_BEGIN(GL_EXT_copy_texture)
GL_FUNCTION(glCopyTexImage1DEXT,void,(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border))
GL_FUNCTION(glCopyTexImage2DEXT,void,(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border))
GL_FUNCTION(glCopyTexSubImage1DEXT,void,(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width))
GL_FUNCTION(glCopyTexSubImage2DEXT,void,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height))
GL_FUNCTION(glCopyTexSubImage3DEXT,void,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height))
GL_GROUP_END()
#endif

#ifdef GL_EXT_cull_vertex
GL_GROUP_BEGIN(GL_EXT_cull_vertex)
GL_FUNCTION(glCullParameterdvEXT,void,(GLenum pname, GLdouble* params))
GL_FUNCTION(glCullParameterfvEXT,void,(GLenum pname, GLfloat* params))
GL_GROUP_END()
#endif

#ifdef GL_EXT_depth_bounds_test
GL_GROUP_BEGIN(GL_EXT_depth_bounds_test)
GL_FUNCTION(glDepthBoundsEXT,void,(GLclampd zmin, GLclampd zmax))
GL_GROUP_END()
#endif

#ifdef GL_EXT_draw_range_elements
GL_GROUP_BEGIN(GL_EXT_draw_range_elements)
GL_FUNCTION(glDrawRangeElementsEXT,void,(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices))
GL_GROUP_END()
#endif

#ifdef GL_EXT_fog_coord
GL_GROUP_BEGIN(GL_EXT_fog_coord)
GL_FUNCTION(glFogCoordfEXT,void,(GLfloat coord))
GL_FUNCTION(glFogCoordfvEXT,void,(const GLfloat *coord))
GL_FUNCTION(glFogCoorddEXT,void,(GLdouble coord))
GL_FUNCTION(glFogCoorddvEXT,void,(const GLdouble *coord))
GL_FUNCTION(glFogCoordPointerEXT,void,(GLenum type, GLsizei stride, const GLvoid *pointer))
GL_GROUP_END()
#endif

#ifdef GL_EXT_fragment_lighting
GL_GROUP_BEGIN(GL_EXT_fragment_lighting)
GL_FUNCTION(glFragmentColorMaterialEXT,void,(GLenum face, GLenum mode))
GL_FUNCTION(glFragmentLightModelfEXT,void,(GLenum pname, GLfloat param))
GL_FUNCTION(glFragmentLightModelfvEXT,void,(GLenum pname, GLfloat* params))
GL_FUNCTION(glFragmentLightModeliEXT,void,(GLenum pname, GLint param))
GL_FUNCTION(glFragmentLightModelivEXT,void,(GLenum pname, GLint* params))
GL_FUNCTION(glFragmentLightfEXT,void,(GLenum light, GLenum pname, GLfloat param))
GL_FUNCTION(glFragmentLightfvEXT,void,(GLenum light, GLenum pname, GLfloat* params))
GL_FUNCTION(glFragmentLightiEXT,void,(GLenum light, GLenum pname, GLint param))
GL_FUNCTION(glFragmentLightivEXT,void,(GLenum light, GLenum pname, GLint* params))
GL_FUNCTION(glFragmentMaterialfEXT,void,(GLenum face, GLenum pname, const GLfloat param))
GL_FUNCTION(glFragmentMaterialfvEXT,void,(GLenum face, GLenum pname, const GLfloat* params))
GL_FUNCTION(glFragmentMaterialiEXT,void,(GLenum face, GLenum pname, const GLint param))
GL_FUNCTION(glFragmentMaterialivEXT,void,(GLenum face, GLenum pname, const GLint* params))
GL_FUNCTION(glGetFragmentLightfvEXT,void,(GLenum light, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetFragmentLightivEXT,void,(GLenum light, GLenum pname, GLint* params))
GL_FUNCTION(glGetFragmentMaterialfvEXT,void,(GLenum face, GLenum pname, const GLfloat* params))
GL_FUNCTION(glGetFragmentMaterialivEXT,void,(GLenum face, GLenum pname, const GLint* params))
GL_FUNCTION(glLightEnviEXT,void,(GLenum pname, GLint param))
GL_GROUP_END()
#endif

#ifdef GL_EXT_framebuffer_blit
GL_GROUP_BEGIN(GL_EXT_framebuffer_blit)
GL_FUNCTION(glBlitFramebufferEXT,void,(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum fliter))
GL_GROUP_END()
#endif

#ifdef GL_EXT_framebuffer_object
GL_GROUP_BEGIN(GL_EXT_framebuffer_object)
GL_FUNCTION(glBindFramebufferEXT,void,(GLenum target, GLuint framebuffer))
GL_FUNCTION(glBindRenderbufferEXT,void,(GLenum target, GLuint renderbuffer))
GL_FUNCTION(glCheckFramebufferStatusEXT,GLenum,(GLenum target))
GL_FUNCTION(glDeleteFramebuffersEXT,void,(GLsizei n, const GLuint* framebuffers))
GL_FUNCTION(glDeleteRenderbuffersEXT,void,(GLsizei n, const GLuint* renderbuffers))
GL_FUNCTION(glFramebufferRenderbufferEXT,void,(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer))
GL_FUNCTION(glFramebufferTexture1DEXT,void,(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))
GL_FUNCTION(glFramebufferTexture2DEXT,void,(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))
GL_FUNCTION(glFramebufferTexture3DEXT,void,(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset))
GL_FUNCTION(glGenFramebuffersEXT,void,(GLsizei n, GLuint* framebuffers))
GL_FUNCTION(glGenRenderbuffersEXT,void,(GLsizei n, GLuint* renderbuffers))
GL_FUNCTION(glGenerateMipmapEXT,void,(GLenum target))
GL_FUNCTION(glGetFramebufferAttachmentParameterivEXT,void,(GLenum target, GLenum attachment, GLenum pname, GLint* params))
GL_FUNCTION(glGetRenderbufferParameterivEXT,void,(GLenum target, GLenum pname, GLint* params))
GL_FUNCTION(glIsFramebufferEXT,GLboolean,(GLuint framebuffer))
GL_FUNCTION(glIsRenderbufferEXT,GLboolean,(GLuint renderbuffer))
GL_FUNCTION(glRenderbufferStorageEXT,void,(GLenum target, GLenum internalformat, GLsizei width, GLsizei height))
GL_GROUP_END()
#endif

#ifdef GL_EXT_histogram
GL_GROUP_BEGIN(GL_EXT_histogram)
GL_FUNCTION(glGetHistogramEXT,void,(GLenum target, GLboolean reset, GLenum format, GLenum type, void* values))
GL_FUNCTION(glGetHistogramParameterfvEXT,void,(GLenum target, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetHistogramParameterivEXT,void,(GLenum target, GLenum pname, GLint* params))
GL_FUNCTION(glGetMinmaxEXT,void,(GLenum target, GLboolean reset, GLenum format, GLenum type, void* values))
GL_FUNCTION(glGetMinmaxParameterfvEXT,void,(GLenum target, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetMinmaxParameterivEXT,void,(GLenum target, GLenum pname, GLint* params))
GL_FUNCTION(glHistogramEXT,void,(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink))
GL_FUNCTION(glMinmaxEXT,void,(GLenum target, GLenum internalformat, GLboolean sink))
GL_FUNCTION(glResetHistogramEXT,void,(GLenum target))
GL_FUNCTION(glResetMinmaxEXT,void,(GLenum target))
GL_GROUP_END()
#endif

#ifdef GL_EXT_index_array_formats
GL_GROUP_BEGIN(GL_EXT_index_array_formats)
GL_GROUP_END()
#endif

#ifdef GL_EXT_index_func
GL_GROUP_BEGIN(GL_EXT_index_func)
GL_FUNCTION(glIndexFuncEXT,void,(GLenum func, GLfloat ref))
GL_GROUP_END()
#endif

#ifdef GL_EXT_index_material
GL_GROUP_BEGIN(GL_EXT_index_material)
GL_FUNCTION(glIndexMaterialEXT,void,(GLenum face, GLenum mode))
GL_GROUP_END()
#endif

#ifdef GL_EXT_index_texture
GL_GROUP_BEGIN(GL_EXT_index_texture)
GL_GROUP_END()
#endif

#ifdef GL_EXT_light_texture
GL_GROUP_BEGIN(GL_EXT_light_texture)
GL_FUNCTION(glApplyTextureEXT,void,(GLenum mode))
GL_FUNCTION(glTextureLightEXT,void,(GLenum pname))
GL_FUNCTION(glTextureMaterialEXT,void,(GLenum face, GLenum mode))
GL_GROUP_END()
#endif

#ifdef GL_EXT_misc_attribute
GL_GROUP_BEGIN(GL_EXT_misc_attribute)
GL_GROUP_END()
#endif

#ifdef GL_EXT_multi_draw_arrays
GL_GROUP_BEGIN(GL_EXT_multi_draw_arrays)
GL_FUNCTION(glMultiDrawArraysEXT,void,(GLenum mode, GLint* first, GLsizei *count, GLsizei primcount))
GL_FUNCTION(glMultiDrawElementsEXT,void,(GLenum mode, GLsizei* count, GLenum type, const GLvoid **indices, GLsizei primcount))
GL_GROUP_END()
#endif

#ifdef GL_EXT_multisample
GL_GROUP_BEGIN(GL_EXT_multisample)
GL_FUNCTION(glSampleMaskEXT,void,(GLclampf value, GLboolean invert))
GL_FUNCTION(glSamplePatternEXT,void,(GLenum pattern))
GL_GROUP_END()
#endif

#ifdef GL_EXT_packed_pixels
GL_GROUP_BEGIN(GL_EXT_packed_pixels)
GL_GROUP_END()
#endif

#ifdef GL_EXT_paletted_texture
GL_GROUP_BEGIN(GL_EXT_paletted_texture)
GL_FUNCTION(glColorTableEXT,void,(GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const void* data))
GL_FUNCTION(glGetColorTableEXT,void,(GLenum target, GLenum format, GLenum type, void* data))
GL_FUNCTION(glGetColorTableParameterfvEXT,void,(GLenum target, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetColorTableParameterivEXT,void,(GLenum target, GLenum pname, GLint* params))
GL_GROUP_END()
#endif

#ifdef GL_EXT_pixel_buffer_object
GL_GROUP_BEGIN(GL_EXT_pixel_buffer_object)
GL_GROUP_END()
#endif

#ifdef GL_EXT_pixel_transform
GL_GROUP_BEGIN(GL_EXT_pixel_transform)
GL_FUNCTION(glGetPixelTransformParameterfvEXT,void,(GLenum target, GLenum pname, const GLfloat* params))
GL_FUNCTION(glGetPixelTransformParameterivEXT,void,(GLenum target, GLenum pname, const GLint* params))
GL_FUNCTION(glPixelTransformParameterfEXT,void,(GLenum target, GLenum pname, const GLfloat param))
GL_FUNCTION(glPixelTransformParameterfvEXT,void,(GLenum target, GLenum pname, const GLfloat* params))
GL_FUNCTION(glPixelTransformParameteriEXT,void,(GLenum target, GLenum pname, const GLint param))
GL_FUNCTION(glPixelTransformParameterivEXT,void,(GLenum target, GLenum pname, const GLint* params))
GL_GROUP_END()
#endif

#ifdef GL_EXT_pixel_transform_color_table
GL_GROUP_BEGIN(GL_EXT_pixel_transform_color_table)
GL_GROUP_END()
#endif

#ifdef GL_EXT_point_parameters
GL_GROUP_BEGIN(GL_EXT_point_parameters)
GL_FUNCTION(glPointParameterfEXT,void,(GLenum pname, GLfloat param))
GL_FUNCTION(glPointParameterfvEXT,void,(GLenum pname, GLfloat* params))
GL_GROUP_END()
#endif

#ifdef GL_EXT_polygon_offset
GL_GROUP_BEGIN(GL_EXT_polygon_offset)
GL_FUNCTION(glPolygonOffsetEXT,void,(GLfloat factor, GLfloat bias))
GL_GROUP_END()
#endif

#ifdef GL_EXT_rescale_normal
GL_GROUP_BEGIN(GL_EXT_rescale_normal)
GL_GROUP_END()
#endif

#ifdef GL_EXT_scene_marker
GL_GROUP_BEGIN(GL_EXT_scene_marker)
GL_FUNCTION(glBeginSceneEXT,void,(void))
GL_FUNCTION(glEndSceneEXT,void,(void))
GL_GROUP_END()
#endif

#ifdef GL_EXT_secondary_color
GL_GROUP_BEGIN(GL_EXT_secondary_color)
GL_FUNCTION(glSecondaryColor3bEXT,void,(GLbyte red, GLbyte green, GLbyte blue))
GL_FUNCTION(glSecondaryColor3bvEXT,void,(const GLbyte *v))
GL_FUNCTION(glSecondaryColor3dEXT,void,(GLdouble red, GLdouble green, GLdouble blue))
GL_FUNCTION(glSecondaryColor3dvEXT,void,(const GLdouble *v))
GL_FUNCTION(glSecondaryColor3fEXT,void,(GLfloat red, GLfloat green, GLfloat blue))
GL_FUNCTION(glSecondaryColor3fvEXT,void,(const GLfloat *v))
GL_FUNCTION(glSecondaryColor3iEXT,void,(GLint red, GLint green, GLint blue))
GL_FUNCTION(glSecondaryColor3ivEXT,void,(const GLint *v))
GL_FUNCTION(glSecondaryColor3sEXT,void,(GLshort red, GLshort green, GLshort blue))
GL_FUNCTION(glSecondaryColor3svEXT,void,(const GLshort *v))
GL_FUNCTION(glSecondaryColor3ubEXT,void,(GLubyte red, GLubyte green, GLubyte blue))
GL_FUNCTION(glSecondaryColor3ubvEXT,void,(const GLubyte *v))
GL_FUNCTION(glSecondaryColor3uiEXT,void,(GLuint red, GLuint green, GLuint blue))
GL_FUNCTION(glSecondaryColor3uivEXT,void,(const GLuint *v))
GL_FUNCTION(glSecondaryColor3usEXT,void,(GLushort red, GLushort green, GLushort blue))
GL_FUNCTION(glSecondaryColor3usvEXT,void,(const GLushort *v))
GL_FUNCTION(glSecondaryColorPointerEXT,void,(GLint size, GLenum type, GLsizei stride, GLvoid *pointer))
GL_GROUP_END()
#endif

#ifdef GL_EXT_separate_specular_color
GL_GROUP_BEGIN(GL_EXT_separate_specular_color)
GL_GROUP_END()
#endif

#ifdef GL_EXT_shadow_funcs
GL_GROUP_BEGIN(GL_EXT_shadow_funcs)
GL_GROUP_END()
#endif

#ifdef GL_EXT_shared_texture_palette
GL_GROUP_BEGIN(GL_EXT_shared_texture_palette)
GL_GROUP_END()
#endif

#ifdef GL_EXT_stencil_two_side
GL_GROUP_BEGIN(GL_EXT_stencil_two_side)
GL_FUNCTION(glActiveStencilFaceEXT,void,(GLenum face))
GL_GROUP_END()
#endif

#ifdef GL_EXT_stencil_wrap
GL_GROUP_BEGIN(GL_EXT_stencil_wrap)
GL_GROUP_END()
#endif

#ifdef GL_EXT_subtexture
GL_GROUP_BEGIN(GL_EXT_subtexture)
GL_FUNCTION(glTexSubImage1DEXT,void,(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels))
GL_FUNCTION(glTexSubImage2DEXT,void,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels))
GL_FUNCTION(glTexSubImage3DEXT,void,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels))
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture
GL_GROUP_BEGIN(GL_EXT_texture)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture3D
GL_GROUP_BEGIN(GL_EXT_texture3D)
GL_FUNCTION(glTexImage3DEXT,void,(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels))
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_compression_dxt1
GL_GROUP_BEGIN(GL_EXT_texture_compression_dxt1)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_compression_s3tc
GL_GROUP_BEGIN(GL_EXT_texture_compression_s3tc)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_cube_map
GL_GROUP_BEGIN(GL_EXT_texture_cube_map)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_edge_clamp
GL_GROUP_BEGIN(GL_EXT_texture_edge_clamp)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_env
GL_GROUP_BEGIN(GL_EXT_texture_env)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_env_add
GL_GROUP_BEGIN(GL_EXT_texture_env_add)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_env_combine
GL_GROUP_BEGIN(GL_EXT_texture_env_combine)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_env_dot3
GL_GROUP_BEGIN(GL_EXT_texture_env_dot3)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_filter_anisotropic
GL_GROUP_BEGIN(GL_EXT_texture_filter_anisotropic)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_lod_bias
GL_GROUP_BEGIN(GL_EXT_texture_lod_bias)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_mirror_clamp
GL_GROUP_BEGIN(GL_EXT_texture_mirror_clamp)
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_object
GL_GROUP_BEGIN(GL_EXT_texture_object)
GL_FUNCTION(glAreTexturesResidentEXT,GLboolean,(GLsizei n, const GLuint* textures, GLboolean* residences))
GL_FUNCTION(glBindTextureEXT,void,(GLenum target, GLuint texture))
GL_FUNCTION(glDeleteTexturesEXT,void,(GLsizei n, const GLuint* textures))
GL_FUNCTION(glGenTexturesEXT,void,(GLsizei n, GLuint* textures))
GL_FUNCTION(glIsTextureEXT,GLboolean,(GLuint texture))
GL_FUNCTION(glPrioritizeTexturesEXT,void,(GLsizei n, const GLuint* textures, const GLclampf* priorities))
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_perturb_normal
GL_GROUP_BEGIN(GL_EXT_texture_perturb_normal)
GL_FUNCTION(glTextureNormalEXT,void,(GLenum mode))
GL_GROUP_END()
#endif

#ifdef GL_EXT_texture_rectangle
GL_GROUP_BEGIN(GL_EXT_texture_rectangle)
GL_GROUP_END()
#endif

#ifdef GL_EXT_vertex_array
GL_GROUP_BEGIN(GL_EXT_vertex_array)
GL_FUNCTION(glArrayElementEXT,void,(GLint i))
GL_FUNCTION(glColorPointerEXT,void,(GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer))
GL_FUNCTION(glDrawArraysEXT,void,(GLenum mode, GLint first, GLsizei count))
GL_FUNCTION(glEdgeFlagPointerEXT,void,(GLsizei stride, GLsizei count, const GLboolean* pointer))
GL_FUNCTION(glGetPointervEXT,void,(GLenum pname, void** params))
GL_FUNCTION(glIndexPointerEXT,void,(GLenum type, GLsizei stride, GLsizei count, const void* pointer))
GL_FUNCTION(glNormalPointerEXT,void,(GLenum type, GLsizei stride, GLsizei count, const void* pointer))
GL_FUNCTION(glTexCoordPointerEXT,void,(GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer))
GL_FUNCTION(glVertexPointerEXT,void,(GLint size, GLenum type, GLsizei stride, GLsizei count, const void* pointer))
GL_GROUP_END()
#endif

#ifdef GL_EXT_vertex_shader
GL_GROUP_BEGIN(GL_EXT_vertex_shader)
GL_FUNCTION(glBeginVertexShaderEXT,void,(void))
GL_FUNCTION(glEndVertexShaderEXT,void,(void))
GL_FUNCTION(glBindVertexShaderEXT,void,(GLuint id))
GL_FUNCTION(glGenVertexShadersEXT,GLuint,(GLuint range))
GL_FUNCTION(glDeleteVertexShaderEXT,void,(GLuint id))
GL_FUNCTION(glShaderOp1EXT,void,(GLenum op, GLuint res, GLuint arg1))
GL_FUNCTION(glShaderOp2EXT,void,(GLenum op, GLuint res, GLuint arg1, GLuint arg2))
GL_FUNCTION(glShaderOp3EXT,void,(GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3))
GL_FUNCTION(glSwizzleEXT,void,(GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW))
GL_FUNCTION(glWriteMaskEXT,void,(GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW))
GL_FUNCTION(glInsertComponentEXT,void,(GLuint res, GLuint src, GLuint num))
GL_FUNCTION(glExtractComponentEXT,void,(GLuint res, GLuint src, GLuint num))
GL_FUNCTION(glGenSymbolsEXT,GLuint,(GLenum dataType, GLenum storageType, GLenum range, GLuint components))
GL_FUNCTION(glSetInvariantEXT,void,(GLuint id, GLenum type, GLvoid *addr))
GL_FUNCTION(glSetLocalConstantEXT,void,(GLuint id, GLenum type, GLvoid *addr))
GL_FUNCTION(glVariantbvEXT,void,(GLuint id, GLbyte *addr))
GL_FUNCTION(glVariantsvEXT,void,(GLuint id, GLshort *addr))
GL_FUNCTION(glVariantivEXT,void,(GLuint id, GLint *addr))
GL_FUNCTION(glVariantfvEXT,void,(GLuint id, GLfloat *addr))
GL_FUNCTION(glVariantdvEXT,void,(GLuint id, GLdouble *addr))
GL_FUNCTION(glVariantubvEXT,void,(GLuint id, GLubyte *addr))
GL_FUNCTION(glVariantusvEXT,void,(GLuint id, GLushort *addr))
GL_FUNCTION(glVariantuivEXT,void,(GLuint id, GLuint *addr))
GL_FUNCTION(glVariantPointerEXT,void,(GLuint id, GLenum type, GLuint stride, GLvoid *addr))
GL_FUNCTION(glEnableVariantClientStateEXT,void,(GLuint id))
GL_FUNCTION(glDisableVariantClientStateEXT,void,(GLuint id))
GL_FUNCTION(glBindLightParameterEXT,GLuint,(GLenum light, GLenum value))
GL_FUNCTION(glBindMaterialParameterEXT,GLuint,(GLenum face, GLenum value))
GL_FUNCTION(glBindTexGenParameterEXT,GLuint,(GLenum unit, GLenum coord, GLenum value))
GL_FUNCTION(glBindTextureUnitParameterEXT,GLuint,(GLenum unit, GLenum value))
GL_FUNCTION(glBindParameterEXT,GLuint,(GLenum value))
GL_FUNCTION(glIsVariantEnabledEXT,GLboolean,(GLuint id, GLenum cap))
GL_FUNCTION(glGetVariantBooleanvEXT,void,(GLuint id, GLenum value, GLboolean *data))
GL_FUNCTION(glGetVariantIntegervEXT,void,(GLuint id, GLenum value, GLint *data))
GL_FUNCTION(glGetVariantFloatvEXT,void,(GLuint id, GLenum value, GLfloat *data))
GL_FUNCTION(glGetVariantPointervEXT,void,(GLuint id, GLenum value, GLvoid **data))
GL_FUNCTION(glGetInvariantBooleanvEXT,void,(GLuint id, GLenum value, GLboolean *data))
GL_FUNCTION(glGetInvariantIntegervEXT,void,(GLuint id, GLenum value, GLint *data))
GL_FUNCTION(glGetInvariantFloatvEXT,void,(GLuint id, GLenum value, GLfloat *data))
GL_FUNCTION(glGetLocalConstantBooleanvEXT,void,(GLuint id, GLenum value, GLboolean *data))
GL_FUNCTION(glGetLocalConstantIntegervEXT,void,(GLuint id, GLenum value, GLint *data))
GL_FUNCTION(glGetLocalConstantFloatvEXT,void,(GLuint id, GLenum value, GLfloat *data))
GL_GROUP_END()
#endif

#ifdef GL_EXT_vertex_weighting
GL_GROUP_BEGIN(GL_EXT_vertex_weighting)
GL_FUNCTION(glVertexWeightPointerEXT,void,(GLint size, GLenum type, GLsizei stride, void* pointer))
GL_FUNCTION(glVertexWeightfEXT,void,(GLfloat weight))
GL_FUNCTION(glVertexWeightfvEXT,void,(GLfloat* weight))
GL_GROUP_END()
#endif

#ifdef GL_GREMEDY_string_marker
GL_GROUP_BEGIN(GL_GREMEDY_string_marker)
GL_FUNCTION(glStringMarkerGREMEDY,void,(GLsizei len, const void* string))
GL_GROUP_END()
#endif

#ifdef GL_HP_convolution_border_modes
GL_GROUP_BEGIN(GL_HP_convolution_border_modes)
GL_GROUP_END()
#endif

#ifdef GL_HP_image_transform
GL_GROUP_BEGIN(GL_HP_image_transform)
GL_FUNCTION(glGetImageTransformParameterfvHP,void,(GLenum target, GLenum pname, const GLfloat* params))
GL_FUNCTION(glGetImageTransformParameterivHP,void,(GLenum target, GLenum pname, const GLint* params))
GL_FUNCTION(glImageTransformParameterfHP,void,(GLenum target, GLenum pname, const GLfloat param))
GL_FUNCTION(glImageTransformParameterfvHP,void,(GLenum target, GLenum pname, const GLfloat* params))
GL_FUNCTION(glImageTransformParameteriHP,void,(GLenum target, GLenum pname, const GLint param))
GL_FUNCTION(glImageTransformParameterivHP,void,(GLenum target, GLenum pname, const GLint* params))
GL_GROUP_END()
#endif

#ifdef GL_HP_occlusion_test
GL_GROUP_BEGIN(GL_HP_occlusion_test)
GL_GROUP_END()
#endif

#ifdef GL_HP_texture_lighting
GL_GROUP_BEGIN(GL_HP_texture_lighting)
GL_GROUP_END()
#endif

#ifdef GL_IBM_cull_vertex
GL_GROUP_BEGIN(GL_IBM_cull_vertex)
GL_GROUP_END()
#endif

#ifdef GL_IBM_multimode_draw_arrays
GL_GROUP_BEGIN(GL_IBM_multimode_draw_arrays)
GL_FUNCTION(glMultiModeDrawArraysIBM,void,(const GLenum* mode, const GLint *first, const GLsizei *count, GLsizei primcount, GLint modestride))
GL_FUNCTION(glMultiModeDrawElementsIBM,void,(const GLenum* mode, const GLsizei *count, GLenum type, const GLvoid * const *indices, GLsizei primcount, GLint modestride))
GL_GROUP_END()
#endif

#ifdef GL_IBM_rasterpos_clip
GL_GROUP_BEGIN(GL_IBM_rasterpos_clip)
GL_GROUP_END()
#endif

#ifdef GL_IBM_static_data
GL_GROUP_BEGIN(GL_IBM_static_data)
GL_GROUP_END()
#endif

#ifdef GL_IBM_texture_mirrored_repeat
GL_GROUP_BEGIN(GL_IBM_texture_mirrored_repeat)
GL_GROUP_END()
#endif

#ifdef GL_IBM_vertex_array_lists
GL_GROUP_BEGIN(GL_IBM_vertex_array_lists)
GL_FUNCTION(glColorPointerListIBM,void,(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride))
GL_FUNCTION(glEdgeFlagPointerListIBM,void,(GLint stride, const GLboolean ** pointer, GLint ptrstride))
GL_FUNCTION(glFogCoordPointerListIBM,void,(GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride))
GL_FUNCTION(glIndexPointerListIBM,void,(GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride))
GL_FUNCTION(glNormalPointerListIBM,void,(GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride))
GL_FUNCTION(glSecondaryColorPointerListIBM,void,(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride))
GL_FUNCTION(glTexCoordPointerListIBM,void,(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride))
GL_FUNCTION(glVertexPointerListIBM,void,(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride))
GL_GROUP_END()
#endif

#ifdef GL_INGR_color_clamp
GL_GROUP_BEGIN(GL_INGR_color_clamp)
GL_GROUP_END()
#endif

#ifdef GL_INGR_interlace_read
GL_GROUP_BEGIN(GL_INGR_interlace_read)
GL_GROUP_END()
#endif

#ifdef GL_INTEL_parallel_arrays
GL_GROUP_BEGIN(GL_INTEL_parallel_arrays)
GL_FUNCTION(glColorPointervINTEL,void,(GLint size, GLenum type, const void** pointer))
GL_FUNCTION(glNormalPointervINTEL,void,(GLenum type, const void** pointer))
GL_FUNCTION(glTexCoordPointervINTEL,void,(GLint size, GLenum type, const void** pointer))
GL_FUNCTION(glVertexPointervINTEL,void,(GLint size, GLenum type, const void** pointer))
GL_GROUP_END()
#endif

#ifdef GL_INTEL_texture_scissor
GL_GROUP_BEGIN(GL_INTEL_texture_scissor)
GL_FUNCTION(glTexScissorFuncINTEL,void,(GLenum target, GLenum lfunc, GLenum hfunc))
GL_FUNCTION(glTexScissorINTEL,void,(GLenum target, GLclampf tlow, GLclampf thigh))
GL_GROUP_END()
#endif

#ifdef GL_KTX_buffer_region
GL_GROUP_BEGIN(GL_KTX_buffer_region)
GL_FUNCTION(glBufferRegionEnabledEXT,GLuint,(void))
GL_FUNCTION(glNewBufferRegionEXT,GLuint,(GLenum region))
GL_FUNCTION(glDeleteBufferRegionEXT,void,(GLenum region))
GL_FUNCTION(glReadBufferRegionEXT,void,(GLuint region, GLint x, GLint y, GLsizei width, GLsizei height))
GL_FUNCTION(glDrawBufferRegionEXT,void,(GLuint region, GLint x, GLint y, GLsizei width, GLsizei height, GLint xDest, GLint yDest))
GL_GROUP_END()
#endif

#ifdef GL_MESA_pack_invert
GL_GROUP_BEGIN(GL_MESA_pack_invert)
GL_GROUP_END()
#endif

#ifdef GL_MESA_resize_buffers
GL_GROUP_BEGIN(GL_MESA_resize_buffers)
GL_FUNCTION(glResizeBuffersMESA,void,(void))
GL_GROUP_END()
#endif

#ifdef GL_MESA_window_pos
GL_GROUP_BEGIN(GL_MESA_window_pos)
GL_FUNCTION(glWindowPos2dMESA,void,(GLdouble x, GLdouble y))
GL_FUNCTION(glWindowPos2dvMESA,void,(const GLdouble* p))
GL_FUNCTION(glWindowPos2fMESA,void,(GLfloat x, GLfloat y))
GL_FUNCTION(glWindowPos2fvMESA,void,(const GLfloat* p))
GL_FUNCTION(glWindowPos2iMESA,void,(GLint x, GLint y))
GL_FUNCTION(glWindowPos2ivMESA,void,(const GLint* p))
GL_FUNCTION(glWindowPos2sMESA,void,(GLshort x, GLshort y))
GL_FUNCTION(glWindowPos2svMESA,void,(const GLshort* p))
GL_FUNCTION(glWindowPos3dMESA,void,(GLdouble x, GLdouble y, GLdouble z))
GL_FUNCTION(glWindowPos3dvMESA,void,(const GLdouble* p))
GL_FUNCTION(glWindowPos3fMESA,void,(GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glWindowPos3fvMESA,void,(const GLfloat* p))
GL_FUNCTION(glWindowPos3iMESA,void,(GLint x, GLint y, GLint z))
GL_FUNCTION(glWindowPos3ivMESA,void,(const GLint* p))
GL_FUNCTION(glWindowPos3sMESA,void,(GLshort x, GLshort y, GLshort z))
GL_FUNCTION(glWindowPos3svMESA,void,(const GLshort* p))
GL_FUNCTION(glWindowPos4dMESA,void,(GLdouble x, GLdouble y, GLdouble z, GLdouble))
GL_FUNCTION(glWindowPos4dvMESA,void,(const GLdouble* p))
GL_FUNCTION(glWindowPos4fMESA,void,(GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_FUNCTION(glWindowPos4fvMESA,void,(const GLfloat* p))
GL_FUNCTION(glWindowPos4iMESA,void,(GLint x, GLint y, GLint z, GLint w))
GL_FUNCTION(glWindowPos4ivMESA,void,(const GLint* p))
GL_FUNCTION(glWindowPos4sMESA,void,(GLshort x, GLshort y, GLshort z, GLshort w))
GL_FUNCTION(glWindowPos4svMESA,void,(const GLshort* p))
GL_GROUP_END()
#endif

#ifdef GL_MESA_ycbcr_texture
GL_GROUP_BEGIN(GL_MESA_ycbcr_texture)
GL_GROUP_END()
#endif

#ifdef GL_NV_blend_square
GL_GROUP_BEGIN(GL_NV_blend_square)
GL_GROUP_END()
#endif

#ifdef GL_NV_copy_depth_to_color
GL_GROUP_BEGIN(GL_NV_copy_depth_to_color)
GL_GROUP_END()
#endif

#ifdef GL_NV_depth_clamp
GL_GROUP_BEGIN(GL_NV_depth_clamp)
GL_GROUP_END()
#endif

#ifdef GL_NV_evaluators
GL_GROUP_BEGIN(GL_NV_evaluators)
GL_FUNCTION(glEvalMapsNV,void,(GLenum target, GLenum mode))
GL_FUNCTION(glGetMapAttribParameterfvNV,void,(GLenum target, GLuint index, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetMapAttribParameterivNV,void,(GLenum target, GLuint index, GLenum pname, GLint* params))
GL_FUNCTION(glGetMapControlPointsNV,void,(GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLboolean packed, void* points))
GL_FUNCTION(glGetMapParameterfvNV,void,(GLenum target, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetMapParameterivNV,void,(GLenum target, GLenum pname, GLint* params))
GL_FUNCTION(glMapControlPointsNV,void,(GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder, GLint vorder, GLboolean packed, const void* points))
GL_FUNCTION(glMapParameterfvNV,void,(GLenum target, GLenum pname, const GLfloat* params))
GL_FUNCTION(glMapParameterivNV,void,(GLenum target, GLenum pname, const GLint* params))
GL_GROUP_END()
#endif

#ifdef GL_NV_fence
GL_GROUP_BEGIN(GL_NV_fence)
GL_FUNCTION(glDeleteFencesNV,void,(GLsizei n, const GLuint* fences))
GL_FUNCTION(glFinishFenceNV,void,(GLuint fence))
GL_FUNCTION(glGenFencesNV,void,(GLsizei n, GLuint* fences))
GL_FUNCTION(glGetFenceivNV,void,(GLuint fence, GLenum pname, GLint* params))
GL_FUNCTION(glIsFenceNV,GLboolean,(GLuint fence))
GL_FUNCTION(glSetFenceNV,void,(GLuint fence, GLenum condition))
GL_FUNCTION(glTestFenceNV,GLboolean,(GLuint fence))
GL_GROUP_END()
#endif

#ifdef GL_NV_float_buffer
GL_GROUP_BEGIN(GL_NV_float_buffer)
GL_GROUP_END()
#endif

#ifdef GL_NV_fog_distance
GL_GROUP_BEGIN(GL_NV_fog_distance)
GL_GROUP_END()
#endif

#ifdef GL_NV_fragment_program
GL_GROUP_BEGIN(GL_NV_fragment_program)
GL_FUNCTION(glGetProgramNamedParameterdvNV,void,(GLuint id, GLsizei len, const GLubyte* name, GLdouble *params))
GL_FUNCTION(glGetProgramNamedParameterfvNV,void,(GLuint id, GLsizei len, const GLubyte* name, GLfloat *params))
GL_FUNCTION(glProgramNamedParameter4dNV,void,(GLuint id, GLsizei len, const GLubyte* name, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_FUNCTION(glProgramNamedParameter4dvNV,void,(GLuint id, GLsizei len, const GLubyte* name, const GLdouble v[]))
GL_FUNCTION(glProgramNamedParameter4fNV,void,(GLuint id, GLsizei len, const GLubyte* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_FUNCTION(glProgramNamedParameter4fvNV,void,(GLuint id, GLsizei len, const GLubyte* name, const GLfloat v[]))
GL_GROUP_END()
#endif

#ifdef GL_NV_fragment_program2
GL_GROUP_BEGIN(GL_NV_fragment_program2)
GL_GROUP_END()
#endif

#ifdef GL_NV_fragment_program_option
GL_GROUP_BEGIN(GL_NV_fragment_program_option)
GL_GROUP_END()
#endif

#ifdef GL_NV_half_float
GL_GROUP_BEGIN(GL_NV_half_float)
GL_FUNCTION(glColor3hNV,void,(GLhalf red, GLhalf green, GLhalf blue))
GL_FUNCTION(glColor3hvNV,void,(const GLhalf* v))
GL_FUNCTION(glColor4hNV,void,(GLhalf red, GLhalf green, GLhalf blue, GLhalf alpha))
GL_FUNCTION(glColor4hvNV,void,(const GLhalf* v))
GL_FUNCTION(glFogCoordhNV,void,(GLhalf fog))
GL_FUNCTION(glFogCoordhvNV,void,(const GLhalf* fog))
GL_FUNCTION(glMultiTexCoord1hNV,void,(GLenum target, GLhalf s))
GL_FUNCTION(glMultiTexCoord1hvNV,void,(GLenum target, const GLhalf* v))
GL_FUNCTION(glMultiTexCoord2hNV,void,(GLenum target, GLhalf s, GLhalf t))
GL_FUNCTION(glMultiTexCoord2hvNV,void,(GLenum target, const GLhalf* v))
GL_FUNCTION(glMultiTexCoord3hNV,void,(GLenum target, GLhalf s, GLhalf t, GLhalf r))
GL_FUNCTION(glMultiTexCoord3hvNV,void,(GLenum target, const GLhalf* v))
GL_FUNCTION(glMultiTexCoord4hNV,void,(GLenum target, GLhalf s, GLhalf t, GLhalf r, GLhalf q))
GL_FUNCTION(glMultiTexCoord4hvNV,void,(GLenum target, const GLhalf* v))
GL_FUNCTION(glNormal3hNV,void,(GLhalf nx, GLhalf ny, GLhalf nz))
GL_FUNCTION(glNormal3hvNV,void,(const GLhalf* v))
GL_FUNCTION(glSecondaryColor3hNV,void,(GLhalf red, GLhalf green, GLhalf blue))
GL_FUNCTION(glSecondaryColor3hvNV,void,(const GLhalf* v))
GL_FUNCTION(glTexCoord1hNV,void,(GLhalf s))
GL_FUNCTION(glTexCoord1hvNV,void,(const GLhalf* v))
GL_FUNCTION(glTexCoord2hNV,void,(GLhalf s, GLhalf t))
GL_FUNCTION(glTexCoord2hvNV,void,(const GLhalf* v))
GL_FUNCTION(glTexCoord3hNV,void,(GLhalf s, GLhalf t, GLhalf r))
GL_FUNCTION(glTexCoord3hvNV,void,(const GLhalf* v))
GL_FUNCTION(glTexCoord4hNV,void,(GLhalf s, GLhalf t, GLhalf r, GLhalf q))
GL_FUNCTION(glTexCoord4hvNV,void,(const GLhalf* v))
GL_FUNCTION(glVertex2hNV,void,(GLhalf x, GLhalf y))
GL_FUNCTION(glVertex2hvNV,void,(const GLhalf* v))
GL_FUNCTION(glVertex3hNV,void,(GLhalf x, GLhalf y, GLhalf z))
GL_FUNCTION(glVertex3hvNV,void,(const GLhalf* v))
GL_FUNCTION(glVertex4hNV,void,(GLhalf x, GLhalf y, GLhalf z, GLhalf w))
GL_FUNCTION(glVertex4hvNV,void,(const GLhalf* v))
GL_FUNCTION(glVertexAttrib1hNV,void,(GLuint index, GLhalf x))
GL_FUNCTION(glVertexAttrib1hvNV,void,(GLuint index, const GLhalf* v))
GL_FUNCTION(glVertexAttrib2hNV,void,(GLuint index, GLhalf x, GLhalf y))
GL_FUNCTION(glVertexAttrib2hvNV,void,(GLuint index, const GLhalf* v))
GL_FUNCTION(glVertexAttrib3hNV,void,(GLuint index, GLhalf x, GLhalf y, GLhalf z))
GL_FUNCTION(glVertexAttrib3hvNV,void,(GLuint index, const GLhalf* v))
GL_FUNCTION(glVertexAttrib4hNV,void,(GLuint index, GLhalf x, GLhalf y, GLhalf z, GLhalf w))
GL_FUNCTION(glVertexAttrib4hvNV,void,(GLuint index, const GLhalf* v))
GL_FUNCTION(glVertexAttribs1hvNV,void,(GLuint index, GLsizei n, const GLhalf* v))
GL_FUNCTION(glVertexAttribs2hvNV,void,(GLuint index, GLsizei n, const GLhalf* v))
GL_FUNCTION(glVertexAttribs3hvNV,void,(GLuint index, GLsizei n, const GLhalf* v))
GL_FUNCTION(glVertexAttribs4hvNV,void,(GLuint index, GLsizei n, const GLhalf* v))
GL_FUNCTION(glVertexWeighthNV,void,(GLhalf weight))
GL_FUNCTION(glVertexWeighthvNV,void,(const GLhalf* weight))
GL_GROUP_END()
#endif

#ifdef GL_NV_light_max_exponent
GL_GROUP_BEGIN(GL_NV_light_max_exponent)
GL_GROUP_END()
#endif

#ifdef GL_NV_multisample_filter_hint
GL_GROUP_BEGIN(GL_NV_multisample_filter_hint)
GL_GROUP_END()
#endif

#ifdef GL_NV_occlusion_query
GL_GROUP_BEGIN(GL_NV_occlusion_query)
GL_FUNCTION(glBeginOcclusionQueryNV,void,(GLuint id))
GL_FUNCTION(glDeleteOcclusionQueriesNV,void,(GLsizei n, const GLuint* ids))
GL_FUNCTION(glEndOcclusionQueryNV,void,(void))
GL_FUNCTION(glGenOcclusionQueriesNV,void,(GLsizei n, GLuint* ids))
GL_FUNCTION(glGetOcclusionQueryivNV,void,(GLuint id, GLenum pname, GLint* params))
GL_FUNCTION(glGetOcclusionQueryuivNV,void,(GLuint id, GLenum pname, GLuint* params))
GL_FUNCTION(glIsOcclusionQueryNV,GLboolean,(GLuint id))
GL_GROUP_END()
#endif

#ifdef GL_NV_packed_depth_stencil
GL_GROUP_BEGIN(GL_NV_packed_depth_stencil)
GL_GROUP_END()
#endif

#ifdef GL_NV_pixel_data_range
GL_GROUP_BEGIN(GL_NV_pixel_data_range)
GL_FUNCTION(glFlushPixelDataRangeNV,void,(GLenum target))
GL_FUNCTION(glPixelDataRangeNV,void,(GLenum target, GLsizei length, void* pointer))
GL_GROUP_END()
#endif

#ifdef GL_NV_point_sprite
GL_GROUP_BEGIN(GL_NV_point_sprite)
GL_FUNCTION(glPointParameteriNV,void,(GLenum pname, GLint param))
GL_FUNCTION(glPointParameterivNV,void,(GLenum pname, const GLint* params))
GL_GROUP_END()
#endif

#ifdef GL_NV_primitive_restart
GL_GROUP_BEGIN(GL_NV_primitive_restart)
GL_FUNCTION(glPrimitiveRestartIndexNV,void,(GLuint index))
GL_FUNCTION(glPrimitiveRestartNV,void,(void))
GL_GROUP_END()
#endif

#ifdef GL_NV_register_combiners
GL_GROUP_BEGIN(GL_NV_register_combiners)
GL_FUNCTION(glCombinerInputNV,void,(GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage))
GL_FUNCTION(glCombinerOutputNV,void,(GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum))
GL_FUNCTION(glCombinerParameterfNV,void,(GLenum pname, GLfloat param))
GL_FUNCTION(glCombinerParameterfvNV,void,(GLenum pname, const GLfloat* params))
GL_FUNCTION(glCombinerParameteriNV,void,(GLenum pname, GLint param))
GL_FUNCTION(glCombinerParameterivNV,void,(GLenum pname, const GLint* params))
GL_FUNCTION(glFinalCombinerInputNV,void,(GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage))
GL_FUNCTION(glGetCombinerInputParameterfvNV,void,(GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetCombinerInputParameterivNV,void,(GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint* params))
GL_FUNCTION(glGetCombinerOutputParameterfvNV,void,(GLenum stage, GLenum portion, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetCombinerOutputParameterivNV,void,(GLenum stage, GLenum portion, GLenum pname, GLint* params))
GL_FUNCTION(glGetFinalCombinerInputParameterfvNV,void,(GLenum variable, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetFinalCombinerInputParameterivNV,void,(GLenum variable, GLenum pname, GLint* params))
GL_GROUP_END()
#endif

#ifdef GL_NV_register_combiners2
GL_GROUP_BEGIN(GL_NV_register_combiners2)
GL_FUNCTION(glCombinerStageParameterfvNV,void,(GLenum stage, GLenum pname, const GLfloat* params))
GL_FUNCTION(glGetCombinerStageParameterfvNV,void,(GLenum stage, GLenum pname, GLfloat* params))
GL_GROUP_END()
#endif

#ifdef GL_NV_texgen_emboss
GL_GROUP_BEGIN(GL_NV_texgen_emboss)
GL_GROUP_END()
#endif

#ifdef GL_NV_texgen_reflection
GL_GROUP_BEGIN(GL_NV_texgen_reflection)
GL_GROUP_END()
#endif

#ifdef GL_NV_texture_compression_vtc
GL_GROUP_BEGIN(GL_NV_texture_compression_vtc)
GL_GROUP_END()
#endif

#ifdef GL_NV_texture_env_combine4
GL_GROUP_BEGIN(GL_NV_texture_env_combine4)
GL_GROUP_END()
#endif

#ifdef GL_NV_texture_expand_normal
GL_GROUP_BEGIN(GL_NV_texture_expand_normal)
GL_GROUP_END()
#endif

#ifdef GL_NV_texture_rectangle
GL_GROUP_BEGIN(GL_NV_texture_rectangle)
GL_GROUP_END()
#endif

#ifdef GL_NV_texture_shader
GL_GROUP_BEGIN(GL_NV_texture_shader)
GL_GROUP_END()
#endif

#ifdef GL_NV_texture_shader2
GL_GROUP_BEGIN(GL_NV_texture_shader2)
GL_GROUP_END()
#endif

#ifdef GL_NV_texture_shader3
GL_GROUP_BEGIN(GL_NV_texture_shader3)
GL_GROUP_END()
#endif

#ifdef GL_NV_vertex_array_range
GL_GROUP_BEGIN(GL_NV_vertex_array_range)
GL_FUNCTION(glFlushVertexArrayRangeNV,void,(void))
GL_FUNCTION(glVertexArrayRangeNV,void,(GLsizei length, void* pointer))
GL_GROUP_END()
#endif

#ifdef GL_NV_vertex_array_range2
GL_GROUP_BEGIN(GL_NV_vertex_array_range2)
GL_GROUP_END()
#endif

#ifdef GL_NV_vertex_program
GL_GROUP_BEGIN(GL_NV_vertex_program)
GL_FUNCTION(glAreProgramsResidentNV,GLboolean,(GLsizei n, const GLuint* ids, GLboolean *residences))
GL_FUNCTION(glBindProgramNV,void,(GLenum target, GLuint id))
GL_FUNCTION(glDeleteProgramsNV,void,(GLsizei n, const GLuint* ids))
GL_FUNCTION(glExecuteProgramNV,void,(GLenum target, GLuint id, const GLfloat* params))
GL_FUNCTION(glGenProgramsNV,void,(GLsizei n, GLuint* ids))
GL_FUNCTION(glGetProgramParameterdvNV,void,(GLenum target, GLuint index, GLenum pname, GLdouble* params))
GL_FUNCTION(glGetProgramParameterfvNV,void,(GLenum target, GLuint index, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetProgramStringNV,void,(GLuint id, GLenum pname, GLubyte* program))
GL_FUNCTION(glGetProgramivNV,void,(GLuint id, GLenum pname, GLint* params))
GL_FUNCTION(glGetTrackMatrixivNV,void,(GLenum target, GLuint address, GLenum pname, GLint* params))
GL_FUNCTION(glGetVertexAttribPointervNV,void,(GLuint index, GLenum pname, GLvoid** pointer))
GL_FUNCTION(glGetVertexAttribdvNV,void,(GLuint index, GLenum pname, GLdouble* params))
GL_FUNCTION(glGetVertexAttribfvNV,void,(GLuint index, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetVertexAttribivNV,void,(GLuint index, GLenum pname, GLint* params))
GL_FUNCTION(glIsProgramNV,GLboolean,(GLuint id))
GL_FUNCTION(glLoadProgramNV,void,(GLenum target, GLuint id, GLsizei len, const GLubyte* program))
GL_FUNCTION(glProgramParameter4dNV,void,(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_FUNCTION(glProgramParameter4dvNV,void,(GLenum target, GLuint index, const GLdouble* params))
GL_FUNCTION(glProgramParameter4fNV,void,(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_FUNCTION(glProgramParameter4fvNV,void,(GLenum target, GLuint index, const GLfloat* params))
GL_FUNCTION(glProgramParameters4dvNV,void,(GLenum target, GLuint index, GLuint num, const GLdouble* params))
GL_FUNCTION(glProgramParameters4fvNV,void,(GLenum target, GLuint index, GLuint num, const GLfloat* params))
GL_FUNCTION(glRequestResidentProgramsNV,void,(GLsizei n, GLuint* ids))
GL_FUNCTION(glTrackMatrixNV,void,(GLenum target, GLuint address, GLenum matrix, GLenum transform))
GL_FUNCTION(glVertexAttrib1dNV,void,(GLuint index, GLdouble x))
GL_FUNCTION(glVertexAttrib1dvNV,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib1fNV,void,(GLuint index, GLfloat x))
GL_FUNCTION(glVertexAttrib1fvNV,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib1sNV,void,(GLuint index, GLshort x))
GL_FUNCTION(glVertexAttrib1svNV,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib2dNV,void,(GLuint index, GLdouble x, GLdouble y))
GL_FUNCTION(glVertexAttrib2dvNV,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib2fNV,void,(GLuint index, GLfloat x, GLfloat y))
GL_FUNCTION(glVertexAttrib2fvNV,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib2sNV,void,(GLuint index, GLshort x, GLshort y))
GL_FUNCTION(glVertexAttrib2svNV,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib3dNV,void,(GLuint index, GLdouble x, GLdouble y, GLdouble z))
GL_FUNCTION(glVertexAttrib3dvNV,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib3fNV,void,(GLuint index, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glVertexAttrib3fvNV,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib3sNV,void,(GLuint index, GLshort x, GLshort y, GLshort z))
GL_FUNCTION(glVertexAttrib3svNV,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib4dNV,void,(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_FUNCTION(glVertexAttrib4dvNV,void,(GLuint index, const GLdouble* v))
GL_FUNCTION(glVertexAttrib4fNV,void,(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_FUNCTION(glVertexAttrib4fvNV,void,(GLuint index, const GLfloat* v))
GL_FUNCTION(glVertexAttrib4sNV,void,(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w))
GL_FUNCTION(glVertexAttrib4svNV,void,(GLuint index, const GLshort* v))
GL_FUNCTION(glVertexAttrib4ubNV,void,(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w))
GL_FUNCTION(glVertexAttrib4ubvNV,void,(GLuint index, const GLubyte* v))
GL_FUNCTION(glVertexAttribPointerNV,void,(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer))
GL_FUNCTION(glVertexAttribs1dvNV,void,(GLuint index, GLsizei n, const GLdouble* v))
GL_FUNCTION(glVertexAttribs1fvNV,void,(GLuint index, GLsizei n, const GLfloat* v))
GL_FUNCTION(glVertexAttribs1svNV,void,(GLuint index, GLsizei n, const GLshort* v))
GL_FUNCTION(glVertexAttribs2dvNV,void,(GLuint index, GLsizei n, const GLdouble* v))
GL_FUNCTION(glVertexAttribs2fvNV,void,(GLuint index, GLsizei n, const GLfloat* v))
GL_FUNCTION(glVertexAttribs2svNV,void,(GLuint index, GLsizei n, const GLshort* v))
GL_FUNCTION(glVertexAttribs3dvNV,void,(GLuint index, GLsizei n, const GLdouble* v))
GL_FUNCTION(glVertexAttribs3fvNV,void,(GLuint index, GLsizei n, const GLfloat* v))
GL_FUNCTION(glVertexAttribs3svNV,void,(GLuint index, GLsizei n, const GLshort* v))
GL_FUNCTION(glVertexAttribs4dvNV,void,(GLuint index, GLsizei n, const GLdouble* v))
GL_FUNCTION(glVertexAttribs4fvNV,void,(GLuint index, GLsizei n, const GLfloat* v))
GL_FUNCTION(glVertexAttribs4svNV,void,(GLuint index, GLsizei n, const GLshort* v))
GL_FUNCTION(glVertexAttribs4ubvNV,void,(GLuint index, GLsizei n, const GLubyte* v))
GL_GROUP_END()
#endif

#ifdef GL_NV_vertex_program1_1
GL_GROUP_BEGIN(GL_NV_vertex_program1_1)
GL_GROUP_END()
#endif

#ifdef GL_NV_vertex_program2
GL_GROUP_BEGIN(GL_NV_vertex_program2)
GL_GROUP_END()
#endif

#ifdef GL_NV_vertex_program2_option
GL_GROUP_BEGIN(GL_NV_vertex_program2_option)
GL_GROUP_END()
#endif

#ifdef GL_NV_vertex_program3
GL_GROUP_BEGIN(GL_NV_vertex_program3)
GL_GROUP_END()
#endif

#ifdef GL_OML_interlace
GL_GROUP_BEGIN(GL_OML_interlace)
GL_GROUP_END()
#endif

#ifdef GL_OML_resample
GL_GROUP_BEGIN(GL_OML_resample)
GL_GROUP_END()
#endif

#ifdef GL_OML_subsample
GL_GROUP_BEGIN(GL_OML_subsample)
GL_GROUP_END()
#endif

#ifdef GL_PGI_misc_hints
GL_GROUP_BEGIN(GL_PGI_misc_hints)
GL_GROUP_END()
#endif

#ifdef GL_PGI_vertex_hints
GL_GROUP_BEGIN(GL_PGI_vertex_hints)
GL_GROUP_END()
#endif

#ifdef GL_REND_screen_coordinates
GL_GROUP_BEGIN(GL_REND_screen_coordinates)
GL_GROUP_END()
#endif

#ifdef GL_S3_s3tc
GL_GROUP_BEGIN(GL_S3_s3tc)
GL_GROUP_END()
#endif

#ifdef GL_SGIS_color_range
GL_GROUP_BEGIN(GL_SGIS_color_range)
GL_GROUP_END()
#endif

#ifdef GL_SGIS_detail_texture
GL_GROUP_BEGIN(GL_SGIS_detail_texture)
GL_FUNCTION(glDetailTexFuncSGIS,void,(GLenum target, GLsizei n, const GLfloat* points))
GL_FUNCTION(glGetDetailTexFuncSGIS,void,(GLenum target, GLfloat* points))
GL_GROUP_END()
#endif

#ifdef GL_SGIS_fog_function
GL_GROUP_BEGIN(GL_SGIS_fog_function)
GL_FUNCTION(glFogFuncSGIS,void,(GLsizei n, const GLfloat* points))
GL_FUNCTION(glGetFogFuncSGIS,void,(GLfloat* points))
GL_GROUP_END()
#endif

#ifdef GL_SGIS_generate_mipmap
GL_GROUP_BEGIN(GL_SGIS_generate_mipmap)
GL_GROUP_END()
#endif

#ifdef GL_SGIS_multisample
GL_GROUP_BEGIN(GL_SGIS_multisample)
GL_FUNCTION(glSampleMaskSGIS,void,(GLclampf value, GLboolean invert))
GL_FUNCTION(glSamplePatternSGIS,void,(GLenum pattern))
GL_GROUP_END()
#endif

#ifdef GL_SGIS_pixel_texture
GL_GROUP_BEGIN(GL_SGIS_pixel_texture)
GL_GROUP_END()
#endif

#ifdef GL_SGIS_sharpen_texture
GL_GROUP_BEGIN(GL_SGIS_sharpen_texture)
GL_FUNCTION(glGetSharpenTexFuncSGIS,void,(GLenum target, GLfloat* points))
GL_FUNCTION(glSharpenTexFuncSGIS,void,(GLenum target, GLsizei n, const GLfloat* points))
GL_GROUP_END()
#endif

#ifdef GL_SGIS_texture4D
GL_GROUP_BEGIN(GL_SGIS_texture4D)
GL_FUNCTION(glTexImage4DSGIS,void,(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei extent, GLint border, GLenum format, GLenum type, const void* pixels))
GL_FUNCTION(glTexSubImage4DSGIS,void,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei extent, GLenum format, GLenum type, const void* pixels))
GL_GROUP_END()
#endif

#ifdef GL_SGIS_texture_border_clamp
GL_GROUP_BEGIN(GL_SGIS_texture_border_clamp)
GL_GROUP_END()
#endif

#ifdef GL_SGIS_texture_edge_clamp
GL_GROUP_BEGIN(GL_SGIS_texture_edge_clamp)
GL_GROUP_END()
#endif

#ifdef GL_SGIS_texture_filter4
GL_GROUP_BEGIN(GL_SGIS_texture_filter4)
GL_FUNCTION(glGetTexFilterFuncSGIS,void,(GLenum target, GLenum filter, GLfloat* weights))
GL_FUNCTION(glTexFilterFuncSGIS,void,(GLenum target, GLenum filter, GLsizei n, const GLfloat* weights))
GL_GROUP_END()
#endif

#ifdef GL_SGIS_texture_lod
GL_GROUP_BEGIN(GL_SGIS_texture_lod)
GL_GROUP_END()
#endif

#ifdef GL_SGIS_texture_select
GL_GROUP_BEGIN(GL_SGIS_texture_select)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_async
GL_GROUP_BEGIN(GL_SGIX_async)
GL_FUNCTION(glAsyncMarkerSGIX,void,(GLuint marker))
GL_FUNCTION(glDeleteAsyncMarkersSGIX,void,(GLuint marker, GLsizei range))
GL_FUNCTION(glFinishAsyncSGIX,GLint,(GLuint* markerp))
GL_FUNCTION(glGenAsyncMarkersSGIX,GLuint,(GLsizei range))
GL_FUNCTION(glIsAsyncMarkerSGIX,GLboolean,(GLuint marker))
GL_FUNCTION(glPollAsyncSGIX,GLint,(GLuint* markerp))
GL_GROUP_END()
#endif

#ifdef GL_SGIX_async_histogram
GL_GROUP_BEGIN(GL_SGIX_async_histogram)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_async_pixel
GL_GROUP_BEGIN(GL_SGIX_async_pixel)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_blend_alpha_minmax
GL_GROUP_BEGIN(GL_SGIX_blend_alpha_minmax)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_clipmap
GL_GROUP_BEGIN(GL_SGIX_clipmap)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_depth_texture
GL_GROUP_BEGIN(GL_SGIX_depth_texture)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_flush_raster
GL_GROUP_BEGIN(GL_SGIX_flush_raster)
GL_FUNCTION(glFlushRasterSGIX,void,(void))
GL_GROUP_END()
#endif

#ifdef GL_SGIX_fog_offset
GL_GROUP_BEGIN(GL_SGIX_fog_offset)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_fog_texture
GL_GROUP_BEGIN(GL_SGIX_fog_texture)
GL_FUNCTION(glTextureFogSGIX,void,(GLenum pname))
GL_GROUP_END()
#endif

#ifdef GL_SGIX_fragment_specular_lighting
GL_GROUP_BEGIN(GL_SGIX_fragment_specular_lighting)
GL_FUNCTION(glFragmentColorMaterialSGIX,void,(GLenum face, GLenum mode))
GL_FUNCTION(glFragmentLightModelfSGIX,void,(GLenum pname, GLfloat param))
GL_FUNCTION(glFragmentLightModelfvSGIX,void,(GLenum pname, GLfloat* params))
GL_FUNCTION(glFragmentLightModeliSGIX,void,(GLenum pname, GLint param))
GL_FUNCTION(glFragmentLightModelivSGIX,void,(GLenum pname, GLint* params))
GL_FUNCTION(glFragmentLightfSGIX,void,(GLenum light, GLenum pname, GLfloat param))
GL_FUNCTION(glFragmentLightfvSGIX,void,(GLenum light, GLenum pname, GLfloat* params))
GL_FUNCTION(glFragmentLightiSGIX,void,(GLenum light, GLenum pname, GLint param))
GL_FUNCTION(glFragmentLightivSGIX,void,(GLenum light, GLenum pname, GLint* params))
GL_FUNCTION(glFragmentMaterialfSGIX,void,(GLenum face, GLenum pname, const GLfloat param))
GL_FUNCTION(glFragmentMaterialfvSGIX,void,(GLenum face, GLenum pname, const GLfloat* params))
GL_FUNCTION(glFragmentMaterialiSGIX,void,(GLenum face, GLenum pname, const GLint param))
GL_FUNCTION(glFragmentMaterialivSGIX,void,(GLenum face, GLenum pname, const GLint* params))
GL_FUNCTION(glGetFragmentLightfvSGIX,void,(GLenum light, GLenum value, GLfloat* data))
GL_FUNCTION(glGetFragmentLightivSGIX,void,(GLenum light, GLenum value, GLint* data))
GL_FUNCTION(glGetFragmentMaterialfvSGIX,void,(GLenum face, GLenum pname, const GLfloat* data))
GL_FUNCTION(glGetFragmentMaterialivSGIX,void,(GLenum face, GLenum pname, const GLint* data))
GL_GROUP_END()
#endif

#ifdef GL_SGIX_framezoom
GL_GROUP_BEGIN(GL_SGIX_framezoom)
GL_FUNCTION(glFrameZoomSGIX,void,(GLint factor))
GL_GROUP_END()
#endif

#ifdef GL_SGIX_interlace
GL_GROUP_BEGIN(GL_SGIX_interlace)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_ir_instrument1
GL_GROUP_BEGIN(GL_SGIX_ir_instrument1)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_list_priority
GL_GROUP_BEGIN(GL_SGIX_list_priority)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_pixel_texture
GL_GROUP_BEGIN(GL_SGIX_pixel_texture)
GL_FUNCTION(glPixelTexGenSGIX,void,(GLenum mode))
GL_GROUP_END()
#endif

#ifdef GL_SGIX_pixel_texture_bits
GL_GROUP_BEGIN(GL_SGIX_pixel_texture_bits)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_reference_plane
GL_GROUP_BEGIN(GL_SGIX_reference_plane)
GL_FUNCTION(glReferencePlaneSGIX,void,(const GLdouble* equation))
GL_GROUP_END()
#endif

#ifdef GL_SGIX_resample
GL_GROUP_BEGIN(GL_SGIX_resample)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_shadow
GL_GROUP_BEGIN(GL_SGIX_shadow)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_shadow_ambient
GL_GROUP_BEGIN(GL_SGIX_shadow_ambient)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_sprite
GL_GROUP_BEGIN(GL_SGIX_sprite)
GL_FUNCTION(glSpriteParameterfSGIX,void,(GLenum pname, GLfloat param))
GL_FUNCTION(glSpriteParameterfvSGIX,void,(GLenum pname, GLfloat* params))
GL_FUNCTION(glSpriteParameteriSGIX,void,(GLenum pname, GLint param))
GL_FUNCTION(glSpriteParameterivSGIX,void,(GLenum pname, GLint* params))
GL_GROUP_END()
#endif

#ifdef GL_SGIX_tag_sample_buffer
GL_GROUP_BEGIN(GL_SGIX_tag_sample_buffer)
GL_FUNCTION(glTagSampleBufferSGIX,void,(void))
GL_GROUP_END()
#endif

#ifdef GL_SGIX_texture_add_env
GL_GROUP_BEGIN(GL_SGIX_texture_add_env)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_texture_coordinate_clamp
GL_GROUP_BEGIN(GL_SGIX_texture_coordinate_clamp)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_texture_lod_bias
GL_GROUP_BEGIN(GL_SGIX_texture_lod_bias)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_texture_multi_buffer
GL_GROUP_BEGIN(GL_SGIX_texture_multi_buffer)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_texture_range
GL_GROUP_BEGIN(GL_SGIX_texture_range)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_texture_scale_bias
GL_GROUP_BEGIN(GL_SGIX_texture_scale_bias)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_vertex_preclip
GL_GROUP_BEGIN(GL_SGIX_vertex_preclip)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_vertex_preclip_hint
GL_GROUP_BEGIN(GL_SGIX_vertex_preclip_hint)
GL_GROUP_END()
#endif

#ifdef GL_SGIX_ycrcb
GL_GROUP_BEGIN(GL_SGIX_ycrcb)
GL_GROUP_END()
#endif

#ifdef GL_SGI_color_matrix
GL_GROUP_BEGIN(GL_SGI_color_matrix)
GL_GROUP_END()
#endif

#ifdef GL_SGI_color_table
GL_GROUP_BEGIN(GL_SGI_color_table)
GL_FUNCTION(glColorTableParameterfvSGI,void,(GLenum target, GLenum pname, const GLfloat* params))
GL_FUNCTION(glColorTableParameterivSGI,void,(GLenum target, GLenum pname, const GLint* params))
GL_FUNCTION(glColorTableSGI,void,(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void* table))
GL_FUNCTION(glCopyColorTableSGI,void,(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width))
GL_FUNCTION(glGetColorTableParameterfvSGI,void,(GLenum target, GLenum pname, GLfloat* params))
GL_FUNCTION(glGetColorTableParameterivSGI,void,(GLenum target, GLenum pname, GLint* params))
GL_FUNCTION(glGetColorTableSGI,void,(GLenum target, GLenum format, GLenum type, void* table))
GL_GROUP_END()
#endif

#ifdef GL_SGI_texture_color_table
GL_GROUP_BEGIN(GL_SGI_texture_color_table)
GL_GROUP_END()
#endif

#ifdef GL_SUNX_constant_data
GL_GROUP_BEGIN(GL_SUNX_constant_data)
GL_FUNCTION(glFinishTextureSUNX,void,(void))
GL_GROUP_END()
#endif

#ifdef GL_SUN_convolution_border_modes
GL_GROUP_BEGIN(GL_SUN_convolution_border_modes)
GL_GROUP_END()
#endif

#ifdef GL_SUN_global_alpha
GL_GROUP_BEGIN(GL_SUN_global_alpha)
GL_FUNCTION(glGlobalAlphaFactorbSUN,void,(GLbyte factor))
GL_FUNCTION(glGlobalAlphaFactordSUN,void,(GLdouble factor))
GL_FUNCTION(glGlobalAlphaFactorfSUN,void,(GLfloat factor))
GL_FUNCTION(glGlobalAlphaFactoriSUN,void,(GLint factor))
GL_FUNCTION(glGlobalAlphaFactorsSUN,void,(GLshort factor))
GL_FUNCTION(glGlobalAlphaFactorubSUN,void,(GLubyte factor))
GL_FUNCTION(glGlobalAlphaFactoruiSUN,void,(GLuint factor))
GL_FUNCTION(glGlobalAlphaFactorusSUN,void,(GLushort factor))
GL_GROUP_END()
#endif

#ifdef GL_SUN_mesh_array
GL_GROUP_BEGIN(GL_SUN_mesh_array)
GL_GROUP_END()
#endif

#ifdef GL_SUN_read_video_pixels
GL_GROUP_BEGIN(GL_SUN_read_video_pixels)
GL_FUNCTION(glReadVideoPixelsSUN,void,(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels))
GL_GROUP_END()
#endif

#ifdef GL_SUN_slice_accum
GL_GROUP_BEGIN(GL_SUN_slice_accum)
GL_GROUP_END()
#endif

#ifdef GL_SUN_triangle_list
GL_GROUP_BEGIN(GL_SUN_triangle_list)
GL_FUNCTION(glReplacementCodePointerSUN,void,(GLenum type, GLsizei stride, const void* pointer))
GL_FUNCTION(glReplacementCodeubSUN,void,(GLubyte code))
GL_FUNCTION(glReplacementCodeubvSUN,void,(const GLubyte* code))
GL_FUNCTION(glReplacementCodeuiSUN,void,(GLuint code))
GL_FUNCTION(glReplacementCodeuivSUN,void,(const GLuint* code))
GL_FUNCTION(glReplacementCodeusSUN,void,(GLushort code))
GL_FUNCTION(glReplacementCodeusvSUN,void,(const GLushort* code))
GL_GROUP_END()
#endif

#ifdef GL_SUN_vertex
GL_GROUP_BEGIN(GL_SUN_vertex)
GL_FUNCTION(glColor3fVertex3fSUN,void,(GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glColor3fVertex3fvSUN,void,(const GLfloat* c, const GLfloat *v))
GL_FUNCTION(glColor4fNormal3fVertex3fSUN,void,(GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glColor4fNormal3fVertex3fvSUN,void,(const GLfloat* c, const GLfloat *n, const GLfloat *v))
GL_FUNCTION(glColor4ubVertex2fSUN,void,(GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y))
GL_FUNCTION(glColor4ubVertex2fvSUN,void,(const GLubyte* c, const GLfloat *v))
GL_FUNCTION(glColor4ubVertex3fSUN,void,(GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glColor4ubVertex3fvSUN,void,(const GLubyte* c, const GLfloat *v))
GL_FUNCTION(glNormal3fVertex3fSUN,void,(GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glNormal3fVertex3fvSUN,void,(const GLfloat* n, const GLfloat *v))
GL_FUNCTION(glReplacementCodeuiColor3fVertex3fSUN,void,(GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glReplacementCodeuiColor3fVertex3fvSUN,void,(const GLuint* rc, const GLfloat *c, const GLfloat *v))
GL_FUNCTION(glReplacementCodeuiColor4fNormal3fVertex3fSUN,void,(GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glReplacementCodeuiColor4fNormal3fVertex3fvSUN,void,(const GLuint* rc, const GLfloat *c, const GLfloat *n, const GLfloat *v))
GL_FUNCTION(glReplacementCodeuiColor4ubVertex3fSUN,void,(GLuint rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glReplacementCodeuiColor4ubVertex3fvSUN,void,(const GLuint* rc, const GLubyte *c, const GLfloat *v))
GL_FUNCTION(glReplacementCodeuiNormal3fVertex3fSUN,void,(GLuint rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glReplacementCodeuiNormal3fVertex3fvSUN,void,(const GLuint* rc, const GLfloat *n, const GLfloat *v))
GL_FUNCTION(glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN,void,(GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN,void,(const GLuint* rc, const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v))
GL_FUNCTION(glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN,void,(GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN,void,(const GLuint* rc, const GLfloat *tc, const GLfloat *n, const GLfloat *v))
GL_FUNCTION(glReplacementCodeuiTexCoord2fVertex3fSUN,void,(GLuint rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glReplacementCodeuiTexCoord2fVertex3fvSUN,void,(const GLuint* rc, const GLfloat *tc, const GLfloat *v))
GL_FUNCTION(glReplacementCodeuiVertex3fSUN,void,(GLuint rc, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glReplacementCodeuiVertex3fvSUN,void,(const GLuint* rc, const GLfloat *v))
GL_FUNCTION(glTexCoord2fColor3fVertex3fSUN,void,(GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glTexCoord2fColor3fVertex3fvSUN,void,(const GLfloat* tc, const GLfloat *c, const GLfloat *v))
GL_FUNCTION(glTexCoord2fColor4fNormal3fVertex3fSUN,void,(GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glTexCoord2fColor4fNormal3fVertex3fvSUN,void,(const GLfloat* tc, const GLfloat *c, const GLfloat *n, const GLfloat *v))
GL_FUNCTION(glTexCoord2fColor4ubVertex3fSUN,void,(GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glTexCoord2fColor4ubVertex3fvSUN,void,(const GLfloat* tc, const GLubyte *c, const GLfloat *v))
GL_FUNCTION(glTexCoord2fNormal3fVertex3fSUN,void,(GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glTexCoord2fNormal3fVertex3fvSUN,void,(const GLfloat* tc, const GLfloat *n, const GLfloat *v))
GL_FUNCTION(glTexCoord2fVertex3fSUN,void,(GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z))
GL_FUNCTION(glTexCoord2fVertex3fvSUN,void,(const GLfloat* tc, const GLfloat *v))
GL_FUNCTION(glTexCoord4fColor4fNormal3fVertex4fSUN,void,(GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_FUNCTION(glTexCoord4fColor4fNormal3fVertex4fvSUN,void,(const GLfloat* tc, const GLfloat *c, const GLfloat *n, const GLfloat *v))
GL_FUNCTION(glTexCoord4fVertex4fSUN,void,(GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_FUNCTION(glTexCoord4fVertex4fvSUN,void,(const GLfloat* tc, const GLfloat *v))
GL_GROUP_END()
#endif

#ifdef GL_WIN_phong_shading
GL_GROUP_BEGIN(GL_WIN_phong_shading)
GL_GROUP_END()
#endif

#ifdef GL_WIN_specular_fog
GL_GROUP_BEGIN(GL_WIN_specular_fog)
GL_GROUP_END()
#endif

#ifdef GL_WIN_swap_hint
GL_GROUP_BEGIN(GL_WIN_swap_hint)
GL_FUNCTION(glAddSwapHintRectWIN,void,(GLint x, GLint y, GLsizei width, GLsizei height))
GL_GROUP_END()
#endif


