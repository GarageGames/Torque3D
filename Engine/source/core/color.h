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

#ifndef _COLOR_H_
#define _COLOR_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h" 
#endif
#ifndef _MPOINT4_H_
#include "math/mPoint4.h" 
#endif

#ifndef _ENGINEAPI_H_
#include "console/engineAPI.h"
#endif

const F32 gGamma = 2.2f;
const F32 gOneOverGamma = 1.f / 2.2f;
const F32 gOneOver255 = 1.f / 255.f;

class ColorI;

//32bit color in linear space
class LinearColorF
{
public:
   F32 red;
   F32 green;
   F32 blue;
   F32 alpha;

public:
   LinearColorF() : red(0), green(0), blue(0), alpha(0) {}
   LinearColorF(const LinearColorF& in_rCopy);
   LinearColorF(const F32 in_r, const F32 in_g, const F32 in_b, const F32 in_a = 1.0f);
   LinearColorF(const ColorI &color);
   LinearColorF(const char* pStockColorName);

   void set( const F32 in_r, const F32 in_g, const F32 in_b, const F32 in_a = 1.0f );
   void set( const char* pStockColorName );

   static const LinearColorF& StockColor( const char* pStockColorName );
   StringTableEntry StockColor( void );

   LinearColorF& operator*=(const LinearColorF& in_mul);       // Can be useful for lighting
   LinearColorF  operator*(const LinearColorF& in_mul) const;
   LinearColorF& operator+=(const LinearColorF& in_rAdd);
   LinearColorF  operator+(const LinearColorF& in_rAdd) const;
   LinearColorF& operator-=(const LinearColorF& in_rSub);
   LinearColorF  operator-(const LinearColorF& in_rSub) const;

   LinearColorF& operator*=(const F32 in_mul);
   LinearColorF  operator*(const F32 in_mul) const;
   LinearColorF& operator/=(const F32 in_div);
   LinearColorF  operator/(const F32 in_div) const;

   LinearColorF  operator-() const;

   bool operator==(const LinearColorF&) const;
   bool operator!=(const LinearColorF&) const;

   operator F32*() { return &red; }
   operator const F32*() const { return &red; }

   operator Point3F() const { return Point3F( red, green, blue ); }
   operator Point4F() const { return Point4F( red, green, blue, alpha ); }

   U32 getARGBPack() const;
   U32 getRGBAPack() const;
   U32 getABGRPack() const;

   void interpolate(const LinearColorF& in_rC1,
                    const LinearColorF& in_rC2,
                    const F32 in_factor);

   bool isClamped() const { return (red >= 0.0f && red <= 1.0f) &&
                                   (green >= 0.0f && green <= 1.0f) &&
                                   (blue  >= 0.0f && blue  <= 1.0f) &&
                                   (alpha >= 0.0f && alpha <= 1.0f); }
   void clamp();
   
   //calculate luminance
   F32 luminance();

   //convert to ColorI - slow operation, avoid when possible
   ColorI toColorI(const bool keepAsLinear = false);
   
   static const LinearColorF ZERO;
   static const LinearColorF ONE;
   static const LinearColorF WHITE;
   static const LinearColorF BLACK;
   static const LinearColorF RED;
   static const LinearColorF GREEN;
   static const LinearColorF BLUE;

   static F32 sSrgbToLinear[256];
};


//8bit color in srgb space
class ColorI
{
public:
   U8 red;
   U8 green;
   U8 blue;
   U8 alpha;

   struct Hsb
   {
      Hsb() :hue(0), sat(0), brightness(0){};
      Hsb(U32 h, U32 s, U32 b) :hue(h), sat(s), brightness(b){};

      U32 hue;   ///Hue
      U32 sat;   ///Saturation
      U32 brightness;   //Brightness/Value/Lightness
   };

public:
   ColorI() : red(0), green(0), blue(0), alpha(0) {}
   ColorI(const ColorI& in_rCopy);
   ColorI(const Hsb& color);
   ColorI(const U8 in_r, const U8 in_g, const U8 in_b, const U8 in_a = U8(255));
   ColorI(const ColorI& in_rCopy, const U8 in_a);
   ColorI(const char* pStockColorName);

   void set(const Hsb& color);

   void HSLtoRGB_Subfunction(U32& c, const F64& temp1, const F64& temp2, const F64& temp3);

   void set(const String& hex);

   void set(const U8 in_r,
            const U8 in_g,
            const U8 in_b,
            const U8 in_a = U8(255));

