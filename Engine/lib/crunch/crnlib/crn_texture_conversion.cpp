// File: crn_texture_conversion.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_texture_conversion.h"
#include "crn_console.h"
#include "crn_win32_file_utils.h"
#include "crn_cfile_stream.h"
#include "crn_image_utils.h"
#include "crn_texture_comp.h"
#include "crn_strutils.h"

namespace crnlib
{
   namespace texture_conversion
   {
      struct progress_params
      {
         convert_params* m_pParams;
         uint m_start_percentage;
         bool m_canceled;
      };

      convert_stats::convert_stats()
      {
         clear();
      }

      bool convert_stats::init(
         const wchar_t* pSrc_filename,
         const wchar_t* pDst_filename,
         dds_texture& src_tex,
         texture_file_types::format dst_file_type,
         bool lzma_stats)
      {
         m_src_filename = pSrc_filename;
         m_dst_filename = pDst_filename;
         m_dst_file_type = dst_file_type;

         m_pInput_tex = &src_tex;

         win32_file_utils::get_file_size(pSrc_filename, m_input_file_size);
         win32_file_utils::get_file_size(pDst_filename, m_output_file_size);

         m_total_input_pixels = 0;
         for (uint i = 0; i < src_tex.get_num_levels(); i++)
         {
            uint width = math::maximum<uint>(1, src_tex.get_width() >> i);
            uint height = math::maximum<uint>(1, src_tex.get_height() >> i);
            m_total_input_pixels += width*height*src_tex.get_num_faces();
         }

         m_output_comp_file_size = 0;

         m_total_output_pixels = 0;

         if (lzma_stats)
         {
            vector<uint8> dst_tex_bytes;
            if (!cfile_stream::read_file_into_array(pDst_filename, dst_tex_bytes))
            {
               console::error(L"Failed loading output file: %s", pDst_filename);
               return false;
            }
            if (!dst_tex_bytes.size())
            {
               console::error(L"Output file is empty: %s", pDst_filename);
               return false;
            }
            vector<uint8> cmp_tex_bytes;
            lzma_codec lossless_codec;
            if (lossless_codec.pack(dst_tex_bytes.get_ptr(), dst_tex_bytes.size(), cmp_tex_bytes))
            {
               m_output_comp_file_size = cmp_tex_bytes.size();
            }
         }

         if (!m_output_tex.load_from_file(pDst_filename, m_dst_file_type))
         {
            console::error(L"Failed loading output file: %s", pDst_filename);
            return false;
         }

         for (uint i = 0; i < m_output_tex.get_num_levels(); i++)
         {
            uint width = math::maximum<uint>(1, m_output_tex.get_width() >> i);
            uint height = math::maximum<uint>(1, m_output_tex.get_height() >> i);
            m_total_output_pixels += width*height*m_output_tex.get_num_faces();
         }
         CRNLIB_ASSERT(m_total_output_pixels == m_output_tex.get_total_pixels_in_all_faces_and_mips());

         return true;
      }

