// File: crn_image_utils.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_image_utils.h"
#include "crn_console.h"
#include "crn_resampler.h"
#include "crn_threaded_resampler.h"
#include "crn_strutils.h"

#define STBI_HEADER_FILE_ONLY
#include "crn_stb_image.cpp"

#include "crn_pixel_format.h"

namespace crnlib
{
   const float cInfinitePSNR = 999999.0f;

   namespace image_utils
   {
      bool load_from_file_stb(const wchar_t* pFilename, image_u8& img)
      {
         int x = 0, y = 0, n = 0;
         unsigned char* pData = stbi_load_w(pFilename, &x, &y, &n, 4);

         if (!pData)
            return false;

         if ((x > 8192) || (y > 8192))
         {
            stbi_image_free(pData);
            return false;
         }

         const bool has_alpha = ((n == 2) || (n == 4));

         img.resize(x, y);

         bool grayscale = true;

         for (int py = 0; py < y; py++)
         {
            const color_quad_u8* pSrc = reinterpret_cast<const color_quad_u8*>(pData) + (py * x);
            color_quad_u8* pDst = img.get_scanline(py);
            color_quad_u8* pDst_end = pDst + x;

            while (pDst != pDst_end)
            {
               color_quad_u8 c(*pSrc++);
               if (!has_alpha)
                  c.a = 255;

               if (!c.is_grayscale())
                  grayscale = false;

               *pDst++ = c;
            }
         }

         stbi_image_free(pData);

         img.reset_comp_flags();
         img.set_grayscale(grayscale);
         img.set_component_valid(3, has_alpha);

         return true;
      }

      bool save_to_file_stb(const wchar_t* pFilename, const image_u8& img, uint save_flags, int comp_index)
      {
         if (!img.get_width())
            return false;

         bool bSaveBMP = false;
         dynamic_wstring ext(pFilename);
         if (get_extension(ext))
         {
            if (ext == L"bmp")
               bSaveBMP = true;
            else if (ext != L"tga")
            {
               console::error(L"crnlib::image_utils::save_to_file_stb: Can only write .BMP or .TGA files!\n");
               return false;
            }
         }

         if ((img.get_comp_flags() & pixel_format_helpers::cCompFlagGrayscale) || (save_flags & image_utils::cSaveGrayscale))
         {
            CRNLIB_ASSERT(comp_index < 4);
            if (comp_index > 3) comp_index = 3;

            crnlib::vector<uint8> temp(img.get_total_pixels());

            for (uint y = 0; y < img.get_height(); y++)
            {
               const color_quad_u8* pSrc = img.get_scanline(y);
               const color_quad_u8* pSrc_end = pSrc + img.get_width();
               uint8* pDst = &temp[y * img.get_width()];

               if (img.get_comp_flags() & pixel_format_helpers::cCompFlagGrayscale)
               {
                  while (pSrc != pSrc_end)
                     *pDst++ = (*pSrc++)[1];
               }
               else if (comp_index < 0)
               {
                  while (pSrc != pSrc_end)
                     *pDst++ = static_cast<uint8>((*pSrc++).get_luma());
               }
               else
               {
                  while (pSrc != pSrc_end)
                     *pDst++ = (*pSrc++)[comp_index];
               }
            }

            return (bSaveBMP ? stbi_write_bmp_w : stbi_write_tga_w)(pFilename, img.get_width(), img.get_height(), 1, &temp[0]) == CRNLIB_TRUE;
         }
         else if ((!img.is_component_valid(3)) || (save_flags & cSaveIgnoreAlpha))
         {
            crnlib::vector<uint8> temp(img.get_total_pixels() * 3);

            for (uint y = 0; y < img.get_height(); y++)
            {
               const color_quad_u8* pSrc = img.get_scanline(y);
               const color_quad_u8* pSrc_end = pSrc + img.get_width();
               uint8* pDst = &temp[y * img.get_width() * 3];

               while (pSrc != pSrc_end)
               {
                  const color_quad_u8 c(*pSrc++);

                  pDst[0] = c.r;
                  pDst[1] = c.g;
                  pDst[2] = c.b;

                  pDst += 3;
               }
            }

            return (bSaveBMP ? stbi_write_bmp_w : stbi_write_tga_w)(pFilename, img.get_width(), img.get_height(), 3, &temp[0]) == CRNLIB_TRUE;
         }
         else
         {
            return (bSaveBMP ? stbi_write_bmp_w : stbi_write_tga_w)(pFilename, img.get_width(), img.get_height(), 4, img.get_ptr()) == CRNLIB_TRUE;
         }
      }

      bool load_from_file(image_u8& dest, const wchar_t* pFilename, int flags)
      {
         flags;
         return image_utils::load_from_file_stb(pFilename, dest);
      }

      bool save_to_grayscale_file(const wchar_t* pFilename, const image_u8& src, int component, int flags)
      {
         flags;
         return image_utils::save_to_file_stb(pFilename, src, image_utils::cSaveGrayscale, component);
      }

