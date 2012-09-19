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

typedef int ptrdiff_t;
typedef void (*GLFunction)();

#ifdef GL_VERSION_1_2
#define GL_UNSIGNED_BYTE_3_3_2 0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_INT_8_8_8_8 0x8035
#define GL_UNSIGNED_INT_10_10_10_2 0x8036
#define GL_RESCALE_NORMAL 0x803A
#define GL_UNSIGNED_BYTE_2_3_3_REV 0x8362
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1
#define GL_MAX_ELEMENTS_VERTICES 0x80E8
#define GL_MAX_ELEMENTS_INDICES 0x80E9
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE_MIN_LOD 0x813A
#define GL_TEXTURE_MAX_LOD 0x813B
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
#define GL_SINGLE_COLOR 0x81F9
#define GL_SEPARATE_SPECULAR_COLOR 0x81FA
#define GL_SMOOTH_POINT_SIZE_RANGE 0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY 0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE 0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_ALIASED_POINT_SIZE_RANGE 0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE 0x846E
#define GL_PACK_SKIP_IMAGES 0x806B
#define GL_PACK_IMAGE_HEIGHT 0x806C
#define GL_UNPACK_SKIP_IMAGES 0x806D
#define GL_UNPACK_IMAGE_HEIGHT 0x806E
#define GL_TEXTURE_3D 0x806F
#define GL_PROXY_TEXTURE_3D 0x8070
#define GL_TEXTURE_DEPTH 0x8071
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#define GL_TEXTURE_BINDING_3D 0x806A
#define glDrawRangeElements XGL_FUNCPTR(glDrawRangeElements)
#define glTexImage3D XGL_FUNCPTR(glTexImage3D)
#define glTexSubImage3D XGL_FUNCPTR(glTexSubImage3D)
#define glCopyTexSubImage3D XGL_FUNCPTR(glCopyTexSubImage3D)
#endif

#ifdef GL_VERSION_1_3
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF
#define GL_ACTIVE_TEXTURE 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE 0x84E1
#define GL_MAX_TEXTURE_UNITS 0x84E2
#define GL_NORMAL_MAP 0x8511
#define GL_REFLECTION_MAP 0x8512
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP 0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP 0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE 0x851C
#define GL_COMPRESSED_ALPHA 0x84E9
#define GL_COMPRESSED_LUMINANCE 0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA 0x84EB
#define GL_COMPRESSED_INTENSITY 0x84EC
#define GL_COMPRESSED_RGB 0x84ED
#define GL_COMPRESSED_RGBA 0x84EE
#define GL_TEXTURE_COMPRESSION_HINT 0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE 0x86A0
#define GL_TEXTURE_COMPRESSED 0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS 0x86A3
#define GL_MULTISAMPLE 0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#define GL_SAMPLE_ALPHA_TO_ONE 0x809F
#define GL_SAMPLE_COVERAGE 0x80A0
#define GL_SAMPLE_BUFFERS 0x80A8
#define GL_SAMPLES 0x80A9
#define GL_SAMPLE_COVERAGE_VALUE 0x80AA
#define GL_SAMPLE_COVERAGE_INVERT 0x80AB
#define GL_MULTISAMPLE_BIT 0x20000000
#define GL_TRANSPOSE_MODELVIEW_MATRIX 0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX 0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX 0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX 0x84E6
#define GL_COMBINE 0x8570
#define GL_COMBINE_RGB 0x8571
#define GL_COMBINE_ALPHA 0x8572
#define GL_SOURCE0_RGB 0x8580
#define GL_SOURCE1_RGB 0x8581
#define GL_SOURCE2_RGB 0x8582
#define GL_SOURCE0_ALPHA 0x8588
#define GL_SOURCE1_ALPHA 0x8589
#define GL_SOURCE2_ALPHA 0x858A
#define GL_OPERAND0_RGB 0x8590
#define GL_OPERAND1_RGB 0x8591
#define GL_OPERAND2_RGB 0x8592
#define GL_OPERAND0_ALPHA 0x8598
#define GL_OPERAND1_ALPHA 0x8599
#define GL_OPERAND2_ALPHA 0x859A
#define GL_RGB_SCALE 0x8573
#define GL_ADD_SIGNED 0x8574
#define GL_INTERPOLATE 0x8575
#define GL_SUBTRACT 0x84E7
#define GL_CONSTANT 0x8576
#define GL_PRIMARY_COLOR 0x8577
#define GL_PREVIOUS 0x8578
#define GL_DOT3_RGB 0x86AE
#define GL_DOT3_RGBA 0x86AF
#define GL_CLAMP_TO_BORDER 0x812D
#define glActiveTexture XGL_FUNCPTR(glActiveTexture)
#define glClientActiveTexture XGL_FUNCPTR(glClientActiveTexture)
#define glCompressedTexImage1D XGL_FUNCPTR(glCompressedTexImage1D)
#define glCompressedTexImage2D XGL_FUNCPTR(glCompressedTexImage2D)
#define glCompressedTexImage3D XGL_FUNCPTR(glCompressedTexImage3D)
#define glCompressedTexSubImage1D XGL_FUNCPTR(glCompressedTexSubImage1D)
#define glCompressedTexSubImage2D XGL_FUNCPTR(glCompressedTexSubImage2D)
#define glCompressedTexSubImage3D XGL_FUNCPTR(glCompressedTexSubImage3D)
#define glGetCompressedTexImage XGL_FUNCPTR(glGetCompressedTexImage)
#define glLoadTransposeMatrixd XGL_FUNCPTR(glLoadTransposeMatrixd)
#define glLoadTransposeMatrixf XGL_FUNCPTR(glLoadTransposeMatrixf)
#define glMultTransposeMatrixd XGL_FUNCPTR(glMultTransposeMatrixd)
#define glMultTransposeMatrixf XGL_FUNCPTR(glMultTransposeMatrixf)
#define glMultiTexCoord1d XGL_FUNCPTR(glMultiTexCoord1d)
#define glMultiTexCoord1dv XGL_FUNCPTR(glMultiTexCoord1dv)
#define glMultiTexCoord1f XGL_FUNCPTR(glMultiTexCoord1f)
#define glMultiTexCoord1fv XGL_FUNCPTR(glMultiTexCoord1fv)
#define glMultiTexCoord1i XGL_FUNCPTR(glMultiTexCoord1i)
#define glMultiTexCoord1iv XGL_FUNCPTR(glMultiTexCoord1iv)
#define glMultiTexCoord1s XGL_FUNCPTR(glMultiTexCoord1s)
#define glMultiTexCoord1sv XGL_FUNCPTR(glMultiTexCoord1sv)
#define glMultiTexCoord2d XGL_FUNCPTR(glMultiTexCoord2d)
#define glMultiTexCoord2dv XGL_FUNCPTR(glMultiTexCoord2dv)
#define glMultiTexCoord2f XGL_FUNCPTR(glMultiTexCoord2f)
#define glMultiTexCoord2fv XGL_FUNCPTR(glMultiTexCoord2fv)
#define glMultiTexCoord2i XGL_FUNCPTR(glMultiTexCoord2i)
#define glMultiTexCoord2iv XGL_FUNCPTR(glMultiTexCoord2iv)
#define glMultiTexCoord2s XGL_FUNCPTR(glMultiTexCoord2s)
#define glMultiTexCoord2sv XGL_FUNCPTR(glMultiTexCoord2sv)
#define glMultiTexCoord3d XGL_FUNCPTR(glMultiTexCoord3d)
#define glMultiTexCoord3dv XGL_FUNCPTR(glMultiTexCoord3dv)
#define glMultiTexCoord3f XGL_FUNCPTR(glMultiTexCoord3f)
#define glMultiTexCoord3fv XGL_FUNCPTR(glMultiTexCoord3fv)
#define glMultiTexCoord3i XGL_FUNCPTR(glMultiTexCoord3i)
#define glMultiTexCoord3iv XGL_FUNCPTR(glMultiTexCoord3iv)
#define glMultiTexCoord3s XGL_FUNCPTR(glMultiTexCoord3s)
#define glMultiTexCoord3sv XGL_FUNCPTR(glMultiTexCoord3sv)
#define glMultiTexCoord4d XGL_FUNCPTR(glMultiTexCoord4d)
#define glMultiTexCoord4dv XGL_FUNCPTR(glMultiTexCoord4dv)
#define glMultiTexCoord4f XGL_FUNCPTR(glMultiTexCoord4f)
#define glMultiTexCoord4fv XGL_FUNCPTR(glMultiTexCoord4fv)
#define glMultiTexCoord4i XGL_FUNCPTR(glMultiTexCoord4i)
#define glMultiTexCoord4iv XGL_FUNCPTR(glMultiTexCoord4iv)
#define glMultiTexCoord4s XGL_FUNCPTR(glMultiTexCoord4s)
#define glMultiTexCoord4sv XGL_FUNCPTR(glMultiTexCoord4sv)
#define glSampleCoverage XGL_FUNCPTR(glSampleCoverage)
#endif

#ifdef GL_VERSION_1_4
#define GL_GENERATE_MIPMAP 0x8191
#define GL_GENERATE_MIPMAP_HINT 0x8192
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_DEPTH_COMPONENT32 0x81A7
#define GL_TEXTURE_DEPTH_SIZE 0x884A
#define GL_DEPTH_TEXTURE_MODE 0x884B
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_COMPARE_R_TO_TEXTURE 0x884E
#define GL_FOG_COORDINATE_SOURCE 0x8450
#define GL_FOG_COORDINATE 0x8451
#define GL_FRAGMENT_DEPTH 0x8452
#define GL_CURRENT_FOG_COORDINATE 0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE 0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER 0x8456
#define GL_FOG_COORDINATE_ARRAY 0x8457
#define GL_POINT_SIZE_MIN 0x8126
#define GL_POINT_SIZE_MAX 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE 0x8128
#define GL_POINT_DISTANCE_ATTENUATION 0x8129
#define GL_COLOR_SUM 0x8458
#define GL_CURRENT_SECONDARY_COLOR 0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE 0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE 0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER 0x845D
#define GL_SECONDARY_COLOR_ARRAY 0x845E
#define GL_BLEND_DST_RGB 0x80C8
#define GL_BLEND_SRC_RGB 0x80C9
#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_SRC_ALPHA 0x80CB
#define GL_INCR_WRAP 0x8507
#define GL_DECR_WRAP 0x8508
#define GL_TEXTURE_FILTER_CONTROL 0x8500
#define GL_TEXTURE_LOD_BIAS 0x8501
#define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#define GL_MIRRORED_REPEAT 0x8370
#define glBlendEquation XGL_FUNCPTR(glBlendEquation)
#define glBlendColor XGL_FUNCPTR(glBlendColor)
#define glFogCoordf XGL_FUNCPTR(glFogCoordf)
#define glFogCoordfv XGL_FUNCPTR(glFogCoordfv)
#define glFogCoordd XGL_FUNCPTR(glFogCoordd)
#define glFogCoorddv XGL_FUNCPTR(glFogCoorddv)
#define glFogCoordPointer XGL_FUNCPTR(glFogCoordPointer)
#define glMultiDrawArrays XGL_FUNCPTR(glMultiDrawArrays)
#define glMultiDrawElements XGL_FUNCPTR(glMultiDrawElements)
#define glPointParameterf XGL_FUNCPTR(glPointParameterf)
#define glPointParameterfv XGL_FUNCPTR(glPointParameterfv)
#define glSecondaryColor3b XGL_FUNCPTR(glSecondaryColor3b)
#define glSecondaryColor3bv XGL_FUNCPTR(glSecondaryColor3bv)
#define glSecondaryColor3d XGL_FUNCPTR(glSecondaryColor3d)
#define glSecondaryColor3dv XGL_FUNCPTR(glSecondaryColor3dv)
#define glSecondaryColor3f XGL_FUNCPTR(glSecondaryColor3f)
#define glSecondaryColor3fv XGL_FUNCPTR(glSecondaryColor3fv)
#define glSecondaryColor3i XGL_FUNCPTR(glSecondaryColor3i)
#define glSecondaryColor3iv XGL_FUNCPTR(glSecondaryColor3iv)
#define glSecondaryColor3s XGL_FUNCPTR(glSecondaryColor3s)
#define glSecondaryColor3sv XGL_FUNCPTR(glSecondaryColor3sv)
#define glSecondaryColor3ub XGL_FUNCPTR(glSecondaryColor3ub)
#define glSecondaryColor3ubv XGL_FUNCPTR(glSecondaryColor3ubv)
#define glSecondaryColor3ui XGL_FUNCPTR(glSecondaryColor3ui)
#define glSecondaryColor3uiv XGL_FUNCPTR(glSecondaryColor3uiv)
#define glSecondaryColor3us XGL_FUNCPTR(glSecondaryColor3us)
#define glSecondaryColor3usv XGL_FUNCPTR(glSecondaryColor3usv)
#define glSecondaryColorPointer XGL_FUNCPTR(glSecondaryColorPointer)
#define glBlendFuncSeparate XGL_FUNCPTR(glBlendFuncSeparate)
#define glWindowPos2d XGL_FUNCPTR(glWindowPos2d)
#define glWindowPos2f XGL_FUNCPTR(glWindowPos2f)
#define glWindowPos2i XGL_FUNCPTR(glWindowPos2i)
#define glWindowPos2s XGL_FUNCPTR(glWindowPos2s)
#define glWindowPos2dv XGL_FUNCPTR(glWindowPos2dv)
#define glWindowPos2fv XGL_FUNCPTR(glWindowPos2fv)
#define glWindowPos2iv XGL_FUNCPTR(glWindowPos2iv)
#define glWindowPos2sv XGL_FUNCPTR(glWindowPos2sv)
#define glWindowPos3d XGL_FUNCPTR(glWindowPos3d)
#define glWindowPos3f XGL_FUNCPTR(glWindowPos3f)
#define glWindowPos3i XGL_FUNCPTR(glWindowPos3i)
#define glWindowPos3s XGL_FUNCPTR(glWindowPos3s)
#define glWindowPos3dv XGL_FUNCPTR(glWindowPos3dv)
#define glWindowPos3fv XGL_FUNCPTR(glWindowPos3fv)
#define glWindowPos3iv XGL_FUNCPTR(glWindowPos3iv)
#define glWindowPos3sv XGL_FUNCPTR(glWindowPos3sv)
#endif

#ifdef GL_VERSION_1_5
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
#define GL_BUFFER_SIZE 0x8764
#define GL_BUFFER_USAGE 0x8765
#define GL_QUERY_COUNTER_BITS 0x8864
#define GL_CURRENT_QUERY 0x8865
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_BUFFER_ACCESS 0x88BB
#define GL_BUFFER_MAPPED 0x88BC
#define GL_BUFFER_MAP_POINTER 0x88BD
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_SAMPLES_PASSED 0x8914
#define GL_FOG_COORD_SRC GL_FOG_COORDINATE_SOURCE
#define GL_FOG_COORD GL_FOG_COORDINATE
#define GL_CURRENT_FOG_COORD GL_CURRENT_FOG_COORDINATE
#define GL_FOG_COORD_ARRAY_TYPE GL_FOG_COORDINATE_ARRAY_TYPE
#define GL_FOG_COORD_ARRAY_STRIDE GL_FOG_COORDINATE_ARRAY_STRIDE
#define GL_FOG_COORD_ARRAY_POINTER GL_FOG_COORDINATE_ARRAY_POINTER
#define GL_FOG_COORD_ARRAY GL_FOG_COORDINATE_ARRAY
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING
#define GL_SRC0_RGB GL_SOURCE0_RGB
#define GL_SRC1_RGB GL_SOURCE1_RGB
#define GL_SRC2_RGB GL_SOURCE2_RGB
#define GL_SRC0_ALPHA GL_SOURCE0_ALPHA
#define GL_SRC1_ALPHA GL_SOURCE1_ALPHA
#define GL_SRC2_ALPHA GL_SOURCE2_ALPHA
#define glGenQueries XGL_FUNCPTR(glGenQueries)
#define glDeleteQueries XGL_FUNCPTR(glDeleteQueries)
#define glIsQuery XGL_FUNCPTR(glIsQuery)
#define glBeginQuery XGL_FUNCPTR(glBeginQuery)
#define glEndQuery XGL_FUNCPTR(glEndQuery)
#define glGetQueryiv XGL_FUNCPTR(glGetQueryiv)
#define glGetQueryObjectiv XGL_FUNCPTR(glGetQueryObjectiv)
#define glGetQueryObjectuiv XGL_FUNCPTR(glGetQueryObjectuiv)
#define glBindBuffer XGL_FUNCPTR(glBindBuffer)
#define glDeleteBuffers XGL_FUNCPTR(glDeleteBuffers)
#define glGenBuffers XGL_FUNCPTR(glGenBuffers)
#define glIsBuffer XGL_FUNCPTR(glIsBuffer)
#define glBufferData XGL_FUNCPTR(glBufferData)
#define glBufferSubData XGL_FUNCPTR(glBufferSubData)
#define glGetBufferSubData XGL_FUNCPTR(glGetBufferSubData)
#define glMapBuffer XGL_FUNCPTR(glMapBuffer)
#define glUnmapBuffer XGL_FUNCPTR(glUnmapBuffer)
#define glGetBufferParameteriv XGL_FUNCPTR(glGetBufferParameteriv)
#define glGetBufferPointerv XGL_FUNCPTR(glGetBufferPointerv)
#endif

#ifdef GL_VERSION_2_0
typedef char GLchar;
#define GL_BLEND_EQUATION_RGB GL_BLEND_EQUATION
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#define GL_CURRENT_VERTEX_ATTRIB 0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE 0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#define GL_STENCIL_BACK_FUNC 0x8800
#define GL_STENCIL_BACK_FAIL 0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS 0x8803
#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_DRAW_BUFFER0 0x8825
#define GL_DRAW_BUFFER1 0x8826
#define GL_DRAW_BUFFER2 0x8827
#define GL_DRAW_BUFFER3 0x8828
#define GL_DRAW_BUFFER4 0x8829
#define GL_DRAW_BUFFER5 0x882A
#define GL_DRAW_BUFFER6 0x882B
#define GL_DRAW_BUFFER7 0x882C
#define GL_DRAW_BUFFER8 0x882D
#define GL_DRAW_BUFFER9 0x882E
#define GL_DRAW_BUFFER10 0x882F
#define GL_DRAW_BUFFER11 0x8830
#define GL_DRAW_BUFFER12 0x8831
#define GL_DRAW_BUFFER13 0x8832
#define GL_DRAW_BUFFER14 0x8833
#define GL_DRAW_BUFFER15 0x8834
#define GL_BLEND_EQUATION_ALPHA 0x883D
#define GL_POINT_SPRITE 0x8861
#define GL_COORD_REPLACE 0x8862
#define GL_MAX_VERTEX_ATTRIBS 0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_COORDS 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS 0x8B4A
#define GL_MAX_VARYING_FLOATS 0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE 0x8B4F
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_BOOL 0x8B56
#define GL_BOOL_VEC2 0x8B57
#define GL_BOOL_VEC3 0x8B58
#define GL_BOOL_VEC4 0x8B59
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#define GL_SAMPLER_1D 0x8B5D
#define GL_SAMPLER_2D 0x8B5E
#define GL_SAMPLER_3D 0x8B5F
#define GL_SAMPLER_CUBE 0x8B60
#define GL_SAMPLER_1D_SHADOW 0x8B61
#define GL_SAMPLER_2D_SHADOW 0x8B62
#define GL_DELETE_STATUS 0x8B80
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_SHADER_SOURCE_LENGTH 0x8B88
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN 0x8CA0
#define GL_LOWER_LEFT 0x8CA1
#define GL_UPPER_LEFT 0x8CA2
#define GL_STENCIL_BACK_REF 0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK 0x8CA4
#define GL_STENCIL_BACK_WRITEMASK 0x8CA5
#define glBlendEquationSeparate XGL_FUNCPTR(glBlendEquationSeparate)
#define glDrawBuffers XGL_FUNCPTR(glDrawBuffers)
#define glStencilOpSeparate XGL_FUNCPTR(glStencilOpSeparate)
#define glStencilFuncSeparate XGL_FUNCPTR(glStencilFuncSeparate)
#define glStencilMaskSeparate XGL_FUNCPTR(glStencilMaskSeparate)
#define glAttachShader XGL_FUNCPTR(glAttachShader)
#define glBindAttribLocation XGL_FUNCPTR(glBindAttribLocation)
#define glCompileShader XGL_FUNCPTR(glCompileShader)
#define glCreateProgram XGL_FUNCPTR(glCreateProgram)
#define glCreateShader XGL_FUNCPTR(glCreateShader)
#define glDeleteProgram XGL_FUNCPTR(glDeleteProgram)
#define glDeleteShader XGL_FUNCPTR(glDeleteShader)
#define glDetachShader XGL_FUNCPTR(glDetachShader)
#define glDisableVertexAttribArray XGL_FUNCPTR(glDisableVertexAttribArray)
#define glEnableVertexAttribArray XGL_FUNCPTR(glEnableVertexAttribArray)
#define glGetActiveAttrib XGL_FUNCPTR(glGetActiveAttrib)
#define glGetActiveUniform XGL_FUNCPTR(glGetActiveUniform)
#define glGetAttachedShaders XGL_FUNCPTR(glGetAttachedShaders)
#define glGetAttribLocation XGL_FUNCPTR(glGetAttribLocation)
#define glGetProgramiv XGL_FUNCPTR(glGetProgramiv)
#define glGetProgramInfoLog XGL_FUNCPTR(glGetProgramInfoLog)
#define glGetShaderiv XGL_FUNCPTR(glGetShaderiv)
#define glGetShaderInfoLog XGL_FUNCPTR(glGetShaderInfoLog)
#define glShaderSource XGL_FUNCPTR(glShaderSource)
#define glGetUniformLocation XGL_FUNCPTR(glGetUniformLocation)
#define glGetUniformfv XGL_FUNCPTR(glGetUniformfv)
#define glGetUniformiv XGL_FUNCPTR(glGetUniformiv)
#define glGetVertexAttribdv XGL_FUNCPTR(glGetVertexAttribdv)
#define glGetVertexAttribfv XGL_FUNCPTR(glGetVertexAttribfv)
#define glGetVertexAttribiv XGL_FUNCPTR(glGetVertexAttribiv)
#define glGetVertexAttribPointerv XGL_FUNCPTR(glGetVertexAttribPointerv)
#define glIsProgram XGL_FUNCPTR(glIsProgram)
#define glIsShader XGL_FUNCPTR(glIsShader)
#define glLinkProgram XGL_FUNCPTR(glLinkProgram)
#define glGetShaderSource XGL_FUNCPTR(glGetShaderSource)
#define glUseProgram XGL_FUNCPTR(glUseProgram)
#define glUniform1f XGL_FUNCPTR(glUniform1f)
#define glUniform1fv XGL_FUNCPTR(glUniform1fv)
#define glUniform1i XGL_FUNCPTR(glUniform1i)
#define glUniform1iv XGL_FUNCPTR(glUniform1iv)
#define glUniform2f XGL_FUNCPTR(glUniform2f)
#define glUniform2fv XGL_FUNCPTR(glUniform2fv)
#define glUniform2i XGL_FUNCPTR(glUniform2i)
#define glUniform2iv XGL_FUNCPTR(glUniform2iv)
#define glUniform3f XGL_FUNCPTR(glUniform3f)
#define glUniform3fv XGL_FUNCPTR(glUniform3fv)
#define glUniform3i XGL_FUNCPTR(glUniform3i)
#define glUniform3iv XGL_FUNCPTR(glUniform3iv)
#define glUniform4f XGL_FUNCPTR(glUniform4f)
#define glUniform4fv XGL_FUNCPTR(glUniform4fv)
#define glUniform4i XGL_FUNCPTR(glUniform4i)
#define glUniform4iv XGL_FUNCPTR(glUniform4iv)
#define glUniformMatrix2fv XGL_FUNCPTR(glUniformMatrix2fv)
#define glUniformMatrix3fv XGL_FUNCPTR(glUniformMatrix3fv)
#define glUniformMatrix4fv XGL_FUNCPTR(glUniformMatrix4fv)
#define glValidateProgram XGL_FUNCPTR(glValidateProgram)
#define glVertexAttrib1d XGL_FUNCPTR(glVertexAttrib1d)
#define glVertexAttrib1dv XGL_FUNCPTR(glVertexAttrib1dv)
#define glVertexAttrib1f XGL_FUNCPTR(glVertexAttrib1f)
#define glVertexAttrib1fv XGL_FUNCPTR(glVertexAttrib1fv)
#define glVertexAttrib1s XGL_FUNCPTR(glVertexAttrib1s)
#define glVertexAttrib1sv XGL_FUNCPTR(glVertexAttrib1sv)
#define glVertexAttrib2d XGL_FUNCPTR(glVertexAttrib2d)
#define glVertexAttrib2dv XGL_FUNCPTR(glVertexAttrib2dv)
#define glVertexAttrib2f XGL_FUNCPTR(glVertexAttrib2f)
#define glVertexAttrib2fv XGL_FUNCPTR(glVertexAttrib2fv)
#define glVertexAttrib2s XGL_FUNCPTR(glVertexAttrib2s)
#define glVertexAttrib2sv XGL_FUNCPTR(glVertexAttrib2sv)
#define glVertexAttrib3d XGL_FUNCPTR(glVertexAttrib3d)
#define glVertexAttrib3dv XGL_FUNCPTR(glVertexAttrib3dv)
#define glVertexAttrib3f XGL_FUNCPTR(glVertexAttrib3f)
#define glVertexAttrib3fv XGL_FUNCPTR(glVertexAttrib3fv)
#define glVertexAttrib3s XGL_FUNCPTR(glVertexAttrib3s)
#define glVertexAttrib3sv XGL_FUNCPTR(glVertexAttrib3sv)
#define glVertexAttrib4Nbv XGL_FUNCPTR(glVertexAttrib4Nbv)
#define glVertexAttrib4Niv XGL_FUNCPTR(glVertexAttrib4Niv)
#define glVertexAttrib4Nsv XGL_FUNCPTR(glVertexAttrib4Nsv)
#define glVertexAttrib4Nub XGL_FUNCPTR(glVertexAttrib4Nub)
#define glVertexAttrib4Nubv XGL_FUNCPTR(glVertexAttrib4Nubv)
#define glVertexAttrib4Nuiv XGL_FUNCPTR(glVertexAttrib4Nuiv)
#define glVertexAttrib4Nusv XGL_FUNCPTR(glVertexAttrib4Nusv)
#define glVertexAttrib4bv XGL_FUNCPTR(glVertexAttrib4bv)
#define glVertexAttrib4d XGL_FUNCPTR(glVertexAttrib4d)
#define glVertexAttrib4dv XGL_FUNCPTR(glVertexAttrib4dv)
#define glVertexAttrib4f XGL_FUNCPTR(glVertexAttrib4f)
#define glVertexAttrib4fv XGL_FUNCPTR(glVertexAttrib4fv)
#define glVertexAttrib4iv XGL_FUNCPTR(glVertexAttrib4iv)
#define glVertexAttrib4s XGL_FUNCPTR(glVertexAttrib4s)
#define glVertexAttrib4sv XGL_FUNCPTR(glVertexAttrib4sv)
#define glVertexAttrib4ubv XGL_FUNCPTR(glVertexAttrib4ubv)
#define glVertexAttrib4uiv XGL_FUNCPTR(glVertexAttrib4uiv)
#define glVertexAttrib4usv XGL_FUNCPTR(glVertexAttrib4usv)
#define glVertexAttribPointer XGL_FUNCPTR(glVertexAttribPointer)
#endif

#ifdef GL_3DFX_multisample
#define GL_MULTISAMPLE_3DFX 0x86B2
#define GL_SAMPLE_BUFFERS_3DFX 0x86B3
#define GL_SAMPLES_3DFX 0x86B4
#define GL_MULTISAMPLE_BIT_3DFX 0x20000000
#endif

#ifdef GL_3DFX_tbuffer
#define glTbufferMask3DFX XGL_FUNCPTR(glTbufferMask3DFX)
#endif

#ifdef GL_3DFX_texture_compression_FXT1
#define GL_COMPRESSED_RGB_FXT1_3DFX 0x86B0
#define GL_COMPRESSED_RGBA_FXT1_3DFX 0x86B1
#endif

#ifdef GL_APPLE_client_storage
#define GL_UNPACK_CLIENT_STORAGE_APPLE 0x85B2
#endif

#ifdef GL_APPLE_element_array
#define GL_ELEMENT_ARRAY_APPLE 0x8768
#define GL_ELEMENT_ARRAY_TYPE_APPLE 0x8769
#define GL_ELEMENT_ARRAY_POINTER_APPLE 0x876A
#define glDrawElementArrayAPPLE XGL_FUNCPTR(glDrawElementArrayAPPLE)
#define glDrawRangeElementArrayAPPLE XGL_FUNCPTR(glDrawRangeElementArrayAPPLE)
#define glElementPointerAPPLE XGL_FUNCPTR(glElementPointerAPPLE)
#define glMultiDrawElementArrayAPPLE XGL_FUNCPTR(glMultiDrawElementArrayAPPLE)
#define glMultiDrawRangeElementArrayAPPLE XGL_FUNCPTR(glMultiDrawRangeElementArrayAPPLE)
#endif

