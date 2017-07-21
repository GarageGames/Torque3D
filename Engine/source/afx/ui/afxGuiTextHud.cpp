
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

#include "afx/arcaneFX.h"

#include "gui/3d/guiTSControl.h"
#include "gfx/gfxDrawUtil.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/shapeBase.h"

#include "afx/ui/afxGuiTextHud.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

Vector<afxGuiTextHud::HudTextSpec>  afxGuiTextHud::text_items;

IMPLEMENT_CONOBJECT(afxGuiTextHud);

ConsoleDocClass( afxGuiTextHud,
   "@brief Similar to GuiShapeNameHud, afxGuiTextHud displays ShapeBase object names but "
   "also allows Gui Text effects.\n\n"
   
   "@ingroup GuiGame\n"
);

afxGuiTextHud::afxGuiTextHud()
{
   mFillColor.set( 0.25f, 0.25f, 0.25f, 0.25f );
   mFrameColor.set( 0, 0, 0, 1 );
   mTextColor.set( 0, 1, 0, 1 );
   mShowFill = false;
   mShowFrame = true;
   mVerticalOffset = 0.5f;
   mDistanceFade = 0.1f;
   mLabelAllShapes = false;
   mEnableControlObjectOcclusion = false;
}

void afxGuiTextHud::initPersistFields()
{
   addGroup("Colors");
   addField( "fillColor",  TypeColorF, Offset( mFillColor, afxGuiTextHud ),
    "...");
   addField( "frameColor", TypeColorF, Offset( mFrameColor, afxGuiTextHud ),
    "...");
   addField( "textColor",  TypeColorF, Offset( mTextColor, afxGuiTextHud ),
    "...");
   endGroup("Colors");

   addGroup("Misc");
   addField( "showFill",   TypeBool, Offset( mShowFill, afxGuiTextHud ),
    "...");
   addField( "showFrame",  TypeBool, Offset( mShowFrame, afxGuiTextHud ),
    "...");
   addField( "verticalOffset", TypeF32, Offset( mVerticalOffset, afxGuiTextHud  ),
    "...");
   addField( "distanceFade", TypeF32, Offset( mDistanceFade, afxGuiTextHud ),
    "...");
   addField( "labelAllShapes",  TypeBool, Offset( mLabelAllShapes, afxGuiTextHud ),
    "...");
   addField( "enableControlObjectOcclusion",  TypeBool, Offset( mEnableControlObjectOcclusion, afxGuiTextHud ),
    "...");
   endGroup("Misc");

   Parent::initPersistFields();
}

