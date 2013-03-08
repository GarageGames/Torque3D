//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "shaderGen/shaderGenVars.h"

const String ShaderGenVars::modelview("$modelview");
const String ShaderGenVars::worldViewOnly("$worldViewOnly");
const String ShaderGenVars::worldToCamera("$worldToCamera");
const String ShaderGenVars::worldToObj("$worldToObj");
const String ShaderGenVars::viewToObj("$viewToObj");
const String ShaderGenVars::cubeTrans("$cubeTrans");
const String ShaderGenVars::objTrans("$objTrans");
const String ShaderGenVars::cubeEyePos("$cubeEyePos");
const String ShaderGenVars::eyePos("$eyePos");
const String ShaderGenVars::eyePosWorld("$eyePosWorld");
const String ShaderGenVars::vEye("$vEye");
const String ShaderGenVars::eyeMat("$eyeMat");
const String ShaderGenVars::oneOverFarplane("$oneOverFarplane");
const String ShaderGenVars::nearPlaneWorld("$nearPlaneWorld");
const String ShaderGenVars::fogData("$fogData");
const String ShaderGenVars::fogColor("$fogColor");
const String ShaderGenVars::detailScale("$detailScale");
const String ShaderGenVars::visibility("$visibility");
const String ShaderGenVars::colorMultiply("$colorMultiply");
const String ShaderGenVars::alphaTestValue("$alphaTestValue");
const String ShaderGenVars::texMat("$texMat");
const String ShaderGenVars::accumTime("$accumTime");
const String ShaderGenVars::minnaertConstant("$minnaertConstant");
const String ShaderGenVars::subSurfaceParams("$subSurfaceParams");

const String ShaderGenVars::diffuseAtlasParams("$diffuseAtlasParams");
const String ShaderGenVars::diffuseAtlasTileParams("$diffuseAtlasTileParams");
const String ShaderGenVars::bumpAtlasParams("$bumpAtlasParams");
const String ShaderGenVars::bumpAtlasTileParams("$bumpAtlasTileParams");

const String ShaderGenVars::targetSize("$targetSize");
const String ShaderGenVars::oneOverTargetSize("$oneOverTargetSize");

const String ShaderGenVars::lightPosition("$inLightPos");
const String ShaderGenVars::lightDiffuse("$inLightColor"); 
const String ShaderGenVars::lightAmbient("$ambient");
const String ShaderGenVars::lightInvRadiusSq("$inLightInvRadiusSq");
const String ShaderGenVars::lightSpotDir("$inLightSpotDir");
const String ShaderGenVars::lightSpotFalloff("$inLightSpotFalloff");
const String ShaderGenVars::lightSpotAngle("$inLightSpotAngle");
const String ShaderGenVars::specularColor("$specularColor");
const String ShaderGenVars::specularPower("$specularPower");
const String ShaderGenVars::specularStrength("$specularStrength");		// RDM test

// These are ignored by the D3D layers.
const String ShaderGenVars::fogMap("$fogMap");
const String ShaderGenVars::dlightMap("$dlightMap");
const String ShaderGenVars::dlightMask("$dlightMask");
const String ShaderGenVars::dlightMapSec("$dlightMapSec");
const String ShaderGenVars::blackfogMap("$blackfogMap");
const String ShaderGenVars::bumpMap("$bumpMap");
const String ShaderGenVars::lightMap("$lightMap");
const String ShaderGenVars::lightNormMap("$lightNormMap");
const String ShaderGenVars::cubeMap("$cubeMap");
const String ShaderGenVars::dLightMap("$dlightMap");
const String ShaderGenVars::dLightMapSec("$dlightMapSec");
const String ShaderGenVars::dLightMask("$dlightMask");
const String ShaderGenVars::toneMap("$toneMap");