#ifdef GL_APPLE_fence
#define GL_DRAW_PIXELS_APPLE 0x8A0A
#define GL_FENCE_APPLE 0x8A0B
#define glDeleteFencesAPPLE XGL_FUNCPTR(glDeleteFencesAPPLE)
#define glFinishFenceAPPLE XGL_FUNCPTR(glFinishFenceAPPLE)
#define glFinishObjectAPPLE XGL_FUNCPTR(glFinishObjectAPPLE)
#define glGenFencesAPPLE XGL_FUNCPTR(glGenFencesAPPLE)
#define glIsFenceAPPLE XGL_FUNCPTR(glIsFenceAPPLE)
#define glSetFenceAPPLE XGL_FUNCPTR(glSetFenceAPPLE)
#define glTestFenceAPPLE XGL_FUNCPTR(glTestFenceAPPLE)
#define glTestObjectAPPLE XGL_FUNCPTR(glTestObjectAPPLE)
#endif

#ifdef GL_APPLE_float_pixels
#define GL_HALF_APPLE 0x140B
#define GL_COLOR_FLOAT_APPLE 0x8A0F
#define GL_RGBA_FLOAT32_APPLE 0x8814
#define GL_RGB_FLOAT32_APPLE 0x8815
#define GL_ALPHA_FLOAT32_APPLE 0x8816
#define GL_INTENSITY_FLOAT32_APPLE 0x8817
#define GL_LUMINANCE_FLOAT32_APPLE 0x8818
#define GL_LUMINANCE_ALPHA_FLOAT32_APPLE 0x8819
#define GL_RGBA_FLOAT16_APPLE 0x881A
#define GL_RGB_FLOAT16_APPLE 0x881B
#define GL_ALPHA_FLOAT16_APPLE 0x881C
#define GL_INTENSITY_FLOAT16_APPLE 0x881D
#define GL_LUMINANCE_FLOAT16_APPLE 0x881E
#define GL_LUMINANCE_ALPHA_FLOAT16_APPLE 0x881F
#endif

#ifdef GL_APPLE_pixel_buffer
#define GL_MIN_PBUFFER_VIEWPORT_DIMS_APPLE 0x8A10
#endif

#ifdef GL_APPLE_specular_vector
#define GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE 0x85B0
#endif

#ifdef GL_APPLE_texture_range
#define GL_TEXTURE_STORAGE_HINT_APPLE 0x85BC
#define GL_STORAGE_PRIVATE_APPLE 0x85BD
#define GL_STORAGE_CACHED_APPLE 0x85BE
#define GL_STORAGE_SHARED_APPLE 0x85BF
#define GL_TEXTURE_RANGE_LENGTH_APPLE 0x85B7
#define GL_TEXTURE_RANGE_POINTER_APPLE 0x85B8
#define glTextureRangeAPPLE XGL_FUNCPTR(glTextureRangeAPPLE)
#define glGetTexParameterPointervAPPLE XGL_FUNCPTR(glGetTexParameterPointervAPPLE)
#endif

#ifdef GL_APPLE_transform_hint
#define GL_TRANSFORM_HINT_APPLE 0x85B1
#endif

#ifdef GL_APPLE_vertex_array_object
#define GL_VERTEX_ARRAY_BINDING_APPLE 0x85B5
#define glBindVertexArrayAPPLE XGL_FUNCPTR(glBindVertexArrayAPPLE)
#define glDeleteVertexArraysAPPLE XGL_FUNCPTR(glDeleteVertexArraysAPPLE)
#define glGenVertexArraysAPPLE XGL_FUNCPTR(glGenVertexArraysAPPLE)
#define glIsVertexArrayAPPLE XGL_FUNCPTR(glIsVertexArrayAPPLE)
#endif

#ifdef GL_APPLE_vertex_array_range
#define GL_VERTEX_ARRAY_RANGE_APPLE 0x851D
#define GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE 0x851E
#define GL_VERTEX_ARRAY_STORAGE_HINT_APPLE 0x851F
#define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_APPLE 0x8520
#define GL_VERTEX_ARRAY_RANGE_POINTER_APPLE 0x8521
#define GL_STORAGE_CACHED_APPLE 0x85BE
#define GL_STORAGE_SHARED_APPLE 0x85BF
#define glFlushVertexArrayRangeAPPLE XGL_FUNCPTR(glFlushVertexArrayRangeAPPLE)
#define glVertexArrayParameteriAPPLE XGL_FUNCPTR(glVertexArrayParameteriAPPLE)
#define glVertexArrayRangeAPPLE XGL_FUNCPTR(glVertexArrayRangeAPPLE)
#endif

#ifdef GL_APPLE_ycbcr_422
#define GL_YCBCR_422_APPLE 0x85B9
#define GL_UNSIGNED_SHORT_8_8_APPLE 0x85BA
#define GL_UNSIGNED_SHORT_8_8_REV_APPLE 0x85BB
#endif

#ifdef GL_ARB_color_buffer_float
#define GL_RGBA_FLOAT_MODE_ARB 0x8820
#define GL_CLAMP_VERTEX_COLOR_ARB 0x891A
#define GL_CLAMP_FRAGMENT_COLOR_ARB 0x891B
#define GL_CLAMP_READ_COLOR_ARB 0x891C
#define GL_FIXED_ONLY_ARB 0x891D
#define glClampColorARB XGL_FUNCPTR(glClampColorARB)
#endif

#ifdef GL_ARB_depth_texture
#define GL_DEPTH_COMPONENT16_ARB 0x81A5
#define GL_DEPTH_COMPONENT24_ARB 0x81A6
#define GL_DEPTH_COMPONENT32_ARB 0x81A7
#define GL_TEXTURE_DEPTH_SIZE_ARB 0x884A
#define GL_DEPTH_TEXTURE_MODE_ARB 0x884B
#endif

#ifdef GL_ARB_draw_buffers
#define GL_MAX_DRAW_BUFFERS_ARB 0x8824
#define GL_DRAW_BUFFER0_ARB 0x8825
#define GL_DRAW_BUFFER1_ARB 0x8826
#define GL_DRAW_BUFFER2_ARB 0x8827
#define GL_DRAW_BUFFER3_ARB 0x8828
#define GL_DRAW_BUFFER4_ARB 0x8829
#define GL_DRAW_BUFFER5_ARB 0x882A
#define GL_DRAW_BUFFER6_ARB 0x882B
#define GL_DRAW_BUFFER7_ARB 0x882C
#define GL_DRAW_BUFFER8_ARB 0x882D
#define GL_DRAW_BUFFER9_ARB 0x882E
#define GL_DRAW_BUFFER10_ARB 0x882F
#define GL_DRAW_BUFFER11_ARB 0x8830
#define GL_DRAW_BUFFER12_ARB 0x8831
#define GL_DRAW_BUFFER13_ARB 0x8832
#define GL_DRAW_BUFFER14_ARB 0x8833
#define GL_DRAW_BUFFER15_ARB 0x8834
#define glDrawBuffersARB XGL_FUNCPTR(glDrawBuffersARB)
#endif

#ifdef GL_ARB_fragment_program
#define GL_FRAGMENT_PROGRAM_ARB 0x8804
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB 0x8805
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB 0x8806
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB 0x8807
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x8808
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x8809
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x880A
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB 0x880B
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB 0x880C
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB 0x880D
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x880E
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x880F
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x8810
#define GL_MAX_TEXTURE_COORDS_ARB 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB 0x8872
#endif

#ifdef GL_ARB_fragment_program_shadow
#endif

#ifdef GL_ARB_fragment_shader
#define GL_FRAGMENT_SHADER_ARB 0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB 0x8B49
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB 0x8B8B
#endif

#ifdef GL_ARB_half_float_pixel
#define GL_HALF_FLOAT_ARB 0x140B
#endif

#ifdef GL_ARB_imaging
#define GL_CONSTANT_COLOR 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define GL_CONSTANT_ALPHA 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#define GL_BLEND_COLOR 0x8005
#define GL_FUNC_ADD 0x8006
#define GL_MIN 0x8007
#define GL_MAX 0x8008
#define GL_BLEND_EQUATION 0x8009
#define GL_FUNC_SUBTRACT 0x800A
#define GL_FUNC_REVERSE_SUBTRACT 0x800B
#define GL_CONVOLUTION_1D 0x8010
#define GL_CONVOLUTION_2D 0x8011
#define GL_SEPARABLE_2D 0x8012
#define GL_CONVOLUTION_BORDER_MODE 0x8013
#define GL_CONVOLUTION_FILTER_SCALE 0x8014
#define GL_CONVOLUTION_FILTER_BIAS 0x8015
#define GL_REDUCE 0x8016
#define GL_CONVOLUTION_FORMAT 0x8017
#define GL_CONVOLUTION_WIDTH 0x8018
#define GL_CONVOLUTION_HEIGHT 0x8019
#define GL_MAX_CONVOLUTION_WIDTH 0x801A
#define GL_MAX_CONVOLUTION_HEIGHT 0x801B
#define GL_POST_CONVOLUTION_RED_SCALE 0x801C
#define GL_POST_CONVOLUTION_GREEN_SCALE 0x801D
#define GL_POST_CONVOLUTION_BLUE_SCALE 0x801E
#define GL_POST_CONVOLUTION_ALPHA_SCALE 0x801F
#define GL_POST_CONVOLUTION_RED_BIAS 0x8020
#define GL_POST_CONVOLUTION_GREEN_BIAS 0x8021
#define GL_POST_CONVOLUTION_BLUE_BIAS 0x8022
#define GL_POST_CONVOLUTION_ALPHA_BIAS 0x8023
#define GL_HISTOGRAM 0x8024
#define GL_PROXY_HISTOGRAM 0x8025
#define GL_HISTOGRAM_WIDTH 0x8026
#define GL_HISTOGRAM_FORMAT 0x8027
#define GL_HISTOGRAM_RED_SIZE 0x8028
#define GL_HISTOGRAM_GREEN_SIZE 0x8029
#define GL_HISTOGRAM_BLUE_SIZE 0x802A
#define GL_HISTOGRAM_ALPHA_SIZE 0x802B
#define GL_HISTOGRAM_LUMINANCE_SIZE 0x802C
#define GL_HISTOGRAM_SINK 0x802D
#define GL_MINMAX 0x802E
#define GL_MINMAX_FORMAT 0x802F
#define GL_MINMAX_SINK 0x8030
#define GL_TABLE_TOO_LARGE 0x8031
#define GL_COLOR_MATRIX 0x80B1
#define GL_COLOR_MATRIX_STACK_DEPTH 0x80B2
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH 0x80B3
#define GL_POST_COLOR_MATRIX_RED_SCALE 0x80B4
#define GL_POST_COLOR_MATRIX_GREEN_SCALE 0x80B5
#define GL_POST_COLOR_MATRIX_BLUE_SCALE 0x80B6
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE 0x80B7
#define GL_POST_COLOR_MATRIX_RED_BIAS 0x80B8
#define GL_POST_COLOR_MATRIX_GREEN_BIAS 0x80B9
#define GL_POST_COLOR_MATRIX_BLUE_BIAS 0x80BA
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS 0x80BB
#define GL_COLOR_TABLE 0x80D0
#define GL_POST_CONVOLUTION_COLOR_TABLE 0x80D1
#define GL_POST_COLOR_MATRIX_COLOR_TABLE 0x80D2
#define GL_PROXY_COLOR_TABLE 0x80D3
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE 0x80D4
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE 0x80D5
#define GL_COLOR_TABLE_SCALE 0x80D6
#define GL_COLOR_TABLE_BIAS 0x80D7
#define GL_COLOR_TABLE_FORMAT 0x80D8
#define GL_COLOR_TABLE_WIDTH 0x80D9
#define GL_COLOR_TABLE_RED_SIZE 0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE 0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE 0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE 0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE 0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE 0x80DF
#define GL_IGNORE_BORDER 0x8150
#define GL_CONSTANT_BORDER 0x8151
#define GL_WRAP_BORDER 0x8152
#define GL_REPLICATE_BORDER 0x8153
#define GL_CONVOLUTION_BORDER_COLOR 0x8154
#define glColorTable XGL_FUNCPTR(glColorTable)
#define glColorSubTable XGL_FUNCPTR(glColorSubTable)
#define glColorTableParameteriv XGL_FUNCPTR(glColorTableParameteriv)
#define glColorTableParameterfv XGL_FUNCPTR(glColorTableParameterfv)
#define glCopyColorSubTable XGL_FUNCPTR(glCopyColorSubTable)
#define glCopyColorTable XGL_FUNCPTR(glCopyColorTable)
#define glGetColorTable XGL_FUNCPTR(glGetColorTable)
#define glGetColorTableParameterfv XGL_FUNCPTR(glGetColorTableParameterfv)
#define glGetColorTableParameteriv XGL_FUNCPTR(glGetColorTableParameteriv)
#define glHistogram XGL_FUNCPTR(glHistogram)
#define glResetHistogram XGL_FUNCPTR(glResetHistogram)
#define glGetHistogram XGL_FUNCPTR(glGetHistogram)
#define glGetHistogramParameterfv XGL_FUNCPTR(glGetHistogramParameterfv)
#define glGetHistogramParameteriv XGL_FUNCPTR(glGetHistogramParameteriv)
#define glMinmax XGL_FUNCPTR(glMinmax)
#define glResetMinmax XGL_FUNCPTR(glResetMinmax)
#define glGetMinmaxParameterfv XGL_FUNCPTR(glGetMinmaxParameterfv)
#define glGetMinmaxParameteriv XGL_FUNCPTR(glGetMinmaxParameteriv)
#define glConvolutionFilter1D XGL_FUNCPTR(glConvolutionFilter1D)
#define glConvolutionFilter2D XGL_FUNCPTR(glConvolutionFilter2D)
#define glConvolutionParameterf XGL_FUNCPTR(glConvolutionParameterf)
#define glConvolutionParameterfv XGL_FUNCPTR(glConvolutionParameterfv)
#define glConvolutionParameteri XGL_FUNCPTR(glConvolutionParameteri)
#define glConvolutionParameteriv XGL_FUNCPTR(glConvolutionParameteriv)
#define glCopyConvolutionFilter1D XGL_FUNCPTR(glCopyConvolutionFilter1D)
#define glCopyConvolutionFilter2D XGL_FUNCPTR(glCopyConvolutionFilter2D)
#define glGetConvolutionFilter XGL_FUNCPTR(glGetConvolutionFilter)
#define glGetConvolutionParameterfv XGL_FUNCPTR(glGetConvolutionParameterfv)
#define glGetConvolutionParameteriv XGL_FUNCPTR(glGetConvolutionParameteriv)
#define glSeparableFilter2D XGL_FUNCPTR(glSeparableFilter2D)
#define glGetSeparableFilter XGL_FUNCPTR(glGetSeparableFilter)
#define glGetMinmax XGL_FUNCPTR(glGetMinmax)
#endif

#ifdef GL_ARB_matrix_palette
#define GL_MATRIX_PALETTE_ARB 0x8840
#define GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB 0x8841
#define GL_MAX_PALETTE_MATRICES_ARB 0x8842
#define GL_CURRENT_PALETTE_MATRIX_ARB 0x8843
#define GL_MATRIX_INDEX_ARRAY_ARB 0x8844
#define GL_CURRENT_MATRIX_INDEX_ARB 0x8845
#define GL_MATRIX_INDEX_ARRAY_SIZE_ARB 0x8846
#define GL_MATRIX_INDEX_ARRAY_TYPE_ARB 0x8847
#define GL_MATRIX_INDEX_ARRAY_STRIDE_ARB 0x8848
#define GL_MATRIX_INDEX_ARRAY_POINTER_ARB 0x8849
#define glCurrentPaletteMatrixARB XGL_FUNCPTR(glCurrentPaletteMatrixARB)
#define glMatrixIndexPointerARB XGL_FUNCPTR(glMatrixIndexPointerARB)
#define glMatrixIndexubvARB XGL_FUNCPTR(glMatrixIndexubvARB)
#define glMatrixIndexusvARB XGL_FUNCPTR(glMatrixIndexusvARB)
#define glMatrixIndexuivARB XGL_FUNCPTR(glMatrixIndexuivARB)
#endif

#ifdef GL_ARB_multisample
#define GL_MULTISAMPLE_ARB 0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB 0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_ARB 0x809F
#define GL_SAMPLE_COVERAGE_ARB 0x80A0
#define GL_SAMPLE_BUFFERS_ARB 0x80A8
#define GL_SAMPLES_ARB 0x80A9
#define GL_SAMPLE_COVERAGE_VALUE_ARB 0x80AA
#define GL_SAMPLE_COVERAGE_INVERT_ARB 0x80AB
#define GL_MULTISAMPLE_BIT_ARB 0x20000000
#define glSampleCoverageARB XGL_FUNCPTR(glSampleCoverageARB)
#endif

#ifdef GL_ARB_multitexture
#define GL_TEXTURE0_ARB 0x84C0
#define GL_TEXTURE1_ARB 0x84C1
#define GL_TEXTURE2_ARB 0x84C2
#define GL_TEXTURE3_ARB 0x84C3
#define GL_TEXTURE4_ARB 0x84C4
#define GL_TEXTURE5_ARB 0x84C5
#define GL_TEXTURE6_ARB 0x84C6
#define GL_TEXTURE7_ARB 0x84C7
#define GL_TEXTURE8_ARB 0x84C8
#define GL_TEXTURE9_ARB 0x84C9
#define GL_TEXTURE10_ARB 0x84CA
#define GL_TEXTURE11_ARB 0x84CB
#define GL_TEXTURE12_ARB 0x84CC
#define GL_TEXTURE13_ARB 0x84CD
#define GL_TEXTURE14_ARB 0x84CE
#define GL_TEXTURE15_ARB 0x84CF
#define GL_TEXTURE16_ARB 0x84D0
#define GL_TEXTURE17_ARB 0x84D1
#define GL_TEXTURE18_ARB 0x84D2
#define GL_TEXTURE19_ARB 0x84D3
#define GL_TEXTURE20_ARB 0x84D4
#define GL_TEXTURE21_ARB 0x84D5
#define GL_TEXTURE22_ARB 0x84D6
#define GL_TEXTURE23_ARB 0x84D7
#define GL_TEXTURE24_ARB 0x84D8
#define GL_TEXTURE25_ARB 0x84D9
#define GL_TEXTURE26_ARB 0x84DA
#define GL_TEXTURE27_ARB 0x84DB
#define GL_TEXTURE28_ARB 0x84DC
#define GL_TEXTURE29_ARB 0x84DD
#define GL_TEXTURE30_ARB 0x84DE
#define GL_TEXTURE31_ARB 0x84DF
#define GL_ACTIVE_TEXTURE_ARB 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB 0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB 0x84E2
#define glActiveTextureARB XGL_FUNCPTR(glActiveTextureARB)
#define glClientActiveTextureARB XGL_FUNCPTR(glClientActiveTextureARB)
#define glMultiTexCoord1dARB XGL_FUNCPTR(glMultiTexCoord1dARB)
#define glMultiTexCoord1dvARB XGL_FUNCPTR(glMultiTexCoord1dvARB)
#define glMultiTexCoord1fARB XGL_FUNCPTR(glMultiTexCoord1fARB)
#define glMultiTexCoord1fvARB XGL_FUNCPTR(glMultiTexCoord1fvARB)
#define glMultiTexCoord1iARB XGL_FUNCPTR(glMultiTexCoord1iARB)
#define glMultiTexCoord1ivARB XGL_FUNCPTR(glMultiTexCoord1ivARB)
#define glMultiTexCoord1sARB XGL_FUNCPTR(glMultiTexCoord1sARB)
#define glMultiTexCoord1svARB XGL_FUNCPTR(glMultiTexCoord1svARB)
#define glMultiTexCoord2dARB XGL_FUNCPTR(glMultiTexCoord2dARB)
#define glMultiTexCoord2dvARB XGL_FUNCPTR(glMultiTexCoord2dvARB)
#define glMultiTexCoord2fARB XGL_FUNCPTR(glMultiTexCoord2fARB)
#define glMultiTexCoord2fvARB XGL_FUNCPTR(glMultiTexCoord2fvARB)
#define glMultiTexCoord2iARB XGL_FUNCPTR(glMultiTexCoord2iARB)
#define glMultiTexCoord2ivARB XGL_FUNCPTR(glMultiTexCoord2ivARB)
#define glMultiTexCoord2sARB XGL_FUNCPTR(glMultiTexCoord2sARB)
#define glMultiTexCoord2svARB XGL_FUNCPTR(glMultiTexCoord2svARB)
#define glMultiTexCoord3dARB XGL_FUNCPTR(glMultiTexCoord3dARB)
#define glMultiTexCoord3dvARB XGL_FUNCPTR(glMultiTexCoord3dvARB)
#define glMultiTexCoord3fARB XGL_FUNCPTR(glMultiTexCoord3fARB)
#define glMultiTexCoord3fvARB XGL_FUNCPTR(glMultiTexCoord3fvARB)
#define glMultiTexCoord3iARB XGL_FUNCPTR(glMultiTexCoord3iARB)
#define glMultiTexCoord3ivARB XGL_FUNCPTR(glMultiTexCoord3ivARB)
#define glMultiTexCoord3sARB XGL_FUNCPTR(glMultiTexCoord3sARB)
#define glMultiTexCoord3svARB XGL_FUNCPTR(glMultiTexCoord3svARB)
#define glMultiTexCoord4dARB XGL_FUNCPTR(glMultiTexCoord4dARB)
#define glMultiTexCoord4dvARB XGL_FUNCPTR(glMultiTexCoord4dvARB)
#define glMultiTexCoord4fARB XGL_FUNCPTR(glMultiTexCoord4fARB)
#define glMultiTexCoord4fvARB XGL_FUNCPTR(glMultiTexCoord4fvARB)
#define glMultiTexCoord4iARB XGL_FUNCPTR(glMultiTexCoord4iARB)
#define glMultiTexCoord4ivARB XGL_FUNCPTR(glMultiTexCoord4ivARB)
#define glMultiTexCoord4sARB XGL_FUNCPTR(glMultiTexCoord4sARB)
#define glMultiTexCoord4svARB XGL_FUNCPTR(glMultiTexCoord4svARB)
#endif

#ifdef GL_ARB_occlusion_query
#define GL_QUERY_COUNTER_BITS_ARB 0x8864
#define GL_CURRENT_QUERY_ARB 0x8865
#define GL_QUERY_RESULT_ARB 0x8866
#define GL_QUERY_RESULT_AVAILABLE_ARB 0x8867
#define GL_SAMPLES_PASSED_ARB 0x8914
#define glBeginQueryARB XGL_FUNCPTR(glBeginQueryARB)
#define glDeleteQueriesARB XGL_FUNCPTR(glDeleteQueriesARB)
#define glEndQueryARB XGL_FUNCPTR(glEndQueryARB)
#define glGenQueriesARB XGL_FUNCPTR(glGenQueriesARB)
#define glGetQueryObjectivARB XGL_FUNCPTR(glGetQueryObjectivARB)
#define glGetQueryObjectuivARB XGL_FUNCPTR(glGetQueryObjectuivARB)
#define glGetQueryivARB XGL_FUNCPTR(glGetQueryivARB)
#define glIsQueryARB XGL_FUNCPTR(glIsQueryARB)
#endif

#ifdef GL_ARB_pixel_buffer_object
#define GL_PIXEL_PACK_BUFFER_ARB 0x88EB
#define GL_PIXEL_UNPACK_BUFFER_ARB 0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING_ARB 0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING_ARB 0x88EF
#endif

#ifdef GL_ARB_point_parameters
#define GL_POINT_SIZE_MIN_ARB 0x8126
#define GL_POINT_SIZE_MAX_ARB 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE_ARB 0x8128
#define GL_POINT_DISTANCE_ATTENUATION_ARB 0x8129
#define glPointParameterfARB XGL_FUNCPTR(glPointParameterfARB)
#define glPointParameterfvARB XGL_FUNCPTR(glPointParameterfvARB)
#endif

#ifdef GL_ARB_point_sprite
#define GL_POINT_SPRITE_ARB 0x8861
#define GL_COORD_REPLACE_ARB 0x8862
#endif

#ifdef GL_ARB_shader_objects
typedef char GLcharARB;
typedef unsigned int GLhandleARB;
#define GL_PROGRAM_OBJECT_ARB 0x8B40
#define GL_SHADER_OBJECT_ARB 0x8B48
#define GL_OBJECT_TYPE_ARB 0x8B4E
#define GL_OBJECT_SUBTYPE_ARB 0x8B4F
#define GL_FLOAT_VEC2_ARB 0x8B50
#define GL_FLOAT_VEC3_ARB 0x8B51
#define GL_FLOAT_VEC4_ARB 0x8B52
#define GL_INT_VEC2_ARB 0x8B53
#define GL_INT_VEC3_ARB 0x8B54
#define GL_INT_VEC4_ARB 0x8B55
#define GL_BOOL_ARB 0x8B56
#define GL_BOOL_VEC2_ARB 0x8B57
#define GL_BOOL_VEC3_ARB 0x8B58
#define GL_BOOL_VEC4_ARB 0x8B59
#define GL_FLOAT_MAT2_ARB 0x8B5A
#define GL_FLOAT_MAT3_ARB 0x8B5B
#define GL_FLOAT_MAT4_ARB 0x8B5C
#define GL_SAMPLER_1D_ARB 0x8B5D
#define GL_SAMPLER_2D_ARB 0x8B5E
#define GL_SAMPLER_3D_ARB 0x8B5F
#define GL_SAMPLER_CUBE_ARB 0x8B60
#define GL_SAMPLER_1D_SHADOW_ARB 0x8B61
#define GL_SAMPLER_2D_SHADOW_ARB 0x8B62
#define GL_SAMPLER_2D_RECT_ARB 0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW_ARB 0x8B64
#define GL_OBJECT_DELETE_STATUS_ARB 0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB 0x8B81
#define GL_OBJECT_LINK_STATUS_ARB 0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB 0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB 0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB 0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB 0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB 0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB 0x8B88
#define glAttachObjectARB XGL_FUNCPTR(glAttachObjectARB)
#define glCompileShaderARB XGL_FUNCPTR(glCompileShaderARB)
#define glCreateProgramObjectARB XGL_FUNCPTR(glCreateProgramObjectARB)
#define glCreateShaderObjectARB XGL_FUNCPTR(glCreateShaderObjectARB)
#define glDeleteObjectARB XGL_FUNCPTR(glDeleteObjectARB)
#define glDetachObjectARB XGL_FUNCPTR(glDetachObjectARB)
#define glGetActiveUniformARB XGL_FUNCPTR(glGetActiveUniformARB)
#define glGetAttachedObjectsARB XGL_FUNCPTR(glGetAttachedObjectsARB)
#define glGetHandleARB XGL_FUNCPTR(glGetHandleARB)
#define glGetInfoLogARB XGL_FUNCPTR(glGetInfoLogARB)
#define glGetObjectParameterfvARB XGL_FUNCPTR(glGetObjectParameterfvARB)
#define glGetObjectParameterivARB XGL_FUNCPTR(glGetObjectParameterivARB)
#define glGetShaderSourceARB XGL_FUNCPTR(glGetShaderSourceARB)
#define glGetUniformLocationARB XGL_FUNCPTR(glGetUniformLocationARB)
#define glGetUniformfvARB XGL_FUNCPTR(glGetUniformfvARB)
#define glGetUniformivARB XGL_FUNCPTR(glGetUniformivARB)
#define glLinkProgramARB XGL_FUNCPTR(glLinkProgramARB)
#define glShaderSourceARB XGL_FUNCPTR(glShaderSourceARB)
#define glUniform1fARB XGL_FUNCPTR(glUniform1fARB)
#define glUniform1fvARB XGL_FUNCPTR(glUniform1fvARB)
#define glUniform1iARB XGL_FUNCPTR(glUniform1iARB)
#define glUniform1ivARB XGL_FUNCPTR(glUniform1ivARB)
#define glUniform2fARB XGL_FUNCPTR(glUniform2fARB)
#define glUniform2fvARB XGL_FUNCPTR(glUniform2fvARB)
#define glUniform2iARB XGL_FUNCPTR(glUniform2iARB)
#define glUniform2ivARB XGL_FUNCPTR(glUniform2ivARB)
#define glUniform3fARB XGL_FUNCPTR(glUniform3fARB)
#define glUniform3fvARB XGL_FUNCPTR(glUniform3fvARB)
#define glUniform3iARB XGL_FUNCPTR(glUniform3iARB)
#define glUniform3ivARB XGL_FUNCPTR(glUniform3ivARB)
#define glUniform4fARB XGL_FUNCPTR(glUniform4fARB)
#define glUniform4fvARB XGL_FUNCPTR(glUniform4fvARB)
#define glUniform4iARB XGL_FUNCPTR(glUniform4iARB)
#define glUniform4ivARB XGL_FUNCPTR(glUniform4ivARB)
#define glUniformMatrix2fvARB XGL_FUNCPTR(glUniformMatrix2fvARB)
#define glUniformMatrix3fvARB XGL_FUNCPTR(glUniformMatrix3fvARB)
#define glUniformMatrix4fvARB XGL_FUNCPTR(glUniformMatrix4fvARB)
#define glUseProgramObjectARB XGL_FUNCPTR(glUseProgramObjectARB)
#define glValidateProgramARB XGL_FUNCPTR(glValidateProgramARB)
#endif