      bool save_to_file(const wchar_t* pFilename, const image_u8& src, int flags, bool ignore_alpha)
      {
         if (src.is_grayscale())
            return save_to_grayscale_file(pFilename, src, cSaveLuma, flags);
         else
         {
            uint save_flags = 0;
            if (ignore_alpha)
               save_flags |= image_utils::cSaveIgnoreAlpha;
            return image_utils::save_to_file_stb(pFilename, src, save_flags);
         }
      }

      bool has_alpha(const image_u8& img)
      {
         for (uint y = 0; y < img.get_height(); y++)
            for (uint x = 0; x < img.get_width(); x++)
               if (img(x, y).a < 255)
                  return true;

         return false;
      }

      void renorm_normal_map(image_u8& img)
      {
         for (uint y = 0; y < img.get_height(); y++)
         {
            for (uint x = 0; x < img.get_width(); x++)
            {
               color_quad_u8& c = img(x, y);
               if ((c.r == 128) && (c.g == 128) && (c.b == 128))
                  continue;

               vec3F v(c.r, c.g, c.b);
               v *= 1.0f/255.0f;
               v *= 2.0f;
               v -= vec3F(1.0f);
               v.clamp(-1.0f, 1.0f);

               float length = v.length();
               if (length < .077f)
                  c.set(128, 128, 128, c.a);
               else if (fabs(length - 1.0f) > .077f)
               {
                  if (length)
                     v /= length;

                  for (uint i = 0; i < 3; i++)
                     c[i] = static_cast<uint8>(math::clamp<float>(floor((v[i] + 1.0f) * .5f * 255.0f + .5f), 0.0f, 255.0f));

                  if ((c.r == 128) && (c.g == 128))
                  {
                     if (c.b < 128)
                        c.b = 0;
                     else
                        c.b = 255;
                  }
               }
            }
         }
      }

      bool is_normal_map(const image_u8& img, const wchar_t* pFilename)
      {
         float score = 0.0f;

         uint num_invalid_pixels = 0;

         // TODO: Derive better score from pixel mean, eigenvecs/vals
         //crnlib::vector<vec3F> pixels;

         for (uint y = 0; y < img.get_height(); y++)
         {
            for (uint x = 0; x < img.get_width(); x++)
            {
               const color_quad_u8& c = img(x, y);

               if (c.b < 123)
               {
                  num_invalid_pixels++;
                  continue;
               }
               else if ((c.r != 128) || (c.g != 128) || (c.b != 128))
               {
                  vec3F v(c.r, c.g, c.b);
                  v -= vec3F(128.0f);
                  v /= vec3F(127.0f);
                  //pixels.push_back(v);
                  v.clamp(-1.0f, 1.0f);

                  float norm = v.norm();
                  if ((norm < 0.83f) || (norm > 1.29f))
                     num_invalid_pixels++;
               }
            }
         }

         score -= math::clamp(float(num_invalid_pixels) / (img.get_width() * img.get_height()) - .026f, 0.0f, 1.0f) * 5.0f;

         if (pFilename)
         {
            dynamic_wstring str(pFilename);
            str.tolower();

            if (str.contains(L"normal")  || str.contains(L"local") || str.contains(L"nmap"))
               score += 1.0f;

            if (str.contains(L"diffuse") || str.contains(L"spec") || str.contains(L"gloss"))
               score -= 1.0f;
         }

         return score >= 0.0f;
      }