      bool convert_stats::print(bool psnr_metrics, bool mip_stats, bool grayscale_sampling, const wchar_t *pCSVStatsFile) const
      {
         if (!m_pInput_tex)
            return false;

         console::info(L"Input texture: %ux%u, Levels: %u, Faces: %u, Format: %s",
            m_pInput_tex->get_width(),
            m_pInput_tex->get_height(),
            m_pInput_tex->get_num_levels(),
            m_pInput_tex->get_num_faces(),
            pixel_format_helpers::get_pixel_format_string(m_pInput_tex->get_format()));

         // Just casting the uint64's filesizes to uint32 here to work around gcc issues - it's not even possible to have files that large anyway.
         console::info(L"Input pixels: %u, Input file size: %u, Input bits/pixel: %1.3f",
            m_total_input_pixels, (uint32)m_input_file_size, (m_input_file_size * 8.0f) / m_total_input_pixels);

         console::info(L"Output texture: %ux%u, Levels: %u, Faces: %u, Format: %s",
            m_output_tex.get_width(),
            m_output_tex.get_height(),
            m_output_tex.get_num_levels(),
            m_output_tex.get_num_faces(),
            pixel_format_helpers::get_pixel_format_string(m_output_tex.get_format()));

         console::info(L"Output pixels: %u, Output file size: %u, Output bits/pixel: %1.3f",
            m_total_output_pixels, (uint32)m_output_file_size, (m_output_file_size * 8.0f) / m_total_output_pixels);

         if (m_output_comp_file_size)
         {
            console::info(L"LZMA compressed output file size: %u bytes, %1.3f bits/pixel",
               (uint32)m_output_comp_file_size, (m_output_comp_file_size * 8.0f) / m_total_output_pixels);
         }
         if (psnr_metrics)
         {
            if ( (m_pInput_tex->get_width() != m_output_tex.get_width()) || (m_pInput_tex->get_height() != m_output_tex.get_height()) || (m_pInput_tex->get_num_faces() != m_output_tex.get_num_faces()) )
            {
               console::warning(L"Unable to compute image statistics - input/output texture dimensions are different.");
            }
            else
            {
               uint num_levels = math::minimum(m_pInput_tex->get_num_levels(), m_output_tex.get_num_levels());

               if (!mip_stats)
                  num_levels = 1;

               for (uint level = 0; level < num_levels; level++)
               {
                  image_u8 a, b;
                  image_u8* pA = m_pInput_tex->get_level_image(0, level, a);
                  image_u8* pB = m_output_tex.get_level_image(0, level, b);

                  if (pA && pB)
                  {
                     image_u8 grayscale_a, grayscale_b;
                     if (grayscale_sampling)
                     {
                        grayscale_a = *pA;
                        grayscale_a.convert_to_grayscale();
                        pA = &grayscale_a;

                        grayscale_b = *pB;
                        grayscale_b.convert_to_grayscale();
                        pB = &grayscale_b;
                     }

                     console::info(L"Mipmap level %u statistics:", level);
                     image_utils::print_image_metrics(*pA, *pB);

                     if ((pA->has_rgb()) || (pB->has_rgb()))
                        image_utils::print_ssim(*pA, *pB);
                  }
               }

               if (pCSVStatsFile)
               {
                  // FIXME: This is kind of a hack, and should be combine with the code above.
                  image_u8 a, b;
                  image_u8* pA = m_pInput_tex->get_level_image(0, 0, a);
                  image_u8* pB = m_output_tex.get_level_image(0, 0, b);
                  if (pA && pB)
                  {
                     image_u8 grayscale_a, grayscale_b;
                     if (grayscale_sampling)
                     {
                        grayscale_a = *pA;
                        grayscale_a.convert_to_grayscale();
                        pA = &grayscale_a;

                        grayscale_b = *pB;
                        grayscale_b.convert_to_grayscale();
                        pB = &grayscale_b;
                     }

                     image_utils::error_metrics rgb_error;
                     image_utils::error_metrics luma_error;
                     if (rgb_error.compute(*pA, *pB, 0, 3, false) && luma_error.compute(*pA, *pB, 0, 0, true))
                     {
                        FILE *pFile = NULL;
#ifdef _MSC_VER
                        _wfopen_s(&pFile, pCSVStatsFile, L"a");
#else
                        pFile = _wfopen(pCSVStatsFile, L"a");
#endif
                        if (!pFile)
                           console::warning(L"Unable to append to CSV stats file: %s\n", pCSVStatsFile);
                        else
                        {
                           dynamic_wstring filename;
                           split_path(m_src_filename.get_ptr(), NULL, NULL, &filename, NULL);
                           dynamic_string filenamea;
                           uint64 effective_output_size = m_output_comp_file_size ? m_output_comp_file_size : m_output_file_size;
                           float bitrate = (effective_output_size * 8.0f) / m_total_output_pixels;
                           fprintf(pFile, "%s,%u,%u,%u,%f,%f,%u,%f\n",
                              filename.as_ansi(filenamea).get_ptr(),
                              pB->get_width(), pB->get_height(), m_output_tex.get_num_levels(),
                              rgb_error.mRootMeanSquared, luma_error.mRootMeanSquared,
                              (uint32)effective_output_size, bitrate);
                           fclose(pFile);
                        }
                     }
                  }
               }
            }
         }

         return true;
      }

