//-----------------------------------------------------------------------------
// Copyright (c) 2016 GarageGames, LLC
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

////////////////////////////////////////////////////////////////////////////////
// Portions Copyright (c) Microsoft Corporation. All rights reserved.
// https://github.com/Microsoft/DirectXTex
////////////////////////////////////////////////////////////////////////////////

#ifndef _DDSDATA_H_
#define _DDSDATA_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

#include "core/util/fourcc.h"

#ifdef TORQUE_OS_WIN
#include <dxgiformat.h>
#endif

namespace dds
{
   ///////////////////////////////////////////////////////////////////////////////////
   //                           DXGI Formats                                        //
   ///////////////////////////////////////////////////////////////////////////////////
#ifndef TORQUE_OS_WIN
   //From directx SDK
   typedef enum DXGI_FORMAT
   {
      DXGI_FORMAT_UNKNOWN = 0,
      DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
      DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
      DXGI_FORMAT_R32G32B32A32_UINT = 3,
      DXGI_FORMAT_R32G32B32A32_SINT = 4,
      DXGI_FORMAT_R32G32B32_TYPELESS = 5,
      DXGI_FORMAT_R32G32B32_FLOAT = 6,
      DXGI_FORMAT_R32G32B32_UINT = 7,
      DXGI_FORMAT_R32G32B32_SINT = 8,
      DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
      DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
      DXGI_FORMAT_R16G16B16A16_UNORM = 11,
      DXGI_FORMAT_R16G16B16A16_UINT = 12,
      DXGI_FORMAT_R16G16B16A16_SNORM = 13,
      DXGI_FORMAT_R16G16B16A16_SINT = 14,
      DXGI_FORMAT_R32G32_TYPELESS = 15,
      DXGI_FORMAT_R32G32_FLOAT = 16,
      DXGI_FORMAT_R32G32_UINT = 17,
      DXGI_FORMAT_R32G32_SINT = 18,
      DXGI_FORMAT_R32G8X24_TYPELESS = 19,
      DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
      DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
      DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
      DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
      DXGI_FORMAT_R10G10B10A2_UNORM = 24,
      DXGI_FORMAT_R10G10B10A2_UINT = 25,
      DXGI_FORMAT_R11G11B10_FLOAT = 26,
      DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
      DXGI_FORMAT_R8G8B8A8_UNORM = 28,
      DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
      DXGI_FORMAT_R8G8B8A8_UINT = 30,
      DXGI_FORMAT_R8G8B8A8_SNORM = 31,
      DXGI_FORMAT_R8G8B8A8_SINT = 32,
      DXGI_FORMAT_R16G16_TYPELESS = 33,
      DXGI_FORMAT_R16G16_FLOAT = 34,
      DXGI_FORMAT_R16G16_UNORM = 35,
      DXGI_FORMAT_R16G16_UINT = 36,
      DXGI_FORMAT_R16G16_SNORM = 37,
      DXGI_FORMAT_R16G16_SINT = 38,
      DXGI_FORMAT_R32_TYPELESS = 39,
      DXGI_FORMAT_D32_FLOAT = 40,
      DXGI_FORMAT_R32_FLOAT = 41,
      DXGI_FORMAT_R32_UINT = 42,
      DXGI_FORMAT_R32_SINT = 43,
      DXGI_FORMAT_R24G8_TYPELESS = 44,
      DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
      DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
      DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
      DXGI_FORMAT_R8G8_TYPELESS = 48,
      DXGI_FORMAT_R8G8_UNORM = 49,
      DXGI_FORMAT_R8G8_UINT = 50,
      DXGI_FORMAT_R8G8_SNORM = 51,
      DXGI_FORMAT_R8G8_SINT = 52,
      DXGI_FORMAT_R16_TYPELESS = 53,
      DXGI_FORMAT_R16_FLOAT = 54,
      DXGI_FORMAT_D16_UNORM = 55,
      DXGI_FORMAT_R16_UNORM = 56,
      DXGI_FORMAT_R16_UINT = 57,
      DXGI_FORMAT_R16_SNORM = 58,
      DXGI_FORMAT_R16_SINT = 59,
      DXGI_FORMAT_R8_TYPELESS = 60,
      DXGI_FORMAT_R8_UNORM = 61,
      DXGI_FORMAT_R8_UINT = 62,
      DXGI_FORMAT_R8_SNORM = 63,
      DXGI_FORMAT_R8_SINT = 64,
      DXGI_FORMAT_A8_UNORM = 65,
      DXGI_FORMAT_R1_UNORM = 66,
      DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
      DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
      DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
      DXGI_FORMAT_BC1_TYPELESS = 70,
      DXGI_FORMAT_BC1_UNORM = 71,
      DXGI_FORMAT_BC1_UNORM_SRGB = 72,
      DXGI_FORMAT_BC2_TYPELESS = 73,
      DXGI_FORMAT_BC2_UNORM = 74,
      DXGI_FORMAT_BC2_UNORM_SRGB = 75,
      DXGI_FORMAT_BC3_TYPELESS = 76,
      DXGI_FORMAT_BC3_UNORM = 77,
      DXGI_FORMAT_BC3_UNORM_SRGB = 78,
      DXGI_FORMAT_BC4_TYPELESS = 79,
      DXGI_FORMAT_BC4_UNORM = 80,
      DXGI_FORMAT_BC4_SNORM = 81,
      DXGI_FORMAT_BC5_TYPELESS = 82,
      DXGI_FORMAT_BC5_UNORM = 83,
      DXGI_FORMAT_BC5_SNORM = 84,
      DXGI_FORMAT_B5G6R5_UNORM = 85,
      DXGI_FORMAT_B5G5R5A1_UNORM = 86,
      DXGI_FORMAT_B8G8R8A8_UNORM = 87,
      DXGI_FORMAT_B8G8R8X8_UNORM = 88,
      DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
      DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
      DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
      DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
      DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
      DXGI_FORMAT_BC6H_TYPELESS = 94,
      DXGI_FORMAT_BC6H_UF16 = 95,
      DXGI_FORMAT_BC6H_SF16 = 96,
      DXGI_FORMAT_BC7_TYPELESS = 97,
      DXGI_FORMAT_BC7_UNORM = 98,
      DXGI_FORMAT_BC7_UNORM_SRGB = 99,
      DXGI_FORMAT_AYUV = 100,
      DXGI_FORMAT_Y410 = 101,
      DXGI_FORMAT_Y416 = 102,
      DXGI_FORMAT_NV12 = 103,
      DXGI_FORMAT_P010 = 104,
      DXGI_FORMAT_P016 = 105,
      DXGI_FORMAT_420_OPAQUE = 106,
      DXGI_FORMAT_YUY2 = 107,
      DXGI_FORMAT_Y210 = 108,
      DXGI_FORMAT_Y216 = 109,
      DXGI_FORMAT_NV11 = 110,
      DXGI_FORMAT_AI44 = 111,
      DXGI_FORMAT_IA44 = 112,
      DXGI_FORMAT_P8 = 113,
      DXGI_FORMAT_A8P8 = 114,
      DXGI_FORMAT_B4G4R4A4_UNORM = 115,
      DXGI_FORMAT_FORCE_UINT = 0xffffffff
   } DXGI_FORMAT;
#endif

