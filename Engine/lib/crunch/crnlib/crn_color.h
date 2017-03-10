// File: crn_color.h
// See Copyright Notice and license at the end of inc/crnlib.h
#pragma once
#include "crn_core.h"

namespace crnlib
{
   template<typename component_type> struct color_quad_component_traits
   {
      enum
      {
         cSigned = false,
         cFloat = false,
         cMin = UINT8_MIN,
         cMax = UINT8_MAX
      };
   };

   template<> struct color_quad_component_traits<int16>
   {
      enum
      {
         cSigned = true,
         cFloat = false,
         cMin = INT16_MIN,
         cMax = INT16_MAX
      };
   };

   template<> struct color_quad_component_traits<uint16>
   {
      enum
      {
         cSigned = false,
         cFloat = false,
         cMin = UINT16_MIN,
         cMax = UINT16_MAX
      };
   };

   template<> struct color_quad_component_traits<int32>
   {
      enum
      {
         cSigned = true,
         cFloat = false,
         cMin = INT32_MIN,
         cMax = INT32_MAX
      };
   };

   template<> struct color_quad_component_traits<uint32>
   {
      enum
      {
         cSigned = false,
         cFloat = false,
         cMin = UINT32_MIN,
         cMax = UINT32_MAX
      };
   };

   template<> struct color_quad_component_traits<float>
   {
      enum
      {
         cSigned = false,
         cFloat = true,
         cMin = INT32_MIN,
         cMax = INT32_MAX
      };
   };

   template<> struct color_quad_component_traits<double>
   {
      enum
      {
         cSigned = false,
         cFloat = true,
         cMin = INT32_MIN,
         cMax = INT32_MAX
      };
   };

   template<typename component_type, typename parameter_type>
   class color_quad : public helpers::rel_ops<color_quad<component_type, parameter_type> >
   {
      template<typename T>
      static inline T clamp(T v)
      {
         if (!component_traits::cFloat)
         {
            if (v < component_traits::cMin)
               v = component_traits::cMin;
            else if (v > component_traits::cMax)
               v = component_traits::cMax;
         }
         return v;
      }

#ifdef _MSC_VER
      template<>
      static inline int clamp(int v)
      {
         if (!component_traits::cFloat)
         {
            if ((!component_traits::cSigned) && (component_traits::cMin == 0) && (component_traits::cMax == 0xFF))
            {
               if (v & 0xFFFFFF00U)
                  v = (~(static_cast<int>(v) >> 31)) & 0xFF;
            }
            else
            {
               if (v < component_traits::cMin)
                  v = component_traits::cMin;
               else if (v > component_traits::cMax)
                  v = component_traits::cMax;
            }
         }
         return v;
      }
#endif

   public:
      typedef component_type component_t;
      typedef parameter_type parameter_t;
      typedef color_quad_component_traits<component_type> component_traits;

      enum { cNumComps = 4 };

      union
      {
         struct
         {
            component_type r;
            component_type g;
            component_type b;
            component_type a;
         };

         component_type c[cNumComps];

         uint32 m_u32;
      };

      inline color_quad()
      {
      }

      inline color_quad(eClear) :
         r(0), g(0), b(0), a(0)
      {
      }

      inline color_quad(const color_quad& other) :
         r(other.r), g(other.g), b(other.b), a(other.a)
      {
      }

      explicit inline color_quad(parameter_type y, parameter_type alpha = component_traits::cMax)
      {
         set(y, alpha);
      }

      inline color_quad(parameter_type red, parameter_type green, parameter_type blue, parameter_type alpha = component_traits::cMax)
      {
         set(red, green, blue, alpha);
      }

      explicit inline color_quad(eNoClamp, parameter_type y, parameter_type alpha = component_traits::cMax)
      {
         set_noclamp_y_alpha(y, alpha);
      }

      inline color_quad(eNoClamp, parameter_type red, parameter_type green, parameter_type blue, parameter_type alpha = component_traits::cMax)
      {
         set_noclamp_rgba(red, green, blue, alpha);
      }

