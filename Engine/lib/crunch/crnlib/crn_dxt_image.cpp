// File: crn_dxt_image.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_dxt_image.h"
#if CRNLIB_SUPPORT_SQUISH
#include "squish\squish.h"
#endif
#include "crn_ryg_dxt.hpp"
#include "crn_dxt_fast.h"
#include "crn_task_pool.h"
#include "crn_console.h"

#if CRNLIB_SUPPORT_ATI_COMPRESS
   #ifdef _DLL
      #pragma comment(lib, "ATI_Compress_MT_DLL_VC8.lib")
   #else
      #pragma comment(lib, "ATI_Compress_MT_VC8.lib")
   #endif
   #include "..\ext\ATI_Compress\ATI_Compress.h"
#endif

namespace crnlib
{
   dxt_image::dxt_image() :
      m_pElements(NULL),
      m_width(0),
      m_height(0),
      m_blocks_x(0),
      m_blocks_y(0),
      m_total_blocks(0),
      m_total_elements(0),
      m_num_elements_per_block(0),
      m_bytes_per_block(0),
      m_format(cDXTInvalid)
   {
      utils::zero_object(m_element_type);
      utils::zero_object(m_element_component_index);
   }

   dxt_image::dxt_image(const dxt_image& other) :
      m_pElements(NULL)
   {
      *this = other;
   }

   dxt_image& dxt_image::operator= (const dxt_image& rhs)
   {
      if (this == &rhs)
         return *this;

      clear();

      m_width = rhs.m_width;
      m_height = rhs.m_height;
      m_blocks_x = rhs.m_blocks_x;
      m_blocks_y = rhs.m_blocks_y;
      m_num_elements_per_block = rhs.m_num_elements_per_block;
      m_bytes_per_block = rhs.m_bytes_per_block;
      m_format = rhs.m_format;
      m_total_blocks = rhs.m_total_blocks;
      m_total_elements = rhs.m_total_elements;
      m_pElements = NULL;
      memcpy(m_element_type, rhs.m_element_type, sizeof(m_element_type));
      memcpy(m_element_component_index, rhs.m_element_component_index, sizeof(m_element_component_index));

      if (rhs.m_pElements)
      {
         m_elements.resize(m_total_elements);
         memcpy(&m_elements[0], rhs.m_pElements, sizeof(element) * m_total_elements);
         m_pElements = &m_elements[0];
      }

      return *this;
   }

   void dxt_image::clear()
   {
      m_elements.clear();
      m_width = 0;
      m_height = 0;
      m_blocks_x = 0;
      m_blocks_y = 0;
      m_num_elements_per_block = 0;
      m_bytes_per_block = 0;
      m_format = cDXTInvalid;
      utils::zero_object(m_element_type);
      utils::zero_object(m_element_component_index);
      m_total_blocks = 0;
      m_total_elements = 0;
      m_pElements = NULL;
   }

   bool dxt_image::init_internal(dxt_format fmt, uint width, uint height)
   {
      CRNLIB_ASSERT((fmt != cDXTInvalid) && (width > 0) && (height > 0));

      clear();

      m_width = width;
      m_height = height;

      m_blocks_x = (m_width + 3) >> cDXTBlockShift;
      m_blocks_y = (m_height + 3) >> cDXTBlockShift;

      m_num_elements_per_block = 2;
      if ((fmt == cDXT1) || (fmt == cDXT1A) || (fmt == cDXT5A))
         m_num_elements_per_block = 1;

      m_total_blocks = m_blocks_x * m_blocks_y;
      m_total_elements = m_total_blocks * m_num_elements_per_block;

      m_bytes_per_block = cDXT1BytesPerBlock * m_num_elements_per_block;

      m_format = fmt;

      switch (m_format)
      {
         case cDXT1:
         case cDXT1A:
         {
            m_element_type[0] = cColor;
            m_element_component_index[0] = -1;
            break;
         }
         case cDXT3:
         {
            m_element_type[0] = cAlpha3;
            m_element_type[1] = cColor;
            m_element_component_index[0] = 3;
            m_element_component_index[1] = -1;
            break;
         }
         case cDXT5:
         {
            m_element_type[0] = cAlpha5;
            m_element_type[1] = cColor;
            m_element_component_index[0] = 3;
            m_element_component_index[1] = -1;
            break;
         }
         case cDXT5A:
         {
            m_element_type[0] = cAlpha5;
            m_element_component_index[0] = 3;
            break;
         }
         case cDXN_XY:
         {
            m_element_type[0] = cAlpha5;
            m_element_type[1] = cAlpha5;
            m_element_component_index[0] = 0;
            m_element_component_index[1] = 1;
            break;
         }
         case cDXN_YX:
         {
            m_element_type[0] = cAlpha5;
            m_element_type[1] = cAlpha5;
            m_element_component_index[0] = 1;
            m_element_component_index[1] = 0;
            break;
         }
         default:
         {
            CRNLIB_ASSERT(0);
            clear();
            return false;
         }
      }

      return true;
   }

