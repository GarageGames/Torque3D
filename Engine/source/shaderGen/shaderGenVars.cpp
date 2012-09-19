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
const String ShaderGenVars::lightSpotAngle("$inLightSpotAngle");
const String ShaderGenVars::lightSpotFalloff("$inLightSpotFalloff");
const String ShaderGenVars::specularColor("$specularColor");
const String ShaderGenVars::specularPower("$specularPower");

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