      template<typename other_component_type, typename other_parameter_type>
      inline color_quad(const color_quad<other_component_type, other_parameter_type>& other) :
         r(clamp(other.r)), g(clamp(other.g)), b(clamp(other.b)), a(clamp(other.a))
      {
      }

      inline void clear()
      {
         r = 0;
         g = 0;
         b = 0;
         a = 0;
      }

      inline color_quad& operator= (const color_quad& other)
      {
         r = other.r;
         g = other.g;
         b = other.b;
         a = other.a;
         return *this;
      }

      template<typename other_component_type, typename other_parameter_type>
      inline color_quad& operator=(const color_quad<other_component_type, other_parameter_type>& other)
      {
         r = clamp(other.r);
         g = clamp(other.g);
         b = clamp(other.b);
         a = clamp(other.a);
         return *this;
      }

      inline color_quad& operator= (parameter_type y)
      {
         set(y, component_traits::cMax);
         return *this;
      }

      inline color_quad& set(parameter_type y, parameter_type alpha = component_traits::cMax)
      {
         y = clamp(y);
         alpha = clamp(alpha);
         r = static_cast<component_type>(y);
         g = static_cast<component_type>(y);
         b = static_cast<component_type>(y);
         a = static_cast<component_type>(alpha);
         return *this;
      }

      inline color_quad& set_noclamp_y_alpha(parameter_type y, parameter_type alpha = component_traits::cMax)
      {
         CRNLIB_ASSERT( (y >= component_traits::cMin) && (y <= component_traits::cMax) );
         CRNLIB_ASSERT( (alpha >= component_traits::cMin) && (alpha <= component_traits::cMax) );

         r = static_cast<component_type>(y);
         g = static_cast<component_type>(y);
         b = static_cast<component_type>(y);
         a = static_cast<component_type>(alpha);
         return *this;
      }

      inline color_quad& set(parameter_type red, parameter_type green, parameter_type blue, parameter_type alpha = component_traits::cMax)
      {
         r = static_cast<component_type>(clamp(red));
         g = static_cast<component_type>(clamp(green));
         b = static_cast<component_type>(clamp(blue));
         a = static_cast<component_type>(clamp(alpha));
         return *this;
      }

      inline color_quad& set_noclamp_rgba(parameter_type red, parameter_type green, parameter_type blue, parameter_type alpha)
      {
         CRNLIB_ASSERT( (red >= component_traits::cMin) && (red <= component_traits::cMax) );
         CRNLIB_ASSERT( (green >= component_traits::cMin) && (green <= component_traits::cMax) );
         CRNLIB_ASSERT( (blue >= component_traits::cMin) && (blue <= component_traits::cMax) );
         CRNLIB_ASSERT( (alpha >= component_traits::cMin) && (alpha <= component_traits::cMax) );

         r = static_cast<component_type>(red);
         g = static_cast<component_type>(green);
         b = static_cast<component_type>(blue);
         a = static_cast<component_type>(alpha);
         return *this;
      }

      inline color_quad& set_noclamp_rgb(parameter_type red, parameter_type green, parameter_type blue)
      {
         CRNLIB_ASSERT( (red >= component_traits::cMin) && (red <= component_traits::cMax) );
         CRNLIB_ASSERT( (green >= component_traits::cMin) && (green <= component_traits::cMax) );
         CRNLIB_ASSERT( (blue >= component_traits::cMin) && (blue <= component_traits::cMax) );

         r = static_cast<component_type>(red);
         g = static_cast<component_type>(green);
         b = static_cast<component_type>(blue);
         return *this;
      }

      static inline parameter_type get_min_comp() { return component_traits::cMin; }
      static inline parameter_type get_max_comp() { return component_traits::cMax; }
      static inline bool get_comps_are_signed() { return component_traits::cSigned; }