   bool dxt_image::init(dxt_format fmt, uint width, uint height, bool clear_elements)
   {
      if (!init_internal(fmt, width, height))
         return false;

      m_elements.resize(m_total_elements);
      m_pElements = &m_elements[0];

      if (clear_elements)
         memset(m_pElements, 0, sizeof(element) * m_total_elements);

      return true;
   }

   bool dxt_image::init(dxt_format fmt, uint width, uint height, uint num_elements, element* pElements, bool create_copy)
   {
      CRNLIB_ASSERT(num_elements && pElements);

      if (!init_internal(fmt, width, height))
         return false;

      if (num_elements != m_total_elements)
      {
         clear();
         return false;
      }

      if (create_copy)
      {
         m_elements.resize(m_total_elements);
         m_pElements = &m_elements[0];

         memcpy(m_pElements, pElements, m_total_elements * sizeof(element));
      }
      else
         m_pElements = pElements;

      return true;
   }

   struct init_task_params
   {
      dxt_format                    m_fmt;
      const image_u8*               m_pImg;
      const dxt_image::pack_params* m_pParams;
      uint32                        m_main_thread;
      int32                         m_canceled;
   };

   void dxt_image::init_task(uint64 data, void* pData_ptr)
   {
      const uint thread_index = static_cast<uint>(data);
      init_task_params* pInit_params = static_cast<init_task_params*>(pData_ptr);

      const image_u8& img = *pInit_params->m_pImg;
      const pack_params& p = *pInit_params->m_pParams;
      const bool is_main_thread = (get_current_thread_id() == pInit_params->m_main_thread);

      uint block_index = 0;

      dxt1_endpoint_optimizer dxt1_optimizer;
      dxt5_endpoint_optimizer dxt5_optimizer;
      int prev_progress_percentage = -1;

      for (uint block_y = 0; block_y < m_blocks_y; block_y++)
      {
         const uint pixel_ofs_y = block_y * cDXTBlockSize;

         for (uint block_x = 0; block_x < m_blocks_x; block_x++, block_index++)
         {
            if (pInit_params->m_canceled)
               return;

            if (p.m_pProgress_callback && is_main_thread && ((block_index & 63) == 63))
            {
               const uint progress_percentage = p.m_progress_start + ((block_index * p.m_progress_range + get_total_blocks() / 2) / get_total_blocks());
               if ((int)progress_percentage != prev_progress_percentage)
               {
                  prev_progress_percentage = progress_percentage;
                  if (!(p.m_pProgress_callback)(progress_percentage, p.m_pProgress_callback_user_data_ptr))
                  {
                     interlocked_exchange32(&pInit_params->m_canceled, CRNLIB_TRUE);
                     return;
                  }
               }
            }

            if (p.m_num_helper_threads)
            {
               if ((block_index % (p.m_num_helper_threads + 1)) != thread_index)
                  continue;
            }

            color_quad_u8 pixels[cDXTBlockSize * cDXTBlockSize];

            const uint pixel_ofs_x = block_x * cDXTBlockSize;

            for (uint y = 0; y < cDXTBlockSize; y++)
            {
               const uint iy = math::minimum(pixel_ofs_y + y, img.get_height() - 1);

               for (uint x = 0; x < cDXTBlockSize; x++)
               {
                  const uint ix = math::minimum(pixel_ofs_x + x, img.get_width() - 1);

                  pixels[x + y * cDXTBlockSize] = img(ix, iy);
               }
            }

            set_block_pixels(block_x, block_y, pixels, p, dxt1_optimizer, dxt5_optimizer);
         }
      }
   }

#if CRNLIB_SUPPORT_ATI_COMPRESS
   bool dxt_image::init_ati_compress(dxt_format fmt, const image_u8& img, const pack_params& p)
   {
      image_u8 tmp_img(img);
      for (uint y = 0; y < img.get_height(); y++)
      {
         for (uint x = 0; x < img.get_width(); x++)
         {
            color_quad_u8 c(img(x, y));
            std::swap(c.r, c.b);
            tmp_img(x, y) = c;
         }
      }

      ATI_TC_Texture src_tex;
      utils::zero_object(src_tex);
      src_tex.dwSize = sizeof(ATI_TC_Texture);
      src_tex.dwWidth = tmp_img.get_width();
      src_tex.dwHeight = tmp_img.get_height();
      src_tex.dwPitch = tmp_img.get_pitch_in_bytes();
      src_tex.format = ATI_TC_FORMAT_ARGB_8888;
      src_tex.dwDataSize = src_tex.dwPitch * tmp_img.get_height();
      src_tex.pData = (ATI_TC_BYTE*)tmp_img.get_ptr();

      ATI_TC_Texture dst_tex;
      utils::zero_object(dst_tex);
      dst_tex.dwSize = sizeof(ATI_TC_Texture);
      dst_tex.dwWidth = tmp_img.get_width();
      dst_tex.dwHeight = tmp_img.get_height();
      dst_tex.dwDataSize = get_size_in_bytes();
      dst_tex.pData = (ATI_TC_BYTE*)get_element_ptr();

      switch (fmt)
      {
         case cDXT1:
         case cDXT1A:
            dst_tex.format = ATI_TC_FORMAT_DXT1;
            break;
         case cDXT3:
            dst_tex.format = ATI_TC_FORMAT_DXT3;
            break;
         case cDXT5:
            dst_tex.format = ATI_TC_FORMAT_DXT5;
            break;
         case cDXT5A:
            dst_tex.format = ATI_TC_FORMAT_ATI1N;
            break;
         case cDXN_XY:
            dst_tex.format = ATI_TC_FORMAT_ATI2N_XY;
            break;
         case cDXN_YX:
            dst_tex.format = ATI_TC_FORMAT_ATI2N;
            break;
         default:
         {
            CRNLIB_ASSERT(false);
            return false;
         }
      }

      ATI_TC_CompressOptions options;
      utils::zero_object(options);
      options.dwSize = sizeof(ATI_TC_CompressOptions);

      if (fmt == cDXT1A)
      {
         options.bDXT1UseAlpha = TRUE;
         options.nAlphaThreshold = (ATI_TC_BYTE)p.m_dxt1a_alpha_threshold;
      }
      options.bDisableMultiThreading = (p.m_num_helper_threads == 0);
      switch (p.m_quality)
      {
         case cCRNDXTQualityFast:
            options.nCompressionSpeed = ATI_TC_Speed_Fast;
            break;
         case cCRNDXTQualitySuperFast:
            options.nCompressionSpeed = ATI_TC_Speed_SuperFast;
            break;
         default:
            options.nCompressionSpeed = ATI_TC_Speed_Normal;
            break;
      }

      if (p.m_perceptual)
      {
         options.bUseChannelWeighting = TRUE;
         options.fWeightingRed =   .212671f;
         options.fWeightingGreen = .715160f;
         options.fWeightingBlue =  .072169f;
      }

      ATI_TC_ERROR err = ATI_TC_ConvertTexture(&src_tex, &dst_tex, &options, NULL, NULL, NULL);
      return err == ATI_TC_OK;
   }
#endif

