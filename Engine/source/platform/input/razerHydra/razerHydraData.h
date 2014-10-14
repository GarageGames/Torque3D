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

#ifndef _RAZERHYDRADATA_H_
#define _RAZERHYDRADATA_H_

#include "console/consoleTypes.h"
#include "math/mMathFn.h"
#include "math/mMatrix.h"
#include "math/mQuat.h"
#include "sixense.h"

struct RazerHyrdaControllerData
{
   enum DataDifferences {
      DIFF_NONE            = 0,
      DIFF_POSX            = (1<<0),
      DIFF_POSY            = (1<<1),
      DIFF_POSZ            = (1<<2),
      DIFF_ROT             = (1<<3),
      DIFF_ROTAXISX        = (1<<4),
      DIFF_ROTAXISY        = (1<<5),
      DIFF_AXISX           = (1<<6),
      DIFF_AXISY           = (1<<7),
      DIFF_TRIGGER         = (1<<8),
      DIFF_BUTTON1         = (1<<9),
      DIFF_BUTTON2         = (1<<10),
      DIFF_BUTTON3         = (1<<11),
      DIFF_BUTTON4         = (1<<12),
      DIFF_BUTTON_START    = (1<<13),
      DIFF_BUTTON_SHOULDER = (1<<14),
      DIFF_BUTTON_THUMB    = (1<<15),

      DIFF_POS = (DIFF_POSX | DIFF_POSY | DIFF_POSZ),
      DIFF_AXIS = (DIFF_AXISX | DIFF_AXISY),
      DIFF_ROTAXIS = (DIFF_ROTAXISX | DIFF_ROTAXISY),
   };

   enum MetaDataDifferences {
      METADIFF_NONE        = 0,
      METADIFF_DOCKED      = (1<<0),
   };

   bool mDataSet;

   // Position
   F32 mRawPos[3];
   S32 mPos[3];
   Point3F mPosPoint;

   // Rotation
   MatrixF mRot;
   QuatF   mRotQuat;

   // Controller rotation as axis x, y
   Point2F mRotAxis;

   // Thumb stick x, y and trigger
   F32 mThumbStick[2];
   F32 mTrigger;

   // Buttons
   bool mShoulder;
   bool mThumb;
   bool mStart;
   bool mButton1;
   bool mButton2;
   bool mButton3;
   bool mButton4;

   // Other data
   bool mIsDocked;

   // Sequence number from sixense
   U32 mSequenceNum;

   RazerHyrdaControllerData();

   /// Reset controller data
   void reset();

   /// Set position based on sixense controller data
   void setData(const sixenseControllerData& data, const F32& maxAxisRadius);

   /// Compare this data and given and return differences
   U32 compare(RazerHyrdaControllerData* other);

   /// Compare meta data between this and given and return differences
   U32 compareMeta(RazerHyrdaControllerData* other);
};

#endif   // _RAZERHYDRADATA_H_
