/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_OGL_ES2 && !SDL_RENDER_DISABLED

#include "SDL_video.h"
#include "SDL_opengles2.h"
#include "SDL_shaders_gles2.h"
#include "SDL_stdinc.h"

/*************************************************************************************************
 * Vertex/fragment shader source                                                                 *
 *************************************************************************************************/

static const Uint8 GLES2_VertexSrc_Default_[] = " \
    uniform mat4 u_projection; \
    attribute vec2 a_position; \
    attribute vec2 a_texCoord; \
    attribute float a_angle; \
    attribute vec2 a_center; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        float angle = radians(a_angle); \
        float c = cos(angle); \
        float s = sin(angle); \
        mat2 rotationMatrix = mat2(c, -s, s, c); \
        vec2 position = rotationMatrix * (a_position - a_center) + a_center; \
        v_texCoord = a_texCoord; \
        gl_Position = u_projection * vec4(position, 0.0, 1.0);\
        gl_PointSize = 1.0; \
    } \
";

static const Uint8 GLES2_FragmentSrc_SolidSrc_[] = " \
    precision mediump float; \
    uniform vec4 u_color; \
    \
    void main() \
    { \
        gl_FragColor = u_color; \
    } \
";

static const Uint8 GLES2_FragmentSrc_TextureABGRSrc_[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_modulation; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        gl_FragColor = texture2D(u_texture, v_texCoord); \
        gl_FragColor *= u_modulation; \
    } \
";

/* ARGB to ABGR conversion */
static const Uint8 GLES2_FragmentSrc_TextureARGBSrc_[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_modulation; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        vec4 abgr = texture2D(u_texture, v_texCoord); \
        gl_FragColor = abgr; \
        gl_FragColor.r = abgr.b; \
        gl_FragColor.b = abgr.r; \
        gl_FragColor *= u_modulation; \
    } \
";

/* RGB to ABGR conversion */
static const Uint8 GLES2_FragmentSrc_TextureRGBSrc_[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_modulation; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        vec4 abgr = texture2D(u_texture, v_texCoord); \
        gl_FragColor = abgr; \
        gl_FragColor.r = abgr.b; \
        gl_FragColor.b = abgr.r; \
        gl_FragColor.a = 1.0; \
        gl_FragColor *= u_modulation; \
    } \
";

/* BGR to ABGR conversion */
static const Uint8 GLES2_FragmentSrc_TextureBGRSrc_[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_modulation; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        vec4 abgr = texture2D(u_texture, v_texCoord); \
        gl_FragColor = abgr; \
        gl_FragColor.a = 1.0; \
        gl_FragColor *= u_modulation; \
    } \
";

static const GLES2_ShaderInstance GLES2_VertexSrc_Default = {
    GL_VERTEX_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_VertexSrc_Default_),
    GLES2_VertexSrc_Default_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_SolidSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_SolidSrc_),
    GLES2_FragmentSrc_SolidSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureABGRSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureABGRSrc_),
    GLES2_FragmentSrc_TextureABGRSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureARGBSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureARGBSrc_),
    GLES2_FragmentSrc_TextureARGBSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureRGBSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureRGBSrc_),
    GLES2_FragmentSrc_TextureRGBSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureBGRSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureBGRSrc_),
    GLES2_FragmentSrc_TextureBGRSrc_
};

/*************************************************************************************************
 * Vertex/fragment shader binaries (NVIDIA Tegra 1/2)                                            *
 *************************************************************************************************/

#if GLES2_INCLUDE_NVIDIA_SHADERS

#define GL_NVIDIA_PLATFORM_BINARY_NV 0x890B