   bool dxt_image::init(dxt_format fmt, const image_u8& img, const pack_params& p)
   {
      if (!init(fmt, img.get_width(), img.get_height(), false))
         return false;

#if CRNLIB_SUPPORT_ATI_COMPRESS
      if (p.m_compressor == cCRNDXTCompressorATI)
         return init_ati_compress(fmt, img, p);
#endif

      task_pool *pPool = p.m_pTask_pool;

      task_pool tmp_pool;
      if (!pPool)
      {
         if (!tmp_pool.init(p.m_num_helper_threads))
            return false;
         pPool = &tmp_pool;
      }

      init_task_params init_params;
      init_params.m_fmt = fmt;
      init_params.m_pImg = &img;
      init_params.m_pParams = &p;
      init_params.m_main_thread = get_current_thread_id();
      init_params.m_canceled = false;

      for (uint i = 0; i <= p.m_num_helper_threads; i++)
         pPool->queue_object_task(this, &dxt_image::init_task, i, &init_params);

      pPool->join();

      if (init_params.m_canceled)
         return false;

      return true;
   }

   bool dxt_image::unpack(image_u8& img) const
   {
      if (!m_total_elements)
         return false;

      img.resize(m_width, m_height);

      color_quad_u8 pixels[cDXTBlockSize * cDXTBlockSize];
      for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
         pixels[i].set(0, 0, 0, 255);

      for (uint block_y = 0; block_y < m_blocks_y; block_y++)
      {
         const uint pixel_ofs_y = block_y * cDXTBlockSize;
         const uint limit_y = math::minimum<uint>(cDXTBlockSize, img.get_height() - pixel_ofs_y);

         for (uint block_x = 0; block_x < m_blocks_x; block_x++)
         {
            get_block_pixels(block_x, block_y, pixels);

            const uint pixel_ofs_x = block_x * cDXTBlockSize;

            const uint limit_x = math::minimum<uint>(cDXTBlockSize, img.get_width() - pixel_ofs_x);

            for (uint y = 0; y < limit_y; y++)
            {
               const uint iy = pixel_ofs_y + y;

               for (uint x = 0; x < limit_x; x++)
               {
                  const uint ix = pixel_ofs_x + x;

                  img(ix, iy) = pixels[x + (y << cDXTBlockShift)];
               }
            }
         }
      }

      img.reset_comp_flags();
      img.set_component_valid(0, false);
      img.set_component_valid(1, false);
      img.set_component_valid(2, false);
      for (uint i = 0; i < m_num_elements_per_block; i++)
      {
         if (m_element_component_index[i] < 0)
         {
            img.set_component_valid(0, true);
            img.set_component_valid(1, true);
            img.set_component_valid(2, true);
         }
         else
            img.set_component_valid(m_element_component_index[i], true);
      }

      img.set_component_valid(3, get_dxt_format_has_alpha(m_format));

      return true;
   }

   void dxt_image::endian_swap()
   {
      utils::endian_switch_words(reinterpret_cast<uint16*>(m_elements.get_ptr()), m_elements.size_in_bytes() / sizeof(uint16));
   }

   const dxt_image::element& dxt_image::get_element(uint block_x, uint block_y, uint element_index) const
   {
      CRNLIB_ASSERT((block_x < m_blocks_x) && (block_y < m_blocks_y) && (element_index < m_num_elements_per_block));
      return m_pElements[(block_x + block_y * m_blocks_x) * m_num_elements_per_block + element_index];
   }

