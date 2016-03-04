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

//------------------------------------------------------------------------------
// Mission Editor Manager
new ActionMap(EditorMap);

function mouseWheelScroll( %val )
{
   //$Camera::speedCurveTime += $Camera::scrollStepSize * ( (%val>0.0) ? 1 : -1 );
   //$Camera::speedCurveTime = mClamp( $Camera::speedCurveTime, 0.0, 1.0 );
   //calculateCameraSpeed();   
   //EditorGui-->CameraSpeedSpinner.setText( $Camera::movementSpeed );

   %rollAdj = getMouseAdjustAmount(%val);
   %rollAdj = mClamp(%rollAdj, -mPi()+0.01, mPi()-0.01);
   $mvRoll += %rollAdj;
}

function editorYaw(%val)
{
   %yawAdj = getMouseAdjustAmount(%val);

   if(ServerConnection.isControlObjectRotDampedCamera() || EWorldEditor.isMiddleMouseDown())
   {
      // Clamp and scale
      %yawAdj = mClamp(%yawAdj, -m2Pi()+0.01, m2Pi()-0.01);
      %yawAdj *= 0.5;
   }

   if( EditorSettings.value( "Camera/invertXAxis" ) )
      %yawAdj *= -1;

   $mvYaw += %yawAdj;
}

function editorPitch(%val)
{
   %pitchAdj = getMouseAdjustAmount(%val);

   if(ServerConnection.isControlObjectRotDampedCamera() || EWorldEditor.isMiddleMouseDown())
   {
      // Clamp and scale
      %pitchAdj = mClamp(%pitchAdj, -m2Pi()+0.01, m2Pi()-0.01);
      %pitchAdj *= 0.5;
   }

   if( EditorSettings.value( "Camera/invertYAxis" ) )
      %pitchAdj *= -1;

   $mvPitch += %pitchAdj;
}

function editorWheelFadeScroll( %val )
{
   EWorldEditor.fadeIconsDist += %val * 0.1;
   if( EWorldEditor.fadeIconsDist < 0 )
      EWorldEditor.fadeIconsDist = 0;
}

EditorMap.bind( mouse, xaxis, editorYaw );
EditorMap.bind( mouse, yaxis, editorPitch );
EditorMap.bind( mouse, zaxis, mouseWheelScroll );

EditorMap.bind( mouse, "alt zaxis", editorWheelFadeScroll );
