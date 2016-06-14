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

#ifndef _IDISPLAYDEVICE_H_
#define _IDISPLAYDEVICE_H_

#include "console/consoleTypes.h"

class GameConnection;
class GuiCanvas;

// Defines a custom display device that requires particular rendering settings
// in order for a scene to display correctly.

/// Defines the basic display pose common to most display devices
typedef struct DisplayPose
{
   QuatF orientation;  /// Direction device is facing
   Point3F position;    /// Relative position of device in view space

   Point3F velocity;
   Point3F angularVelocity;

#ifdef DEBUG_DISPLAY_POSE 
   MatrixF actualMatrix;
   MatrixF originalMatrix;
#endif

   U32 state; /// Generic state

   bool valid; /// Pose set
   bool connected; /// Device connected
} IDevicePose;

class IDisplayDevice
{
public:
   virtual bool providesFrameEyePose() const = 0;

	/// Get a display pose for the specified eye, or the HMD if eyeId is -1.
   virtual void getFrameEyePose(IDevicePose *pose, S32 eyeId) const = 0;

   virtual bool providesEyeOffsets() const = 0;
   /// Returns eye offset not taking into account any position tracking info
   virtual void getEyeOffsets(Point3F *dest) const = 0;

   virtual bool providesFovPorts() const = 0;
   virtual void getFovPorts(FovPort *out) const = 0;

   virtual void getStereoViewports(RectI *out) const = 0;
   virtual void getStereoTargets(GFXTextureTarget **out) const = 0;

   virtual void setDrawCanvas(GuiCanvas *canvas) = 0;
   virtual void setDrawMode(GFXDevice::GFXDeviceRenderStyles style) = 0;

   virtual void setCurrentConnection(GameConnection *connection) = 0;
   virtual GameConnection* getCurrentConnection() = 0;

   virtual void onStartFrame() = 0;

   /// Returns a texture handle representing a preview of the composited VR view
   virtual GFXTexHandle getPreviewTexture() = 0;
};

#endif   // _IDISPLAYDEVICE_H_