   ///////////////////////////////////////////////////////////////////////////////////
   //                           D3DFMT Formats                                      //
   ///////////////////////////////////////////////////////////////////////////////////
   enum D3DFMT
   {
      D3DFMT_UNKNOWN = 0,

      D3DFMT_R8G8B8 = 20,
      D3DFMT_A8R8G8B8 = 21,
      D3DFMT_X8R8G8B8 = 22,
      D3DFMT_R5G6B5 = 23,
      D3DFMT_X1R5G5B5 = 24,
      D3DFMT_A1R5G5B5 = 25,
      D3DFMT_A4R4G4B4 = 26,
      D3DFMT_R3G3B2 = 27,
      D3DFMT_A8 = 28,
      D3DFMT_A8R3G3B2 = 29,
      D3DFMT_X4R4G4B4 = 30,
      D3DFMT_A2B10G10R10 = 31,
      D3DFMT_A8B8G8R8 = 32,
      D3DFMT_X8B8G8R8 = 33,
      D3DFMT_G16R16 = 34,
      D3DFMT_A2R10G10B10 = 35,
      D3DFMT_A16B16G16R16 = 36,

      D3DFMT_A8P8 = 40,
      D3DFMT_P8 = 41,

      D3DFMT_L8 = 50,
      D3DFMT_A8L8 = 51,
      D3DFMT_A4L4 = 52,