   dxt_image::element& dxt_image::get_element(uint block_x, uint block_y, uint element_index)
   {
      CRNLIB_ASSERT((block_x < m_blocks_x) && (block_y < m_blocks_y) && (element_index < m_num_elements_per_block));
      return m_pElements[(block_x + block_y * m_blocks_x) * m_num_elements_per_block + element_index];
   }

   bool dxt_image::has_alpha() const
   {
      switch (m_format)
      {
         case cDXT1:
         {
            for (uint i = 0; i < m_total_elements; i++)
            {
               const dxt1_block& blk = *(dxt1_block*)&m_pElements[i];

               if (blk.get_low_color() <= blk.get_high_color())
               {
                  for (uint y = 0; y < cDXTBlockSize; y++)
                     for (uint x = 0; x < cDXTBlockSize; x++)
                        if (blk.get_selector(x, y) == 3)
                           return true;
               }
            }

            break;
         }
         case cDXT1A:
         case cDXT3:
         case cDXT5:
         case cDXT5A:
            return true;
         default: break;
      }

      return false;
   }

   color_quad_u8 dxt_image::get_pixel(uint x, uint y) const
   {
      CRNLIB_ASSERT((x < m_width) && (y < m_height));

      const uint block_x = x >> cDXTBlockShift;
      const uint block_y = y >> cDXTBlockShift;

      const element* pElement = reinterpret_cast<const element*>(&get_element(block_x, block_y, 0));

      color_quad_u8 result(0, 0, 0, 255);

      for (uint element_index = 0; element_index < m_num_elements_per_block; element_index++, pElement++)
      {
         switch (m_element_type[element_index])
         {
            case cColor:
            {
               const dxt1_block* pBlock = reinterpret_cast<const dxt1_block*>(&get_element(block_x, block_y, element_index));

               const uint l = pBlock->get_low_color();
               const uint h = pBlock->get_high_color();

               color_quad_u8 c0(dxt1_block::unpack_color(static_cast<uint16>(l), true));
               color_quad_u8 c1(dxt1_block::unpack_color(static_cast<uint16>(h), true));

               const uint s = pBlock->get_selector(x & 3, y & 3);

               if (l > h)
               {
                  switch (s)
                  {
                     case 0: result.set_noclamp_rgb(c0.r, c0.g, c0.b); break;
                     case 1: result.set_noclamp_rgb(c1.r, c1.g, c1.b); break;
                     case 2: result.set_noclamp_rgb( (c0.r * 2 + c1.r) / 3, (c0.g * 2 + c1.g) / 3, (c0.b * 2 + c1.b) / 3); break;
                     case 3: result.set_noclamp_rgb( (c1.r * 2 + c0.r) / 3, (c1.g * 2 + c0.g) / 3, (c1.b * 2 + c0.b) / 3); break;
                  }
               }
               else
               {
                  switch (s)
                  {
                     case 0: result.set_noclamp_rgb(c0.r, c0.g, c0.b); break;
                     case 1: result.set_noclamp_rgb(c1.r, c1.g, c1.b); break;
                     case 2: result.set_noclamp_rgb( (c0.r + c1.r) >> 1U, (c0.g + c1.g) >> 1U, (c0.b + c1.b) >> 1U); break;
                     case 3:
                     {
                        if (m_format <= cDXT1A)
                           result.set_noclamp_rgba(0, 0, 0, 0);
                        else
                           result.set_noclamp_rgb(0, 0, 0);
                        break;
                     }
                  }
               }

               break;
            }
            case cAlpha5:
            {
               const int comp_index = m_element_component_index[element_index];

               const dxt5_block* pBlock = reinterpret_cast<const dxt5_block*>(&get_element(block_x, block_y, element_index));

               const uint l = pBlock->get_low_alpha();
               const uint h = pBlock->get_high_alpha();

               const uint s = pBlock->get_selector(x & 3, y & 3);

               if (l > h)
               {
                  switch (s)
                  {
                     case 0: result[comp_index] = static_cast<uint8>(l); break;
                     case 1: result[comp_index] = static_cast<uint8>(h); break;
                     case 2: result[comp_index] = static_cast<uint8>((l * 6 + h    ) / 7); break;
                     case 3: result[comp_index] = static_cast<uint8>((l * 5 + h * 2) / 7); break;
                     case 4: result[comp_index] = static_cast<uint8>((l * 4 + h * 3) / 7); break;
                     case 5: result[comp_index] = static_cast<uint8>((l * 3 + h * 4) / 7); break;
                     case 6: result[comp_index] = static_cast<uint8>((l * 2 + h * 5) / 7); break;
                     case 7: result[comp_index] = static_cast<uint8>((l     + h * 6) / 7); break;
                  }
               }
               else
               {
                  switch (s)
                  {
                     case 0: result[comp_index] = static_cast<uint8>(l); break;
                     case 1: result[comp_index] = static_cast<uint8>(h); break;
                     case 2: result[comp_index] = static_cast<uint8>((l * 4 + h    ) / 5); break;
                     case 3: result[comp_index] = static_cast<uint8>((l * 3 + h * 2) / 5); break;
                     case 4: result[comp_index] = static_cast<uint8>((l * 2 + h * 3) / 5); break;
                     case 5: result[comp_index] = static_cast<uint8>((l     + h * 4) / 5); break;
                     case 6: result[comp_index] = 0; break;
                     case 7: result[comp_index] = 255; break;
                  }
               }

               break;
            }
            case cAlpha3:
            {
               const int comp_index = m_element_component_index[element_index];

               const dxt3_block* pBlock = reinterpret_cast<const dxt3_block*>(&get_element(block_x, block_y, element_index));

               result[comp_index] = static_cast<uint8>(pBlock->get_alpha(x & 3, y & 3, true));

               break;
            }
            default: break;
         }
      }

      return result;
   }