static const Uint8 GLES2_VertexTegra_Default_[] = {
    243, 193, 1, 142, 31, 109, 131, 38, 6, 0, 1, 0, 5, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 73, 0,
    0, 0, 46, 0, 0, 0, 48, 0, 0, 0, 2, 0, 0, 0, 85, 0, 0, 0, 2, 0, 0, 0, 24, 0, 0, 0, 3, 0, 0, 0,
    91, 0, 0, 0, 1, 0, 0, 0, 16, 0, 0, 0, 5, 0, 0, 0, 95, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 95, 0, 0, 0, 1, 0, 0, 0, 28, 0, 0, 0,
    13, 0, 0, 0, 102, 0, 0, 0, 2, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 16, 0, 0, 0, 104, 0, 0, 0, 1, 0, 0, 0, 32, 0, 0, 0, 17, 0, 0, 0, 112, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 112, 0, 0, 0, 80, 0, 0, 0, 80, 0, 0, 0, 19, 0, 0, 0, 132, 0,
    0, 0, 104, 0, 0, 0, 104, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 109, 97, 110, 70, 73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 97,
    95, 112, 111, 115, 105, 116, 105, 111, 110, 0, 97, 95, 116, 101, 120, 67, 111, 111, 114, 100,
    0, 118, 95, 116, 101, 120, 67, 111, 111, 114, 100, 0, 117, 95, 112, 114, 111, 106, 101, 99,
    116, 105, 111, 110, 0, 0, 0, 0, 0, 0, 0, 82, 139, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 80, 139, 0,
    0, 1, 0, 0, 0, 22, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 33, 0, 0, 0, 92, 139, 0, 0,
    1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 240, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 64, 0, 0, 0, 80, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 193, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 128, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 66, 24, 0, 6, 34, 108, 28,
    0, 0, 42, 16, 128, 0, 195, 192, 6, 129, 252, 255, 65, 96, 108, 28, 0, 0, 0, 0, 0, 1, 195, 192,
    6, 1, 252, 255, 33, 96, 108, 156, 31, 64, 8, 1, 64, 0, 131, 192, 6, 1, 156, 159, 65, 96, 108,
    28, 0, 0, 85, 32, 0, 1, 195, 192, 6, 1, 252, 255, 33, 96, 108, 156, 31, 64, 0, 64, 64, 0, 131,
    192, 134, 1, 152, 31, 65, 96, 108, 156, 31, 64, 127, 48, 0, 1, 195, 192, 6, 129, 129, 255, 33,
    96
};

static const Uint8 GLES2_FragmentTegra_None_SolidSrc_[] = {
    155, 191, 159, 1, 47, 109, 131, 38, 6, 0, 1, 0, 5, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 73, 0,
    0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 2, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 75,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0,
    75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 75, 0, 0, 0, 1, 0, 0, 0, 28, 0, 0, 0, 13, 0,
    0, 0, 82, 0, 0, 0, 2, 0, 0, 0, 8, 0, 0, 0, 14, 0, 0, 0, 84, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    22, 0, 0, 0, 84, 0, 0, 0, 8, 0, 0, 0, 32, 0, 0, 0, 23, 0, 0, 0, 92, 0, 0, 0, 1, 0, 0, 0, 4,
    0, 0, 0, 15, 0, 0, 0, 93, 0, 0, 0, 1, 0, 0, 0, 80, 0, 0, 0, 17, 0, 0, 0, 113, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 113, 0, 0,
    0, 108, 0, 0, 0, 108, 0, 0, 0, 20, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0,
    0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 109, 97, 110, 70, 73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 117, 95, 99, 111, 108, 111, 114, 0, 0, 0, 0, 0, 82, 139, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 241, 0, 0, 0, 240, 0, 0,
    0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 21, 32, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 20, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65, 82, 50, 48, 45, 66, 73, 78, 1,
    0, 0, 0, 1, 0, 0, 0, 1, 0, 65, 37, 0, 0, 0, 0, 1, 0, 0, 21, 0, 0, 0, 0, 1, 0, 1, 38, 0, 0, 0,
    0, 1, 0, 1, 39, 0, 0, 0, 0, 1, 0, 1, 40, 1, 0, 0, 0, 8, 0, 4, 40, 0, 40, 0, 0, 0, 242, 65, 63,
    192, 200, 0, 0, 0, 242, 65, 63, 128, 168, 0, 0, 0, 242, 65, 63, 64, 72, 0, 0, 0, 242, 65, 63,
    1, 0, 6, 40, 0, 0, 0, 0, 1, 0, 1, 41, 5, 0, 2, 0
};

