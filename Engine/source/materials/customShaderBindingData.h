#pragma once

#ifndef CUSTOMSHADERBINDINGDATA_H
#define CUSTOMSHADERBINDINGDATA_H
#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif

struct CustomShaderBindingData
{
public:
	enum UniformType
	{
		Float = 0,
		Float2,
		Float3,
		Float4,
		Texture2D,
		Texture3D,
		Cubemap,
		Matrix2x2,
		Matrix2x3,
		Matrix2x4,
		Matrix3x2,
		Matrix3x3,
		Matrix3x4,
		Matrix4x2,
		Matrix4x3,
		Matrix4x4
	};
private:
	StringTableEntry targetedUniformName;

	//ShaderConstHandles shaderConstHandle;

	UniformType type;

	F32 mFloat;
	Point2F mFloat2;
	Point3F mFloat3;
	Point4F mFloat4;

	//Image stuff
	GFXTexHandle texture;
	GFXSamplerStateDesc samplerState;

public:
	void setFloat(StringTableEntry shaderConstName, F32 f)
	{
		targetedUniformName = shaderConstName;
		mFloat = f;
		type = Float;
	}
	F32 getFloat() { return mFloat; }

	void setFloat2(StringTableEntry shaderConstName, Point2F f)
	{
		targetedUniformName = shaderConstName;
		mFloat2 = f;
		type = Float2;
	}
	Point2F getFloat2() { return mFloat2; }

	void setFloat3(StringTableEntry shaderConstName, Point3F f)
	{
		targetedUniformName = shaderConstName;
		mFloat3 = f;
		type = Float3;
	}
	Point3F getFloat3() { return mFloat3; }

	void setFloat4(StringTableEntry shaderConstName, Point4F f)
	{
		targetedUniformName = shaderConstName;
		mFloat4 = f;
		type = Float4;
	}
	Point4F getFloat4() { return mFloat4; }

   void setTexture2D(StringTableEntry shaderConstName, GFXTexHandle f)
   {
      targetedUniformName = shaderConstName;
      texture = f;
      type = Texture2D;
   }
   GFXTexHandle getTexture2D() { return texture; }

	StringTableEntry getHandleName() {
		return targetedUniformName;
	}

	UniformType getType() {
		return type;
	}
};

#endif
