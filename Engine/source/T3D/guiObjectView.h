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

#ifndef _GUIOBJECTVIEW_H_
#define _GUIOBJECTVIEW_H_

#ifndef _GUITSCONTROL_H_
   #include "gui/3d/guiTSControl.h"
#endif
#ifndef _TSSHAPEINSTANCE_H_
   #include "ts/tsShapeInstance.h"
#endif


class LightInfo;


/// A control that displays a TSShape in its view.
class GuiObjectView : public GuiTSCtrl
{
   public:
   
      typedef GuiTSCtrl Parent;

	  DECLARE_CALLBACK( void, onMouseEnter, ());
	  DECLARE_CALLBACK( void, onMouseLeave, ());

   protected:
   
      /// @name Mouse Control
      /// @{
   
      enum MouseState
      {
         None,
         Rotating,
         Zooming
      };

      /// Current mouse operation.
      MouseState  mMouseState;
      
      /// Last mouse position during tracked mouse operations.
      Point2I mLastMousePoint;

      /// @}
      
      /// @name Model
      /// @{
      
      /// Name of the model loaded for display.
      String mModelName;

      /// Model being displayed in the view.
      TSShapeInstance* mModel;

      /// Name of skin to use on model.
      String mSkinName;
      
      /// @}
      
      /// @name Camera State
      /// @{

      Point3F mCameraPos;
      MatrixF mCameraMatrix;
      EulerF mCameraRot;
      Point3F mOrbitPos;
      
      F32 mMaxOrbitDist;
      F32 mMinOrbitDist;
      EulerF mCameraRotation;
      
      ///
      F32 mOrbitDist;
      
      /// Multiplier for camera mouse operations (rotation and zooming).
      F32 mCameraSpeed;

      /// @}
      
      /// @name Mounting
      /// @{
      
      /// Name of model to mount to the primary model.
      String mMountedModelName;
      
      ///
      String mMountSkinName;

      /// Index of the node to mount the secondary model to.  -1 (disabled) by default.
      S32 mMountNode;
      
      /// Name of node to mount the secondary model to.  Unset by default.
      String mMountNodeName;

      /// Model mounted as an image to the primary model.
      TSShapeInstance* mMountedModel;
            
      ///
      MatrixF mMountTransform;
      
      /// @}
      
      /// @name Animation
      /// @{

      /// Index of the animation sequence to play on the model.  -1 (disabled) by default.
      S32 mAnimationSeq;
      
      /// Name of the animation sequence to play on the model.  Unset by default.
      String mAnimationSeqName;

      /// Animation thread on the model.
      TSThread* mRunThread;
      
      /// Last time we rendered the model.  Used for animation.
      S32 mLastRenderTime;
      
      /// @}
      
      /// @name Lighting
      /// @{
      
      /// Light object used as sun during rendering.
      LightInfo* mLight;
      
      ///
      ColorF mLightColor;
      
      ///
      ColorF mLightAmbient;
      
      ///
      Point3F mLightDirection;
      
      /// @}

      ///
      void _initAnimation();
      
      ///
      void _initMount();
      
      ///
      void onStaticModified( StringTableEntry slotName, const char* newValue );

   public:
   
      GuiObjectView();
      ~GuiObjectView();
      
      /// @name Model
      /// @{
      
      ///
      const String& getModelName() const { return mModelName; }

      /// Return the instance of the model being rendered in the view.
      TSShapeInstance* getModel() const { return mModel; }
      
      /// Return the name of the skin used on the primary model.
      const String& getSkin() const { return mSkinName; }
            
      /// Set the skin to use on the primary model.
      void setSkin( const String& name );

      /// Set the model to show in this view.
      void setObjectModel( const String& modelName );
      
      /// @}
      
      /// @name Animation
      /// @{
      
      /// Set the animation sequence to play on the model.
      /// @param seqIndex Index of sequence to play.
      void setObjectAnimation( S32 seqIndex );
            
      /// Set the animation sequence to play on the model.
      /// @param seqIndex Name of sequence to play.
      void setObjectAnimation( const String& sequenceName );
      
      /// @}
      
      /// @name Mounting
      /// @{
      
      /// Return the model mounted to the current primary model; NULL if none.
      TSShapeInstance* getMountedModel() const { return mMountedModel; }
      
      ///
      const String& getMountedModelName() const { return mMountedModelName; }
      
      /// Return the name of the skin used on the mounted model.
      const String& getMountSkin() const { return mMountSkinName; }
      
      /// Set the skin to use on the mounted model.
      void setMountSkin( const String& name );

      ///
      void setMountNode( S32 index );
      
      ///
      void setMountNode( const String& nodeName );
      
      ///
      void setMountedObject( const String& modelName );
            
      /// @}
      
      /// @name Camera
      /// @{
      
      /// Return the current camera speed multiplier.
      F32 getCameraSpeed() const { return mCameraSpeed; }
      
      /// Set the multiplier to apply to camera rotation and zooming.
      void setCameraSpeed( F32 factor );
      
      ///
      F32 getOrbitDistance() const { return mOrbitDist; }
      
      /// Sets the distance at which the camera orbits the object. Clamped to the
      /// acceptable range defined in the class by min and max orbit distances.
      ///
      /// @param distance The distance to set the orbit to (will be clamped).
      void setOrbitDistance( F32 distance );
      
      /// Sets the angle of the camera on it's orbit in relation to the object.
      void setCameraRotation( const EulerF& rotation );
      
      /// @}
      
      /// @name Lighting
      /// @{
      
      ///
      void setLightColor( const ColorF& color );
      
      ///
      void setLightAmbient( const ColorF& color );
      
      ///
      void setLightDirection( const Point3F& direction );
      
      /// @}
      
      // GuiTsCtrl.
      bool onWake();

      void onMouseEnter( const GuiEvent& event );
      void onMouseLeave( const GuiEvent& event );
      void onMouseDown( const GuiEvent& event );
      void onMouseUp( const GuiEvent& event );
      void onMouseDragged( const GuiEvent& event );
      void onRightMouseDown( const GuiEvent& event );
      void onRightMouseUp( const GuiEvent& event );
      void onRightMouseDragged( const GuiEvent& event );

      bool processCameraQuery( CameraQuery* query );
      void renderWorld( const RectI& updateRect );
      
      static void initPersistFields();

      DECLARE_CONOBJECT( GuiObjectView );
      DECLARE_DESCRIPTION( "A control that shows a TSShape model." );   
};

#endif // !_GUIOBJECTVIEW_H_