      bool resample_single_thread(const image_u8& src, image_u8& dst, const resample_params& params)
      {
         const uint src_width = src.get_width();
         const uint src_height = src.get_height();

         if (math::maximum(src_width, src_height) > CRNLIB_RESAMPLER_MAX_DIMENSION)
         {
            printf("Image is too large!\n");
            return EXIT_FAILURE;
         }

         const int cMaxComponents = 4;
         if (((int)params.m_num_comps < 1) || ((int)params.m_num_comps > (int)cMaxComponents))
            return false;

         const uint dst_width = params.m_dst_width;
         const uint dst_height = params.m_dst_height;

         if ((math::minimum(dst_width, dst_height) < 1) || (math::maximum(dst_width, dst_height) > CRNLIB_RESAMPLER_MAX_DIMENSION))
         {
            printf("Image is too large!\n");
            return EXIT_FAILURE;
         }

         if ((src_width == dst_width) && (src_height == dst_height))
         {
            dst = src;
            return true;
         }

         dst.clear();
         dst.resize(params.m_dst_width, params.m_dst_height);

         // Partial gamma correction looks better on mips. Set to 1.0 to disable gamma correction.
         const float source_gamma = params.m_source_gamma;//1.75f;

         float srgb_to_linear[256];
         if (params.m_srgb)
         {
            for (int i = 0; i < 256; ++i)
               srgb_to_linear[i] = (float)pow(i * 1.0f/255.0f, source_gamma);
         }

         const int linear_to_srgb_table_size = 8192;
         unsigned char linear_to_srgb[linear_to_srgb_table_size];

         const float inv_linear_to_srgb_table_size = 1.0f / linear_to_srgb_table_size;
         const float inv_source_gamma = 1.0f / source_gamma;

         if (params.m_srgb)
         {
            for (int i = 0; i < linear_to_srgb_table_size; ++i)
            {
               int k = (int)(255.0f * pow(i * inv_linear_to_srgb_table_size, inv_source_gamma) + .5f);
               if (k < 0) k = 0; else if (k > 255) k = 255;
               linear_to_srgb[i] = (unsigned char)k;
            }
         }

         Resampler* resamplers[cMaxComponents];
         crnlib::vector<float> samples[cMaxComponents];

         resamplers[0] = crnlib_new<Resampler>(src_width, src_height, dst_width, dst_height,
            params.m_wrapping ? Resampler::BOUNDARY_WRAP : Resampler::BOUNDARY_CLAMP, 0.0f, 1.0f,
            params.m_pFilter, (Resampler::Contrib_List*)NULL, (Resampler::Contrib_List*)NULL, params.m_filter_scale, params.m_filter_scale);
         samples[0].resize(src_width);

         for (uint i = 1; i < params.m_num_comps; i++)
         {
            resamplers[i] = crnlib_new<Resampler>(src_width, src_height, dst_width, dst_height,
               params.m_wrapping ? Resampler::BOUNDARY_WRAP : Resampler::BOUNDARY_CLAMP, 0.0f, 1.0f,
               params.m_pFilter, resamplers[0]->get_clist_x(), resamplers[0]->get_clist_y(), params.m_filter_scale, params.m_filter_scale);
            samples[i].resize(src_width);
         }

         uint dst_y = 0;

         for (uint src_y = 0; src_y < src_height; src_y++)
         {
            const color_quad_u8* pSrc = src.get_scanline(src_y);

            for (uint x = 0; x < src_width; x++)
            {
               for (uint c = 0; c < params.m_num_comps; c++)
               {
                  const uint comp_index = params.m_first_comp + c;
                  const uint8 v = (*pSrc)[comp_index];

                  if (!params.m_srgb || (comp_index == 3))
                     samples[c][x] = v * (1.0f/255.0f);
                  else
                     samples[c][x] = srgb_to_linear[v];
               }

               pSrc++;
            }

            for (uint c = 0; c < params.m_num_comps; c++)
            {
               if (!resamplers[c]->put_line(&samples[c][0]))
               {
                  for (uint i = 0; i < params.m_num_comps; i++)
                     crnlib_delete(resamplers[i]);
                  return false;
               }
            }

            for ( ; ; )
            {
               uint c;
               for (c = 0; c < params.m_num_comps; c++)
               {
                  const uint comp_index = params.m_first_comp + c;

                  const float* pOutput_samples = resamplers[c]->get_line();
                  if (!pOutput_samples)
                     break;

                  const bool linear = !params.m_srgb || (comp_index == 3);
                  CRNLIB_ASSERT(dst_y < dst_height);
                  color_quad_u8* pDst = dst.get_scanline(dst_y);

                  for (uint x = 0; x < dst_width; x++)
                  {
                     if (linear)
                     {
                        int c = (int)(255.0f * pOutput_samples[x] + .5f);
                        if (c < 0) c = 0; else if (c > 255) c = 255;
                        (*pDst)[comp_index] = (unsigned char)c;
                     }
                     else
                     {
                        int j = (int)(linear_to_srgb_table_size * pOutput_samples[x] + .5f);
                        if (j < 0) j = 0; else if (j >= linear_to_srgb_table_size) j = linear_to_srgb_table_size - 1;
                        (*pDst)[comp_index] = linear_to_srgb[j];
                     }

                     pDst++;
                  }
               }
               if (c < params.m_num_comps)
                  break;

               dst_y++;
            }
         }

         for (uint i = 0; i < params.m_num_comps; i++)
            crnlib_delete(resamplers[i]);

         return true;
      }

