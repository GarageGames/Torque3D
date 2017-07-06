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

#include "platform/platform.h"
#include "core/color.h"

const LinearColorF LinearColorF::ZERO( 0, 0, 0, 0 );
const LinearColorF LinearColorF::ONE( 1, 1, 1, 1 );
const LinearColorF LinearColorF::WHITE( 1, 1, 1 );
const LinearColorF LinearColorF::BLACK( 0, 0, 0 );
const LinearColorF LinearColorF::RED( 1, 0, 0 );
const LinearColorF LinearColorF::GREEN( 0, 1, 0 );
const LinearColorF LinearColorF::BLUE( 0, 0, 1 );

const ColorI ColorI::ZERO( 0, 0, 0, 0 );
const ColorI ColorI::ONE( 255, 255, 255, 255 );
const ColorI ColorI::WHITE( 255, 255, 255 );
const ColorI ColorI::BLACK( 0, 0, 0 );
const ColorI ColorI::RED( 255, 0, 0 );
const ColorI ColorI::GREEN( 0, 255, 0 );
const ColorI ColorI::BLUE( 0, 0, 255 );

#include "console/console.h"
#include "console/consoleTypes.h"

#ifndef _STRINGUNIT_H_
#include "core/strings/stringUnit.h"
#endif

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif

#include "console/consoleInternal.h"

//-----------------------------------------------------------------------------

typedef HashTable<StringTableEntry, LinearColorF> typeNameToColorFHash;
typedef HashTable<StringTableEntry, ColorI> typeNameToColorIHash;
typedef HashTable<LinearColorF, StringTableEntry> typeColorFToNameHash;
typedef HashTable<ColorI, StringTableEntry> typeColorIToNameHash;

static typeNameToColorFHash    mNameToColorF;
static typeNameToColorIHash    mNameToColorI;
static typeColorFToNameHash    mColorFToName;
static typeColorIToNameHash    mColorIToName;

#define DEFAULT_UNKNOWN_STOCK_COLOR_NAME    "White"

MODULE_BEGIN( StockColors )

   MODULE_INIT_AFTER( GFX )

   MODULE_INIT
   {
      // Create the stock colors.
      StockColor::create();
   }

   MODULE_SHUTDOWN
   {
      // Destroy the stock colors.
      StockColor::destroy();
   }

MODULE_END;

//-----------------------------------------------------------------------------