#ifdef GL_ARB_shading_language_100
#define GL_SHADING_LANGUAGE_VERSION_ARB 0x8B8C
#endif

#ifdef GL_ARB_shadow
#define GL_TEXTURE_COMPARE_MODE_ARB 0x884C
#define GL_TEXTURE_COMPARE_FUNC_ARB 0x884D
#define GL_COMPARE_R_TO_TEXTURE_ARB 0x884E
#endif

#ifdef GL_ARB_shadow_ambient
#define GL_TEXTURE_COMPARE_FAIL_VALUE_ARB 0x80BF
#endif

#ifdef GL_ARB_texture_border_clamp
#define GL_CLAMP_TO_BORDER_ARB 0x812D
#endif

#ifdef GL_ARB_texture_compression
#define GL_COMPRESSED_ALPHA_ARB 0x84E9
#define GL_COMPRESSED_LUMINANCE_ARB 0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA_ARB 0x84EB
#define GL_COMPRESSED_INTENSITY_ARB 0x84EC
#define GL_COMPRESSED_RGB_ARB 0x84ED
#define GL_COMPRESSED_RGBA_ARB 0x84EE
#define GL_TEXTURE_COMPRESSION_HINT_ARB 0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB 0x86A0
#define GL_TEXTURE_COMPRESSED_ARB 0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A3
#define glCompressedTexImage1DARB XGL_FUNCPTR(glCompressedTexImage1DARB)
#define glCompressedTexImage2DARB XGL_FUNCPTR(glCompressedTexImage2DARB)
#define glCompressedTexImage3DARB XGL_FUNCPTR(glCompressedTexImage3DARB)
#define glCompressedTexSubImage1DARB XGL_FUNCPTR(glCompressedTexSubImage1DARB)
#define glCompressedTexSubImage2DARB XGL_FUNCPTR(glCompressedTexSubImage2DARB)
#define glCompressedTexSubImage3DARB XGL_FUNCPTR(glCompressedTexSubImage3DARB)
#define glGetCompressedTexImageARB XGL_FUNCPTR(glGetCompressedTexImageARB)
#endif

#ifdef GL_ARB_texture_cube_map
#define GL_NORMAL_MAP_ARB 0x8511
#define GL_REFLECTION_MAP_ARB 0x8512
#define GL_TEXTURE_CUBE_MAP_ARB 0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB 0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB 0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB 0x851C
#endif

#ifdef GL_ARB_texture_env_add
#endif

#ifdef GL_ARB_texture_env_combine
#define GL_SUBTRACT_ARB 0x84E7
#define GL_COMBINE_ARB 0x8570
#define GL_COMBINE_RGB_ARB 0x8571
#define GL_COMBINE_ALPHA_ARB 0x8572
#define GL_RGB_SCALE_ARB 0x8573
#define GL_ADD_SIGNED_ARB 0x8574
#define GL_INTERPOLATE_ARB 0x8575
#define GL_CONSTANT_ARB 0x8576
#define GL_PRIMARY_COLOR_ARB 0x8577
#define GL_PREVIOUS_ARB 0x8578
#define GL_SOURCE0_RGB_ARB 0x8580
#define GL_SOURCE1_RGB_ARB 0x8581
#define GL_SOURCE2_RGB_ARB 0x8582
#define GL_SOURCE0_ALPHA_ARB 0x8588
#define GL_SOURCE1_ALPHA_ARB 0x8589
#define GL_SOURCE2_ALPHA_ARB 0x858A
#define GL_OPERAND0_RGB_ARB 0x8590
#define GL_OPERAND1_RGB_ARB 0x8591
#define GL_OPERAND2_RGB_ARB 0x8592
#define GL_OPERAND0_ALPHA_ARB 0x8598
#define GL_OPERAND1_ALPHA_ARB 0x8599
#define GL_OPERAND2_ALPHA_ARB 0x859A
#endif

#ifdef GL_ARB_texture_env_crossbar
#endif

#ifdef GL_ARB_texture_env_dot3
#define GL_DOT3_RGB_ARB 0x86AE
#define GL_DOT3_RGBA_ARB 0x86AF
#endif

#ifdef GL_ARB_texture_float
#define GL_RGBA32F_ARB 0x8814
#define GL_RGB32F_ARB 0x8815
#define GL_ALPHA32F_ARB 0x8816
#define GL_INTENSITY32F_ARB 0x8817
#define GL_LUMINANCE32F_ARB 0x8818
#define GL_LUMINANCE_ALPHA32F_ARB 0x8819
#define GL_RGBA16F_ARB 0x881A
#define GL_RGB16F_ARB 0x881B
#define GL_ALPHA16F_ARB 0x881C
#define GL_INTENSITY16F_ARB 0x881D
#define GL_LUMINANCE16F_ARB 0x881E
#define GL_LUMINANCE_ALPHA16F_ARB 0x881F
#define GL_TEXTURE_RED_TYPE_ARB 0x8C10
#define GL_TEXTURE_GREEN_TYPE_ARB 0x8C11
#define GL_TEXTURE_BLUE_TYPE_ARB 0x8C12
#define GL_TEXTURE_ALPHA_TYPE_ARB 0x8C13
#define GL_TEXTURE_LUMINANCE_TYPE_ARB 0x8C14
#define GL_TEXTURE_INTENSITY_TYPE_ARB 0x8C15
#define GL_TEXTURE_DEPTH_TYPE_ARB 0x8C16
#define GL_UNSIGNED_NORMALIZED_ARB 0x8C17
#endif

#ifdef GL_ARB_texture_mirrored_repeat
#define GL_MIRRORED_REPEAT_ARB 0x8370
#endif

#ifdef GL_ARB_texture_non_power_of_two
#endif

#ifdef GL_ARB_texture_rectangle
#define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_ARB 0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_ARB 0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB 0x84F8
#endif

#ifdef GL_ARB_transpose_matrix
#define GL_TRANSPOSE_MODELVIEW_MATRIX_ARB 0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX_ARB 0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX_ARB 0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX_ARB 0x84E6
#define glLoadTransposeMatrixfARB XGL_FUNCPTR(glLoadTransposeMatrixfARB)
#define glLoadTransposeMatrixdARB XGL_FUNCPTR(glLoadTransposeMatrixdARB)
#define glMultTransposeMatrixfARB XGL_FUNCPTR(glMultTransposeMatrixfARB)
#define glMultTransposeMatrixdARB XGL_FUNCPTR(glMultTransposeMatrixdARB)
#endif

#ifdef GL_ARB_vertex_blend
#define GL_MAX_VERTEX_UNITS_ARB 0x86A4
#define GL_ACTIVE_VERTEX_UNITS_ARB 0x86A5
#define GL_WEIGHT_SUM_UNITY_ARB 0x86A6
#define GL_VERTEX_BLEND_ARB 0x86A7
#define GL_CURRENT_WEIGHT_ARB 0x86A8
#define GL_WEIGHT_ARRAY_TYPE_ARB 0x86A9
#define GL_WEIGHT_ARRAY_STRIDE_ARB 0x86AA
#define GL_WEIGHT_ARRAY_SIZE_ARB 0x86AB
#define GL_WEIGHT_ARRAY_POINTER_ARB 0x86AC
#define GL_WEIGHT_ARRAY_ARB 0x86AD
#define GL_MODELVIEW0_ARB 0x1700
#define GL_MODELVIEW1_ARB 0x850A
#define GL_MODELVIEW2_ARB 0x8722
#define GL_MODELVIEW3_ARB 0x8723
#define GL_MODELVIEW4_ARB 0x8724
#define GL_MODELVIEW5_ARB 0x8725
#define GL_MODELVIEW6_ARB 0x8726
#define GL_MODELVIEW7_ARB 0x8727
#define GL_MODELVIEW8_ARB 0x8728
#define GL_MODELVIEW9_ARB 0x8729
#define GL_MODELVIEW10_ARB 0x872A
#define GL_MODELVIEW11_ARB 0x872B
#define GL_MODELVIEW12_ARB 0x872C
#define GL_MODELVIEW13_ARB 0x872D
#define GL_MODELVIEW14_ARB 0x872E
#define GL_MODELVIEW15_ARB 0x872F
#define GL_MODELVIEW16_ARB 0x8730
#define GL_MODELVIEW17_ARB 0x8731
#define GL_MODELVIEW18_ARB 0x8732
#define GL_MODELVIEW19_ARB 0x8733
#define GL_MODELVIEW20_ARB 0x8734
#define GL_MODELVIEW21_ARB 0x8735
#define GL_MODELVIEW22_ARB 0x8736
#define GL_MODELVIEW23_ARB 0x8737
#define GL_MODELVIEW24_ARB 0x8738
#define GL_MODELVIEW25_ARB 0x8739
#define GL_MODELVIEW26_ARB 0x873A
#define GL_MODELVIEW27_ARB 0x873B
#define GL_MODELVIEW28_ARB 0x873C
#define GL_MODELVIEW29_ARB 0x873D
#define GL_MODELVIEW30_ARB 0x873E
#define GL_MODELVIEW31_ARB 0x873F
#define glWeightbvARB XGL_FUNCPTR(glWeightbvARB)
#define glWeightsvARB XGL_FUNCPTR(glWeightsvARB)
#define glWeightivARB XGL_FUNCPTR(glWeightivARB)
#define glWeightfvARB XGL_FUNCPTR(glWeightfvARB)
#define glWeightdvARB XGL_FUNCPTR(glWeightdvARB)
#define glWeightubvARB XGL_FUNCPTR(glWeightubvARB)
#define glWeightusvARB XGL_FUNCPTR(glWeightusvARB)
#define glWeightuivARB XGL_FUNCPTR(glWeightuivARB)
#define glWeightPointerARB XGL_FUNCPTR(glWeightPointerARB)
#define glVertexBlendARB XGL_FUNCPTR(glVertexBlendARB)
#endif

#ifdef GL_ARB_vertex_buffer_object
typedef ptrdiff_t GLsizeiptrARB;
typedef ptrdiff_t GLintptrARB;
#define GL_BUFFER_SIZE_ARB 0x8764
#define GL_BUFFER_USAGE_ARB 0x8765
#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB 0x8893
#define GL_ARRAY_BUFFER_BINDING_ARB 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB 0x889F
#define GL_READ_ONLY_ARB 0x88B8
#define GL_WRITE_ONLY_ARB 0x88B9
#define GL_READ_WRITE_ARB 0x88BA
#define GL_BUFFER_ACCESS_ARB 0x88BB
#define GL_BUFFER_MAPPED_ARB 0x88BC
#define GL_BUFFER_MAP_POINTER_ARB 0x88BD
#define GL_STREAM_DRAW_ARB 0x88E0
#define GL_STREAM_READ_ARB 0x88E1
#define GL_STREAM_COPY_ARB 0x88E2
#define GL_STATIC_DRAW_ARB 0x88E4
#define GL_STATIC_READ_ARB 0x88E5
#define GL_STATIC_COPY_ARB 0x88E6
#define GL_DYNAMIC_DRAW_ARB 0x88E8
#define GL_DYNAMIC_READ_ARB 0x88E9
#define GL_DYNAMIC_COPY_ARB 0x88EA
#define glBindBufferARB XGL_FUNCPTR(glBindBufferARB)
#define glBufferDataARB XGL_FUNCPTR(glBufferDataARB)
#define glBufferSubDataARB XGL_FUNCPTR(glBufferSubDataARB)
#define glDeleteBuffersARB XGL_FUNCPTR(glDeleteBuffersARB)
#define glGenBuffersARB XGL_FUNCPTR(glGenBuffersARB)
#define glGetBufferParameterivARB XGL_FUNCPTR(glGetBufferParameterivARB)
#define glGetBufferPointervARB XGL_FUNCPTR(glGetBufferPointervARB)
#define glGetBufferSubDataARB XGL_FUNCPTR(glGetBufferSubDataARB)
#define glIsBufferARB XGL_FUNCPTR(glIsBufferARB)
#define glMapBufferARB XGL_FUNCPTR(glMapBufferARB)
#define glUnmapBufferARB XGL_FUNCPTR(glUnmapBufferARB)
#endif

#ifdef GL_ARB_vertex_program
#define GL_COLOR_SUM_ARB 0x8458
#define GL_VERTEX_PROGRAM_ARB 0x8620
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB 0x8625
#define GL_CURRENT_VERTEX_ATTRIB_ARB 0x8626
#define GL_PROGRAM_LENGTH_ARB 0x8627
#define GL_PROGRAM_STRING_ARB 0x8628
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB 0x862E
#define GL_MAX_PROGRAM_MATRICES_ARB 0x862F
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB 0x8640
#define GL_CURRENT_MATRIX_ARB 0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB 0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB 0x8645
#define GL_PROGRAM_ERROR_POSITION_ARB 0x864B
#define GL_PROGRAM_BINDING_ARB 0x8677
#define GL_MAX_VERTEX_ATTRIBS_ARB 0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB 0x886A
#define GL_PROGRAM_ERROR_STRING_ARB 0x8874
#define GL_PROGRAM_FORMAT_ASCII_ARB 0x8875
#define GL_PROGRAM_FORMAT_ARB 0x8876
#define GL_PROGRAM_INSTRUCTIONS_ARB 0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB 0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB 0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB 0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A6
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A7
#define GL_PROGRAM_PARAMETERS_ARB 0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB 0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB 0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB 0x88AB
#define GL_PROGRAM_ATTRIBS_ARB 0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB 0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB 0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB 0x88AF
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB 0x88B0
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB 0x88B1
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B2
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B3
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB 0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB 0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB 0x88B6
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB 0x88B7
#define GL_MATRIX0_ARB 0x88C0
#define GL_MATRIX1_ARB 0x88C1
#define GL_MATRIX2_ARB 0x88C2
#define GL_MATRIX3_ARB 0x88C3
#define GL_MATRIX4_ARB 0x88C4
#define GL_MATRIX5_ARB 0x88C5
#define GL_MATRIX6_ARB 0x88C6
#define GL_MATRIX7_ARB 0x88C7
#define GL_MATRIX8_ARB 0x88C8
#define GL_MATRIX9_ARB 0x88C9
#define GL_MATRIX10_ARB 0x88CA
#define GL_MATRIX11_ARB 0x88CB
#define GL_MATRIX12_ARB 0x88CC
#define GL_MATRIX13_ARB 0x88CD
#define GL_MATRIX14_ARB 0x88CE
#define GL_MATRIX15_ARB 0x88CF
#define GL_MATRIX16_ARB 0x88D0
#define GL_MATRIX17_ARB 0x88D1
#define GL_MATRIX18_ARB 0x88D2
#define GL_MATRIX19_ARB 0x88D3
#define GL_MATRIX20_ARB 0x88D4
#define GL_MATRIX21_ARB 0x88D5
#define GL_MATRIX22_ARB 0x88D6
#define GL_MATRIX23_ARB 0x88D7
#define GL_MATRIX24_ARB 0x88D8
#define GL_MATRIX25_ARB 0x88D9
#define GL_MATRIX26_ARB 0x88DA
#define GL_MATRIX27_ARB 0x88DB
#define GL_MATRIX28_ARB 0x88DC
#define GL_MATRIX29_ARB 0x88DD
#define GL_MATRIX30_ARB 0x88DE
#define GL_MATRIX31_ARB 0x88DF
#define glBindProgramARB XGL_FUNCPTR(glBindProgramARB)
#define glDeleteProgramsARB XGL_FUNCPTR(glDeleteProgramsARB)
#define glDisableVertexAttribArrayARB XGL_FUNCPTR(glDisableVertexAttribArrayARB)
#define glEnableVertexAttribArrayARB XGL_FUNCPTR(glEnableVertexAttribArrayARB)
#define glGenProgramsARB XGL_FUNCPTR(glGenProgramsARB)
#define glGetProgramEnvParameterdvARB XGL_FUNCPTR(glGetProgramEnvParameterdvARB)
#define glGetProgramEnvParameterfvARB XGL_FUNCPTR(glGetProgramEnvParameterfvARB)
#define glGetProgramLocalParameterdvARB XGL_FUNCPTR(glGetProgramLocalParameterdvARB)
#define glGetProgramLocalParameterfvARB XGL_FUNCPTR(glGetProgramLocalParameterfvARB)
#define glGetProgramStringARB XGL_FUNCPTR(glGetProgramStringARB)
#define glGetProgramivARB XGL_FUNCPTR(glGetProgramivARB)
#define glGetVertexAttribPointervARB XGL_FUNCPTR(glGetVertexAttribPointervARB)
#define glGetVertexAttribdvARB XGL_FUNCPTR(glGetVertexAttribdvARB)
#define glGetVertexAttribfvARB XGL_FUNCPTR(glGetVertexAttribfvARB)
#define glGetVertexAttribivARB XGL_FUNCPTR(glGetVertexAttribivARB)
#define glIsProgramARB XGL_FUNCPTR(glIsProgramARB)
#define glProgramEnvParameter4dARB XGL_FUNCPTR(glProgramEnvParameter4dARB)
#define glProgramEnvParameter4dvARB XGL_FUNCPTR(glProgramEnvParameter4dvARB)
#define glProgramEnvParameter4fARB XGL_FUNCPTR(glProgramEnvParameter4fARB)
#define glProgramEnvParameter4fvARB XGL_FUNCPTR(glProgramEnvParameter4fvARB)
#define glProgramLocalParameter4dARB XGL_FUNCPTR(glProgramLocalParameter4dARB)
#define glProgramLocalParameter4dvARB XGL_FUNCPTR(glProgramLocalParameter4dvARB)
#define glProgramLocalParameter4fARB XGL_FUNCPTR(glProgramLocalParameter4fARB)
#define glProgramLocalParameter4fvARB XGL_FUNCPTR(glProgramLocalParameter4fvARB)
#define glProgramStringARB XGL_FUNCPTR(glProgramStringARB)
#define glVertexAttrib1dARB XGL_FUNCPTR(glVertexAttrib1dARB)
#define glVertexAttrib1dvARB XGL_FUNCPTR(glVertexAttrib1dvARB)
#define glVertexAttrib1fARB XGL_FUNCPTR(glVertexAttrib1fARB)
#define glVertexAttrib1fvARB XGL_FUNCPTR(glVertexAttrib1fvARB)
#define glVertexAttrib1sARB XGL_FUNCPTR(glVertexAttrib1sARB)
#define glVertexAttrib1svARB XGL_FUNCPTR(glVertexAttrib1svARB)
#define glVertexAttrib2dARB XGL_FUNCPTR(glVertexAttrib2dARB)
#define glVertexAttrib2dvARB XGL_FUNCPTR(glVertexAttrib2dvARB)
#define glVertexAttrib2fARB XGL_FUNCPTR(glVertexAttrib2fARB)
#define glVertexAttrib2fvARB XGL_FUNCPTR(glVertexAttrib2fvARB)
#define glVertexAttrib2sARB XGL_FUNCPTR(glVertexAttrib2sARB)
#define glVertexAttrib2svARB XGL_FUNCPTR(glVertexAttrib2svARB)
#define glVertexAttrib3dARB XGL_FUNCPTR(glVertexAttrib3dARB)
#define glVertexAttrib3dvARB XGL_FUNCPTR(glVertexAttrib3dvARB)
#define glVertexAttrib3fARB XGL_FUNCPTR(glVertexAttrib3fARB)
#define glVertexAttrib3fvARB XGL_FUNCPTR(glVertexAttrib3fvARB)
#define glVertexAttrib3sARB XGL_FUNCPTR(glVertexAttrib3sARB)
#define glVertexAttrib3svARB XGL_FUNCPTR(glVertexAttrib3svARB)
#define glVertexAttrib4NbvARB XGL_FUNCPTR(glVertexAttrib4NbvARB)
#define glVertexAttrib4NivARB XGL_FUNCPTR(glVertexAttrib4NivARB)
#define glVertexAttrib4NsvARB XGL_FUNCPTR(glVertexAttrib4NsvARB)
#define glVertexAttrib4NubARB XGL_FUNCPTR(glVertexAttrib4NubARB)
#define glVertexAttrib4NubvARB XGL_FUNCPTR(glVertexAttrib4NubvARB)
#define glVertexAttrib4NuivARB XGL_FUNCPTR(glVertexAttrib4NuivARB)
#define glVertexAttrib4NusvARB XGL_FUNCPTR(glVertexAttrib4NusvARB)
#define glVertexAttrib4bvARB XGL_FUNCPTR(glVertexAttrib4bvARB)
#define glVertexAttrib4dARB XGL_FUNCPTR(glVertexAttrib4dARB)
#define glVertexAttrib4dvARB XGL_FUNCPTR(glVertexAttrib4dvARB)
#define glVertexAttrib4fARB XGL_FUNCPTR(glVertexAttrib4fARB)
#define glVertexAttrib4fvARB XGL_FUNCPTR(glVertexAttrib4fvARB)
#define glVertexAttrib4ivARB XGL_FUNCPTR(glVertexAttrib4ivARB)
#define glVertexAttrib4sARB XGL_FUNCPTR(glVertexAttrib4sARB)
#define glVertexAttrib4svARB XGL_FUNCPTR(glVertexAttrib4svARB)
#define glVertexAttrib4ubvARB XGL_FUNCPTR(glVertexAttrib4ubvARB)
#define glVertexAttrib4uivARB XGL_FUNCPTR(glVertexAttrib4uivARB)
#define glVertexAttrib4usvARB XGL_FUNCPTR(glVertexAttrib4usvARB)
#define glVertexAttribPointerARB XGL_FUNCPTR(glVertexAttribPointerARB)
#endif

#ifdef GL_ARB_vertex_shader
#define GL_VERTEX_SHADER_ARB 0x8B31
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB 0x8B4A
#define GL_MAX_VARYING_FLOATS_ARB 0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB 0x8B4D
#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB 0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB 0x8B8A
#define glBindAttribLocationARB XGL_FUNCPTR(glBindAttribLocationARB)
#define glGetActiveAttribARB XGL_FUNCPTR(glGetActiveAttribARB)
#define glGetAttribLocationARB XGL_FUNCPTR(glGetAttribLocationARB)
#endif

#ifdef GL_ARB_window_pos
#define glWindowPos2dARB XGL_FUNCPTR(glWindowPos2dARB)
#define glWindowPos2dvARB XGL_FUNCPTR(glWindowPos2dvARB)
#define glWindowPos2fARB XGL_FUNCPTR(glWindowPos2fARB)
#define glWindowPos2fvARB XGL_FUNCPTR(glWindowPos2fvARB)
#define glWindowPos2iARB XGL_FUNCPTR(glWindowPos2iARB)
#define glWindowPos2ivARB XGL_FUNCPTR(glWindowPos2ivARB)
#define glWindowPos2sARB XGL_FUNCPTR(glWindowPos2sARB)
#define glWindowPos2svARB XGL_FUNCPTR(glWindowPos2svARB)
#define glWindowPos3dARB XGL_FUNCPTR(glWindowPos3dARB)
#define glWindowPos3dvARB XGL_FUNCPTR(glWindowPos3dvARB)
#define glWindowPos3fARB XGL_FUNCPTR(glWindowPos3fARB)
#define glWindowPos3fvARB XGL_FUNCPTR(glWindowPos3fvARB)
#define glWindowPos3iARB XGL_FUNCPTR(glWindowPos3iARB)
#define glWindowPos3ivARB XGL_FUNCPTR(glWindowPos3ivARB)
#define glWindowPos3sARB XGL_FUNCPTR(glWindowPos3sARB)
#define glWindowPos3svARB XGL_FUNCPTR(glWindowPos3svARB)
#endif

#ifdef GL_ATIX_point_sprites
#define GL_TEXTURE_POINT_MODE_ATIX 0x60b0
#define GL_TEXTURE_POINT_ONE_COORD_ATIX 0x60b1
#define GL_TEXTURE_POINT_SPRITE_ATIX 0x60b2
#define GL_POINT_SPRITE_CULL_MODE_ATIX 0x60b3
#define GL_POINT_SPRITE_CULL_CENTER_ATIX 0x60b4
#define GL_POINT_SPRITE_CULL_CLIP_ATIX 0x60b5
#endif

#ifdef GL_ATIX_texture_env_combine3
#define GL_MODULATE_ADD_ATIX 0x8744
#define GL_MODULATE_SIGNED_ADD_ATIX 0x8745
#define GL_MODULATE_SUBTRACT_ATIX 0x8746
#endif

#ifdef GL_ATIX_texture_env_route
#define GL_SECONDARY_COLOR_ATIX 0x8747
#define GL_TEXTURE_OUTPUT_RGB_ATIX 0x8748
#define GL_TEXTURE_OUTPUT_ALPHA_ATIX 0x8749
#endif

#ifdef GL_ATIX_vertex_shader_output_point_size
#define GL_OUTPUT_POINT_SIZE_ATIX 0x610E
#endif

#ifdef GL_ATI_draw_buffers
#define GL_MAX_DRAW_BUFFERS_ATI 0x8824
#define GL_DRAW_BUFFER0_ATI 0x8825
#define GL_DRAW_BUFFER1_ATI 0x8826
#define GL_DRAW_BUFFER2_ATI 0x8827
#define GL_DRAW_BUFFER3_ATI 0x8828
#define GL_DRAW_BUFFER4_ATI 0x8829
#define GL_DRAW_BUFFER5_ATI 0x882A
#define GL_DRAW_BUFFER6_ATI 0x882B
#define GL_DRAW_BUFFER7_ATI 0x882C
#define GL_DRAW_BUFFER8_ATI 0x882D
#define GL_DRAW_BUFFER9_ATI 0x882E
#define GL_DRAW_BUFFER10_ATI 0x882F
#define GL_DRAW_BUFFER11_ATI 0x8830
#define GL_DRAW_BUFFER12_ATI 0x8831
#define GL_DRAW_BUFFER13_ATI 0x8832
#define GL_DRAW_BUFFER14_ATI 0x8833
#define GL_DRAW_BUFFER15_ATI 0x8834
#define glDrawBuffersATI XGL_FUNCPTR(glDrawBuffersATI)
#endif

#ifdef GL_ATI_element_array
#define GL_ELEMENT_ARRAY_ATI 0x8768
#define GL_ELEMENT_ARRAY_TYPE_ATI 0x8769
#define GL_ELEMENT_ARRAY_POINTER_ATI 0x876A
#define glDrawElementArrayATI XGL_FUNCPTR(glDrawElementArrayATI)
#define glDrawRangeElementArrayATI XGL_FUNCPTR(glDrawRangeElementArrayATI)
#define glElementPointerATI XGL_FUNCPTR(glElementPointerATI)
#endif

#ifdef GL_ATI_envmap_bumpmap
#define GL_BUMP_ROT_MATRIX_ATI 0x8775
#define GL_BUMP_ROT_MATRIX_SIZE_ATI 0x8776
#define GL_BUMP_NUM_TEX_UNITS_ATI 0x8777
#define GL_BUMP_TEX_UNITS_ATI 0x8778
#define GL_DUDV_ATI 0x8779
#define GL_DU8DV8_ATI 0x877A
#define GL_BUMP_ENVMAP_ATI 0x877B
#define GL_BUMP_TARGET_ATI 0x877C
#define glTexBumpParameterivATI XGL_FUNCPTR(glTexBumpParameterivATI)
#define glTexBumpParameterfvATI XGL_FUNCPTR(glTexBumpParameterfvATI)
#define glGetTexBumpParameterivATI XGL_FUNCPTR(glGetTexBumpParameterivATI)
#define glGetTexBumpParameterfvATI XGL_FUNCPTR(glGetTexBumpParameterfvATI)
#endif