      inline component_type operator[] (uint i) const { CRNLIB_ASSERT(i < cNumComps); return c[i]; }
      inline component_type& operator[] (uint i) { CRNLIB_ASSERT(i < cNumComps); return c[i]; }

      inline color_quad& set_component(uint i, parameter_type f)
      {
         CRNLIB_ASSERT(i < cNumComps);

         c[i] = static_cast<component_type>(clamp(f));

         return *this;
      }

      inline color_quad& set_grayscale(parameter_t l)
      {
         component_t x = static_cast<component_t>(clamp(l));
         c[0] = x;
         c[1] = x;
         c[2] = x;
         return *this;
      }

      inline color_quad& clamp(const color_quad& l, const color_quad& h)
      {
         for (uint i = 0; i < cNumComps; i++)
            c[i] = static_cast<component_type>(math::clamp<parameter_type>(c[i], l[i], h[i]));
         return *this;
      }

      inline color_quad& clamp(parameter_type l, parameter_type h)
      {
         for (uint i = 0; i < cNumComps; i++)
            c[i] = static_cast<component_type>(math::clamp<parameter_type>(c[i], l, h));
         return *this;
      }

      // Returns CCIR 601 luma (consistent with color_utils::RGB_To_Y).
      inline parameter_type get_luma() const
      {
         return static_cast<parameter_type>((19595U * r + 38470U * g + 7471U * b + 32768U) >> 16U);
      }

      // Returns REC 709 luma.
      inline parameter_type get_luma_rec709() const
      {
         return static_cast<parameter_type>((13938U * r + 46869U * g + 4729U * b + 32768U) >> 16U);
      }

      // Beware of endianness!
      inline uint32 get_uint32() const
      {
         CRNLIB_ASSERT(sizeof(*this) == sizeof(uint32));
         return *reinterpret_cast<const uint32*>(this);
      }

      // Beware of endianness!
      inline uint64 get_uint64() const
      {
         CRNLIB_ASSERT(sizeof(*this) == sizeof(uint64));
         return *reinterpret_cast<const uint64*>(this);
      }

      inline uint squared_distance(const color_quad& c, bool alpha = true) const
      {
         return math::square(r - c.r) + math::square(g - c.g) + math::square(b - c.b) + (alpha ? math::square(a - c.a) : 0);
      }

      inline bool rgb_equals(const color_quad& rhs) const
      {
         return (r == rhs.r) && (g == rhs.g) && (b == rhs.b);
      }

      inline bool operator== (const color_quad& rhs) const
      {
         if (sizeof(color_quad) == sizeof(uint32))
            return m_u32 == rhs.m_u32;
         else
            return (r == rhs.r) && (g == rhs.g) && (b == rhs.b) && (a == rhs.a);
      }

      inline bool operator< (const color_quad& rhs) const
      {
         for (uint i = 0; i < cNumComps; i++)
         {
            if (c[i] < rhs.c[i])
               return true;
            else if (!(c[i] == rhs.c[i]))
               return false;
         }
         return false;
      }

      color_quad& operator+= (const color_quad& other)
      {
         for (uint i = 0; i < 4; i++)
            c[i] = static_cast<component_type>(clamp(c[i] + other.c[i]));
         return *this;
      }

      color_quad& operator-= (const color_quad& other)
      {
         for (uint i = 0; i < 4; i++)
            c[i] = static_cast<component_type>(clamp(c[i] - other.c[i]));
         return *this;
      }

      color_quad& operator*= (parameter_type v)
      {
         for (uint i = 0; i < 4; i++)
            c[i] = static_cast<component_type>(clamp(c[i] * v));
         return *this;
      }

      color_quad& operator/= (parameter_type v)
      {
         for (uint i = 0; i < 4; i++)
            c[i] = static_cast<component_type>(c[i] / v);
         return *this;
      }

      color_quad get_swizzled(uint x, uint y, uint z, uint w) const
      {
         CRNLIB_ASSERT((x | y | z | w) < 4);
         return color_quad(c[x], c[y], c[z], c[w]);
      }

