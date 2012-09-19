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

#ifndef _GUISHAPEEDPREVIEW_H_
#define _GUISHAPEEDPREVIEW_H_

#include "gui/worldEditor/editTSCtrl.h"
#include "ts/tsShapeInstance.h"

class LightInfo;

class GuiShapeEdPreview : public EditTSCtrl
{
   struct Thread
   {
      TSThread*   key;              //!< TSThread key
      String      seqName;          //!< Name of the sequence for this thread
      S32         direction;        //!< Playback direction: -1 (reverse), 0 (paused), 1 (forward)
      bool        pingpong;         //!< Pingpong flag (auto-reverse direction at start/end)

      Thread()
      {
         init();
      }

      void init()
      {
         key = NULL;
         direction = 1.0f;
         pingpong = false;
      }
   };

   struct MountedShape
   {
      enum eMountType
      {
         Object,           //!< Mount origin of shape to target node
         Image,            //!< Mount 'mountPoint' or origin of shape to target node
         Wheel,            //!< Mount origin of shape to target node, ignore target node rotation
      };

      TSShapeInstance*     mShape;     //!< The mounted shape instance
      S32                  mNode;      //!< Index of the node this shape is mounted to
      MatrixF              mTransform; //!< Mount offset transform
      eMountType           mType;      //!< Type of mount
      Thread               mThread;    //!< Animation thread for the mounted shape

      MountedShape() : mShape(0), mNode(-1), mTransform(true), mType(Object) { }
      ~MountedShape() { delete mShape; }
   };

protected:
   typedef EditTSCtrl Parent;

   // View and node selection
   bool        mUsingAxisGizmo;
   bool        mEditingSun;         //!< True if editing the sun direction, false otherwise

   S32         mGizmoDragID;        //!< Used to track transform changes within a single gizmo drag action
   S32         mSelectedNode;       //!< Index of the selected node
   S32         mHoverNode;          //!< Index of the node the mouse is over
   Vector<Point3F> mProjectedNodes; //!< Projected screen positions of the model nodes

   S32         mSelectedObject;     //!< Name of the selected object
   S32         mSelectedObjDetail;  //!< Detail mesh index of the selected object

   // Camera
   EulerF      mCameraRot;
   bool        mRenderCameraAxes;
   Point3F     mOrbitPos;
   F32         mOrbitDist;
   F32         mMoveSpeed;
   F32         mZoomSpeed;

   // Current Detail
   bool        mFixedDetail;        //!< If false, the detail level will be determined based on camera distance
   S32         mCurrentDL;          //!< Current detail level
   S32         mDetailSize;         //!< Size of current detail level
   S32         mDetailPolys;        //!< Number of polys (triangles) in current detail level
   F32         mPixelSize;          //!< Current pixel size
   S32         mNumMaterials;       //!< Number of materials in the current detail level
   S32         mNumDrawCalls;       //!< Number of draw calls in the current detail level
   S32         mNumBones;           //!< Number of bones in the current detail level (skins only)
   S32         mNumWeights;         //!< Number of vertex weights in the current detail level (skins only)
   S32         mColMeshes;          //!< Number of collision meshes
   S32         mColPolys;           //!< Number of collision polygons (all meshes)

   // Rendering
   Point2I     mGridDimension;      //!< Dimension of grid in perspective view (eg. 30x30)
   bool        mRenderGhost;
   bool        mRenderNodes;
   bool        mRenderBounds;
   bool        mRenderObjBox;
   bool        mRenderColMeshes;
   bool        mRenderMounts;
   TSShapeInstance*  mModel;

   LightInfo*  mFakeSun;
   EulerF      mSunRot;
   ColorI      mSunDiffuseColor;
   ColorI      mSunAmbientColor;

   // Animation and playback control
   Vector<Thread> mThreads;
   F32         mTimeScale;
   S32         mActiveThread;
   S32         mLastRenderTime;

   // Mounted objects
   Vector<MountedShape*>   mMounts;

   static bool setFieldCurrentDL( void *object, const char *index, const char *data );
   static bool setFieldSunDiffuse( void *object, const char *index, const char *data );
   static bool setFieldSunAmbient( void *object, const char *index, const char *data );
   static bool setFieldSunAngleX( void *object, const char *index, const char *data );
   static bool setFieldSunAngleZ( void *object, const char *index, const char *data );

   static bool setFieldThreadPos( void *object, const char *index, const char *data );
   static bool setFieldThreadIn( void *object, const char *index, const char *data );
   static bool setFieldThreadOut( void *object, const char *index, const char *data );
   static bool setFieldThreadDir( void *object, const char *index, const char *data );
   static bool setFieldThreadPingPong( void *object, const char *index, const char *data );

