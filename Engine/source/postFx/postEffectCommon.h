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

#ifndef _POSTEFFECTCOMMON_H_
#define _POSTEFFECTCOMMON_H_

#ifndef _DYNAMIC_CONSOLETYPES_H_
   #include "console/dynamicTypes.h"
#endif


/// 
enum PFXRenderTime
{
   /// Before a RenderInstManager bin.
   PFXBeforeBin,

   /// After a RenderInstManager bin.
   PFXAfterBin,

   /// After the diffuse rendering pass.
   PFXAfterDiffuse,

   /// When the end of the frame is reached.
   PFXEndOfFrame,

   /// This PostEffect is not processed by the manager.
   /// It will generate its texture when it is requested.
   PFXTexGenOnDemand
};

DefineEnumType( PFXRenderTime );


/// PFXTargetClear specifies whether and how
/// often a given PostEffect's target will be cleared.
enum PFXTargetClear
{
   /// Never clear the PostEffect target.
   PFXTargetClear_None,

   /// Clear once on create.
   PFXTargetClear_OnCreate,

   /// Clear before every draw.
   PFXTargetClear_OnDraw,
};

DefineEnumType( PFXTargetClear );

///
struct PFXFrameState
{
   MatrixF worldToCamera;
   MatrixF cameraToScreen;

   PFXFrameState() 
      :  worldToCamera( true ),
         cameraToScreen( true )
   {
   }
};

///
GFX_DeclareTextureProfile( PostFxTextureProfile );

///
GFXDeclareVertexFormat( PFXVertex )
{
   /// xyz position.
   Point3F point;

   /// The screen space texture coord.
   Point2F texCoord;

   /// 
   Point3F wsEyeRay;
};

#endif // _POSTEFFECTCOMMON_H_