StockColorItem StockColorTable[] =
{
   StockColorItem( "InvisibleBlack", 0, 0, 0, 0 ),
   StockColorItem( "TransparentWhite", 255, 255, 255, 0 ),
   StockColorItem( "AliceBlue", 240, 248, 255 ),
   StockColorItem( "AntiqueWhite", 250, 235, 215 ),
   StockColorItem( "Aqua", 0, 255, 255 ),
   StockColorItem( "Aquamarine", 127, 255, 212 ),
   StockColorItem( "Azure", 240, 255, 255 ),
   StockColorItem( "Beige", 245, 245, 220 ),
   StockColorItem( "Bisque", 255, 228, 196 ),
   StockColorItem( "Black", 0, 0, 0, 255 ),
   StockColorItem( "BlanchedAlmond", 255, 235, 205, 255 ),
   StockColorItem( "Blue", 0, 0, 255 ),
   StockColorItem( "BlueViolet", 138, 43, 226 ),
   StockColorItem( "Brown", 165, 42, 42, 255 ),
   StockColorItem( "BurlyWood", 222, 184, 135 ),
   StockColorItem( "CadetBlue", 95, 158, 160 ),
   StockColorItem( "Chartreuse", 127, 255, 0 ),
   StockColorItem( "Chocolate", 210, 105, 30 ),
   StockColorItem( "Coral", 255, 127, 80 ),
   StockColorItem( "CornflowerBlue", 100, 149, 237 ),
   StockColorItem( "Cornsilk", 255, 248, 220 ),
   StockColorItem( "Crimson", 220, 20, 60 ),
   StockColorItem( "Cyan", 0, 255, 255 ),
   StockColorItem( "DarkBlue", 0, 0, 139 ),
   StockColorItem( "DarkCyan", 0, 139, 139 ),
   StockColorItem( "DarkGoldenrod", 184, 134, 11 ),
   StockColorItem( "DarkGray", 169, 169, 169),
   StockColorItem( "DarkGreen", 0, 100, 0 ),
   StockColorItem( "DarkKhaki", 189, 183, 107 ),
   StockColorItem( "DarkMagenta", 139, 0, 139 ),
   StockColorItem( "DarkOliveGreen", 85, 107, 47 ),
   StockColorItem( "DarkOrange", 255, 140, 0 ),
   StockColorItem( "DarkOrchid", 153, 50, 204 ),
   StockColorItem( "DarkRed", 139, 0, 0 ),
   StockColorItem( "DarkSalmon", 233, 150, 122 ),
   StockColorItem( "DarkSeaGreen", 143, 188, 139 ),
   StockColorItem( "DarkSlateBlue", 72, 61, 139 ),
   StockColorItem( "DarkSlateGray", 47, 79, 79 ),
   StockColorItem( "DarkTurquoise", 0, 206, 209 ),
   StockColorItem( "DarkViolet", 148, 0, 211 ),
   StockColorItem( "DeepPink", 255, 20, 147 ),
   StockColorItem( "DeepSkyBlue", 0, 191, 255 ),
   StockColorItem( "DimGray", 105, 105, 105 ),
   StockColorItem( "DodgerBlue", 30, 144, 255 ),
   StockColorItem( "Firebrick", 178, 34, 34 ),
   StockColorItem( "FloralWhite", 255, 250, 240 ),
   StockColorItem( "ForestGreen", 34, 139, 34 ),
   StockColorItem( "Fuchsia", 255, 0, 255 ),
   StockColorItem( "Gainsboro", 220, 220, 220 ),
   StockColorItem( "GhostWhite", 248, 248, 255 ),
   StockColorItem( "Gold", 255, 215, 0 ),
   StockColorItem( "Goldenrod", 218, 165, 32 ),
   StockColorItem( "Gray", 128, 128, 128 ),
   StockColorItem( "Green", 0, 128, 0 ),
   StockColorItem( "GreenYellow", 173, 255, 47 ),
   StockColorItem( "Honeydew", 240, 255, 24 ),
   StockColorItem( "HotPink", 255, 105, 180 ),
   StockColorItem( "IndianRed", 205, 92, 92 ),
   StockColorItem( "Indigo", 75, 0, 130 ),
   StockColorItem( "Ivory", 255, 255, 240 ),
   StockColorItem( "Khaki", 240, 230, 140 ),
   StockColorItem( "Lavender", 230, 230, 250 ),
   StockColorItem( "LavenderBlush", 255, 240, 245 ),
   StockColorItem( "LawnGreen", 124, 252, 0 ),
   StockColorItem( "LemonChiffon", 255, 250, 205 ),
   StockColorItem( "LightBlue", 173, 216, 230 ),
   StockColorItem( "LightCoral", 240, 128, 128 ),
   StockColorItem( "LightCyan", 224, 255, 255),
   StockColorItem( "LightGoldenrodYellow", 250, 250, 210 ),
   StockColorItem( "LightGray", 211, 211, 211),
   StockColorItem( "LightGreen", 144, 238, 144 ),
   StockColorItem( "LightPink", 255, 182, 193 ),
   StockColorItem( "LightSalmon", 255, 160, 122 ),
   StockColorItem( "LightSeaGreen", 32, 178, 170 ),
   StockColorItem( "LightSkyBlue",135, 206, 250 ),
   StockColorItem( "LightSlateGray", 119, 136, 153 ),
   StockColorItem( "LightSteelBlue", 176, 196, 222 ),
   StockColorItem( "LightYellow", 255, 255, 224 ),
   StockColorItem( "Lime", 0, 255, 0 ),
   StockColorItem( "LimeGreen", 50, 205, 50 ),
   StockColorItem( "Linen", 250, 240, 230 ),
   StockColorItem( "Magenta", 255, 0, 255 ),
   StockColorItem( "Maroon", 128, 0, 0 ),
   StockColorItem( "MediumAquamarine", 102, 205, 170 ),
   StockColorItem( "MediumBlue", 0, 0, 205 ),
   StockColorItem( "MediumOrchid", 186, 85, 211 ),
   StockColorItem( "MediumPurple", 147, 112, 219 ),
   StockColorItem( "MediumSeaGreen", 60, 179, 113 ),
   StockColorItem( "MediumSlateBlue", 123, 104, 238 ),
   StockColorItem( "MediumSpringGreen", 0, 250, 154 ),
   StockColorItem( "MediumTurquoise", 72, 209, 204 ),
   StockColorItem( "MediumVioletRed", 199, 21, 133 ),
   StockColorItem( "MidnightBlue", 25, 25, 112 ),
   StockColorItem( "MintCream", 245, 255, 250 ),
   StockColorItem( "MistyRose", 255, 228, 225 ),
   StockColorItem( "Moccasin", 255, 228, 181 ),
   StockColorItem( "NavajoWhite", 255, 222, 173 ),
   StockColorItem( "Navy", 0, 0, 128 ),
   StockColorItem( "OldLace", 253, 245, 230 ),
   StockColorItem( "Olive", 128, 128, 0 ),
   StockColorItem( "OliveDrab", 107, 142, 35 ),
   StockColorItem( "Orange", 255, 165, 0 ),
   StockColorItem( "OrangeRed", 255, 69, 0 ),
   StockColorItem( "Orchid", 218, 112, 214 ),
   StockColorItem( "PaleGoldenrod", 238, 232, 170 ),
   StockColorItem( "PaleGreen", 152, 251, 152 ),
   StockColorItem( "PaleTurquoise", 175, 238, 238 ),
   StockColorItem( "PaleVioletRed", 219, 112, 147 ),
   StockColorItem( "PapayaWhip", 255, 239, 213 ),
   StockColorItem( "PeachPuff", 255, 218, 185 ),
   StockColorItem( "Peru", 205, 133, 63 ),
   StockColorItem( "Pink", 55, 192, 203 ),
   StockColorItem( "Plum", 221, 160, 221 ),
   StockColorItem( "PowderBlue", 176, 224, 230 ),
   StockColorItem( "Purple", 128, 0, 128 ),
   StockColorItem( "Red", 255, 0, 0 ),
   StockColorItem( "RosyBrown", 188, 143, 143 ),
   StockColorItem( "RoyalBlue", 65, 105, 225 ),
   StockColorItem( "SaddleBrown", 139, 69, 19 ),
   StockColorItem( "Salmon", 250, 128, 114 ),
   StockColorItem( "SandyBrown", 244, 164, 96 ),
   StockColorItem( "SeaGreen", 46, 139, 87 ),
   StockColorItem( "SeaShell", 255, 245, 238 ),
   StockColorItem( "Sienna", 160, 82, 45 ),
   StockColorItem( "Silver", 192, 192, 192 ),
   StockColorItem( "SkyBlue", 135, 206, 235 ),
   StockColorItem( "SlateBlue", 106, 90, 205 ),
   StockColorItem( "SlateGray", 112, 128, 144 ),
   StockColorItem( "Snow", 255, 250, 250 ),
   StockColorItem( "SpringGreen", 0, 255, 127 ),
   StockColorItem( "SteelBlue", 70, 130, 180 ),
   StockColorItem( "Tan", 210, 180, 140 ),
   StockColorItem( "Teal", 0, 128, 128 ),
   StockColorItem( "Thistle", 216, 191, 216 ),
   StockColorItem( "Tomato", 255, 99, 71 ),
   StockColorItem( "Turquoise", 64, 224, 208 ),
   StockColorItem( "Violet", 238, 130, 238 ),
   StockColorItem( "Wheat", 245, 222, 179 ),
   StockColorItem( "White", 255, 255, 255 ),
   StockColorItem( "WhiteSmoke", 245, 245, 245 ),
   StockColorItem( "Yellow", 255, 255, 0 ),
   StockColorItem( "YellowGreen", 154, 205, 50 )
};

