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

#ifndef _SCENESPACE_H_
#define _SCENESPACE_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif


/// Abstract base class for SceneObjects that define subspaces in a scene.
///
/// Use SceneObject::containsPoint to find out whether a given space contains a particular point.
class SceneSpace : public SceneObject
{
   public:

      typedef SceneObject Parent;

   protected:

      enum
      {
         TransformMask = Parent::NextFreeMask << 0,   ///< Object transform has changed.
         NextFreeMask  = Parent::NextFreeMask << 1,
      };

      ///
      BaseMatInstance* mEditorRenderMaterial;

      ///
      virtual BaseMatInstance* _createEditorRenderMaterial();

      /// Render a visualization of the volume.
      virtual void _renderObject( ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* overrideMat );

      ///
      virtual ColorI _getDefaultEditorSolidColor() const { return ColorI( 255, 255, 255, 45 ); }
      virtual ColorI _getDefaultEditorWireframeColor() const { return ColorI::BLACK; }

   public:

      SceneSpace();
      ~SceneSpace();

      // SimObject.
      virtual bool onAdd();
      virtual void onRemove();

      // SceneObject.
      virtual void setTransform( const MatrixF &mat );
      virtual void prepRenderImage( SceneRenderState* state );

      // NetObject.
      virtual U32 packUpdate( NetConnection* connection, U32 mask, BitStream* stream );
      virtual void unpackUpdate( NetConnection* connection, BitStream* stream );

      // SimObject.
      virtual void onEditorEnable();
      virtual void onEditorDisable();
};

#endif // !_SCENESPACE_H_