      D3DFMT_V8U8 = 60,
      D3DFMT_L6V5U5 = 61,
      D3DFMT_X8L8V8U8 = 62,
      D3DFMT_Q8W8V8U8 = 63,
      D3DFMT_V16U16 = 64,
      D3DFMT_A2W10V10U10 = 67,

      D3DFMT_UYVY = MakeFourCC('U', 'Y', 'V', 'Y'),
      D3DFMT_R8G8_B8G8 = MakeFourCC('R', 'G', 'B', 'G'),
      D3DFMT_YUY2 = MakeFourCC('Y', 'U', 'Y', '2'),
      D3DFMT_G8R8_G8B8 = MakeFourCC('G', 'R', 'G', 'B'),
      D3DFMT_DXT1 = MakeFourCC('D', 'X', 'T', '1'),
      D3DFMT_DXT2 = MakeFourCC('D', 'X', 'T', '2'),
      D3DFMT_DXT3 = MakeFourCC('D', 'X', 'T', '3'),
      D3DFMT_DXT4 = MakeFourCC('D', 'X', 'T', '4'),
      D3DFMT_DXT5 = MakeFourCC('D', 'X', 'T', '5'),

      D3DFMT_ATI1 = MakeFourCC('A', 'T', 'I', '1'),
      D3DFMT_AT1N = MakeFourCC('A', 'T', '1', 'N'),
      D3DFMT_ATI2 = MakeFourCC('A', 'T', 'I', '2'),
      D3DFMT_AT2N = MakeFourCC('A', 'T', '2', 'N'),

      D3DFMT_BC4U = MakeFourCC('B', 'C', '4', 'U'),
      D3DFMT_BC4S = MakeFourCC('B', 'C', '4', 'S'),
      D3DFMT_BC5U = MakeFourCC('B', 'C', '5', 'U'),
      D3DFMT_BC5S = MakeFourCC('B', 'C', '5', 'S'),

      D3DFMT_ETC = MakeFourCC('E', 'T', 'C', ' '),
      D3DFMT_ETC1 = MakeFourCC('E', 'T', 'C', '1'),
      D3DFMT_ATC = MakeFourCC('A', 'T', 'C', ' '),
      D3DFMT_ATCA = MakeFourCC('A', 'T', 'C', 'A'),
      D3DFMT_ATCI = MakeFourCC('A', 'T', 'C', 'I'),

      D3DFMT_POWERVR_2BPP = MakeFourCC('P', 'T', 'C', '2'),
      D3DFMT_POWERVR_4BPP = MakeFourCC('P', 'T', 'C', '4'),

      D3DFMT_D16_LOCKABLE = 70,
      D3DFMT_D32 = 71,
      D3DFMT_D15S1 = 73,
      D3DFMT_D24S8 = 75,
      D3DFMT_D24X8 = 77,
      D3DFMT_D24X4S4 = 79,
      D3DFMT_D16 = 80,

      D3DFMT_D32F_LOCKABLE = 82,
      D3DFMT_D24FS8 = 83,

      D3DFMT_L16 = 81,

      D3DFMT_VERTEXDATA = 100,
      D3DFMT_INDEX16 = 101,
      D3DFMT_INDEX32 = 102,

      D3DFMT_Q16W16V16U16 = 110,

      D3DFMT_MULTI2_ARGB8 = MakeFourCC('M', 'E', 'T', '1'),

      D3DFMT_R16F = 111,
      D3DFMT_G16R16F = 112,
      D3DFMT_A16B16G16R16F = 113,

      D3DFMT_R32F = 114,
      D3DFMT_G32R32F = 115,
      D3DFMT_A32B32G32R32F = 116,

      D3DFMT_CxV8U8 = 117,

      D3DFMT_DX10 = MakeFourCC('D', 'X', '1', '0'),

      D3DFMT_FORCE_DWORD = 0x7fffffff
   };

   ///////////////////////////////////////////////////////////////////////////////////
   //                               Defines                                         //
   ///////////////////////////////////////////////////////////////////////////////////
   #pragma pack(push,1)