   void set(const ColorI& in_rCopy,
            const U8 in_a);

   void set( const char* pStockColorName );

   static const ColorI& StockColor( const char* pStockColorName );
   StringTableEntry StockColor( void );
   
   bool operator==(const ColorI&) const;
   bool operator!=(const ColorI&) const;
   
   U32 getARGBPack() const;
   U32 getRGBAPack() const;
   U32 getABGRPack() const;

   U32 getBGRPack() const;
   U32 getRGBPack() const;

   U32 getRGBEndian() const;
   U32 getARGBEndian() const;

   U16 get565()  const;
   U16 get4444() const;

   Hsb getHSB() const;

   String getHex() const;
   S32 convertFromHex(const String& hex) const;

   operator const U8*() const { return &red; }

   //convert linear color to srgb - slow operation, avoid when possible
   ColorI fromLinear();
   
   static const ColorI ZERO;
   static const ColorI ONE;
   static const ColorI WHITE;
   static const ColorI BLACK;
   static const ColorI RED;
   static const ColorI GREEN;
   static const ColorI BLUE;
};

//-----------------------------------------------------------------------------

class StockColorItem
{
private:
   StockColorItem() {}

public:
   StockColorItem( const char* pName, const U8 red, const U8 green, const U8 blue, const U8 alpha = 255 )
   {
      // Sanity!
      AssertFatal( pName != NULL, "Stock color name cannot be NULL." );

      // Set stock color.
      // NOTE:-   We'll use the char pointer here.  We can yet use the string-table unfortunately.
      mColorName = pName;
      mColorI.set( red, green, blue, alpha );
      mColorF = mColorI;
   }

   inline const char*      getColorName( void ) const { return mColorName; }
   inline const LinearColorF&    getColorF( void ) const { return mColorF; }
   inline const ColorI&    getColorI( void ) const { return mColorI; }

   const char*         mColorName;
   LinearColorF              mColorF;
   ColorI              mColorI;
};

//-----------------------------------------------------------------------------

class StockColor
{
public:
   static bool isColor( const char* pStockColorName );
   static const LinearColorF& colorF( const char* pStockColorName );
   static const ColorI& colorI( const char* pStockColorName );
   static StringTableEntry name( const LinearColorF& color );
   static StringTableEntry name( const ColorI& color );

   static S32 getCount( void );
   static const StockColorItem* getColorItem( const S32 index );

   static void create( void );
   static void destroy( void );
};

//------------------------------------------------------------------------------
//-------------------------------------- INLINES (LinearColorF)
//
inline void LinearColorF::set(const F32 in_r, const F32 in_g, const F32 in_b, const F32 in_a)
{
   red   = in_r;
   green = in_g;
   blue  = in_b;
   alpha = in_a;
}

inline LinearColorF::LinearColorF(const LinearColorF& in_rCopy)
{
   red   = in_rCopy.red;
   green = in_rCopy.green;
   blue  = in_rCopy.blue;
   alpha = in_rCopy.alpha;
}

inline LinearColorF::LinearColorF(const F32 in_r, const F32 in_g, const F32 in_b, const F32 in_a)
{
   set(in_r, in_g, in_b, in_a);
}

inline LinearColorF& LinearColorF::operator*=(const LinearColorF& in_mul)
{
   red   *= in_mul.red;
   green *= in_mul.green;
   blue  *= in_mul.blue;
   alpha *= in_mul.alpha;

   return *this;
}

inline LinearColorF LinearColorF::operator*(const LinearColorF& in_mul) const
{
   LinearColorF tmp(*this);
   tmp *= in_mul;
   return tmp;
}

inline LinearColorF& LinearColorF::operator+=(const LinearColorF& in_rAdd)
{
   red += in_rAdd.red;
   green += in_rAdd.green;
   blue += in_rAdd.blue;
   alpha += in_rAdd.alpha;
   return *this;
}

inline LinearColorF LinearColorF::operator+(const LinearColorF& in_rAdd) const
{
   LinearColorF temp(*this);
   temp += in_rAdd;
   return temp;
}

inline LinearColorF& LinearColorF::operator-=(const LinearColorF& in_rSub)
{
   red -= in_rSub.red;
   green -= in_rSub.green;
   blue -= in_rSub.blue;
   alpha -= in_rSub.alpha;
   return *this;
}