static const Uint8 GLES2_FragmentTegra_Alpha_SolidSrc_[] = {
    169, 153, 195, 28, 47, 109, 131, 38, 6, 0, 1, 0, 5, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 73, 0,
    0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 2, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 75,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0,
    75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 75, 0, 0, 0, 1, 0, 0, 0, 28, 0, 0, 0, 13, 0,
    0, 0, 82, 0, 0, 0, 2, 0, 0, 0, 8, 0, 0, 0, 14, 0, 0, 0, 84, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    22, 0, 0, 0, 84, 0, 0, 0, 8, 0, 0, 0, 32, 0, 0, 0, 23, 0, 0, 0, 92, 0, 0, 0, 1, 0, 0, 0, 4,
    0, 0, 0, 15, 0, 0, 0, 93, 0, 0, 0, 1, 0, 0, 0, 80, 0, 0, 0, 17, 0, 0, 0, 113, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 113, 0, 0,
    0, 220, 0, 0, 0, 220, 0, 0, 0, 20, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0,
    0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 109, 97, 110, 70, 73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 117, 95, 99, 111, 108, 111, 114, 0, 0, 0, 0, 0, 82, 139, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 48, 0, 0, 0, 0, 0, 0, 118, 118, 17, 241, 0, 0, 0, 240, 0,
    0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 21, 32, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65, 82, 50, 48, 45, 66, 73, 78,
    1, 0, 0, 0, 3, 0, 0, 0, 3, 0, 65, 37, 8, 0, 129, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 21, 0,
    0, 0, 0, 3, 0, 1, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 1, 39, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 3, 0, 1, 40, 1, 0, 0, 0, 5, 0, 0, 0, 9, 0, 0, 0, 24, 0, 4, 40, 232, 231, 15,
    0, 0, 242, 65, 62, 194, 72, 1, 0, 0, 250, 65, 63, 194, 40, 1, 0, 0, 250, 65, 63, 192, 168, 1,
    0, 0, 242, 1, 64, 192, 168, 1, 0, 0, 242, 1, 68, 168, 32, 0, 0, 0, 50, 64, 0, 192, 168, 15,
    0, 0, 242, 1, 66, 168, 64, 0, 16, 0, 242, 65, 1, 232, 231, 15, 0, 0, 242, 65, 62, 168, 160,
    0, 0, 0, 50, 64, 2, 104, 192, 0, 0, 36, 48, 66, 4, 232, 231, 15, 0, 0, 242, 65, 62, 3, 0, 6,
    40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 1, 41, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 2, 0
};

static const Uint8 GLES2_FragmentTegra_Additive_SolidSrc_[] = {
    59, 71, 42, 17, 47, 109, 131, 38, 6, 0, 1, 0, 5, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 73, 0, 0,
    0, 8, 0, 0, 0, 8, 0, 0, 0, 2, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 75,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0,
    75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 75, 0, 0, 0, 1, 0, 0, 0, 28, 0, 0, 0, 13, 0,
    0, 0, 82, 0, 0, 0, 2, 0, 0, 0, 8, 0, 0, 0, 14, 0, 0, 0, 84, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    22, 0, 0, 0, 84, 0, 0, 0, 8, 0, 0, 0, 32, 0, 0, 0, 23, 0, 0, 0, 92, 0, 0, 0, 1, 0, 0, 0, 4,
    0, 0, 0, 15, 0, 0, 0, 93, 0, 0, 0, 1, 0, 0, 0, 80, 0, 0, 0, 17, 0, 0, 0, 113, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 113, 0, 0,
    0, 108, 0, 0, 0, 108, 0, 0, 0, 20, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0,
    0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 109, 97, 110, 70, 73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 117, 95, 99, 111, 108, 111, 114, 0, 0, 0, 0, 0, 82, 139, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 48, 0, 0, 0, 0, 0, 0, 22, 22, 17, 241, 0, 0, 0, 240, 0,
    0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 21, 32, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65, 82, 50, 48, 45, 66, 73, 78,
    1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 65, 37, 8, 0, 129, 0, 1, 0, 0, 21, 0, 0, 0, 0, 1, 0, 1, 38, 0,
    0, 0, 0, 1, 0, 1, 39, 0, 0, 0, 0, 1, 0, 1, 40, 1, 0, 0, 0, 8, 0, 4, 40, 192, 200, 0, 0, 0, 26,
    0, 70, 192, 40, 0, 0, 0, 2, 0, 64, 192, 72, 0, 0, 0, 10, 0, 66, 192, 168, 0, 0, 0, 18, 0, 68,
    1, 0, 6, 40, 0, 0, 0, 0, 1, 0, 1, 41, 5, 0, 2, 0
};

static const Uint8 GLES2_FragmentTegra_Modulated_SolidSrc_[] = {
    37, 191, 49, 17, 47, 109, 131, 38, 6, 0, 1, 0, 5, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 73, 0, 0,
    0, 8, 0, 0, 0, 8, 0, 0, 0, 2, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 75,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0,
    75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 75, 0, 0, 0, 1, 0, 0, 0, 28, 0, 0, 0, 13, 0,
    0, 0, 82, 0, 0, 0, 2, 0, 0, 0, 8, 0, 0, 0, 14, 0, 0, 0, 84, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    22, 0, 0, 0, 84, 0, 0, 0, 8, 0, 0, 0, 32, 0, 0, 0, 23, 0, 0, 0, 92, 0, 0, 0, 1, 0, 0, 0, 4,
    0, 0, 0, 15, 0, 0, 0, 93, 0, 0, 0, 1, 0, 0, 0, 80, 0, 0, 0, 17, 0, 0, 0, 113, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 113, 0, 0,
    0, 108, 0, 0, 0, 108, 0, 0, 0, 20, 0, 0, 0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0,
    0, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 109, 97, 110, 70, 73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 117, 95, 99, 111, 108, 111, 114, 0, 0, 0, 0, 0, 82, 139, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 48, 0, 0, 0, 0, 0, 0, 32, 32, 17, 241, 0, 0, 0, 240, 0,
    0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 21, 32, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65, 82, 50, 48, 45, 66, 73, 78,
    1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 65, 37, 8, 0, 129, 0, 1, 0, 0, 21, 0, 0, 0, 0, 1, 0, 1, 38, 0,
    0, 0, 0, 1, 0, 1, 39, 0, 0, 0, 0, 1, 0, 1, 40, 1, 0, 0, 0, 8, 0, 4, 40, 104, 192, 0, 0, 0, 242,
    1, 70, 8, 32, 0, 0, 0, 242, 1, 64, 40, 64, 0, 0, 0, 242, 1, 66, 72, 160, 0, 0, 0, 242, 1, 68,
    1, 0, 6, 40, 0, 0, 0, 0, 1, 0, 1, 41, 5, 0, 2, 0
};

