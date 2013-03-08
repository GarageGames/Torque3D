//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
#ifndef _SHADERGENVARS_H_
#define _SHADERGENVARS_H_

#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif

///
/// ShaderGenVars, predefined string names for variables that shadergen based shaders use, this avoids
/// misspelling and string creation issues
///
struct ShaderGenVars
{
   const static String modelview;
   const static String worldViewOnly;
   const static String worldToCamera;
   const static String worldToObj;
   const static String viewToObj;
   const static String cubeTrans;
   const static String objTrans;
   const static String cubeEyePos;
   const static String eyePos;
   const static String eyePosWorld;
   const static String vEye;
   const static String eyeMat;
   const static String oneOverFarplane;
   const static String nearPlaneWorld;
   const static String fogData;
   const static String fogColor;
   const static String detailScale;
   const static String visibility;
   const static String colorMultiply;
   const static String alphaTestValue;
   const static String texMat;
   const static String accumTime;
   const static String minnaertConstant;
   const static String subSurfaceParams;

   // Texture atlasing parameters
   const static String diffuseAtlasParams;
   const static String diffuseAtlasTileParams;
   const static String bumpAtlasParams;
   const static String bumpAtlasTileParams;

   // Render target parameters
   const static String targetSize;
   const static String oneOverTargetSize;

   // Lighting parameters used by the default
   // RTLighting shader feature.
   const static String lightPosition;
   const static String lightDiffuse;
   const static String lightAmbient;
   const static String lightInvRadiusSq;
   const static String lightSpotDir;
   const static String lightSpotAngle;
   const static String lightSpotFalloff;
   const static String specularColor;
   const static String specularPower;
	const static String specularStrength;		// RDM test
   
   // Textures
   const static String fogMap;
   const static String dlightMap;
   const static String dlightMask;
   const static String dlightMapSec;
   const static String blackfogMap;
   const static String bumpMap;
   const static String lightMap;
   const static String lightNormMap;
   const static String cubeMap;
   const static String dLightMap;
   const static String dLightMapSec;
   const static String dLightMask;
   const static String toneMap;
};

#endif