inline LinearColorF LinearColorF::operator-(const LinearColorF& in_rSub) const
{
   LinearColorF tmp(*this);
   tmp -= in_rSub;
   return tmp;
}

inline LinearColorF& LinearColorF::operator*=(const F32 in_mul)
{
   red   *= in_mul;
   green *= in_mul;
   blue  *= in_mul;
   alpha *= in_mul;
   return *this;
}

inline LinearColorF LinearColorF::operator*(const F32 in_mul) const
{
   LinearColorF tmp(*this);
   tmp *= in_mul;
   return tmp;
}

inline LinearColorF& LinearColorF::operator/=(const F32 in_div)
{
   AssertFatal(in_div != 0.0f, "Error, div by zero...");
   F32 inv = 1.0f / in_div;

   red *= inv;
   green *= inv;
   blue *= inv;
   alpha *= inv;
   return *this;
}

inline LinearColorF LinearColorF::operator/(const F32 in_div) const
{
   AssertFatal(in_div != 0.0f, "Error, div by zero...");
   F32 inv = 1.0f / in_div;
   LinearColorF tmp(*this);
   tmp /= inv;
   return tmp;
}

inline LinearColorF LinearColorF::operator-() const
{
   return LinearColorF(-red, -green, -blue, -alpha);
}

inline bool LinearColorF::operator==(const LinearColorF& in_Cmp) const
{
   return (red == in_Cmp.red && green == in_Cmp.green && blue == in_Cmp.blue && alpha == in_Cmp.alpha);
}

inline bool LinearColorF::operator!=(const LinearColorF& in_Cmp) const
{
   return (red != in_Cmp.red || green != in_Cmp.green || blue != in_Cmp.blue || alpha != in_Cmp.alpha);
}

inline U32 LinearColorF::getARGBPack() const
{
   return (U32(alpha * 255.0f + 0.5) << 24) |
          (U32(red   * 255.0f + 0.5) << 16) |
          (U32(green * 255.0f + 0.5) <<  8) |
          (U32(blue  * 255.0f + 0.5) <<  0);
}

inline U32 LinearColorF::getRGBAPack() const
{
   return ( U32( red   * 255.0f + 0.5) <<  0 ) |
          ( U32( green * 255.0f + 0.5) <<  8 ) |
          ( U32( blue  * 255.0f + 0.5) << 16 ) |
          ( U32( alpha * 255.0f + 0.5) << 24 );
}

inline U32 LinearColorF::getABGRPack() const
{
   return (U32(alpha * 255.0f + 0.5) << 24) |
          (U32(blue  * 255.0f + 0.5) << 16) |
          (U32(green * 255.0f + 0.5) <<  8) |
          (U32(red   * 255.0f + 0.5) <<  0);

}

inline void LinearColorF::interpolate(const LinearColorF& in_rC1,
                    const LinearColorF& in_rC2,
                    const F32  in_factor)
{
   if (in_factor <= 0 || in_rC1 == in_rC2)
   {
      red = in_rC1.red;
      green = in_rC1.green;
      blue =in_rC1.blue;
      alpha = in_rC1.alpha;
      return;
   }
   else if (in_factor >= 1)
   {
      red = in_rC2.red;
      green = in_rC2.green;
      blue = in_rC2.blue;
      alpha = in_rC2.alpha;
      return;
   }

   F32 f2 = 1.0f - in_factor;
   red = (in_rC1.red   * f2) + (in_rC2.red   * in_factor);
   green = (in_rC1.green * f2) + (in_rC2.green * in_factor);
   blue = (in_rC1.blue  * f2) + (in_rC2.blue  * in_factor);
   alpha = (in_rC1.alpha * f2) + (in_rC2.alpha * in_factor);
}

inline void LinearColorF::clamp()
{
   red = mClampF(red, 0.0f, 1.0f);
   green = mClampF(green, 0.0f, 1.0f);
   blue = mClampF(blue, 0.0f, 1.0f);
   alpha = mClampF(alpha, 0.0f, 1.0f);
}

inline F32 LinearColorF::luminance()
{
   // ITU BT.709
   //return red * 0.2126f + green * 0.7152f + blue * 0.0722f;
   // ITU BT.601
   return red * 0.3f + green * 0.59f + blue * 0.11f;
}

//------------------------------------------------------------------------------
//-------------------------------------- INLINES (ColorI)
//
inline void ColorI::set(const U8 in_r,
            const U8 in_g,
            const U8 in_b,
            const U8 in_a)
{
   red   = in_r;
   green = in_g;
   blue  = in_b;
   alpha = in_a;
}