   uint dxt_image::get_pixel_alpha(uint x, uint y, uint element_index) const
   {
      CRNLIB_ASSERT((x < m_width) && (y < m_height) && (element_index < m_num_elements_per_block));

      const uint block_x = x >> cDXTBlockShift;
      const uint block_y = y >> cDXTBlockShift;

      switch (m_element_type[element_index])
      {
         case cColor:
         {
            if (m_format <= cDXT1A)
            {
               const dxt1_block* pBlock = reinterpret_cast<const dxt1_block*>(&get_element(block_x, block_y, element_index));

               const uint l = pBlock->get_low_color();
               const uint h = pBlock->get_high_color();

               if (l <= h)
               {
                  uint s = pBlock->get_selector(x & 3, y & 3);

                  return (s == 3) ? 0 : 255;
               }
               else
               {
                  return 255;
               }
            }

            break;
         }
         case cAlpha5:
         {
            const dxt5_block* pBlock = reinterpret_cast<const dxt5_block*>(&get_element(block_x, block_y, element_index));

            const uint l = pBlock->get_low_alpha();
            const uint h = pBlock->get_high_alpha();

            const uint s = pBlock->get_selector(x & 3, y & 3);

            if (l > h)
            {
               switch (s)
               {
                  case 0: return l;
                  case 1: return h;
                  case 2: return (l * 6 + h    ) / 7;
                  case 3: return (l * 5 + h * 2) / 7;
                  case 4: return (l * 4 + h * 3) / 7;
                  case 5: return (l * 3 + h * 4) / 7;
                  case 6: return (l * 2 + h * 5) / 7;
                  case 7: return (l     + h * 6) / 7;
               }
            }
            else
            {
               switch (s)
               {
                  case 0: return l;
                  case 1: return h;
                  case 2: return (l * 4 + h    ) / 5;
                  case 3: return (l * 3 + h * 2) / 5;
                  case 4: return (l * 2 + h * 3) / 5;
                  case 5: return (l     + h * 4) / 5;
                  case 6: return 0;
                  case 7: return 255;
               }
            }
         }
         case cAlpha3:
         {
            const dxt3_block* pBlock = reinterpret_cast<const dxt3_block*>(&get_element(block_x, block_y, element_index));

            return pBlock->get_alpha(x & 3, y & 3, true);
         }
         default: break;
      }

      return 255;
   }

   void dxt_image::set_pixel(uint x, uint y, const color_quad_u8& c, bool perceptual)
   {
      CRNLIB_ASSERT((x < m_width) && (y < m_height));

      const uint block_x = x >> cDXTBlockShift;
      const uint block_y = y >> cDXTBlockShift;

      element* pElement = &get_element(block_x, block_y, 0);

      for (uint element_index = 0; element_index < m_num_elements_per_block; element_index++, pElement++)
      {
         switch (m_element_type[element_index])
         {
            case cColor:
            {
               dxt1_block* pDXT1_block = reinterpret_cast<dxt1_block*>(pElement);

               color_quad_u8 colors[cDXT1SelectorValues];
               const uint n = pDXT1_block->get_block_colors(colors, static_cast<uint16>(pDXT1_block->get_low_color()), static_cast<uint16>(pDXT1_block->get_high_color()));

               if ((m_format == cDXT1A) && (c.a < 128))
                  pDXT1_block->set_selector(x & 3, y & 3, 3);
               else
               {
                  uint best_error = UINT_MAX;
                  uint best_selector = 0;

                  for (uint i = 0; i < n; i++)
                  {
                     uint error = color::color_distance(perceptual, colors[i], c, false);
                     if (error < best_error)
                     {
                        best_error = error;
                        best_selector = i;
                     }
                  }

                  pDXT1_block->set_selector(x & 3, y & 3, best_selector);
               }

               break;
            }
            case cAlpha5:
            {
               dxt5_block* pDXT5_block = reinterpret_cast<dxt5_block*>(pElement);

               uint values[cDXT5SelectorValues];
               dxt5_block::get_block_values(values, pDXT5_block->get_low_alpha(), pDXT5_block->get_high_alpha());

               const int comp_index = m_element_component_index[element_index];

               uint best_error = UINT_MAX;
               uint best_selector = 0;

               for (uint i = 0; i < cDXT5SelectorValues; i++)
               {
                  uint error = labs(values[i] - c[comp_index]); // no need to square

                  if (error < best_error)
                  {
                     best_error = error;
                     best_selector = i;
                  }
               }

               pDXT5_block->set_selector(x & 3, y & 3, best_selector);

               break;
            }
            case cAlpha3:
            {
               const int comp_index = m_element_component_index[element_index];

               dxt3_block* pDXT3_block = reinterpret_cast<dxt3_block*>(pElement);

               pDXT3_block->set_alpha(x & 3, y & 3, c[comp_index], true);

               break;
            }
            default: break;
         }
      } // element_index
   }

