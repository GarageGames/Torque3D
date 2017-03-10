// File: crn_dds_texture.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_dds_texture.h"
#include "crn_cfile_stream.h"
#include "crn_image_utils.h"
#include "crn_console.h"
#include "crn_texture_comp.h"

#define CRND_HEADER_FILE_ONLY
#include "../inc/crn_decomp.h"

namespace crnlib
{
   const vec2I g_vertical_cross_image_offsets[6] = { vec2I(2, 1), vec2I(0, 1), vec2I(1, 0), vec2I(1, 2), vec2I(1, 1), vec2I(1, 3) };

   mip_level::mip_level() :
      m_width(0),
      m_height(0),
      m_comp_flags(pixel_format_helpers::cDefaultCompFlags),
      m_format(PIXEL_FMT_INVALID),
      m_pImage(NULL),
      m_pDXTImage(NULL)
   {
   }

   mip_level::mip_level(const mip_level& other) :
      m_width(0),
      m_height(0),
      m_comp_flags(pixel_format_helpers::cDefaultCompFlags),
      m_format(PIXEL_FMT_INVALID),
      m_pImage(NULL),
      m_pDXTImage(NULL)
   {
      *this = other;
   }

   mip_level& mip_level::operator= (const mip_level& rhs)
   {
      clear();

      m_width = rhs.m_width;
      m_height = rhs.m_height;
      m_comp_flags = rhs.m_comp_flags;
      m_format = rhs.m_format;

      if (rhs.m_pImage)
         m_pImage = crnlib_new<image_u8>(*rhs.m_pImage);

      if (rhs.m_pDXTImage)
         m_pDXTImage = crnlib_new<dxt_image>(*rhs.m_pDXTImage);

      return *this;
   }

   mip_level::~mip_level()
   {
      crnlib_delete(m_pImage);
      crnlib_delete(m_pDXTImage);
   }

   void mip_level::clear()
   {
      m_width = 0;
      m_height = 0;
      m_comp_flags = pixel_format_helpers::cDefaultCompFlags;
      m_format = PIXEL_FMT_INVALID;

      if (m_pImage)
      {
         crnlib_delete(m_pImage);
         m_pImage = NULL;
      }

      if (m_pDXTImage)
      {
         crnlib_delete(m_pDXTImage);
         m_pDXTImage = NULL;
      }
   }

   void mip_level::assign(image_u8* p, pixel_format fmt)
   {
      CRNLIB_ASSERT(p);

      clear();

      m_pImage = p;

      m_width = p->get_width();
      m_height = p->get_height();

      if (fmt != PIXEL_FMT_INVALID)
         m_format = fmt;
      else
      {
         if (p->is_grayscale())
            m_format = p->is_component_valid(3) ? PIXEL_FMT_A8L8 : PIXEL_FMT_L8;
         else
            m_format = p->is_component_valid(3) ? PIXEL_FMT_A8R8G8B8 : PIXEL_FMT_R8G8B8;
      }

      m_comp_flags = p->get_comp_flags(); //pixel_format_helpers::get_component_flags(m_format);
   }

   void mip_level::assign(dxt_image* p, pixel_format fmt)
   {
      CRNLIB_ASSERT(p);

      clear();

      m_pDXTImage = p;

      m_width = p->get_width();
      m_height = p->get_height();

      if (fmt != PIXEL_FMT_INVALID)
         m_format = fmt;
      else
         m_format = pixel_format_helpers::from_dxt_format(p->get_format());

      m_comp_flags = pixel_format_helpers::get_component_flags(m_format);
   }

   bool mip_level::pack_to_dxt(const image_u8& img, pixel_format fmt, bool cook, const dxt_image::pack_params& orig_params)
   {
      CRNLIB_ASSERT(pixel_format_helpers::is_dxt(fmt));
      if (!pixel_format_helpers::is_dxt(fmt))
         return false;

      dxt_image::pack_params p(orig_params);
      if (pixel_format_helpers::is_pixel_format_non_srgb(fmt) || (img.get_comp_flags() & pixel_format_helpers::cCompFlagNormalMap) || (img.get_comp_flags() & pixel_format_helpers::cCompFlagLumaChroma))
      {
         // Disable perceptual colorspace metrics when packing to swizzled or non-RGB pixel formats.
         p.m_perceptual = false;
      }

      image_u8 tmp_img(img);

      clear();

      m_format = fmt;

      if (cook)
         cook_image(tmp_img);

      if ((pixel_format_helpers::is_alpha_only(fmt)) && (!tmp_img.has_alpha()))
         tmp_img.set_alpha_to_luma();

      dxt_format dxt_fmt = pixel_format_helpers::get_dxt_format(fmt);

      dxt_image* pDXT_image = crnlib_new<dxt_image>();
      if (!pDXT_image->init(dxt_fmt, tmp_img, p))
      {
         clear();
         return false;
      }

      assign(pDXT_image, fmt);

      return true;
   }

   bool mip_level::pack_to_dxt(pixel_format fmt, bool cook, const dxt_image::pack_params& p)
   {
      CRNLIB_ASSERT(pixel_format_helpers::is_dxt(fmt));
      if (!pixel_format_helpers::is_dxt(fmt))
         return false;

      image_u8 tmp_img;
      image_u8* pImage = get_unpacked_image(tmp_img, true);

      return pack_to_dxt(*pImage, fmt, cook, p);
   }

   bool mip_level::unpack_from_dxt(bool uncook)
   {
      if (!m_pDXTImage)
         return false;

      image_u8* pNew_img = crnlib_new<image_u8>();
      image_u8* pImg = get_unpacked_image(*pNew_img, uncook);
      pImg;

      CRNLIB_ASSERT(pImg == pNew_img);

      assign(pNew_img);
      return true;
   }

   bool mip_level::set_alpha_to_luma()
   {
      if (m_pDXTImage)
         unpack_from_dxt(true);

      m_pImage->set_alpha_to_luma();

      m_comp_flags = m_pImage->get_comp_flags();

      if (m_pImage->is_grayscale())
         m_format = PIXEL_FMT_A8L8;
      else
         m_format = PIXEL_FMT_A8R8G8B8;

      return true;
   }

   bool mip_level::convert(image_utils::conversion_type conv_type)
   {
      if (m_pDXTImage)
         unpack_from_dxt(true);

      image_utils::convert_image(*m_pImage, conv_type);

      m_comp_flags = m_pImage->get_comp_flags();

      if (m_pImage->is_grayscale())
         m_format = m_pImage->has_alpha() ? PIXEL_FMT_A8L8 : PIXEL_FMT_L8;
      else 
         m_format = m_pImage->has_alpha() ? PIXEL_FMT_A8R8G8B8 : PIXEL_FMT_R8G8B8;

      return true;
   }

   bool mip_level::convert(pixel_format fmt, bool cook, const dxt_image::pack_params& p)
   {
      if (pixel_format_helpers::is_dxt(fmt))
         return pack_to_dxt(fmt, cook, p);

      image_u8 tmp_img;
      image_u8* pImg = get_unpacked_image(tmp_img, true);

      image_u8* pImage = crnlib_new<image_u8>();
      pImage->set_comp_flags(pixel_format_helpers::get_component_flags(fmt));

      if (!pImage->resize(pImg->get_width(), pImg->get_height()))
         return false;

      for (uint y = 0; y < pImg->get_height(); y++)
      {
         for (uint x = 0; x < pImg->get_width(); x++)
         {
            color_quad_u8 c((*pImg)(x, y));

            if ((pixel_format_helpers::is_alpha_only(fmt)) && (!pImg->has_alpha()))
            {
               c.a = static_cast<uint8>(c.get_luma());
            }
            else
            {
               if (pImage->is_grayscale())
               {
                  uint8 g = static_cast<uint8>(c.get_luma());
                  c.r = g;
                  c.g = g;
                  c.b = g;
               }

               if (!pImage->is_component_valid(3))
                  c.a = 255;
            }

            (*pImage)(x, y) = c;
         }
      }

      assign(pImage, fmt);

      return true;
   }

   void mip_level::cook_image(image_u8& img) const
   {
      image_utils::conversion_type conv_type = image_utils::get_conversion_type(true, m_format);

      if (conv_type != image_utils::cConversion_Invalid)
         image_utils::convert_image(img, conv_type);
   }

   void mip_level::uncook_image(image_u8& img) const
   {
      image_utils::conversion_type conv_type = image_utils::get_conversion_type(false, m_format);

      if (conv_type != image_utils::cConversion_Invalid)
         image_utils::convert_image(img, conv_type);
   }

   image_u8* mip_level::get_unpacked_image(image_u8& tmp, bool uncook) const
   {
      if (m_pImage)
         return m_pImage;

      if (m_pDXTImage)
      {
         m_pDXTImage->unpack(tmp);

         tmp.set_comp_flags(m_comp_flags);

         if (uncook)
            uncook_image(tmp);

         return &tmp;
      }

      return NULL;
   }

   // -------------------------------------------------------------------------

   dds_texture::dds_texture() :
      m_width(0),
      m_height(0),
      m_comp_flags(pixel_format_helpers::cDefaultCompFlags),
      m_format(PIXEL_FMT_INVALID),
      m_source_file_type(texture_file_types::cFormatInvalid)
   {
   }

   dds_texture::~dds_texture()
   {
      free_all_mips();
   }

   void dds_texture::clear()
   {
      free_all_mips();

      m_name.clear();
      m_width = 0;
      m_height = 0;
      m_comp_flags = pixel_format_helpers::cDefaultCompFlags;
      m_format = PIXEL_FMT_INVALID;
      m_source_file_type = texture_file_types::cFormatInvalid;
      m_last_error.clear();
   }

   void dds_texture::free_all_mips()
   {
      for (uint i = 0; i < m_faces.size(); i++)
         for (uint j = 0; j < m_faces[i].size(); j++)
            crnlib_delete(m_faces[i][j]);

      m_faces.clear();
   }

   dds_texture::dds_texture(const dds_texture& other) :
      m_width(0),
      m_height(0),
      m_comp_flags(pixel_format_helpers::cDefaultCompFlags),
      m_format(PIXEL_FMT_INVALID)
   {
      *this = other;
   }