static const Uint8 GLES2_FragmentTegra_None_TextureSrc_[] = {
    220, 217, 41, 211, 47, 109, 131, 38, 6, 0, 1, 0, 5, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 73, 0,
    0, 0, 34, 0, 0, 0, 36, 0, 0, 0, 2, 0, 0, 0, 82, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0,
    82, 0, 0, 0, 1, 0, 0, 0, 20, 0, 0, 0, 6, 0, 0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0,
    0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 87, 0, 0, 0, 2, 0, 0, 0, 56, 0, 0, 0,
    13, 0, 0, 0, 101, 0, 0, 0, 4, 0, 0, 0, 16, 0, 0, 0, 14, 0, 0, 0, 105, 0, 0, 0, 1, 0, 0, 0, 4,
    0, 0, 0, 22, 0, 0, 0, 106, 0, 0, 0, 8, 0, 0, 0, 32, 0, 0, 0, 23, 0, 0, 0, 114, 0, 0, 0, 1, 0,
    0, 0, 4, 0, 0, 0, 15, 0, 0, 0, 115, 0, 0, 0, 1, 0, 0, 0, 80, 0, 0, 0, 17, 0, 0, 0, 135, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 135,
    0, 0, 0, 120, 0, 0, 0, 120, 0, 0, 0, 20, 0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21,
    0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 109, 97, 110, 70, 73, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 118, 95, 116, 101, 120, 67, 111, 111, 114, 100, 0, 117, 95, 109, 111, 100, 117, 108,
    97, 116, 105, 111, 110, 0, 117, 95, 116, 101, 120, 116, 117, 114, 101, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 220, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 82, 139, 0, 0, 1, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 94, 139, 0, 0, 1, 0, 0, 0, 1, 0, 0,
    0, 2, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 5, 48, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 241, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240,
    0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 0, 21, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0,
    0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 65, 82, 50, 48, 45, 66, 73, 78, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 65, 37, 0, 0, 0, 0, 1, 0,
    0, 21, 0, 0, 0, 0, 1, 0, 1, 38, 1, 0, 0, 0, 2, 0, 4, 38, 186, 81, 78, 16, 2, 1, 0, 0, 1, 0,
    1, 39, 0, 4, 0, 0, 1, 0, 1, 40, 1, 0, 0, 0, 8, 0, 4, 40, 104, 192, 0, 0, 0, 242, 1, 70, 8, 32,
    0, 0, 0, 242, 1, 64, 40, 64, 0, 0, 0, 242, 1, 66, 72, 160, 0, 0, 0, 242, 1, 68, 1, 0, 6, 40,
    0, 0, 0, 0, 1, 0, 1, 41, 5, 0, 2, 0
};

