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

#ifndef _GFX_GFXDRAWER_H_
#define _GFX_GFXDRAWER_H_

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif

#ifndef _MPOLYHEDRON_H_
#include "math/mPolyhedron.h"
#endif



class FontRenderBatcher;
class Frustum;


/// Helper class containing utility functions for useful drawing routines
/// (line, box, rect, billboard, text).
class GFXDrawUtil
{
public:
   GFXDrawUtil(GFXDevice *);
   ~GFXDrawUtil();

   //-----------------------------------------------------------------------------
   // Draw Rectangles
   //-----------------------------------------------------------------------------
   void drawRect( const Point2F &upperLeft, const Point2F &lowerRight, const ColorI &color );
   void drawRect( const RectF &rect, const ColorI &color );
   void drawRect( const Point2I &upperLeft, const Point2I &lowerRight, const ColorI &color );
   void drawRect( const RectI &rect, const ColorI &color );

   void drawRectFill( const Point2F &upperL, const Point2F &lowerR, const ColorI &color );
   void drawRectFill( const RectF &rect, const ColorI &color );
   void drawRectFill( const Point2I &upperLeft, const Point2I &lowerRight, const ColorI &color );
   void drawRectFill( const RectI &rect, const ColorI &color );

   void draw2DSquare( const Point2F &screenPoint, F32 width, F32 spinAngle = 0.0f );

   //-----------------------------------------------------------------------------
   // Draw Lines
   //-----------------------------------------------------------------------------
   void drawLine( const Point3F &startPt, const Point3F &endPt, const ColorI &color );
   void drawLine( const Point2F &startPt, const Point2F &endPt, const ColorI &color );
   void drawLine( const Point2I &startPt, const Point2I &endPt, const ColorI &color );
   void drawLine( F32 x1, F32 y1, F32 x2, F32 y2, const ColorI &color );
   void drawLine( F32 x1, F32 y1, F32 z1, F32 x2, F32 y2, F32 z2, const ColorI &color );

   //-----------------------------------------------------------------------------
   // Draw Text
   //-----------------------------------------------------------------------------
   U32 drawText( GFont *font, const Point2I &ptDraw, const UTF8 *in_string, const ColorI *colorTable = NULL, const U32 maxColorIndex = 9, F32 rot = 0.f );
   U32 drawTextN( GFont *font, const Point2I &ptDraw, const UTF8 *in_string, U32 n, const ColorI *colorTable = NULL, const U32 maxColorIndex = 9, F32 rot = 0.f );
   U32 drawText( GFont *font, const Point2I &ptDraw, const UTF16 *in_string, const ColorI *colorTable = NULL, const U32 maxColorIndex = 9, F32 rot = 0.f );
   U32 drawTextN( GFont *font, const Point2I &ptDraw, const UTF16 *in_string, U32 n, const ColorI *colorTable = NULL, const U32 maxColorIndex = 9, F32 rot = 0.f );

   U32 drawText( GFont *font, const Point2F &ptDraw, const UTF8 *in_string, const ColorI *colorTable = NULL, const U32 maxColorIndex = 9, F32 rot = 0.f );
   U32 drawTextN( GFont *font, const Point2F &ptDraw, const UTF8 *in_string, U32 n, const ColorI *colorTable = NULL, const U32 maxColorIndex = 9, F32 rot = 0.f );
   U32 drawText( GFont *font, const Point2F &ptDraw, const UTF16 *in_string, const ColorI *colorTable = NULL, const U32 maxColorIndex = 9, F32 rot = 0.f );
   U32 drawTextN( GFont *font, const Point2F &ptDraw, const UTF16 *in_string, U32 n, const ColorI *colorTable = NULL, const U32 maxColorIndex = 9, F32 rot = 0.f );

   //-----------------------------------------------------------------------------
   // Color Modulation
   //-----------------------------------------------------------------------------
   void setBitmapModulation( const ColorI &modColor );
   void setTextAnchorColor( const ColorI &ancColor );
   void clearBitmapModulation();
   void getBitmapModulation( ColorI *color );