   dds_texture& dds_texture::operator= (const dds_texture& rhs)
   {
      if (this == &rhs)
         return *this;

      clear();

      m_name = rhs.m_name;
      m_width = rhs.m_width;
      m_height = rhs.m_height;

      m_comp_flags = rhs.m_comp_flags;
      m_format = rhs.m_format;

      m_faces.resize(rhs.m_faces.size());
      for (uint i = 0; i < m_faces.size(); i++)
      {
         m_faces[i].resize(rhs.m_faces[i].size());

         for (uint j = 0; j < rhs.m_faces[i].size(); j++)
            m_faces[i][j] = crnlib_new<mip_level>(*rhs.m_faces[i][j]);
      }

      CRNLIB_ASSERT((!is_valid()) || check());

      return *this;
   }

   bool dds_texture::write_dds(const wchar_t* pFilename) const
   {
      cfile_stream out_stream(pFilename, cDataStreamWritable | cDataStreamSeekable);
      if (!out_stream.is_opened())
         return false;

      data_stream_serializer out_serializer(out_stream);
      return write_dds(out_serializer);
   }

   bool dds_texture::read_dds(const wchar_t* pFilename)
   {
      cfile_stream in_stream(pFilename);
      if (!in_stream.is_opened())
         return false;

      data_stream_serializer in_serializer(in_stream);
      return read_dds(in_serializer);
   }

   bool dds_texture::read_dds(data_stream_serializer& serializer)
   {
      if (!read_dds_internal(serializer))
      {
         clear();
         return false;
      }

      return true;
   }