static const Uint8 GLES2_FragmentTegra_Alpha_TextureSrc_[] = {
    71, 202, 114, 229, 47, 109, 131, 38, 6, 0, 1, 0, 5, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 73, 0,
    0, 0, 34, 0, 0, 0, 36, 0, 0, 0, 2, 0, 0, 0, 82, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0,
    82, 0, 0, 0, 1, 0, 0, 0, 20, 0, 0, 0, 6, 0, 0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0,
    0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 87, 0, 0, 0, 2, 0, 0, 0, 56, 0, 0, 0,
    13, 0, 0, 0, 101, 0, 0, 0, 4, 0, 0, 0, 16, 0, 0, 0, 14, 0, 0, 0, 105, 0, 0, 0, 1, 0, 0, 0, 4,
    0, 0, 0, 22, 0, 0, 0, 106, 0, 0, 0, 8, 0, 0, 0, 32, 0, 0, 0, 23, 0, 0, 0, 114, 0, 0, 0, 1, 0,
    0, 0, 4, 0, 0, 0, 15, 0, 0, 0, 115, 0, 0, 0, 1, 0, 0, 0, 80, 0, 0, 0, 17, 0, 0, 0, 135, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 135,
    0, 0, 0, 176, 0, 0, 0, 176, 0, 0, 0, 20, 0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21,
    0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 109, 97, 110, 70, 73, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 118, 95, 116, 101, 120, 67, 111, 111, 114, 100, 0, 117, 95, 109, 111, 100, 117, 108,
    97, 116, 105, 111, 110, 0, 117, 95, 116, 101, 120, 116, 117, 114, 101, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 220, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 82, 139, 0, 0, 1, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 94, 139, 0, 0, 1, 0, 0, 0, 1, 0, 0,
    0, 2, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 5, 48, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 118, 118, 17, 241, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0,
    240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0,
    1, 0, 0, 0, 2, 0, 0, 0, 21, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 16, 0, 0, 0, 16,
    0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 65, 82, 50, 48, 45, 66, 73, 78, 1, 0, 0, 0, 2, 0, 0, 0, 2, 0, 65, 37, 0, 0, 0, 0,
    8, 0, 129, 0, 1, 0, 0, 21, 0, 0, 0, 0, 2, 0, 1, 38, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 4, 38, 186,
    81, 78, 16, 2, 1, 0, 0, 2, 0, 1, 39, 0, 4, 0, 0, 0, 0, 0, 0, 2, 0, 1, 40, 1, 0, 0, 0, 5, 0,
    0, 0, 16, 0, 4, 40, 40, 160, 1, 0, 0, 242, 1, 66, 8, 192, 1, 0, 0, 242, 1, 64, 104, 32, 1, 0,
    0, 242, 1, 70, 72, 64, 1, 0, 0, 242, 1, 68, 154, 192, 0, 0, 37, 34, 64, 3, 8, 32, 0, 0, 5, 58,
    208, 4, 40, 64, 0, 0, 5, 50, 208, 4, 72, 160, 0, 0, 37, 42, 208, 4, 2, 0, 6, 40, 0, 0, 0, 0,
    0, 0, 0, 0, 2, 0, 1, 41, 0, 0, 0, 0, 5, 0, 2, 0
};

static const Uint8 GLES2_FragmentTegra_Additive_TextureSrc_[] = {
    161, 234, 193, 234, 47, 109, 131, 38, 6, 0, 1, 0, 5, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 73, 0,
    0, 0, 34, 0, 0, 0, 36, 0, 0, 0, 2, 0, 0, 0, 82, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0,
    82, 0, 0, 0, 1, 0, 0, 0, 20, 0, 0, 0, 6, 0, 0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0,
    0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 87, 0, 0, 0, 2, 0, 0, 0, 56, 0, 0, 0,
    13, 0, 0, 0, 101, 0, 0, 0, 4, 0, 0, 0, 16, 0, 0, 0, 14, 0, 0, 0, 105, 0, 0, 0, 1, 0, 0, 0, 4,
    0, 0, 0, 22, 0, 0, 0, 106, 0, 0, 0, 8, 0, 0, 0, 32, 0, 0, 0, 23, 0, 0, 0, 114, 0, 0, 0, 1, 0,
    0, 0, 4, 0, 0, 0, 15, 0, 0, 0, 115, 0, 0, 0, 1, 0, 0, 0, 80, 0, 0, 0, 17, 0, 0, 0, 135, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 135,
    0, 0, 0, 176, 0, 0, 0, 176, 0, 0, 0, 20, 0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21,
    0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 109, 97, 110, 70, 73, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 118, 95, 116, 101, 120, 67, 111, 111, 114, 100, 0, 117, 95, 109, 111, 100, 117, 108,
    97, 116, 105, 111, 110, 0, 117, 95, 116, 101, 120, 116, 117, 114, 101, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 220, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 82, 139, 0, 0, 1, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 94, 139, 0, 0, 1, 0, 0, 0, 1, 0, 0,
    0, 2, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 5, 48, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 22, 22, 17, 241, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240,
    0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0, 1, 0,
    0, 0, 2, 0, 0, 0, 21, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0,
    0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 65, 82, 50, 48, 45, 66, 73, 78, 1, 0, 0, 0, 2, 0, 0, 0, 2, 0, 65, 37, 0, 0, 0, 0, 8, 0,
    129, 0, 1, 0, 0, 21, 0, 0, 0, 0, 2, 0, 1, 38, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 4, 38, 186, 81,
    78, 16, 2, 1, 0, 0, 2, 0, 1, 39, 0, 4, 0, 0, 0, 0, 0, 0, 2, 0, 1, 40, 1, 0, 0, 0, 5, 0, 0, 0,
    16, 0, 4, 40, 40, 160, 1, 0, 0, 242, 1, 66, 104, 32, 1, 0, 0, 242, 1, 70, 8, 192, 1, 0, 0, 242,
    1, 64, 72, 64, 1, 0, 0, 242, 1, 68, 136, 192, 0, 0, 0, 26, 64, 4, 136, 32, 0, 0, 0, 2, 64, 7,
    136, 64, 0, 0, 0, 10, 64, 6, 136, 160, 0, 0, 0, 18, 64, 5, 2, 0, 6, 40, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 0, 1, 41, 0, 0, 0, 0, 5, 0, 2, 0
};

