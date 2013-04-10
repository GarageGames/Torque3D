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

#ifndef _TERRAINACTIONS_H_
#define _TERRAINACTIONS_H_

#ifndef _TERRAINEDITOR_H_
#include "gui/worldEditor/terrainEditor.h"
#endif
#ifndef _GUIFILTERCTRL_H_
#include "gui/editor/guiFilterCtrl.h"
#endif
#ifndef _UNDO_H_
#include "util/undo.h"
#endif
#ifndef _NOISE2D_H_
#include "util/noise2d.h"
#endif

class TerrainAction
{
   protected:
      TerrainEditor *         mTerrainEditor;

   public:

      virtual ~TerrainAction(){};
      TerrainAction(TerrainEditor * editor) : mTerrainEditor(editor){}

      virtual StringTableEntry getName() = 0;

      enum Type {
         Begin = 0,
         Update,
         End,
         Process
      };

      //
      virtual void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type) = 0;
      virtual bool useMouseBrush() { return(true); }
};

//------------------------------------------------------------------------------

class SelectAction : public TerrainAction
{
   public:
      SelectAction(TerrainEditor * editor) : TerrainAction(editor){};
      StringTableEntry getName(){return("select");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

class DeselectAction : public TerrainAction
{
   public:
      DeselectAction(TerrainEditor * editor) : TerrainAction(editor){};
      StringTableEntry getName(){return("deselect");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

class ClearAction : public TerrainAction
{
   public:
      ClearAction(TerrainEditor * editor) : TerrainAction(editor){};
      StringTableEntry getName(){return("clear");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type) {};
      bool useMouseBrush() { mTerrainEditor->getCurrentSel()->reset(); return true; }
};


class SoftSelectAction : public TerrainAction
{
   public:
      SoftSelectAction(TerrainEditor * editor) : TerrainAction(editor){};
      StringTableEntry getName(){return("softSelect");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);

      Filter   mFilter;
};

//------------------------------------------------------------------------------

class OutlineSelectAction : public TerrainAction
{
   public:
      OutlineSelectAction(TerrainEditor * editor) : TerrainAction(editor){};
      StringTableEntry getName(){return("outlineSelect");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
      bool useMouseBrush() { return(false); }

   private:

      Gui3DMouseEvent   mLastEvent;
};

//------------------------------------------------------------------------------

class PaintMaterialAction : public TerrainAction
{
   public:
      PaintMaterialAction(TerrainEditor * editor) : TerrainAction(editor){}
      StringTableEntry getName(){return("paintMaterial");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

//------------------------------------------------------------------------------

class ClearMaterialsAction : public TerrainAction
{
public:
   ClearMaterialsAction(TerrainEditor * editor) : TerrainAction(editor){}
   StringTableEntry getName(){return("clearMaterials");}

   void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

//------------------------------------------------------------------------------

class RaiseHeightAction : public TerrainAction
{
   public:
      RaiseHeightAction(TerrainEditor * editor) : TerrainAction(editor){}
      StringTableEntry getName(){return("raiseHeight");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

//------------------------------------------------------------------------------

class LowerHeightAction : public TerrainAction
{
   public:
      LowerHeightAction(TerrainEditor * editor) : TerrainAction(editor){}
      StringTableEntry getName(){return("lowerHeight");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

//------------------------------------------------------------------------------

class SetHeightAction : public TerrainAction
{
   public:
      SetHeightAction(TerrainEditor * editor) : TerrainAction(editor){}
      StringTableEntry getName(){return("setHeight");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

//------------------------------------------------------------------------------

class SetEmptyAction : public TerrainAction
{
   public:
      SetEmptyAction(TerrainEditor * editor) : TerrainAction(editor){}
      StringTableEntry getName(){return("setEmpty");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

//------------------------------------------------------------------------------

class ClearEmptyAction : public TerrainAction
{
   public:
      ClearEmptyAction(TerrainEditor * editor) : TerrainAction(editor){}
      StringTableEntry getName(){return("clearEmpty");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

//------------------------------------------------------------------------------

class ScaleHeightAction : public TerrainAction
{
   public:
      ScaleHeightAction(TerrainEditor * editor) : TerrainAction(editor){}
      StringTableEntry getName(){return("scaleHeight");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

//------------------------------------------------------------------------------

class BrushAdjustHeightAction : public TerrainAction
{
   public:
      BrushAdjustHeightAction(TerrainEditor * editor) : TerrainAction(editor){}
      StringTableEntry getName(){return("brushAdjustHeight");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);

   private:
      PlaneF mIntersectionPlane;
      Point3F mTerrainUpVector;
      F32 mPreviousZ;
};

class AdjustHeightAction : public BrushAdjustHeightAction
{
   public:
      AdjustHeightAction(TerrainEditor * editor);
      StringTableEntry getName(){return("adjustHeight");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
      bool useMouseBrush() { return(false); }

   private:
      //
      Point3F                    mHitPos;
      Point3F                    mLastPos;
      SimObjectPtr<GuiCursor>    mCursor;
};

//------------------------------------------------------------------------------

class FlattenHeightAction : public TerrainAction
{
   public:
      FlattenHeightAction(TerrainEditor * editor) : TerrainAction(editor){}
      StringTableEntry getName(){return("flattenHeight");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

class SmoothHeightAction : public TerrainAction
{
   public:
      SmoothHeightAction(TerrainEditor * editor) : TerrainAction(editor){}
      StringTableEntry getName(){return("smoothHeight");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);
};

class SmoothSlopeAction : public TerrainAction  
{  
   public:  
      SmoothSlopeAction(TerrainEditor * editor) : TerrainAction(editor){}  
      StringTableEntry getName(){return("smoothSlope");}  
  
      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);  
};  

class PaintNoiseAction : public TerrainAction
{
   public:

      PaintNoiseAction( TerrainEditor *editor ) 
         :  TerrainAction( editor ),
            mNoiseSize( 256 )
      {
         mNoise.setSeed( 5342219 );
         mNoiseData.setSize( mNoiseSize * mNoiseSize );
         mNoise.fBm( &mNoiseData, mNoiseSize, 12, 1.0f, 5.0f );
         //Vector<F32> scratch = mNoiseData;
         //mNoise.rigidMultiFractal( &mNoiseData, &scratch, TerrainBlock::BlockSize, 12, 1.0f, 5.0f );
         mNoise.getMinMax( &mNoiseData, &mMinMaxNoise.x, &mMinMaxNoise.y, mNoiseSize );
       
         mScale = 1.5f / ( mMinMaxNoise.x - mMinMaxNoise.y);
      }
      
      StringTableEntry getName() { return "paintNoise"; }

      void process( Selection *sel, const Gui3DMouseEvent &event, bool selChanged, Type type );

   protected:

      const U32 mNoiseSize;

      Noise2D mNoise;

      Vector<F32> mNoiseData;

      Point2F mMinMaxNoise;

      F32 mScale;
};

/*
class ThermalErosionAction : public TerrainAction
{
   public:
      ThermalErosionAction(TerrainEditor * editor) 
      : TerrainAction(editor)
      {
         mNoise.setSeed( 1 );//Sim::getCurrentTime() );
         mNoiseData.setSize( TerrainBlock::BlockSize * TerrainBlock::BlockSize );
         mTerrainHeights.setSize( TerrainBlock::BlockSize * TerrainBlock::BlockSize );
      }
      
      StringTableEntry getName(){return("thermalErode");}

      void process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type);

      Noise2D mNoise;
      Vector<F32> mNoiseData;
      Vector<F32> mTerrainHeights;
};
*/

/// An undo action used to perform terrain wide smoothing.
class TerrainSmoothAction : public UndoAction
{
   typedef UndoAction Parent;

protected:

   SimObjectId mTerrainId;

   U32 mSteps;

   F32 mFactor;

   Vector<U16> mUnsmoothedHeights;

public:

   TerrainSmoothAction();

   // SimObject
   DECLARE_CONOBJECT( TerrainSmoothAction );
   static void initPersistFields();

   // UndoAction
   virtual void undo();
   virtual void redo();

   /// Performs the initial smoothing and stores
   /// the heighfield state for later undo.
   void smooth( TerrainBlock *terrain, F32 factor, U32 steps );
};


#endif // _TERRAINACTIONS_H_
