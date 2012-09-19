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

#ifndef _FOREST_SHADERGEN_WINDDEFORMATIONGLSL_H_
#define _FOREST_SHADERGEN_WINDDEFORMATIONGLSL_H_

#ifndef _SHADERGEN_GLSL_SHADERFEATUREGLSL_H_
#include "shaderGen/GLSL/shaderFeatureGLSL.h"
#endif
#ifndef _FEATURETYPE_H_
#include "materials/materialFeatureTypes.h"
#endif

class GFXShaderConstHandle;


class WindDeformationGLSL : public ShaderFeatureGLSL
{
protected:

   ShaderIncludeDependency mDep;

public:

   WindDeformationGLSL();

   virtual void processVert( Vector<ShaderComponent*> &componentList,
                             const MaterialFeatureData &fd );

   virtual String getName()
   {
      return "Wind Effect";
   }

   virtual void determineFeature(   Material *material,
                                    const GFXVertexFormat *vertexFormat,
                                    U32 stageNum,
                                    const FeatureType &type,
                                    const FeatureSet &features,
                                    MaterialFeatureData *outFeatureData );

   virtual ShaderFeatureConstHandles* createConstHandles( GFXShader *shader, SimObject *userObject );
};

DeclareFeatureType( MFT_WindEffect );

#endif // _FOREST_SHADERGEN_WINDDEFORMATIONGLSL_H_