static const Uint8 GLES2_FragmentTegra_Modulated_TextureSrc_[] = {
    75, 132, 201, 227, 47, 109, 131, 38, 6, 0, 1, 0, 5, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 73, 0,
    0, 0, 34, 0, 0, 0, 36, 0, 0, 0, 2, 0, 0, 0, 82, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0,
    82, 0, 0, 0, 1, 0, 0, 0, 20, 0, 0, 0, 6, 0, 0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0,
    0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 87, 0, 0, 0, 2, 0, 0, 0, 56, 0, 0, 0,
    13, 0, 0, 0, 101, 0, 0, 0, 4, 0, 0, 0, 16, 0, 0, 0, 14, 0, 0, 0, 105, 0, 0, 0, 1, 0, 0, 0, 4,
    0, 0, 0, 22, 0, 0, 0, 106, 0, 0, 0, 8, 0, 0, 0, 32, 0, 0, 0, 23, 0, 0, 0, 114, 0, 0, 0, 1, 0,
    0, 0, 4, 0, 0, 0, 15, 0, 0, 0, 115, 0, 0, 0, 1, 0, 0, 0, 80, 0, 0, 0, 17, 0, 0, 0, 135, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 135,
    0, 0, 0, 176, 0, 0, 0, 176, 0, 0, 0, 20, 0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21,
    0, 0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 109, 97, 110, 70, 73, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 118, 95, 116, 101, 120, 67, 111, 111, 114, 100, 0, 117, 95, 109, 111, 100, 117, 108,
    97, 116, 105, 111, 110, 0, 117, 95, 116, 101, 120, 116, 117, 114, 101, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 220, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 82, 139, 0, 0, 1, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 94, 139, 0, 0, 1, 0, 0, 0, 1, 0, 0,
    0, 2, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 5, 48, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 32, 32, 17, 241, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 240,
    0, 0, 0, 240, 0, 0, 0, 240, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0, 1, 0,
    0, 0, 2, 0, 0, 0, 21, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 16, 0, 0, 0, 16, 0, 0,
    0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 65, 82, 50, 48, 45, 66, 73, 78, 1, 0, 0, 0, 2, 0, 0, 0, 2, 0, 65, 37, 0, 0, 0, 0, 8, 0,
    129, 0, 1, 0, 0, 21, 0, 0, 0, 0, 2, 0, 1, 38, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 4, 38, 186, 81,
    78, 16, 2, 1, 0, 0, 2, 0, 1, 39, 0, 4, 0, 0, 0, 0, 0, 0, 2, 0, 1, 40, 1, 0, 0, 0, 5, 0, 0, 0,
    16, 0, 4, 40, 40, 160, 1, 0, 0, 242, 1, 66, 8, 192, 1, 0, 0, 242, 1, 64, 104, 32, 1, 0, 0, 242,
    1, 70, 72, 64, 1, 0, 0, 242, 1, 68, 104, 192, 0, 0, 0, 242, 65, 4, 232, 32, 0, 0, 0, 242, 65,
    0, 40, 64, 0, 0, 0, 242, 65, 6, 72, 160, 0, 0, 0, 242, 65, 5, 2, 0, 6, 40, 0, 0, 0, 0, 0, 0,
    0, 0, 2, 0, 1, 41, 0, 0, 0, 0, 5, 0, 2, 0
};

static const GLES2_ShaderInstance GLES2_VertexTegra_Default = {
    GL_VERTEX_SHADER,
    GL_NVIDIA_PLATFORM_BINARY_NV,
    sizeof(GLES2_VertexTegra_Default_),
    GLES2_VertexTegra_Default_
};