      void convert_stats::clear()
      {
         m_src_filename.clear();
         m_dst_filename.clear();
         m_dst_file_type = texture_file_types::cFormatInvalid;

         m_pInput_tex = NULL;
         m_output_tex.clear();

         m_input_file_size = 0;
         m_total_input_pixels = 0;

         m_output_file_size = 0;
         m_total_output_pixels = 0;

         m_output_comp_file_size = 0;
      }

      //-----------------------------------------------------------------------

      static crn_bool crn_progress_callback(crn_uint32 phase_index, crn_uint32 total_phases, crn_uint32 subphase_index, crn_uint32 total_subphases, void* pUser_data_ptr)
      {
         progress_params& params = *static_cast<progress_params*>(pUser_data_ptr);

         if (params.m_canceled)
            return false;
         if (!params.m_pParams->m_pProgress_func)
            return true;

         int percentage_complete = params.m_start_percentage + (int)(.5f + (phase_index + float(subphase_index) / total_subphases) * (100.0f - params.m_start_percentage) / total_phases);

         percentage_complete = math::clamp<int>(percentage_complete, 0, 100);

         if (!params.m_pParams->m_pProgress_func(percentage_complete, params.m_pParams->m_pProgress_user_data))
         {
            params.m_canceled = true;
            return false;
         }

         return true;
      }

      static bool dxt_progress_callback_func(uint percentage_complete, void* pUser_data_ptr)
      {
         progress_params& params = *static_cast<progress_params*>(pUser_data_ptr);

         if (params.m_canceled)
            return false;

         if (!params.m_pParams->m_pProgress_func)
            return true;

         int scaled_percentage_complete = params.m_start_percentage + (percentage_complete * (100 - params.m_start_percentage)) / 100;

         scaled_percentage_complete = math::clamp<int>(scaled_percentage_complete, 0, 100);

         if (!params.m_pParams->m_pProgress_func(scaled_percentage_complete, params.m_pParams->m_pProgress_user_data))
         {
            params.m_canceled = true;
            return false;
         }

         return true;
      }

      static bool convert_error(const convert_params& params, const wchar_t* pError_msg)
      {
         params.m_status = false;
         params.m_error_message = pError_msg;

         _wremove(params.m_dst_filename.get_ptr());

         return false;
      }

      static pixel_format choose_pixel_format(convert_params& params, const crn_comp_params &comp_params, const dds_texture& src_tex, texture_type tex_type)
      {
         const bool is_normal_map = (tex_type == cTextureTypeNormalMap);

         if (params.m_dst_file_type == texture_file_types::cFormatCRN)
         {
            if (is_normal_map)
            {
               switch (src_tex.get_format())
               {
                  case PIXEL_FMT_DXN:
                  case PIXEL_FMT_3DC:
                  case PIXEL_FMT_DXT5_xGBR:
                  case PIXEL_FMT_DXT5_AGBR:
                  case PIXEL_FMT_DXT5_xGxR:
                     return src_tex.get_format();
                  default:
                     return PIXEL_FMT_DXT5_AGBR;
               }
            }
         }
         else if (params.m_dst_file_type == texture_file_types::cFormatDDS)
         {
            if (src_tex.get_source_file_type() != texture_file_types::cFormatCRN)
            {
               if (is_normal_map)
               {
                  switch (src_tex.get_format())
                  {
                     case PIXEL_FMT_DXN:
                     case PIXEL_FMT_3DC:
                     case PIXEL_FMT_DXT5_xGBR:
                     case PIXEL_FMT_DXT5_AGBR:
                     case PIXEL_FMT_DXT5_xGxR:
                        return src_tex.get_format();
                     default:
                        return PIXEL_FMT_DXT5_AGBR;
                  }
               }
               else if (pixel_format_helpers::is_grayscale(src_tex.get_format()))
               {
                  if (pixel_format_helpers::has_alpha(src_tex.get_format()))
                     return comp_params.get_flag(cCRNCompFlagDXT1AForTransparency) ? PIXEL_FMT_DXT1A : PIXEL_FMT_DXT5;
                  else
                     return PIXEL_FMT_DXT1;
               }
               else if (pixel_format_helpers::has_alpha(src_tex.get_format()))
                  return comp_params.get_flag(cCRNCompFlagDXT1AForTransparency) ? PIXEL_FMT_DXT1A : PIXEL_FMT_DXT5;
               else
                  return PIXEL_FMT_DXT1;
            }
         }
         else
         {
            // A regular image format.
            if (pixel_format_helpers::is_grayscale(src_tex.get_format()))
            {
               if (pixel_format_helpers::has_alpha(src_tex.get_format()))
                  return PIXEL_FMT_A8L8;
               else
                  return PIXEL_FMT_L8;
            }
            else if (pixel_format_helpers::has_alpha(src_tex.get_format()))
               return PIXEL_FMT_A8R8G8B8;
            else
               return PIXEL_FMT_R8G8B8;
         }

         return src_tex.get_format();
      }

