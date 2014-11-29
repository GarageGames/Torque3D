// File: crn_texture_file_types.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once
#include "../inc/crnlib.h"
#include "crn_vec.h"
#include "crn_pixel_format.h"

namespace crnlib
{
   struct texture_file_types
   {
      enum format
      {
         cFormatInvalid = -1,

         cFormatTGA = 0,
         cFormatPNG,
         cFormatJPG,
         cFormatJPEG,
         cFormatBMP,
         cFormatGIF,
         cFormatTIF,
         cFormatTIFF,
         cFormatPPM,
         cFormatPGM,
         cFormatDDS,
         cFormatPSD,
         cFormatJP2,
         cFormatCRN,
         
         cNumRegularFileFormats,
         
         // Not really a file format 
         cFormatClipboard = cNumRegularFileFormats,
         cFormatDragDrop,

         cNumFileFormats
      };

      static const wchar_t* get_extension(format fmt);

      static format determine_file_format(const wchar_t* pFilename);
      
      static bool supports_mipmaps(format fmt);
      static bool supports_alpha(format fmt);
   };  
   
   enum texture_type
   {
      cTextureTypeUnknown = 0,
      cTextureTypeRegularMap,
      cTextureTypeNormalMap,
      cTextureTypeVerticalCrossCubemap,
      cTextureTypeCubemap,

      cNumTextureTypes
   };
   
   const wchar_t* get_texture_type_desc(texture_type t);
               
} // namespace crnlib
   