inline void ColorI::set(const ColorI& in_rCopy,
            const U8 in_a)
{
   red   = in_rCopy.red;
   green = in_rCopy.green;
   blue  = in_rCopy.blue;
   alpha = in_a;
}

inline void ColorI::set(const Hsb& color)
{
	U32 r = 0;
	U32 g = 0;
	U32 b = 0;

	F64 L = ((F64)color.brightness) / 100.0;
	F64 S = ((F64)color.sat) / 100.0;
	F64 H = ((F64)color.hue) / 360.0;

	if (color.sat == 0)
	{
		r = color.brightness;
		g = color.brightness;
		b = color.brightness;
	}
	else
	{
		F64 temp1 = 0;
		if (L < 0.50)
		{
			temp1 = L*(1 + S);
		}
		else
		{
			temp1 = L + S - (L*S);
		}

		F64 temp2 = 2.0*L - temp1;

		F64 temp3 = 0;
		for (S32 i = 0; i < 3; i++)
		{
			switch (i)
			{
			case 0: // red
			{
				temp3 = H + 0.33333;
				if (temp3 > 1.0)
					temp3 -= 1.0;
				HSLtoRGB_Subfunction(r, temp1, temp2, temp3);
				break;
			}
			case 1: // green
			{
				temp3 = H;
				HSLtoRGB_Subfunction(g, temp1, temp2, temp3);
				break;
			}
			case 2: // blue
			{
				temp3 = H - 0.33333;
				if (temp3 < 0)
					temp3 += 1;
				HSLtoRGB_Subfunction(b, temp1, temp2, temp3);
				break;
			}
			default:
			{

			}
			}
		}
	}
	red = (U32)((((F64)r) / 100) * 255);
	green = (U32)((((F64)g) / 100) * 255);
	blue = (U32)((((F64)b) / 100) * 255);
}

// This is a subfunction of HSLtoRGB
inline void ColorI::HSLtoRGB_Subfunction(U32& c, const F64& temp1, const F64& temp2, const F64& temp3)
{
	if ((temp3 * 6.0) < 1.0)
		c = (U32)((temp2 + (temp1 - temp2)*6.0*temp3)*100.0);
	else
		if ((temp3 * 2.0) < 1.0)
			c = (U32)(temp1*100.0);
		else
			if ((temp3 * 3.0) < 2.0)
				c = (U32)((temp2 + (temp1 - temp2)*(0.66666 - temp3)*6.0)*100.0);
			else
				c = (U32)(temp2*100.0);
	return;
}

inline void ColorI::set(const String& hex)
{
	String redString;
	String greenString;
	String blueString;

	//if the prefix # was attached to hex
	if (hex[0] == '#')
	{
		redString = hex.substr(1, 2);
		greenString = hex.substr(3, 2);
		blueString = hex.substr(5, 2);
	}
	else
	{
		// since there is no prefix attached to hex
		redString = hex.substr(0, 2);
		greenString = hex.substr(2, 2);
		blueString = hex.substr(4, 2);
	}

	red = (U8)(convertFromHex(redString));
	green = (U8)(convertFromHex(greenString));
	blue = (U8)(convertFromHex(blueString));
}

inline S32 ColorI::convertFromHex(const String& hex) const
{
	S32 hexValue = 0;

	S32 a = 0;
	S32 b = hex.length() - 1;

	for (; b >= 0; a++, b--)
	{
		if (hex[b] >= '0' && hex[b] <= '9')
		{
			hexValue += (hex[b] - '0') * (1 << (a * 4));
		}
		else
		{
			switch (hex[b])
			{
			case 'A':
			case 'a':
				hexValue += 10 * (1 << (a * 4));
				break;

			case 'B':
			case 'b':
				hexValue += 11 * (1 << (a * 4));
				break;

			case 'C':
			case 'c':
				hexValue += 12 * (1 << (a * 4));
				break;

			case 'D':
			case 'd':
				hexValue += 13 * (1 << (a * 4));
				break;

			case 'E':
			case 'e':
				hexValue += 14 * (1 << (a * 4));
				break;

			case 'F':
			case 'f':
				hexValue += 15 * (1 << (a * 4));
				break;

			default:
				Con::errorf("Error, invalid character '%c' in hex number", hex[a]);
				break;
			}
		}
	}

	return hexValue;
}