#ifdef GL_ATI_fragment_shader
#define GL_RED_BIT_ATI 0x00000001
#define GL_2X_BIT_ATI 0x00000001
#define GL_4X_BIT_ATI 0x00000002
#define GL_GREEN_BIT_ATI 0x00000002
#define GL_COMP_BIT_ATI 0x00000002
#define GL_BLUE_BIT_ATI 0x00000004
#define GL_8X_BIT_ATI 0x00000004
#define GL_NEGATE_BIT_ATI 0x00000004
#define GL_BIAS_BIT_ATI 0x00000008
#define GL_HALF_BIT_ATI 0x00000008
#define GL_QUARTER_BIT_ATI 0x00000010
#define GL_EIGHTH_BIT_ATI 0x00000020
#define GL_SATURATE_BIT_ATI 0x00000040
#define GL_FRAGMENT_SHADER_ATI 0x8920
#define GL_REG_0_ATI 0x8921
#define GL_REG_1_ATI 0x8922
#define GL_REG_2_ATI 0x8923
#define GL_REG_3_ATI 0x8924
#define GL_REG_4_ATI 0x8925
#define GL_REG_5_ATI 0x8926
#define GL_CON_0_ATI 0x8941
#define GL_CON_1_ATI 0x8942
#define GL_CON_2_ATI 0x8943
#define GL_CON_3_ATI 0x8944
#define GL_CON_4_ATI 0x8945
#define GL_CON_5_ATI 0x8946
#define GL_CON_6_ATI 0x8947
#define GL_CON_7_ATI 0x8948
#define GL_MOV_ATI 0x8961
#define GL_ADD_ATI 0x8963
#define GL_MUL_ATI 0x8964
#define GL_SUB_ATI 0x8965
#define GL_DOT3_ATI 0x8966
#define GL_DOT4_ATI 0x8967
#define GL_MAD_ATI 0x8968
#define GL_LERP_ATI 0x8969
#define GL_CND_ATI 0x896A
#define GL_CND0_ATI 0x896B
#define GL_DOT2_ADD_ATI 0x896C
#define GL_SECONDARY_INTERPOLATOR_ATI 0x896D
#define GL_SWIZZLE_STR_ATI 0x8976
#define GL_SWIZZLE_STQ_ATI 0x8977
#define GL_SWIZZLE_STR_DR_ATI 0x8978
#define GL_SWIZZLE_STQ_DQ_ATI 0x8979
#define GL_NUM_FRAGMENT_REGISTERS_ATI 0x896E
#define GL_NUM_FRAGMENT_CONSTANTS_ATI 0x896F
#define GL_NUM_PASSES_ATI 0x8970
#define GL_NUM_INSTRUCTIONS_PER_PASS_ATI 0x8971
#define GL_NUM_INSTRUCTIONS_TOTAL_ATI 0x8972
#define GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI 0x8973
#define GL_NUM_LOOPBACK_COMPONENTS_ATI 0x8974
#define GL_COLOR_ALPHA_PAIRING_ATI 0x8975
#define GL_SWIZZLE_STRQ_ATI 0x897A
#define GL_SWIZZLE_STRQ_DQ_ATI 0x897B
#define glAlphaFragmentOp1ATI XGL_FUNCPTR(glAlphaFragmentOp1ATI)
#define glAlphaFragmentOp2ATI XGL_FUNCPTR(glAlphaFragmentOp2ATI)
#define glAlphaFragmentOp3ATI XGL_FUNCPTR(glAlphaFragmentOp3ATI)
#define glBeginFragmentShaderATI XGL_FUNCPTR(glBeginFragmentShaderATI)
#define glBindFragmentShaderATI XGL_FUNCPTR(glBindFragmentShaderATI)
#define glColorFragmentOp1ATI XGL_FUNCPTR(glColorFragmentOp1ATI)
#define glColorFragmentOp2ATI XGL_FUNCPTR(glColorFragmentOp2ATI)
#define glColorFragmentOp3ATI XGL_FUNCPTR(glColorFragmentOp3ATI)
#define glDeleteFragmentShaderATI XGL_FUNCPTR(glDeleteFragmentShaderATI)
#define glEndFragmentShaderATI XGL_FUNCPTR(glEndFragmentShaderATI)
#define glGenFragmentShadersATI XGL_FUNCPTR(glGenFragmentShadersATI)
#define glPassTexCoordATI XGL_FUNCPTR(glPassTexCoordATI)
#define glSampleMapATI XGL_FUNCPTR(glSampleMapATI)
#define glSetFragmentShaderConstantATI XGL_FUNCPTR(glSetFragmentShaderConstantATI)
#endif

#ifdef GL_ATI_map_object_buffer
#define glMapObjectBufferATI XGL_FUNCPTR(glMapObjectBufferATI)
#define glUnmapObjectBufferATI XGL_FUNCPTR(glUnmapObjectBufferATI)
#endif

#ifdef GL_ATI_pn_triangles
#define GL_PN_TRIANGLES_ATI 0x87F0
#define GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI 0x87F1
#define GL_PN_TRIANGLES_POINT_MODE_ATI 0x87F2
#define GL_PN_TRIANGLES_NORMAL_MODE_ATI 0x87F3
#define GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI 0x87F4
#define GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI 0x87F5
#define GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI 0x87F6
#define GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI 0x87F7
#define GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI 0x87F8
#define glPNTrianglesiATI XGL_FUNCPTR(glPNTrianglesiATI)
#define glPNTrianglesfATI XGL_FUNCPTR(glPNTrianglesfATI)
#endif

#ifdef GL_ATI_separate_stencil
#define GL_STENCIL_BACK_FUNC_ATI 0x8800
#define GL_STENCIL_BACK_FAIL_ATI 0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI 0x8803
#define glStencilOpSeparateATI XGL_FUNCPTR(glStencilOpSeparateATI)
#define glStencilFuncSeparateATI XGL_FUNCPTR(glStencilFuncSeparateATI)
#endif

#ifdef GL_ATI_text_fragment_shader
#define GL_TEXT_FRAGMENT_SHADER_ATI 0x8200
#endif

#ifdef GL_ATI_texture_compression_3dc
#define GL_COMPRESSED_RGB_3DC_ATI 0x8837
#endif

#ifdef GL_ATI_texture_env_combine3
#define GL_MODULATE_ADD_ATI 0x8744
#define GL_MODULATE_SIGNED_ADD_ATI 0x8745
#define GL_MODULATE_SUBTRACT_ATI 0x8746
#endif

#ifdef GL_ATI_texture_float
#define GL_RGBA_FLOAT32_ATI 0x8814
#define GL_RGB_FLOAT32_ATI 0x8815
#define GL_ALPHA_FLOAT32_ATI 0x8816
#define GL_INTENSITY_FLOAT32_ATI 0x8817
#define GL_LUMINANCE_FLOAT32_ATI 0x8818
#define GL_LUMINANCE_ALPHA_FLOAT32_ATI 0x8819
#define GL_RGBA_FLOAT16_ATI 0x881A
#define GL_RGB_FLOAT16_ATI 0x881B
#define GL_ALPHA_FLOAT16_ATI 0x881C
#define GL_INTENSITY_FLOAT16_ATI 0x881D
#define GL_LUMINANCE_FLOAT16_ATI 0x881E
#define GL_LUMINANCE_ALPHA_FLOAT16_ATI 0x881F
#endif

#ifdef GL_ATI_texture_mirror_once
#define GL_MIRROR_CLAMP_ATI 0x8742
#define GL_MIRROR_CLAMP_TO_EDGE_ATI 0x8743
#endif

#ifdef GL_ATI_vertex_array_object
#define GL_STATIC_ATI 0x8760
#define GL_DYNAMIC_ATI 0x8761
#define GL_PRESERVE_ATI 0x8762
#define GL_DISCARD_ATI 0x8763
#define GL_OBJECT_BUFFER_SIZE_ATI 0x8764
#define GL_OBJECT_BUFFER_USAGE_ATI 0x8765
#define GL_ARRAY_OBJECT_BUFFER_ATI 0x8766
#define GL_ARRAY_OBJECT_OFFSET_ATI 0x8767
#define glArrayObjectATI XGL_FUNCPTR(glArrayObjectATI)
#define glFreeObjectBufferATI XGL_FUNCPTR(glFreeObjectBufferATI)
#define glGetArrayObjectfvATI XGL_FUNCPTR(glGetArrayObjectfvATI)
#define glGetArrayObjectivATI XGL_FUNCPTR(glGetArrayObjectivATI)
#define glGetObjectBufferfvATI XGL_FUNCPTR(glGetObjectBufferfvATI)
#define glGetObjectBufferivATI XGL_FUNCPTR(glGetObjectBufferivATI)
#define glGetVariantArrayObjectfvATI XGL_FUNCPTR(glGetVariantArrayObjectfvATI)
#define glGetVariantArrayObjectivATI XGL_FUNCPTR(glGetVariantArrayObjectivATI)
#define glIsObjectBufferATI XGL_FUNCPTR(glIsObjectBufferATI)
#define glNewObjectBufferATI XGL_FUNCPTR(glNewObjectBufferATI)
#define glUpdateObjectBufferATI XGL_FUNCPTR(glUpdateObjectBufferATI)
#define glVariantArrayObjectATI XGL_FUNCPTR(glVariantArrayObjectATI)
#endif

#ifdef GL_ATI_vertex_attrib_array_object
#define glGetVertexAttribArrayObjectfvATI XGL_FUNCPTR(glGetVertexAttribArrayObjectfvATI)
#define glGetVertexAttribArrayObjectivATI XGL_FUNCPTR(glGetVertexAttribArrayObjectivATI)
#define glVertexAttribArrayObjectATI XGL_FUNCPTR(glVertexAttribArrayObjectATI)
#endif

#ifdef GL_ATI_vertex_streams
#define GL_MAX_VERTEX_STREAMS_ATI 0x876B
#define GL_VERTEX_SOURCE_ATI 0x876C
#define GL_VERTEX_STREAM0_ATI 0x876D
#define GL_VERTEX_STREAM1_ATI 0x876E
#define GL_VERTEX_STREAM2_ATI 0x876F
#define GL_VERTEX_STREAM3_ATI 0x8770
#define GL_VERTEX_STREAM4_ATI 0x8771
#define GL_VERTEX_STREAM5_ATI 0x8772
#define GL_VERTEX_STREAM6_ATI 0x8773
#define GL_VERTEX_STREAM7_ATI 0x8774
#define glClientActiveVertexStreamATI XGL_FUNCPTR(glClientActiveVertexStreamATI)
#define glVertexBlendEnviATI XGL_FUNCPTR(glVertexBlendEnviATI)
#define glVertexBlendEnvfATI XGL_FUNCPTR(glVertexBlendEnvfATI)
#define glVertexStream2sATI XGL_FUNCPTR(glVertexStream2sATI)
#define glVertexStream2svATI XGL_FUNCPTR(glVertexStream2svATI)
#define glVertexStream2iATI XGL_FUNCPTR(glVertexStream2iATI)
#define glVertexStream2ivATI XGL_FUNCPTR(glVertexStream2ivATI)
#define glVertexStream2fATI XGL_FUNCPTR(glVertexStream2fATI)
#define glVertexStream2fvATI XGL_FUNCPTR(glVertexStream2fvATI)
#define glVertexStream2dATI XGL_FUNCPTR(glVertexStream2dATI)
#define glVertexStream2dvATI XGL_FUNCPTR(glVertexStream2dvATI)
#define glVertexStream3sATI XGL_FUNCPTR(glVertexStream3sATI)
#define glVertexStream3svATI XGL_FUNCPTR(glVertexStream3svATI)
#define glVertexStream3iATI XGL_FUNCPTR(glVertexStream3iATI)
#define glVertexStream3ivATI XGL_FUNCPTR(glVertexStream3ivATI)
#define glVertexStream3fATI XGL_FUNCPTR(glVertexStream3fATI)
#define glVertexStream3fvATI XGL_FUNCPTR(glVertexStream3fvATI)
#define glVertexStream3dATI XGL_FUNCPTR(glVertexStream3dATI)
#define glVertexStream3dvATI XGL_FUNCPTR(glVertexStream3dvATI)
#define glVertexStream4sATI XGL_FUNCPTR(glVertexStream4sATI)
#define glVertexStream4svATI XGL_FUNCPTR(glVertexStream4svATI)
#define glVertexStream4iATI XGL_FUNCPTR(glVertexStream4iATI)
#define glVertexStream4ivATI XGL_FUNCPTR(glVertexStream4ivATI)
#define glVertexStream4fATI XGL_FUNCPTR(glVertexStream4fATI)
#define glVertexStream4fvATI XGL_FUNCPTR(glVertexStream4fvATI)
#define glVertexStream4dATI XGL_FUNCPTR(glVertexStream4dATI)
#define glVertexStream4dvATI XGL_FUNCPTR(glVertexStream4dvATI)
#define glNormalStream3bATI XGL_FUNCPTR(glNormalStream3bATI)
#define glNormalStream3bvATI XGL_FUNCPTR(glNormalStream3bvATI)
#define glNormalStream3sATI XGL_FUNCPTR(glNormalStream3sATI)
#define glNormalStream3svATI XGL_FUNCPTR(glNormalStream3svATI)
#define glNormalStream3iATI XGL_FUNCPTR(glNormalStream3iATI)
#define glNormalStream3ivATI XGL_FUNCPTR(glNormalStream3ivATI)
#define glNormalStream3fATI XGL_FUNCPTR(glNormalStream3fATI)
#define glNormalStream3fvATI XGL_FUNCPTR(glNormalStream3fvATI)
#define glNormalStream3dATI XGL_FUNCPTR(glNormalStream3dATI)
#define glNormalStream3dvATI XGL_FUNCPTR(glNormalStream3dvATI)
#endif

#ifdef GL_EXT_422_pixels
#define GL_422_EXT 0x80CC
#define GL_422_REV_EXT 0x80CD
#define GL_422_AVERAGE_EXT 0x80CE
#define GL_422_REV_AVERAGE_EXT 0x80CF
#endif

#ifdef GL_EXT_Cg_shader
#define GL_CG_VERTEX_SHADER_EXT 0x890E
#define GL_CG_FRAGMENT_SHADER_EXT 0x890F
#endif

#ifdef GL_EXT_abgr
#define GL_ABGR_EXT 0x8000
#endif

#ifdef GL_EXT_bgra
#define GL_BGR_EXT 0x80E0
#define GL_BGRA_EXT 0x80E1
#endif

#ifdef GL_EXT_blend_color
#define GL_CONSTANT_COLOR_EXT 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR_EXT 0x8002
#define GL_CONSTANT_ALPHA_EXT 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA_EXT 0x8004
#define GL_BLEND_COLOR_EXT 0x8005
#define glBlendColorEXT XGL_FUNCPTR(glBlendColorEXT)
#endif

#ifdef GL_EXT_blend_equation_separate
#define GL_BLEND_EQUATION_RGB_EXT 0x8009
#define GL_BLEND_EQUATION_ALPHA_EXT 0x883D
#define glBlendEquationSeparateEXT XGL_FUNCPTR(glBlendEquationSeparateEXT)
#endif

#ifdef GL_EXT_blend_func_separate
#define GL_BLEND_DST_RGB_EXT 0x80C8
#define GL_BLEND_SRC_RGB_EXT 0x80C9
#define GL_BLEND_DST_ALPHA_EXT 0x80CA
#define GL_BLEND_SRC_ALPHA_EXT 0x80CB
#define glBlendFuncSeparateEXT XGL_FUNCPTR(glBlendFuncSeparateEXT)
#endif

#ifdef GL_EXT_blend_logic_op
#endif

#ifdef GL_EXT_blend_minmax
#define GL_FUNC_ADD_EXT 0x8006
#define GL_MIN_EXT 0x8007
#define GL_MAX_EXT 0x8008
#define GL_BLEND_EQUATION_EXT 0x8009
#define glBlendEquationEXT XGL_FUNCPTR(glBlendEquationEXT)
#endif

#ifdef GL_EXT_blend_subtract
#define GL_FUNC_SUBTRACT_EXT 0x800A
#define GL_FUNC_REVERSE_SUBTRACT_EXT 0x800B
#endif

#ifdef GL_EXT_clip_volume_hint
#define GL_CLIP_VOLUME_CLIPPING_HINT_EXT 0x80F0
#endif

#ifdef GL_EXT_cmyka
#define GL_CMYK_EXT 0x800C
#define GL_CMYKA_EXT 0x800D
#define GL_PACK_CMYK_HINT_EXT 0x800E
#define GL_UNPACK_CMYK_HINT_EXT 0x800F
#endif

#ifdef GL_EXT_color_subtable
#define glColorSubTableEXT XGL_FUNCPTR(glColorSubTableEXT)
#define glCopyColorSubTableEXT XGL_FUNCPTR(glCopyColorSubTableEXT)
#endif

#ifdef GL_EXT_compiled_vertex_array
#define glLockArraysEXT XGL_FUNCPTR(glLockArraysEXT)
#define glUnlockArraysEXT XGL_FUNCPTR(glUnlockArraysEXT)
#endif

#ifdef GL_EXT_convolution
#define GL_CONVOLUTION_1D_EXT 0x8010
#define GL_CONVOLUTION_2D_EXT 0x8011
#define GL_SEPARABLE_2D_EXT 0x8012
#define GL_CONVOLUTION_BORDER_MODE_EXT 0x8013
#define GL_CONVOLUTION_FILTER_SCALE_EXT 0x8014
#define GL_CONVOLUTION_FILTER_BIAS_EXT 0x8015
#define GL_REDUCE_EXT 0x8016
#define GL_CONVOLUTION_FORMAT_EXT 0x8017
#define GL_CONVOLUTION_WIDTH_EXT 0x8018
#define GL_CONVOLUTION_HEIGHT_EXT 0x8019
#define GL_MAX_CONVOLUTION_WIDTH_EXT 0x801A
#define GL_MAX_CONVOLUTION_HEIGHT_EXT 0x801B
#define GL_POST_CONVOLUTION_RED_SCALE_EXT 0x801C
#define GL_POST_CONVOLUTION_GREEN_SCALE_EXT 0x801D
#define GL_POST_CONVOLUTION_BLUE_SCALE_EXT 0x801E
#define GL_POST_CONVOLUTION_ALPHA_SCALE_EXT 0x801F
#define GL_POST_CONVOLUTION_RED_BIAS_EXT 0x8020
#define GL_POST_CONVOLUTION_GREEN_BIAS_EXT 0x8021
#define GL_POST_CONVOLUTION_BLUE_BIAS_EXT 0x8022
#define GL_POST_CONVOLUTION_ALPHA_BIAS_EXT 0x8023
#define glConvolutionFilter1DEXT XGL_FUNCPTR(glConvolutionFilter1DEXT)
#define glConvolutionFilter2DEXT XGL_FUNCPTR(glConvolutionFilter2DEXT)
#define glConvolutionParameterfEXT XGL_FUNCPTR(glConvolutionParameterfEXT)
#define glConvolutionParameterfvEXT XGL_FUNCPTR(glConvolutionParameterfvEXT)
#define glConvolutionParameteriEXT XGL_FUNCPTR(glConvolutionParameteriEXT)
#define glConvolutionParameterivEXT XGL_FUNCPTR(glConvolutionParameterivEXT)
#define glCopyConvolutionFilter1DEXT XGL_FUNCPTR(glCopyConvolutionFilter1DEXT)
#define glCopyConvolutionFilter2DEXT XGL_FUNCPTR(glCopyConvolutionFilter2DEXT)
#define glGetConvolutionFilterEXT XGL_FUNCPTR(glGetConvolutionFilterEXT)
#define glGetConvolutionParameterfvEXT XGL_FUNCPTR(glGetConvolutionParameterfvEXT)
#define glGetConvolutionParameterivEXT XGL_FUNCPTR(glGetConvolutionParameterivEXT)
#define glGetSeparableFilterEXT XGL_FUNCPTR(glGetSeparableFilterEXT)
#define glSeparableFilter2DEXT XGL_FUNCPTR(glSeparableFilter2DEXT)
#endif

#ifdef GL_EXT_coordinate_frame
#define GL_TANGENT_ARRAY_EXT 0x8439
#define GL_BINORMAL_ARRAY_EXT 0x843A
#define GL_CURRENT_TANGENT_EXT 0x843B
#define GL_CURRENT_BINORMAL_EXT 0x843C
#define GL_TANGENT_ARRAY_TYPE_EXT 0x843E
#define GL_TANGENT_ARRAY_STRIDE_EXT 0x843F
#define GL_BINORMAL_ARRAY_TYPE_EXT 0x8440
#define GL_BINORMAL_ARRAY_STRIDE_EXT 0x8441
#define GL_TANGENT_ARRAY_POINTER_EXT 0x8442
#define GL_BINORMAL_ARRAY_POINTER_EXT 0x8443
#define GL_MAP1_TANGENT_EXT 0x8444
#define GL_MAP2_TANGENT_EXT 0x8445
#define GL_MAP1_BINORMAL_EXT 0x8446
#define GL_MAP2_BINORMAL_EXT 0x8447
#define glBinormalPointerEXT XGL_FUNCPTR(glBinormalPointerEXT)
#define glTangentPointerEXT XGL_FUNCPTR(glTangentPointerEXT)
#endif

#ifdef GL_EXT_copy_texture
#define glCopyTexImage1DEXT XGL_FUNCPTR(glCopyTexImage1DEXT)
#define glCopyTexImage2DEXT XGL_FUNCPTR(glCopyTexImage2DEXT)
#define glCopyTexSubImage1DEXT XGL_FUNCPTR(glCopyTexSubImage1DEXT)
#define glCopyTexSubImage2DEXT XGL_FUNCPTR(glCopyTexSubImage2DEXT)
#define glCopyTexSubImage3DEXT XGL_FUNCPTR(glCopyTexSubImage3DEXT)
#endif

#ifdef GL_EXT_cull_vertex
#define glCullParameterdvEXT XGL_FUNCPTR(glCullParameterdvEXT)
#define glCullParameterfvEXT XGL_FUNCPTR(glCullParameterfvEXT)
#endif

#ifdef GL_EXT_depth_bounds_test
#define GL_DEPTH_BOUNDS_TEST_EXT 0x8890
#define GL_DEPTH_BOUNDS_EXT 0x8891
#define glDepthBoundsEXT XGL_FUNCPTR(glDepthBoundsEXT)
#endif

#ifdef GL_EXT_draw_range_elements
#define GL_MAX_ELEMENTS_VERTICES 0x80E8
#define GL_MAX_ELEMENTS_INDICES 0x80E9
#define glDrawRangeElementsEXT XGL_FUNCPTR(glDrawRangeElementsEXT)
#endif

#ifdef GL_EXT_fog_coord
#define GL_FOG_COORDINATE_SOURCE_EXT 0x8450
#define GL_FOG_COORDINATE_EXT 0x8451
#define GL_FRAGMENT_DEPTH_EXT 0x8452
#define GL_CURRENT_FOG_COORDINATE_EXT 0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE_EXT 0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE_EXT 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER_EXT 0x8456
#define GL_FOG_COORDINATE_ARRAY_EXT 0x8457
#define glFogCoordfEXT XGL_FUNCPTR(glFogCoordfEXT)
#define glFogCoordfvEXT XGL_FUNCPTR(glFogCoordfvEXT)
#define glFogCoorddEXT XGL_FUNCPTR(glFogCoorddEXT)
#define glFogCoorddvEXT XGL_FUNCPTR(glFogCoorddvEXT)
#define glFogCoordPointerEXT XGL_FUNCPTR(glFogCoordPointerEXT)
#endif

#ifdef GL_EXT_fragment_lighting
#define GL_FRAGMENT_LIGHTING_EXT 0x8400
#define GL_FRAGMENT_COLOR_MATERIAL_EXT 0x8401
#define GL_FRAGMENT_COLOR_MATERIAL_FACE_EXT 0x8402
#define GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_EXT 0x8403
#define GL_MAX_FRAGMENT_LIGHTS_EXT 0x8404
#define GL_MAX_ACTIVE_LIGHTS_EXT 0x8405
#define GL_CURRENT_RASTER_NORMAL_EXT 0x8406
#define GL_LIGHT_ENV_MODE_EXT 0x8407
#define GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_EXT 0x8408
#define GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_EXT 0x8409
#define GL_FRAGMENT_LIGHT_MODEL_AMBIENT_EXT 0x840A
#define GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_EXT 0x840B
#define GL_FRAGMENT_LIGHT0_EXT 0x840C
#define GL_FRAGMENT_LIGHT7_EXT 0x8413
#define glFragmentColorMaterialEXT XGL_FUNCPTR(glFragmentColorMaterialEXT)
#define glFragmentLightModelfEXT XGL_FUNCPTR(glFragmentLightModelfEXT)
#define glFragmentLightModelfvEXT XGL_FUNCPTR(glFragmentLightModelfvEXT)
#define glFragmentLightModeliEXT XGL_FUNCPTR(glFragmentLightModeliEXT)
#define glFragmentLightModelivEXT XGL_FUNCPTR(glFragmentLightModelivEXT)
#define glFragmentLightfEXT XGL_FUNCPTR(glFragmentLightfEXT)
#define glFragmentLightfvEXT XGL_FUNCPTR(glFragmentLightfvEXT)
#define glFragmentLightiEXT XGL_FUNCPTR(glFragmentLightiEXT)
#define glFragmentLightivEXT XGL_FUNCPTR(glFragmentLightivEXT)
#define glFragmentMaterialfEXT XGL_FUNCPTR(glFragmentMaterialfEXT)
#define glFragmentMaterialfvEXT XGL_FUNCPTR(glFragmentMaterialfvEXT)
#define glFragmentMaterialiEXT XGL_FUNCPTR(glFragmentMaterialiEXT)
#define glFragmentMaterialivEXT XGL_FUNCPTR(glFragmentMaterialivEXT)
#define glGetFragmentLightfvEXT XGL_FUNCPTR(glGetFragmentLightfvEXT)
#define glGetFragmentLightivEXT XGL_FUNCPTR(glGetFragmentLightivEXT)
#define glGetFragmentMaterialfvEXT XGL_FUNCPTR(glGetFragmentMaterialfvEXT)
#define glGetFragmentMaterialivEXT XGL_FUNCPTR(glGetFragmentMaterialivEXT)
#define glLightEnviEXT XGL_FUNCPTR(glLightEnviEXT)
#endif

#ifdef GL_EXT_framebuffer_blit
#define GL_READ_FRAMEBUFFER_EXT 0x8CA8
#define GL_DRAW_FRAMEBUFFER_EXT 0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING_EXT 0x8CA6
#define GL_READ_FRAMEBUFFER_BINDING_EXT 0x8CAA
#define glBlitFramebufferEXT XGL_FUNCPTR(glBlitFramebufferEXT)
#endif

#ifdef GL_EXT_framebuffer_object
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT 0x0506
#define GL_MAX_RENDERBUFFER_SIZE_EXT 0x84E8
#define GL_FRAMEBUFFER_BINDING_EXT 0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT 0x8CA7
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT 0x8CD4
#define GL_FRAMEBUFFER_COMPLETE_EXT 0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT 0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT 0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT 0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT 0x8CDD
#define GL_FRAMEBUFFER_STATUS_ERROR_EXT 0x8CDE
#define GL_MAX_COLOR_ATTACHMENTS_EXT 0x8CDF
#define GL_COLOR_ATTACHMENT0_EXT 0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT 0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT 0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT 0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT 0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT 0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT 0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT 0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT 0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT 0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT 0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT 0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT 0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT 0x8CED
#define GL_COLOR_ATTACHMENT14_EXT 0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT 0x8CEF
#define GL_DEPTH_ATTACHMENT_EXT 0x8D00
#define GL_STENCIL_ATTACHMENT_EXT 0x8D20
#define GL_FRAMEBUFFER_EXT 0x8D40
#define GL_RENDERBUFFER_EXT 0x8D41
#define GL_RENDERBUFFER_WIDTH_EXT 0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT 0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT 0x8D44
#define GL_STENCIL_INDEX_EXT 0x8D45
#define GL_STENCIL_INDEX1_EXT 0x8D46
#define GL_STENCIL_INDEX4_EXT 0x8D47
#define GL_STENCIL_INDEX8_EXT 0x8D48
#define GL_STENCIL_INDEX16_EXT 0x8D49
#define glBindFramebufferEXT XGL_FUNCPTR(glBindFramebufferEXT)
#define glBindRenderbufferEXT XGL_FUNCPTR(glBindRenderbufferEXT)
#define glCheckFramebufferStatusEXT XGL_FUNCPTR(glCheckFramebufferStatusEXT)
#define glDeleteFramebuffersEXT XGL_FUNCPTR(glDeleteFramebuffersEXT)
#define glDeleteRenderbuffersEXT XGL_FUNCPTR(glDeleteRenderbuffersEXT)
#define glFramebufferRenderbufferEXT XGL_FUNCPTR(glFramebufferRenderbufferEXT)
#define glFramebufferTexture1DEXT XGL_FUNCPTR(glFramebufferTexture1DEXT)
#define glFramebufferTexture2DEXT XGL_FUNCPTR(glFramebufferTexture2DEXT)
#define glFramebufferTexture3DEXT XGL_FUNCPTR(glFramebufferTexture3DEXT)
#define glGenFramebuffersEXT XGL_FUNCPTR(glGenFramebuffersEXT)
#define glGenRenderbuffersEXT XGL_FUNCPTR(glGenRenderbuffersEXT)
#define glGenerateMipmapEXT XGL_FUNCPTR(glGenerateMipmapEXT)
#define glGetFramebufferAttachmentParameterivEXT XGL_FUNCPTR(glGetFramebufferAttachmentParameterivEXT)
#define glGetRenderbufferParameterivEXT XGL_FUNCPTR(glGetRenderbufferParameterivEXT)
#define glIsFramebufferEXT XGL_FUNCPTR(glIsFramebufferEXT)
#define glIsRenderbufferEXT XGL_FUNCPTR(glIsRenderbufferEXT)
#define glRenderbufferStorageEXT XGL_FUNCPTR(glRenderbufferStorageEXT)
#endif