   bool dds_texture::read_dds_internal(data_stream_serializer& serializer)
   {
      CRNLIB_ASSERT(serializer.get_little_endian());

      clear();

      set_last_error(L"Not a DDS file");

      uint8 hdr[4];
      if (!serializer.read(hdr, sizeof(hdr)))
         return false;

      if (memcmp(hdr, "DDS ", 4) != 0)
         return false;

      DDSURFACEDESC2 desc;
      if (!serializer.read(&desc, sizeof(desc)))
         return false;

      if (!c_crnlib_little_endian_platform)
         utils::endian_switch_dwords(reinterpret_cast<uint32*>(&desc), sizeof(desc) / sizeof(uint32));

      if (desc.dwSize != sizeof(desc))
         return false;

      if ((!desc.dwHeight) || (!desc.dwWidth) || (desc.dwHeight > cDDSMaxImageDimensions) || (desc.dwWidth > cDDSMaxImageDimensions))
         return false;

      m_width = desc.dwWidth;
      m_height = desc.dwHeight;

      uint num_mip_levels = 1;

      if ((desc.dwFlags & DDSD_MIPMAPCOUNT) && (desc.ddsCaps.dwCaps & DDSCAPS_MIPMAP) && (desc.dwMipMapCount))
      {
         num_mip_levels = desc.dwMipMapCount;
         if (num_mip_levels > utils::compute_max_mips(desc.dwWidth, desc.dwHeight))
            return false;
      }

      uint num_faces = 1;

      if (desc.ddsCaps.dwCaps & DDSCAPS_COMPLEX)
      {
         if (desc.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP)
         {
            const uint all_faces_mask = DDSCAPS2_CUBEMAP_POSITIVEX|DDSCAPS2_CUBEMAP_NEGATIVEX|DDSCAPS2_CUBEMAP_POSITIVEY|DDSCAPS2_CUBEMAP_NEGATIVEY|DDSCAPS2_CUBEMAP_POSITIVEZ|DDSCAPS2_CUBEMAP_NEGATIVEZ;
            if ((desc.ddsCaps.dwCaps2 & all_faces_mask) != all_faces_mask)
            {
               set_last_error(L"Incomplete cubemaps unsupported");
               return false;
            }

            num_faces = 6;
         }
         else if (desc.ddsCaps.dwCaps2 & DDSCAPS2_VOLUME)
         {
            set_last_error(L"Volume textures unsupported");
            return false;
         }
      }

      if (desc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
      {
         // It's difficult to even make P8 textures with existing tools:
         // nvdxt just hangs
         // dxtex.exe just makes all-white textures
         // So screw it.
         set_last_error(L"Palettized textures unsupported");
         return false;
      }

      dxt_format dxt_fmt = cDXTInvalid;

      if (desc.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
      {
         // http://code.google.com/p/nvidia-texture-tools/issues/detail?id=41
         // ATI2 YX:            0 (0x00000000)
         // ATI2 XY:   1498952257 (0x59583241) (BC5)
         // ATI Compressonator obeys this stuff, nvidia's tools (like readdxt) don't - oh great

         switch (desc.ddpfPixelFormat.dwFourCC)
         {
            case PIXEL_FMT_DXT1:
            {
               m_format = PIXEL_FMT_DXT1;
               dxt_fmt = cDXT1;
               break;
            }
            case PIXEL_FMT_DXT2:
            case PIXEL_FMT_DXT3:
            {
               m_format = PIXEL_FMT_DXT3;
               dxt_fmt = cDXT3;
               break;
            }
            case PIXEL_FMT_DXT4:
            case PIXEL_FMT_DXT5:
            {
               switch (desc.ddpfPixelFormat.dwRGBBitCount)
               {
                  case PIXEL_FMT_DXT5_CCxY:
                     m_format = PIXEL_FMT_DXT5_CCxY;
                     break;
                  case PIXEL_FMT_DXT5_xGxR:
                     m_format = PIXEL_FMT_DXT5_xGxR;
                     break;
                  case PIXEL_FMT_DXT5_xGBR:
                     m_format = PIXEL_FMT_DXT5_xGBR;
                     break;
                  case PIXEL_FMT_DXT5_AGBR:
                     m_format = PIXEL_FMT_DXT5_AGBR;
                     break;
                  default:
                     m_format = PIXEL_FMT_DXT5;
                     break;
               }

               dxt_fmt = cDXT5;
               break;
            }
            case PIXEL_FMT_3DC:
            {
               if (desc.ddpfPixelFormat.dwRGBBitCount == CRNLIB_PIXEL_FMT_FOURCC('A', '2', 'X', 'Y'))
               {
                  dxt_fmt = cDXN_XY;
                  m_format = PIXEL_FMT_DXN;
               }
               else
               {
                  dxt_fmt = cDXN_YX; // aka ATI2
                  m_format = PIXEL_FMT_3DC;
               }

               break;
            }
            case PIXEL_FMT_DXT5A:
            {
               m_format = PIXEL_FMT_DXT5A;
               dxt_fmt = cDXT5A;
               break;
            }
            default:
            {
               dynamic_wstring err_msg(cVarArg, L"Unsupported DDS FOURCC format: 0x%08X", desc.ddpfPixelFormat.dwFourCC);
               set_last_error(err_msg.get_ptr());
               return false;
            }
         }
      }
      else if ((desc.ddpfPixelFormat.dwRGBBitCount < 8) || (desc.ddpfPixelFormat.dwRGBBitCount > 32) || (desc.ddpfPixelFormat.dwRGBBitCount & 7))
      {
         set_last_error(L"Unsupported bit count");
         return false;
      }
      else if (desc.ddpfPixelFormat.dwFlags & DDPF_RGB)
      {
         if (desc.ddpfPixelFormat.dwFlags & DDPF_LUMINANCE)
         {
            if (desc.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
               m_format = PIXEL_FMT_A8L8;
            else
               m_format = PIXEL_FMT_L8;
         }
         else if (desc.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
            m_format = PIXEL_FMT_A8R8G8B8;
         else
            m_format = PIXEL_FMT_R8G8B8;
      }
      else if (desc.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
      {
         if (desc.ddpfPixelFormat.dwFlags & DDPF_LUMINANCE)
            m_format = PIXEL_FMT_A8L8;
         else
            m_format = PIXEL_FMT_A8;
      }
      else if (desc.ddpfPixelFormat.dwFlags & DDPF_LUMINANCE)
      {
         m_format = PIXEL_FMT_L8;
      }
      else if (desc.ddpfPixelFormat.dwFlags & DDPF_ALPHA)
      {
         m_format = PIXEL_FMT_A8;
      }
      else
      {
         set_last_error(L"Unsupported format");
         return false;
      }

      m_comp_flags = pixel_format_helpers::get_component_flags(m_format);

      uint bits_per_pixel = desc.ddpfPixelFormat.dwRGBBitCount;

      if (desc.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
         //bits_per_pixel = ((m_format == PIXEL_FMT_DXT1) || (m_format == PIXEL_FMT_DXT5A)) ? 4 : 8;
         bits_per_pixel = pixel_format_helpers::get_bpp(m_format);

      set_last_error(L"Load failed");

      uint default_pitch;
      if (desc.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
         default_pitch = (((desc.dwWidth + 3) & ~3) * ((desc.dwHeight + 3) & ~3) * bits_per_pixel) >> 3;
      else
         default_pitch = (desc.dwWidth * bits_per_pixel) >> 3;

      uint pitch = desc.lPitch;
      if (!pitch)
         pitch = default_pitch;
      else if ((pitch > default_pitch * 8) || (pitch & 3))
      {
         set_last_error(L"Invalid pitch");
         return false;
      }

      crnlib::vector<uint8> load_buf;

      uint mask_size[4];
      mask_size[0] = math::bitmask_size(desc.ddpfPixelFormat.dwRBitMask);
      mask_size[1] = math::bitmask_size(desc.ddpfPixelFormat.dwGBitMask);
      mask_size[2] = math::bitmask_size(desc.ddpfPixelFormat.dwBBitMask);
      mask_size[3] = math::bitmask_size(desc.ddpfPixelFormat.dwRGBAlphaBitMask);

      uint mask_ofs[4];
      mask_ofs[0] = math::bitmask_ofs(desc.ddpfPixelFormat.dwRBitMask);
      mask_ofs[1] = math::bitmask_ofs(desc.ddpfPixelFormat.dwGBitMask);
      mask_ofs[2] = math::bitmask_ofs(desc.ddpfPixelFormat.dwBBitMask);
      mask_ofs[3] = math::bitmask_ofs(desc.ddpfPixelFormat.dwRGBAlphaBitMask);

      if ((desc.ddpfPixelFormat.dwFlags & DDPF_LUMINANCE) && (!mask_size[0]))
      {
         mask_size[0] = desc.ddpfPixelFormat.dwRGBBitCount >> 3;
         if (desc.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
            mask_size[0] /= 2;
      }

      m_faces.resize(num_faces);

      bool dxt1_alpha = false;

      for (uint face_index = 0; face_index < num_faces; face_index++)
      {
         m_faces[face_index].resize(num_mip_levels);

         for (uint level_index = 0; level_index < num_mip_levels; level_index++)
         {
            const uint width = math::maximum<uint>(desc.dwWidth >> level_index, 1U);
            const uint height = math::maximum<uint>(desc.dwHeight >> level_index, 1U);

            mip_level* pMip = crnlib_new<mip_level>();
            m_faces[face_index][level_index] = pMip;

            if (desc.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
            {
               const uint bytes_per_block = ((m_format == PIXEL_FMT_DXT1) || (m_format == PIXEL_FMT_DXT1A) || (m_format == PIXEL_FMT_DXT5A)) ? 8 : 16;

               const uint num_blocks_x = (width + 3) >> 2;
               const uint num_blocks_y = (height + 3) >> 2;

               const uint actual_level_pitch = num_blocks_x * num_blocks_y * bytes_per_block;
               const uint level_pitch = level_index ? actual_level_pitch : pitch;

               dxt_image* pDXTImage = crnlib_new<dxt_image>();
               if (!pDXTImage->init(dxt_fmt, width, height, false))
               {
                  crnlib_delete(pDXTImage);

                  CRNLIB_ASSERT(0);
                  return false;
               }

               CRNLIB_ASSERT(pDXTImage->get_element_vec().size() * sizeof(dxt_image::element) == actual_level_pitch);

               if (!serializer.read(&pDXTImage->get_element_vec()[0], actual_level_pitch))
               {
                  crnlib_delete(pDXTImage);

                  return false;
               }

               // DDS image in memory are always assumed to be little endian - the same as DDS itself.
               //if (c_crnlib_big_endian_platform)
               //   utils::endian_switch_words(reinterpret_cast<uint16*>(&pDXTImage->get_element_vec()[0]), actual_level_pitch / sizeof(uint16));

               if (level_pitch > actual_level_pitch)
               {
                  if (!serializer.skip(level_pitch - actual_level_pitch))
                  {
                     crnlib_delete(pDXTImage);

                     return false;
                  }
               }

               if ((m_format == PIXEL_FMT_DXT1) && (!dxt1_alpha))
                  dxt1_alpha = pDXTImage->has_alpha();

               pMip->assign(pDXTImage, m_format);
            }
            else
            {
               image_u8* pImage = crnlib_new<image_u8>(width, height);

               pImage->set_comp_flags(m_comp_flags);

               const uint bytes_per_pixel = desc.ddpfPixelFormat.dwRGBBitCount >> 3;
               const uint actual_line_pitch = width * bytes_per_pixel;
               const uint line_pitch = level_index ? actual_line_pitch : pitch;

               if (load_buf.size() < line_pitch)
                  load_buf.resize(line_pitch);

               color_quad_u8 q(0, 0, 0, 255);

               for (uint y = 0; y < height; y++)
               {
                  if (!serializer.read(&load_buf[0], line_pitch))
                  {
                     crnlib_delete(pImage);
                     return false;
                  }

                  color_quad_u8* pDst = pImage->get_scanline(y);

                  for (uint x = 0; x < width; x++)
                  {
                     const uint8* pPixel = &load_buf[x * bytes_per_pixel];

                     uint c = 0;
                     // Assumes DDS is always little endian.
                     for (uint l = 0; l < bytes_per_pixel; l++)
                        c |= (pPixel[l] << (l * 8U));

                     for (uint i = 0; i < 4; i++)
                     {
                        if (!mask_size[i])
                           continue;

                        uint mask = (1U << mask_size[i]) - 1U;
                        uint bits = (c >> mask_ofs[i]) & mask;

                        uint v = (bits * 255 + (mask >> 1)) / mask;

                        q.set_component(i, v);
                     }

                     if (desc.ddpfPixelFormat.dwFlags & DDPF_LUMINANCE)
                     {
                        q.g = q.r;
                        q.b = q.r;
                     }

                     *pDst++ = q;
                  }
               }

               pMip->assign(pImage, m_format);

               CRNLIB_ASSERT(pMip->get_comp_flags() == m_comp_flags);
            }
         }
      }

      clear_last_error();

      if (dxt1_alpha)
      {
         m_format = PIXEL_FMT_DXT1A;

         m_comp_flags = pixel_format_helpers::get_component_flags(m_format);

         for (uint f = 0; f < m_faces.size(); f++)
         {
            for (uint l = 0; l < m_faces[f].size(); l++)
            {
               if (m_faces[f][l]->get_dxt_image())
               {
                  m_faces[f][l]->set_format(m_format);
                  m_faces[f][l]->set_comp_flags(m_comp_flags);

                  m_faces[f][l]->get_dxt_image()->change_dxt1_to_dxt1a();
               }
            }
         }
      }

      CRNLIB_ASSERT(check());

      return true;
   }

   bool dds_texture::check() const
   {
      uint levels = 0;
      for (uint f = 0; f < m_faces.size(); f++)
      {
         if (!f)
            levels = m_faces[f].size();
         else if (m_faces[f].size() != levels)
            return false;

         for (uint l = 0; l < m_faces[f].size(); l++)
         {
            mip_level* p = m_faces[f][l];
            if (!p)
               return false;

            if (!p->is_valid())
               return false;

            if (!l)
            {
               if (m_width != p->get_width())
                  return false;
               if (m_height != p->get_height())
                  return false;
            }

            if (p->get_comp_flags() != m_comp_flags)
               return false;

            if (p->get_format() != m_format)
               return false;

            if (p->get_image())
            {
               if (pixel_format_helpers::is_dxt(p->get_format()))
                  return false;

               if (p->get_image()->get_width() != p->get_width())
                  return false;
               if (p->get_image()->get_height() != p->get_height())
                  return false;
               if (p->get_image()->get_comp_flags() != m_comp_flags)
                  return false;
            }
            else if (!pixel_format_helpers::is_dxt(p->get_format()))
               return false;
         }
      }

      return true;
   }

   bool dds_texture::write_dds(data_stream_serializer& serializer) const
   {
      if (!m_width)
      {
         set_last_error(L"Nothing to write");
         return false;
      }

      set_last_error(L"Write_dds() failed");

      if (!serializer.write("DDS ", sizeof(uint32)))
         return false;

      DDSURFACEDESC2 desc;
      utils::zero_object(desc);

      desc.dwSize = sizeof(desc);
      desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;

      desc.dwWidth = m_width;
      desc.dwHeight = m_height;

      desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
      desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);

      if (get_num_levels() > 1)
      {
         desc.dwMipMapCount = get_num_levels();
         desc.dwFlags |= DDSD_MIPMAPCOUNT;
         desc.ddsCaps.dwCaps |= (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX);
      }

      if (get_num_faces() > 1)
      {
         desc.ddsCaps.dwCaps |= DDSCAPS_COMPLEX;
         desc.ddsCaps.dwCaps2 |= DDSCAPS2_CUBEMAP;
         desc.ddsCaps.dwCaps2 |= DDSCAPS2_CUBEMAP_POSITIVEX|DDSCAPS2_CUBEMAP_NEGATIVEX|DDSCAPS2_CUBEMAP_POSITIVEY|DDSCAPS2_CUBEMAP_NEGATIVEY|DDSCAPS2_CUBEMAP_POSITIVEZ|DDSCAPS2_CUBEMAP_NEGATIVEZ;
      }

      bool dxt_format = false;
      if (pixel_format_helpers::is_dxt(m_format))
      {
         dxt_format = true;

         desc.ddpfPixelFormat.dwFlags |= DDPF_FOURCC;

         switch (m_format)
         {
            case PIXEL_FMT_DXN:
            {
               desc.ddpfPixelFormat.dwFourCC = (uint32)PIXEL_FMT_3DC;
               desc.ddpfPixelFormat.dwRGBBitCount = PIXEL_FMT_DXN;
               break;
            }
            case PIXEL_FMT_DXT1A:
            {
               desc.ddpfPixelFormat.dwFourCC = (uint32)PIXEL_FMT_DXT1;
               desc.ddpfPixelFormat.dwRGBBitCount = 0;
               break;
            }
            case PIXEL_FMT_DXT5_CCxY:
            {
               desc.ddpfPixelFormat.dwFourCC = (uint32)PIXEL_FMT_DXT5;
               desc.ddpfPixelFormat.dwRGBBitCount = (uint32)PIXEL_FMT_DXT5_CCxY;
               break;
            }
            case PIXEL_FMT_DXT5_xGxR:
            {
               desc.ddpfPixelFormat.dwFourCC = (uint32)PIXEL_FMT_DXT5;
               desc.ddpfPixelFormat.dwRGBBitCount = (uint32)PIXEL_FMT_DXT5_xGxR;
               break;
            }
            case PIXEL_FMT_DXT5_xGBR:
            {
               desc.ddpfPixelFormat.dwFourCC = (uint32)PIXEL_FMT_DXT5;
               desc.ddpfPixelFormat.dwRGBBitCount = (uint32)PIXEL_FMT_DXT5_xGBR;
               break;
            }
            case PIXEL_FMT_DXT5_AGBR:
            {
               desc.ddpfPixelFormat.dwFourCC = (uint32)PIXEL_FMT_DXT5;
               desc.ddpfPixelFormat.dwRGBBitCount = (uint32)PIXEL_FMT_DXT5_AGBR;
               break;
            }
            default:
            {
               desc.ddpfPixelFormat.dwFourCC = (uint32)m_format;
               desc.ddpfPixelFormat.dwRGBBitCount = 0;
               break;
            }
         }
      }
      else
      {
         switch (m_format)
         {
            case PIXEL_FMT_A8R8G8B8:
            {
               desc.ddpfPixelFormat.dwFlags |= (DDPF_RGB | DDPF_ALPHAPIXELS);
               desc.ddpfPixelFormat.dwRGBBitCount = 32;
               desc.ddpfPixelFormat.dwRBitMask = 0xFF0000;
               desc.ddpfPixelFormat.dwGBitMask = 0x00FF00;
               desc.ddpfPixelFormat.dwBBitMask = 0x0000FF;
               desc.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
               break;
            }
            case PIXEL_FMT_R8G8B8:
            {
               desc.ddpfPixelFormat.dwFlags |= DDPF_RGB;
               desc.ddpfPixelFormat.dwRGBBitCount = 24;
               desc.ddpfPixelFormat.dwRBitMask = 0xFF0000;
               desc.ddpfPixelFormat.dwGBitMask = 0x00FF00;
               desc.ddpfPixelFormat.dwBBitMask = 0x0000FF;
               break;
            }
            case PIXEL_FMT_A8:
            {
               desc.ddpfPixelFormat.dwFlags |= DDPF_ALPHA;
               desc.ddpfPixelFormat.dwRGBBitCount = 8;
               desc.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF;
               break;
            }
            case PIXEL_FMT_L8:
            {
               desc.ddpfPixelFormat.dwFlags |= DDPF_LUMINANCE;
               desc.ddpfPixelFormat.dwRGBBitCount = 8;
               desc.ddpfPixelFormat.dwRBitMask = 0xFF;
               break;
            }
            case PIXEL_FMT_A8L8:
            {
               desc.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS | DDPF_LUMINANCE;
               desc.ddpfPixelFormat.dwRGBBitCount = 16;
               desc.ddpfPixelFormat.dwRBitMask = 0xFF;
               desc.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF00;
               break;
            }
            default:
            {
               CRNLIB_ASSERT(false);
               return false;
            }
         }
      }

      if (!c_crnlib_little_endian_platform)
         utils::endian_switch_dwords(reinterpret_cast<uint32*>(&desc), sizeof(desc) / sizeof(uint32));

      if (!serializer.write(&desc, sizeof(desc)))
         return false;

      if (!c_crnlib_little_endian_platform)
         utils::endian_switch_dwords(reinterpret_cast<uint32*>(&desc), sizeof(desc) / sizeof(uint32));

      crnlib::vector<uint8> write_buf;

      for (uint face = 0; face < get_num_faces(); face++)
      {
         for (uint level = 0; level < get_num_levels(); level++)
         {
            const mip_level* pLevel = get_level(face, level);

            if (dxt_format)
            {
               const uint width = pLevel->get_width();
               const uint height = pLevel->get_height();

               CRNLIB_ASSERT(width == math::maximum<uint>(1, m_width >> level));
               CRNLIB_ASSERT(height == math::maximum<uint>(1, m_height >> level));

               const dxt_image* p = pLevel->get_dxt_image();

               const uint num_blocks_x = (width + 3) >> 2;
               const uint num_blocks_y = (height + 3) >> 2;

               CRNLIB_ASSERT(num_blocks_x * num_blocks_y * p->get_elements_per_block() == p->get_num_elements());
               width, height, num_blocks_x, num_blocks_y;

               const uint size_in_bytes = p->get_num_elements() * sizeof(dxt_image::element);
               if (size_in_bytes > write_buf.size())
                  write_buf.resize(size_in_bytes);

               memcpy(&write_buf[0], p->get_element_ptr(), size_in_bytes);

               // DXT data is always little endian in memory, just like the DDS format.
               //if (!c_crnlib_little_endian_platform)
               //   utils::endian_switch_words(reinterpret_cast<WORD*>(&write_buf[0]), size_in_bytes / sizeof(WORD));

               if (!serializer.write(&write_buf[0], size_in_bytes))
                  return false;
            }
            else
            {
               const uint width = pLevel->get_width();
               const uint height = pLevel->get_height();

               const image_u8* p = pLevel->get_image();

               const uint bits_per_pixel = desc.ddpfPixelFormat.dwRGBBitCount;
               const uint bytes_per_pixel = bits_per_pixel >> 3;

               const uint pitch = width * bytes_per_pixel;
               if (pitch > write_buf.size())
                  write_buf.resize(pitch);

               for (uint y = 0; y < height; y++)
               {
                  const color_quad_u8* pSrc = p->get_scanline(y);
                  const color_quad_u8* pEnd = pSrc + width;

                  uint8* pDst = &write_buf[0];

                  do
                  {
                     const color_quad_u8& c = *pSrc;

                     uint x = 0;
                     switch (m_format)
                     {
                        case PIXEL_FMT_A8R8G8B8:
                        {
                           x = (c.a << 24) | (c.r << 16) | (c.g << 8) | c.b;
                           break;
                        }
                        case PIXEL_FMT_R8G8B8:
                        {
                           x = (c.r << 16) | (c.g << 8) | c.b;
                           break;
                        }
                        case PIXEL_FMT_A8:
                        {
                           x = c.a;
                           break;
                        }
                        case PIXEL_FMT_A8L8:
                        {
                           x = (c.a << 8) | c.get_luma();
                           break;
                        }
                        case PIXEL_FMT_L8:
                        {
                           x = c.get_luma();
                           break;
                        }
                        default: break;
                     }

                     pDst[0] = static_cast<uint8>(x);
                     if (bytes_per_pixel > 1)
                     {
                        pDst[1] = static_cast<uint8>(x >> 8);

                        if (bytes_per_pixel > 2)
                        {
                           pDst[2] = static_cast<uint8>(x >> 16);

                           if (bytes_per_pixel > 3)
                              pDst[3] = static_cast<uint8>(x >> 24);
                        }
                     }

                     pSrc++;
                     pDst += bytes_per_pixel;

                  } while (pSrc != pEnd);

                  if (!serializer.write(&write_buf[0], pitch))
                     return false;
               }
            }
         }
      }

      clear_last_error();

      return true;
   }

   void dds_texture::assign(face_vec& faces)
   {
      CRNLIB_ASSERT(!faces.empty());
      if (faces.empty())
         return;

      free_all_mips();

#ifdef CRNLIB_BUILD_DEBUG
      for (uint i = 1; i < faces.size(); i++)
         CRNLIB_ASSERT(faces[i].size() == faces[0].size());
#endif

      mip_level* p = faces[0][0];
      m_width = p->get_width();
      m_height = p->get_height();
      m_comp_flags = p->get_comp_flags();
      m_format = p->get_format();

      m_faces.swap(faces);

      CRNLIB_ASSERT(check());
   }

   void dds_texture::assign(mip_level* pLevel)
   {
      face_vec faces(1, mip_ptr_vec(1, pLevel));
      assign(faces);
   }

   void dds_texture::assign(image_u8* p, pixel_format fmt)
   {
      mip_level* pLevel = crnlib_new<mip_level>();
      pLevel->assign(p, fmt);
      assign(pLevel);
   }

   void dds_texture::assign(dxt_image* p, pixel_format fmt)
   {
      mip_level* pLevel = crnlib_new<mip_level>();
      pLevel->assign(p, fmt);
      assign(pLevel);
   }

   void dds_texture::set(texture_file_types::format source_file_type, const dds_texture& dds_texture)
   {
      clear();

      *this = dds_texture;
      m_source_file_type = source_file_type;
   }

   image_u8* dds_texture::get_level_image(uint face, uint level, image_u8& img, bool uncook) const
   {
      if (!is_valid())
         return NULL;

      const mip_level* pLevel = get_level(face, level);

      return pLevel->get_unpacked_image(img, uncook);
   }

   void dds_texture::swap(dds_texture& img)
   {
      utils::swap(m_width, img.m_width);
      utils::swap(m_height, img.m_height);
      utils::swap(m_comp_flags, img.m_comp_flags);
      utils::swap(m_format, img.m_format);
      m_faces.swap(img.m_faces);
      m_last_error.swap(img.m_last_error);
      utils::swap(m_source_file_type, img.m_source_file_type);

      CRNLIB_ASSERT(check());
   }

   texture_type dds_texture::determine_texture_type() const
   {
      if (!is_valid())
         return cTextureTypeUnknown;

      if (get_num_faces() == 6)
         return cTextureTypeCubemap;
      else if (is_vertical_cross())
         return cTextureTypeVerticalCrossCubemap;
      else if (is_normal_map())
         return cTextureTypeNormalMap;

      return cTextureTypeRegularMap;
   }

   void dds_texture::discard_mips()
   {
      for (uint f = 0; f < m_faces.size(); f++)
      {
         if (m_faces[f].size() > 1)
         {
            for (uint l = 1; l < m_faces[f].size(); l++)
               crnlib_delete(m_faces[f][l]);

            m_faces[f].resize(1);
         }
      }

      CRNLIB_ASSERT(check());
   }

   void dds_texture::init(uint width, uint height, uint levels, uint faces, pixel_format fmt, const wchar_t* pName)
   {
      clear();

      CRNLIB_ASSERT((width > 0) && (height > 0) && (levels > 0));
      CRNLIB_ASSERT((faces == 1) || (faces == 6));

      m_width = width;
      m_height = height;
      m_comp_flags = pixel_format_helpers::get_component_flags(fmt);
      m_format = fmt;
      if (pName)
         m_name.set(pName);

      m_faces.resize(faces);
      for (uint f = 0; f < faces; f++)
      {
         m_faces[f].resize(levels);
         for (uint l = 0; l < levels; l++)
         {
            m_faces[f][l] = crnlib_new<mip_level>();

            const uint mip_width = math::maximum(1U, width >> l);
            const uint mip_height = math::maximum(1U, height >> l);
            if (pixel_format_helpers::is_dxt(fmt))
            {
               dxt_image* p = crnlib_new<dxt_image>();
               p->init(pixel_format_helpers::get_dxt_format(fmt), mip_width, mip_height, true);
               m_faces[f][l]->assign(p, m_format);
            }
            else
            {
               image_u8* p = crnlib_new<image_u8>(mip_width, mip_height);
               m_faces[f][l]->assign(p, m_format);
            }
         }
      }

      CRNLIB_ASSERT(check());
   }

   void dds_texture::discard_mipmaps()
   {
      if (!is_valid())
         return;

      discard_mips();
   }

   bool dds_texture::convert(pixel_format fmt, bool cook, const dxt_image::pack_params& p)
   {
      if (!is_valid())
         return false;

      if (fmt == get_format())
         return true;

      uint total_pixels = 0;
      for (uint f = 0; f < m_faces.size(); f++)
         for (uint l = 0; l < m_faces[f].size(); l++)
            total_pixels += m_faces[f][l]->get_total_pixels();

      uint num_pixels_processed = 0;

      uint progress_start = p.m_progress_start;

      for (uint f = 0; f < m_faces.size(); f++)
      {
         for (uint l = 0; l < m_faces[f].size(); l++)
         {
            const uint num_pixels = m_faces[f][l]->get_total_pixels();

            uint progress_range = (num_pixels * p.m_progress_range) / total_pixels;

            dxt_image::pack_params tmp_params(p);
            tmp_params.m_progress_start = math::clamp<uint>(progress_start, 0, p.m_progress_range);
            tmp_params.m_progress_range = math::clamp<uint>(progress_range, 0, p.m_progress_range - tmp_params.m_progress_start);

            progress_start += tmp_params.m_progress_range;

            if (!m_faces[f][l]->convert(fmt, cook, tmp_params))
            {
               clear();
               return false;
            }

            num_pixels_processed += num_pixels;
         }
      }

      m_format = get_level(0, 0)->get_format();
      m_comp_flags = get_level(0, 0)->get_comp_flags();

      CRNLIB_ASSERT(check());

      if (p.m_pProgress_callback)
      {
         if (!p.m_pProgress_callback(p.m_progress_start + p.m_progress_range, p.m_pProgress_callback_user_data_ptr))
            return false;
      }

      return true;
   }

   bool dds_texture::convert(pixel_format fmt, const dxt_image::pack_params& p)
   {
      return convert(fmt, true, p);
   }

   bool dds_texture::convert(pixel_format fmt, bool cook, const dxt_image::pack_params& p, int qdxt_quality, bool hierarchical)
   {
      if ((!pixel_format_helpers::is_dxt(fmt)) || (fmt == PIXEL_FMT_DXT3))
      {
         // QDXT doesn't support DXT3.
         return convert(fmt, cook, p);
      }

      dds_texture src_tex(*this);

      if (src_tex.is_packed())
         src_tex.unpack_from_dxt(true);

      if (cook)
      {
         dds_texture cooked_tex(src_tex);

         for (uint f = 0; f < m_faces.size(); f++)
            for (uint l = 0; l < m_faces[f].size(); l++)
               src_tex.m_faces[f][l]->cook_image(*cooked_tex.m_faces[f][l]->get_image());

         src_tex.swap(cooked_tex);
      }

      qdxt1_params q1_params;
      q1_params.init(p, qdxt_quality, hierarchical);

      qdxt5_params q5_params;
      q5_params.init(p, qdxt_quality, hierarchical);

      if (pixel_format_helpers::is_pixel_format_non_srgb(fmt) || (m_comp_flags & pixel_format_helpers::cCompFlagNormalMap) || (m_comp_flags & pixel_format_helpers::cCompFlagLumaChroma))
      {
         // Disable perceptual colorspace metrics when packing to swizzled or non-RGB pixel formats.
         q1_params.m_perceptual = false;
      }

      task_pool tp;
      if (!tp.init(p.m_num_helper_threads))
         return false;

      dds_texture packed_tex;

      qdxt_state state(tp);
      if (!src_tex.qdxt_pack_init(state, packed_tex, q1_params, q5_params, fmt, false))
         return false;

      if (!src_tex.qdxt_pack(state, packed_tex, q1_params, q5_params))
         return false;

      swap(packed_tex);

      return true;
   }

   bool dds_texture::is_packed() const
   {
      CRNLIB_ASSERT(is_valid());
      if (!is_valid())
         return false;

      return get_level(0, 0)->is_packed();
   }

   bool dds_texture::set_alpha_to_luma()
   {
      CRNLIB_ASSERT(is_valid());
      if (!is_valid())
         return false;

      if (is_packed())
         unpack_from_dxt(true);

      for (uint f = 0; f < m_faces.size(); f++)
         for (uint l = 0; l < get_num_levels(); l++)
            get_level(f, l)->set_alpha_to_luma();

      m_format = get_level(0, 0)->get_format();
      m_comp_flags = get_level(0, 0)->get_comp_flags();

      CRNLIB_ASSERT(check());

      return true;
   }

   bool dds_texture::convert(image_utils::conversion_type conv_type)
   {
      CRNLIB_ASSERT(is_valid());
      if (!is_valid())
         return false;

      if (is_packed())
         unpack_from_dxt(true);

      for (uint f = 0; f < m_faces.size(); f++)
         for (uint l = 0; l < get_num_levels(); l++)
            get_level(f, l)->convert(conv_type);

      m_format = get_level(0, 0)->get_format();
      m_comp_flags = get_level(0, 0)->get_comp_flags();

      CRNLIB_ASSERT(check());

      return true;
   }

   bool dds_texture::unpack_from_dxt(bool uncook)
   {
      CRNLIB_ASSERT(is_valid());
      if (!is_valid())
         return false;

      CRNLIB_ASSERT(pixel_format_helpers::is_dxt(m_format));
      if (!pixel_format_helpers::is_dxt(m_format))
         return false;

      for (uint f = 0; f < m_faces.size(); f++)
         for (uint l = 0; l < get_num_levels(); l++)
            if (!get_level(f, l)->unpack_from_dxt(uncook))
               return false;

      m_format = get_level(0, 0)->get_format();
      m_comp_flags = get_level(0, 0)->get_comp_flags();

      CRNLIB_ASSERT(check());

      return true;
   }

   bool dds_texture::has_alpha() const
   {
      CRNLIB_ASSERT(is_valid());
      if (!is_valid())
         return false;

      if (pixel_format_helpers::has_alpha(m_format))
         return true;

      if ((m_format == PIXEL_FMT_DXT1) && (get_level(0, 0)->get_dxt_image()))
      {
         // Try scanning DXT1 mip levels to find blocks with transparent pixels.
         for (uint f = 0; f < get_num_faces(); f++)
            if (get_level(f, 0)->get_dxt_image()->has_alpha())
               return true;
      }

      return false;
   }

   bool dds_texture::is_normal_map() const
   {
      CRNLIB_ASSERT(is_valid());
      if (!is_valid())
         return false;

      if (pixel_format_helpers::is_normal_map(get_format()))
         return true;

      const mip_level* pLevel = get_level(0, 0);

      if (pLevel->get_image())
         return image_utils::is_normal_map(*pLevel->get_image(), m_name.get_ptr());

      image_u8 tmp;
      pLevel->get_dxt_image()->unpack(tmp);
      return image_utils::is_normal_map(tmp, m_name.get_ptr());
   }

   bool dds_texture::is_vertical_cross() const
   {
      CRNLIB_ASSERT(is_valid());
      if (!is_valid())
         return false;

      if (get_num_faces() > 1)
         return false;

      if (!((math::is_power_of_2(m_height)) && (!math::is_power_of_2(m_width)) && (m_height / 4U == m_width / 3U)))
         return false;

      return true;
   }

   bool dds_texture::resize(uint new_width, uint new_height, const resample_params& params)
   {
      CRNLIB_ASSERT(is_valid());
      if (!is_valid())
         return false;

      CRNLIB_ASSERT((new_width >= 1) && (new_height >= 1));

      face_vec faces(get_num_faces());
      for (uint f = 0; f < faces.size(); f++)
      {
         faces[f].resize(1);
         faces[f][0] = crnlib_new<mip_level>();
      }

      for (uint f = 0; f < faces.size(); f++)
      {
         image_u8 tmp;
         image_u8* pImg = get_level(f, 0)->get_unpacked_image(tmp, true);

         image_u8* pMip = crnlib_new<image_u8>();

         image_utils::resample_params rparams;
         rparams.m_dst_width = new_width;
         rparams.m_dst_height = new_height;
         rparams.m_filter_scale = params.m_filter_scale;
         rparams.m_first_comp = 0;
         rparams.m_num_comps = pImg->is_component_valid(3) ? 4 : 3;
         rparams.m_srgb = params.m_srgb;
         rparams.m_wrapping = params.m_wrapping;
         rparams.m_pFilter = params.m_pFilter;
         rparams.m_multithreaded = params.m_multithreaded;

         if (!image_utils::resample(*pImg, *pMip, rparams))
         {
            crnlib_delete(pMip);

            for (uint f = 0; f < faces.size(); f++)
               for (uint l = 0; l < faces[f].size(); l++)
                  crnlib_delete(faces[f][l]);

            return false;
         }

         if (params.m_renormalize)
            image_utils::renorm_normal_map(*pMip);

         pMip->set_comp_flags(pImg->get_comp_flags());

         faces[f][0]->assign(pMip);
      }

      assign(faces);

      CRNLIB_ASSERT(check());

      return true;
   }

   bool dds_texture::generate_mipmaps(const generate_mipmap_params& params, bool force)
   {
      CRNLIB_ASSERT(is_valid());
      if (!is_valid())
         return false;

      uint num_levels = 1;
      {
         uint width = get_width();
         uint height = get_height();
         while ((width > params.m_min_mip_size) || (height > params.m_min_mip_size))
         {
            width >>= 1U;
            height >>= 1U;
            num_levels++;
         }
      }

      if ((params.m_max_mips > 0) && (num_levels > params.m_max_mips))
         num_levels = params.m_max_mips;

      if ((force) && (get_num_levels() > 1))
         discard_mipmaps();

      if (num_levels == get_num_levels())
         return true;

      face_vec faces(get_num_faces());
      for (uint f = 0; f < faces.size(); f++)
      {
         faces[f].resize(num_levels);
         for (uint l = 0; l < num_levels; l++)
            faces[f][l] = crnlib_new<mip_level>();
      }

      for (uint f = 0; f < faces.size(); f++)
      {
         image_u8 tmp;
         image_u8* pImg = get_level(f, 0)->get_unpacked_image(tmp, true);

         for (uint l = 0; l < num_levels; l++)
         {
            const uint mip_width = math::maximum<uint>(1U, get_width() >> l);
            const uint mip_height = math::maximum<uint>(1U, get_height() >> l);

            image_u8* pMip = crnlib_new<image_u8>();

            if (!l)
               *pMip = *pImg;
            else
            {
               image_utils::resample_params rparams;
               rparams.m_dst_width = mip_width;
               rparams.m_dst_height = mip_height;
               rparams.m_filter_scale = params.m_filter_scale;
               rparams.m_first_comp = 0;
               rparams.m_num_comps = pImg->is_component_valid(3) ? 4 : 3;
               rparams.m_srgb = params.m_srgb;
               rparams.m_wrapping = params.m_wrapping;
               rparams.m_pFilter = params.m_pFilter;
               rparams.m_multithreaded = params.m_multithreaded;

               if (!image_utils::resample(*pImg, *pMip, rparams))
               {
                  crnlib_delete(pMip);

                  for (uint f = 0; f < faces.size(); f++)
                     for (uint l = 0; l < faces[f].size(); l++)
                        crnlib_delete(faces[f][l]);

                  return false;
               }

               if (params.m_renormalize)
                  image_utils::renorm_normal_map(*pMip);

               pMip->set_comp_flags(pImg->get_comp_flags());
            }

            faces[f][l]->assign(pMip);
         }
      }

      assign(faces);

      CRNLIB_ASSERT(check());

      return true;
   }

   bool dds_texture::crop(uint x, uint y, uint width, uint height)
   {
      CRNLIB_ASSERT(is_valid());
      if (!is_valid())
         return false;
      if (get_num_faces() > 1)
         return false;

      if ((width < 1) || (height < 1))
         return false;

      image_u8 tmp;
      image_u8* pImg = get_level(0, 0)->get_unpacked_image(tmp, true);

      image_u8* pMip = crnlib_new<image_u8>(width, height);

      if (!pImg->extract_block(pMip->get_ptr(), x, y, width, height))
         return false;

      face_vec faces(1);
      faces[0].resize(1);
      faces[0][0] = crnlib_new<mip_level>();

      pMip->set_comp_flags(pImg->get_comp_flags());

      faces[0][0]->assign(pMip);

      assign(faces);

      CRNLIB_ASSERT(check());

      return true;
   }

   bool dds_texture::vertical_cross_to_cubemap()
   {
      if (!is_vertical_cross())
         return false;

      const uint face_width = get_height() / 4;

      bool alpha_is_valid = has_alpha();

      dds_texture cubemap;

      pixel_format fmt = alpha_is_valid ? PIXEL_FMT_A8R8G8B8 : PIXEL_FMT_R8G8B8;

      cubemap.init(face_width, face_width, 1, 6, fmt, m_name.get_ptr());

      // +x -x +y -y +z -z
      //     0  1  2
      // 0     +y
      // 1  -x +z +x
      // 2     -y
      // 3     -z

      for (uint face_index = 0; face_index < 6; face_index++)
      {
         const mip_level* pSrc = get_level(0, 0);

         image_u8 tmp_img;
         image_u8* pSrc_image = pSrc->get_unpacked_image(tmp_img, true);

         const mip_level* pDst = get_level(face_index, 0);
         image_u8* pDst_image = pDst->get_image();
         CRNLIB_ASSERT(pDst_image);

         const bool flipped = (face_index == 5);
         const uint x_ofs = g_vertical_cross_image_offsets[face_index][0] * face_width;
         const uint y_ofs = g_vertical_cross_image_offsets[face_index][1] * face_width;

         for (uint y = 0; y < face_width; y++)
         {
            for (uint x = 0; x < face_width; x++)
            {
               const color_quad_u8& c = (*pSrc_image)(x_ofs + x, y_ofs + y);

               if (!flipped)
                  (*pDst_image)(x, y) = c;
               else
                  (*pDst_image)(face_width - 1 - x, face_width - 1 - y) = c;
            }
         }
      }

      swap(cubemap);

      return true;
   }

   bool dds_texture::qdxt_pack_init(qdxt_state& state, dds_texture& dst_tex, const qdxt1_params& dxt1_params, const qdxt5_params& dxt5_params, pixel_format fmt, bool cook)
   {
      if (!is_valid())
         return false;

      state.m_qdxt1_params = dxt1_params;
      state.m_qdxt5_params[0] = dxt5_params;
      state.m_qdxt5_params[1] = dxt5_params;
      utils::zero_object(state.m_has_blocks);

      switch (fmt)
      {
         case PIXEL_FMT_DXT1:
         {
            state.m_has_blocks[0] = true;
            break;
         }
         case PIXEL_FMT_DXT1A:
         {
            state.m_has_blocks[0] = true;
            state.m_qdxt1_params.m_use_alpha_blocks = true;
            break;
         }
         case PIXEL_FMT_DXT4:
         case PIXEL_FMT_DXT5:
         {
            state.m_has_blocks[0] = true;
            state.m_has_blocks[1] = true;
            state.m_qdxt1_params.m_use_alpha_blocks = false;
            state.m_qdxt5_params[0].m_comp_index = 3;
            break;
         }
         case PIXEL_FMT_DXT5_CCxY:
         case PIXEL_FMT_DXT5_xGxR:
         case PIXEL_FMT_DXT5_xGBR:
         case PIXEL_FMT_DXT5_AGBR:
         {
            state.m_has_blocks[0] = true;
            state.m_has_blocks[1] = true;
            state.m_qdxt1_params.m_use_alpha_blocks = false;
            state.m_qdxt1_params.m_perceptual = false;
            state.m_qdxt5_params[0].m_comp_index = 3;
            break;
         }
         case PIXEL_FMT_3DC:
         {
            state.m_has_blocks[1] = true;
            state.m_has_blocks[2] = true;
            state.m_qdxt5_params[0].m_comp_index = 1;
            state.m_qdxt5_params[1].m_comp_index = 0;
            break;
         }
         case PIXEL_FMT_DXN:
         {
            state.m_has_blocks[1] = true;
            state.m_has_blocks[2] = true;
            state.m_qdxt5_params[0].m_comp_index = 0;
            state.m_qdxt5_params[1].m_comp_index = 1;
            break;
         }
         case PIXEL_FMT_DXT5A:
         {
            state.m_has_blocks[1] = true;
            state.m_qdxt5_params[0].m_comp_index = 3;
            break;
         }
         default:
         {
            return false;
         }
      }

      const uint num_elements = state.m_has_blocks[0] + state.m_has_blocks[1] + state.m_has_blocks[2];

      uint cur_progress_start = dxt1_params.m_progress_start;
      if (state.m_has_blocks[0])
      {
         state.m_qdxt1_params.m_progress_start = cur_progress_start;
         state.m_qdxt1_params.m_progress_range = dxt1_params.m_progress_range / num_elements;
         cur_progress_start += state.m_qdxt1_params.m_progress_range;
      }

      if (state.m_has_blocks[1])
      {
         state.m_qdxt5_params[0].m_progress_start = cur_progress_start;
         state.m_qdxt5_params[0].m_progress_range = dxt1_params.m_progress_range / num_elements;
         cur_progress_start += state.m_qdxt5_params[0].m_progress_range;
      }

      if (state.m_has_blocks[2])
      {
         state.m_qdxt5_params[1].m_progress_start = cur_progress_start;
         state.m_qdxt5_params[1].m_progress_range = dxt1_params.m_progress_range - cur_progress_start;
      }

      state.m_fmt = fmt;

      dst_tex.init(get_width(), get_height(), get_num_levels(), get_num_faces(), fmt, get_name().get_ptr());

      state.m_pixel_blocks.resize(0);

      image_utils::conversion_type cook_conv_type = image_utils::cConversion_Invalid;
      if (cook)
      {
         cook_conv_type = image_utils::get_conversion_type(true, fmt);
         if (pixel_format_helpers::is_alpha_only(fmt) && !pixel_format_helpers::has_alpha(m_format))
            cook_conv_type = image_utils::cConversion_Y_To_A;
      }

      state.m_qdxt1_params.m_num_mips = 0;
      state.m_qdxt5_params[0].m_num_mips = 0;
      state.m_qdxt5_params[1].m_num_mips = 0;

      for (uint f = 0; f < get_num_faces(); f++)
      {
         for (uint l = 0; l < get_num_levels(); l++)
         {
            mip_level* pLevel = get_level(f, l);

            image_u8 tmp_img;
            image_u8 img(*pLevel->get_unpacked_image(tmp_img, true));

            if (cook_conv_type != image_utils::cConversion_Invalid)
               image_utils::convert_image(img, cook_conv_type);

            const uint num_blocks_x = (img.get_width() + 3) / 4;
            const uint num_blocks_y = (img.get_height() + 3) / 4;
            const uint total_blocks = num_blocks_x * num_blocks_y;

            const uint cur_size = state.m_pixel_blocks.size();
            state.m_pixel_blocks.resize(cur_size + total_blocks);
            dxt_pixel_block* pDst_blocks = &state.m_pixel_blocks[cur_size];

            {
               CRNLIB_ASSERT(state.m_qdxt1_params.m_num_mips < qdxt1_params::cMaxMips);
               qdxt1_params::mip_desc& mip_desc = state.m_qdxt1_params.m_mip_desc[state.m_qdxt1_params.m_num_mips];
               mip_desc.m_first_block = cur_size;
               mip_desc.m_block_width = num_blocks_x;
               mip_desc.m_block_height = num_blocks_y;
               state.m_qdxt1_params.m_num_mips++;
            }

            for (uint i = 0; i < 2; i++)
            {
               CRNLIB_ASSERT(state.m_qdxt5_params[i].m_num_mips < qdxt5_params::cMaxMips);
               qdxt5_params::mip_desc& mip_desc = state.m_qdxt5_params[i].m_mip_desc[state.m_qdxt5_params[i].m_num_mips];
               mip_desc.m_first_block = cur_size;
               mip_desc.m_block_width = num_blocks_x;
               mip_desc.m_block_height = num_blocks_y;
               state.m_qdxt5_params[i].m_num_mips++;
            }

            for (uint block_y = 0; block_y < num_blocks_y; block_y++)
            {
               const uint img_y = block_y << 2;

               for (uint block_x = 0; block_x < num_blocks_x; block_x++)
               {
                  const uint img_x = block_x << 2;

                  color_quad_u8* pDst_pixel = &pDst_blocks->m_pixels[0][0];

                  pDst_blocks++;

                  for (uint by = 0; by < 4; by++)
                     for (uint bx = 0; bx < 4; bx++)
                        *pDst_pixel++ = img.get_clamped(img_x + bx, img_y + by);
               } // block_x
            } // block_y
         } // l
      } // f

      if (state.m_has_blocks[0])
      {
         if (!state.m_qdxt1.init(state.m_pixel_blocks.size(), &state.m_pixel_blocks[0], state.m_qdxt1_params))
            return false;
      }

      if (state.m_has_blocks[1])
      {
         if (!state.m_qdxt5a.init(state.m_pixel_blocks.size(), &state.m_pixel_blocks[0], state.m_qdxt5_params[0]))
            return false;
      }

      if (state.m_has_blocks[2])
      {
         if (!state.m_qdxt5b.init(state.m_pixel_blocks.size(), &state.m_pixel_blocks[0], state.m_qdxt5_params[1]))
            return false;
      }

      return true;
   }

   bool dds_texture::qdxt_pack(qdxt_state& state, dds_texture& dst_tex, const qdxt1_params& dxt1_params, const qdxt5_params& dxt5_params)
   {
      if (!is_valid())
         return false;

      CRNLIB_ASSERT(dxt1_params.m_quality_level <= qdxt1_params::cMaxQuality);
      CRNLIB_ASSERT(dxt5_params.m_quality_level <= qdxt5_params::cMaxQuality);

      state.m_qdxt1_params.m_quality_level = dxt1_params.m_quality_level;
      state.m_qdxt1_params.m_pProgress_func = dxt1_params.m_pProgress_func;
      state.m_qdxt1_params.m_pProgress_data = dxt1_params.m_pProgress_data;

      state.m_qdxt5_params[0].m_quality_level = dxt5_params.m_quality_level;
      state.m_qdxt5_params[0].m_pProgress_func = dxt5_params.m_pProgress_func;
      state.m_qdxt5_params[0].m_pProgress_data = dxt5_params.m_pProgress_data;

      state.m_qdxt5_params[1].m_quality_level = dxt5_params.m_quality_level;
      state.m_qdxt5_params[1].m_pProgress_func = dxt5_params.m_pProgress_func;
      state.m_qdxt5_params[1].m_pProgress_data = dxt5_params.m_pProgress_data;

      const uint num_elements = state.m_has_blocks[0] + state.m_has_blocks[1] + state.m_has_blocks[2];

      uint cur_progress_start = dxt1_params.m_progress_start;
      if (state.m_has_blocks[0])
      {
         state.m_qdxt1_params.m_progress_start = cur_progress_start;
         state.m_qdxt1_params.m_progress_range = dxt1_params.m_progress_range / num_elements;
         cur_progress_start += state.m_qdxt1_params.m_progress_range;
      }

      if (state.m_has_blocks[1])
      {
         state.m_qdxt5_params[0].m_progress_start = cur_progress_start;
         state.m_qdxt5_params[0].m_progress_range = dxt1_params.m_progress_range / num_elements;
         cur_progress_start += state.m_qdxt5_params[0].m_progress_range;
      }

      if (state.m_has_blocks[2])
      {
         state.m_qdxt5_params[1].m_progress_start = cur_progress_start;
         state.m_qdxt5_params[1].m_progress_range = dxt1_params.m_progress_range - cur_progress_start;
      }

      crnlib::vector<dxt1_block> dxt1_blocks;
      if (state.m_has_blocks[0])
      {
         dxt1_blocks.resize(state.m_pixel_blocks.size());
         float pow_mul = 1.0f;
         
         if (state.m_fmt == PIXEL_FMT_DXT5_CCxY)
         {
            // use a "deeper" codebook size curves when compressing chroma into DXT1, because it's not as important
            pow_mul = 1.5f;
         }
         else if (state.m_fmt == PIXEL_FMT_DXT5)
         {
            // favor color more than alpha
            pow_mul = .75f;
         }

         if (!state.m_qdxt1.pack(&dxt1_blocks[0], 1, state.m_qdxt1_params, pow_mul))
            return false;
      }

      crnlib::vector<dxt5_block> dxt5_blocks[2];
      for (uint i = 0; i < 2; i++)
      {
         if (state.m_has_blocks[i + 1])
         {
            dxt5_blocks[i].resize(state.m_pixel_blocks.size());

            if (!(i ? state.m_qdxt5b : state.m_qdxt5a).pack(&dxt5_blocks[i][0], 1, state.m_qdxt5_params[i]))
               return false;
         }
      }

      uint cur_block_ofs = 0;

      for (uint f = 0; f < dst_tex.get_num_faces(); f++)
      {
         for (uint l = 0; l < dst_tex.get_num_levels(); l++)
         {
            mip_level* pDst_level = dst_tex.get_level(f, l);

            const uint num_blocks_x = (pDst_level->get_width() + 3) / 4;
            const uint num_blocks_y = (pDst_level->get_height() + 3) / 4;
            const uint total_blocks = num_blocks_x * num_blocks_y;

            dxt_image* pDst_dxt_image = pDst_level->get_dxt_image();

            dxt_image::element* pDst = pDst_dxt_image->get_element_ptr();
            for (uint block_index = 0; block_index < total_blocks; block_index++)
            {
               if (state.m_has_blocks[1])
                  memcpy(pDst, &dxt5_blocks[0][cur_block_ofs + block_index], 8);

               if (state.m_has_blocks[2])
                  memcpy(pDst + 1, &dxt5_blocks[1][cur_block_ofs + block_index], 8);

               if (state.m_has_blocks[0])
                  memcpy(pDst + state.m_has_blocks[1], &dxt1_blocks[cur_block_ofs + block_index], 8);

               pDst += pDst_dxt_image->get_elements_per_block();
            }

            cur_block_ofs += total_blocks;
         }
      }

      if (dxt1_params.m_pProgress_func)
      {
         if (!dxt1_params.m_pProgress_func(dxt1_params.m_progress_start + dxt1_params.m_progress_range, dxt1_params.m_pProgress_data))
            return false;
      }

      return true;
   }

   bool dds_texture::load_from_file(const wchar_t* pFilename, texture_file_types::format file_format)
   {
      clear();

      if (file_format == texture_file_types::cFormatInvalid)
         file_format = texture_file_types::determine_file_format(pFilename);

      if (file_format == texture_file_types::cFormatInvalid)
      {
         set_last_error(L"Unrecognized image format extension");
         return false;
      }

      set_last_error(L"Image file load failed");

      bool success = false;
      switch (file_format)
      {
         case texture_file_types::cFormatDDS:
         {
            success = load_dds(pFilename);
            break;
         }
         case texture_file_types::cFormatCRN:
         {
            success = load_crn(pFilename);
            break;
         }
         default:
         {
            success = load_regular(pFilename, file_format);
            break;
         }
      }

      if (success)
      {
         m_source_file_type = file_format;
         clear_last_error();
      }

      return success;
   }

   bool dds_texture::load_regular(const wchar_t* pFilename, texture_file_types::format file_format)
   {
      file_format;

      image_u8* pImg = crnlib_new<image_u8>();
      bool status = image_utils::load_from_file(*pImg, pFilename, 0);
      if (!status)
      {
         crnlib_delete(pImg);

         set_last_error(L"Failed loading image file");
         return false;
      }

      mip_level* pLevel = crnlib_new<mip_level>();
      pLevel->assign(pImg);

      assign(pLevel);
      set_name(pFilename);

      return true;
   }

   bool dds_texture::load_dds(const wchar_t* pFilename)
   {
      cfile_stream in_stream;
      if (!in_stream.open(pFilename))
      {
         set_last_error(L"Failed opening file");
         return false;
      }

      data_stream_serializer serializer(in_stream);

      if (!read_dds(serializer))
      {
         set_last_error(get_last_error().get_ptr());
         return false;
      }

      set_name(pFilename);

      return true;
   }

   bool dds_texture::load_crn_from_memory(const wchar_t* pFilename, const void *pData, uint data_size)
   {
      clear();

      set_last_error(L"Image file load failed");

      if ((!pData) || (data_size < 1)) return false;

      crnd::crn_texture_info tex_info;
      tex_info.m_struct_size = sizeof(crnd::crn_texture_info);
      if (!crnd_get_texture_info(pData, data_size, &tex_info))
      {
         set_last_error(L"crnd_get_texture_info() failed");
         return false;
      }

      const pixel_format dds_fmt = (pixel_format)crnd::crnd_crn_format_to_fourcc(tex_info.m_format);
      if (dds_fmt == PIXEL_FMT_INVALID)
      {
         set_last_error(L"Unsupported DXT format");
         return false;
      }

      const dxt_format dxt_fmt = pixel_format_helpers::get_dxt_format(dds_fmt);

      face_vec faces(tex_info.m_faces);
      for (uint f = 0; f < tex_info.m_faces; f++)
      {
         faces[f].resize(tex_info.m_levels);

         for (uint l = 0; l < tex_info.m_levels; l++)
            faces[f][l] = crnlib_new<mip_level>();
      }

      const uint tex_num_blocks_x = (tex_info.m_width + 3) >> 2;
      const uint tex_num_blocks_y = (tex_info.m_height + 3) >> 2;

      vector<uint8> dxt_data;
      // Create temp buffer big enough to hold the largest mip level, and all faces if it's a cubemap.
      dxt_data.resize(tex_info.m_bytes_per_block * tex_num_blocks_x * tex_num_blocks_y * tex_info.m_faces);

      set_last_error(L"CRN unpack failed");

      timer t;
      double total_time = 0.0f;
      t.start();
      crnd::crnd_unpack_context pContext = crnd::crnd_unpack_begin(pData, data_size);
      total_time += t.get_elapsed_secs();

      if (!pContext)
      {
         for (uint f = 0; f < faces.size(); f++)
            for (uint l = 0; l < faces[f].size(); l++)
               crnlib_delete(faces[f][l]);
         return false;
      }

      uint total_pixels = 0;

      void* pFaces[cCRNMaxFaces];
      for (uint f = tex_info.m_faces; f < cCRNMaxFaces; f++)
         pFaces[f] = NULL;

      for (uint l = 0; l < tex_info.m_levels; l++)
      {
         const uint level_width = math::maximum<uint>(1U, tex_info.m_width >> l);
         const uint level_height = math::maximum<uint>(1U, tex_info.m_height >> l);
         const uint num_blocks_x = (level_width + 3U) >> 2U;
         const uint num_blocks_y = (level_height + 3U) >> 2U;

         const uint row_pitch = num_blocks_x * tex_info.m_bytes_per_block;
         const uint size_of_face = num_blocks_y * row_pitch;

         total_pixels += num_blocks_x * num_blocks_y * 4 * 4 * tex_info.m_faces;

         t.start();

         for (uint f = 0; f < tex_info.m_faces; f++)
            pFaces[f] = &dxt_data[f * size_of_face];

         if (!crnd::crnd_unpack_level(pContext, pFaces, dxt_data.size(), row_pitch, l))
         {
            crnd::crnd_unpack_end(pContext);
            for (uint f = 0; f < faces.size(); f++)
               for (uint l = 0; l < faces[f].size(); l++)
                  crnlib_delete(faces[f][l]);
            return false;
         }

         total_time += t.get_elapsed_secs();

         for (uint f = 0; f < tex_info.m_faces; f++)
         {
            dxt_image* pDXT_image = crnlib_new<dxt_image>();

            if (!pDXT_image->init(
               dxt_fmt, level_width, level_height,
               num_blocks_x * num_blocks_y * (tex_info.m_bytes_per_block / sizeof(dxt_image::element)),
               reinterpret_cast<dxt_image::element*>(pFaces[f]), true))
            {
               crnlib_delete(pDXT_image);

               crnd::crnd_unpack_end(pContext);
               for (uint f = 0; f < faces.size(); f++)
                  for (uint l = 0; l < faces[f].size(); l++)
                     crnlib_delete(faces[f][l]);

               return false;
            }

            faces[f][l]->assign(pDXT_image, dds_fmt);
         }
      }

#if 0
      if (total_pixels)
      {
         console::info(L"load_crn: Total pixels: %u, ms: %3.3fms, megapixels/sec: %3.3f",
            total_pixels, total_time * 1000.0f, total_pixels / total_time);
      }
#endif

      crnd::crnd_unpack_end(pContext);

      assign(faces);
      set_name(pFilename);

      m_source_file_type = texture_file_types::cFormatCRN;
      clear_last_error();

      return true;
   }

   bool dds_texture::load_crn(const wchar_t* pFilename)
   {
      cfile_stream in_stream;
      if (!in_stream.open(pFilename))
      {
         set_last_error(L"Failed opening CRN file");
         return false;
      }

      crnlib::vector<uint8> crn_data;
      if (!in_stream.read_array(crn_data))
      {
         set_last_error(L"Failed reading CRN file");
         return false;
      }

      in_stream.close();

      return load_crn_from_memory(pFilename, crn_data.get_ptr(), crn_data.size());
   }

   bool dds_texture::write_to_file(
      const wchar_t* pFilename,
      texture_file_types::format file_format,
      crn_comp_params* pCRN_comp_params,
      uint32 *pActual_quality_level, float *pActual_bitrate)
   {
      if (pActual_quality_level) *pActual_quality_level = 0;
      if (pActual_bitrate) *pActual_bitrate = 0.0f;

      if (!is_valid())
      {
         set_last_error(L"Unable to save empty texture");
         return false;
      }

      bool success = false;

      switch (file_format)
      {
         case texture_file_types::cFormatDDS:
         {
            if (!pCRN_comp_params)
               success = save_dds(pFilename);
            else
               success = save_comp_texture(pFilename, *pCRN_comp_params, pActual_quality_level, pActual_bitrate);
            break;
         }
         case texture_file_types::cFormatCRN:
         {
            if (!pCRN_comp_params)
               return false;
            success = save_comp_texture(pFilename, *pCRN_comp_params, pActual_quality_level, pActual_bitrate);
            break;
         }
         default:
         {
            success = save_regular(pFilename);
            break;
         }
      }

      return success;
   }

   bool dds_texture::save_regular(const wchar_t* pFilename)
   {
      image_u8 tmp;
      image_u8* pLevel_image = get_level_image(0, 0, tmp);

      if (!image_utils::save_to_file(pFilename, *pLevel_image, 0))
      {
         set_last_error(L"File write failed");
         return false;
      }

      return true;
   }

   bool dds_texture::save_dds(const wchar_t* pFilename)
   {
      cfile_stream out_stream;
      if (!out_stream.open(pFilename, cDataStreamWritable | cDataStreamSeekable))
      {
         set_last_error(L"Unable to open file");
         return false;
      }

      data_stream_serializer serializer(out_stream);

      if (!write_dds(serializer))
      {
         set_last_error(L"File write failed");
         return false;
      }

      return true;
   }

   void dds_texture::print_crn_comp_params(const crn_comp_params& p)
   {
      console::debug(L"CRN compression params:");
      console::debug(L"      File Type: %s", crn_get_file_type_ext(p.m_file_type));
      console::debug(L"  Quality level: %u", p.m_quality_level);
      console::debug(L" Target Bitrate: %f", p.m_target_bitrate);
      console::debug(L"          Faces: %u", p.m_faces);
      console::debug(L"          Width: %u", p.m_width);
      console::debug(L"         Height: %u", p.m_height);
      console::debug(L"         Levels: %u", p.m_levels);
      console::debug(L"   Pixel Format: %s", crn_get_format_string(p.m_format));
      console::debug(L"Use manual CRN palette sizes: %u", p.get_flag(cCRNCompFlagManualPaletteSizes));
      console::debug(L"Color endpoints: %u", p.m_crn_color_endpoint_palette_size);
      console::debug(L"Color selectors: %u", p.m_crn_color_selector_palette_size);
      console::debug(L"Alpha endpoints: %u", p.m_crn_alpha_endpoint_palette_size);
      console::debug(L"Alpha selectors: %u", p.m_crn_alpha_selector_palette_size);
      console::debug(L"Flags:");
      console::debug(L"    Perceptual: %u", p.get_flag(cCRNCompFlagPerceptual));
      console::debug(L"  Hierarchical: %u", p.get_flag(cCRNCompFlagHierarchical));
      console::debug(L"  UseBothBlockTypes: %u", p.get_flag(cCRNCompFlagUseBothBlockTypes));
      console::debug(L"  UseTransparentIndicesForBlack: %u", p.get_flag(cCRNCompFlagUseTransparentIndicesForBlack));
      console::debug(L"  DisableEndpointCaching: %u", p.get_flag(cCRNCompFlagDisableEndpointCaching));
      console::debug(L"GrayscaleSampling: %u", p.get_flag(cCRNCompFlagGrayscaleSampling));
      console::debug(L"  UseDXT1ATransparency: %u", p.get_flag(cCRNCompFlagDXT1AForTransparency));
      console::debug(L"AdaptiveTileColorPSNRDerating: %2.2fdB", p.m_crn_adaptive_tile_color_psnr_derating);
      console::debug(L"AdaptiveTileAlphaPSNRDerating: %2.2fdB", p.m_crn_adaptive_tile_alpha_psnr_derating);
      console::debug(L"NumHelperThreads: %u", p.m_num_helper_threads);
   }

   bool dds_texture::save_comp_texture(const wchar_t* pFilename, const crn_comp_params &orig_comp_params, uint32 *pActual_quality_level, float *pActual_bitrate)
   {
      crn_comp_params comp_params(orig_comp_params);

      if (pActual_quality_level) *pActual_quality_level = 0;
      if (pActual_bitrate) *pActual_bitrate = 0.0f;

      if (math::maximum(get_height(), get_width()) > cCRNMaxLevelResolution)
      {
         set_last_error(L"Texture resolution is too big!");
         return false;
      }

      comp_params.m_faces = get_num_faces();
      comp_params.m_levels = get_num_levels();
      comp_params.m_width = get_width();
      comp_params.m_height = get_height();

      image_u8 temp_images[cCRNMaxFaces][cCRNMaxLevels];
      for (uint f = 0; f < get_num_faces(); f++)
      {
         for (uint l = 0; l < get_num_levels(); l++)
         {
            image_u8* p = get_level_image(f, l, temp_images[f][l]);

            comp_params.m_pImages[f][l] = (crn_uint32*)p->get_ptr();
         }
      }

      if (comp_params.get_flag(cCRNCompFlagDebugging))
         print_crn_comp_params(comp_params);

      timer t;
      t.start();

      crnlib::vector<uint8> comp_data;
      if (!create_compressed_texture(comp_params, comp_data, pActual_quality_level, pActual_bitrate))
      {
         set_last_error(L"CRN compression failed");
         return false;
      }

      double total_time = t.get_elapsed_secs();
      if (comp_params.get_flag(cCRNCompFlagDebugging))
      {
         console::debug(L"\nTotal compression time: %3.3fs", total_time);
      }

      cfile_stream out_stream;
      if (!out_stream.open(pFilename, cDataStreamWritable | cDataStreamSeekable))
      {
         set_last_error(L"Failed opening file");
         return false;
      }

      if (out_stream.write(comp_data.get_ptr(), comp_data.size()) != comp_data.size())
      {
         set_last_error(L"Failed writing to file");
         return false;
      }

      if (!out_stream.close())
      {
         set_last_error(L"Failed writing to file");
         return false;
      }

      return true;
   }

   uint dds_texture::get_total_pixels_in_all_faces_and_mips() const
   {
      uint total_pixels = 0;
      for (uint l = 0; l < m_faces.size(); l++)
         for (uint m = 0; m < m_faces[l].size(); m++)
            total_pixels += m_faces[l][m]->get_total_pixels();

      return total_pixels;
   }

} // namespace crnlib

