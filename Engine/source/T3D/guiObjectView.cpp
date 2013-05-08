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

#include "T3D/guiObjectView.h"
#include "renderInstance/renderPassManager.h"
#include "lighting/lightManager.h"
#include "lighting/lightInfo.h"
#include "core/resourceManager.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "console/consoleTypes.h"
#include "math/mathTypes.h"
#include "gfx/gfxTransformSaver.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT( GuiObjectView );

ConsoleDocClass( GuiObjectView,
   "@brief GUI control which displays a 3D model.\n\n"

   "Model displayed in the control can have other objects mounted onto it, and the light settings can be adjusted.\n\n"

   "@tsexample\n"
	"	new GuiObjectView(ObjectPreview)\n"
	"	{\n"
	"		shapeFile = \"art/shapes/items/kit/healthkit.dts\";\n"
	"		mountedNode = \"mount0\";\n"
	"		lightColor = \"1 1 1 1\";\n"
	"		lightAmbient = \"0.5 0.5 0.5 1\";\n"
	"		lightDirection = \"0 0.707 -0.707\";\n"
	"		orbitDiststance = \"2\";\n"
	"		minOrbitDiststance = \"0.917688\";\n"
	"		maxOrbitDiststance = \"5\";\n"
	"		cameraSpeed = \"0.01\";\n"
	"		cameraZRot = \"0\";\n"
	"		forceFOV = \"0\";\n"
	"		reflectPriority = \"0\";\n"
	"	};\n"
   "@endtsexample\n\n"

   "@see GuiControl\n\n"

   "@ingroup Gui3D\n"
);