inline ColorI::ColorI(const ColorI& in_rCopy)
{
   red   = in_rCopy.red;
   green = in_rCopy.green;
   blue  = in_rCopy.blue;
   alpha = in_rCopy.alpha;
}

inline ColorI::ColorI(const Hsb& color)
{
	set(color);
}

inline ColorI::ColorI(const U8 in_r,
               const U8 in_g,
               const U8 in_b,
               const U8 in_a)
{
   set(in_r, in_g, in_b, in_a);
}

inline ColorI::ColorI(const ColorI& in_rCopy,
                      const U8 in_a)
{
   set(in_rCopy, in_a);
}

inline bool ColorI::operator==(const ColorI& in_Cmp) const
{
	return (red == in_Cmp.red && green == in_Cmp.green && blue == in_Cmp.blue && alpha == in_Cmp.alpha);
}

inline bool ColorI::operator!=(const ColorI& in_Cmp) const
{
	return (red != in_Cmp.red || green != in_Cmp.green || blue != in_Cmp.blue || alpha != in_Cmp.alpha);
}

inline U32 ColorI::getARGBPack() const
{
   return (U32(alpha) << 24) |
          (U32(red)   << 16) |
          (U32(green) <<  8) |
          (U32(blue)  <<  0);
}

inline U32 ColorI::getRGBAPack() const
{
   return ( U32( red )   <<  0 ) |
          ( U32( green ) <<  8 ) |
          ( U32( blue )  << 16 ) |
          ( U32( alpha ) << 24 );
}

inline U32 ColorI::getABGRPack() const
{
   return (U32(alpha) << 24) |
          (U32(blue)  << 16) |
          (U32(green) <<  8) |
          (U32(red)   <<  0);
}


inline U32 ColorI::getBGRPack() const
{
   return (U32(blue)  << 16) |
          (U32(green) <<  8) |
          (U32(red)   <<  0);
}

inline U32 ColorI::getRGBPack() const
{
   return (U32(red)   << 16) |
          (U32(green) <<  8) |
          (U32(blue)  <<  0);
}

inline U32 ColorI::getRGBEndian() const
{
#if defined(TORQUE_BIG_ENDIAN)
      return(getRGBPack());
#else
      return(getBGRPack());
#endif
}

inline U32 ColorI::getARGBEndian() const
{
#if defined(TORQUE_BIG_ENDIAN)
   return(getABGRPack());
#else
   return(getARGBPack());
#endif
}

inline U16 ColorI::get565() const
{
   return U16((U16(red   >> 3) << 11) |
              (U16(green >> 2) <<  5) |
              (U16(blue  >> 3) <<  0));
}

inline U16 ColorI::get4444() const
{
   return U16(U16(U16(alpha >> 4) << 12) |
              U16(U16(red   >> 4) <<  8) |
              U16(U16(green >> 4) <<  4) |
              U16(U16(blue  >> 4) <<  0));
}

inline ColorI::Hsb ColorI::getHSB() const
{
	F64 rPercent = ((F64)red) / 255;
	F64 gPercent = ((F64)green) / 255;
	F64 bPercent = ((F64)blue) / 255;

	F64 maxColor = 0.0;
	if ((rPercent >= gPercent) && (rPercent >= bPercent))
		maxColor = rPercent;
	if ((gPercent >= rPercent) && (gPercent >= bPercent))
		maxColor = gPercent;
	if ((bPercent >= rPercent) && (bPercent >= gPercent))
		maxColor = bPercent;

	F64 minColor = 0.0;
	if ((rPercent <= gPercent) && (rPercent <= bPercent))
		minColor = rPercent;
	if ((gPercent <= rPercent) && (gPercent <= bPercent))
		minColor = gPercent;
	if ((bPercent <= rPercent) && (bPercent <= gPercent))
		minColor = bPercent;

	F64 H = 0.0;
	F64 S = 0.0;
	F64 B = 0.0;

	B = (maxColor + minColor) / 2.0;

	if (maxColor == minColor)
	{
		H = 0.0;
		S = 0.0;
	}
	else
	{
		if (B < 0.50)
		{
			S = (maxColor - minColor) / (maxColor + minColor);
		}
		else
		{
			S = (maxColor - minColor) / (2.0 - maxColor - minColor);
		}
		if (maxColor == rPercent)
		{
			H = (gPercent - bPercent) / (maxColor - minColor);
		}
		if (maxColor == gPercent)
		{
			H = 2.0 + (bPercent - rPercent) / (maxColor - minColor);
		}
		if (maxColor == bPercent)
		{
			H = 4.0 + (rPercent - gPercent) / (maxColor - minColor);
		}
	}

	ColorI::Hsb val;
	val.sat = (U32)(S * 100);
	val.brightness = (U32)(B * 100);
	H = H*60.0;
	if (H < 0.0)
		H += 360.0;
	val.hue = (U32)H;

	return val;
}