      friend color_quad operator+ (const color_quad& lhs, const color_quad& rhs)
      {
         color_quad result(lhs);
         result += rhs;
         return result;
      }

      friend color_quad operator- (const color_quad& lhs, const color_quad& rhs)
      {
         color_quad result(lhs);
         result -= rhs;
         return result;
      }

      friend color_quad operator* (const color_quad& lhs, parameter_type v)
      {
         color_quad result(lhs);
         result *= v;
         return result;
      }

      friend color_quad operator/ (const color_quad& lhs, parameter_type v)
      {
         color_quad result(lhs);
         result /= v;
         return result;
      }

      friend color_quad operator* (parameter_type v, const color_quad& rhs)
      {
         color_quad result(rhs);
         result *= v;
         return result;
      }

      inline bool is_grayscale() const
      {
         return (c[0] == c[1]) && (c[1] == c[2]);
      }

      uint get_min_component_index(bool alpha = true) const
      {
         uint index = 0;
         uint limit = alpha ? cNumComps : (cNumComps - 1);
         for (uint i = 1; i < limit; i++)
            if (c[i] < c[index])
               index = i;
         return index;
      }

      uint get_max_component_index(bool alpha = true) const
      {
         uint index = 0;
         uint limit = alpha ? cNumComps : (cNumComps - 1);
         for (uint i = 1; i < limit; i++)
            if (c[i] > c[index])
               index = i;
         return index;
      }

      operator size_t() const
      {
         return (size_t)fast_hash(this, sizeof(*this));
      }

      void get_float4(float* pDst)
      {
         for (uint i = 0; i < 4; i++)
            pDst[i] = ((*this)[i] - component_traits::cMin) / float(component_traits::cMax - component_traits::cMin);
      }

      void get_float3(float* pDst)
      {
         for (uint i = 0; i < 3; i++)
            pDst[i] = ((*this)[i] - component_traits::cMin) / float(component_traits::cMax - component_traits::cMin);
      }

      static color_quad component_min(const color_quad& a, const color_quad& b)
      {
         color_quad result;
         for (uint i = 0; i < 4; i++)
            result[i] = static_cast<component_type>(math::minimum(a[i], b[i]));
         return result;
      }

      static color_quad component_max(const color_quad& a, const color_quad& b)
      {
         color_quad result;
         for (uint i = 0; i < 4; i++)
            result[i] = static_cast<component_type>(math::maximum(a[i], b[i]));
         return result;
      }

      static color_quad make_black()
      {
         return color_quad(0, 0, 0, component_traits::cMax);
      }

      static color_quad make_white()
      {
         return color_quad(component_traits::cMax, component_traits::cMax, component_traits::cMax, component_traits::cMax);
      }
   }; // class color_quad

   template<typename c, typename q>
   struct scalar_type< color_quad<c, q> >
   {
      enum { cFlag = true };
      static inline void construct(color_quad<c, q>* p) { }
      static inline void construct(color_quad<c, q>* p, const color_quad<c, q>& init) { memcpy(p, &init, sizeof(color_quad<c, q>)); }
      static inline void construct_array(color_quad<c, q>* p, uint n) { p, n; }
      static inline void destruct(color_quad<c, q>* p) { p; }
      static inline void destruct_array(color_quad<c, q>* p, uint n) { p, n; }
   };

   typedef color_quad<uint8, int>      color_quad_u8;
   typedef color_quad<int16, int>      color_quad_i16;
   typedef color_quad<uint16, int>     color_quad_u16;
   typedef color_quad<int32, int>      color_quad_i32;
   typedef color_quad<uint32, uint>    color_quad_u32;
   typedef color_quad<float, float>    color_quad_f;
   typedef color_quad<double, double>  color_quad_d;

   namespace color
   {
      inline uint elucidian_distance(uint r0, uint g0, uint b0, uint r1, uint g1, uint b1)
      {
         int dr = (int)r0 - (int)r1;
         int dg = (int)g0 - (int)g1;
         int db = (int)b0 - (int)b1;

         return static_cast<uint>(dr * dr + dg * dg + db * db);
      }