      bool resample_multithreaded(const image_u8& src, image_u8& dst, const resample_params& params)
      {
         const uint src_width = src.get_width();
         const uint src_height = src.get_height();

         if (math::maximum(src_width, src_height) > CRNLIB_RESAMPLER_MAX_DIMENSION)
         {
            printf("Image is too large!\n");
            return EXIT_FAILURE;
         }

         const int cMaxComponents = 4;
         if (((int)params.m_num_comps < 1) || ((int)params.m_num_comps > (int)cMaxComponents))
            return false;

         const uint dst_width = params.m_dst_width;
         const uint dst_height = params.m_dst_height;

         if ((math::minimum(dst_width, dst_height) < 1) || (math::maximum(dst_width, dst_height) > CRNLIB_RESAMPLER_MAX_DIMENSION))
         {
            printf("Image is too large!\n");
            return EXIT_FAILURE;
         }

         if ((src_width == dst_width) && (src_height == dst_height))
         {
            dst = src;
            return true;
         }

         dst.clear();

         // Partial gamma correction looks better on mips. Set to 1.0 to disable gamma correction.
         const float source_gamma = params.m_source_gamma;//1.75f;

         float srgb_to_linear[256];
         if (params.m_srgb)
         {
            for (int i = 0; i < 256; ++i)
               srgb_to_linear[i] = (float)pow(i * 1.0f/255.0f, source_gamma);
         }

         const int linear_to_srgb_table_size = 8192;
         unsigned char linear_to_srgb[linear_to_srgb_table_size];

         const float inv_linear_to_srgb_table_size = 1.0f / linear_to_srgb_table_size;
         const float inv_source_gamma = 1.0f / source_gamma;

         if (params.m_srgb)
         {
            for (int i = 0; i < linear_to_srgb_table_size; ++i)
            {
               int k = (int)(255.0f * pow(i * inv_linear_to_srgb_table_size, inv_source_gamma) + .5f);
               if (k < 0) k = 0; else if (k > 255) k = 255;
               linear_to_srgb[i] = (unsigned char)k;
            }
         }

         task_pool tp;
         tp.init(g_number_of_processors - 1);

         threaded_resampler resampler(tp);
         threaded_resampler::params p;
         p.m_src_width = src_width;
         p.m_src_height = src_height;
         p.m_dst_width = dst_width;
         p.m_dst_height = dst_height;
         p.m_sample_low = 0.0f;
         p.m_sample_high = 1.0f;
         p.m_boundary_op = params.m_wrapping ? Resampler::BOUNDARY_WRAP : Resampler::BOUNDARY_CLAMP;
         p.m_Pfilter_name = params.m_pFilter;
         p.m_filter_x_scale = params.m_filter_scale;
         p.m_filter_y_scale = params.m_filter_scale;

         uint resampler_comps = 4;
         if (params.m_num_comps == 1)
         {
            p.m_fmt = threaded_resampler::cPF_Y_F32;
            resampler_comps = 1;
         }
         else if (params.m_num_comps <= 3)
            p.m_fmt = threaded_resampler::cPF_RGBX_F32;
         else
            p.m_fmt = threaded_resampler::cPF_RGBA_F32;

         crnlib::vector<float> src_samples;
         crnlib::vector<float> dst_samples;

         if (!src_samples.try_resize(src_width * src_height * resampler_comps))
            return false;

         if (!dst_samples.try_resize(dst_width * dst_height * resampler_comps))
            return false;

         p.m_pSrc_pixels = src_samples.get_ptr();
         p.m_src_pitch = src_width * resampler_comps * sizeof(float);
         p.m_pDst_pixels = dst_samples.get_ptr();
         p.m_dst_pitch = dst_width * resampler_comps * sizeof(float);

         for (uint src_y = 0; src_y < src_height; src_y++)
         {
            const color_quad_u8* pSrc = src.get_scanline(src_y);
            float* pDst = src_samples.get_ptr() + src_width * resampler_comps * src_y;

            for (uint x = 0; x < src_width; x++)
            {
               for (uint c = 0; c < params.m_num_comps; c++)
               {
                  const uint comp_index = params.m_first_comp + c;
                  const uint8 v = (*pSrc)[comp_index];

                  if (!params.m_srgb || (comp_index == 3))
                     pDst[c] = v * (1.0f/255.0f);
                  else
                     pDst[c] = srgb_to_linear[v];
               }

               pSrc++;
               pDst += resampler_comps;
            }
         }

         if (!resampler.resample(p))
            return false;

         src_samples.clear();

         if (!dst.resize(params.m_dst_width, params.m_dst_height))
            return false;

         for (uint dst_y = 0; dst_y < dst_height; dst_y++)
         {
            const float* pSrc = dst_samples.get_ptr() + dst_width * resampler_comps * dst_y;
            color_quad_u8* pDst = dst.get_scanline(dst_y);

            for (uint x = 0; x < dst_width; x++)
            {
               color_quad_u8 dst(0, 0, 0, 255);

               for (uint c = 0; c < params.m_num_comps; c++)
               {
                  const uint comp_index = params.m_first_comp + c;
                  const float v = pSrc[c];

                  if ((!params.m_srgb) || (comp_index == 3))
                  {
                     int c = static_cast<int>(255.0f * v + .5f);
                     if (c < 0) c = 0; else if (c > 255) c = 255;
                     dst[comp_index] = (unsigned char)c;
                  }
                  else
                  {
                     int j = static_cast<int>(linear_to_srgb_table_size * v + .5f);
                     if (j < 0) j = 0; else if (j >= linear_to_srgb_table_size) j = linear_to_srgb_table_size - 1;
                     dst[comp_index] = linear_to_srgb[j];
                  }
               }

               *pDst++ = dst;

               pSrc += resampler_comps;
            }
         }

         return true;
      }