      static void print_comp_params(const crn_comp_params &comp_params)
      {
         console::debug(L"\nTexture conversion compression parameters:");
         console::debug(L"          Desired bitrate: %3.3f", comp_params.m_target_bitrate);
         console::debug(L"              CRN Quality: %i", comp_params.m_quality_level);
         console::debug(L"CRN C endpoints/selectors: %u %u", comp_params.m_crn_color_endpoint_palette_size, comp_params.m_crn_color_selector_palette_size);
         console::debug(L"CRN A endpoints/selectors: %u %u", comp_params.m_crn_alpha_endpoint_palette_size, comp_params.m_crn_alpha_selector_palette_size);
         console::debug(L"     DXT both block types: %u, Alpha threshold: %u", comp_params.get_flag(cCRNCompFlagUseBothBlockTypes), comp_params.m_dxt1a_alpha_threshold);
         console::debug(L"  DXT compression quality: %s", crn_get_dxt_quality_string(comp_params.m_dxt_quality));
         console::debug(L"               Perceptual: %u, Large Blocks: %u", comp_params.get_flag(cCRNCompFlagPerceptual), comp_params.get_flag(cCRNCompFlagHierarchical));
         console::debug(L"               Compressor: %s", get_dxt_compressor_name(comp_params.m_dxt_compressor_type));
         console::debug(L" Disable endpoint caching: %u", comp_params.get_flag(cCRNCompFlagDisableEndpointCaching));
         console::debug(L"       Grayscale sampling: %u", comp_params.get_flag(cCRNCompFlagGrayscaleSampling));
         console::debug(L"       Max helper threads: %u", comp_params.m_num_helper_threads);
         console::debug(L"");
      }

      static void print_mipmap_params(const crn_mipmap_params &mipmap_params)
      {
         console::debug(L"\nTexture conversion MIP-map parameters:");
         console::debug(L"           Mode: %s", crn_get_mip_mode_name(mipmap_params.m_mode));
         console::debug(L"         Filter: %S", crn_get_mip_filter_name(mipmap_params.m_filter));
         console::debug(L"Gamma filtering: %u, Gamma: %2.2f", mipmap_params.m_gamma_filtering, mipmap_params.m_gamma);
         console::debug(L"     Blurriness: %2.2f", mipmap_params.m_blurriness);
         console::debug(L"    Renormalize: %u", mipmap_params.m_renormalize);
         console::debug(L"          Tiled: %u", mipmap_params.m_tiled);
         console::debug(L"     Max Levels: %u", mipmap_params.m_max_levels);
         console::debug(L" Min level size: %u", mipmap_params.m_min_mip_size);
         console::debug(L"       window: %u %u %u %u", mipmap_params.m_window_left, mipmap_params.m_window_top, mipmap_params.m_window_right, mipmap_params.m_window_bottom);
         console::debug(L"   scale mode: %s", crn_get_scale_mode_desc(mipmap_params.m_scale_mode));
         console::debug(L"        scale: %f %f", mipmap_params.m_scale_x, mipmap_params.m_scale_y);
         console::debug(L"        clamp: %u %u, clamp_scale: %u", mipmap_params.m_clamp_width, mipmap_params.m_clamp_height, mipmap_params.m_clamp_scale);
         console::debug(L"");
      }

