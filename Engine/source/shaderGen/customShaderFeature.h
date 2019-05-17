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
#pragma once
#ifndef CUSTOMSHADERFEATURE_H
#define CUSTOMSHADERFEATURE_H

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

class CustomFeatureHLSL;
class CustomFeatureGLSL;

class CustomShaderFeatureData : public SimObject
{
	typedef SimObject Parent;

public:
	CustomFeatureHLSL* mFeatureHLSL;
   CustomFeatureGLSL* mFeatureGLSL;

	Vector<StringTableEntry> mAddedShaderConstants;

public:
	CustomShaderFeatureData();
	virtual ~CustomShaderFeatureData();

	// Declare this object as a ConsoleObject so that we can
	// instantiate it into the world and network it
	DECLARE_CONOBJECT(CustomShaderFeatureData);

	//--------------------------------------------------------------------------
	// Object Editing
	// Since there is always a server and a client object in Torque and we
	// actually edit the server object we need to implement some basic
	// networking functions
	//--------------------------------------------------------------------------
	// Set up any fields that we want to be editable (like position)
	static void initPersistFields();

	// Handle when we are added to the scene and removed from the scene
	bool onAdd();
	void onRemove();

	//shadergen setup
	void addVariable(String name, String type, String defaultValue);
	void addUniform(String name, String type, String defaultValue, U32 arraySize);
	void addSampler(String name, String type, U32 arraySize);
	void addTexture(String name, String type, String samplerState, U32 arraySize);
	void addConnector(String name, String type, String elementName);
   void addVertTexCoord(String name);
	bool hasFeature(String name);

	void writeLine(String format, S32 argc, ConsoleValueRef *argv);
};

#endif
