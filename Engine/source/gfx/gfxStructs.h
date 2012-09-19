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

#ifndef _GFXSTRUCTS_H_
#define _GFXSTRUCTS_H_

#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _GFXVERTEXCOLOR_H_
#include "gfx/gfxVertexColor.h"
#endif
#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h"
#endif
#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _PROFILER_H_
#include "platform/profiler.h"
#endif
#ifndef _GFXRESOURCE_H_
#include "gfx/gfxResource.h"
#endif
#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif
#ifndef _GFXVERTEXTYPES_H_
#include "gfx/gfxVertexTypes.h"
#endif


//-----------------------------------------------------------------------------
// This class is used to interact with an API's fixed function lights.  See GFX->setLight
class GFXLightInfo 
{
public:
   enum Type {
      Point    = 0,
      Spot     = 1,
      Vector   = 2,
      Ambient  = 3,
   };
   Type        mType;

   Point3F     mPos;
   VectorF     mDirection;
   ColorF      mColor;
   ColorF      mAmbient;
   F32         mRadius;
   F32         mInnerConeAngle;
   F32         mOuterConeAngle;

   /// @todo Revisit below (currently unused by fixed function lights)
	Point3F position;
	ColorF ambient;
	ColorF diffuse;
	ColorF specular;
	VectorF spotDirection;
	F32 spotExponent;
	F32 spotCutoff;
	F32 constantAttenuation;
	F32 linearAttenuation;
	F32 quadraticAttenuation;
};

//-----------------------------------------------------------------------------

// Material definition for FF lighting
struct GFXLightMaterial
{
   ColorF ambient;
   ColorF diffuse;
   ColorF specular;
   ColorF emissive;
   F32 shininess;
};

//-----------------------------------------------------------------------------

struct GFXVideoMode 
{
   GFXVideoMode();

   Point2I resolution;
   U32 bitDepth;
   U32 refreshRate;
   bool fullScreen;
   bool wideScreen;
   // When this is returned from GFX, it's the max, otherwise it's the desired AA level.
   U32 antialiasLevel;

   inline bool operator == ( const GFXVideoMode &otherMode ) const 
   {
      if( otherMode.fullScreen != fullScreen )
         return false;
      if( otherMode.resolution != resolution )
         return false;
      if( otherMode.bitDepth != bitDepth )
         return false;
      if( otherMode.refreshRate != refreshRate )
         return false;
      if( otherMode.wideScreen != wideScreen )
         return false;
      if( otherMode.antialiasLevel != antialiasLevel)
         return false;

      return true;
   }
   
   inline bool operator !=( const GFXVideoMode& otherMode ) const
   {
      return !( *this == otherMode );
   }

   /// Fill whatever fields we can from the passed string, which should be 
   /// of form "width height [bitDepth [refreshRate] [antialiasLevel]]" Unspecified fields
   /// aren't modified, so you may want to set defaults before parsing.
   void parseFromString( const char *str );

   /// Gets a string representation of the object as
   /// "resolution.x resolution.y fullScreen bitDepth refreshRate antialiasLevel"
   ///
   /// \return (string) A string representation of the object.
   const String toString() const;
};


//-----------------------------------------------------------------------------

struct GFXPrimitive
{
   GFXPrimitiveType type;

   U32 startVertex;    /// offset into vertex buffer to change where vertex[0] is
   U32 minIndex;       /// minimal value we will see in the indices
   U32 startIndex;     /// start of indices in buffer
   U32 numPrimitives;  /// how many prims to render
   U32 numVertices;    /// how many vertices... (used for locking, we lock from minIndex to minIndex + numVertices)

   GFXPrimitive()
   {
      dMemset( this, 0, sizeof( GFXPrimitive ) );
   }
};

/// Passed to GFX for shader defines.
struct GFXShaderMacro
{
   GFXShaderMacro() {}

   GFXShaderMacro( const GFXShaderMacro &macro )
      :  name( macro.name ), 
         value( macro.value ) 
      {}

   GFXShaderMacro(   const String &name_, 
                     const String &value_ = String::EmptyString )
      :  name( name_ ), 
         value( value_ ) 
      {}

   ~GFXShaderMacro() {}

   /// The macro name.
   String name;

   /// The optional macro value.
   String value;

   static void stringize( const Vector<GFXShaderMacro> &macros, String *outString );
};


#endif // _GFXSTRUCTS_H_