      void convert_params::print()
      {
         console::debug(L"\nTexture conversion parameters:");
         console::debug(L"   Resolution: %ux%u, Faces: %u, Levels: %u, Format: %s",
            m_pInput_texture->get_width(),
            m_pInput_texture->get_height(),
            m_pInput_texture->get_num_faces(),
            m_pInput_texture->get_num_levels(),
            pixel_format_helpers::get_pixel_format_string(m_pInput_texture->get_format()));

         console::debug(L" texture_type: %s", get_texture_type_desc(m_texture_type));
         console::debug(L" dst_filename: %s", m_dst_filename.get_ptr());
         console::debug(L"dst_file_type: %s", texture_file_types::get_extension(m_dst_file_type));
         console::debug(L"   dst_format: %s", pixel_format_helpers::get_pixel_format_string(m_dst_format));
         console::debug(L"        quick: %u", m_quick);
      }

      static bool write_compressed_texture(
         dds_texture& work_tex, convert_params& params, crn_comp_params &comp_params, pixel_format dst_format, progress_params& progress_state, bool perceptual, convert_stats &stats)
      {
         comp_params.m_file_type = (params.m_dst_file_type == texture_file_types::cFormatCRN) ? cCRNFileTypeCRN : cCRNFileTypeDDS;

         comp_params.m_pProgress_func = crn_progress_callback;
         comp_params.m_pProgress_func_data = &progress_state;
         comp_params.set_flag(cCRNCompFlagPerceptual, perceptual);

         crn_format crn_fmt = pixel_format_helpers::convert_pixel_format_to_best_crn_format(dst_format);
         comp_params.m_format = crn_fmt;

         console::message(L"Writing %s texture to file: \"%s\"", crn_get_format_string(crn_fmt), params.m_dst_filename.get_ptr());

         uint32 actual_quality_level;
         float actual_bitrate;
         bool status = work_tex.write_to_file(params.m_dst_filename.get_ptr(), params.m_dst_file_type, &comp_params, &actual_quality_level, &actual_bitrate);
         if (!status)
            return convert_error(params, L"Failed writing output file!");

         if (!params.m_no_stats)
         {
            if (!stats.init(params.m_pInput_texture->get_source_filename().get_ptr(), params.m_dst_filename.get_ptr(), *params.m_pIntermediate_texture, params.m_dst_file_type, params.m_lzma_stats))
            {
               console::warning(L"Unable to compute output statistics for file: %s", params.m_pInput_texture->get_source_filename().get_ptr());
            }
         }

         return true;
      }

