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

#ifndef _DECALDATA_H_
#define _DECALDATA_H_

#ifndef _SIMDATABLOCK_H_
#include "console/simDatablock.h"
#endif
#ifndef _MATERIALDEFINITION_H_
#include "materials/materialDefinition.h"
#endif
#ifndef _MRECT_H_
#include "math/mRect.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif

GFXDeclareVertexFormat( DecalVertex )
{
   // .xyz = coords
   Point3F point;
   Point3F normal;
   Point3F tangent;
   GFXVertexColor color;
   Point2F texCoord;   
};

/// DataBlock implementation for decals.
class DecalData : public SimDataBlock
{
      typedef SimDataBlock Parent;

   public:

      enum { MAX_TEXCOORD_COUNT = 16 };

      F32 size;
      
      /// Milliseconds for decal to expire.
      U32 lifeSpan;
      
      /// Milliseconds for decal to fade after expiration.
      U32 fadeTime;

      S32 texCoordCount;
      RectF texRect[MAX_TEXCOORD_COUNT];

      ///
      S32 frame;
      bool randomize;
      S32 texRows;
      S32 texCols;

      F32 fadeStartPixelSize;
      F32 fadeEndPixelSize;

      /// Name of material to use.
      String materialName;
      
      /// Render material for decal.
      SimObjectPtr<Material> material;
      
      /// Material instance for decal.
      BaseMatInstance *matInst;

      String lookupName;

      U8 renderPriority;
      
      S32 clippingMasks;

      /// The angle in degress used to clip geometry
      /// that faces away from the decal projection.
      F32 clippingAngle;

      /// Skip generating and collecting vertex normals for decals.
      bool skipVertexNormals;

   public:

      DecalData();
      ~DecalData();

      DECLARE_CONOBJECT(DecalData);
      static void initPersistFields();
      virtual void onStaticModified( const char *slotName, const char *newValue = NULL );
      
      virtual bool onAdd();
      virtual void onRemove();

      virtual bool preload( bool server, String &errorStr );
      virtual void packData( BitStream* );
      virtual void unpackData( BitStream* );      
      
      Material* getMaterial();
      BaseMatInstance* getMaterialInstance();

      static SimSet* getSet();
      static DecalData* findDatablock( String lookupName );

      virtual void inspectPostApply();
      void reloadRects();

   protected:

      void _initMaterial();
      void _updateMaterial();
};

inline SimSet* DecalData::getSet()
{   
   SimSet *set = NULL;
   if ( !Sim::findObject( "DecalDataSet", set ) )
   {      
      set = new SimSet;
      set->registerObject( "DecalDataSet" );
      Sim::getRootGroup()->addObject( set );
   }
   return set;
}

#endif // _DECALDATA_H_