      inline uint elucidian_distance(uint r0, uint g0, uint b0, uint a0, uint r1, uint g1, uint b1, uint a1)
      {
         int dr = (int)r0 - (int)r1;
         int dg = (int)g0 - (int)g1;
         int db = (int)b0 - (int)b1;
         int da = (int)a0 - (int)a1;

         return static_cast<uint>(dr * dr + dg * dg + db * db + da * da);
      }

      inline uint elucidian_distance(const color_quad_u8& c0, const color_quad_u8& c1, bool alpha)
      {
         if (alpha)
            return elucidian_distance(c0.r, c0.g, c0.b, c0.a, c1.r, c1.g, c1.b, c1.a);
         else
            return elucidian_distance(c0.r, c0.g, c0.b, c1.r, c1.g, c1.b);
      }

      inline uint weighted_elucidian_distance(uint r0, uint g0, uint b0, uint r1, uint g1, uint b1, uint wr, uint wg, uint wb)
      {
         int dr = (int)r0 - (int)r1;
         int dg = (int)g0 - (int)g1;
         int db = (int)b0 - (int)b1;

         return static_cast<uint>((wr * dr * dr) + (wg * dg * dg) + (wb * db * db));
      }

      inline uint weighted_elucidian_distance(
         uint r0, uint g0, uint b0, uint a0,
         uint r1, uint g1, uint b1, uint a1,
         uint wr, uint wg, uint wb, uint wa)
      {
         int dr = (int)r0 - (int)r1;
         int dg = (int)g0 - (int)g1;
         int db = (int)b0 - (int)b1;
         int da = (int)a0 - (int)a1;

         return static_cast<uint>((wr * dr * dr) + (wg * dg * dg) + (wb * db * db) + (wa * da * da));
      }

      inline uint weighted_elucidian_distance(const color_quad_u8& c0, const color_quad_u8& c1, uint wr, uint wg, uint wb, uint wa)
      {
         return weighted_elucidian_distance(c0.r, c0.g, c0.b, c0.a, c1.r, c1.g, c1.b, c1.a, wr, wg, wb, wa);
      }

      //const uint cRWeight = 8;//24;
      //const uint cGWeight = 24;//73;
      //const uint cBWeight = 1;//3;

      const uint cRWeight = 8;//24;
      const uint cGWeight = 25;//73;
      const uint cBWeight = 1;//3;

      inline uint color_distance(bool perceptual, const color_quad_u8& e1, const color_quad_u8& e2, bool alpha)
      {
         if (perceptual)
         {
            if (alpha)
               return weighted_elucidian_distance(e1, e2, cRWeight, cGWeight, cBWeight, cRWeight+cGWeight+cBWeight);
            else
               return weighted_elucidian_distance(e1, e2, cRWeight, cGWeight, cBWeight, 0);
         }
         else
            return elucidian_distance(e1, e2, alpha);
      }

      inline uint peak_color_error(const color_quad_u8& e1, const color_quad_u8& e2)
      {
         return math::maximum<uint>(labs(e1[0] - e2[0]), labs(e1[1] - e2[1]), labs(e1[2] - e2[2]));
         //return math::square<int>(e1[0] - e2[0]) + math::square<int>(e1[1] - e2[1]) + math::square<int>(e1[2] - e2[2]);
      }

      // y - [0,255]
      // co - [-127,127]
      // cg - [-126,127]
      inline void RGB_to_YCoCg(int r, int g, int b, int& y, int& co, int& cg)
      {
         y  =  (r >> 2) + (g >> 1) + (b >> 2);
         co =  (r >> 1)            - (b >> 1);
         cg = -(r >> 2) + (g >> 1) - (b >> 2);
      }

      inline void YCoCg_to_RGB(int y, int co, int cg, int& r, int& g, int& b)
      {
         int tmp = y - cg;
         g = y + cg;
         r = tmp + co;
         b = tmp - co;
      }