      static bool convert_and_write_normal_texture(dds_texture& work_tex, convert_params& params, const crn_comp_params &comp_params, pixel_format dst_format, progress_params& progress_state, bool formats_differ, bool perceptual, convert_stats& stats)
      {
         if (formats_differ)
         {
            dxt_image::pack_params pack_params;

            pack_params.m_perceptual = perceptual;
            pack_params.m_compressor = comp_params.m_dxt_compressor_type;
            pack_params.m_pProgress_callback = dxt_progress_callback_func;
            pack_params.m_pProgress_callback_user_data_ptr = &progress_state;
            pack_params.m_dxt1a_alpha_threshold = comp_params.m_dxt1a_alpha_threshold;
            pack_params.m_quality = comp_params.m_dxt_quality;
            pack_params.m_endpoint_caching = !comp_params.get_flag(cCRNCompFlagDisableEndpointCaching);
            pack_params.m_grayscale_sampling = comp_params.get_flag(cCRNCompFlagGrayscaleSampling);
            if ((!comp_params.get_flag(cCRNCompFlagUseBothBlockTypes)) && (!comp_params.get_flag(cCRNCompFlagDXT1AForTransparency)))
               pack_params.m_use_both_block_types = false;

            pack_params.m_num_helper_threads = comp_params.m_num_helper_threads;
            pack_params.m_use_transparent_indices_for_black = comp_params.get_flag(cCRNCompFlagUseTransparentIndicesForBlack);

            console::info(L"Converting texture format from %s to %s", pixel_format_helpers::get_pixel_format_string(work_tex.get_format()), pixel_format_helpers::get_pixel_format_string(dst_format));

            timer tm;
            tm.start();

            bool status = work_tex.convert(dst_format, pack_params);

            double t = tm.get_elapsed_secs();

            console::info(L"");

            if (!status)
            {
               if (progress_state.m_canceled)
               {
                  params.m_canceled = true;
                  return false;
               }
               else
               {
                  return convert_error(params, L"Failed converting texture to output format!");
               }
            }

            console::info(L"Texture format conversion took %3.3fs", t);
         }

         if (params.m_write_mipmaps_to_multiple_files)
         {
            for (uint f = 0; f < work_tex.get_num_faces(); f++)
            {
               for (uint l = 0; l < work_tex.get_num_levels(); l++)
               {
                  dynamic_wstring filename(params.m_dst_filename.get_ptr());

                  dynamic_wstring drv, dir, fn, ext;
                  if (!split_path(params.m_dst_filename.get_ptr(), &drv, &dir, &fn, &ext))
                     return false;

                  fn += dynamic_wstring(cVarArg, L"_face%u_mip%u", f, l).get_ptr();
                  filename = drv + dir + fn + ext;

                  mip_level *pLevel = work_tex.get_level(f, l);

                  face_vec face(1);
                  face[0].push_back(crnlib_new<mip_level>(*pLevel));

                  dds_texture new_tex;
                  new_tex.assign(face);

                  console::info(L"Writing texture face %u mip level %u to file %s", f, l, filename.get_ptr());

                  if (!new_tex.write_to_file(filename.get_ptr(), params.m_dst_file_type, NULL, NULL, NULL))
                     return convert_error(params, L"Failed writing output file!");
               }
            }
         }
         else
         {
            console::message(L"Writing texture to file: \"%s\"", params.m_dst_filename.get_ptr());

            if (!work_tex.write_to_file(params.m_dst_filename.get_ptr(), params.m_dst_file_type, NULL, NULL, NULL))
               return convert_error(params, L"Failed writing output file!");

            if (!params.m_no_stats)
            {
               if (!stats.init(params.m_pInput_texture->get_source_filename().get_ptr(), params.m_dst_filename.get_ptr(), *params.m_pIntermediate_texture, params.m_dst_file_type, params.m_lzma_stats))
               {
                  console::warning(L"Unable to compute output statistics for file: %s", params.m_pInput_texture->get_source_filename().get_ptr());
               }
            }
         }

         return true;
      }