//-----------------------------------------------------------------------------

static bool stockColorsCreated = false;

void StockColor::create( void )
{
   // Finish if already created.
   if ( stockColorsCreated )
      return;

   // Fetch stock color count.
   const S32 stockColorCount = sizeof(StockColorTable) / sizeof(StockColorItem);

   // Insert all stock colors.
   for( S32 index = 0; index < stockColorCount; ++index )
   {
      // Fetch stock color item.
      StockColorItem& stockColor = StockColorTable[index];

      // Fetch stock color item.
      StringTableEntry colorName = StringTable->insert( stockColor.mColorName );

      // Insert stock color mappings.
      mNameToColorF.insertUnique(colorName, stockColor.mColorF);
      mNameToColorI.insertUnique(colorName, stockColor.mColorI);
      mColorFToName.insertUnique(stockColor.mColorF, colorName);
      mColorIToName.insertUnique(stockColor.mColorI, colorName);
   }

   // Flag as created.
   stockColorsCreated = true;
}

//-----------------------------------------------------------------------------

void StockColor::destroy( void )
{
   // Finish if not created.
   if ( !stockColorsCreated )
      return;

   // Clear stock color mappings.
   mNameToColorF.clear();
   mNameToColorI.clear();
   mColorFToName.clear();
   mColorIToName.clear();

   // Flag as not created.
   stockColorsCreated = false;
}