      static inline uint8 clamp_component(int i) { if (static_cast<uint>(i) > 255U) { if (i < 0) i = 0; else if (i > 255) i = 255; } return static_cast<uint8>(i); }

      // RGB->YCbCr constants, scaled by 2^16
      const int YR = 19595, YG = 38470, YB = 7471, CB_R = -11059, CB_G = -21709, CB_B = 32768, CR_R = 32768, CR_G = -27439, CR_B = -5329;
      // YCbCr->RGB constants, scaled by 2^16
      const int R_CR = 91881, B_CB = 116130, G_CR = -46802, G_CB = -22554;
      
      inline int RGB_to_Y(const color_quad_u8& rgb)
      {
         const int r = rgb[0], g = rgb[1], b = rgb[2];
         return (r * YR + g * YG + b * YB + 32768) >> 16;
      }

      // RGB to YCbCr (same as JFIF JPEG).
      // Odd default biases account for 565 endpoint packing.
      inline void RGB_to_YCC(color_quad_u8& ycc, const color_quad_u8& rgb, int cb_bias = 123, int cr_bias = 125)
      {
         const int r = rgb[0], g = rgb[1], b = rgb[2];
         ycc.a = static_cast<uint8>((r * YR + g * YG + b * YB + 32768) >> 16);
         ycc.r = clamp_component(cb_bias + ((r * CB_R + g * CB_G + b * CB_B + 32768) >> 16));
         ycc.g = clamp_component(cr_bias + ((r * CR_R + g * CR_G + b * CR_B + 32768) >> 16));
         ycc.b = 0;
      }

      // YCbCr to RGB.
      // Odd biases account for 565 endpoint packing.
      inline void YCC_to_RGB(color_quad_u8& rgb, const color_quad_u8& ycc, int cb_bias = 123, int cr_bias = 125)
      {
         const int y = ycc.a;
         const int cb = ycc.r - cb_bias;
         const int cr = ycc.g - cr_bias;
         rgb.r = clamp_component(y + ((R_CR * cr             + 32768) >> 16));
         rgb.g = clamp_component(y + ((G_CR * cr + G_CB * cb + 32768) >> 16));
         rgb.b = clamp_component(y + ((B_CB * cb             + 32768) >> 16));
         rgb.a = 255;
      }
      
      // Float RGB->YCbCr constants
      const float S = 1.0f/65536.0f;
      const float F_YR = S*YR, F_YG = S*YG, F_YB = S*YB, F_CB_R = S*CB_R, F_CB_G = S*CB_G, F_CB_B = S*CB_B, F_CR_R = S*CR_R, F_CR_G = S*CR_G, F_CR_B = S*CR_B;
      // Float YCbCr->RGB constants
      const float F_R_CR = S*R_CR, F_B_CB = S*B_CB, F_G_CR = S*G_CR, F_G_CB = S*G_CB;

      inline void RGB_to_YCC_float(color_quad_f& ycc, const color_quad_u8& rgb)
      {
         const int r = rgb[0], g = rgb[1], b = rgb[2];
         ycc.a = r * F_YR   + g * F_YG   + b * F_YB;
         ycc.r = r * F_CB_R + g * F_CB_G + b * F_CB_B;
         ycc.g = r * F_CR_R + g * F_CR_G + b * F_CR_B;
         ycc.b = 0;
      }

      inline void YCC_float_to_RGB(color_quad_u8& rgb, const color_quad_f& ycc)
      {
         float y = ycc.a, cb = ycc.r, cr = ycc.g;
         rgb.r = color::clamp_component(static_cast<int>(.5f + y + F_R_CR * cr));
         rgb.g = color::clamp_component(static_cast<int>(.5f + y + F_G_CR * cr + F_G_CB * cb));
         rgb.b = color::clamp_component(static_cast<int>(.5f + y               + F_B_CB * cb));
         rgb.a = 255;
      }

   } // namespace color

} // namespace crnlib