   static const char *getFieldThreadPos( void *object, const char *data );
   static const char *getFieldThreadIn( void *object, const char *data );
   static const char *getFieldThreadOut( void *object, const char *data );
   static const char *getFieldThreadDir( void *object, const char *data );
   static const char *getFieldThreadPingPong( void *object, const char *data );

   // Generic mouse event handlers
   void handleMouseDown(const GuiEvent& event, GizmoMode mode);
   void handleMouseUp(const GuiEvent& event, GizmoMode mode);
   void handleMouseMove(const GuiEvent& event, GizmoMode mode);
   void handleMouseDragged(const GuiEvent& event, GizmoMode mode);

   // Node picking
   S32 collideNode(const Gui3DMouseEvent& event) const;
   void updateProjectedNodePoints();

   void updateSun();
   void updateDetailLevel(const SceneRenderState* state);
   void updateThreads(F32 delta);

   // Rendering
   void renderGrid();
   void renderNodes() const;
   void renderNodeAxes(S32 index, const ColorF& nodeColor) const;
   void renderNodeName(S32 index, const ColorF& textColor) const;
   void renderSunDirection() const;
   void renderCollisionMeshes() const;

public:
   bool onWake();

   void setDisplayType(S32 type);

   /// @name Mouse event handlers
   ///@{
   void onMouseDown(const GuiEvent& event) { handleMouseDown(event, NoneMode); }
   void onMouseUp(const GuiEvent& event) { handleMouseUp(event, NoneMode); }
   void onMouseMove(const GuiEvent& event) { handleMouseMove(event, NoneMode); }
   void onMouseDragged(const GuiEvent& event) { handleMouseDragged(event, NoneMode); }

   void onMiddleMouseDown(const GuiEvent& event) { handleMouseDown(event, MoveMode); }
   void onMiddleMouseUp(const GuiEvent& event) { handleMouseUp(event, MoveMode); }
   void onMiddleMouseDragged(const GuiEvent& event) { handleMouseDragged(event, MoveMode); }

   void onRightMouseDown(const GuiEvent& event) { handleMouseDown(event, RotateMode); }
   void onRightMouseUp(const GuiEvent& event) { handleMouseUp(event, RotateMode); }
   void onRightMouseDragged(const GuiEvent& event) { handleMouseDragged(event, RotateMode); }

   void on3DMouseWheelUp(const Gui3DMouseEvent& event);
   void on3DMouseWheelDown(const Gui3DMouseEvent& event);
   ///@}

   // Setters/Getters
   TSShapeInstance* getModel() { return mModel; }

   void setCurrentDetail(S32 dl);
   bool setObjectModel(const char * modelName);

   /// @name Threads
   ///@{
   void addThread();
   void removeThread(S32 slot);
   S32 getThreadCount() const { return mThreads.size(); }
   void setTimeScale(F32 scale);
   void setActiveThreadSequence(const char* seqName, F32 duration, F32 pos, bool play);
   void setThreadSequence(Thread& thread, TSShapeInstance* shape, const char * seqName, F32 duration=0.0f, F32 pos=0.0f, bool play=false);
   const char* getThreadSequence() const;
   void refreshThreadSequences();

   DECLARE_CALLBACK( void, onThreadPosChanged, ( F32 pos, bool inTransition ) );
   ///@}

   /// @name Mounting
   ///@{
   bool mountShape(const char* modelName, const char* nodeName, const char* mountType, S32 slot=-1);
   void setMountNode(S32 mountSlot, const char* nodeName);
   const char* getMountThreadSequence(S32 mountSlot) const;
   void setMountThreadSequence(S32 mountSlot, const char* seqName);
   F32 getMountThreadPos(S32 mountSlot) const;
   void setMountThreadPos(S32 mountSlot, F32 pos);
   F32 getMountThreadDir(S32 mountSlot) const;
   void setMountThreadDir(S32 mountSlot, F32 dir);
   void unmountShape(S32 mountSlot);
   void unmountAll();
   ///@}

   void refreshShape();
   void updateNodeTransforms();

   void get3DCursor(GuiCursor *& cursor, bool& visible, const Gui3DMouseEvent& event_);

   void fitToShape();
   void setOrbitPos( const Point3F& pos );

   void exportToCollada( const String& path );

   /// @name Rendering
   ///@{
   bool getCameraTransform(MatrixF* cameraMatrix);
   void computeSceneBounds(Box3F& bounds);

   bool getMeshHidden(const char* name) const;
   void setMeshHidden(const char* name, bool hidden);
   void setAllMeshesHidden(bool hidden);

   void renderWorld(const RectI& updateRect);
   void renderGui(Point2I offset, const RectI& updateRect);
   ///@}

   DECLARE_CONOBJECT(GuiShapeEdPreview);
   DECLARE_CATEGORY( "Gui Editor" );

   static void initPersistFields();

   GuiShapeEdPreview();
   ~GuiShapeEdPreview();
};

#endif