//-----------------------------------------------------------------------------

bool StockColor::isColor( const char* pStockColorName )
{
   // Sanity!
   AssertFatal( pStockColorName != NULL, "Cannot fetch a NULL stock color name." );

   // Fetch color name.
   StringTableEntry colorName = StringTable->insert( pStockColorName );

   // Find if color name exists or not.
   return mNameToColorF.find( colorName ) != mNameToColorF.end();
}

//-----------------------------------------------------------------------------

const LinearColorF& StockColor::colorF( const char* pStockColorName )
{
   // Sanity!
   AssertFatal( pStockColorName != NULL, "Cannot fetch a NULL stock color name." );

   // Fetch color name.
   StringTableEntry colorName = StringTable->insert( pStockColorName );

   // Find stock color.
   typeNameToColorFHash::Iterator colorItr = mNameToColorF.find( colorName );

   // Return color if found.
   if ( colorItr != mNameToColorF.end() )
      return colorItr->value;

   // Warn.
   Con::warnf( "Could not find stock color name '%s'.", pStockColorName );

   // Return default stock color.
   return mNameToColorF.find( DEFAULT_UNKNOWN_STOCK_COLOR_NAME )->value;          
}

//-----------------------------------------------------------------------------

const ColorI& StockColor::colorI( const char* pStockColorName )
{
   // Sanity!
   AssertFatal( pStockColorName != NULL, "Cannot fetch a NULL stock color name." );

   // Fetch color name.
   StringTableEntry colorName = StringTable->insert( pStockColorName );

   // Find stock color.
   typeNameToColorIHash::Iterator colorItr = mNameToColorI.find( colorName );

   // Return color if found.
   if ( colorItr != mNameToColorI.end() )
      return colorItr->value;

   // Warn.
   Con::warnf( "Could not find stock color name '%s'.", colorName );

   // Return default stock color.
   return mNameToColorI.find( DEFAULT_UNKNOWN_STOCK_COLOR_NAME )->value; 
}