   void dxt_image::get_block_pixels(uint block_x, uint block_y, color_quad_u8* pPixels) const
   {
      const element* pElement = &get_element(block_x, block_y, 0);

      for (uint element_index = 0; element_index < m_num_elements_per_block; element_index++, pElement++)
      {
         switch (m_element_type[element_index])
         {
            case cColor:
            {
               const dxt1_block* pDXT1_block = reinterpret_cast<const dxt1_block*>(pElement);

               color_quad_u8 colors[cDXT1SelectorValues];
               pDXT1_block->get_block_colors(colors, static_cast<uint16>(pDXT1_block->get_low_color()), static_cast<uint16>(pDXT1_block->get_high_color()));

               for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
               {
                  uint s = pDXT1_block->get_selector(i & 3, i >> 2);

                  pPixels[i].r = colors[s].r;
                  pPixels[i].g = colors[s].g;
                  pPixels[i].b = colors[s].b;

                  if (m_format <= cDXT1A)
                     pPixels[i].a = colors[s].a;
               }

               break;
            }
            case cAlpha5:
            {
               const dxt5_block* pDXT5_block = reinterpret_cast<const dxt5_block*>(pElement);

               uint values[cDXT5SelectorValues];
               dxt5_block::get_block_values(values, pDXT5_block->get_low_alpha(), pDXT5_block->get_high_alpha());

               const int comp_index = m_element_component_index[element_index];

               for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
               {
                  uint s = pDXT5_block->get_selector(i & 3, i >> 2);

                  pPixels[i][comp_index] = static_cast<uint8>(values[s]);
               }

               break;
            }
            case cAlpha3:
            {
               const dxt3_block* pDXT3_block = reinterpret_cast<const dxt3_block*>(pElement);

               const int comp_index = m_element_component_index[element_index];

               for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
               {
                  uint a = pDXT3_block->get_alpha(i & 3, i >> 2, true);

                  pPixels[i][comp_index] = static_cast<uint8>(a);
               }

               break;
            }
            default: break;
         }
      } // element_index
   }

   void dxt_image::set_block_pixels(uint block_x, uint block_y, const color_quad_u8* pPixels, const pack_params& p)
   {
      dxt1_endpoint_optimizer dxt1_optimizer;
      dxt5_endpoint_optimizer dxt5_optimizer;
      set_block_pixels(block_x, block_y, pPixels, p, dxt1_optimizer, dxt5_optimizer);
   }