inline String ColorI::getHex() const
{
	char r[255];
	dSprintf(r, sizeof(r), "%.2X", red);
	String result(r);

	char g[255];
	dSprintf(g, sizeof(g), "%.2X", green);
	result += g;

	char b[255];
	dSprintf(b, sizeof(b), "%.2X", blue);
	result += b;

	return result;
}

inline LinearColorF::LinearColorF( const ColorI &color)
{
   red = sSrgbToLinear[color.red],
   green = sSrgbToLinear[color.green],
   blue = sSrgbToLinear[color.blue],
   alpha = F32(color.alpha * gOneOver255);
}

inline ColorI LinearColorF::toColorI(const bool keepAsLinear)
{
   if (isClamped())
   {
      if (keepAsLinear)
      {
         return ColorI(U8(red * 255.0f + 0.5), U8(green * 255.0f + 0.5), U8(blue * 255.0f + 0.5), U8(alpha * 255.0f + 0.5));
      }
      else
      {
   #ifdef TORQUE_USE_LEGACY_GAMMA
         float r = mPow(red, gOneOverGamma);
         float g = mPow(green, gOneOverGamma);
         float b = mPow(blue, gOneOverGamma);
         return ColorI(U8(r * 255.0f + 0.5), U8(g * 255.0f + 0.5), U8(b * 255.0f + 0.5), U8(alpha * 255.0f + 0.5));
   #else
         float r = red < 0.0031308f ? 12.92f * red : 1.055 * mPow(red, 1.0f / 2.4f) - 0.055f;
         float g = green < 0.0031308f ? 12.92f * green : 1.055 * mPow(green, 1.0f / 2.4f) - 0.055f;
         float b = blue < 0.0031308f ? 12.92f * blue : 1.055 * mPow(blue, 1.0f / 2.4f) - 0.055f;
         return ColorI(U8(r * 255.0f + 0.5), U8(g * 255.0f + 0.5), U8(b * 255.0f + 0.5), U8(alpha * 255.0f + 0.5));
   #endif
      }
   }
   else
   {
      LinearColorF color = LinearColorF(*this);
      color.clamp();

      if (keepAsLinear)
      {
         return ColorI(U8(color.red * 255.0f + 0.5), U8(color.green * 255.0f + 0.5), U8(color.blue * 255.0f + 0.5), U8(color.alpha * 255.0f + 0.5));
      }
      else
      {
   #ifdef TORQUE_USE_LEGACY_GAMMA
         float r = mPow(red, gOneOverGamma);
         float g = mPow(green, gOneOverGamma);
         float b = mPow(blue, gOneOverGamma);
         return ColorI(U8(r * 255.0f + 0.5), U8(g * 255.0f + 0.5), U8(b * 255.0f + 0.5), U8(alpha * 255.0f + 0.5));
   #else
         float r = red < 0.0031308f ? 12.92f * red : 1.055 * mPow(red, 1.0f / 2.4f) - 0.055f;
         float g = green < 0.0031308f ? 12.92f * green : 1.055 * mPow(green, 1.0f / 2.4f) - 0.055f;
         float b = blue < 0.0031308f ? 12.92f * blue : 1.055 * mPow(blue, 1.0f / 2.4f) - 0.055f;
         return ColorI(U8(r * 255.0f + 0.5), U8(g * 255.0f + 0.5), U8(b * 255.0f + 0.5), U8(alpha * 255.0f + 0.5));
   #endif
      }
   }
}

inline ColorI ColorI::fromLinear()
{
   //manually create LinearColorF, otherwise it will try and convert to linear first
   LinearColorF linearColor = LinearColorF(F32(red) * 255.0f + 0.5f,
                                           F32(red) * 255.0f + 0.5f,
                                           F32(red) * 255.0f + 0.5f,
                                           F32(alpha) * 255.0f + 0.5f);
   //convert back to srgb
   return linearColor.toColorI();
}

#endif //_COLOR_H_