   #define DDS_HEADER_SIZE       124
   #define DDS_HEADER_DX10_SIZE  20
   #define DDS_MAGIC       0x20534444  // "DDS "
   #define DDS_FOURCC      0x00000004  // DDPF_FOURCC
   #define DDS_RGB         0x00000040  // DDPF_RGB
   #define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
   #define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
   #define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
   #define DDS_ALPHAPIXELS 0x00000001  // DDPF_ALPHAPIXELS
   #define DDS_ALPHA       0x00000002  // DDPF_ALPHA
   #define DDS_PAL8        0x00000020  // DDPF_PALETTEINDEXED8
   #define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV
   #define DDS_YUV         0x00000200  //DDPF_YUV

   #define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
   #define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
   #define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
   #define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
   #define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

   #define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
   #define DDS_WIDTH  0x00000004 // DDSD_WIDTH

   #define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
   #define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
   #define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

   #define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP
   #define DDS_CUBEMAP_POSITIVEX 0x00000400 //  DDSCAPS2_CUBEMAP_POSITIVEX
   #define DDS_CUBEMAP_NEGATIVEX 0x00000800 //  DDSCAPS2_CUBEMAP_NEGATIVEX
   #define DDS_CUBEMAP_POSITIVEY 0x00001000 //  DDSCAPS2_CUBEMAP_POSITIVEY
   #define DDS_CUBEMAP_NEGATIVEY 0x00002000 //  DDSCAPS2_CUBEMAP_NEGATIVEY
   #define DDS_CUBEMAP_POSITIVEZ 0x00004000 //  DDSCAPS2_CUBEMAP_POSITIVEZ
   #define DDS_CUBEMAP_NEGATIVEZ 0x00008000 //  DDSCAPS2_CUBEMAP_NEGATIVEZ

   #define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP | DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                                  DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                                  DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

   #define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME

   ///////////////////////////////////////////////////////////////////////////////////
   //                                Enums                                          //
   ///////////////////////////////////////////////////////////////////////////////////
   // Subset here matches D3D10_RESOURCE_DIMENSION and D3D11_RESOURCE_DIMENSION
   enum DDS_RESOURCE_DIMENSION
   {
      DDS_DIMENSION_TEXTURE1D = 2,
      DDS_DIMENSION_TEXTURE2D = 3,
      DDS_DIMENSION_TEXTURE3D = 4,
   };

   // Subset here matches D3D10_RESOURCE_MISC_FLAG and D3D11_RESOURCE_MISC_FLAG
   enum DDS_RESOURCE_MISC_FLAG
   {
      DDS_RESOURCE_MISC_TEXTURECUBE = 0x4L,
   };

   enum DDS_MISC_FLAGS2
   {
      DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
   };

   enum DDS_ALPHA_MODE
   {
      DDS_ALPHA_MODE_UNKNOWN = 0,
      DDS_ALPHA_MODE_STRAIGHT = 1,
      DDS_ALPHA_MODE_PREMULTIPLIED = 2,
      DDS_ALPHA_MODE_OPAQUE = 3,
      DDS_ALPHA_MODE_CUSTOM = 4,
   };

   enum D3D10_RESOURCE_DIMENSION
   {
      D3D10_RESOURCE_DIMENSION_UNKNOWN = 0,
      D3D10_RESOURCE_DIMENSION_BUFFER = 1,
      D3D10_RESOURCE_DIMENSION_TEXTURE1D = 2,
      D3D10_RESOURCE_DIMENSION_TEXTURE2D = 3,
      D3D10_RESOURCE_DIMENSION_TEXTURE3D = 4
   };

   enum D3D10_RESOURCE_MISC_FLAG
   {
      D3D10_RESOURCE_MISC_GENERATE_MIPS = 0x1L,
      D3D10_RESOURCE_MISC_SHARED = 0x2L,
      D3D10_RESOURCE_MISC_TEXTURECUBE = 0x4L,
      D3D10_RESOURCE_MISC_SHARED_KEYEDMUTEX = 0x10L,
      D3D10_RESOURCE_MISC_GDI_COMPATIBLE = 0x20L,
   };

   ///////////////////////////////////////////////////////////////////////////////////
   //                                Structs                                        //
   ///////////////////////////////////////////////////////////////////////////////////

