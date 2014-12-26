<?php
//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

beginModule('openGL');

	addProjectDefine('TORQUE_OPENGL');

	addEngineSrcDir("shaderGen/GLSL");

	if(!isDefined("TORQUE_DEDICATED"))
	{
		addEngineSrcDir("gfx/gl");
		addEngineSrcDir("gfx/gl/tGL");
		addEngineSrcDir("shaderGen/GLSL");
		addEngineSrcDir("lighting/advanced/glsl");
		addEngineSrcDir("terrain/glsl");
		addEngineSrcDir("forest/glsl");
		addLibSrcFile("glew/src/glew.c");
	}

	if(!isDefined("TORQUE_SDL"))
	{
		addEngineSrcDir("gfx/gl/win32");
	}
    
	addIncludePath( getAppLibSrcDir() . 'glew/include/' );
    
    if ( T3D_Generator::$platform == "win32" )
    {
        addProjectLibInput('OpenGL32.lib');
        addProjectDefine('GLEW_STATIC');
    }

endModule();

?>