//-----------------------------------------------------------------------------

StringTableEntry StockColor::name( const LinearColorF& color )
{
   // Find stock color name.
   typeColorFToNameHash::Iterator colorNameItr = mColorFToName.find( color );

   // Return name if found.
   if ( colorNameItr != mColorFToName.end() )
      return colorNameItr->value;

   // Return empty string.
   return StringTable->EmptyString();
}

//-----------------------------------------------------------------------------

StringTableEntry StockColor::name( const ColorI& color )
{
   // Find stock color name.
   typeColorIToNameHash::Iterator colorNameItr = mColorIToName.find( color );

   // Return name if found.
   if ( colorNameItr != mColorIToName.end() )
      return colorNameItr->value;

   // Return empty string.
   return StringTable->EmptyString();
}

//-----------------------------------------------------------------------------

S32 StockColor::getCount( void )
{
   return sizeof(StockColorTable) / sizeof(StockColorItem);
}

//-----------------------------------------------------------------------------

const StockColorItem* StockColor::getColorItem( const S32 index )
{
   // Fetch stock color count.
   const S32 stockColorCount = StockColor::getCount();

   // Is the stock color index in range?
   if ( index < 0 || index >= stockColorCount )
   {
      // No, so warn.
      Con::warnf("StockColor::getName() - Specified color index '%d' is out of range.  Range is 0 to %d.", index, stockColorCount-1 );
      return NULL;
   }

   // Return color name.
   return &(StockColorTable[index]);
}

//-----------------------------------------------------------------------------

LinearColorF::LinearColorF( const char* pStockColorName )
{
   // Set stock color.
   *this = StockColor::colorF( pStockColorName );
}

//-----------------------------------------------------------------------------

void LinearColorF::set( const char* pStockColorName )
{
   // Set stock color.
   *this = StockColor::colorF( pStockColorName );
}

//-----------------------------------------------------------------------------

const LinearColorF& LinearColorF::StockColor( const char* pStockColorName )
{
   return StockColor::colorF( pStockColorName );
}

//-----------------------------------------------------------------------------

StringTableEntry LinearColorF::StockColor( void )
{
   // Return stock color name.
   return StockColor::name( *this );
}

//-----------------------------------------------------------------------------

ColorI::ColorI( const char* pStockColorName )
{
   // Set stock color.
   *this = StockColor::colorI( pStockColorName );
}

//-----------------------------------------------------------------------------

void ColorI::set( const char* pStockColorName )
{
   // Set stock color.
   *this = StockColor::colorI( pStockColorName );
}

//-----------------------------------------------------------------------------

const ColorI& ColorI::StockColor( const char* pStockColorName )
{
   return StockColor::colorI( pStockColorName );
}

//-----------------------------------------------------------------------------

StringTableEntry ColorI::StockColor( void )
{
   // Return stock color name.
   return StockColor::name( *this );
}

//-----------------------------------------------------------------------------