#ifdef GL_EXT_histogram
#define GL_HISTOGRAM_EXT 0x8024
#define GL_PROXY_HISTOGRAM_EXT 0x8025
#define GL_HISTOGRAM_WIDTH_EXT 0x8026
#define GL_HISTOGRAM_FORMAT_EXT 0x8027
#define GL_HISTOGRAM_RED_SIZE_EXT 0x8028
#define GL_HISTOGRAM_GREEN_SIZE_EXT 0x8029
#define GL_HISTOGRAM_BLUE_SIZE_EXT 0x802A
#define GL_HISTOGRAM_ALPHA_SIZE_EXT 0x802B
#define GL_HISTOGRAM_LUMINANCE_SIZE_EXT 0x802C
#define GL_HISTOGRAM_SINK_EXT 0x802D
#define GL_MINMAX_EXT 0x802E
#define GL_MINMAX_FORMAT_EXT 0x802F
#define GL_MINMAX_SINK_EXT 0x8030
#define glGetHistogramEXT XGL_FUNCPTR(glGetHistogramEXT)
#define glGetHistogramParameterfvEXT XGL_FUNCPTR(glGetHistogramParameterfvEXT)
#define glGetHistogramParameterivEXT XGL_FUNCPTR(glGetHistogramParameterivEXT)
#define glGetMinmaxEXT XGL_FUNCPTR(glGetMinmaxEXT)
#define glGetMinmaxParameterfvEXT XGL_FUNCPTR(glGetMinmaxParameterfvEXT)
#define glGetMinmaxParameterivEXT XGL_FUNCPTR(glGetMinmaxParameterivEXT)
#define glHistogramEXT XGL_FUNCPTR(glHistogramEXT)
#define glMinmaxEXT XGL_FUNCPTR(glMinmaxEXT)
#define glResetHistogramEXT XGL_FUNCPTR(glResetHistogramEXT)
#define glResetMinmaxEXT XGL_FUNCPTR(glResetMinmaxEXT)
#endif

#ifdef GL_EXT_index_array_formats
#endif

#ifdef GL_EXT_index_func
#define glIndexFuncEXT XGL_FUNCPTR(glIndexFuncEXT)
#endif

#ifdef GL_EXT_index_material
#define glIndexMaterialEXT XGL_FUNCPTR(glIndexMaterialEXT)
#endif

#ifdef GL_EXT_index_texture
#endif

#ifdef GL_EXT_light_texture
#define GL_FRAGMENT_MATERIAL_EXT 0x8349
#define GL_FRAGMENT_NORMAL_EXT 0x834A
#define GL_FRAGMENT_COLOR_EXT 0x834C
#define GL_ATTENUATION_EXT 0x834D
#define GL_SHADOW_ATTENUATION_EXT 0x834E
#define GL_TEXTURE_APPLICATION_MODE_EXT 0x834F
#define GL_TEXTURE_LIGHT_EXT 0x8350
#define GL_TEXTURE_MATERIAL_FACE_EXT 0x8351
#define GL_TEXTURE_MATERIAL_PARAMETER_EXT 0x8352
#define GL_FRAGMENT_DEPTH_EXT 0x8452
#define glApplyTextureEXT XGL_FUNCPTR(glApplyTextureEXT)
#define glTextureLightEXT XGL_FUNCPTR(glTextureLightEXT)
#define glTextureMaterialEXT XGL_FUNCPTR(glTextureMaterialEXT)
#endif

#ifdef GL_EXT_misc_attribute
#endif

#ifdef GL_EXT_multi_draw_arrays
#define glMultiDrawArraysEXT XGL_FUNCPTR(glMultiDrawArraysEXT)
#define glMultiDrawElementsEXT XGL_FUNCPTR(glMultiDrawElementsEXT)
#endif

#ifdef GL_EXT_multisample
#define GL_MULTISAMPLE_EXT 0x809D
#define GL_SAMPLE_ALPHA_TO_MASK_EXT 0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_EXT 0x809F
#define GL_SAMPLE_MASK_EXT 0x80A0
#define GL_1PASS_EXT 0x80A1
#define GL_2PASS_0_EXT 0x80A2
#define GL_2PASS_1_EXT 0x80A3
#define GL_4PASS_0_EXT 0x80A4
#define GL_4PASS_1_EXT 0x80A5
#define GL_4PASS_2_EXT 0x80A6
#define GL_4PASS_3_EXT 0x80A7
#define GL_SAMPLE_BUFFERS_EXT 0x80A8
#define GL_SAMPLES_EXT 0x80A9
#define GL_SAMPLE_MASK_VALUE_EXT 0x80AA
#define GL_SAMPLE_MASK_INVERT_EXT 0x80AB
#define GL_SAMPLE_PATTERN_EXT 0x80AC
#define GL_MULTISAMPLE_BIT_EXT 0x20000000
#define glSampleMaskEXT XGL_FUNCPTR(glSampleMaskEXT)
#define glSamplePatternEXT XGL_FUNCPTR(glSamplePatternEXT)
#endif

#ifdef GL_EXT_packed_pixels
#define GL_UNSIGNED_BYTE_3_3_2_EXT 0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4_EXT 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT 0x8034
#define GL_UNSIGNED_INT_8_8_8_8_EXT 0x8035
#define GL_UNSIGNED_INT_10_10_10_2_EXT 0x8036
#endif

#ifdef GL_EXT_paletted_texture
#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1
#define GL_PROXY_TEXTURE_1D 0x8063
#define GL_PROXY_TEXTURE_2D 0x8064
#define GL_TEXTURE_3D_EXT 0x806F
#define GL_PROXY_TEXTURE_3D_EXT 0x8070
#define GL_COLOR_TABLE_FORMAT_EXT 0x80D8
#define GL_COLOR_TABLE_WIDTH_EXT 0x80D9
#define GL_COLOR_TABLE_RED_SIZE_EXT 0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE_EXT 0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE_EXT 0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE_EXT 0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE_EXT 0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE_EXT 0x80DF
#define GL_COLOR_INDEX1_EXT 0x80E2
#define GL_COLOR_INDEX2_EXT 0x80E3
#define GL_COLOR_INDEX4_EXT 0x80E4
#define GL_COLOR_INDEX8_EXT 0x80E5
#define GL_COLOR_INDEX12_EXT 0x80E6
#define GL_COLOR_INDEX16_EXT 0x80E7
#define GL_TEXTURE_INDEX_SIZE_EXT 0x80ED
#define GL_TEXTURE_CUBE_MAP_ARB 0x8513
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB 0x851B
#define glColorTableEXT XGL_FUNCPTR(glColorTableEXT)
#define glGetColorTableEXT XGL_FUNCPTR(glGetColorTableEXT)
#define glGetColorTableParameterfvEXT XGL_FUNCPTR(glGetColorTableParameterfvEXT)
#define glGetColorTableParameterivEXT XGL_FUNCPTR(glGetColorTableParameterivEXT)
#endif

#ifdef GL_EXT_pixel_buffer_object
#define GL_PIXEL_PACK_BUFFER_EXT 0x88EB
#define GL_PIXEL_UNPACK_BUFFER_EXT 0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING_EXT 0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING_EXT 0x88EF
#endif

#ifdef GL_EXT_pixel_transform
#define GL_PIXEL_TRANSFORM_2D_EXT 0x8330
#define GL_PIXEL_MAG_FILTER_EXT 0x8331
#define GL_PIXEL_MIN_FILTER_EXT 0x8332
#define GL_PIXEL_CUBIC_WEIGHT_EXT 0x8333
#define GL_CUBIC_EXT 0x8334
#define GL_AVERAGE_EXT 0x8335
#define GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT 0x8336
#define GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT 0x8337
#define GL_PIXEL_TRANSFORM_2D_MATRIX_EXT 0x8338
#define glGetPixelTransformParameterfvEXT XGL_FUNCPTR(glGetPixelTransformParameterfvEXT)
#define glGetPixelTransformParameterivEXT XGL_FUNCPTR(glGetPixelTransformParameterivEXT)
#define glPixelTransformParameterfEXT XGL_FUNCPTR(glPixelTransformParameterfEXT)
#define glPixelTransformParameterfvEXT XGL_FUNCPTR(glPixelTransformParameterfvEXT)
#define glPixelTransformParameteriEXT XGL_FUNCPTR(glPixelTransformParameteriEXT)
#define glPixelTransformParameterivEXT XGL_FUNCPTR(glPixelTransformParameterivEXT)
#endif

#ifdef GL_EXT_pixel_transform_color_table
#endif

#ifdef GL_EXT_point_parameters
#define GL_POINT_SIZE_MIN_EXT 0x8126
#define GL_POINT_SIZE_MAX_EXT 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE_EXT 0x8128
#define GL_DISTANCE_ATTENUATION_EXT 0x8129
#define glPointParameterfEXT XGL_FUNCPTR(glPointParameterfEXT)
#define glPointParameterfvEXT XGL_FUNCPTR(glPointParameterfvEXT)
#endif

#ifdef GL_EXT_polygon_offset
#define GL_POLYGON_OFFSET_EXT 0x8037
#define GL_POLYGON_OFFSET_FACTOR_EXT 0x8038
#define GL_POLYGON_OFFSET_BIAS_EXT 0x8039
#define glPolygonOffsetEXT XGL_FUNCPTR(glPolygonOffsetEXT)
#endif

#ifdef GL_EXT_rescale_normal
#endif

#ifdef GL_EXT_scene_marker
#define glBeginSceneEXT XGL_FUNCPTR(glBeginSceneEXT)
#define glEndSceneEXT XGL_FUNCPTR(glEndSceneEXT)
#endif

#ifdef GL_EXT_secondary_color
#define GL_COLOR_SUM_EXT 0x8458
#define GL_CURRENT_SECONDARY_COLOR_EXT 0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE_EXT 0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE_EXT 0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER_EXT 0x845D
#define GL_SECONDARY_COLOR_ARRAY_EXT 0x845E
#define glSecondaryColor3bEXT XGL_FUNCPTR(glSecondaryColor3bEXT)
#define glSecondaryColor3bvEXT XGL_FUNCPTR(glSecondaryColor3bvEXT)
#define glSecondaryColor3dEXT XGL_FUNCPTR(glSecondaryColor3dEXT)
#define glSecondaryColor3dvEXT XGL_FUNCPTR(glSecondaryColor3dvEXT)
#define glSecondaryColor3fEXT XGL_FUNCPTR(glSecondaryColor3fEXT)
#define glSecondaryColor3fvEXT XGL_FUNCPTR(glSecondaryColor3fvEXT)
#define glSecondaryColor3iEXT XGL_FUNCPTR(glSecondaryColor3iEXT)
#define glSecondaryColor3ivEXT XGL_FUNCPTR(glSecondaryColor3ivEXT)
#define glSecondaryColor3sEXT XGL_FUNCPTR(glSecondaryColor3sEXT)
#define glSecondaryColor3svEXT XGL_FUNCPTR(glSecondaryColor3svEXT)
#define glSecondaryColor3ubEXT XGL_FUNCPTR(glSecondaryColor3ubEXT)
#define glSecondaryColor3ubvEXT XGL_FUNCPTR(glSecondaryColor3ubvEXT)
#define glSecondaryColor3uiEXT XGL_FUNCPTR(glSecondaryColor3uiEXT)
#define glSecondaryColor3uivEXT XGL_FUNCPTR(glSecondaryColor3uivEXT)
#define glSecondaryColor3usEXT XGL_FUNCPTR(glSecondaryColor3usEXT)
#define glSecondaryColor3usvEXT XGL_FUNCPTR(glSecondaryColor3usvEXT)
#define glSecondaryColorPointerEXT XGL_FUNCPTR(glSecondaryColorPointerEXT)
#endif

#ifdef GL_EXT_separate_specular_color
#define GL_LIGHT_MODEL_COLOR_CONTROL_EXT 0x81F8
#define GL_SINGLE_COLOR_EXT 0x81F9
#define GL_SEPARATE_SPECULAR_COLOR_EXT 0x81FA
#endif

#ifdef GL_EXT_shadow_funcs
#endif

#ifdef GL_EXT_shared_texture_palette
#define GL_SHARED_TEXTURE_PALETTE_EXT 0x81FB
#endif

#ifdef GL_EXT_stencil_two_side
#define GL_STENCIL_TEST_TWO_SIDE_EXT 0x8910
#define GL_ACTIVE_STENCIL_FACE_EXT 0x8911
#define glActiveStencilFaceEXT XGL_FUNCPTR(glActiveStencilFaceEXT)
#endif

#ifdef GL_EXT_stencil_wrap
#define GL_INCR_WRAP_EXT 0x8507
#define GL_DECR_WRAP_EXT 0x8508
#endif

#ifdef GL_EXT_subtexture
#define glTexSubImage1DEXT XGL_FUNCPTR(glTexSubImage1DEXT)
#define glTexSubImage2DEXT XGL_FUNCPTR(glTexSubImage2DEXT)
#define glTexSubImage3DEXT XGL_FUNCPTR(glTexSubImage3DEXT)
#endif

#ifdef GL_EXT_texture
#define GL_ALPHA4_EXT 0x803B
#define GL_ALPHA8_EXT 0x803C
#define GL_ALPHA12_EXT 0x803D
#define GL_ALPHA16_EXT 0x803E
#define GL_LUMINANCE4_EXT 0x803F
#define GL_LUMINANCE8_EXT 0x8040
#define GL_LUMINANCE12_EXT 0x8041
#define GL_LUMINANCE16_EXT 0x8042
#define GL_LUMINANCE4_ALPHA4_EXT 0x8043
#define GL_LUMINANCE6_ALPHA2_EXT 0x8044
#define GL_LUMINANCE8_ALPHA8_EXT 0x8045
#define GL_LUMINANCE12_ALPHA4_EXT 0x8046
#define GL_LUMINANCE12_ALPHA12_EXT 0x8047
#define GL_LUMINANCE16_ALPHA16_EXT 0x8048
#define GL_INTENSITY_EXT 0x8049
#define GL_INTENSITY4_EXT 0x804A
#define GL_INTENSITY8_EXT 0x804B
#define GL_INTENSITY12_EXT 0x804C
#define GL_INTENSITY16_EXT 0x804D
#define GL_RGB2_EXT 0x804E
#define GL_RGB4_EXT 0x804F
#define GL_RGB5_EXT 0x8050
#define GL_RGB8_EXT 0x8051
#define GL_RGB10_EXT 0x8052
#define GL_RGB12_EXT 0x8053
#define GL_RGB16_EXT 0x8054
#define GL_RGBA2_EXT 0x8055
#define GL_RGBA4_EXT 0x8056
#define GL_RGB5_A1_EXT 0x8057
#define GL_RGBA8_EXT 0x8058
#define GL_RGB10_A2_EXT 0x8059
#define GL_RGBA12_EXT 0x805A
#define GL_RGBA16_EXT 0x805B
#define GL_TEXTURE_RED_SIZE_EXT 0x805C
#define GL_TEXTURE_GREEN_SIZE_EXT 0x805D
#define GL_TEXTURE_BLUE_SIZE_EXT 0x805E
#define GL_TEXTURE_ALPHA_SIZE_EXT 0x805F
#define GL_TEXTURE_LUMINANCE_SIZE_EXT 0x8060
#define GL_TEXTURE_INTENSITY_SIZE_EXT 0x8061
#define GL_REPLACE_EXT 0x8062
#define GL_PROXY_TEXTURE_1D_EXT 0x8063
#define GL_PROXY_TEXTURE_2D_EXT 0x8064
#endif

#ifdef GL_EXT_texture3D
#define GL_PACK_SKIP_IMAGES_EXT 0x806B
#define GL_PACK_IMAGE_HEIGHT_EXT 0x806C
#define GL_UNPACK_SKIP_IMAGES_EXT 0x806D
#define GL_UNPACK_IMAGE_HEIGHT_EXT 0x806E
#define GL_TEXTURE_3D_EXT 0x806F
#define GL_PROXY_TEXTURE_3D_EXT 0x8070
#define GL_TEXTURE_DEPTH_EXT 0x8071
#define GL_TEXTURE_WRAP_R_EXT 0x8072
#define GL_MAX_3D_TEXTURE_SIZE_EXT 0x8073
#define glTexImage3DEXT XGL_FUNCPTR(glTexImage3DEXT)
#endif

#ifdef GL_EXT_texture_compression_dxt1
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif

#ifdef GL_EXT_texture_compression_s3tc
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif

#ifdef GL_EXT_texture_cube_map
#define GL_NORMAL_MAP_EXT 0x8511
#define GL_REFLECTION_MAP_EXT 0x8512
#define GL_TEXTURE_CUBE_MAP_EXT 0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT 0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT 0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT 0x851C
#endif

#ifdef GL_EXT_texture_edge_clamp
#define GL_CLAMP_TO_EDGE_EXT	0x812F
#endif

#ifdef GL_EXT_texture_env
#define GL_TEXTURE_ENV0_EXT 0
#define GL_TEXTURE_ENV_SHIFT_EXT 0
#define GL_ENV_BLEND_EXT 0
#define GL_ENV_ADD_EXT 0
#define GL_ENV_REPLACE_EXT 0
#define GL_ENV_SUBTRACT_EXT 0
#define GL_TEXTURE_ENV_MODE_ALPHA_EXT 0
#define GL_ENV_REVERSE_BLEND_EXT 0
#define GL_ENV_REVERSE_SUBTRACT_EXT 0
#define GL_ENV_COPY_EXT 0
#define GL_ENV_MODULATE_EXT 0
#endif

#ifdef GL_EXT_texture_env_add
#endif

#ifdef GL_EXT_texture_env_combine
#define GL_COMBINE_EXT 0x8570
#define GL_COMBINE_RGB_EXT 0x8571
#define GL_COMBINE_ALPHA_EXT 0x8572
#define GL_RGB_SCALE_EXT 0x8573
#define GL_ADD_SIGNED_EXT 0x8574
#define GL_INTERPOLATE_EXT 0x8575
#define GL_CONSTANT_EXT 0x8576
#define GL_PRIMARY_COLOR_EXT 0x8577
#define GL_PREVIOUS_EXT 0x8578
#define GL_SOURCE0_RGB_EXT 0x8580
#define GL_SOURCE1_RGB_EXT 0x8581
#define GL_SOURCE2_RGB_EXT 0x8582
#define GL_SOURCE0_ALPHA_EXT 0x8588
#define GL_SOURCE1_ALPHA_EXT 0x8589
#define GL_SOURCE2_ALPHA_EXT 0x858A
#define GL_OPERAND0_RGB_EXT 0x8590
#define GL_OPERAND1_RGB_EXT 0x8591
#define GL_OPERAND2_RGB_EXT 0x8592
#define GL_OPERAND0_ALPHA_EXT 0x8598
#define GL_OPERAND1_ALPHA_EXT 0x8599
#define GL_OPERAND2_ALPHA_EXT 0x859A
#endif

#ifdef GL_EXT_texture_env_dot3
#define GL_DOT3_RGB_EXT 0x8740
#define GL_DOT3_RGBA_EXT 0x8741
#endif

#ifdef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#ifdef GL_EXT_texture_lod_bias
#define GL_MAX_TEXTURE_LOD_BIAS_EXT 0x84FD
#define GL_TEXTURE_FILTER_CONTROL_EXT 0x8500
#define GL_TEXTURE_LOD_BIAS_EXT 0x8501
#endif

#ifdef GL_EXT_texture_mirror_clamp
#define GL_MIRROR_CLAMP_EXT 0x8742
#define GL_MIRROR_CLAMP_TO_EDGE_EXT 0x8743
#define GL_MIRROR_CLAMP_TO_BORDER_EXT 0x8912
#endif

#ifdef GL_EXT_texture_object
#define GL_TEXTURE_PRIORITY_EXT 0x8066
#define GL_TEXTURE_RESIDENT_EXT 0x8067
#define GL_TEXTURE_1D_BINDING_EXT 0x8068
#define GL_TEXTURE_2D_BINDING_EXT 0x8069
#define GL_TEXTURE_3D_BINDING_EXT 0x806A
#define glAreTexturesResidentEXT XGL_FUNCPTR(glAreTexturesResidentEXT)
#define glBindTextureEXT XGL_FUNCPTR(glBindTextureEXT)
#define glDeleteTexturesEXT XGL_FUNCPTR(glDeleteTexturesEXT)
#define glGenTexturesEXT XGL_FUNCPTR(glGenTexturesEXT)
#define glIsTextureEXT XGL_FUNCPTR(glIsTextureEXT)
#define glPrioritizeTexturesEXT XGL_FUNCPTR(glPrioritizeTexturesEXT)
#endif

#ifdef GL_EXT_texture_perturb_normal
#define GL_PERTURB_EXT 0x85AE
#define GL_TEXTURE_NORMAL_EXT 0x85AF
#define glTextureNormalEXT XGL_FUNCPTR(glTextureNormalEXT)
#endif

#ifdef GL_EXT_texture_rectangle
#define GL_TEXTURE_RECTANGLE_EXT 0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_EXT 0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_EXT 0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT 0x84F8
#endif

#ifdef GL_EXT_vertex_array
#define GL_DOUBLE_EXT 0x140A
#define GL_VERTEX_ARRAY_EXT 0x8074
#define GL_NORMAL_ARRAY_EXT 0x8075
#define GL_COLOR_ARRAY_EXT 0x8076
#define GL_INDEX_ARRAY_EXT 0x8077
#define GL_TEXTURE_COORD_ARRAY_EXT 0x8078
#define GL_EDGE_FLAG_ARRAY_EXT 0x8079
#define GL_VERTEX_ARRAY_SIZE_EXT 0x807A
#define GL_VERTEX_ARRAY_TYPE_EXT 0x807B
#define GL_VERTEX_ARRAY_STRIDE_EXT 0x807C
#define GL_VERTEX_ARRAY_COUNT_EXT 0x807D
#define GL_NORMAL_ARRAY_TYPE_EXT 0x807E
#define GL_NORMAL_ARRAY_STRIDE_EXT 0x807F
#define GL_NORMAL_ARRAY_COUNT_EXT 0x8080
#define GL_COLOR_ARRAY_SIZE_EXT 0x8081
#define GL_COLOR_ARRAY_TYPE_EXT 0x8082
#define GL_COLOR_ARRAY_STRIDE_EXT 0x8083
#define GL_COLOR_ARRAY_COUNT_EXT 0x8084
#define GL_INDEX_ARRAY_TYPE_EXT 0x8085
#define GL_INDEX_ARRAY_STRIDE_EXT 0x8086
#define GL_INDEX_ARRAY_COUNT_EXT 0x8087
#define GL_TEXTURE_COORD_ARRAY_SIZE_EXT 0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE_EXT 0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT 0x808A
#define GL_TEXTURE_COORD_ARRAY_COUNT_EXT 0x808B
#define GL_EDGE_FLAG_ARRAY_STRIDE_EXT 0x808C
#define GL_EDGE_FLAG_ARRAY_COUNT_EXT 0x808D
#define GL_VERTEX_ARRAY_POINTER_EXT 0x808E
#define GL_NORMAL_ARRAY_POINTER_EXT 0x808F
#define GL_COLOR_ARRAY_POINTER_EXT 0x8090
#define GL_INDEX_ARRAY_POINTER_EXT 0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER_EXT 0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER_EXT 0x8093
#define glArrayElementEXT XGL_FUNCPTR(glArrayElementEXT)
#define glColorPointerEXT XGL_FUNCPTR(glColorPointerEXT)
#define glDrawArraysEXT XGL_FUNCPTR(glDrawArraysEXT)
#define glEdgeFlagPointerEXT XGL_FUNCPTR(glEdgeFlagPointerEXT)
#define glGetPointervEXT XGL_FUNCPTR(glGetPointervEXT)
#define glIndexPointerEXT XGL_FUNCPTR(glIndexPointerEXT)
#define glNormalPointerEXT XGL_FUNCPTR(glNormalPointerEXT)
#define glTexCoordPointerEXT XGL_FUNCPTR(glTexCoordPointerEXT)
#define glVertexPointerEXT XGL_FUNCPTR(glVertexPointerEXT)
#endif