   void dxt_image::set_block_pixels(
      uint block_x, uint block_y, const color_quad_u8* pPixels, const pack_params& p,
      dxt1_endpoint_optimizer& dxt1_optimizer, dxt5_endpoint_optimizer& dxt5_optimizer)
   {
      element* pElement = &get_element(block_x, block_y, 0);

#if CRNLIB_SUPPORT_SQUISH
      if ((p.m_compressor == cCRNDXTCompressorSquish) && ((m_format == cDXT1) || (m_format == cDXT1A) || (m_format == cDXT3) || (m_format == cDXT5) || (m_format == cDXT5A)))
      {
         uint squish_flags = 0;
         if ((m_format == cDXT1) || (m_format == cDXT1A))
            squish_flags = squish::kDxt1;
         else if (m_format == cDXT3)
            squish_flags = squish::kDxt3;
         else  if (m_format == cDXT5A)
            squish_flags = squish::kDxt5A;
         else
            squish_flags = squish::kDxt5;

         if (p.m_perceptual)
            squish_flags |= squish::kColourMetricPerceptual;
         else
            squish_flags |= squish::kColourMetricUniform;

         if (p.m_quality >= cCRNDXTQualityBetter)
            squish_flags |= squish::kColourIterativeClusterFit;
         else if (p.m_quality == cCRNDXTQualitySuperFast)
            squish_flags |= squish::kColourRangeFit;

         color_quad_u8 pixels[cDXTBlockSize * cDXTBlockSize];

         memcpy(pixels, pPixels, sizeof(color_quad_u8) * cDXTBlockSize * cDXTBlockSize);

         if (m_format == cDXT1)
         {
            for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
               pixels[i].a = 255;
         }
         else if (m_format == cDXT1A)
         {
            for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
               if (pixels[i].a < p.m_dxt1a_alpha_threshold)
                  pixels[i].a = 0;
               else
                  pixels[i].a = 255;
         }

         squish::Compress(reinterpret_cast<const squish::u8*>(pixels), pElement, squish_flags);
      }

      else
#endif // CRNLIB_SUPPORT_SQUISH
      // RYG doesn't support DXT1A
      if ((p.m_compressor == cCRNDXTCompressorRYG) && ((m_format == cDXT1) || (m_format == cDXT5) || (m_format == cDXT5A)))
      {
         color_quad_u8 pixels[cDXTBlockSize * cDXTBlockSize];

         for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
         {
            pixels[i].r = pPixels[i].b;
            pixels[i].g = pPixels[i].g;
            pixels[i].b = pPixels[i].r;

            if (m_format == cDXT1)
               pixels[i].a = 255;
            else
               pixels[i].a = pPixels[i].a;
         }

         if (m_format == cDXT5A)
            ryg_dxt::sCompressDXT5ABlock((sU8*)pElement, (const sU32*)pixels, 0);
         else
            ryg_dxt::sCompressDXTBlock((sU8*)pElement, (const sU32*)pixels, m_format == cDXT5, 0);
      }
      else if ((p.m_compressor == cCRNDXTCompressorCRNF) && (m_format != cDXT1A))
      {
         for (uint element_index = 0; element_index < m_num_elements_per_block; element_index++, pElement++)
         {
            switch (m_element_type[element_index])
            {
               case cColor:
               {
                  dxt1_block* pDXT1_block = reinterpret_cast<dxt1_block*>(pElement);
                  dxt_fast::compress_color_block(pDXT1_block, pPixels, p.m_quality >= cCRNDXTQualityNormal);

                  break;
               }
               case cAlpha5:
               {
                  dxt5_block* pDXT5_block = reinterpret_cast<dxt5_block*>(pElement);
                  dxt_fast::compress_alpha_block(pDXT5_block, pPixels, m_element_component_index[element_index]);

                  break;
               }
               case cAlpha3:
               {
                  const int comp_index = m_element_component_index[element_index];

                  dxt3_block* pDXT3_block = reinterpret_cast<dxt3_block*>(pElement);

                  for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
                     pDXT3_block->set_alpha(i & 3, i >> 2, pPixels[i][comp_index], true);

                  break;
               }
               default: break;
            }
         }
      }
      else
      {
         for (uint element_index = 0; element_index < m_num_elements_per_block; element_index++, pElement++)
         {
            switch (m_element_type[element_index])
            {
               case cColor:
               {
                  dxt1_block* pDXT1_block = reinterpret_cast<dxt1_block*>(pElement);

                  bool pixels_have_alpha = false;
                  if (m_format == cDXT1A)
                  {
                     for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
                        if (pPixels[i].a < p.m_dxt1a_alpha_threshold)
                        {
                           pixels_have_alpha = true;
                           break;
                        }
                  }

                  dxt1_endpoint_optimizer::results results;
                  uint8 selectors[cDXTBlockSize * cDXTBlockSize];
                  results.m_pSelectors = selectors;

                  dxt1_endpoint_optimizer::params params;
                  params.m_block_index = block_x + block_y * m_blocks_x;
                  params.m_quality = p.m_quality;
                  params.m_perceptual = p.m_perceptual;
                  params.m_grayscale_sampling = p.m_grayscale_sampling;
                  params.m_pixels_have_alpha = pixels_have_alpha;
                  params.m_use_alpha_blocks = p.m_use_both_block_types;
                  params.m_use_transparent_indices_for_black = p.m_use_transparent_indices_for_black;
                  params.m_dxt1a_alpha_threshold = p.m_dxt1a_alpha_threshold;
                  params.m_pPixels = pPixels;
                  params.m_num_pixels = cDXTBlockSize * cDXTBlockSize;
                  params.m_endpoint_caching = p.m_endpoint_caching;
                  params.m_color_weights[0] = p.m_color_weights[0];
                  params.m_color_weights[1] = p.m_color_weights[1];
                  params.m_color_weights[2] = p.m_color_weights[2];

                  if ((m_format != cDXT1) && (m_format != cDXT1A))
                     params.m_use_alpha_blocks = false;

                  if (!dxt1_optimizer.compute(params, results))
                  {
                     CRNLIB_ASSERT(0);
                     break;
                  }

                  pDXT1_block->set_low_color(results.m_low_color);
                  pDXT1_block->set_high_color(results.m_high_color);

                  for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
                     pDXT1_block->set_selector(i & 3, i >> 2, selectors[i]);

                  break;
               }
               case cAlpha5:
               {
                  dxt5_block* pDXT5_block = reinterpret_cast<dxt5_block*>(pElement);

                  dxt5_endpoint_optimizer::results results;

                  uint8 selectors[cDXTBlockSize * cDXTBlockSize];
                  results.m_pSelectors = selectors;

                  dxt5_endpoint_optimizer::params params;
                  params.m_block_index = block_x + block_y * m_blocks_x;
                  params.m_pPixels = pPixels;
                  params.m_num_pixels = cDXTBlockSize * cDXTBlockSize;
                  params.m_comp_index = m_element_component_index[element_index];
                  params.m_quality = p.m_quality;
                  params.m_use_both_block_types = p.m_use_both_block_types;

                  if (!dxt5_optimizer.compute(params, results))
                  {
                     CRNLIB_ASSERT(0);
                     break;
                  }

                  pDXT5_block->set_low_alpha(results.m_first_endpoint);
                  pDXT5_block->set_high_alpha(results.m_second_endpoint);

                  for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
                     pDXT5_block->set_selector(i & 3, i >> 2, selectors[i]);

                  break;
               }
               case cAlpha3:
               {
                  const int comp_index = m_element_component_index[element_index];

                  dxt3_block* pDXT3_block = reinterpret_cast<dxt3_block*>(pElement);

                  for (uint i = 0; i < cDXTBlockSize * cDXTBlockSize; i++)
                     pDXT3_block->set_alpha(i & 3, i >> 2, pPixels[i][comp_index], true);

                  break;
               }
               default: break;
            }
         }
      }
   }

