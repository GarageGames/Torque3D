
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#ifndef _AFX_ZODIAC_DEFS_H_
#define _AFX_ZODIAC_DEFS_H_

class afxZodiacDefs
{
public:
  enum 
  {
     MAX_ZODIACS = 256,
     N_ZODIAC_FIELD_INTS = (MAX_ZODIACS - 1) / 32 + 1,
  };

  enum
  {
    BLEND_NORMAL      = 0x0,
    BLEND_ADDITIVE    = 0x1,
    BLEND_SUBTRACTIVE = 0x2,
    BLEND_RESERVED    = 0x3,
    BLEND_MASK        = 0x3
  };

  enum 
  {
    SHOW_ON_TERRAIN       = BIT(2),
    SHOW_ON_INTERIORS     = BIT(3),
    SHOW_ON_WATER         = BIT(4),
    SHOW_ON_MODELS        = BIT(5),
    //
    SHOW_IN_NON_REFLECTIONS = BIT(6),
    SHOW_IN_REFLECTIONS     = BIT(7),
    //
    RESPECT_ORIENTATION   = BIT(8),
    SCALE_VERT_RANGE      = BIT(9),
    INVERT_GRADE_RANGE    = BIT(10),
    USE_GRADE_RANGE       = BIT(11),
    PREFER_DEST_GRADE     = BIT(12),
    //
    INTERIOR_VERT_IGNORE    = BIT(13),
    INTERIOR_HORIZ_ONLY     = BIT(14),
    INTERIOR_BACK_IGNORE    = BIT(15),
    INTERIOR_OPAQUE_IGNORE  = BIT(16),
    INTERIOR_TRANSP_IGNORE  = BIT(17),
    //
    INTERIOR_FILTERS = INTERIOR_VERT_IGNORE | INTERIOR_HORIZ_ONLY | INTERIOR_BACK_IGNORE
  };
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxZodiacBitmask : public afxZodiacDefs
{
  U32 mBits[N_ZODIAC_FIELD_INTS];
  
public:
  afxZodiacBitmask() { clear(); }
  afxZodiacBitmask(const afxZodiacBitmask& field) { *this = field; }
  
  bool test(U32 index)
  {
    U32 word = index / 32;
    U32 bit = index % 32;
    return mBits[word] & (1 << bit);
  }
  
  void set(U32 index)
  {
    U32 word = index / 32;
    U32 bit = index % 32;
    mBits[word] |= 1 << bit;
  }
  
  void unset(U32 index)
  {
    U32 word = index / 32;
    U32 bit = index % 32;
    mBits[word] &= ~(1 << bit);
  }
  
  S32 findLastSetBit(U32 startbit=(MAX_ZODIACS-1))
  {
    U32 word = startbit / 32;
    U32 bit = startbit % 32;

    if (mBits[word] != 0)
    { 
      U32 mask = mBits[word] << (31-bit);
      for (U32 j = bit; j >= 0; j--)
      {
        if (mask & 0x80000000)
          return word*32 + j;
        mask <<= 1;
      }
    }
    
    for (U32 k = word-1; k >= 0; k--)
    {
      if (mBits[k] != 0)
      { 
        U32 mask = mBits[k];
        for (U32 j = 31; j >= 0; j--)
        {
          if (mask & 0x80000000)
            return k*32 + j;
          mask <<= 1;
        }
      }
    }

    return -1;
  }
  
  void clear()
  {
    for (U32 k = 0; k < N_ZODIAC_FIELD_INTS; k++)
      mBits[k] = 0x00000000;
  }
  
  bool isEmpty()
  {
    for (U32 k = 0; k < N_ZODIAC_FIELD_INTS; k++)
      if (mBits[k] != 0)
        return false;
      return true;
  }
  
  afxZodiacBitmask& operator=(const afxZodiacBitmask& field)
  {
    for (U32 k = 0; k < N_ZODIAC_FIELD_INTS; k++)
      mBits[k] = field.mBits[k];
    return *this;
  }
  
  operator bool() { return !isEmpty(); }
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_ZODIAC_DEFS_H_