      bool resample(const image_u8& src, image_u8& dst, const resample_params& params)
      {
         if ((params.m_multithreaded) && (g_number_of_processors > 1))
            return resample_multithreaded(src, dst, params);
         else
            return resample_single_thread(src, dst, params);
      }

      bool compute_delta(image_u8& dest, image_u8& a, image_u8& b, uint scale)
      {
         if ( (a.get_width() != b.get_width()) || (a.get_height() != b.get_height()) )
            return false;

         dest.resize(a.get_width(), b.get_height());

         for (uint y = 0; y < a.get_height(); y++)
         {
            for (uint x = 0; x < a.get_width(); x++)
            {
               const color_quad_u8& ca = a(x, y);
               const color_quad_u8& cb = b(x, y);

               color_quad_u8 cd;
               for (uint c = 0; c < 4; c++)
               {
                  int d = (ca[c] - cb[c]) * scale + 128;
                  d = math::clamp(d, 0, 255);
                  cd[c] = static_cast<uint8>(d);
               }

               dest(x, y) = cd;
            }
         }

         return true;
      }

      // FIXME: Totally hack-ass computation.
      // Perhaps port http://www.lomont.org/Software/Misc/SSIM/SSIM.html?
      double compute_block_ssim(uint t, const uint8* pX, const uint8* pY)
      {
         double ave_x = 0.0f;
         double ave_y = 0.0f;
         for (uint i = 0; i < t; i++)
         {
            ave_x += pX[i];
            ave_y += pY[i];
         }

         ave_x /= t;
         ave_y /= t;

         double var_x = 0.0f;
         double var_y = 0.0f;
         for (uint i = 0; i < t; i++)
         {
            var_x += math::square(pX[i] - ave_x);
            var_y += math::square(pY[i] - ave_y);
         }

         var_x = sqrt(var_x / (t - 1));
         var_y = sqrt(var_y / (t - 1));

         double covar_xy = 0.0f;
         for (uint i = 0; i < t; i++)
            covar_xy += (pX[i] - ave_x) * (pY[i] - ave_y);

         covar_xy /= (t - 1);

         const double c1 = 6.5025; //(255*.01)^2
         const double c2 = 58.5225; //(255*.03)^2

         double n = (2.0f * ave_x * ave_y + c1) * (2.0f * covar_xy + c2);
         double d = (ave_x * ave_x + ave_y * ave_y + c1) * (var_x * var_x + var_y * var_y + c2);

         return n / d;
      }

      double compute_ssim(const image_u8& a, const image_u8& b, int channel_index)
      {
         const uint N = 6;
         uint8 sx[N*N], sy[N*N];

         double total_ssim = 0.0f;
         uint total_blocks = 0;

         //image_u8 yimg((a.get_width() + N - 1) / N, (a.get_height() + N - 1) / N);

         for (uint y = 0; y < a.get_height(); y += N)
         {
            for (uint x = 0; x < a.get_width(); x += N)
            {
               for (uint iy = 0; iy < N; iy++)
               {
                  for (uint ix = 0; ix < N; ix++)
                  {
                     if (channel_index < 0)
                        sx[ix+iy*N] = (uint8)a.get_clamped(x+ix, y+iy).get_luma();
                     else
                        sx[ix+iy*N] = (uint8)a.get_clamped(x+ix, y+iy)[channel_index];

                     if (channel_index < 0)
                        sy[ix+iy*N] = (uint8)b.get_clamped(x+ix, y+iy).get_luma();
                     else
                        sy[ix+iy*N] = (uint8)b.get_clamped(x+ix, y+iy)[channel_index];
                  }
               }

               double ssim = compute_block_ssim(N*N, sx, sy);
               total_ssim += ssim;
               total_blocks++;

               //uint ssim_c = (uint)math::clamp<double>(ssim * 127.0f + 128.0f, 0, 255);
               //yimg(x / N, y / N).set(ssim_c, ssim_c, ssim_c, 255);
            }
         }

         if (!total_blocks)
            return 0.0f;

         //save_to_file_stb(L"ssim.tga", yimg, cSaveGrayscale);

         return total_ssim / total_blocks;
      }

      void print_ssim(const image_u8& src_img, const image_u8& dst_img)
      {
         double y_ssim = compute_ssim(src_img, dst_img, -1);
         console::printf(L"Luma MSSIM: %f, Scaled: %f", y_ssim, (y_ssim - .8f) / .2f);

         //double r_ssim = compute_ssim(src_img, dst_img, 0);
         //console::printf(L"   R MSSIM: %f", r_ssim);

         //double g_ssim = compute_ssim(src_img, dst_img, 1);
         //console::printf(L"   G MSSIM: %f", g_ssim);

         //double b_ssim = compute_ssim(src_img, dst_img, 2);
         //console::printf(L"   B MSSIM: %f", b_ssim);
      }

