// File: crn_texture_file_types.cpp
// See Copyright Notice and license at the end of inc/crnlib.h
#include "crn_core.h"
#include "crn_texture_file_types.h"
#include "crn_strutils.h"

namespace crnlib
{
   const wchar_t* texture_file_types::get_extension(format fmt)
   {
      CRNLIB_ASSERT(fmt < cNumFileFormats);
      if (fmt >= cNumFileFormats)
         return NULL;

      static const wchar_t* extensions[cNumFileFormats] =
      {
         L"tga",
         L"png",
         L"jpg",
         L"jpeg",
         L"bmp",
         L"gif",
         L"tif",
         L"tiff",
         L"ppm",
         L"pgm",
         L"dds",
         L"psd",
         L"jp2",
         L"crn",
         L"<clipboard>",
         L"<dragdrop>"
      };
      return extensions[fmt];
   }

   texture_file_types::format texture_file_types::determine_file_format(const wchar_t* pFilename)
   {
      dynamic_wstring ext;
      if (!split_path(pFilename, NULL, NULL, NULL, &ext))
         return cFormatInvalid;

      if (ext.is_empty())
         return cFormatInvalid;

      if (ext[0] == L'.')
         ext.right(1);

      for (uint i = 0; i < cNumFileFormats; i++)
         if (ext == get_extension(static_cast<format>(i)))
            return static_cast<format>(i);

      return cFormatInvalid;
   }

   bool texture_file_types::supports_mipmaps(format fmt)
   {
      switch (fmt)
      {
         case cFormatCRN:
         case cFormatDDS:
            return true;
         default: break;
      }

      return false;
   }

   bool texture_file_types::supports_alpha(format fmt)
   {
      switch (fmt)
      {
         case cFormatJPG:
         case cFormatJPEG:
         case cFormatGIF:
         case cFormatJP2:
            return false;
         default: break;
      }

      return true;
   }

   const wchar_t* get_texture_type_desc(texture_type t)
   {
      switch (t)
      {
         case cTextureTypeUnknown:                 return L"Unknown";
         case cTextureTypeRegularMap:              return L"2D map";
         case cTextureTypeNormalMap:               return L"Normal map";
         case cTextureTypeVerticalCrossCubemap:    return L"Vertical Cross Cubemap";
         case cTextureTypeCubemap:                 return L"Cubemap";
         default: break;
      }

      CRNLIB_ASSERT(false);

      return L"?";
   }

} // namespace crnlib
