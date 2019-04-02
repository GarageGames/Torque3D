//-----------------------------------------------------------------------------
// 3D Action Adventure Kit for T3D
// Copyright (C) Ubiq Visuals, Inc.
// http://www.ubiqvisuals.com/
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Camera Goal Player
//-----------------------------------------------------------------------------
datablock CameraGoalPlayerData(CameraGoalPlayerDB)
{
   //shapeFile = "~/data/shapes/markers/octahedron.dts";    //use for debugging
   observeThroughObject = true; //should always be true
   
   manualTransitionTime = 250;   //transition time in/out of manual mode (ms)
   radiusDefault = 8;            //default radius from player (world units)
   radiusManual = 4;             //radius from player during manual mode (world units)
   
   //rule of thirds effect
   offCenterY = 0.5;             //screen-space Y (height) offset (world units)
   offCenterX = 0.9;             //screen-space X (width) offset (world units)
   
   pitchMax = 1.5;               //max allowed pitch above or below the horizion (radians)
   pitchMult = 0.98;             //pitch multiplier applied every tick (returns pitch to 0). Between 0 (instant level) and 1 (never level).
   pitchDetectRadius = 2.5;      //radius around player where pitch detection raycasts are performed (world units)
   
   cameraMinFov = 10;
   cameraDefaultFov = $pref::Player::DefaultFOV;
   cameraMaxFov = 135;
};

//-----------------------------------------------------------------------------
// Camera Goal Path
//-----------------------------------------------------------------------------
datablock ShapeBaseData(CameraGoalPathDB)
{
   //shapeFile = "~/data/shapes/markers/octahedron.dts";    //use for debugging
   observeThroughObject = true; //should always be true
   
   cameraMinFov = 10;
   cameraDefaultFov = $pref::Player::DefaultFOV;
   cameraMaxFov = 135;
};

//-----------------------------------------------------------------------------
// Goal Camera
//-----------------------------------------------------------------------------
datablock CameraGoalFollowerData(CameraGoalFollowerDB)
{
   //shapeFile = "~/data/shapes/markers/octahedron.dts";    //use for debugging
   rotationHistorySize = 10;   //1 = instant move, larger = more easing
   positionHistorySize = 10;   //1 = instant move, larger = more easing
   
   cameraMinFov = 10;
   cameraDefaultFov = $pref::Player::DefaultFOV;
   cameraMaxFov = 135;
};