   struct DDS_PIXELFORMAT
   {
      U32   size;
      U32   flags;
      U32   fourCC;
      U32   bpp;
      U32   RBitMask;
      U32   GBitMask;
      U32   BBitMask;
      U32   ABitMask;

      bool operator==(const DDS_PIXELFORMAT& _test) const
      {
         return ( size == _test.size &&
                  flags == _test.flags &&
                  fourCC == _test.fourCC &&
                  bpp == _test.bpp &&
                  RBitMask == _test.RBitMask &&
                  GBitMask == _test.GBitMask &&
                  BBitMask == _test.BBitMask &&
                  ABitMask == _test.ABitMask);
      }
   };

   struct DDS_HEADER
   {
      U32   size;
      U32   flags;
      U32   height;
      U32   width;
      U32   pitchOrLinearSize;
      U32   depth; // only if DDS_HEADER_FLAGS_VOLUME is set in dwFlags
      U32   mipMapCount;
      U32   reserved1[11];
      DDS_PIXELFORMAT ddspf;
      U32   surfaceFlags;
      U32   cubemapFlags;
      U32   reserved2[3];
   };

   struct DDS_HEADER_DXT10
   {
      DXGI_FORMAT dxgiFormat;
      U32    resourceDimension;
      U32    miscFlag; // see DDS_RESOURCE_MISC_FLAG
      U32    arraySize;
      U32    miscFlags2; // see DDS_MISC_FLAGS2
   };

   ///////////////////////////////////////////////////////////////////////////////////
   //                            Pixel Formats                                      //
   ///////////////////////////////////////////////////////////////////////////////////