#ifdef TORQUE_USE_LEGACY_GAMMA
//legacy pow 2.2 - powf(color, 2.2f);
float LinearColorF::sSrgbToLinear[256] =
{
   0.000000f,  0.000005f,  0.000023f,  0.000057f,  0.000107f,  0.000175f,  0.000262f,  0.000367f,  0.000493f,
   0.000638f,  0.000805f,  0.000992f,  0.001202f,  0.001433f,  0.001687f,  0.001963f,  0.002263f,  0.002586f,
   0.002932f,  0.003303f,  0.003697f,  0.004116f,  0.004560f,  0.005028f,  0.005522f,  0.006041f,  0.006585f,
   0.007155f,  0.007751f,  0.008373f,  0.009021f,  0.009696f,  0.010398f,  0.011126f,  0.011881f,  0.012664f,
   0.013473f,  0.014311f,  0.015175f,  0.016068f,  0.016988f,  0.017936f,  0.018913f,  0.019918f,  0.020951f,
   0.022013f,  0.023104f,  0.024223f,  0.025371f,  0.026549f,  0.027755f,  0.028991f,  0.030257f,  0.031551f,
   0.032876f,  0.034230f,  0.035614f,  0.037029f,  0.038473f,  0.039947f,  0.041452f,  0.042987f,  0.044553f,
   0.046149f,  0.047776f,  0.049433f,  0.051122f,  0.052842f,  0.054592f,  0.056374f,  0.058187f,  0.060032f,
   0.061907f,  0.063815f,  0.065754f,  0.067725f,  0.069727f,  0.071761f,  0.073828f,  0.075926f,  0.078057f,
   0.080219f,  0.082414f,  0.084642f,  0.086901f,  0.089194f,  0.091518f,  0.093876f,  0.096266f,  0.098689f,
   0.101145f,  0.103634f,  0.106156f,  0.108711f,  0.111299f,  0.113921f,  0.116576f,  0.119264f,  0.121986f,
   0.124741f,  0.127530f,  0.130352f,  0.133209f,  0.136099f,  0.139022f,  0.141980f,  0.144972f,  0.147998f,
   0.151058f,  0.154152f,  0.157281f,  0.160444f,  0.163641f,  0.166872f,  0.170138f,  0.173439f,  0.176774f,
   0.180144f,  0.183549f,  0.186989f,  0.190463f,  0.193972f,  0.197516f,  0.201096f,  0.204710f,  0.208360f,
   0.212044f,  0.215764f,  0.219520f,  0.223310f,  0.227137f,  0.230998f,  0.234895f,  0.238828f,  0.242796f,
   0.246800f,  0.250840f,  0.254916f,  0.259027f,  0.263175f,  0.267358f,  0.271577f,  0.275833f,  0.280124f,
   0.284452f,  0.288816f,  0.293216f,  0.297653f,  0.302126f,  0.306635f,  0.311181f,  0.315763f,  0.320382f,
   0.325037f,  0.329729f,  0.334458f,  0.339223f,  0.344026f,  0.348865f,  0.353741f,  0.358654f,  0.363604f,
   0.368591f,  0.373615f,  0.378676f,  0.383775f,  0.388910f,  0.394083f,  0.399293f,  0.404541f,  0.409826f,
   0.415148f,  0.420508f,  0.425905f,  0.431340f,  0.436813f,  0.442323f,  0.447871f,  0.453456f,  0.459080f,
   0.464741f,  0.470440f,  0.476177f,  0.481952f,  0.487765f,  0.493616f,  0.499505f,  0.505432f,  0.511398f,
   0.517401f,  0.523443f,  0.529523f,  0.535642f,  0.541798f,  0.547994f,  0.554227f,  0.560499f,  0.566810f,
   0.573159f,  0.579547f,  0.585973f,  0.592438f,  0.598942f,  0.605484f,  0.612066f,  0.618686f,  0.625345f,
   0.632043f,  0.638779f,  0.645555f,  0.652370f,  0.659224f,  0.666117f,  0.673049f,  0.680020f,  0.687031f,
   0.694081f,  0.701169f,  0.708298f,  0.715465f,  0.722672f,  0.729919f,  0.737205f,  0.744530f,  0.751895f,
   0.759300f,  0.766744f,  0.774227f,  0.781751f,  0.789314f,  0.796917f,  0.804559f,  0.812241f,  0.819964f,
   0.827726f,  0.835528f,  0.843370f,  0.851252f,  0.859174f,  0.867136f,  0.875138f,  0.883180f,  0.891262f,
   0.899384f,  0.907547f,  0.915750f,  0.923993f,  0.932277f,  0.940601f,  0.948965f,  0.957370f,  0.965815f,
   0.974300f,  0.982826f,  0.991393f,  1.000000f
};
#else
//sRGB - color < 0.04045f ? (1.0f / 12.92f) * color : powf((color + 0.055f) * (1.0f / 1.055f), 2.4f);
float LinearColorF::sSrgbToLinear[256] =
{
   0.000000f,  0.000304f,  0.000607f,  0.000911f,  0.001214f,  0.001518f,  0.001821f,  0.002125f,  0.002428f,
   0.002732f,  0.003035f,  0.003347f,  0.003677f,  0.004025f,  0.004391f,  0.004777f,  0.005182f,  0.005605f,
   0.006049f,  0.006512f,  0.006995f,  0.007499f,  0.008023f,  0.008568f,  0.009134f,  0.009721f,  0.010330f,
   0.010960f,  0.011612f,  0.012286f,  0.012983f,  0.013702f,  0.014444f,  0.015209f,  0.015996f,  0.016807f,
   0.017642f,  0.018500f,  0.019382f,  0.020289f,  0.021219f,  0.022174f,  0.023153f,  0.024158f,  0.025187f,
   0.026241f,  0.027321f,  0.028426f,  0.029557f,  0.030713f,  0.031896f,  0.033105f,  0.034340f,  0.035601f,
   0.036889f,  0.038204f,  0.039546f,  0.040915f,  0.042311f,  0.043735f,  0.045186f,  0.046665f,  0.048172f,
   0.049707f,  0.051269f,  0.052861f,  0.054480f,  0.056128f,  0.057805f,  0.059511f,  0.061246f,  0.063010f,
   0.064803f,  0.066626f,  0.068478f,  0.070360f,  0.072272f,  0.074214f,  0.076185f,  0.078187f,  0.080220f,
   0.082283f,  0.084376f,  0.086500f,  0.088656f,  0.090842f,  0.093059f,  0.095307f,  0.097587f,  0.099899f,
   0.102242f,  0.104616f,  0.107023f,  0.109462f,  0.111932f,  0.114435f,  0.116971f,  0.119538f,  0.122139f,
   0.124772f,  0.127438f,  0.130136f,  0.132868f,  0.135633f,  0.138432f,  0.141263f,  0.144128f,  0.147027f,
   0.149960f,  0.152926f,  0.155926f,  0.158961f,  0.162029f,  0.165132f,  0.168269f,  0.171441f,  0.174647f,
   0.177888f,  0.181164f,  0.184475f,  0.187821f,  0.191202f,  0.194618f,  0.198069f,  0.201556f,  0.205079f,
   0.208637f,  0.212231f,  0.215861f,  0.219526f,  0.223228f,  0.226966f,  0.230740f,  0.234551f,  0.238398f,
   0.242281f,  0.246201f,  0.250158f,  0.254152f,  0.258183f,  0.262251f,  0.266356f,  0.270498f,  0.274677f,
   0.278894f,  0.283149f,  0.287441f,  0.291771f,  0.296138f,  0.300544f,  0.304987f,  0.309469f,  0.313989f,
   0.318547f,  0.323143f,  0.327778f,  0.332452f,  0.337164f,  0.341914f,  0.346704f,  0.351533f,  0.356400f,
   0.361307f,  0.366253f,  0.371238f,  0.376262f,  0.381326f,  0.386430f,  0.391573f,  0.396755f,  0.401978f,
   0.407240f,  0.412543f,  0.417885f,  0.423268f,  0.428691f,  0.434154f,  0.439657f,  0.445201f,  0.450786f,
   0.456411f,  0.462077f,  0.467784f,  0.473532f,  0.479320f,  0.485150f,  0.491021f,  0.496933f,  0.502886f,
   0.508881f,  0.514918f,  0.520996f,  0.527115f,  0.533276f,  0.539480f,  0.545725f,  0.552011f,  0.558340f,
   0.564712f,  0.571125f,  0.577581f,  0.584078f,  0.590619f,  0.597202f,  0.603827f,  0.610496f,  0.617207f,
   0.623960f,  0.630757f,  0.637597f,  0.644480f,  0.651406f,  0.658375f,  0.665387f,  0.672443f,  0.679543f,
   0.686685f,  0.693872f,  0.701102f,  0.708376f,  0.715694f,  0.723055f,  0.730461f,  0.737911f,  0.745404f,
   0.752942f,  0.760525f,  0.768151f,  0.775822f,  0.783538f,  0.791298f,  0.799103f,  0.806952f,  0.814847f,
   0.822786f,  0.830770f,  0.838799f,  0.846873f,  0.854993f,  0.863157f,  0.871367f,  0.879622f,  0.887923f,
   0.896269f,  0.904661f,  0.913099f,  0.921582f,  0.930111f,  0.938686f,  0.947307f,  0.955974f,  0.964686f,
   0.973445f,  0.982251f,  0.991102f,  1.000000f
};
#endif
//-----------------------------------------------------------------------------