//----------------------------------------------------------------------------
/// Core rendering method for this control.
///
/// This method scans through all the current client ShapeBase objects.
/// If one is named, it displays the name and damage information for it.
///
/// Information is offset from the center of the object's bounding box,
/// unless the object is a PlayerObjectType, in which case the eye point
/// is used.
///
/// @param   updateRect   Extents of control.
void afxGuiTextHud::onRender( Point2I, const RectI &updateRect)
{
   // Background fill first
   if (mShowFill)
      GFX->getDrawUtil()->drawRectFill(updateRect, mFillColor);

   // Must be in a TS Control
   GuiTSCtrl *parent = dynamic_cast<GuiTSCtrl*>(getParent());
   if (!parent) return;

   // Must have a connection and control object
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn)
      return;

   GameBase * control = dynamic_cast<GameBase*>(conn->getControlObject());
   if (!control)
      return;

   // Get control camera info
   MatrixF cam;
   Point3F camPos;
   VectorF camDir;
   conn->getControlCameraTransform(0,&cam);
   cam.getColumn(3, &camPos);
   cam.getColumn(1, &camDir);

   F32 camFovCos;
   conn->getControlCameraFov(&camFovCos);
   camFovCos = mCos(mDegToRad(camFovCos) / 2);

   // Visible distance info & name fading
   F32 visDistance = gClientSceneGraph->getVisibleDistance();
   F32 visDistanceSqr = visDistance * visDistance;
   F32 fadeDistance = visDistance * mDistanceFade;

   // Collision info. We're going to be running LOS tests and we
   // don't want to collide with the control object.
   static U32 losMask = TerrainObjectType | TerrainLikeObjectType | ShapeBaseObjectType;

   if (!mEnableControlObjectOcclusion)
      control->disableCollision();

   if (mLabelAllShapes)
   {
     // This section works just like GuiShapeNameHud and renders labels for
     // all the shapes.

     // All ghosted objects are added to the server connection group,
     // so we can find all the shape base objects by iterating through
     // our current connection.
     for (SimSetIterator itr(conn); *itr; ++itr) 
     {
       ///if ((*itr)->getTypeMask() & ShapeBaseObjectType) 
       ///{
       ShapeBase* shape = dynamic_cast<ShapeBase*>(*itr);
       if ( shape ) {
         if (shape != control && shape->getShapeName()) 
         {

           // Target pos to test, if it's a player run the LOS to his eye
           // point, otherwise we'll grab the generic box center.
           Point3F shapePos;
           if (shape->getTypeMask() & PlayerObjectType) 
           {
             MatrixF eye;

             // Use the render eye transform, otherwise we'll see jittering
             shape->getRenderEyeTransform(&eye);
             eye.getColumn(3, &shapePos);
           }
           else 
           {
             // Use the render transform instead of the box center
             // otherwise it'll jitter.
             MatrixF srtMat = shape->getRenderTransform();
             srtMat.getColumn(3, &shapePos);
           }

           VectorF shapeDir = shapePos - camPos;

           // Test to see if it's in range
           F32 shapeDist = shapeDir.lenSquared();
           if (shapeDist == 0 || shapeDist > visDistanceSqr)
             continue;
           shapeDist = mSqrt(shapeDist);

           // Test to see if it's within our viewcone, this test doesn't
           // actually match the viewport very well, should consider
           // projection and box test.
           shapeDir.normalize();
           F32 dot = mDot(shapeDir, camDir);
           if (dot < camFovCos)
             continue;

           // Test to see if it's behind something, and we want to
           // ignore anything it's mounted on when we run the LOS.
           RayInfo info;
           shape->disableCollision();
           SceneObject *mount = shape->getObjectMount();
           if (mount)
             mount->disableCollision();
           bool los = !gClientContainer.castRay(camPos, shapePos,losMask, &info);
           shape->enableCollision();
           if (mount)
             mount->enableCollision();

           if (!los)
             continue;

           // Project the shape pos into screen space and calculate
           // the distance opacity used to fade the labels into the
           // distance.
           Point3F projPnt;
           shapePos.z += mVerticalOffset;
           if (!parent->project(shapePos, &projPnt))
             continue;
           F32 opacity = (shapeDist < fadeDistance)? 1.0:
             1.0 - (shapeDist - fadeDistance) / (visDistance - fadeDistance);

           // Render the shape's name
           drawName(Point2I((S32)projPnt.x, (S32)projPnt.y),shape->getShapeName(),opacity);
         }
       }
     }
   }

   // This section renders all text added by afxGuiText effects.
   for (S32 i = 0; i < text_items.size(); i++)
   {
     HudTextSpec* spec = &text_items[i];
     if (spec->text && spec->text[0] != '\0') 
     {
       VectorF shapeDir = spec->pos - camPos;

       // do range test
       F32 shapeDist = shapeDir.lenSquared();
       if (shapeDist == 0 || shapeDist > visDistanceSqr)
         continue;
       shapeDist = mSqrt(shapeDist);

       // Test to see if it's within our viewcone, this test doesn't
       // actually match the viewport very well, should consider
       // projection and box test.
       shapeDir.normalize();
       F32 dot = mDot(shapeDir, camDir);
       if (dot < camFovCos)
         continue;

       // Test to see if it's behind something, and we want to
       // ignore anything it's mounted on when we run the LOS.
       RayInfo info;
       if (spec->obj)
         spec->obj->disableCollision();
       bool los = !gClientContainer.castRay(camPos, spec->pos, losMask, &info);
       if (spec->obj)
         spec->obj->enableCollision();
       if (!los)
         continue;

       // Project the shape pos into screen space.
       Point3F projPnt;
       if (!parent->project(spec->pos, &projPnt))
         continue;

       // Calculate the distance opacity used to fade text into the distance.
       F32 opacity = (shapeDist < fadeDistance)? 1.0 : 1.0 - (shapeDist - fadeDistance) / (25.0f);
       if (opacity > 0.01f)
        drawName(Point2I((S32)projPnt.x, (S32)projPnt.y), spec->text, opacity, &spec->text_clr);
     }
   }

   // Restore control object collision
   if (!mEnableControlObjectOcclusion)
      control->enableCollision();

   // Border last
   if (mShowFrame)
      GFX->getDrawUtil()->drawRect(updateRect, mFrameColor);

   reset();
}

//----------------------------------------------------------------------------
/// Render object names.
///
/// Helper function for GuiShapeNameHud::onRender
///
/// @param   offset  Screen coordinates to render name label. (Text is centered
///                  horizontally about this location, with bottom of text at
///                  specified y position.)
/// @param   name    String name to display.
/// @param   opacity Opacity of name (a fraction).
void afxGuiTextHud::drawName(Point2I offset, const char *name, F32 opacity, ColorF* color)
{
   // Center the name
   offset.x -= mProfile->mFont->getStrWidth((const UTF8 *)name) / 2;
   offset.y -= mProfile->mFont->getHeight();

   // Deal with opacity and draw.
   ColorF draw_color = (color) ? *color : mTextColor;
   draw_color.alpha *= opacity;
   GFX->getDrawUtil()->setBitmapModulation(draw_color);
   GFX->getDrawUtil()->drawText(mProfile->mFont, offset, name);
   GFX->getDrawUtil()->clearBitmapModulation();
}

void afxGuiTextHud::addTextItem(const Point3F& pos, const char* text, ColorF& color, SceneObject* obj)
{
  if (!text || text[0] == '\0')
    return;

  HudTextSpec spec;
  spec.pos = pos;
  spec.text = text;
  spec.text_clr = color;
  spec.obj = obj;

  text_items.push_back(spec);
}

void afxGuiTextHud::reset()
{
  text_items.clear();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