   const DDS_PIXELFORMAT DDSPF_DXT1 =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_DXT1, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_DXT2 =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_DXT2, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_DXT3 =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_DXT3, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_DXT4 =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_DXT4, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_DXT5 =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_DXT5, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_BC4_UNORM =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_BC4U, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_BC4_SNORM =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_BC4S, 0, 0, 0, 0, 0 };

   //todo check diff between this and ('B','C','5','U')
   const DDS_PIXELFORMAT DDSPF_ATI2 =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_ATI2, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_ATI1 =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_ATI1, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_BC5_UNORM =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_BC5U, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_BC5_SNORM =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_BC5S, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_R8G8_B8G8 =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_R8G8_B8G8, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_G8R8_G8B8 =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_G8R8_G8B8, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_YUY2 =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_YUY2, 0, 0, 0, 0, 0 };

   const DDS_PIXELFORMAT DDSPF_A8R8G8B8 =
   { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };

   const DDS_PIXELFORMAT DDSPF_X8R8G8B8 =
   { sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };

   const DDS_PIXELFORMAT DDSPF_A8B8G8R8 =
   { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

   const DDS_PIXELFORMAT DDSPF_X8B8G8R8 =
   { sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000 };

   const DDS_PIXELFORMAT DDSPF_G16R16 =
   { sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };

   const DDS_PIXELFORMAT DDSPF_R5G6B5 =
   { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 };

   const DDS_PIXELFORMAT DDSPF_A1R5G5B5 =
   { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 };

   const DDS_PIXELFORMAT DDSPF_A4R4G4B4 =
   { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 };

   const DDS_PIXELFORMAT DDSPF_R8G8B8 =
   { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };

   const DDS_PIXELFORMAT DDSPF_L8 =
   { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0,  8, 0xff, 0x00, 0x00, 0x00 };

   const DDS_PIXELFORMAT DDSPF_L16 =
   { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0, 16, 0xffff, 0x0000, 0x0000, 0x0000 };

   const DDS_PIXELFORMAT DDSPF_A8L8 =
   { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCEA, 0, 16, 0x00ff, 0x0000, 0x0000, 0xff00 };

   const DDS_PIXELFORMAT DDSPF_A4L4 =
   { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCEA, 0, 8, 0x0000000f, 0x0000, 0x0000, 0x000000f0 };

   const DDS_PIXELFORMAT DDSPF_A8 =
   { sizeof(DDS_PIXELFORMAT), DDS_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff };

   const DDS_PIXELFORMAT DDSPF_V8U8 =
   { sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 16, 0x00ff, 0xff00, 0x0000, 0x0000 };

   const DDS_PIXELFORMAT DDSPF_Q8W8V8U8 =
   { sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

   const DDS_PIXELFORMAT DDSPF_V16U16 =
   { sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };

   // D3DFMT_A2R10G10B10/D3DFMT_A2B10G10R10 should be written using DX10 extension to avoid D3DX 10:10:10:2 reversal issue

   // This indicates the DDS_HEADER_DXT10 extension is present (the format is in dxgiFormat)
   const DDS_PIXELFORMAT DDSPF_DX10 =
   { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, D3DFMT_DX10, 0, 0, 0, 0, 0 };

   #pragma pack(pop)

   ///////////////////////////////////////////////////////////////////////////////////
   //                               Functions                                       //
   ///////////////////////////////////////////////////////////////////////////////////

   //get DDS_PIXELFORMAT struct from GFXFormat - todo more formats
   const DDS_PIXELFORMAT getDDSFormat(const GFXFormat format)
   {
      switch (format)
      {
         case GFXFormatA4L4:     return DDSPF_A4L4;
         case GFXFormatL8:       return DDSPF_L8;
         case GFXFormatA8:       return DDSPF_A8;
         case GFXFormatA8L8:     return DDSPF_A8L8;
         case GFXFormatL16:      return DDSPF_L16;
         case GFXFormatR5G6B5:   return DDSPF_R5G6B5;
         case GFXFormatR5G5B5A1: return DDSPF_A1R5G5B5;
         case GFXFormatR8G8B8:   return DDSPF_R8G8B8;
         case GFXFormatR8G8B8A8: return DDSPF_A8R8G8B8;
         case GFXFormatR8G8B8X8: return DDSPF_X8R8G8B8;
         case GFXFormatB8G8R8A8: return DDSPF_A8B8G8R8;
         case GFXFormatR16G16B16A16F:
         case GFXFormatR32G32B32A32F: return DDSPF_DX10;
         //compressed
         case GFXFormatBC1:      return DDSPF_DXT1;
         case GFXFormatBC2:      return DDSPF_DXT3;
         case GFXFormatBC3:      return DDSPF_DXT5;
         case GFXFormatBC4:      return DDSPF_ATI1;
         case GFXFormatBC5:      return DDSPF_ATI2;
         default:
         {
            Con::errorf("dds::getDDSFormat: unknown format");
            return DDSPF_A8R8G8B8;
         }
      }
   }

   //get DXGI_FORMAT from GFXFormat - todo more formats
   const DXGI_FORMAT getDXGIFormat(const GFXFormat format)
   {
      switch (format)
      {
         //byte
         case GFXFormatR5G6B5:         return DXGI_FORMAT_B5G6R5_UNORM;
         case GFXFormatR5G5B5A1:       return DXGI_FORMAT_B5G5R5A1_UNORM;
         case GFXFormatB8G8R8A8:       return DXGI_FORMAT_R8G8B8A8_UNORM;
         case GFXFormatR8G8B8A8:       return DXGI_FORMAT_B8G8R8A8_UNORM;
         case GFXFormatR8G8B8X8:       return DXGI_FORMAT_B8G8R8X8_UNORM;
         case GFXFormatR10G10B10A2:    return DXGI_FORMAT_R10G10B10A2_UNORM;
         //uint
         case GFXFormatR16G16:         return DXGI_FORMAT_R16G16_UINT;
         case GFXFormatR16G16B16A16:   return DXGI_FORMAT_R16G16B16A16_UINT;
         //float
         case GFXFormatR16F:           return DXGI_FORMAT_R16_FLOAT;
         case GFXFormatR32F:           return DXGI_FORMAT_R32_FLOAT;
         case GFXFormatR16G16B16A16F:  return DXGI_FORMAT_R16G16B16A16_FLOAT;
         case GFXFormatR32G32B32A32F:  return DXGI_FORMAT_R32G32B32A32_FLOAT;
         //compressed
         case GFXFormatBC1:            return DXGI_FORMAT_BC1_UNORM;
         case GFXFormatBC2:            return DXGI_FORMAT_BC2_UNORM;
         case GFXFormatBC3:            return DXGI_FORMAT_BC3_UNORM;
         case GFXFormatBC4:            return DXGI_FORMAT_BC4_UNORM;
         case GFXFormatBC5:            return DXGI_FORMAT_BC5_UNORM;
         default:
         {
            Con::errorf("dds::getDXGIFormat: unknown format");
            return DXGI_FORMAT_UNKNOWN;
         }
      }
   }

   //get GFXFormat from D3DFMT - todo more formats
   const GFXFormat getGFXFormat(const D3DFMT format)
   {
      switch (format)
      {
         //byte        
         case D3DFMT_A4L4:          return GFXFormatA4L4;
         case D3DFMT_L8:            return GFXFormatL8;
         case D3DFMT_A8:            return GFXFormatA8;
         case D3DFMT_A8L8:          return GFXFormatA8L8;
         case D3DFMT_L16:           return GFXFormatL16;
         case D3DFMT_R5G6B5:        return GFXFormatR5G6B5;
         case D3DFMT_A1R5G5B5:      return GFXFormatR5G5B5A1;
         case D3DFMT_R8G8B8:        return GFXFormatR8G8B8;
         case D3DFMT_A8R8G8B8:      return GFXFormatR8G8B8A8;
         case D3DFMT_X8R8G8B8:      return GFXFormatR8G8B8A8;
         case D3DFMT_A8B8G8R8:      return GFXFormatB8G8R8A8;
         case D3DFMT_X8B8G8R8:      return GFXFormatB8G8R8A8;
         //uint
         case D3DFMT_G16R16:        return GFXFormatR16G16;
         case D3DFMT_A16B16G16R16:  return GFXFormatR16G16B16A16;
         //float
         case D3DFMT_R16F:          return GFXFormatR16F;
         case D3DFMT_R32F:          return GFXFormatR32F;
         case D3DFMT_A16B16G16R16F: return GFXFormatR16G16B16A16F;
         case D3DFMT_A32B32G32R32F: return GFXFormatR32G32B32A32F;
         //compressed
         case D3DFMT_DXT1:          return GFXFormatBC1;
         case D3DFMT_DXT2:          
         case D3DFMT_DXT3:          return GFXFormatBC2;
         case D3DFMT_DXT4:
         case D3DFMT_DXT5:          return GFXFormatBC3;
         case D3DFMT_ATI1:          return GFXFormatBC4;
         case D3DFMT_ATI2:          return GFXFormatBC5;
         default:
         {
            Con::errorf("dds::getGFXFormat: unknown format");
            return GFXFormat_FIRST;
         }
      }
   }

   //get GFXFormat from DXGI_FORMAT - todo more formats
   const GFXFormat getGFXFormat(const DXGI_FORMAT format)
   {
      switch (format)
      {
         //byte
         case DXGI_FORMAT_B5G6R5_UNORM:         return GFXFormatR5G6B5;
         case DXGI_FORMAT_B5G5R5A1_UNORM:       return GFXFormatR5G5B5A1;
         case DXGI_FORMAT_R8G8B8A8_UNORM:       return GFXFormatB8G8R8A8;
         case DXGI_FORMAT_B8G8R8A8_UNORM:       return GFXFormatR8G8B8A8;
         case DXGI_FORMAT_B8G8R8X8_UNORM:       return GFXFormatR8G8B8X8;
         case DXGI_FORMAT_R10G10B10A2_UNORM:    return GFXFormatR10G10B10A2;
         //uint
         case DXGI_FORMAT_R16G16_UINT:          return GFXFormatR16G16;
         case DXGI_FORMAT_R16G16B16A16_UINT:    return GFXFormatR16G16B16A16;
         //float
         case DXGI_FORMAT_R16_FLOAT:            return GFXFormatR16F;
         case DXGI_FORMAT_R32_FLOAT:            return GFXFormatR32F;
         case DXGI_FORMAT_R16G16B16A16_FLOAT:   return GFXFormatR16G16B16A16F;
         case DXGI_FORMAT_R32G32B32A32_FLOAT:   return GFXFormatR32G32B32A32F;
         //compressed
         case DXGI_FORMAT_BC1_UNORM:            return GFXFormatBC1;
         case DXGI_FORMAT_BC2_UNORM:            return GFXFormatBC2;
         case DXGI_FORMAT_BC3_UNORM:            return GFXFormatBC3;
         case DXGI_FORMAT_BC4_UNORM:            return GFXFormatBC4;
         case DXGI_FORMAT_BC5_UNORM:            return GFXFormatBC5;
         default:
         {
            Con::errorf("dds::getGFXFormatDxgi: unknown format");
            return GFXFormat_FIRST;
         }
      }
   }

   //get GFXFormat from DDS_PIXELFORMAT struct - todo more formats
   const GFXFormat getGFXFormat(const DDS_PIXELFORMAT &format)
   {
      if (format == DDSPF_DXT1)
         return GFXFormatBC1;
      else if (format == DDSPF_DXT2)
         return GFXFormatBC2;
      else if (format == DDSPF_DXT3)
         return GFXFormatBC2;
      else if (format == DDSPF_DXT4)
         return GFXFormatBC3;
      else if (format == DDSPF_DXT5)
         return GFXFormatBC3;
      else if (format == DDSPF_ATI1)
         return GFXFormatBC4;
      else if (format == DDSPF_ATI2)
         return GFXFormatBC5;
      else if (format == DDSPF_A8R8G8B8)
         return GFXFormatR8G8B8A8;
      else if (format == DDSPF_X8R8G8B8)
         return GFXFormatR8G8B8A8;
      else if (format == DDSPF_A8B8G8R8)
         return GFXFormatB8G8R8A8;
      else if (format == DDSPF_X8B8G8R8)
         return GFXFormatB8G8R8A8;
      else if (format == DDSPF_R8G8B8)
         return GFXFormatR8G8B8;
      else if (format == DDSPF_A8L8)
         return GFXFormatA8L8;
      else if (format == DDSPF_A4L4)
         return GFXFormatA4L4;
      else if (format == DDSPF_A8)
         return GFXFormatA8;
      else if (format == DDSPF_L8)
         return GFXFormatL8;
      else if (format == DDSPF_R5G6B5)
         return GFXFormatR5G6B5;
      else if (format == DDSPF_A1R5G5B5)
         return GFXFormatR5G5B5A1;
      else
      {
         Con::errorf("dds::getGFXFormat: unknown format");
         return GFXFormat_FIRST;
      }
   }

   //get GFXFormat from fourcc value - todo more formats
   const GFXFormat getGFXFormat(const U32 fourcc)
   {
      switch (fourcc)
      {
         case D3DFMT_DXT1: return GFXFormatBC1;
         case D3DFMT_DXT2:
         case D3DFMT_DXT3: return GFXFormatBC2;
         case D3DFMT_DXT4:
         case D3DFMT_DXT5: return GFXFormatBC3;
         case D3DFMT_ATI1: return GFXFormatBC4;
         case D3DFMT_ATI2: return GFXFormatBC5;
         case D3DFMT_A16B16G16R16F: return GFXFormatR16G16B16A16F;
         case D3DFMT_A32B32G32R32F: return GFXFormatR32G32B32A32F;
         default:
         {
            Con::errorf("dds::getGFXFormatFourcc: unknown format");
            return GFXFormat_FIRST;
         }
      }
   }

   const bool validateHeader(const DDS_HEADER &header)
   {
      if (header.size != DDS_HEADER_SIZE)
      {
         Con::errorf("DDS_HEADER - incorrect header size. Expected 124 bytes.");
         return false;
      }

      if (!(header.flags & DDS_HEADER_FLAGS_TEXTURE))
      {
         Con::errorf("DDS_HEADER - incorrect surface description flags.");
         return false;
      }

      if ((header.flags & (DDS_HEADER_FLAGS_LINEARSIZE | DDS_HEADER_FLAGS_PITCH)) == (DDS_HEADER_FLAGS_LINEARSIZE | DDS_HEADER_FLAGS_PITCH))
      {
         // Both are invalid!
         Con::errorf("DDS_HEADER - encountered both DDSD_LINEARSIZE and DDSD_PITCH!");
         return false;
      }

      return true;
   }

   const bool validateHeaderDx10(const DDS_HEADER_DXT10 &header)
   {
      if (sizeof(DDS_HEADER_DXT10) != DDS_HEADER_DX10_SIZE)
      {
         Con::errorf("DDS_HEADER_DXT10 - incorrect header size. Expected 20 bytes.");
         return false;
      }

      return true;
   }

}

#endif