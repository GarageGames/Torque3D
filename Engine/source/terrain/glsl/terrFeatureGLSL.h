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

#ifndef _TERRFEATUREGLSL_H_
#define _TERRFEATUREGLSL_H_

#ifndef _SHADERGEN_GLSL_SHADERFEATUREGLSL_H_
#include "shaderGen/GLSL/shaderFeatureGLSL.h"
#endif
#ifndef _LANG_ELEMENT_H_
#include "shaderGen/langElement.h"
#endif


/// A shared base class for terrain features which
/// includes some helper functions.
class TerrainFeatGLSL : public ShaderFeatureGLSL
{
protected:
   ShaderIncludeDependency mTorqueDep;
   
public:
   TerrainFeatGLSL();
   Var* _getInDetailCoord(Vector<ShaderComponent*> &componentList );
   
   Var* _getInMacroCoord(Vector<ShaderComponent*> &componentList );

   Var* _getNormalMapTex();
   
   static Var* _getUniformVar( const char *name, const char *type, ConstantSortPosition csp );
   
   Var* _getDetailIdStrengthParallax();
   Var* _getMacroIdStrengthParallax();
      
};


class TerrainBaseMapFeatGLSL : public TerrainFeatGLSL
{
public:

   virtual void processVert( Vector<ShaderComponent*> &componentList,
                             const MaterialFeatureData &fd );

   virtual void processPix( Vector<ShaderComponent*> &componentList, 
                            const MaterialFeatureData &fd );
          
   virtual Resources getResources( const MaterialFeatureData &fd );

   virtual String getName() { return "Terrain Base Texture"; }

   virtual U32 getOutputTargets( const MaterialFeatureData &fd ) const;
};


class TerrainDetailMapFeatGLSL : public TerrainFeatGLSL
{
protected:

   ShaderIncludeDependency mTorqueDep;
   ShaderIncludeDependency mTerrainDep;

public:

   TerrainDetailMapFeatGLSL();

   virtual void processVert(  Vector<ShaderComponent*> &componentList,
                              const MaterialFeatureData &fd );

   virtual void processPix(   Vector<ShaderComponent*> &componentList, 
                              const MaterialFeatureData &fd );

   virtual Resources getResources( const MaterialFeatureData &fd );

   virtual String getName() { return "Terrain Detail Texture"; }

   virtual U32 getOutputTargets( const MaterialFeatureData &fd ) const;
};


class TerrainMacroMapFeatGLSL : public TerrainFeatGLSL
{
protected:

   ShaderIncludeDependency mTorqueDep;
   ShaderIncludeDependency mTerrainDep;

public:

   TerrainMacroMapFeatGLSL();

   virtual void processVert(  Vector<ShaderComponent*> &componentList,
                              const MaterialFeatureData &fd );

   virtual void processPix(   Vector<ShaderComponent*> &componentList, 
                              const MaterialFeatureData &fd );

   virtual Resources getResources( const MaterialFeatureData &fd );

   virtual String getName() { return "Terrain Macro Texture"; }

   virtual U32 getOutputTargets( const MaterialFeatureData &fd ) const;
};


class TerrainNormalMapFeatGLSL : public TerrainFeatGLSL
{
public:

   virtual void processVert(  Vector<ShaderComponent*> &componentList,
                            const MaterialFeatureData &fd );
   
   virtual void processPix(   Vector<ShaderComponent*> &componentList, 
                           const MaterialFeatureData &fd );
   
   virtual Resources getResources( const MaterialFeatureData &fd );
   
   virtual String getName() { return "Terrain Normal Texture"; }
};

class TerrainLightMapFeatGLSL : public TerrainFeatGLSL
{
public:

   virtual void processPix( Vector<ShaderComponent*> &componentList, 
                            const MaterialFeatureData &fd );
          
   virtual Resources getResources( const MaterialFeatureData &fd );

   virtual String getName() { return "Terrain Lightmap Texture"; }
};


class TerrainAdditiveFeatGLSL : public TerrainFeatGLSL
{
public:

   virtual void processPix( Vector<ShaderComponent*> &componentList, 
                            const MaterialFeatureData &fd );

   virtual String getName() { return "Terrain Additive"; }
};

class TerrainBlankInfoMapFeatGLSL : public ShaderFeatureGLSL
{
public:

   virtual void processPix(Vector<ShaderComponent*> &componentList,
      const MaterialFeatureData &fd);

   virtual U32 getOutputTargets(const MaterialFeatureData &fd) const;
   virtual String getName() { return "Blank Matinfo map"; }
};

#endif // _TERRFEATUREGLSL_H_