ConsoleFunction( getStockColorCount, S32, 1, 1, "() - Gets a count of available stock colors.\n"
   "@return A count of available stock colors." )
{
   return StockColor::getCount();
}

//-----------------------------------------------------------------------------

ConsoleFunction( getStockColorName, const char*, 2, 2,  "(stockColorIndex) - Gets the stock color name at the specified index.\n"
   "@param stockColorIndex The zero-based index of the stock color name to retrieve.\n"
   "@return The stock color name at the specified index or nothing if the string is invalid." )
{
   // Fetch stock color index.
   const S32 stockColorIndex = dAtoi(argv[1]);

   // Fetch the color item.
   const StockColorItem* pColorItem = StockColor::getColorItem( stockColorIndex );

   return pColorItem == NULL ? NULL : pColorItem->getColorName();
}

//-----------------------------------------------------------------------------

ConsoleFunction( isStockColor, bool, 2, 2,  "(stockColorName) - Gets whether the specified name is a stock color or not.\n"
   "@param stockColorName - The stock color name to test for.\n"
   "@return Whether the specified name is a stock color or not.\n" )
{
   // Fetch stock color name.
   const char* pStockColorName = argv[1];

   // Return whether this is a stock color name or not.
   return StockColor::isColor( pStockColorName );
}