      void error_metrics::print(const wchar_t* pName) const
      {
         if (mPeakSNR >= cInfinitePSNR)
            console::printf(L"%s Error: Max: %3u, Mean: %3.3f, RMS: %3.3f, PSNR: Infinite", pName, mMax, mMean, mRootMeanSquared);
         else
            console::printf(L"%s Error: Max: %3u, Mean: %3.3f, RMS: %3.3f, PSNR: %3.3f", pName, mMax, mMean, mRootMeanSquared, mPeakSNR);
      }

      bool error_metrics::compute(const image_u8& a, const image_u8& b, uint first_channel, uint num_channels, bool average_component_error)
      {
         //if ( (!a.get_width()) || (!b.get_height()) || (a.get_width() != b.get_width()) || (a.get_height() != b.get_height()) )
         //   return false;

         const uint width = math::minimum(a.get_width(), b.get_width());
         const uint height = math::minimum(a.get_height(), b.get_height());

         CRNLIB_ASSERT((first_channel < 4U) && (first_channel + num_channels <= 4U));

         // Histogram approach due to Charles Bloom.
         double hist[256];
         utils::zero_object(hist);

         for (uint y = 0; y < height; y++)
         {
            for (uint x = 0; x < width; x++)
            {
               const color_quad_u8& ca = a(x, y);
               const color_quad_u8& cb = b(x, y);

               if (!num_channels)
                  hist[labs(ca.get_luma() - cb.get_luma())]++;
               else
               {
                  for (uint c = 0; c < num_channels; c++)
                     hist[labs(ca[first_channel + c] - cb[first_channel + c])]++;
               }
            }
         }

         mMax = 0;
         double sum = 0.0f, sum2 = 0.0f;
         for (uint i = 0; i < 256; i++)
         {
            if (!hist[i])
               continue;

            mMax = math::maximum(mMax, i);

            double x = i * hist[i];

            sum += x;
            sum2 += i * x;
         }

         // See http://bmrc.berkeley.edu/courseware/cs294/fall97/assignment/psnr.html
         double total_values = width * height;

         if (average_component_error)
            total_values *= math::clamp<uint>(num_channels, 1, 4);

         mMean = math::clamp<double>(sum / total_values, 0.0f, 255.0f);
         mMeanSquared = math::clamp<double>(sum2 / total_values, 0.0f, 255.0f*255.0f);

         mRootMeanSquared = sqrt(mMeanSquared);

         if (!mRootMeanSquared)
            mPeakSNR = cInfinitePSNR;
         else
            mPeakSNR = math::clamp<double>(log10(255.0f / mRootMeanSquared) * 20.0f, 0.0f, 500.0f);

         return true;
      }

      void print_image_metrics(const image_u8& src_img, const image_u8& dst_img)
      {
         if ( (!src_img.get_width()) || (!dst_img.get_height()) || (src_img.get_width() != dst_img.get_width()) || (src_img.get_height() != dst_img.get_height()) )
            console::printf(L"print_image_metrics: Image resolutions don't match exactly (%ux%u) vs. (%ux%u)", src_img.get_width(), src_img.get_height(), dst_img.get_width(), dst_img.get_height());

         image_utils::error_metrics error_metrics;

         if (src_img.has_rgb() || dst_img.has_rgb())
         {
            error_metrics.compute(src_img, dst_img, 0, 3, false);
            error_metrics.print(L"RGB Total  ");

            error_metrics.compute(src_img, dst_img, 0, 3, true);
            error_metrics.print(L"RGB Average");

            error_metrics.compute(src_img, dst_img, 0, 0);
            error_metrics.print(L"Luma       ");

            error_metrics.compute(src_img, dst_img, 0, 1);
            error_metrics.print(L"Red        ");

            error_metrics.compute(src_img, dst_img, 1, 1);
            error_metrics.print(L"Green      ");

            error_metrics.compute(src_img, dst_img, 2, 1);
            error_metrics.print(L"Blue       ");
         }

         if (src_img.has_alpha() || dst_img.has_alpha())
         {
            error_metrics.compute(src_img, dst_img, 3, 1);
            error_metrics.print(L"Alpha      ");
         }
      }

      static uint8 regen_z(uint x, uint y)
      {
         float vx = math::clamp((x - 128.0f) * 1.0f/127.0f, -1.0f, 1.0f);
         float vy = math::clamp((y - 128.0f) * 1.0f/127.0f, -1.0f, 1.0f);
         float vz = sqrt(math::clamp(1.0f - vx * vx - vy * vy, 0.0f, 1.0f));

         vz = vz * 127.0f + 128.0f;

         if (vz < 128.0f)
            vz -= .5f;
         else
            vz += .5f;

         int ib = math::float_to_int(vz);

         return static_cast<uint8>(math::clamp(ib, 0, 255));
      }