#ifdef GL_EXT_vertex_shader
#define GL_VERTEX_SHADER_EXT 0x8780
#define GL_VERTEX_SHADER_BINDING_EXT 0x8781
#define GL_OP_INDEX_EXT 0x8782
#define GL_OP_NEGATE_EXT 0x8783
#define GL_OP_DOT3_EXT 0x8784
#define GL_OP_DOT4_EXT 0x8785
#define GL_OP_MUL_EXT 0x8786
#define GL_OP_ADD_EXT 0x8787
#define GL_OP_MADD_EXT 0x8788
#define GL_OP_FRAC_EXT 0x8789
#define GL_OP_MAX_EXT 0x878A
#define GL_OP_MIN_EXT 0x878B
#define GL_OP_SET_GE_EXT 0x878C
#define GL_OP_SET_LT_EXT 0x878D
#define GL_OP_CLAMP_EXT 0x878E
#define GL_OP_FLOOR_EXT 0x878F
#define GL_OP_ROUND_EXT 0x8790
#define GL_OP_EXP_BASE_2_EXT 0x8791
#define GL_OP_LOG_BASE_2_EXT 0x8792
#define GL_OP_POWER_EXT 0x8793
#define GL_OP_RECIP_EXT 0x8794
#define GL_OP_RECIP_SQRT_EXT 0x8795
#define GL_OP_SUB_EXT 0x8796
#define GL_OP_CROSS_PRODUCT_EXT 0x8797
#define GL_OP_MULTIPLY_MATRIX_EXT 0x8798
#define GL_OP_MOV_EXT 0x8799
#define GL_OUTPUT_VERTEX_EXT 0x879A
#define GL_OUTPUT_COLOR0_EXT 0x879B
#define GL_OUTPUT_COLOR1_EXT 0x879C
#define GL_OUTPUT_TEXTURE_COORD0_EXT 0x879D
#define GL_OUTPUT_TEXTURE_COORD1_EXT 0x879E
#define GL_OUTPUT_TEXTURE_COORD2_EXT 0x879F
#define GL_OUTPUT_TEXTURE_COORD3_EXT 0x87A0
#define GL_OUTPUT_TEXTURE_COORD4_EXT 0x87A1
#define GL_OUTPUT_TEXTURE_COORD5_EXT 0x87A2
#define GL_OUTPUT_TEXTURE_COORD6_EXT 0x87A3
#define GL_OUTPUT_TEXTURE_COORD7_EXT 0x87A4
#define GL_OUTPUT_TEXTURE_COORD8_EXT 0x87A5
#define GL_OUTPUT_TEXTURE_COORD9_EXT 0x87A6
#define GL_OUTPUT_TEXTURE_COORD10_EXT 0x87A7
#define GL_OUTPUT_TEXTURE_COORD11_EXT 0x87A8
#define GL_OUTPUT_TEXTURE_COORD12_EXT 0x87A9
#define GL_OUTPUT_TEXTURE_COORD13_EXT 0x87AA
#define GL_OUTPUT_TEXTURE_COORD14_EXT 0x87AB
#define GL_OUTPUT_TEXTURE_COORD15_EXT 0x87AC
#define GL_OUTPUT_TEXTURE_COORD16_EXT 0x87AD
#define GL_OUTPUT_TEXTURE_COORD17_EXT 0x87AE
#define GL_OUTPUT_TEXTURE_COORD18_EXT 0x87AF
#define GL_OUTPUT_TEXTURE_COORD19_EXT 0x87B0
#define GL_OUTPUT_TEXTURE_COORD20_EXT 0x87B1
#define GL_OUTPUT_TEXTURE_COORD21_EXT 0x87B2
#define GL_OUTPUT_TEXTURE_COORD22_EXT 0x87B3
#define GL_OUTPUT_TEXTURE_COORD23_EXT 0x87B4
#define GL_OUTPUT_TEXTURE_COORD24_EXT 0x87B5
#define GL_OUTPUT_TEXTURE_COORD25_EXT 0x87B6
#define GL_OUTPUT_TEXTURE_COORD26_EXT 0x87B7
#define GL_OUTPUT_TEXTURE_COORD27_EXT 0x87B8
#define GL_OUTPUT_TEXTURE_COORD28_EXT 0x87B9
#define GL_OUTPUT_TEXTURE_COORD29_EXT 0x87BA
#define GL_OUTPUT_TEXTURE_COORD30_EXT 0x87BB
#define GL_OUTPUT_TEXTURE_COORD31_EXT 0x87BC
#define GL_OUTPUT_FOG_EXT 0x87BD
#define GL_SCALAR_EXT 0x87BE
#define GL_VECTOR_EXT 0x87BF
#define GL_MATRIX_EXT 0x87C0
#define GL_VARIANT_EXT 0x87C1
#define GL_INVARIANT_EXT 0x87C2
#define GL_LOCAL_CONSTANT_EXT 0x87C3
#define GL_LOCAL_EXT 0x87C4
#define GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT 0x87C5
#define GL_MAX_VERTEX_SHADER_VARIANTS_EXT 0x87C6
#define GL_MAX_VERTEX_SHADER_INVARIANTS_EXT 0x87C7
#define GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT 0x87C8
#define GL_MAX_VERTEX_SHADER_LOCALS_EXT 0x87C9
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT 0x87CA
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT 0x87CB
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT 0x87CC
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT 0x87CD
#define GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT 0x87CE
#define GL_VERTEX_SHADER_INSTRUCTIONS_EXT 0x87CF
#define GL_VERTEX_SHADER_VARIANTS_EXT 0x87D0
#define GL_VERTEX_SHADER_INVARIANTS_EXT 0x87D1
#define GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT 0x87D2
#define GL_VERTEX_SHADER_LOCALS_EXT 0x87D3
#define GL_VERTEX_SHADER_OPTIMIZED_EXT 0x87D4
#define GL_X_EXT 0x87D5
#define GL_Y_EXT 0x87D6
#define GL_Z_EXT 0x87D7
#define GL_W_EXT 0x87D8
#define GL_NEGATIVE_X_EXT 0x87D9
#define GL_NEGATIVE_Y_EXT 0x87DA
#define GL_NEGATIVE_Z_EXT 0x87DB
#define GL_NEGATIVE_W_EXT 0x87DC
#define GL_ZERO_EXT 0x87DD
#define GL_ONE_EXT 0x87DE
#define GL_NEGATIVE_ONE_EXT 0x87DF
#define GL_NORMALIZED_RANGE_EXT 0x87E0
#define GL_FULL_RANGE_EXT 0x87E1
#define GL_CURRENT_VERTEX_EXT 0x87E2
#define GL_MVP_MATRIX_EXT 0x87E3
#define GL_VARIANT_VALUE_EXT 0x87E4
#define GL_VARIANT_DATATYPE_EXT 0x87E5
#define GL_VARIANT_ARRAY_STRIDE_EXT 0x87E6
#define GL_VARIANT_ARRAY_TYPE_EXT 0x87E7
#define GL_VARIANT_ARRAY_EXT 0x87E8
#define GL_VARIANT_ARRAY_POINTER_EXT 0x87E9
#define GL_INVARIANT_VALUE_EXT 0x87EA
#define GL_INVARIANT_DATATYPE_EXT 0x87EB
#define GL_LOCAL_CONSTANT_VALUE_EXT 0x87EC
#define GL_LOCAL_CONSTANT_DATATYPE_EXT 0x87ED
#define glBeginVertexShaderEXT XGL_FUNCPTR(glBeginVertexShaderEXT)
#define glEndVertexShaderEXT XGL_FUNCPTR(glEndVertexShaderEXT)
#define glBindVertexShaderEXT XGL_FUNCPTR(glBindVertexShaderEXT)
#define glGenVertexShadersEXT XGL_FUNCPTR(glGenVertexShadersEXT)
#define glDeleteVertexShaderEXT XGL_FUNCPTR(glDeleteVertexShaderEXT)
#define glShaderOp1EXT XGL_FUNCPTR(glShaderOp1EXT)
#define glShaderOp2EXT XGL_FUNCPTR(glShaderOp2EXT)
#define glShaderOp3EXT XGL_FUNCPTR(glShaderOp3EXT)
#define glSwizzleEXT XGL_FUNCPTR(glSwizzleEXT)
#define glWriteMaskEXT XGL_FUNCPTR(glWriteMaskEXT)
#define glInsertComponentEXT XGL_FUNCPTR(glInsertComponentEXT)
#define glExtractComponentEXT XGL_FUNCPTR(glExtractComponentEXT)
#define glGenSymbolsEXT XGL_FUNCPTR(glGenSymbolsEXT)
#define glSetInvariantEXT XGL_FUNCPTR(glSetInvariantEXT)
#define glSetLocalConstantEXT XGL_FUNCPTR(glSetLocalConstantEXT)
#define glVariantbvEXT XGL_FUNCPTR(glVariantbvEXT)
#define glVariantsvEXT XGL_FUNCPTR(glVariantsvEXT)
#define glVariantivEXT XGL_FUNCPTR(glVariantivEXT)
#define glVariantfvEXT XGL_FUNCPTR(glVariantfvEXT)
#define glVariantdvEXT XGL_FUNCPTR(glVariantdvEXT)
#define glVariantubvEXT XGL_FUNCPTR(glVariantubvEXT)
#define glVariantusvEXT XGL_FUNCPTR(glVariantusvEXT)
#define glVariantuivEXT XGL_FUNCPTR(glVariantuivEXT)
#define glVariantPointerEXT XGL_FUNCPTR(glVariantPointerEXT)
#define glEnableVariantClientStateEXT XGL_FUNCPTR(glEnableVariantClientStateEXT)
#define glDisableVariantClientStateEXT XGL_FUNCPTR(glDisableVariantClientStateEXT)
#define glBindLightParameterEXT XGL_FUNCPTR(glBindLightParameterEXT)
#define glBindMaterialParameterEXT XGL_FUNCPTR(glBindMaterialParameterEXT)
#define glBindTexGenParameterEXT XGL_FUNCPTR(glBindTexGenParameterEXT)
#define glBindTextureUnitParameterEXT XGL_FUNCPTR(glBindTextureUnitParameterEXT)
#define glBindParameterEXT XGL_FUNCPTR(glBindParameterEXT)
#define glIsVariantEnabledEXT XGL_FUNCPTR(glIsVariantEnabledEXT)
#define glGetVariantBooleanvEXT XGL_FUNCPTR(glGetVariantBooleanvEXT)
#define glGetVariantIntegervEXT XGL_FUNCPTR(glGetVariantIntegervEXT)
#define glGetVariantFloatvEXT XGL_FUNCPTR(glGetVariantFloatvEXT)
#define glGetVariantPointervEXT XGL_FUNCPTR(glGetVariantPointervEXT)
#define glGetInvariantBooleanvEXT XGL_FUNCPTR(glGetInvariantBooleanvEXT)
#define glGetInvariantIntegervEXT XGL_FUNCPTR(glGetInvariantIntegervEXT)
#define glGetInvariantFloatvEXT XGL_FUNCPTR(glGetInvariantFloatvEXT)
#define glGetLocalConstantBooleanvEXT XGL_FUNCPTR(glGetLocalConstantBooleanvEXT)
#define glGetLocalConstantIntegervEXT XGL_FUNCPTR(glGetLocalConstantIntegervEXT)
#define glGetLocalConstantFloatvEXT XGL_FUNCPTR(glGetLocalConstantFloatvEXT)
#endif

#ifdef GL_EXT_vertex_weighting
#define GL_MODELVIEW0_STACK_DEPTH_EXT 0x0BA3
#define GL_MODELVIEW0_MATRIX_EXT 0x0BA6
#define GL_MODELVIEW0_EXT 0x1700
#define GL_MODELVIEW1_STACK_DEPTH_EXT 0x8502
#define GL_MODELVIEW1_MATRIX_EXT 0x8506
#define GL_VERTEX_WEIGHTING_EXT 0x8509
#define GL_MODELVIEW1_EXT 0x850A
#define GL_CURRENT_VERTEX_WEIGHT_EXT 0x850B
#define GL_VERTEX_WEIGHT_ARRAY_EXT 0x850C
#define GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT 0x850D
#define GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT 0x850E
#define GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT 0x850F
#define GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT 0x8510
#define glVertexWeightPointerEXT XGL_FUNCPTR(glVertexWeightPointerEXT)
#define glVertexWeightfEXT XGL_FUNCPTR(glVertexWeightfEXT)
#define glVertexWeightfvEXT XGL_FUNCPTR(glVertexWeightfvEXT)
#endif

#ifdef GL_GREMEDY_string_marker
#define glStringMarkerGREMEDY XGL_FUNCPTR(glStringMarkerGREMEDY)
#endif

#ifdef GL_HP_convolution_border_modes
#endif

#ifdef GL_HP_image_transform
#define glGetImageTransformParameterfvHP XGL_FUNCPTR(glGetImageTransformParameterfvHP)
#define glGetImageTransformParameterivHP XGL_FUNCPTR(glGetImageTransformParameterivHP)
#define glImageTransformParameterfHP XGL_FUNCPTR(glImageTransformParameterfHP)
#define glImageTransformParameterfvHP XGL_FUNCPTR(glImageTransformParameterfvHP)
#define glImageTransformParameteriHP XGL_FUNCPTR(glImageTransformParameteriHP)
#define glImageTransformParameterivHP XGL_FUNCPTR(glImageTransformParameterivHP)
#endif

#ifdef GL_HP_occlusion_test
#define GL_OCCLUSION_TEST_RESULT_HP 0x8166
#define GL_OCCLUSION_TEST_HP 0x8165
#endif

#ifdef GL_HP_texture_lighting
#endif

#ifdef GL_IBM_cull_vertex
#define GL_CULL_VERTEX_IBM 103050
#endif

#ifdef GL_IBM_multimode_draw_arrays
#define glMultiModeDrawArraysIBM XGL_FUNCPTR(glMultiModeDrawArraysIBM)
#define glMultiModeDrawElementsIBM XGL_FUNCPTR(glMultiModeDrawElementsIBM)
#endif

#ifdef GL_IBM_rasterpos_clip
#define GL_RASTER_POSITION_UNCLIPPED_IBM 103010
#endif

#ifdef GL_IBM_static_data
#define GL_ALL_STATIC_DATA_IBM 103060
#define GL_STATIC_VERTEX_ARRAY_IBM 103061
#endif

#ifdef GL_IBM_texture_mirrored_repeat
#define GL_MIRRORED_REPEAT_IBM 0x8370
#endif

#ifdef GL_IBM_vertex_array_lists
#define GL_VERTEX_ARRAY_LIST_IBM 103070
#define GL_NORMAL_ARRAY_LIST_IBM 103071
#define GL_COLOR_ARRAY_LIST_IBM 103072
#define GL_INDEX_ARRAY_LIST_IBM 103073
#define GL_TEXTURE_COORD_ARRAY_LIST_IBM 103074
#define GL_EDGE_FLAG_ARRAY_LIST_IBM 103075
#define GL_FOG_COORDINATE_ARRAY_LIST_IBM 103076
#define GL_SECONDARY_COLOR_ARRAY_LIST_IBM 103077
#define GL_VERTEX_ARRAY_LIST_STRIDE_IBM 103080
#define GL_NORMAL_ARRAY_LIST_STRIDE_IBM 103081
#define GL_COLOR_ARRAY_LIST_STRIDE_IBM 103082
#define GL_INDEX_ARRAY_LIST_STRIDE_IBM 103083
#define GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM 103084
#define GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM 103085
#define GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM 103086
#define GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM 103087
#define glColorPointerListIBM XGL_FUNCPTR(glColorPointerListIBM)
#define glEdgeFlagPointerListIBM XGL_FUNCPTR(glEdgeFlagPointerListIBM)
#define glFogCoordPointerListIBM XGL_FUNCPTR(glFogCoordPointerListIBM)
#define glIndexPointerListIBM XGL_FUNCPTR(glIndexPointerListIBM)
#define glNormalPointerListIBM XGL_FUNCPTR(glNormalPointerListIBM)
#define glSecondaryColorPointerListIBM XGL_FUNCPTR(glSecondaryColorPointerListIBM)
#define glTexCoordPointerListIBM XGL_FUNCPTR(glTexCoordPointerListIBM)
#define glVertexPointerListIBM XGL_FUNCPTR(glVertexPointerListIBM)
#endif

#ifdef GL_INGR_color_clamp
#define GL_RED_MIN_CLAMP_INGR 0x8560
#define GL_GREEN_MIN_CLAMP_INGR 0x8561
#define GL_BLUE_MIN_CLAMP_INGR 0x8562
#define GL_ALPHA_MIN_CLAMP_INGR 0x8563
#define GL_RED_MAX_CLAMP_INGR 0x8564
#define GL_GREEN_MAX_CLAMP_INGR 0x8565
#define GL_BLUE_MAX_CLAMP_INGR 0x8566
#define GL_ALPHA_MAX_CLAMP_INGR 0x8567
#endif

#ifdef GL_INGR_interlace_read
#define GL_INTERLACE_READ_INGR 0x8568
#endif

#ifdef GL_INTEL_parallel_arrays
#define GL_PARALLEL_ARRAYS_INTEL 0x83F4
#define GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL 0x83F5
#define GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL 0x83F6
#define GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL 0x83F7
#define GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL 0x83F8
#define glColorPointervINTEL XGL_FUNCPTR(glColorPointervINTEL)
#define glNormalPointervINTEL XGL_FUNCPTR(glNormalPointervINTEL)
#define glTexCoordPointervINTEL XGL_FUNCPTR(glTexCoordPointervINTEL)
#define glVertexPointervINTEL XGL_FUNCPTR(glVertexPointervINTEL)
#endif

#ifdef GL_INTEL_texture_scissor
#define glTexScissorFuncINTEL XGL_FUNCPTR(glTexScissorFuncINTEL)
#define glTexScissorINTEL XGL_FUNCPTR(glTexScissorINTEL)
#endif

#ifdef GL_KTX_buffer_region
#define GL_KTX_FRONT_REGION 0x0
#define GL_KTX_BACK_REGION 0x1
#define GL_KTX_Z_REGION 0x2
#define GL_KTX_STENCIL_REGION 0x3
#define glBufferRegionEnabledEXT XGL_FUNCPTR(glBufferRegionEnabledEXT)
#define glNewBufferRegionEXT XGL_FUNCPTR(glNewBufferRegionEXT)
#define glDeleteBufferRegionEXT XGL_FUNCPTR(glDeleteBufferRegionEXT)
#define glReadBufferRegionEXT XGL_FUNCPTR(glReadBufferRegionEXT)
#define glDrawBufferRegionEXT XGL_FUNCPTR(glDrawBufferRegionEXT)
#endif

#ifdef GL_MESA_pack_invert
#define GL_PACK_INVERT_MESA 0x8758
#endif

#ifdef GL_MESA_resize_buffers
#define glResizeBuffersMESA XGL_FUNCPTR(glResizeBuffersMESA)
#endif

#ifdef GL_MESA_window_pos
#define glWindowPos2dMESA XGL_FUNCPTR(glWindowPos2dMESA)
#define glWindowPos2dvMESA XGL_FUNCPTR(glWindowPos2dvMESA)
#define glWindowPos2fMESA XGL_FUNCPTR(glWindowPos2fMESA)
#define glWindowPos2fvMESA XGL_FUNCPTR(glWindowPos2fvMESA)
#define glWindowPos2iMESA XGL_FUNCPTR(glWindowPos2iMESA)
#define glWindowPos2ivMESA XGL_FUNCPTR(glWindowPos2ivMESA)
#define glWindowPos2sMESA XGL_FUNCPTR(glWindowPos2sMESA)
#define glWindowPos2svMESA XGL_FUNCPTR(glWindowPos2svMESA)
#define glWindowPos3dMESA XGL_FUNCPTR(glWindowPos3dMESA)
#define glWindowPos3dvMESA XGL_FUNCPTR(glWindowPos3dvMESA)
#define glWindowPos3fMESA XGL_FUNCPTR(glWindowPos3fMESA)
#define glWindowPos3fvMESA XGL_FUNCPTR(glWindowPos3fvMESA)
#define glWindowPos3iMESA XGL_FUNCPTR(glWindowPos3iMESA)
#define glWindowPos3ivMESA XGL_FUNCPTR(glWindowPos3ivMESA)
#define glWindowPos3sMESA XGL_FUNCPTR(glWindowPos3sMESA)
#define glWindowPos3svMESA XGL_FUNCPTR(glWindowPos3svMESA)
#define glWindowPos4dMESA XGL_FUNCPTR(glWindowPos4dMESA)
#define glWindowPos4dvMESA XGL_FUNCPTR(glWindowPos4dvMESA)
#define glWindowPos4fMESA XGL_FUNCPTR(glWindowPos4fMESA)
#define glWindowPos4fvMESA XGL_FUNCPTR(glWindowPos4fvMESA)
#define glWindowPos4iMESA XGL_FUNCPTR(glWindowPos4iMESA)
#define glWindowPos4ivMESA XGL_FUNCPTR(glWindowPos4ivMESA)
#define glWindowPos4sMESA XGL_FUNCPTR(glWindowPos4sMESA)
#define glWindowPos4svMESA XGL_FUNCPTR(glWindowPos4svMESA)
#endif

#ifdef GL_MESA_ycbcr_texture
#define GL_UNSIGNED_SHORT_8_8_MESA 0x85BA
#define GL_UNSIGNED_SHORT_8_8_REV_MESA 0x85BB
#define GL_YCBCR_MESA 0x8757
#endif

#ifdef GL_NV_blend_square
#endif

#ifdef GL_NV_copy_depth_to_color
#define GL_DEPTH_STENCIL_TO_RGBA_NV 0x886E
#define GL_DEPTH_STENCIL_TO_BGRA_NV 0x886F
#endif

#ifdef GL_NV_depth_clamp
#define GL_DEPTH_CLAMP_NV 0x864F
#endif

#ifdef GL_NV_evaluators
#define GL_EVAL_2D_NV 0x86C0
#define GL_EVAL_TRIANGULAR_2D_NV 0x86C1
#define GL_MAP_TESSELLATION_NV 0x86C2
#define GL_MAP_ATTRIB_U_ORDER_NV 0x86C3
#define GL_MAP_ATTRIB_V_ORDER_NV 0x86C4
#define GL_EVAL_FRACTIONAL_TESSELLATION_NV 0x86C5
#define GL_EVAL_VERTEX_ATTRIB0_NV 0x86C6
#define GL_EVAL_VERTEX_ATTRIB1_NV 0x86C7
#define GL_EVAL_VERTEX_ATTRIB2_NV 0x86C8
#define GL_EVAL_VERTEX_ATTRIB3_NV 0x86C9
#define GL_EVAL_VERTEX_ATTRIB4_NV 0x86CA
#define GL_EVAL_VERTEX_ATTRIB5_NV 0x86CB
#define GL_EVAL_VERTEX_ATTRIB6_NV 0x86CC
#define GL_EVAL_VERTEX_ATTRIB7_NV 0x86CD
#define GL_EVAL_VERTEX_ATTRIB8_NV 0x86CE
#define GL_EVAL_VERTEX_ATTRIB9_NV 0x86CF
#define GL_EVAL_VERTEX_ATTRIB10_NV 0x86D0
#define GL_EVAL_VERTEX_ATTRIB11_NV 0x86D1
#define GL_EVAL_VERTEX_ATTRIB12_NV 0x86D2
#define GL_EVAL_VERTEX_ATTRIB13_NV 0x86D3
#define GL_EVAL_VERTEX_ATTRIB14_NV 0x86D4
#define GL_EVAL_VERTEX_ATTRIB15_NV 0x86D5
#define GL_MAX_MAP_TESSELLATION_NV 0x86D6
#define GL_MAX_RATIONAL_EVAL_ORDER_NV 0x86D7
#define glEvalMapsNV XGL_FUNCPTR(glEvalMapsNV)
#define glGetMapAttribParameterfvNV XGL_FUNCPTR(glGetMapAttribParameterfvNV)
#define glGetMapAttribParameterivNV XGL_FUNCPTR(glGetMapAttribParameterivNV)
#define glGetMapControlPointsNV XGL_FUNCPTR(glGetMapControlPointsNV)
#define glGetMapParameterfvNV XGL_FUNCPTR(glGetMapParameterfvNV)
#define glGetMapParameterivNV XGL_FUNCPTR(glGetMapParameterivNV)
#define glMapControlPointsNV XGL_FUNCPTR(glMapControlPointsNV)
#define glMapParameterfvNV XGL_FUNCPTR(glMapParameterfvNV)
#define glMapParameterivNV XGL_FUNCPTR(glMapParameterivNV)
#endif

#ifdef GL_NV_fence
#define GL_ALL_COMPLETED_NV 0x84F2
#define GL_FENCE_STATUS_NV 0x84F3
#define GL_FENCE_CONDITION_NV 0x84F4
#define glDeleteFencesNV XGL_FUNCPTR(glDeleteFencesNV)
#define glFinishFenceNV XGL_FUNCPTR(glFinishFenceNV)
#define glGenFencesNV XGL_FUNCPTR(glGenFencesNV)
#define glGetFenceivNV XGL_FUNCPTR(glGetFenceivNV)
#define glIsFenceNV XGL_FUNCPTR(glIsFenceNV)
#define glSetFenceNV XGL_FUNCPTR(glSetFenceNV)
#define glTestFenceNV XGL_FUNCPTR(glTestFenceNV)
#endif

#ifdef GL_NV_float_buffer
#define GL_FLOAT_R_NV 0x8880
#define GL_FLOAT_RG_NV 0x8881
#define GL_FLOAT_RGB_NV 0x8882
#define GL_FLOAT_RGBA_NV 0x8883
#define GL_FLOAT_R16_NV 0x8884
#define GL_FLOAT_R32_NV 0x8885
#define GL_FLOAT_RG16_NV 0x8886
#define GL_FLOAT_RG32_NV 0x8887
#define GL_FLOAT_RGB16_NV 0x8888
#define GL_FLOAT_RGB32_NV 0x8889
#define GL_FLOAT_RGBA16_NV 0x888A
#define GL_FLOAT_RGBA32_NV 0x888B
#define GL_TEXTURE_FLOAT_COMPONENTS_NV 0x888C
#define GL_FLOAT_CLEAR_COLOR_VALUE_NV 0x888D
#define GL_FLOAT_RGBA_MODE_NV 0x888E
#endif

#ifdef GL_NV_fog_distance
#define GL_FOG_DISTANCE_MODE_NV 0x855A
#define GL_EYE_RADIAL_NV 0x855B
#define GL_EYE_PLANE_ABSOLUTE_NV 0x855C
#endif

#ifdef GL_NV_fragment_program
#define GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV 0x8868
#define GL_FRAGMENT_PROGRAM_NV 0x8870
#define GL_MAX_TEXTURE_COORDS_NV 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_NV 0x8872
#define GL_FRAGMENT_PROGRAM_BINDING_NV 0x8873
#define GL_PROGRAM_ERROR_STRING_NV 0x8874
#define glGetProgramNamedParameterdvNV XGL_FUNCPTR(glGetProgramNamedParameterdvNV)
#define glGetProgramNamedParameterfvNV XGL_FUNCPTR(glGetProgramNamedParameterfvNV)
#define glProgramNamedParameter4dNV XGL_FUNCPTR(glProgramNamedParameter4dNV)
#define glProgramNamedParameter4dvNV XGL_FUNCPTR(glProgramNamedParameter4dvNV)
#define glProgramNamedParameter4fNV XGL_FUNCPTR(glProgramNamedParameter4fNV)
#define glProgramNamedParameter4fvNV XGL_FUNCPTR(glProgramNamedParameter4fvNV)
#endif

#ifdef GL_NV_fragment_program2
#define GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV 0x88F4
#define GL_MAX_PROGRAM_CALL_DEPTH_NV 0x88F5
#define GL_MAX_PROGRAM_IF_DEPTH_NV 0x88F6
#define GL_MAX_PROGRAM_LOOP_DEPTH_NV 0x88F7
#define GL_MAX_PROGRAM_LOOP_COUNT_NV 0x88F8
#endif

#ifdef GL_NV_fragment_program_option
#endif

#ifdef GL_NV_half_float
typedef unsigned short GLhalf;
#define GL_HALF_FLOAT_NV 0x140B
#define glColor3hNV XGL_FUNCPTR(glColor3hNV)
#define glColor3hvNV XGL_FUNCPTR(glColor3hvNV)
#define glColor4hNV XGL_FUNCPTR(glColor4hNV)
#define glColor4hvNV XGL_FUNCPTR(glColor4hvNV)
#define glFogCoordhNV XGL_FUNCPTR(glFogCoordhNV)
#define glFogCoordhvNV XGL_FUNCPTR(glFogCoordhvNV)
#define glMultiTexCoord1hNV XGL_FUNCPTR(glMultiTexCoord1hNV)
#define glMultiTexCoord1hvNV XGL_FUNCPTR(glMultiTexCoord1hvNV)
#define glMultiTexCoord2hNV XGL_FUNCPTR(glMultiTexCoord2hNV)
#define glMultiTexCoord2hvNV XGL_FUNCPTR(glMultiTexCoord2hvNV)
#define glMultiTexCoord3hNV XGL_FUNCPTR(glMultiTexCoord3hNV)
#define glMultiTexCoord3hvNV XGL_FUNCPTR(glMultiTexCoord3hvNV)
#define glMultiTexCoord4hNV XGL_FUNCPTR(glMultiTexCoord4hNV)
#define glMultiTexCoord4hvNV XGL_FUNCPTR(glMultiTexCoord4hvNV)
#define glNormal3hNV XGL_FUNCPTR(glNormal3hNV)
#define glNormal3hvNV XGL_FUNCPTR(glNormal3hvNV)
#define glSecondaryColor3hNV XGL_FUNCPTR(glSecondaryColor3hNV)
#define glSecondaryColor3hvNV XGL_FUNCPTR(glSecondaryColor3hvNV)
#define glTexCoord1hNV XGL_FUNCPTR(glTexCoord1hNV)
#define glTexCoord1hvNV XGL_FUNCPTR(glTexCoord1hvNV)
#define glTexCoord2hNV XGL_FUNCPTR(glTexCoord2hNV)
#define glTexCoord2hvNV XGL_FUNCPTR(glTexCoord2hvNV)
#define glTexCoord3hNV XGL_FUNCPTR(glTexCoord3hNV)
#define glTexCoord3hvNV XGL_FUNCPTR(glTexCoord3hvNV)
#define glTexCoord4hNV XGL_FUNCPTR(glTexCoord4hNV)
#define glTexCoord4hvNV XGL_FUNCPTR(glTexCoord4hvNV)
#define glVertex2hNV XGL_FUNCPTR(glVertex2hNV)
#define glVertex2hvNV XGL_FUNCPTR(glVertex2hvNV)
#define glVertex3hNV XGL_FUNCPTR(glVertex3hNV)
#define glVertex3hvNV XGL_FUNCPTR(glVertex3hvNV)
#define glVertex4hNV XGL_FUNCPTR(glVertex4hNV)
#define glVertex4hvNV XGL_FUNCPTR(glVertex4hvNV)
#define glVertexAttrib1hNV XGL_FUNCPTR(glVertexAttrib1hNV)
#define glVertexAttrib1hvNV XGL_FUNCPTR(glVertexAttrib1hvNV)
#define glVertexAttrib2hNV XGL_FUNCPTR(glVertexAttrib2hNV)
#define glVertexAttrib2hvNV XGL_FUNCPTR(glVertexAttrib2hvNV)
#define glVertexAttrib3hNV XGL_FUNCPTR(glVertexAttrib3hNV)
#define glVertexAttrib3hvNV XGL_FUNCPTR(glVertexAttrib3hvNV)
#define glVertexAttrib4hNV XGL_FUNCPTR(glVertexAttrib4hNV)
#define glVertexAttrib4hvNV XGL_FUNCPTR(glVertexAttrib4hvNV)
#define glVertexAttribs1hvNV XGL_FUNCPTR(glVertexAttribs1hvNV)
#define glVertexAttribs2hvNV XGL_FUNCPTR(glVertexAttribs2hvNV)
#define glVertexAttribs3hvNV XGL_FUNCPTR(glVertexAttribs3hvNV)
#define glVertexAttribs4hvNV XGL_FUNCPTR(glVertexAttribs4hvNV)
#define glVertexWeighthNV XGL_FUNCPTR(glVertexWeighthNV)
#define glVertexWeighthvNV XGL_FUNCPTR(glVertexWeighthvNV)
#endif

#ifdef GL_NV_light_max_exponent
#define GL_MAX_SHININESS_NV 0x8504
#define GL_MAX_SPOT_EXPONENT_NV 0x8505
#endif

#ifdef GL_NV_multisample_filter_hint
#define GL_MULTISAMPLE_FILTER_HINT_NV 0x8534
#endif

#ifdef GL_NV_occlusion_query
#define GL_PIXEL_COUNTER_BITS_NV 0x8864
#define GL_CURRENT_OCCLUSION_QUERY_ID_NV 0x8865
#define GL_PIXEL_COUNT_NV 0x8866
#define GL_PIXEL_COUNT_AVAILABLE_NV 0x8867
#define glBeginOcclusionQueryNV XGL_FUNCPTR(glBeginOcclusionQueryNV)
#define glDeleteOcclusionQueriesNV XGL_FUNCPTR(glDeleteOcclusionQueriesNV)
#define glEndOcclusionQueryNV XGL_FUNCPTR(glEndOcclusionQueryNV)
#define glGenOcclusionQueriesNV XGL_FUNCPTR(glGenOcclusionQueriesNV)
#define glGetOcclusionQueryivNV XGL_FUNCPTR(glGetOcclusionQueryivNV)
#define glGetOcclusionQueryuivNV XGL_FUNCPTR(glGetOcclusionQueryuivNV)
#define glIsOcclusionQueryNV XGL_FUNCPTR(glIsOcclusionQueryNV)
#endif

#ifdef GL_NV_packed_depth_stencil
#define GL_DEPTH_STENCIL_NV 0x84F9
#define GL_UNSIGNED_INT_24_8_NV 0x84FA
#endif