//-----------------------------------------------------------------------------

ConsoleFunction( getStockColorF, const char*, 2, 2, "(stockColorName) - Gets a floating-point-based stock color by name.\n"
   "@param stockColorName - The stock color name to retrieve.\n"
   "@return The stock color that matches the specified color name.  Returns nothing if the color name is not found.\n" )
{
   // Fetch stock color name.
   const char* pStockColorName = argv[1];

   // Return nothing if stock color name is invalid.
   if ( !StockColor::isColor( pStockColorName ) )
      return StringTable->EmptyString();

   // Fetch stock color.
   const LinearColorF& color = StockColor::colorF( pStockColorName );

   // Format stock color.
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%g %g %g %g", color.red, color.green, color.blue, color.alpha);
   return(returnBuffer);
}

//-----------------------------------------------------------------------------

ConsoleFunction( getStockColorI, const char*, 2, 2, "(stockColorName) - Gets a byte-based stock color by name.\n"
   "@param stockColorName - The stock color name to retrieve.\n"
   "@return The stock color that matches the specified color name.  Returns nothing if the color name is not found.\n" )
{
   // Fetch stock color name.
   const char* pStockColorName = argv[1];

   // Return nothing if stock color name is invalid.
   if ( !StockColor::isColor( pStockColorName ) )
      return StringTable->EmptyString();

   // Fetch stock color.
   const ColorI& color = StockColor::colorI( pStockColorName );

   // Format stock color.
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d %d %d %d", color.red, color.green, color.blue, color.alpha);
   return(returnBuffer);
}