      void convert_image(image_u8& img, image_utils::conversion_type conv_type)
      {
         switch (conv_type)
         {
            case image_utils::cConversion_To_CCxY:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(pixel_format_helpers::cCompFlagRValid | pixel_format_helpers::cCompFlagGValid | pixel_format_helpers::cCompFlagAValid | pixel_format_helpers::cCompFlagLumaChroma));
               break;
            }
            case image_utils::cConversion_From_CCxY:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(pixel_format_helpers::cCompFlagRValid | pixel_format_helpers::cCompFlagGValid | pixel_format_helpers::cCompFlagBValid));
               break;
            }
            case image_utils::cConversion_To_xGxR:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(pixel_format_helpers::cCompFlagGValid | pixel_format_helpers::cCompFlagAValid | pixel_format_helpers::cCompFlagNormalMap));
               break;
            }
            case image_utils::cConversion_From_xGxR:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(pixel_format_helpers::cCompFlagRValid | pixel_format_helpers::cCompFlagGValid | pixel_format_helpers::cCompFlagBValid | pixel_format_helpers::cCompFlagNormalMap));
               break;
            }
            case image_utils::cConversion_To_xGBR:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(pixel_format_helpers::cCompFlagGValid | pixel_format_helpers::cCompFlagBValid | pixel_format_helpers::cCompFlagAValid | pixel_format_helpers::cCompFlagNormalMap));
               break;
            }
            case image_utils::cConversion_To_AGBR:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(pixel_format_helpers::cCompFlagRValid | pixel_format_helpers::cCompFlagGValid | pixel_format_helpers::cCompFlagBValid | pixel_format_helpers::cCompFlagAValid | pixel_format_helpers::cCompFlagNormalMap));
               break;
            }
            case image_utils::cConversion_From_xGBR:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(pixel_format_helpers::cCompFlagRValid | pixel_format_helpers::cCompFlagGValid | pixel_format_helpers::cCompFlagBValid | pixel_format_helpers::cCompFlagNormalMap));
               break;
            }
            case image_utils::cConversion_From_AGBR:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(pixel_format_helpers::cCompFlagRValid | pixel_format_helpers::cCompFlagGValid | pixel_format_helpers::cCompFlagBValid | pixel_format_helpers::cCompFlagAValid | pixel_format_helpers::cCompFlagNormalMap));
               break;
            }
            case image_utils::cConversion_XY_to_XYZ:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(pixel_format_helpers::cCompFlagRValid | pixel_format_helpers::cCompFlagGValid | pixel_format_helpers::cCompFlagBValid | pixel_format_helpers::cCompFlagNormalMap));
               break;
            }
            case cConversion_Y_To_A:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(img.get_comp_flags() | pixel_format_helpers::cCompFlagAValid));
               break;
            }
            case cConversion_A_To_RGBA:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(pixel_format_helpers::cCompFlagRValid | pixel_format_helpers::cCompFlagGValid | pixel_format_helpers::cCompFlagBValid | pixel_format_helpers::cCompFlagAValid));
               break;
            }
            case cConversion_Y_To_RGB:
            {
               img.set_comp_flags(static_cast<pixel_format_helpers::component_flags>(pixel_format_helpers::cCompFlagRValid | pixel_format_helpers::cCompFlagGValid | pixel_format_helpers::cCompFlagBValid | pixel_format_helpers::cCompFlagGrayscale | (img.has_alpha() ? pixel_format_helpers::cCompFlagAValid : 0)));
               break;
            }
            default:
            {
               CRNLIB_ASSERT(false);
               return;
            }
         }

         for (uint y = 0; y < img.get_height(); y++)
         {
            for (uint x = 0; x < img.get_width(); x++)
            {
               color_quad_u8 src(img(x, y));
               color_quad_u8 dst;

               switch (conv_type)
               {
                  case image_utils::cConversion_To_CCxY:
                  {
                     color::RGB_to_YCC(dst, src);
                     break;
                  }
                  case image_utils::cConversion_From_CCxY:
                  {
                     color::YCC_to_RGB(dst, src);
                     break;
                  }
                  case image_utils::cConversion_To_xGxR:
                  {
                     dst.r = 0;
                     dst.g = src.g;
                     dst.b = 0;
                     dst.a = src.r;
                     break;
                  }
                  case image_utils::cConversion_From_xGxR:
                  {
                     dst.r = src.a;
                     dst.g = src.g;
                     // This is kinda iffy, we're assuming the image is a normal map here.
                     dst.b = regen_z(src.a, src.g);
                     dst.a = 255;
                     break;
                  }
                  case image_utils::cConversion_To_xGBR:
                  {
                     dst.r = 0;
                     dst.g = src.g;
                     dst.b = src.b;
                     dst.a = src.r;
                     break;
                  }
                  case image_utils::cConversion_To_AGBR:
                  {
                     dst.r = src.a;
                     dst.g = src.g;
                     dst.b = src.b;
                     dst.a = src.r;
                     break;
                  }
                  case image_utils::cConversion_From_xGBR:
                  {
                     dst.r = src.a;
                     dst.g = src.g;
                     dst.b = src.b;
                     dst.a = 255;
                     break;
                  }
                  case image_utils::cConversion_From_AGBR:
                  {
                     dst.r = src.a;
                     dst.g = src.g;
                     dst.b = src.b;
                     dst.a = src.r;
                     break;
                  }
                  case image_utils::cConversion_XY_to_XYZ:
                  {
                     dst.r = src.r;
                     dst.g = src.g;
                     // This is kinda iffy, we're assuming the image is a normal map here.
                     dst.b = regen_z(src.r, src.g);
                     dst.a = 255;
                     break;
                  }
                  case image_utils::cConversion_Y_To_A:
                  {
                     dst.r = src.r;
                     dst.g = src.g;
                     dst.b = src.b;
                     dst.a = static_cast<uint8>(src.get_luma());
                     break;
                  }
                  case image_utils::cConversion_Y_To_RGB:
                  {
                     uint8 y = static_cast<uint8>(src.get_luma());
                     dst.r = y;
                     dst.g = y;
                     dst.b = y;
                     dst.a = src.a;
                     break;
                  }
                  case image_utils::cConversion_A_To_RGBA:
                  {
                     dst.r = src.a;
                     dst.g = src.a;
                     dst.b = src.a;
                     dst.a = src.a;
                     break;
                  }
                  default:
                  {
                     CRNLIB_ASSERT(false);
                     dst = src;
                     break;
                  }
               }

               img(x, y) = dst;
            }
         }
      }

      image_utils::conversion_type get_conversion_type(bool cooking, pixel_format fmt)
      {
         image_utils::conversion_type conv_type = image_utils::cConversion_Invalid;

         if (cooking)
         {
            switch (fmt)
            {
               case PIXEL_FMT_DXT5_CCxY:
               {
                  conv_type = image_utils::cConversion_To_CCxY;
                  break;
               }
               case PIXEL_FMT_DXT5_xGxR:
               {
                  conv_type = image_utils::cConversion_To_xGxR;
                  break;
               }
               case PIXEL_FMT_DXT5_xGBR:
               {
                  conv_type = image_utils::cConversion_To_xGBR;
                  break;
               }
               case PIXEL_FMT_DXT5_AGBR:
               {
                  conv_type = image_utils::cConversion_To_AGBR;
                  break;
               }
               default: break;
            }
         }
         else
         {
            switch (fmt)
            {
               case PIXEL_FMT_3DC:
               case PIXEL_FMT_DXN:
               {
                  conv_type = image_utils::cConversion_XY_to_XYZ;
                  break;
               }
               case PIXEL_FMT_DXT5_CCxY:
               {
                  conv_type = image_utils::cConversion_From_CCxY;
                  break;
               }
               case PIXEL_FMT_DXT5_xGxR:
               {
                  conv_type = image_utils::cConversion_From_xGxR;
                  break;
               }
               case PIXEL_FMT_DXT5_xGBR:
               {
                  conv_type = image_utils::cConversion_From_xGBR;
                  break;
               }
               case PIXEL_FMT_DXT5_AGBR:
               {
                  conv_type = image_utils::cConversion_From_AGBR;
                  break;
               }
               default: break;
            }
         }

         return conv_type;
      }

      image_utils::conversion_type get_image_conversion_type_from_crn_format(crn_format fmt)
      {
         switch (fmt)
         {
            case cCRNFmtDXT5_CCxY: return image_utils::cConversion_To_CCxY;
            case cCRNFmtDXT5_xGxR: return image_utils::cConversion_To_xGxR;
            case cCRNFmtDXT5_xGBR: return image_utils::cConversion_To_xGBR;
            case cCRNFmtDXT5_AGBR: return image_utils::cConversion_To_AGBR;
            default: break;
         }
         return image_utils::cConversion_Invalid;
      }

      double compute_std_dev(uint n, const color_quad_u8* pPixels, uint first_channel, uint num_channels)
      {
         if (!n)
            return 0.0f;

         double sum = 0.0f;
         double sum2 = 0.0f;

         for (uint i = 0; i < n; i++)
         {
            const color_quad_u8& cp = pPixels[i];

            if (!num_channels)
            {
               uint l = cp.get_luma();
               sum += l;
               sum2 += l*l;
            }
            else
            {
               for (uint c = 0; c < num_channels; c++)
               {
                  uint l = cp[first_channel + c];
                  sum += l;
                  sum2 += l*l;
               }
            }
         }

         double w = math::maximum(1U, num_channels) * n;
         sum /= w;
         sum2 /= w;

         double var = sum2 - sum * sum;
         var = math::maximum<double>(var, 0.0f);

         return sqrt(var);
      }

   } // namespace image_utils

} // namespace crnlib
