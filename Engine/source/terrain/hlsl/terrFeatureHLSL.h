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

#ifndef _TERRFEATUREHLSL_H_
#define _TERRFEATUREHLSL_H_

#ifndef _SHADERGEN_HLSL_SHADERFEATUREHLSL_H_
#include "shaderGen/HLSL/shaderFeatureHLSL.h"
#endif
#ifndef _LANG_ELEMENT_H_
#include "shaderGen/langElement.h"
#endif


/// A shared base class for terrain features which
/// includes some helper functions.
class TerrainFeatHLSL : public ShaderFeatureHLSL
{
protected:

   ShaderIncludeDependency mTorqueDep;

public:
   TerrainFeatHLSL();
   Var* _getInDetailCoord(Vector<ShaderComponent*> &componentList );

   Var* _getInMacroCoord(Vector<ShaderComponent*> &componentList );

   Var* _getNormalMapTex();

   static Var* _getUniformVar( const char *name, const char *type, ConstantSortPosition csp );

   Var* _getDetailIdStrengthParallax();
   Var* _getMacroIdStrengthParallax();

};


class TerrainBaseMapFeatHLSL : public TerrainFeatHLSL
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


class TerrainDetailMapFeatHLSL : public TerrainFeatHLSL
{
protected:

   ShaderIncludeDependency mTorqueDep;
   ShaderIncludeDependency mTerrainDep;

public:

   TerrainDetailMapFeatHLSL();

   virtual void processVert(  Vector<ShaderComponent*> &componentList,
                              const MaterialFeatureData &fd );

   virtual void processPix(   Vector<ShaderComponent*> &componentList, 
                              const MaterialFeatureData &fd );

   virtual Resources getResources( const MaterialFeatureData &fd );

   virtual String getName() { return "Terrain Detail Texture"; }

   virtual U32 getOutputTargets( const MaterialFeatureData &fd ) const;
};


class TerrainMacroMapFeatHLSL : public TerrainFeatHLSL
{
protected:

   ShaderIncludeDependency mTorqueDep;
   ShaderIncludeDependency mTerrainDep;

public:

   TerrainMacroMapFeatHLSL();

   virtual void processVert(  Vector<ShaderComponent*> &componentList,
                              const MaterialFeatureData &fd );

   virtual void processPix(   Vector<ShaderComponent*> &componentList, 
                              const MaterialFeatureData &fd );

   virtual Resources getResources( const MaterialFeatureData &fd );

   virtual String getName() { return "Terrain Macro Texture"; }

   virtual U32 getOutputTargets( const MaterialFeatureData &fd ) const;
};


class TerrainNormalMapFeatHLSL : public TerrainFeatHLSL
{
public:

   virtual void processVert(  Vector<ShaderComponent*> &componentList,
                              const MaterialFeatureData &fd );

   virtual void processPix(   Vector<ShaderComponent*> &componentList, 
                              const MaterialFeatureData &fd );

   virtual Resources getResources( const MaterialFeatureData &fd );

   virtual String getName() { return "Terrain Normal Texture"; }
};

class TerrainLightMapFeatHLSL : public TerrainFeatHLSL
{
public:

   virtual void processPix( Vector<ShaderComponent*> &componentList, 
                            const MaterialFeatureData &fd );
          
   virtual Resources getResources( const MaterialFeatureData &fd );

   virtual String getName() { return "Terrain Lightmap Texture"; }
};


class TerrainAdditiveFeatHLSL : public TerrainFeatHLSL
{
public:

   virtual void processPix( Vector<ShaderComponent*> &componentList, 
                            const MaterialFeatureData &fd );

   virtual String getName() { return "Terrain Additive"; }
};

class TerrainBlankInfoMapFeatHLSL : public TerrainFeatHLSL
{
public:

   virtual void processPix(Vector<ShaderComponent*> &componentList,
      const MaterialFeatureData &fd);
   
   virtual U32 getOutputTargets(const MaterialFeatureData &fd) const;
   virtual String getName() { return "Blank Matinfo map"; }
};

#endif // _TERRFEATUREHLSL_H_