#ifdef GL_NV_pixel_data_range
#define GL_WRITE_PIXEL_DATA_RANGE_NV 0x8878
#define GL_READ_PIXEL_DATA_RANGE_NV 0x8879
#define GL_WRITE_PIXEL_DATA_RANGE_LENGTH_NV 0x887A
#define GL_READ_PIXEL_DATA_RANGE_LENGTH_NV 0x887B
#define GL_WRITE_PIXEL_DATA_RANGE_POINTER_NV 0x887C
#define GL_READ_PIXEL_DATA_RANGE_POINTER_NV 0x887D
#define glFlushPixelDataRangeNV XGL_FUNCPTR(glFlushPixelDataRangeNV)
#define glPixelDataRangeNV XGL_FUNCPTR(glPixelDataRangeNV)
#endif

#ifdef GL_NV_point_sprite
#define GL_POINT_SPRITE_NV 0x8861
#define GL_COORD_REPLACE_NV 0x8862
#define GL_POINT_SPRITE_R_MODE_NV 0x8863
#define glPointParameteriNV XGL_FUNCPTR(glPointParameteriNV)
#define glPointParameterivNV XGL_FUNCPTR(glPointParameterivNV)
#endif

#ifdef GL_NV_primitive_restart
#define GL_PRIMITIVE_RESTART_NV 0x8558
#define GL_PRIMITIVE_RESTART_INDEX_NV 0x8559
#define glPrimitiveRestartIndexNV XGL_FUNCPTR(glPrimitiveRestartIndexNV)
#define glPrimitiveRestartNV XGL_FUNCPTR(glPrimitiveRestartNV)
#endif

#ifdef GL_NV_register_combiners
#define GL_REGISTER_COMBINERS_NV 0x8522
#define GL_VARIABLE_A_NV 0x8523
#define GL_VARIABLE_B_NV 0x8524
#define GL_VARIABLE_C_NV 0x8525
#define GL_VARIABLE_D_NV 0x8526
#define GL_VARIABLE_E_NV 0x8527
#define GL_VARIABLE_F_NV 0x8528
#define GL_VARIABLE_G_NV 0x8529
#define GL_CONSTANT_COLOR0_NV 0x852A
#define GL_CONSTANT_COLOR1_NV 0x852B
#define GL_PRIMARY_COLOR_NV 0x852C
#define GL_SECONDARY_COLOR_NV 0x852D
#define GL_SPARE0_NV 0x852E
#define GL_SPARE1_NV 0x852F
#define GL_DISCARD_NV 0x8530
#define GL_E_TIMES_F_NV 0x8531
#define GL_SPARE0_PLUS_SECONDARY_COLOR_NV 0x8532
#define GL_UNSIGNED_IDENTITY_NV 0x8536
#define GL_UNSIGNED_INVERT_NV 0x8537
#define GL_EXPAND_NORMAL_NV 0x8538
#define GL_EXPAND_NEGATE_NV 0x8539
#define GL_HALF_BIAS_NORMAL_NV 0x853A
#define GL_HALF_BIAS_NEGATE_NV 0x853B
#define GL_SIGNED_IDENTITY_NV 0x853C
#define GL_SIGNED_NEGATE_NV 0x853D
#define GL_SCALE_BY_TWO_NV 0x853E
#define GL_SCALE_BY_FOUR_NV 0x853F
#define GL_SCALE_BY_ONE_HALF_NV 0x8540
#define GL_BIAS_BY_NEGATIVE_ONE_HALF_NV 0x8541
#define GL_COMBINER_INPUT_NV 0x8542
#define GL_COMBINER_MAPPING_NV 0x8543
#define GL_COMBINER_COMPONENT_USAGE_NV 0x8544
#define GL_COMBINER_AB_DOT_PRODUCT_NV 0x8545
#define GL_COMBINER_CD_DOT_PRODUCT_NV 0x8546
#define GL_COMBINER_MUX_SUM_NV 0x8547
#define GL_COMBINER_SCALE_NV 0x8548
#define GL_COMBINER_BIAS_NV 0x8549
#define GL_COMBINER_AB_OUTPUT_NV 0x854A
#define GL_COMBINER_CD_OUTPUT_NV 0x854B
#define GL_COMBINER_SUM_OUTPUT_NV 0x854C
#define GL_MAX_GENERAL_COMBINERS_NV 0x854D
#define GL_NUM_GENERAL_COMBINERS_NV 0x854E
#define GL_COLOR_SUM_CLAMP_NV 0x854F
#define GL_COMBINER0_NV 0x8550
#define GL_COMBINER1_NV 0x8551
#define GL_COMBINER2_NV 0x8552
#define GL_COMBINER3_NV 0x8553
#define GL_COMBINER4_NV 0x8554
#define GL_COMBINER5_NV 0x8555
#define GL_COMBINER6_NV 0x8556
#define GL_COMBINER7_NV 0x8557
#define glCombinerInputNV XGL_FUNCPTR(glCombinerInputNV)
#define glCombinerOutputNV XGL_FUNCPTR(glCombinerOutputNV)
#define glCombinerParameterfNV XGL_FUNCPTR(glCombinerParameterfNV)
#define glCombinerParameterfvNV XGL_FUNCPTR(glCombinerParameterfvNV)
#define glCombinerParameteriNV XGL_FUNCPTR(glCombinerParameteriNV)
#define glCombinerParameterivNV XGL_FUNCPTR(glCombinerParameterivNV)
#define glFinalCombinerInputNV XGL_FUNCPTR(glFinalCombinerInputNV)
#define glGetCombinerInputParameterfvNV XGL_FUNCPTR(glGetCombinerInputParameterfvNV)
#define glGetCombinerInputParameterivNV XGL_FUNCPTR(glGetCombinerInputParameterivNV)
#define glGetCombinerOutputParameterfvNV XGL_FUNCPTR(glGetCombinerOutputParameterfvNV)
#define glGetCombinerOutputParameterivNV XGL_FUNCPTR(glGetCombinerOutputParameterivNV)
#define glGetFinalCombinerInputParameterfvNV XGL_FUNCPTR(glGetFinalCombinerInputParameterfvNV)
#define glGetFinalCombinerInputParameterivNV XGL_FUNCPTR(glGetFinalCombinerInputParameterivNV)
#endif

#ifdef GL_NV_register_combiners2
#define GL_PER_STAGE_CONSTANTS_NV 0x8535
#define glCombinerStageParameterfvNV XGL_FUNCPTR(glCombinerStageParameterfvNV)
#define glGetCombinerStageParameterfvNV XGL_FUNCPTR(glGetCombinerStageParameterfvNV)
#endif

#ifdef GL_NV_texgen_emboss
#define GL_EMBOSS_LIGHT_NV 0x855D
#define GL_EMBOSS_CONSTANT_NV 0x855E
#define GL_EMBOSS_MAP_NV 0x855F
#endif

#ifdef GL_NV_texgen_reflection
#define GL_NORMAL_MAP_NV 0x8511
#define GL_REFLECTION_MAP_NV 0x8512
#endif

#ifdef GL_NV_texture_compression_vtc
#endif

#ifdef GL_NV_texture_env_combine4
#define GL_COMBINE4_NV 0x8503
#define GL_SOURCE3_RGB_NV 0x8583
#define GL_SOURCE3_ALPHA_NV 0x858B
#define GL_OPERAND3_RGB_NV 0x8593
#define GL_OPERAND3_ALPHA_NV 0x859B
#endif

#ifdef GL_NV_texture_expand_normal
#define GL_TEXTURE_UNSIGNED_REMAP_MODE_NV 0x888F
#endif

#ifdef GL_NV_texture_rectangle
#define GL_TEXTURE_RECTANGLE_NV 0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_NV 0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_NV 0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_NV 0x84F8
#endif

#ifdef GL_NV_texture_shader
#define GL_OFFSET_TEXTURE_RECTANGLE_NV 0x864C
#define GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV 0x864D
#define GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV 0x864E
#define GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV 0x86D9
#define GL_UNSIGNED_INT_S8_S8_8_8_NV 0x86DA
#define GL_UNSIGNED_INT_8_8_S8_S8_REV_NV 0x86DB
#define GL_DSDT_MAG_INTENSITY_NV 0x86DC
#define GL_SHADER_CONSISTENT_NV 0x86DD
#define GL_TEXTURE_SHADER_NV 0x86DE
#define GL_SHADER_OPERATION_NV 0x86DF
#define GL_CULL_MODES_NV 0x86E0
#define GL_OFFSET_TEXTURE_MATRIX_NV 0x86E1
#define GL_OFFSET_TEXTURE_SCALE_NV 0x86E2
#define GL_OFFSET_TEXTURE_BIAS_NV 0x86E3
#define GL_PREVIOUS_TEXTURE_INPUT_NV 0x86E4
#define GL_CONST_EYE_NV 0x86E5
#define GL_PASS_THROUGH_NV 0x86E6
#define GL_CULL_FRAGMENT_NV 0x86E7
#define GL_OFFSET_TEXTURE_2D_NV 0x86E8
#define GL_DEPENDENT_AR_TEXTURE_2D_NV 0x86E9
#define GL_DEPENDENT_GB_TEXTURE_2D_NV 0x86EA
#define GL_DOT_PRODUCT_NV 0x86EC
#define GL_DOT_PRODUCT_DEPTH_REPLACE_NV 0x86ED
#define GL_DOT_PRODUCT_TEXTURE_2D_NV 0x86EE
#define GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV 0x86F0
#define GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV 0x86F1
#define GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV 0x86F2
#define GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV 0x86F3
#define GL_HILO_NV 0x86F4
#define GL_DSDT_NV 0x86F5
#define GL_DSDT_MAG_NV 0x86F6
#define GL_DSDT_MAG_VIB_NV 0x86F7
#define GL_HILO16_NV 0x86F8
#define GL_SIGNED_HILO_NV 0x86F9
#define GL_SIGNED_HILO16_NV 0x86FA
#define GL_SIGNED_RGBA_NV 0x86FB
#define GL_SIGNED_RGBA8_NV 0x86FC
#define GL_SIGNED_RGB_NV 0x86FE
#define GL_SIGNED_RGB8_NV 0x86FF
#define GL_SIGNED_LUMINANCE_NV 0x8701
#define GL_SIGNED_LUMINANCE8_NV 0x8702
#define GL_SIGNED_LUMINANCE_ALPHA_NV 0x8703
#define GL_SIGNED_LUMINANCE8_ALPHA8_NV 0x8704
#define GL_SIGNED_ALPHA_NV 0x8705
#define GL_SIGNED_ALPHA8_NV 0x8706
#define GL_SIGNED_INTENSITY_NV 0x8707
#define GL_SIGNED_INTENSITY8_NV 0x8708
#define GL_DSDT8_NV 0x8709
#define GL_DSDT8_MAG8_NV 0x870A
#define GL_DSDT8_MAG8_INTENSITY8_NV 0x870B
#define GL_SIGNED_RGB_UNSIGNED_ALPHA_NV 0x870C
#define GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV 0x870D
#define GL_HI_SCALE_NV 0x870E
#define GL_LO_SCALE_NV 0x870F
#define GL_DS_SCALE_NV 0x8710
#define GL_DT_SCALE_NV 0x8711
#define GL_MAGNITUDE_SCALE_NV 0x8712
#define GL_VIBRANCE_SCALE_NV 0x8713
#define GL_HI_BIAS_NV 0x8714
#define GL_LO_BIAS_NV 0x8715
#define GL_DS_BIAS_NV 0x8716
#define GL_DT_BIAS_NV 0x8717
#define GL_MAGNITUDE_BIAS_NV 0x8718
#define GL_VIBRANCE_BIAS_NV 0x8719
#define GL_TEXTURE_BORDER_VALUES_NV 0x871A
#define GL_TEXTURE_HI_SIZE_NV 0x871B
#define GL_TEXTURE_LO_SIZE_NV 0x871C
#define GL_TEXTURE_DS_SIZE_NV 0x871D
#define GL_TEXTURE_DT_SIZE_NV 0x871E
#define GL_TEXTURE_MAG_SIZE_NV 0x871F
#endif

#ifdef GL_NV_texture_shader2
#define GL_UNSIGNED_INT_S8_S8_8_8_NV 0x86DA
#define GL_UNSIGNED_INT_8_8_S8_S8_REV_NV 0x86DB
#define GL_DSDT_MAG_INTENSITY_NV 0x86DC
#define GL_DOT_PRODUCT_TEXTURE_3D_NV 0x86EF
#define GL_HILO_NV 0x86F4
#define GL_DSDT_NV 0x86F5
#define GL_DSDT_MAG_NV 0x86F6
#define GL_DSDT_MAG_VIB_NV 0x86F7
#define GL_HILO16_NV 0x86F8
#define GL_SIGNED_HILO_NV 0x86F9
#define GL_SIGNED_HILO16_NV 0x86FA
#define GL_SIGNED_RGBA_NV 0x86FB
#define GL_SIGNED_RGBA8_NV 0x86FC
#define GL_SIGNED_RGB_NV 0x86FE
#define GL_SIGNED_RGB8_NV 0x86FF
#define GL_SIGNED_LUMINANCE_NV 0x8701
#define GL_SIGNED_LUMINANCE8_NV 0x8702
#define GL_SIGNED_LUMINANCE_ALPHA_NV 0x8703
#define GL_SIGNED_LUMINANCE8_ALPHA8_NV 0x8704
#define GL_SIGNED_ALPHA_NV 0x8705
#define GL_SIGNED_ALPHA8_NV 0x8706
#define GL_SIGNED_INTENSITY_NV 0x8707
#define GL_SIGNED_INTENSITY8_NV 0x8708
#define GL_DSDT8_NV 0x8709
#define GL_DSDT8_MAG8_NV 0x870A
#define GL_DSDT8_MAG8_INTENSITY8_NV 0x870B
#define GL_SIGNED_RGB_UNSIGNED_ALPHA_NV 0x870C
#define GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV 0x870D
#endif

#ifdef GL_NV_texture_shader3
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV 0x8850
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV 0x8851
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV 0x8852
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV 0x8853
#define GL_OFFSET_HILO_TEXTURE_2D_NV 0x8854
#define GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV 0x8855
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV 0x8856
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV 0x8857
#define GL_DEPENDENT_HILO_TEXTURE_2D_NV 0x8858
#define GL_DEPENDENT_RGB_TEXTURE_3D_NV 0x8859
#define GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV 0x885A
#define GL_DOT_PRODUCT_PASS_THROUGH_NV 0x885B
#define GL_DOT_PRODUCT_TEXTURE_1D_NV 0x885C
#define GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV 0x885D
#define GL_HILO8_NV 0x885E
#define GL_SIGNED_HILO8_NV 0x885F
#define GL_FORCE_BLUE_TO_ONE_NV 0x8860
#endif

#ifdef GL_NV_vertex_array_range
#define GL_VERTEX_ARRAY_RANGE_NV 0x851D
#define GL_VERTEX_ARRAY_RANGE_LENGTH_NV 0x851E
#define GL_VERTEX_ARRAY_RANGE_VALID_NV 0x851F
#define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV 0x8520
#define GL_VERTEX_ARRAY_RANGE_POINTER_NV 0x8521
#define glFlushVertexArrayRangeNV XGL_FUNCPTR(glFlushVertexArrayRangeNV)
#define glVertexArrayRangeNV XGL_FUNCPTR(glVertexArrayRangeNV)
#endif

#ifdef GL_NV_vertex_array_range2
#define GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV 0x8533
#endif

#ifdef GL_NV_vertex_program
#define GL_VERTEX_PROGRAM_NV 0x8620
#define GL_VERTEX_STATE_PROGRAM_NV 0x8621
#define GL_ATTRIB_ARRAY_SIZE_NV 0x8623
#define GL_ATTRIB_ARRAY_STRIDE_NV 0x8624
#define GL_ATTRIB_ARRAY_TYPE_NV 0x8625
#define GL_CURRENT_ATTRIB_NV 0x8626
#define GL_PROGRAM_LENGTH_NV 0x8627
#define GL_PROGRAM_STRING_NV 0x8628
#define GL_MODELVIEW_PROJECTION_NV 0x8629
#define GL_IDENTITY_NV 0x862A
#define GL_INVERSE_NV 0x862B
#define GL_TRANSPOSE_NV 0x862C
#define GL_INVERSE_TRANSPOSE_NV 0x862D
#define GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV 0x862E
#define GL_MAX_TRACK_MATRICES_NV 0x862F
#define GL_MATRIX0_NV 0x8630
#define GL_MATRIX1_NV 0x8631
#define GL_MATRIX2_NV 0x8632
#define GL_MATRIX3_NV 0x8633
#define GL_MATRIX4_NV 0x8634
#define GL_MATRIX5_NV 0x8635
#define GL_MATRIX6_NV 0x8636
#define GL_MATRIX7_NV 0x8637
#define GL_CURRENT_MATRIX_STACK_DEPTH_NV 0x8640
#define GL_CURRENT_MATRIX_NV 0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_NV 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_NV 0x8643
#define GL_PROGRAM_PARAMETER_NV 0x8644
#define GL_ATTRIB_ARRAY_POINTER_NV 0x8645
#define GL_PROGRAM_TARGET_NV 0x8646
#define GL_PROGRAM_RESIDENT_NV 0x8647
#define GL_TRACK_MATRIX_NV 0x8648
#define GL_TRACK_MATRIX_TRANSFORM_NV 0x8649
#define GL_VERTEX_PROGRAM_BINDING_NV 0x864A
#define GL_PROGRAM_ERROR_POSITION_NV 0x864B
#define GL_VERTEX_ATTRIB_ARRAY0_NV 0x8650
#define GL_VERTEX_ATTRIB_ARRAY1_NV 0x8651
#define GL_VERTEX_ATTRIB_ARRAY2_NV 0x8652
#define GL_VERTEX_ATTRIB_ARRAY3_NV 0x8653
#define GL_VERTEX_ATTRIB_ARRAY4_NV 0x8654
#define GL_VERTEX_ATTRIB_ARRAY5_NV 0x8655
#define GL_VERTEX_ATTRIB_ARRAY6_NV 0x8656
#define GL_VERTEX_ATTRIB_ARRAY7_NV 0x8657
#define GL_VERTEX_ATTRIB_ARRAY8_NV 0x8658
#define GL_VERTEX_ATTRIB_ARRAY9_NV 0x8659
#define GL_VERTEX_ATTRIB_ARRAY10_NV 0x865A
#define GL_VERTEX_ATTRIB_ARRAY11_NV 0x865B
#define GL_VERTEX_ATTRIB_ARRAY12_NV 0x865C
#define GL_VERTEX_ATTRIB_ARRAY13_NV 0x865D
#define GL_VERTEX_ATTRIB_ARRAY14_NV 0x865E
#define GL_VERTEX_ATTRIB_ARRAY15_NV 0x865F
#define GL_MAP1_VERTEX_ATTRIB0_4_NV 0x8660
#define GL_MAP1_VERTEX_ATTRIB1_4_NV 0x8661
#define GL_MAP1_VERTEX_ATTRIB2_4_NV 0x8662
#define GL_MAP1_VERTEX_ATTRIB3_4_NV 0x8663
#define GL_MAP1_VERTEX_ATTRIB4_4_NV 0x8664
#define GL_MAP1_VERTEX_ATTRIB5_4_NV 0x8665
#define GL_MAP1_VERTEX_ATTRIB6_4_NV 0x8666
#define GL_MAP1_VERTEX_ATTRIB7_4_NV 0x8667
#define GL_MAP1_VERTEX_ATTRIB8_4_NV 0x8668
#define GL_MAP1_VERTEX_ATTRIB9_4_NV 0x8669
#define GL_MAP1_VERTEX_ATTRIB10_4_NV 0x866A
#define GL_MAP1_VERTEX_ATTRIB11_4_NV 0x866B
#define GL_MAP1_VERTEX_ATTRIB12_4_NV 0x866C
#define GL_MAP1_VERTEX_ATTRIB13_4_NV 0x866D
#define GL_MAP1_VERTEX_ATTRIB14_4_NV 0x866E
#define GL_MAP1_VERTEX_ATTRIB15_4_NV 0x866F
#define GL_MAP2_VERTEX_ATTRIB0_4_NV 0x8670
#define GL_MAP2_VERTEX_ATTRIB1_4_NV 0x8671
#define GL_MAP2_VERTEX_ATTRIB2_4_NV 0x8672
#define GL_MAP2_VERTEX_ATTRIB3_4_NV 0x8673
#define GL_MAP2_VERTEX_ATTRIB4_4_NV 0x8674
#define GL_MAP2_VERTEX_ATTRIB5_4_NV 0x8675
#define GL_MAP2_VERTEX_ATTRIB6_4_NV 0x8676
#define GL_MAP2_VERTEX_ATTRIB7_4_NV 0x8677
#define GL_MAP2_VERTEX_ATTRIB8_4_NV 0x8678
#define GL_MAP2_VERTEX_ATTRIB9_4_NV 0x8679
#define GL_MAP2_VERTEX_ATTRIB10_4_NV 0x867A
#define GL_MAP2_VERTEX_ATTRIB11_4_NV 0x867B
#define GL_MAP2_VERTEX_ATTRIB12_4_NV 0x867C
#define GL_MAP2_VERTEX_ATTRIB13_4_NV 0x867D
#define GL_MAP2_VERTEX_ATTRIB14_4_NV 0x867E
#define GL_MAP2_VERTEX_ATTRIB15_4_NV 0x867F
#define glAreProgramsResidentNV XGL_FUNCPTR(glAreProgramsResidentNV)
#define glBindProgramNV XGL_FUNCPTR(glBindProgramNV)
#define glDeleteProgramsNV XGL_FUNCPTR(glDeleteProgramsNV)
#define glExecuteProgramNV XGL_FUNCPTR(glExecuteProgramNV)
#define glGenProgramsNV XGL_FUNCPTR(glGenProgramsNV)
#define glGetProgramParameterdvNV XGL_FUNCPTR(glGetProgramParameterdvNV)
#define glGetProgramParameterfvNV XGL_FUNCPTR(glGetProgramParameterfvNV)
#define glGetProgramStringNV XGL_FUNCPTR(glGetProgramStringNV)
#define glGetProgramivNV XGL_FUNCPTR(glGetProgramivNV)
#define glGetTrackMatrixivNV XGL_FUNCPTR(glGetTrackMatrixivNV)
#define glGetVertexAttribPointervNV XGL_FUNCPTR(glGetVertexAttribPointervNV)
#define glGetVertexAttribdvNV XGL_FUNCPTR(glGetVertexAttribdvNV)
#define glGetVertexAttribfvNV XGL_FUNCPTR(glGetVertexAttribfvNV)
#define glGetVertexAttribivNV XGL_FUNCPTR(glGetVertexAttribivNV)
#define glIsProgramNV XGL_FUNCPTR(glIsProgramNV)
#define glLoadProgramNV XGL_FUNCPTR(glLoadProgramNV)
#define glProgramParameter4dNV XGL_FUNCPTR(glProgramParameter4dNV)
#define glProgramParameter4dvNV XGL_FUNCPTR(glProgramParameter4dvNV)
#define glProgramParameter4fNV XGL_FUNCPTR(glProgramParameter4fNV)
#define glProgramParameter4fvNV XGL_FUNCPTR(glProgramParameter4fvNV)
#define glProgramParameters4dvNV XGL_FUNCPTR(glProgramParameters4dvNV)
#define glProgramParameters4fvNV XGL_FUNCPTR(glProgramParameters4fvNV)
#define glRequestResidentProgramsNV XGL_FUNCPTR(glRequestResidentProgramsNV)
#define glTrackMatrixNV XGL_FUNCPTR(glTrackMatrixNV)
#define glVertexAttrib1dNV XGL_FUNCPTR(glVertexAttrib1dNV)
#define glVertexAttrib1dvNV XGL_FUNCPTR(glVertexAttrib1dvNV)
#define glVertexAttrib1fNV XGL_FUNCPTR(glVertexAttrib1fNV)
#define glVertexAttrib1fvNV XGL_FUNCPTR(glVertexAttrib1fvNV)
#define glVertexAttrib1sNV XGL_FUNCPTR(glVertexAttrib1sNV)
#define glVertexAttrib1svNV XGL_FUNCPTR(glVertexAttrib1svNV)
#define glVertexAttrib2dNV XGL_FUNCPTR(glVertexAttrib2dNV)
#define glVertexAttrib2dvNV XGL_FUNCPTR(glVertexAttrib2dvNV)
#define glVertexAttrib2fNV XGL_FUNCPTR(glVertexAttrib2fNV)
#define glVertexAttrib2fvNV XGL_FUNCPTR(glVertexAttrib2fvNV)
#define glVertexAttrib2sNV XGL_FUNCPTR(glVertexAttrib2sNV)
#define glVertexAttrib2svNV XGL_FUNCPTR(glVertexAttrib2svNV)
#define glVertexAttrib3dNV XGL_FUNCPTR(glVertexAttrib3dNV)
#define glVertexAttrib3dvNV XGL_FUNCPTR(glVertexAttrib3dvNV)
#define glVertexAttrib3fNV XGL_FUNCPTR(glVertexAttrib3fNV)
#define glVertexAttrib3fvNV XGL_FUNCPTR(glVertexAttrib3fvNV)
#define glVertexAttrib3sNV XGL_FUNCPTR(glVertexAttrib3sNV)
#define glVertexAttrib3svNV XGL_FUNCPTR(glVertexAttrib3svNV)
#define glVertexAttrib4dNV XGL_FUNCPTR(glVertexAttrib4dNV)
#define glVertexAttrib4dvNV XGL_FUNCPTR(glVertexAttrib4dvNV)
#define glVertexAttrib4fNV XGL_FUNCPTR(glVertexAttrib4fNV)
#define glVertexAttrib4fvNV XGL_FUNCPTR(glVertexAttrib4fvNV)
#define glVertexAttrib4sNV XGL_FUNCPTR(glVertexAttrib4sNV)
#define glVertexAttrib4svNV XGL_FUNCPTR(glVertexAttrib4svNV)
#define glVertexAttrib4ubNV XGL_FUNCPTR(glVertexAttrib4ubNV)
#define glVertexAttrib4ubvNV XGL_FUNCPTR(glVertexAttrib4ubvNV)
#define glVertexAttribPointerNV XGL_FUNCPTR(glVertexAttribPointerNV)
#define glVertexAttribs1dvNV XGL_FUNCPTR(glVertexAttribs1dvNV)
#define glVertexAttribs1fvNV XGL_FUNCPTR(glVertexAttribs1fvNV)
#define glVertexAttribs1svNV XGL_FUNCPTR(glVertexAttribs1svNV)
#define glVertexAttribs2dvNV XGL_FUNCPTR(glVertexAttribs2dvNV)
#define glVertexAttribs2fvNV XGL_FUNCPTR(glVertexAttribs2fvNV)
#define glVertexAttribs2svNV XGL_FUNCPTR(glVertexAttribs2svNV)
#define glVertexAttribs3dvNV XGL_FUNCPTR(glVertexAttribs3dvNV)
#define glVertexAttribs3fvNV XGL_FUNCPTR(glVertexAttribs3fvNV)
#define glVertexAttribs3svNV XGL_FUNCPTR(glVertexAttribs3svNV)
#define glVertexAttribs4dvNV XGL_FUNCPTR(glVertexAttribs4dvNV)
#define glVertexAttribs4fvNV XGL_FUNCPTR(glVertexAttribs4fvNV)
#define glVertexAttribs4svNV XGL_FUNCPTR(glVertexAttribs4svNV)
#define glVertexAttribs4ubvNV XGL_FUNCPTR(glVertexAttribs4ubvNV)
#endif

#ifdef GL_NV_vertex_program1_1
#endif

#ifdef GL_NV_vertex_program2
#endif

#ifdef GL_NV_vertex_program2_option
#define GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV 0x88F4
#define GL_MAX_PROGRAM_CALL_DEPTH_NV 0x88F5
#endif

#ifdef GL_NV_vertex_program3
#define MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB 0x8B4C
#endif

#ifdef GL_OML_interlace
#define GL_INTERLACE_OML 0x8980
#define GL_INTERLACE_READ_OML 0x8981
#endif

#ifdef GL_OML_resample
#define GL_PACK_RESAMPLE_OML 0x8984
#define GL_UNPACK_RESAMPLE_OML 0x8985
#define GL_RESAMPLE_REPLICATE_OML 0x8986
#define GL_RESAMPLE_ZERO_FILL_OML 0x8987
#define GL_RESAMPLE_AVERAGE_OML 0x8988
#define GL_RESAMPLE_DECIMATE_OML 0x8989
#endif

#ifdef GL_OML_subsample
#define GL_FORMAT_SUBSAMPLE_24_24_OML 0x8982
#define GL_FORMAT_SUBSAMPLE_244_244_OML 0x8983
#endif

#ifdef GL_PGI_misc_hints
#define GL_PREFER_DOUBLEBUFFER_HINT_PGI 107000
#define GL_CONSERVE_MEMORY_HINT_PGI 107005
#define GL_RECLAIM_MEMORY_HINT_PGI 107006
#define GL_NATIVE_GRAPHICS_HANDLE_PGI 107010
#define GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI 107011
#define GL_NATIVE_GRAPHICS_END_HINT_PGI 107012
#define GL_ALWAYS_FAST_HINT_PGI 107020
#define GL_ALWAYS_SOFT_HINT_PGI 107021
#define GL_ALLOW_DRAW_OBJ_HINT_PGI 107022
#define GL_ALLOW_DRAW_WIN_HINT_PGI 107023
#define GL_ALLOW_DRAW_FRG_HINT_PGI 107024
#define GL_ALLOW_DRAW_MEM_HINT_PGI 107025
#define GL_STRICT_DEPTHFUNC_HINT_PGI 107030
#define GL_STRICT_LIGHTING_HINT_PGI 107031
#define GL_STRICT_SCISSOR_HINT_PGI 107032
#define GL_FULL_STIPPLE_HINT_PGI 107033
#define GL_CLIP_NEAR_HINT_PGI 107040
#define GL_CLIP_FAR_HINT_PGI 107041
#define GL_WIDE_LINE_HINT_PGI 107042
#define GL_BACK_NORMALS_HINT_PGI 107043
#endif