      bool process(convert_params& params, convert_stats& stats)
      {
         texture_type tex_type = params.m_texture_type;

         crn_comp_params comp_params(params.m_comp_params);
         crn_mipmap_params mipmap_params(params.m_mipmap_params);

         progress_params progress_state;
         progress_state.m_pParams = &params;
         progress_state.m_canceled = false;
         progress_state.m_start_percentage = 0;

         params.m_status = false;
         params.m_error_message.clear();

         if (params.m_pIntermediate_texture)
         {
            crnlib_delete(params.m_pIntermediate_texture);
            params.m_pIntermediate_texture = NULL;
         }

         params.m_pIntermediate_texture = crnlib_new<dds_texture>(*params.m_pInput_texture);

         dds_texture& work_tex = *params.m_pInput_texture;

         if ((params.m_dst_format != PIXEL_FMT_INVALID) && (pixel_format_helpers::is_alpha_only(params.m_dst_format)))
         {
            if ((work_tex.get_comp_flags() & pixel_format_helpers::cCompFlagAValid) == 0)
            {
               console::warning(L"Output format is alpha-only, but input doesn't have alpha, so setting alpha to luminance.");

               work_tex.convert(PIXEL_FMT_A8, crnlib::dxt_image::pack_params());

               if (tex_type == cTextureTypeNormalMap)
                  tex_type = cTextureTypeRegularMap;
            }
         }

         pixel_format dst_format = params.m_dst_format;

         if (dst_format == PIXEL_FMT_INVALID)
         {
            // Caller didn't specify a format to use, so try to pick something reasonable.
            // This is actually much trickier than it seems, and the current approach kind of sucks.
            dst_format = choose_pixel_format(params, comp_params, work_tex, tex_type);
         }

         if ((dst_format == PIXEL_FMT_DXT1) && (comp_params.get_flag(cCRNCompFlagDXT1AForTransparency)))
            dst_format = PIXEL_FMT_DXT1A;
         else if (dst_format == PIXEL_FMT_DXT1A)
            comp_params.set_flag(cCRNCompFlagDXT1AForTransparency, true);

         const bool is_normal_map = (tex_type == cTextureTypeNormalMap);
         bool perceptual = comp_params.get_flag(cCRNCompFlagPerceptual);
         if (is_normal_map)
         {
            perceptual = false;
            mipmap_params.m_gamma_filtering = false;
         }

         if (pixel_format_helpers::is_pixel_format_non_srgb(dst_format))
         {
            if (perceptual)
            {
               //console::warning(L"Output pixel format is swizzled or not RGB, disabling perceptual color metrics");
               perceptual = false;
            }
         }

         if (pixel_format_helpers::is_normal_map(dst_format))
         {
            //if (perceptual)
               //console::warning(L"Output pixel format is intended for normal maps, disabling perceptual color metrics");

            perceptual = false;
         }

         bool generate_mipmaps = texture_file_types::supports_mipmaps(params.m_dst_file_type);
         if ((params.m_write_mipmaps_to_multiple_files) && ((params.m_dst_file_type != texture_file_types::cFormatCRN) && (params.m_dst_file_type != texture_file_types::cFormatDDS)))
         {
            generate_mipmaps = true;
         }

         if (params.m_param_debugging)
         {
            params.print();

            print_comp_params(comp_params);
            print_mipmap_params(mipmap_params);
         }

         if (!create_texture_mipmaps(work_tex, comp_params, mipmap_params, generate_mipmaps))
            return convert_error(params, L"Failed creating texture mipmaps!");

         bool formats_differ = work_tex.get_format() != dst_format;
         if (formats_differ)
         {
            if (pixel_format_helpers::is_dxt1(work_tex.get_format()) && pixel_format_helpers::is_dxt1(dst_format))
               formats_differ = false;
         }

         bool status = false;

         timer t;
         t.start();

         if ( (params.m_dst_file_type == texture_file_types::cFormatCRN) ||
               ( (params.m_dst_file_type == texture_file_types::cFormatDDS) && (pixel_format_helpers::is_dxt(dst_format)) &&
                 ((formats_differ) || (comp_params.m_target_bitrate > 0.0f) || (comp_params.m_quality_level < cCRNMaxQualityLevel))
               )
            )
         {
            status = write_compressed_texture(work_tex, params, comp_params, dst_format, progress_state, perceptual, stats);
         }
         else
         {
            status = convert_and_write_normal_texture(work_tex, params, comp_params, dst_format, progress_state, formats_differ, perceptual, stats);
         }

         console::progress(L"");

         if (progress_state.m_canceled)
         {
            params.m_canceled = true;
            return false;
         }

         double total_write_time = t.get_elapsed_secs();

         if (status)
         {
            if (params.m_param_debugging)
               console::info(L"Work texture format: %s, desired destination format: %s", pixel_format_helpers::get_pixel_format_string(work_tex.get_format()), pixel_format_helpers::get_pixel_format_string(dst_format));

            console::message(L"Texture successfully written in %3.3fs", total_write_time);
         }
         else
         {
            dynamic_wstring str;

            if (work_tex.get_last_error().is_empty())
               str.format(L"Failed writing texture to file \"%s\"", params.m_dst_filename.get_ptr());
            else
               str.format(L"Failed writing texture to file \"%s\", Reason: %s", params.m_dst_filename.get_ptr(), work_tex.get_last_error().get_ptr());

            return convert_error(params, str.get_ptr());
         }

         if (params.m_debugging)
         {
            crnlib_print_mem_stats();
         }

         params.m_status = true;
         return true;
      }

   } // namespace texture_conversion

} // namespace crnlib