IMPLEMENT_CALLBACK( GuiObjectView, onMouseEnter, void, (),(),
   "@brief Called whenever the mouse enters the control.\n\n"
   "@tsexample\n"
   "// The mouse has entered the control, causing the callback to occur\n"
   "GuiObjectView::onMouseEnter(%this)\n"
   "	{\n"
   "		// Code to run when the mouse enters this control\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiObjectView, onMouseLeave, void, (),(),
   "@brief Called whenever the mouse leaves the control.\n\n"
   "@tsexample\n"
   "// The mouse has left the control, causing the callback to occur\n"
   "GuiObjectView::onMouseLeave(%this)\n"
   "	{\n"
   "		// Code to run when the mouse leaves this control\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

//------------------------------------------------------------------------------

GuiObjectView::GuiObjectView()
   :  mMaxOrbitDist( 5.0f ),
      mMinOrbitDist( 0.0f ),
      mOrbitDist( 5.0f ),
      mMouseState( None ),
      mModel( NULL ),
      mMountedModel( NULL ),
      mLastMousePoint( 0, 0 ),
      mLastRenderTime( 0 ),
      mRunThread( NULL ),
      mLight( NULL ),
      mAnimationSeq( -1 ),
      mMountNodeName( "mount0" ),
      mMountNode( -1 ),
      mCameraSpeed( 0.01f ),
      mLightColor( 1.0f, 1.0f, 1.0f ),
      mLightAmbient( 0.5f, 0.5f, 0.5f ),
      mLightDirection( 0.f, 0.707f, -0.707f )
{
   mCameraMatrix.identity();
   mCameraRot.set( 0.0f, 0.0f, 3.9f );
   mCameraPos.set( 0.0f, 1.75f, 1.25f );
   mCameraMatrix.setColumn( 3, mCameraPos );
   mOrbitPos.set( 0.0f, 0.0f, 0.0f );

   // By default don't do dynamic reflection
   // updates for this viewport.
   mReflectPriority = 0.0f;
}

//------------------------------------------------------------------------------

GuiObjectView::~GuiObjectView()
{
   if( mModel )
      SAFE_DELETE( mModel );
   if( mMountedModel )
      SAFE_DELETE( mMountedModel );
   if( mLight )
      SAFE_DELETE( mLight );
}

//------------------------------------------------------------------------------

void GuiObjectView::initPersistFields()
{
   addGroup( "Model" );
   
      addField( "shapeFile", TypeStringFilename, Offset( mModelName, GuiObjectView ),
         "The object model shape file to show in the view." );
      addField( "skin", TypeRealString, Offset( mSkinName, GuiObjectView ),
         "The skin to use on the object model." );
   
   endGroup( "Model" );
   
   addGroup( "Animation" );
   
      addField( "animSequence", TypeRealString, Offset( mAnimationSeqName, GuiObjectView ),
         "The animation sequence to play on the model." );

   endGroup( "Animation" );
   
   addGroup( "Mounting" );
   
      addField( "mountedShapeFile", TypeStringFilename, Offset( mMountedModelName, GuiObjectView ),
         "Optional shape file to mount on the primary model (e.g. weapon)." );
      addField( "mountedSkin", TypeRealString, Offset( mMountSkinName, GuiObjectView ),
         "Skin name used on mounted shape file." );
      addField( "mountedNode", TypeRealString, Offset( mMountNodeName, GuiObjectView ),
         "Name of node on primary model to which to mount the secondary shape." );
   
   endGroup( "Mounting" );
   
   addGroup( "Lighting" );
   
      addField( "lightColor", TypeColorF, Offset( mLightColor, GuiObjectView ),
         "Diffuse color of the sunlight used to render the model." );
      addField( "lightAmbient", TypeColorF, Offset( mLightAmbient, GuiObjectView ),
         "Ambient color of the sunlight used to render the model." );
      addField( "lightDirection", TypePoint3F, Offset( mLightDirection, GuiObjectView ),
         "Direction from which the model is illuminated." );
   
   endGroup( "Lighting" );
   
   addGroup( "Camera" );
   
      addField( "orbitDiststance", TypeF32, Offset( mOrbitDist, GuiObjectView ),
         "Distance from which to render the model." );
      addField( "minOrbitDiststance", TypeF32, Offset( mMinOrbitDist, GuiObjectView ),
         "Maxiumum distance to which the camera can be zoomed out." );
      addField( "maxOrbitDiststance", TypeF32, Offset( mMaxOrbitDist, GuiObjectView ),
         "Minimum distance below which the camera will not zoom in further." );
      addField( "cameraSpeed", TypeF32, Offset( mCameraSpeed, GuiObjectView ),
         "Multiplier for mouse camera operations." );
      addField( "cameraRotation", TypePoint3F, Offset( mCameraRotation, GuiObjectView ),
         "Set the camera rotation." );
   endGroup( "Camera" );
   
   Parent::initPersistFields();
}

//------------------------------------------------------------------------------

void GuiObjectView::onStaticModified( StringTableEntry slotName, const char* newValue )
{
   Parent::onStaticModified( slotName, newValue );
   
   static StringTableEntry sShapeFile = StringTable->insert( "shapeFile" );
   static StringTableEntry sSkin = StringTable->insert( "skin" );
   static StringTableEntry sMountedShapeFile = StringTable->insert( "mountedShapeFile" );
   static StringTableEntry sMountedSkin = StringTable->insert( "mountedSkin" );
   static StringTableEntry sMountedNode = StringTable->insert( "mountedNode" );
   static StringTableEntry sLightColor = StringTable->insert( "lightColor" );
   static StringTableEntry sLightAmbient = StringTable->insert( "lightAmbient" );
   static StringTableEntry sLightDirection = StringTable->insert( "lightDirection" );
   static StringTableEntry sOrbitDistance = StringTable->insert( "orbitDistance" );
   static StringTableEntry sMinOrbitDistance = StringTable->insert( "minOrbitDistance" );
   static StringTableEntry sMaxOrbitDistance = StringTable->insert( "maxOrbitDistance" );
   static StringTableEntry sCameraRotation = StringTable->insert( "cameraRotation" );
   static StringTableEntry sAnimSequence = StringTable->insert( "animSequence" );
   
   if( slotName == sShapeFile )
      setObjectModel( String( mModelName ) );
   else if( slotName == sSkin )
      setSkin( String( mSkinName ) );
   else if( slotName == sMountedShapeFile )
      setMountedObject( String( mMountedModelName ) );
   else if( slotName == sMountedSkin )
      setMountSkin( String( mMountSkinName ) );
   else if( slotName == sMountedNode )
      setMountNode( String( mMountNodeName ) );
   else if( slotName == sLightColor )
      setLightColor( mLightColor );
   else if( slotName == sLightAmbient )
      setLightAmbient( mLightAmbient );
   else if( slotName == sLightDirection )
      setLightDirection( mLightDirection );
   else if( slotName == sOrbitDistance || slotName == sMinOrbitDistance || slotName == sMaxOrbitDistance )
      setOrbitDistance( mOrbitDist );
   else if( slotName == sCameraRotation )
      setCameraRotation( mCameraRotation );
   else if( slotName == sAnimSequence )
      setObjectAnimation( String( mAnimationSeqName ) );
}

//------------------------------------------------------------------------------

bool GuiObjectView::onWake()
{
   if( !Parent::onWake() )
      return false;

   if( !mLight )
   {
      mLight = LIGHTMGR->createLightInfo();   

      mLight->setColor( mLightColor );
      mLight->setAmbient( mLightAmbient );
      mLight->setDirection( mLightDirection );
   }

   return true;
}

//------------------------------------------------------------------------------

void GuiObjectView::onMouseDown( const GuiEvent &event )
{
   if( !mActive || !mVisible || !mAwake )
      return;

   mMouseState = Rotating;
   mLastMousePoint = event.mousePoint;
   mouseLock();
}

//------------------------------------------------------------------------------

void GuiObjectView::onMouseUp( const GuiEvent &event )
{
   mouseUnlock();
   mMouseState = None;
}

//------------------------------------------------------------------------------

void GuiObjectView::onMouseDragged( const GuiEvent &event )
{
   if( mMouseState != Rotating )
      return;

   Point2I delta = event.mousePoint - mLastMousePoint;
   mLastMousePoint = event.mousePoint;

   mCameraRot.x += ( delta.y * mCameraSpeed );
   mCameraRot.z += ( delta.x * mCameraSpeed );
}

//------------------------------------------------------------------------------

void GuiObjectView::onRightMouseDown( const GuiEvent &event )
{
   mMouseState = Zooming;
   mLastMousePoint = event.mousePoint;
   mouseLock();
}

//------------------------------------------------------------------------------

void GuiObjectView::onRightMouseUp( const GuiEvent &event )
{
   mouseUnlock();
   mMouseState = None;
}

//------------------------------------------------------------------------------

void GuiObjectView::onRightMouseDragged( const GuiEvent &event )
{
   if( mMouseState != Zooming )
      return;

   S32 delta = event.mousePoint.y - mLastMousePoint.y;
   mLastMousePoint = event.mousePoint;

   mOrbitDist += ( delta * mCameraSpeed );
}

//------------------------------------------------------------------------------

void GuiObjectView::setObjectAnimation( S32 index )
{
   mAnimationSeq = index;
   mAnimationSeqName = String();
   
   if( mModel )
      _initAnimation();
}

//------------------------------------------------------------------------------

void GuiObjectView::setObjectAnimation( const String& sequenceName )
{
   mAnimationSeq = -1;
   mAnimationSeqName = sequenceName;
   
   if( mModel )
      _initAnimation();
}

//------------------------------------------------------------------------------

void GuiObjectView::setObjectModel( const String& modelName )
{
   SAFE_DELETE( mModel );
   mRunThread = 0;
   mModelName = String::EmptyString;
   
   // Load the shape.

   Resource< TSShape > model = ResourceManager::get().load( modelName );
   if( !model )
   {
      Con::warnf( "GuiObjectView::setObjectModel - Failed to load model '%s'", modelName.c_str() );
      return;
   }
   
   // Instantiate it.

   mModel = new TSShapeInstance( model, true );
   mModelName = modelName;
   
   if( !mSkinName.isEmpty() )
      mModel->reSkin( mSkinName );

   // Initialize camera values.
   
   mOrbitPos = mModel->getShape()->center;
   mMinOrbitDist = mModel->getShape()->radius;

   // Initialize animation.
   
   _initAnimation();
   _initMount();
}

//------------------------------------------------------------------------------

void GuiObjectView::setSkin( const String& name )
{
   if( mModel )
      mModel->reSkin( name, mSkinName );
      
   mSkinName = name;
}

//------------------------------------------------------------------------------

void GuiObjectView::setMountSkin( const String& name )
{
   if( mMountedModel )
      mMountedModel->reSkin( name, mMountSkinName );
      
   mMountSkinName = name;
}

//------------------------------------------------------------------------------

void GuiObjectView::setMountNode( S32 index )
{
   setMountNode( String::ToString( "mount%i", index ) );
}

//------------------------------------------------------------------------------

void GuiObjectView::setMountNode( const String& name )
{
   mMountNodeName = name;
   
   if( mModel )
      _initMount();
}

//------------------------------------------------------------------------------

void GuiObjectView::setMountedObject( const String& modelName )
{
   SAFE_DELETE( mMountedModel );
   mMountedModelName = String::EmptyString;

   // Load the model.
   
   Resource< TSShape > model = ResourceManager::get().load( modelName );
   if( !model )
   {
      Con::warnf( "GuiObjectView::setMountedObject -  Failed to load object model '%s'",
         modelName.c_str() );
      return;
   }

   mMountedModel = new TSShapeInstance( model, true );
   mMountedModelName = modelName;
   
   if( !mMountSkinName.isEmpty() )
      mMountedModel->reSkin( mMountSkinName );
   
   if( mModel )
      _initMount();
}

//------------------------------------------------------------------------------

bool GuiObjectView::processCameraQuery( CameraQuery* query )
{
   // Adjust the camera so that we are still facing the model.
   
   Point3F vec;
   MatrixF xRot, zRot;
   xRot.set( EulerF( mCameraRot.x, 0.0f, 0.0f ) );
   zRot.set( EulerF( 0.0f, 0.0f, mCameraRot.z ) );

   mCameraMatrix.mul( zRot, xRot );
   mCameraMatrix.getColumn( 1, &vec );
   vec *= mOrbitDist;
   mCameraPos = mOrbitPos - vec;

   query->farPlane = 2100.0f;
   query->nearPlane = query->farPlane / 5000.0f;
   query->fov = 45.0f;
   mCameraMatrix.setColumn( 3, mCameraPos );
   query->cameraMatrix = mCameraMatrix;

   return true;
}

//------------------------------------------------------------------------------

void GuiObjectView::onMouseEnter( const GuiEvent & event )
{
   onMouseEnter_callback();
}

//------------------------------------------------------------------------------

void GuiObjectView::onMouseLeave( const GuiEvent & event )
{
   onMouseLeave_callback();
}

//------------------------------------------------------------------------------

void GuiObjectView::renderWorld( const RectI& updateRect )
{
   if( !mModel )
      return;
      
   GFXTransformSaver _saveTransforms;

   // Determine the camera position, and store off render state.
   
   MatrixF modelview;
   MatrixF mv;
   Point3F cp;

   modelview = GFX->getWorldMatrix();

   mv = modelview;
   mv.inverse();
   mv.getColumn( 3, &cp );

   RenderPassManager* renderPass = gClientSceneGraph->getDefaultRenderPass();

   S32 time = Platform::getVirtualMilliseconds();
   S32 dt = time - mLastRenderTime;
   mLastRenderTime = time;

   LIGHTMGR->unregisterAllLights();
   LIGHTMGR->setSpecialLight( LightManager::slSunLightType, mLight );
  
   GFX->setStateBlock( mDefaultGuiSB );

   F32 left, right, top, bottom, nearPlane, farPlane;
   bool isOrtho;
   GFX->getFrustum( &left, &right, &bottom, &top, &nearPlane, &farPlane, &isOrtho );

   Frustum frust( false, left, right, top, bottom, nearPlane, farPlane, MatrixF::Identity );

   SceneRenderState state
   (
      gClientSceneGraph,
      SPT_Diffuse,
      SceneCameraState( GFX->getViewport(), frust, GFX->getWorldMatrix(), GFX->getProjectionMatrix() ),
      renderPass,
      false
   );

   // Set up our TS render state here.   
   TSRenderState rdata;
   rdata.setSceneState( &state );

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init( SphereF( Point3F::Zero, 1.0f ) );
   rdata.setLightQuery( &query );

   // Render primary model.

   if( mModel )
   {
      if( mRunThread )
      {
         mModel->advanceTime( dt / 1000.f, mRunThread );
         mModel->animate();
      }
      
      mModel->render( rdata );
   }
   
   // Render mounted model.

   if( mMountedModel && mMountNode != -1 )
   {
      GFX->pushWorldMatrix();
      GFX->multWorld( mModel->mNodeTransforms[ mMountNode ] );
      GFX->multWorld( mMountTransform );
      
      mMountedModel->render( rdata );

      GFX->popWorldMatrix();
   }

   renderPass->renderPass( &state );

   // Make sure to remove our fake sun.
   LIGHTMGR->unregisterAllLights();
}

//------------------------------------------------------------------------------

void GuiObjectView::setOrbitDistance( F32 distance )
{
   // Make sure the orbit distance is within the acceptable range
   mOrbitDist = mClampF( distance, mMinOrbitDist, mMaxOrbitDist );
}

//------------------------------------------------------------------------------

void GuiObjectView::setCameraSpeed( F32 factor )
{
   mCameraSpeed = factor;
}

//------------------------------------------------------------------------------

void GuiObjectView::setCameraRotation( const EulerF& rotation )
{
    mCameraRot.set(rotation);
}

//------------------------------------------------------------------------------
void GuiObjectView::setLightColor( const ColorF& color )
{
   mLightColor = color;
   if( mLight )
      mLight->setColor( color );
}

//------------------------------------------------------------------------------

void GuiObjectView::setLightAmbient( const ColorF& color )
{
   mLightAmbient = color;
   if( mLight )
      mLight->setAmbient( color );
}

//------------------------------------------------------------------------------

void GuiObjectView::setLightDirection( const Point3F& direction )
{
   mLightDirection = direction;
   if( mLight )
      mLight->setDirection( direction );
}

//------------------------------------------------------------------------------

void GuiObjectView::_initAnimation()
{
   AssertFatal( mModel, "GuiObjectView::_initAnimation - No model loaded!" );
   
   if( mAnimationSeqName.isEmpty() && mAnimationSeq == -1 )
      return;
      
   // Look up sequence by name.
            
   if( !mAnimationSeqName.isEmpty() )
   {
      mAnimationSeq = mModel->getShape()->findSequence( mAnimationSeqName );
      
      if( mAnimationSeq == -1 )
      {
         Con::errorf( "GuiObjectView::_initAnimation - Cannot find animation sequence '%s' on '%s'",
            mAnimationSeqName.c_str(),
            mModelName.c_str()
         );
         
         return;
      }
   }
   
   // Start sequence.
      
   if( mAnimationSeq != -1 )
   {
      if( mAnimationSeq >= mModel->getShape()->sequences.size() )
      {
         Con::errorf( "GuiObjectView::_initAnimation - Sequence '%i' out of range for model '%s'",
            mAnimationSeq,
            mModelName.c_str()
         );
         
         mAnimationSeq = -1;
         return;
      }
      
      if( !mRunThread )
         mRunThread = mModel->addThread();
         
      mModel->setSequence( mRunThread, mAnimationSeq, 0.f );
   }
   
   mLastRenderTime = Platform::getVirtualMilliseconds();
}

//------------------------------------------------------------------------------

void GuiObjectView::_initMount()
{
   AssertFatal( mModel, "GuiObjectView::_initMount - No model loaded!" );
      
   if( !mMountedModel )
      return;
      
   mMountTransform.identity();

   // Look up the node to which to mount to.
   
   if( !mMountNodeName.isEmpty() )
   {
      mMountNode = mModel->getShape()->findNode( mMountNodeName );
      if( mMountNode == -1 )
      {
         Con::errorf( "GuiObjectView::_initMount - No node '%s' on '%s'",
            mMountNodeName.c_str(),
            mModelName.c_str()
         );
         
         return;
      }
   }
   
   // Make sure mount node is valid.
   
   if( mMountNode != -1 && mMountNode >= mModel->getShape()->nodes.size() )
   {
      Con::errorf( "GuiObjectView::_initMount - Mount node index '%i' out of range for '%s'",
         mMountNode,
         mModelName.c_str()
      );
      
      mMountNode = -1;
      return;
   }
   
   // Look up node on the mounted model from
   // which to mount to the primary model's node.

   S32 mountPoint = mMountedModel->getShape()->findNode( "mountPoint" );
   if( mountPoint != -1 )
   {
      mMountedModel->getShape()->getNodeWorldTransform( mountPoint, &mMountTransform ),
      mMountTransform.inverse();
   }
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, getModel, const char*, (),,
   "@brief Return the model displayed in this view.\n\n"
   "@tsexample\n"
   "// Request the displayed model name from the GuiObjectView object.\n"
   "%modelName = %thisGuiObjectView.getModel();\n"
   "@endtsexample\n\n"
   "@return Name of the displayed model.\n\n"
   "@see GuiControl")
{
   return Con::getReturnBuffer( object->getModelName() );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, setModel, void, (const char* shapeName),,
   "@brief Sets the model to be displayed in this control.\n\n"
   "@param shapeName Name of the model to display.\n"
   "@tsexample\n"
   "// Define the model we want to display\n"
   "%shapeName = \"gideon.dts\";\n\n"
   "// Tell the GuiObjectView object to display the defined model\n"
   "%thisGuiObjectView.setModel(%shapeName);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setObjectModel( shapeName );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, getMountedModel, const char*, (),,
   "@brief Return the name of the mounted model.\n\n"
   "@tsexample\n"
   "// Request the name of the mounted model from the GuiObjectView object\n"
   "%mountedModelName = %thisGuiObjectView.getMountedModel();\n"
   "@endtsexample\n\n"
   "@return Name of the mounted model.\n\n"
   "@see GuiControl")
{
   return Con::getReturnBuffer( object->getMountedModelName() );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, setMountedModel, void, (const char* shapeName),,
   "@brief Sets the model to be mounted on the primary model.\n\n"
   "@param shapeName Name of the model to mount.\n"
   "@tsexample\n"
   "// Define the model name to mount\n"
   "%modelToMount = \"GideonGlasses.dts\";\n\n"
   "// Inform the GuiObjectView object to mount the defined model to the existing model in the control\n"
   "%thisGuiObjectView.setMountedModel(%modelToMount);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setObjectModel(shapeName);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, getSkin, const char*, (),,
   "@brief Return the name of skin used on the primary model.\n\n"
   "@tsexample\n"
   "// Request the name of the skin used on the primary model in the control\n"
   "%skinName = %thisGuiObjectView.getSkin();\n"
   "@endtsexample\n\n"
   "@return Name of the skin used on the primary model.\n\n"
   "@see GuiControl")
{
   return Con::getReturnBuffer( object->getSkin() );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, setSkin, void, (const char* skinName),,
   "@brief Sets the skin to use on the model being displayed.\n\n"
   "@param skinName Name of the skin to use.\n"
   "@tsexample\n"
   "// Define the skin we want to apply to the main model in the control\n"
   "%skinName = \"disco_gideon\";\n\n"
   "// Inform the GuiObjectView control to update the skin the to defined skin\n"
   "%thisGuiObjectView.setSkin(%skinName);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setSkin( skinName );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, getMountSkin, const char*, ( S32 param1, S32 param2),,
   "@brief Return the name of skin used on the mounted model.\n\n"
   "@tsexample\n"
   "// Request the skin name from the model mounted on to the main model in the control\n"
   "%mountModelSkin = %thisGuiObjectView.getMountSkin();\n"
   "@endtsexample\n\n"
   "@return Name of the skin used on the mounted model.\n\n"
   "@see GuiControl")
{
   return Con::getReturnBuffer( object->getMountSkin() );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, setMountSkin, void, (const char* skinName),,
   "@brief Sets the skin to use on the mounted model.\n\n"
   "@param skinName Name of the skin to set on the model mounted to the main model in the control\n"
   "@tsexample\n"
   "// Define the name of the skin\n"
   "%skinName = \"BronzeGlasses\";\n\n"
   "// Inform the GuiObjectView Control of the skin to use on the mounted model\n"
   "%thisGuiObjectViewCtrl.setMountSkin(%skinName);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setMountSkin(skinName);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, setSeq, void, (const char* indexOrName),,
   "@brief Sets the animation to play for the viewed object.\n\n"
   "@param indexOrName The index or name of the animation to play.\n"
   "@tsexample\n"
   "// Set the animation index value, or animation sequence name.\n"
   "%indexVal = \"3\";\n"
   "//OR:\n"
   "%indexVal = \"idle\";\n\n"
   "// Inform the GuiObjectView object to set the animation sequence of the object in the control.\n"
   "%thisGuiObjectVew.setSeq(%indexVal);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   if( dIsdigit( indexOrName[0] ) )
      object->setObjectAnimation( dAtoi( indexOrName ) );
   else
      object->setObjectAnimation( indexOrName );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, setMount, void, ( const char* shapeName, const char* mountNodeIndexOrName),,
   "@brief Mounts the given model to the specified mount point of the primary model displayed in this control.\n\n"
   "Detailed description\n\n"
   "@param shapeName Name of the model to mount.\n"
   "@param mountNodeIndexOrName Index or name of the mount point to be mounted to. If index, corresponds to \"mountN\" in your shape where N is the number passed here.\n"
   "@tsexample\n"
   "// Set the shapeName to mount\n"
   "%shapeName = \"GideonGlasses.dts\"\n\n"
   "// Set the mount node of the primary model in the control to mount the new shape at\n"
   "%mountNodeIndexOrName = \"3\";\n"
   "//OR:\n"
   "%mountNodeIndexOrName = \"Face\";\n\n"
   "// Inform the GuiObjectView object to mount the shape at the specified node.\n"
   "%thisGuiObjectView.setMount(%shapeName,%mountNodeIndexOrName);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   if( dIsdigit( mountNodeIndexOrName[0] ) )
      object->setMountNode( dAtoi( mountNodeIndexOrName ) );
   else
      object->setMountNode( mountNodeIndexOrName );
      
   object->setMountedObject( shapeName );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, getOrbitDistance, F32, (),,
   "@brief Return the current distance at which the camera orbits the object.\n\n"
   "@tsexample\n"
   "// Request the current orbit distance\n"
   "%orbitDistance = %thisGuiObjectView.getOrbitDistance();\n"
   "@endtsexample\n\n"
   "@return The distance at which the camera orbits the object.\n\n"
   "@see GuiControl")
{
   return object->getOrbitDistance();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, setOrbitDistance, void, (F32 distance),,
   "@brief Sets the distance at which the camera orbits the object. Clamped to the acceptable range defined in the class by min and max orbit distances.\n\n"
   "Detailed description\n\n"
   "@param distance The distance to set the orbit to (will be clamped).\n"
   "@tsexample\n"
   "// Define the orbit distance value\n"
   "%orbitDistance = \"1.5\";\n\n"
   "// Inform the GuiObjectView object to set the orbit distance to the defined value\n"
   "%thisGuiObjectView.setOrbitDistance(%orbitDistance);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setOrbitDistance( distance );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, getCameraSpeed, F32, (),,
   "@brief Return the current multiplier for camera zooming and rotation.\n\n"
   "@tsexample\n"
   "// Request the current camera zooming and rotation multiplier value\n"
   "%multiplier = %thisGuiObjectView.getCameraSpeed();\n"
   "@endtsexample\n\n"
   "@return Camera zooming / rotation multiplier value.\n\n"
   "@see GuiControl")
{
   return object->getCameraSpeed();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, setCameraSpeed, void, (F32 factor),,
   "@brief Sets the multiplier for the camera rotation and zoom speed.\n\n"
   "@param factor Multiplier for camera rotation and zoom speed.\n"
   "@tsexample\n"
   "// Set the factor value\n"
   "%factor = \"0.75\";\n\n"
   "// Inform the GuiObjectView object to set the camera speed.\n"
   "%thisGuiObjectView.setCameraSpeed(%factor);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setCameraSpeed( factor );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, setLightColor, void, ( ColorF color),,
   "@brief Set the light color on the sun object used to render the model.\n\n"
   "@param color Color of sunlight.\n"
   "@tsexample\n"
   "// Set the color value for the sun\n"
   "%color = \"1.0 0.4 0.5\";\n\n"
   "// Inform the GuiObjectView object to change the sun color to the defined value\n"
   "%thisGuiObjectView.setLightColor(%color);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{  
   object->setLightColor( color );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, setLightAmbient, void, (ColorF color),,
   "@brief Set the light ambient color on the sun object used to render the model.\n\n"
   "@param color Ambient color of sunlight.\n"
   "@tsexample\n"
   "// Define the sun ambient color value\n"
   "%color = \"1.0 0.4 0.6\";\n\n"
   "// Inform the GuiObjectView object to set the sun ambient color to the requested value\n"
   "%thisGuiObjectView.setLightAmbient(%color);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setLightAmbient( color );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiObjectView, setLightDirection, void, (Point3F direction),,
   "@brief Set the light direction from which to light the model.\n\n"
   "@param direction XYZ direction from which the light will shine on the model\n"
   "@tsexample\n"
   "// Set the light direction\n"
   "%direction = \"1.0 0.2 0.4\"\n\n"
   "// Inform the GuiObjectView object to change the light direction to the defined value\n"
   "%thisGuiObjectView.setLightDirection(%direction);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setLightDirection( direction );
}
