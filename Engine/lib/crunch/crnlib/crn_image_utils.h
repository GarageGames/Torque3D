// File: crn_image_utils.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once
#include "crn_image.h"

namespace crnlib
{
   enum pixel_format;
   
   namespace image_utils
   {
      bool load_from_file_stb(const wchar_t* pFilename, image_u8& img);
      
      enum 
      {
         cSaveIgnoreAlpha  = 1,
         cSaveGrayscale    = 2
      };

      const int cSaveLuma = -1;

      bool save_to_file_stb(const wchar_t* pFilename, const image_u8& img, uint save_flags = 0, int comp_index = cSaveLuma);
      
      bool load_from_file(image_u8& dest, const wchar_t* pFilename, int flags = 0);

      bool save_to_grayscale_file(const wchar_t* pFilename, const image_u8& src, int component, int flags = 0);

      bool save_to_file(const wchar_t* pFilename, const image_u8& src, int flags = 0, bool ignore_alpha = false);
   
      bool has_alpha(const image_u8& img);
      bool is_normal_map(const image_u8& img, const wchar_t* pFilename = NULL);
      void renorm_normal_map(image_u8& img);
      
      struct resample_params
      {
         resample_params() :
            m_dst_width(0),
            m_dst_height(0),
            m_pFilter("lanczos4"),
            m_filter_scale(1.0f),
            m_srgb(true),
            m_wrapping(false),
            m_first_comp(0),
            m_num_comps(4),
            m_source_gamma(2.2f), // 1.75f
            m_multithreaded(true)
         {
         }
         
         uint        m_dst_width;
         uint        m_dst_height;
         const char* m_pFilter;
         float       m_filter_scale;
         bool        m_srgb;
         bool        m_wrapping;
         uint        m_first_comp;
         uint        m_num_comps;
         float       m_source_gamma;
         bool        m_multithreaded;
      };
      
      bool resample_single_thread(const image_u8& src, image_u8& dst, const resample_params& params);
      bool resample_multithreaded(const image_u8& src, image_u8& dst, const resample_params& params);
      bool resample(const image_u8& src, image_u8& dst, const resample_params& params);
   
      bool compute_delta(image_u8& dest, image_u8& a, image_u8& b, uint scale = 2);
      
      class error_metrics
      {
      public:
         error_metrics() { utils::zero_this(this); }
         
         void print(const wchar_t* pName) const;
         
         // If num_channels==0, luma error is computed.
         // If pHist != NULL, it must point to a 256 entry array.
         bool compute(const image_u8& a, const image_u8& b, uint first_channel, uint num_channels, bool average_component_error = true);
                           
         uint  mMax;
         double mMean;
         double mMeanSquared;
         double mRootMeanSquared;
         double mPeakSNR;
         
         inline bool operator== (const error_metrics& other) const
         {
            return mPeakSNR == other.mPeakSNR;
         }
         
         inline bool operator< (const error_metrics& other) const
         {
            return mPeakSNR < other.mPeakSNR;
         }
         
         inline bool operator> (const error_metrics& other) const
         {
            return mPeakSNR > other.mPeakSNR;
         }
      };

      void print_image_metrics(const image_u8& src_img, const image_u8& dst_img);
      
      double compute_block_ssim(uint n, const uint8* pX, const uint8* pY);
      double compute_ssim(const image_u8& a, const image_u8& b, int channel_index);
      void print_ssim(const image_u8& src_img, const image_u8& dst_img);
            
      enum conversion_type
      {
         cConversion_Invalid = -1,
         
         cConversion_To_CCxY,
         cConversion_From_CCxY,
         
         cConversion_To_xGxR,
         cConversion_From_xGxR,
         
         cConversion_To_xGBR,
         cConversion_From_xGBR,
         
         cConversion_To_AGBR,
         cConversion_From_AGBR,
         
         cConversion_XY_to_XYZ,
         
         cConversion_Y_To_A,

         cConversion_A_To_RGBA,
         cConversion_Y_To_RGB,
         
         cConversionTotal
      };
      
      void convert_image(image_u8& img, conversion_type conv_type);
      
      image_utils::conversion_type get_conversion_type(bool cooking, pixel_format fmt);

      image_utils::conversion_type get_image_conversion_type_from_crn_format(crn_format fmt);
            
      double compute_std_dev(uint n, const color_quad_u8* pPixels, uint first_channel, uint num_channels);
   }
}