   //-----------------------------------------------------------------------------
   // Draw Bitmaps
   //-----------------------------------------------------------------------------  
   void drawBitmap( GFXTextureObject*texture, const Point2F &in_rAt, const GFXBitmapFlip in_flip = GFXBitmapFlip_None, const GFXTextureFilterType filter = GFXTextureFilterPoint , bool in_wrap = true );
   void drawBitmapSR( GFXTextureObject*texture, const Point2F &in_rAt, const RectF &srcRect, const GFXBitmapFlip in_flip = GFXBitmapFlip_None, const GFXTextureFilterType filter = GFXTextureFilterPoint , bool in_wrap = true );
   void drawBitmapStretch( GFXTextureObject*texture, const RectF &dstRect, const GFXBitmapFlip in_flip = GFXBitmapFlip_None, const GFXTextureFilterType filter = GFXTextureFilterPoint , bool in_wrap = true );
   void drawBitmapStretchSR( GFXTextureObject*texture, const RectF &dstRect, const RectF &srcRect, const GFXBitmapFlip in_flip = GFXBitmapFlip_None, const GFXTextureFilterType filter = GFXTextureFilterPoint , bool in_wrap = true );

   void drawBitmap( GFXTextureObject*texture, const Point2I &in_rAt, const GFXBitmapFlip in_flip = GFXBitmapFlip_None, const GFXTextureFilterType filter = GFXTextureFilterPoint , bool in_wrap = true );
   void drawBitmapSR( GFXTextureObject*texture, const Point2I &in_rAt, const RectI &srcRect, const GFXBitmapFlip in_flip = GFXBitmapFlip_None, const GFXTextureFilterType filter = GFXTextureFilterPoint , bool in_wrap = true );
   void drawBitmapStretch( GFXTextureObject*texture, const RectI &dstRect, const GFXBitmapFlip in_flip = GFXBitmapFlip_None, const GFXTextureFilterType filter = GFXTextureFilterPoint , bool in_wrap = true );
   void drawBitmapStretchSR( GFXTextureObject*texture, const RectI &dstRect, const RectI &srcRect, const GFXBitmapFlip in_flip = GFXBitmapFlip_None, const GFXTextureFilterType filter = GFXTextureFilterPoint , bool in_wrap = true );

   //-----------------------------------------------------------------------------
   // Draw 3D Shapes
   //-----------------------------------------------------------------------------
   void drawTriangle( const GFXStateBlockDesc &desc, const Point3F &p0, const Point3F &p1, const Point3F &p2, const ColorI &color, const MatrixF *xfm = NULL );
   void drawPolygon( const GFXStateBlockDesc& desc, const Point3F* points, U32 numPoints, const ColorI& color, const MatrixF* xfm = NULL );
   void drawCube( const GFXStateBlockDesc &desc, const Point3F &size, const Point3F &pos, const ColorI &color, const MatrixF *xfm = NULL );   
   void drawCube( const GFXStateBlockDesc &desc, const Box3F &box, const ColorI &color, const MatrixF *xfm = NULL );   
   void drawObjectBox( const GFXStateBlockDesc &desc, const Point3F &size, const Point3F &pos, const MatrixF &objMat, const ColorI &color );   
   void drawSphere( const GFXStateBlockDesc &desc, F32 radius, const Point3F &pos, const ColorI &color, bool drawTop = true, bool drawBottom = true, const MatrixF *xfm = NULL );      
   void drawCapsule( const GFXStateBlockDesc &desc, const Point3F &center, F32 radius, F32 height, const ColorI &color, const MatrixF *xfm = NULL );
   void drawCone( const GFXStateBlockDesc &desc, const Point3F &basePnt, const Point3F &tipPnt, F32 baseRadius, const ColorI &color );      
   void drawCylinder( const GFXStateBlockDesc &desc, const Point3F &basePnt, const Point3F &tipPnt, F32 baseRadius, const ColorI &color );      
   void drawArrow( const GFXStateBlockDesc &desc, const Point3F &start, const Point3F &end, const ColorI &color );
   void drawFrustum( const Frustum& f, const ColorI &color );   