#ifdef GL_PGI_vertex_hints
#define GL_VERTEX23_BIT_PGI 0x00000004
#define GL_VERTEX4_BIT_PGI 0x00000008
#define GL_COLOR3_BIT_PGI 0x00010000
#define GL_COLOR4_BIT_PGI 0x00020000
#define GL_EDGEFLAG_BIT_PGI 0x00040000
#define GL_INDEX_BIT_PGI 0x00080000
#define GL_MAT_AMBIENT_BIT_PGI 0x00100000
#define GL_VERTEX_DATA_HINT_PGI 107050
#define GL_VERTEX_CONSISTENT_HINT_PGI 107051
#define GL_MATERIAL_SIDE_HINT_PGI 107052
#define GL_MAX_VERTEX_HINT_PGI 107053
#define GL_MAT_AMBIENT_AND_DIFFUSE_BIT_PGI 0x00200000
#define GL_MAT_DIFFUSE_BIT_PGI 0x00400000
#define GL_MAT_EMISSION_BIT_PGI 0x00800000
#define GL_MAT_COLOR_INDEXES_BIT_PGI 0x01000000
#define GL_MAT_SHININESS_BIT_PGI 0x02000000
#define GL_MAT_SPECULAR_BIT_PGI 0x04000000
#define GL_NORMAL_BIT_PGI 0x08000000
#define GL_TEXCOORD1_BIT_PGI 0x10000000
#define GL_TEXCOORD2_BIT_PGI 0x20000000
#define GL_TEXCOORD3_BIT_PGI 0x40000000
#define GL_TEXCOORD4_BIT_PGI 0x80000000
#endif

#ifdef GL_REND_screen_coordinates
#define GL_SCREEN_COORDINATES_REND 0x8490
#define GL_INVERTED_SCREEN_W_REND 0x8491
#endif

#ifdef GL_S3_s3tc
#define GL_RGB_S3TC 0x83A0
#define GL_RGB4_S3TC 0x83A1
#define GL_RGBA_S3TC 0x83A2
#define GL_RGBA4_S3TC 0x83A3
#define GL_RGBA_DXT5_S3TC 0x83A4
#define GL_RGBA4_DXT5_S3TC 0x83A5
#endif

#ifdef GL_SGIS_color_range
#define GL_EXTENDED_RANGE_SGIS 0x85A5
#define GL_MIN_RED_SGIS 0x85A6
#define GL_MAX_RED_SGIS 0x85A7
#define GL_MIN_GREEN_SGIS 0x85A8
#define GL_MAX_GREEN_SGIS 0x85A9
#define GL_MIN_BLUE_SGIS 0x85AA
#define GL_MAX_BLUE_SGIS 0x85AB
#define GL_MIN_ALPHA_SGIS 0x85AC
#define GL_MAX_ALPHA_SGIS 0x85AD
#endif

#ifdef GL_SGIS_detail_texture
#define glDetailTexFuncSGIS XGL_FUNCPTR(glDetailTexFuncSGIS)
#define glGetDetailTexFuncSGIS XGL_FUNCPTR(glGetDetailTexFuncSGIS)
#endif

#ifdef GL_SGIS_fog_function
#define glFogFuncSGIS XGL_FUNCPTR(glFogFuncSGIS)
#define glGetFogFuncSGIS XGL_FUNCPTR(glGetFogFuncSGIS)
#endif

#ifdef GL_SGIS_generate_mipmap
#define GL_GENERATE_MIPMAP_SGIS 0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS 0x8192
#endif

#ifdef GL_SGIS_multisample
#define GL_MULTISAMPLE_SGIS 0x809D
#define GL_SAMPLE_ALPHA_TO_MASK_SGIS 0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_SGIS 0x809F
#define GL_SAMPLE_MASK_SGIS 0x80A0
#define GL_1PASS_SGIS 0x80A1
#define GL_2PASS_0_SGIS 0x80A2
#define GL_2PASS_1_SGIS 0x80A3
#define GL_4PASS_0_SGIS 0x80A4
#define GL_4PASS_1_SGIS 0x80A5
#define GL_4PASS_2_SGIS 0x80A6
#define GL_4PASS_3_SGIS 0x80A7
#define GL_SAMPLE_BUFFERS_SGIS 0x80A8
#define GL_SAMPLES_SGIS 0x80A9
#define GL_SAMPLE_MASK_VALUE_SGIS 0x80AA
#define GL_SAMPLE_MASK_INVERT_SGIS 0x80AB
#define GL_SAMPLE_PATTERN_SGIS 0x80AC
#define GL_MULTISAMPLE_BIT_EXT 0x20000000
#define glSampleMaskSGIS XGL_FUNCPTR(glSampleMaskSGIS)
#define glSamplePatternSGIS XGL_FUNCPTR(glSamplePatternSGIS)
#endif

#ifdef GL_SGIS_pixel_texture
#endif

#ifdef GL_SGIS_sharpen_texture
#define glGetSharpenTexFuncSGIS XGL_FUNCPTR(glGetSharpenTexFuncSGIS)
#define glSharpenTexFuncSGIS XGL_FUNCPTR(glSharpenTexFuncSGIS)
#endif

#ifdef GL_SGIS_texture4D
#define glTexImage4DSGIS XGL_FUNCPTR(glTexImage4DSGIS)
#define glTexSubImage4DSGIS XGL_FUNCPTR(glTexSubImage4DSGIS)
#endif

#ifdef GL_SGIS_texture_border_clamp
#define GL_CLAMP_TO_BORDER_SGIS 0x812D
#endif

#ifdef GL_SGIS_texture_edge_clamp
#define GL_CLAMP_TO_EDGE_SGIS 0x812F
#endif

#ifdef GL_SGIS_texture_filter4
#define glGetTexFilterFuncSGIS XGL_FUNCPTR(glGetTexFilterFuncSGIS)
#define glTexFilterFuncSGIS XGL_FUNCPTR(glTexFilterFuncSGIS)
#endif

#ifdef GL_SGIS_texture_lod
#define GL_TEXTURE_MIN_LOD_SGIS 0x813A
#define GL_TEXTURE_MAX_LOD_SGIS 0x813B
#define GL_TEXTURE_BASE_LEVEL_SGIS 0x813C
#define GL_TEXTURE_MAX_LEVEL_SGIS 0x813D
#endif

#ifdef GL_SGIS_texture_select
#endif

#ifdef GL_SGIX_async
#define GL_ASYNC_MARKER_SGIX 0x8329
#define glAsyncMarkerSGIX XGL_FUNCPTR(glAsyncMarkerSGIX)
#define glDeleteAsyncMarkersSGIX XGL_FUNCPTR(glDeleteAsyncMarkersSGIX)
#define glFinishAsyncSGIX XGL_FUNCPTR(glFinishAsyncSGIX)
#define glGenAsyncMarkersSGIX XGL_FUNCPTR(glGenAsyncMarkersSGIX)
#define glIsAsyncMarkerSGIX XGL_FUNCPTR(glIsAsyncMarkerSGIX)
#define glPollAsyncSGIX XGL_FUNCPTR(glPollAsyncSGIX)
#endif

#ifdef GL_SGIX_async_histogram
#define GL_ASYNC_HISTOGRAM_SGIX 0x832C
#define GL_MAX_ASYNC_HISTOGRAM_SGIX 0x832D
#endif

#ifdef GL_SGIX_async_pixel
#define GL_ASYNC_TEX_IMAGE_SGIX 0x835C
#define GL_ASYNC_DRAW_PIXELS_SGIX 0x835D
#define GL_ASYNC_READ_PIXELS_SGIX 0x835E
#define GL_MAX_ASYNC_TEX_IMAGE_SGIX 0x835F
#define GL_MAX_ASYNC_DRAW_PIXELS_SGIX 0x8360
#define GL_MAX_ASYNC_READ_PIXELS_SGIX 0x8361
#endif

#ifdef GL_SGIX_blend_alpha_minmax
#define GL_ALPHA_MIN_SGIX 0x8320
#define GL_ALPHA_MAX_SGIX 0x8321
#endif

#ifdef GL_SGIX_clipmap
#endif

#ifdef GL_SGIX_depth_texture
#define GL_DEPTH_COMPONENT16_SGIX 0x81A5
#define GL_DEPTH_COMPONENT24_SGIX 0x81A6
#define GL_DEPTH_COMPONENT32_SGIX 0x81A7
#endif

#ifdef GL_SGIX_flush_raster
#define glFlushRasterSGIX XGL_FUNCPTR(glFlushRasterSGIX)
#endif

#ifdef GL_SGIX_fog_offset
#define GL_FOG_OFFSET_SGIX 0x8198
#define GL_FOG_OFFSET_VALUE_SGIX 0x8199
#endif

#ifdef GL_SGIX_fog_texture
#define GL_TEXTURE_FOG_SGIX 0
#define GL_FOG_PATCHY_FACTOR_SGIX 0
#define GL_FRAGMENT_FOG_SGIX 0
#define glTextureFogSGIX XGL_FUNCPTR(glTextureFogSGIX)
#endif

#ifdef GL_SGIX_fragment_specular_lighting
#define glFragmentColorMaterialSGIX XGL_FUNCPTR(glFragmentColorMaterialSGIX)
#define glFragmentLightModelfSGIX XGL_FUNCPTR(glFragmentLightModelfSGIX)
#define glFragmentLightModelfvSGIX XGL_FUNCPTR(glFragmentLightModelfvSGIX)
#define glFragmentLightModeliSGIX XGL_FUNCPTR(glFragmentLightModeliSGIX)
#define glFragmentLightModelivSGIX XGL_FUNCPTR(glFragmentLightModelivSGIX)
#define glFragmentLightfSGIX XGL_FUNCPTR(glFragmentLightfSGIX)
#define glFragmentLightfvSGIX XGL_FUNCPTR(glFragmentLightfvSGIX)
#define glFragmentLightiSGIX XGL_FUNCPTR(glFragmentLightiSGIX)
#define glFragmentLightivSGIX XGL_FUNCPTR(glFragmentLightivSGIX)
#define glFragmentMaterialfSGIX XGL_FUNCPTR(glFragmentMaterialfSGIX)
#define glFragmentMaterialfvSGIX XGL_FUNCPTR(glFragmentMaterialfvSGIX)
#define glFragmentMaterialiSGIX XGL_FUNCPTR(glFragmentMaterialiSGIX)
#define glFragmentMaterialivSGIX XGL_FUNCPTR(glFragmentMaterialivSGIX)
#define glGetFragmentLightfvSGIX XGL_FUNCPTR(glGetFragmentLightfvSGIX)
#define glGetFragmentLightivSGIX XGL_FUNCPTR(glGetFragmentLightivSGIX)
#define glGetFragmentMaterialfvSGIX XGL_FUNCPTR(glGetFragmentMaterialfvSGIX)
#define glGetFragmentMaterialivSGIX XGL_FUNCPTR(glGetFragmentMaterialivSGIX)
#endif

#ifdef GL_SGIX_framezoom
#define glFrameZoomSGIX XGL_FUNCPTR(glFrameZoomSGIX)
#endif

#ifdef GL_SGIX_interlace
#define GL_INTERLACE_SGIX 0x8094
#endif

#ifdef GL_SGIX_ir_instrument1
#endif

#ifdef GL_SGIX_list_priority
#endif

#ifdef GL_SGIX_pixel_texture
#define glPixelTexGenSGIX XGL_FUNCPTR(glPixelTexGenSGIX)
#endif

#ifdef GL_SGIX_pixel_texture_bits
#endif

#ifdef GL_SGIX_reference_plane
#define glReferencePlaneSGIX XGL_FUNCPTR(glReferencePlaneSGIX)
#endif

#ifdef GL_SGIX_resample
#define GL_PACK_RESAMPLE_SGIX 0x842E
#define GL_UNPACK_RESAMPLE_SGIX 0x842F
#define GL_RESAMPLE_DECIMATE_SGIX 0x8430
#define GL_RESAMPLE_REPLICATE_SGIX 0x8433
#define GL_RESAMPLE_ZERO_FILL_SGIX 0x8434
#endif

#ifdef GL_SGIX_shadow
#define GL_TEXTURE_COMPARE_SGIX 0x819A
#define GL_TEXTURE_COMPARE_OPERATOR_SGIX 0x819B
#define GL_TEXTURE_LEQUAL_R_SGIX 0x819C
#define GL_TEXTURE_GEQUAL_R_SGIX 0x819D
#endif

#ifdef GL_SGIX_shadow_ambient
#define GL_SHADOW_AMBIENT_SGIX 0x80BF
#endif

#ifdef GL_SGIX_sprite
#define glSpriteParameterfSGIX XGL_FUNCPTR(glSpriteParameterfSGIX)
#define glSpriteParameterfvSGIX XGL_FUNCPTR(glSpriteParameterfvSGIX)
#define glSpriteParameteriSGIX XGL_FUNCPTR(glSpriteParameteriSGIX)
#define glSpriteParameterivSGIX XGL_FUNCPTR(glSpriteParameterivSGIX)
#endif

#ifdef GL_SGIX_tag_sample_buffer
#define glTagSampleBufferSGIX XGL_FUNCPTR(glTagSampleBufferSGIX)
#endif

#ifdef GL_SGIX_texture_add_env
#endif

#ifdef GL_SGIX_texture_coordinate_clamp
#define GL_TEXTURE_MAX_CLAMP_S_SGIX 0x8369
#define GL_TEXTURE_MAX_CLAMP_T_SGIX 0x836A
#define GL_TEXTURE_MAX_CLAMP_R_SGIX 0x836B
#endif

#ifdef GL_SGIX_texture_lod_bias
#endif

#ifdef GL_SGIX_texture_multi_buffer
#define GL_TEXTURE_MULTI_BUFFER_HINT_SGIX 0x812E
#endif

#ifdef GL_SGIX_texture_range
#define GL_RGB_SIGNED_SGIX 0x85E0
#define GL_RGBA_SIGNED_SGIX 0x85E1
#define GL_ALPHA_SIGNED_SGIX 0x85E2
#define GL_LUMINANCE_SIGNED_SGIX 0x85E3
#define GL_INTENSITY_SIGNED_SGIX 0x85E4
#define GL_LUMINANCE_ALPHA_SIGNED_SGIX 0x85E5
#define GL_RGB16_SIGNED_SGIX 0x85E6
#define GL_RGBA16_SIGNED_SGIX 0x85E7
#define GL_ALPHA16_SIGNED_SGIX 0x85E8
#define GL_LUMINANCE16_SIGNED_SGIX 0x85E9
#define GL_INTENSITY16_SIGNED_SGIX 0x85EA
#define GL_LUMINANCE16_ALPHA16_SIGNED_SGIX 0x85EB
#define GL_RGB_EXTENDED_RANGE_SGIX 0x85EC
#define GL_RGBA_EXTENDED_RANGE_SGIX 0x85ED
#define GL_ALPHA_EXTENDED_RANGE_SGIX 0x85EE
#define GL_LUMINANCE_EXTENDED_RANGE_SGIX 0x85EF
#define GL_INTENSITY_EXTENDED_RANGE_SGIX 0x85F0
#define GL_LUMINANCE_ALPHA_EXTENDED_RANGE_SGIX 0x85F1
#define GL_RGB16_EXTENDED_RANGE_SGIX 0x85F2
#define GL_RGBA16_EXTENDED_RANGE_SGIX 0x85F3
#define GL_ALPHA16_EXTENDED_RANGE_SGIX 0x85F4
#define GL_LUMINANCE16_EXTENDED_RANGE_SGIX 0x85F5
#define GL_INTENSITY16_EXTENDED_RANGE_SGIX 0x85F6
#define GL_LUMINANCE16_ALPHA16_EXTENDED_RANGE_SGIX 0x85F7
#define GL_MIN_LUMINANCE_SGIS 0x85F8
#define GL_MAX_LUMINANCE_SGIS 0x85F9
#define GL_MIN_INTENSITY_SGIS 0x85FA
#define GL_MAX_INTENSITY_SGIS 0x85FB
#endif

#ifdef GL_SGIX_texture_scale_bias
#define GL_POST_TEXTURE_FILTER_BIAS_SGIX 0x8179
#define GL_POST_TEXTURE_FILTER_SCALE_SGIX 0x817A
#define GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX 0x817B
#define GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX 0x817C
#endif

#ifdef GL_SGIX_vertex_preclip
#define GL_VERTEX_PRECLIP_SGIX 0x83EE
#define GL_VERTEX_PRECLIP_HINT_SGIX 0x83EF
#endif

#ifdef GL_SGIX_vertex_preclip_hint
#define GL_VERTEX_PRECLIP_SGIX 0x83EE
#define GL_VERTEX_PRECLIP_HINT_SGIX 0x83EF
#endif

#ifdef GL_SGIX_ycrcb
#endif

#ifdef GL_SGI_color_matrix
#define GL_COLOR_MATRIX_SGI 0x80B1
#define GL_COLOR_MATRIX_STACK_DEPTH_SGI 0x80B2
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI 0x80B3
#define GL_POST_COLOR_MATRIX_RED_SCALE_SGI 0x80B4
#define GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI 0x80B5
#define GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI 0x80B6
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI 0x80B7
#define GL_POST_COLOR_MATRIX_RED_BIAS_SGI 0x80B8
#define GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI 0x80B9
#define GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI 0x80BA
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI 0x80BB
#endif

#ifdef GL_SGI_color_table
#define GL_COLOR_TABLE_SGI 0x80D0
#define GL_POST_CONVOLUTION_COLOR_TABLE_SGI 0x80D1
#define GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI 0x80D2
#define GL_PROXY_COLOR_TABLE_SGI 0x80D3
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI 0x80D4
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI 0x80D5
#define GL_COLOR_TABLE_SCALE_SGI 0x80D6
#define GL_COLOR_TABLE_BIAS_SGI 0x80D7
#define GL_COLOR_TABLE_FORMAT_SGI 0x80D8
#define GL_COLOR_TABLE_WIDTH_SGI 0x80D9
#define GL_COLOR_TABLE_RED_SIZE_SGI 0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE_SGI 0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE_SGI 0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE_SGI 0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE_SGI 0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE_SGI 0x80DF
#define glColorTableParameterfvSGI XGL_FUNCPTR(glColorTableParameterfvSGI)
#define glColorTableParameterivSGI XGL_FUNCPTR(glColorTableParameterivSGI)
#define glColorTableSGI XGL_FUNCPTR(glColorTableSGI)
#define glCopyColorTableSGI XGL_FUNCPTR(glCopyColorTableSGI)
#define glGetColorTableParameterfvSGI XGL_FUNCPTR(glGetColorTableParameterfvSGI)
#define glGetColorTableParameterivSGI XGL_FUNCPTR(glGetColorTableParameterivSGI)
#define glGetColorTableSGI XGL_FUNCPTR(glGetColorTableSGI)
#endif

#ifdef GL_SGI_texture_color_table
#define GL_TEXTURE_COLOR_TABLE_SGI 0x80BC
#define GL_PROXY_TEXTURE_COLOR_TABLE_SGI 0x80BD
#endif

#ifdef GL_SUNX_constant_data
#define GL_UNPACK_CONSTANT_DATA_SUNX 0x81D5
#define GL_TEXTURE_CONSTANT_DATA_SUNX 0x81D6
#define glFinishTextureSUNX XGL_FUNCPTR(glFinishTextureSUNX)
#endif

#ifdef GL_SUN_convolution_border_modes
#define GL_WRAP_BORDER_SUN 0x81D4
#endif

#ifdef GL_SUN_global_alpha
#define GL_GLOBAL_ALPHA_SUN 0x81D9
#define GL_GLOBAL_ALPHA_FACTOR_SUN 0x81DA
#define glGlobalAlphaFactorbSUN XGL_FUNCPTR(glGlobalAlphaFactorbSUN)
#define glGlobalAlphaFactordSUN XGL_FUNCPTR(glGlobalAlphaFactordSUN)
#define glGlobalAlphaFactorfSUN XGL_FUNCPTR(glGlobalAlphaFactorfSUN)
#define glGlobalAlphaFactoriSUN XGL_FUNCPTR(glGlobalAlphaFactoriSUN)
#define glGlobalAlphaFactorsSUN XGL_FUNCPTR(glGlobalAlphaFactorsSUN)
#define glGlobalAlphaFactorubSUN XGL_FUNCPTR(glGlobalAlphaFactorubSUN)
#define glGlobalAlphaFactoruiSUN XGL_FUNCPTR(glGlobalAlphaFactoruiSUN)
#define glGlobalAlphaFactorusSUN XGL_FUNCPTR(glGlobalAlphaFactorusSUN)
#endif

#ifdef GL_SUN_mesh_array
#define GL_QUAD_MESH_SUN 0x8614
#define GL_TRIANGLE_MESH_SUN 0x8615
#endif

#ifdef GL_SUN_read_video_pixels
#define glReadVideoPixelsSUN XGL_FUNCPTR(glReadVideoPixelsSUN)
#endif

#ifdef GL_SUN_slice_accum
#define GL_SLICE_ACCUM_SUN 0x85CC
#endif

#ifdef GL_SUN_triangle_list
#define GL_RESTART_SUN 0x01
#define GL_REPLACE_MIDDLE_SUN 0x02
#define GL_REPLACE_OLDEST_SUN 0x03
#define GL_TRIANGLE_LIST_SUN 0x81D7
#define GL_REPLACEMENT_CODE_SUN 0x81D8
#define GL_REPLACEMENT_CODE_ARRAY_SUN 0x85C0
#define GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN 0x85C1
#define GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN 0x85C2
#define GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN 0x85C3
#define GL_R1UI_V3F_SUN 0x85C4
#define GL_R1UI_C4UB_V3F_SUN 0x85C5
#define GL_R1UI_C3F_V3F_SUN 0x85C6
#define GL_R1UI_N3F_V3F_SUN 0x85C7
#define GL_R1UI_C4F_N3F_V3F_SUN 0x85C8
#define GL_R1UI_T2F_V3F_SUN 0x85C9
#define GL_R1UI_T2F_N3F_V3F_SUN 0x85CA
#define GL_R1UI_T2F_C4F_N3F_V3F_SUN 0x85CB
#define glReplacementCodePointerSUN XGL_FUNCPTR(glReplacementCodePointerSUN)
#define glReplacementCodeubSUN XGL_FUNCPTR(glReplacementCodeubSUN)
#define glReplacementCodeubvSUN XGL_FUNCPTR(glReplacementCodeubvSUN)
#define glReplacementCodeuiSUN XGL_FUNCPTR(glReplacementCodeuiSUN)
#define glReplacementCodeuivSUN XGL_FUNCPTR(glReplacementCodeuivSUN)
#define glReplacementCodeusSUN XGL_FUNCPTR(glReplacementCodeusSUN)
#define glReplacementCodeusvSUN XGL_FUNCPTR(glReplacementCodeusvSUN)
#endif

#ifdef GL_SUN_vertex
#define glColor3fVertex3fSUN XGL_FUNCPTR(glColor3fVertex3fSUN)
#define glColor3fVertex3fvSUN XGL_FUNCPTR(glColor3fVertex3fvSUN)
#define glColor4fNormal3fVertex3fSUN XGL_FUNCPTR(glColor4fNormal3fVertex3fSUN)
#define glColor4fNormal3fVertex3fvSUN XGL_FUNCPTR(glColor4fNormal3fVertex3fvSUN)
#define glColor4ubVertex2fSUN XGL_FUNCPTR(glColor4ubVertex2fSUN)
#define glColor4ubVertex2fvSUN XGL_FUNCPTR(glColor4ubVertex2fvSUN)
#define glColor4ubVertex3fSUN XGL_FUNCPTR(glColor4ubVertex3fSUN)
#define glColor4ubVertex3fvSUN XGL_FUNCPTR(glColor4ubVertex3fvSUN)
#define glNormal3fVertex3fSUN XGL_FUNCPTR(glNormal3fVertex3fSUN)
#define glNormal3fVertex3fvSUN XGL_FUNCPTR(glNormal3fVertex3fvSUN)
#define glReplacementCodeuiColor3fVertex3fSUN XGL_FUNCPTR(glReplacementCodeuiColor3fVertex3fSUN)
#define glReplacementCodeuiColor3fVertex3fvSUN XGL_FUNCPTR(glReplacementCodeuiColor3fVertex3fvSUN)
#define glReplacementCodeuiColor4fNormal3fVertex3fSUN XGL_FUNCPTR(glReplacementCodeuiColor4fNormal3fVertex3fSUN)
#define glReplacementCodeuiColor4fNormal3fVertex3fvSUN XGL_FUNCPTR(glReplacementCodeuiColor4fNormal3fVertex3fvSUN)
#define glReplacementCodeuiColor4ubVertex3fSUN XGL_FUNCPTR(glReplacementCodeuiColor4ubVertex3fSUN)
#define glReplacementCodeuiColor4ubVertex3fvSUN XGL_FUNCPTR(glReplacementCodeuiColor4ubVertex3fvSUN)
#define glReplacementCodeuiNormal3fVertex3fSUN XGL_FUNCPTR(glReplacementCodeuiNormal3fVertex3fSUN)
#define glReplacementCodeuiNormal3fVertex3fvSUN XGL_FUNCPTR(glReplacementCodeuiNormal3fVertex3fvSUN)
#define glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN XGL_FUNCPTR(glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN)
#define glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN XGL_FUNCPTR(glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN)
#define glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN XGL_FUNCPTR(glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN)
#define glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN XGL_FUNCPTR(glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN)
#define glReplacementCodeuiTexCoord2fVertex3fSUN XGL_FUNCPTR(glReplacementCodeuiTexCoord2fVertex3fSUN)
#define glReplacementCodeuiTexCoord2fVertex3fvSUN XGL_FUNCPTR(glReplacementCodeuiTexCoord2fVertex3fvSUN)
#define glReplacementCodeuiVertex3fSUN XGL_FUNCPTR(glReplacementCodeuiVertex3fSUN)
#define glReplacementCodeuiVertex3fvSUN XGL_FUNCPTR(glReplacementCodeuiVertex3fvSUN)
#define glTexCoord2fColor3fVertex3fSUN XGL_FUNCPTR(glTexCoord2fColor3fVertex3fSUN)
#define glTexCoord2fColor3fVertex3fvSUN XGL_FUNCPTR(glTexCoord2fColor3fVertex3fvSUN)
#define glTexCoord2fColor4fNormal3fVertex3fSUN XGL_FUNCPTR(glTexCoord2fColor4fNormal3fVertex3fSUN)
#define glTexCoord2fColor4fNormal3fVertex3fvSUN XGL_FUNCPTR(glTexCoord2fColor4fNormal3fVertex3fvSUN)
#define glTexCoord2fColor4ubVertex3fSUN XGL_FUNCPTR(glTexCoord2fColor4ubVertex3fSUN)
#define glTexCoord2fColor4ubVertex3fvSUN XGL_FUNCPTR(glTexCoord2fColor4ubVertex3fvSUN)
#define glTexCoord2fNormal3fVertex3fSUN XGL_FUNCPTR(glTexCoord2fNormal3fVertex3fSUN)
#define glTexCoord2fNormal3fVertex3fvSUN XGL_FUNCPTR(glTexCoord2fNormal3fVertex3fvSUN)
#define glTexCoord2fVertex3fSUN XGL_FUNCPTR(glTexCoord2fVertex3fSUN)
#define glTexCoord2fVertex3fvSUN XGL_FUNCPTR(glTexCoord2fVertex3fvSUN)
#define glTexCoord4fColor4fNormal3fVertex4fSUN XGL_FUNCPTR(glTexCoord4fColor4fNormal3fVertex4fSUN)
#define glTexCoord4fColor4fNormal3fVertex4fvSUN XGL_FUNCPTR(glTexCoord4fColor4fNormal3fVertex4fvSUN)
#define glTexCoord4fVertex4fSUN XGL_FUNCPTR(glTexCoord4fVertex4fSUN)
#define glTexCoord4fVertex4fvSUN XGL_FUNCPTR(glTexCoord4fVertex4fvSUN)
#endif

#ifdef GL_WIN_phong_shading
#define GL_PHONG_WIN 0x80EA
#define GL_PHONG_HINT_WIN 0x80EB
#endif

#ifdef GL_WIN_specular_fog
#define GL_FOG_SPECULAR_TEXTURE_WIN 0x80EC
#endif

#ifdef GL_WIN_swap_hint
#define glAddSwapHintRectWIN XGL_FUNCPTR(glAddSwapHintRectWIN)
#endif