   void dxt_image::get_block_endpoints(uint block_x, uint block_y, uint element_index, uint& packed_low_endpoint, uint& packed_high_endpoint) const
   {
      const element& block = get_element(block_x, block_y, element_index);

      switch (m_element_type[element_index])
      {
         case cColor:
         {
            const dxt1_block& block1 = *reinterpret_cast<const dxt1_block*>(&block);

            packed_low_endpoint = block1.get_low_color();
            packed_high_endpoint = block1.get_high_color();

            break;
         }
         case cAlpha5:
         {
            const dxt5_block& block5 = *reinterpret_cast<const dxt5_block*>(&block);

            packed_low_endpoint = block5.get_low_alpha();
            packed_high_endpoint = block5.get_high_alpha();

            break;
         }
         case cAlpha3:
         {
            packed_low_endpoint = 0;
            packed_high_endpoint = 255;

            break;
         }
         default: break;
      }
   }

   int dxt_image::get_block_endpoints(uint block_x, uint block_y, uint element_index, color_quad_u8& low_endpoint, color_quad_u8& high_endpoint, bool scaled) const
   {
      uint l, h;
      get_block_endpoints(block_x, block_y, element_index, l, h);

      switch (m_element_type[element_index])
      {
         case cColor:
         {
            uint r, g, b;

            dxt1_block::unpack_color(r, g, b, static_cast<uint16>(l), scaled);
            low_endpoint.r = static_cast<uint8>(r);
            low_endpoint.g = static_cast<uint8>(g);
            low_endpoint.b = static_cast<uint8>(b);

            dxt1_block::unpack_color(r, g, b, static_cast<uint16>(h), scaled);
            high_endpoint.r = static_cast<uint8>(r);
            high_endpoint.g = static_cast<uint8>(g);
            high_endpoint.b = static_cast<uint8>(b);

            return -1;
         }
         case cAlpha5:
         {
            const int component = m_element_component_index[element_index];

            low_endpoint[component] = static_cast<uint8>(l);
            high_endpoint[component] = static_cast<uint8>(h);

            return component;
         }
         case cAlpha3:
         {
            const int component = m_element_component_index[element_index];

            low_endpoint[component] = static_cast<uint8>(l);
            high_endpoint[component] = static_cast<uint8>(h);

            return component;
         }
         default: break;
      }

      return 0;
   }

   uint dxt_image::get_block_colors(uint block_x, uint block_y, uint element_index, color_quad_u8* pColors)
   {
      const element& block = get_element(block_x, block_y, element_index);

      switch (m_element_type[element_index])
      {
         case cColor:
         {
            const dxt1_block& block1 = *reinterpret_cast<const dxt1_block*>(&block);
            return dxt1_block::get_block_colors(pColors, static_cast<uint16>(block1.get_low_color()), static_cast<uint16>(block1.get_high_color()));
         }
         case cAlpha5:
         {
            const dxt5_block& block5 = *reinterpret_cast<const dxt5_block*>(&block);

            uint values[cDXT5SelectorValues];

            const uint n = dxt5_block::get_block_values(values, block5.get_low_alpha(), block5.get_high_alpha());

            const int comp_index = m_element_component_index[element_index];
            for (uint i = 0; i < n; i++)
               pColors[i][comp_index] = static_cast<uint8>(values[i]);

            return n;
         }
         case cAlpha3:
         {
            const int comp_index = m_element_component_index[element_index];
            for (uint i = 0; i < 16; i++)
               pColors[i][comp_index] = static_cast<uint8>((i << 4) | i);

            return 16;
         }
         default: break;
      }

      return 0;
   }

   uint dxt_image::get_selector(uint x, uint y, uint element_index) const
   {
      CRNLIB_ASSERT((x < m_width) && (y < m_height));

      const uint block_x = x >> cDXTBlockShift;
      const uint block_y = y >> cDXTBlockShift;

      const element& block = get_element(block_x, block_y, element_index);

      switch (m_element_type[element_index])
      {
         case cColor:
         {
            const dxt1_block& block1 = *reinterpret_cast<const dxt1_block*>(&block);
            return block1.get_selector(x & 3, y & 3);
         }
         case cAlpha5:
         {
            const dxt5_block& block5 = *reinterpret_cast<const dxt5_block*>(&block);
            return block5.get_selector(x & 3, y & 3);
         }
         case cAlpha3:
         {
            const dxt3_block& block3 = *reinterpret_cast<const dxt3_block*>(&block);
            return block3.get_alpha(x & 3, y & 3, false);
         }
         default: break;
      }

      return 0;
   }

   void dxt_image::change_dxt1_to_dxt1a()
   {
      if (m_format == cDXT1)
         m_format = cDXT1A;
   }

} // namespace crnlib