   /// Draw a solid or wireframe (depending on fill mode of @a desc) polyhedron with the given color.
   ///
   /// @param desc Render state description.
   /// @param poly Polyhedron.
   /// @param color Color.
   /// @param xfm Optional matrix to transform all vertices of the given polyhedron by.
   void drawPolyhedron( const GFXStateBlockDesc &desc, const AnyPolyhedron &poly, const ColorI &color, const MatrixF *xfm = NULL );

   /// Draws a solid XY plane centered on the point with the specified dimensions.
   void drawSolidPlane( const GFXStateBlockDesc &desc, const Point3F &pos, const Point2F &size, const ColorI &color );
   
   enum Plane
   {
      PlaneXY,
      PlaneXZ,
      PlaneYZ
   };

   /// Draws a grid on XY, XZ, or YZ plane centered on the point with the specified size and step size.
   void drawPlaneGrid( const GFXStateBlockDesc &desc, const Point3F &pos, const Point2F &size, const Point2F &step, const ColorI &color, Plane plane = PlaneXY );

   /// Draws axis lines representing the passed matrix.
   /// If scale is NULL axes will be drawn the length they exist within the MatrixF.
   /// If colors is NULL the default colors are RED, GREEEN, BLUE ( x, y, z ).
   void drawTransform( const GFXStateBlockDesc &desc, const MatrixF &mat, const Point3F *scale = NULL, const ColorI colors[3] = NULL );  

protected:

   void _setupStateBlocks();
   void _drawWireTriangle( const GFXStateBlockDesc &desc, const Point3F &p0, const Point3F &p1, const Point3F &p2, const ColorI &color, const MatrixF *xfm = NULL );
   void _drawSolidTriangle( const GFXStateBlockDesc &desc, const Point3F &p0, const Point3F &p1, const Point3F &p2, const ColorI &color, const MatrixF *xfm = NULL );
   void _drawWireCube( const GFXStateBlockDesc &desc, const Point3F &size, const Point3F &pos, const ColorI &color, const MatrixF *xfm = NULL );
   void _drawSolidCube( const GFXStateBlockDesc &desc, const Point3F &size, const Point3F &pos, const ColorI &color, const MatrixF *xfm = NULL );
   void _drawWireCapsule( const GFXStateBlockDesc &desc, const Point3F &center, F32 radius, F32 height, const ColorI &color, const MatrixF *xfm = NULL );
   void _drawSolidCapsule( const GFXStateBlockDesc &desc, const Point3F &center, F32 radius, F32 height, const ColorI &color, const MatrixF *xfm = NULL );
   void _drawWirePolyhedron( const GFXStateBlockDesc &desc, const AnyPolyhedron &poly, const ColorI &color, const MatrixF *xfm = NULL );
   void _drawSolidPolyhedron( const GFXStateBlockDesc &desc, const AnyPolyhedron &poly, const ColorI &color, const MatrixF *xfm = NULL );

protected:

   /// The device we're rendering to.
   GFXDevice *mDevice;

   /// Bitmap modulation color; bitmaps are multiplied by this color when
   /// drawn.
   GFXVertexColor mBitmapModulation;

   /// Base text color; what color text is drawn at when no other color is
   /// specified.
   GFXVertexColor mTextAnchorColor;

   GFXStateBlockRef mBitmapStretchSB;
   GFXStateBlockRef mBitmapStretchLinearSB;
   GFXStateBlockRef mBitmapStretchWrapSB;
   GFXStateBlockRef mBitmapStretchWrapLinearSB;
   GFXStateBlockRef mRectFillSB;
   
   FontRenderBatcher* mFontRenderBatcher;
};

#endif // _GFX_GFXDRAWER_H_