static const GLES2_ShaderInstance GLES2_FragmentTegra_None_SolidSrc = {
    GL_FRAGMENT_SHADER,
    GL_NVIDIA_PLATFORM_BINARY_NV,
    sizeof(GLES2_FragmentTegra_None_SolidSrc_),
    GLES2_FragmentTegra_None_SolidSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentTegra_Alpha_SolidSrc = {
    GL_FRAGMENT_SHADER,
    GL_NVIDIA_PLATFORM_BINARY_NV,
    sizeof(GLES2_FragmentTegra_Alpha_SolidSrc_),
    GLES2_FragmentTegra_Alpha_SolidSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentTegra_Additive_SolidSrc = {
    GL_FRAGMENT_SHADER,
    GL_NVIDIA_PLATFORM_BINARY_NV,
    sizeof(GLES2_FragmentTegra_Additive_SolidSrc_),
    GLES2_FragmentTegra_Additive_SolidSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentTegra_Modulated_SolidSrc = {
    GL_FRAGMENT_SHADER,
    GL_NVIDIA_PLATFORM_BINARY_NV,
    sizeof(GLES2_FragmentTegra_Modulated_SolidSrc_),
    GLES2_FragmentTegra_Modulated_SolidSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentTegra_None_TextureSrc = {
    GL_FRAGMENT_SHADER,
    GL_NVIDIA_PLATFORM_BINARY_NV,
    sizeof(GLES2_FragmentTegra_None_TextureSrc_),
    GLES2_FragmentTegra_None_TextureSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentTegra_Alpha_TextureSrc = {
    GL_FRAGMENT_SHADER,
    GL_NVIDIA_PLATFORM_BINARY_NV,
    sizeof(GLES2_FragmentTegra_Alpha_TextureSrc_),
    GLES2_FragmentTegra_Alpha_TextureSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentTegra_Additive_TextureSrc = {
    GL_FRAGMENT_SHADER,
    GL_NVIDIA_PLATFORM_BINARY_NV,
    sizeof(GLES2_FragmentTegra_Additive_TextureSrc_),
    GLES2_FragmentTegra_Additive_TextureSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentTegra_Modulated_TextureSrc = {
    GL_FRAGMENT_SHADER,
    GL_NVIDIA_PLATFORM_BINARY_NV,
    sizeof(GLES2_FragmentTegra_Modulated_TextureSrc_),
    GLES2_FragmentTegra_Modulated_TextureSrc_
};

#endif /* GLES2_INCLUDE_NVIDIA_SHADERS */

/*************************************************************************************************
 * Vertex/fragment shader definitions                                                            *
 *************************************************************************************************/

static GLES2_Shader GLES2_VertexShader_Default = {
#if GLES2_INCLUDE_NVIDIA_SHADERS
    2,
#else
    1,
#endif
    {
#if GLES2_INCLUDE_NVIDIA_SHADERS
        &GLES2_VertexTegra_Default,
#endif
        &GLES2_VertexSrc_Default
    }
};

static GLES2_Shader GLES2_FragmentShader_None_SolidSrc = {
#if GLES2_INCLUDE_NVIDIA_SHADERS
    2,
#else
    1,
#endif
    {
#if GLES2_INCLUDE_NVIDIA_SHADERS
        &GLES2_FragmentTegra_None_SolidSrc,
#endif
        &GLES2_FragmentSrc_SolidSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Alpha_SolidSrc = {
#if GLES2_INCLUDE_NVIDIA_SHADERS
    2,
#else
    1,
#endif
    {
#if GLES2_INCLUDE_NVIDIA_SHADERS
        &GLES2_FragmentTegra_Alpha_SolidSrc,
#endif
        &GLES2_FragmentSrc_SolidSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Additive_SolidSrc = {
#if GLES2_INCLUDE_NVIDIA_SHADERS
    2,
#else
    1,
#endif
    {
#if GLES2_INCLUDE_NVIDIA_SHADERS
        &GLES2_FragmentTegra_Additive_SolidSrc,
#endif
        &GLES2_FragmentSrc_SolidSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Modulated_SolidSrc = {
#if GLES2_INCLUDE_NVIDIA_SHADERS
    2,
#else
    1,
#endif
    {
#if GLES2_INCLUDE_NVIDIA_SHADERS
        &GLES2_FragmentTegra_Modulated_SolidSrc,
#endif
        &GLES2_FragmentSrc_SolidSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_None_TextureABGRSrc = {
#if GLES2_INCLUDE_NVIDIA_SHADERS
    2,
#else
    1,
#endif
    {
#if GLES2_INCLUDE_NVIDIA_SHADERS
        &GLES2_FragmentTegra_None_TextureSrc,
#endif
        &GLES2_FragmentSrc_TextureABGRSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Alpha_TextureABGRSrc = {
#if GLES2_INCLUDE_NVIDIA_SHADERS
    2,
#else
    1,
#endif
    {
#if GLES2_INCLUDE_NVIDIA_SHADERS
        &GLES2_FragmentTegra_Alpha_TextureSrc,
#endif
        &GLES2_FragmentSrc_TextureABGRSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Additive_TextureABGRSrc = {
#if GLES2_INCLUDE_NVIDIA_SHADERS
    2,
#else
    1,
#endif
    {
#if GLES2_INCLUDE_NVIDIA_SHADERS
        &GLES2_FragmentTegra_Additive_TextureSrc,
#endif
        &GLES2_FragmentSrc_TextureABGRSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Modulated_TextureABGRSrc = {
#if GLES2_INCLUDE_NVIDIA_SHADERS
    2,
#else
    1,
#endif
    {
#if GLES2_INCLUDE_NVIDIA_SHADERS
        &GLES2_FragmentTegra_Modulated_TextureSrc,
#endif
        &GLES2_FragmentSrc_TextureABGRSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_None_TextureARGBSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureARGBSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Alpha_TextureARGBSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureARGBSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Additive_TextureARGBSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureARGBSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Modulated_TextureARGBSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureARGBSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_None_TextureRGBSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureRGBSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Alpha_TextureRGBSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureRGBSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Additive_TextureRGBSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureRGBSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Modulated_TextureRGBSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureRGBSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_None_TextureBGRSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureBGRSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Alpha_TextureBGRSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureBGRSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Additive_TextureBGRSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureBGRSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_Modulated_TextureBGRSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureBGRSrc
    }
};

/*************************************************************************************************
 * Shader selector                                                                               *
 *************************************************************************************************/

const GLES2_Shader *GLES2_GetShader(GLES2_ShaderType type, SDL_BlendMode blendMode)
{
    switch (type)
    {
    case GLES2_SHADER_VERTEX_DEFAULT:
        return &GLES2_VertexShader_Default;
    case GLES2_SHADER_FRAGMENT_SOLID_SRC:
    switch (blendMode)
    {
    case SDL_BLENDMODE_NONE:
        return &GLES2_FragmentShader_None_SolidSrc;
    case SDL_BLENDMODE_BLEND:
        return &GLES2_FragmentShader_Alpha_SolidSrc;
    case SDL_BLENDMODE_ADD:
        return &GLES2_FragmentShader_Additive_SolidSrc;
    case SDL_BLENDMODE_MOD:
        return &GLES2_FragmentShader_Modulated_SolidSrc;
    default:
        return NULL;
    }
    case GLES2_SHADER_FRAGMENT_TEXTURE_ABGR_SRC:
        switch (blendMode)
    {
        case SDL_BLENDMODE_NONE:
            return &GLES2_FragmentShader_None_TextureABGRSrc;
        case SDL_BLENDMODE_BLEND:
            return &GLES2_FragmentShader_Alpha_TextureABGRSrc;
        case SDL_BLENDMODE_ADD:
            return &GLES2_FragmentShader_Additive_TextureABGRSrc;
        case SDL_BLENDMODE_MOD:
            return &GLES2_FragmentShader_Modulated_TextureABGRSrc;
        default:
            return NULL;
    }
    case GLES2_SHADER_FRAGMENT_TEXTURE_ARGB_SRC:
        switch (blendMode)
    {
        case SDL_BLENDMODE_NONE:
            return &GLES2_FragmentShader_None_TextureARGBSrc;
        case SDL_BLENDMODE_BLEND:
            return &GLES2_FragmentShader_Alpha_TextureARGBSrc;
        case SDL_BLENDMODE_ADD:
            return &GLES2_FragmentShader_Additive_TextureARGBSrc;
        case SDL_BLENDMODE_MOD:
            return &GLES2_FragmentShader_Modulated_TextureARGBSrc;
        default:
            return NULL;
    }

    case GLES2_SHADER_FRAGMENT_TEXTURE_RGB_SRC:
        switch (blendMode)
    {
        case SDL_BLENDMODE_NONE:
            return &GLES2_FragmentShader_None_TextureRGBSrc;
        case SDL_BLENDMODE_BLEND:
            return &GLES2_FragmentShader_Alpha_TextureRGBSrc;
        case SDL_BLENDMODE_ADD:
            return &GLES2_FragmentShader_Additive_TextureRGBSrc;
        case SDL_BLENDMODE_MOD:
            return &GLES2_FragmentShader_Modulated_TextureRGBSrc;
        default:
            return NULL;
    }

    case GLES2_SHADER_FRAGMENT_TEXTURE_BGR_SRC:
        switch (blendMode)
    {
        case SDL_BLENDMODE_NONE:
            return &GLES2_FragmentShader_None_TextureBGRSrc;
        case SDL_BLENDMODE_BLEND:
            return &GLES2_FragmentShader_Alpha_TextureBGRSrc;
        case SDL_BLENDMODE_ADD:
            return &GLES2_FragmentShader_Additive_TextureBGRSrc;
        case SDL_BLENDMODE_MOD:
            return &GLES2_FragmentShader_Modulated_TextureBGRSrc;
        default:
            return NULL;
    }

    default:
        return NULL;
    }
}

#endif /* SDL_VIDEO_RENDER_OGL_ES2 && !SDL_RENDER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
