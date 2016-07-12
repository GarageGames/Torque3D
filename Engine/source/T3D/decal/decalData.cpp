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
#include "T3D/decal/decalData.h"

#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "materials/materialManager.h"
#include "materials/baseMatInstance.h"
#include "T3D/objectTypes.h"
#include "console/engineAPI.h"

GFXImplementVertexFormat( DecalVertex )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "TANGENT", GFXDeclType_Float3 );   
   addElement( "COLOR", GFXDeclType_Color );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );   
}

IMPLEMENT_CO_DATABLOCK_V1( DecalData );

ConsoleDocClass( DecalData,
   "@brief A datablock describing an individual decal.\n\n"

   "The textures defined by the decal Material can be divided into multiple "
   "rectangular sub-textures as shown below, with a different sub-texture "
   "selected by all decals using the same DecalData (via #frame) or each decal "
   "instance (via #randomize).\n"

   "@image html images/decal_example.png \"Example of a Decal imagemap\"\n"

   "@tsexample\n"
   "datablock DecalData(BulletHoleDecal)\n"
   "{\n"
   "   material = \"DECAL_BulletHole\";\n"
   "   size = \"5.0\";\n"
   "   lifeSpan = \"50000\";\n"
   "   randomize = \"1\";\n"
   "   texRows = \"2\";\n"
   "   texCols = \"2\";\n"
   "   clippingAngle = \"60\";\n"
   "};\n"
   "@endtsexample\n\n"
   "@see Decals\n"
   "@ingroup Decals\n"
   "@ingroup FX\n"
);

//-------------------------------------------------------------------------
// DecalData
//-------------------------------------------------------------------------

DecalData::DecalData()
{
   size = 5;
   materialName = "";

   lifeSpan = 5000;
   fadeTime = 1000;

   frame = 0;
   randomize = false;
   texRows = 1;
   texCols = 1;

   fadeStartPixelSize = -1.0f;
   fadeEndPixelSize = 200.0f;

   material = NULL;
   matInst = NULL;

   renderPriority = 10;
   clippingMasks = STATIC_COLLISION_TYPEMASK;
   clippingAngle = 89.0f;

   texCoordCount = 1;

   // TODO: We could in theory calculate if we can skip 
   // normals on the decal by checking the material features.
   skipVertexNormals = false;

   for ( S32 i = 0; i < 16; i++ )
   {
      texRect[i].point.set( 0.0f, 0.0f );
      texRect[i].extent.set( 1.0f, 1.0f );
   }
}

DecalData::~DecalData()
{
   SAFE_DELETE( matInst );
}

bool DecalData::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   if (size < 0.0) {
      Con::warnf("DecalData::onAdd: size < 0");
      size = 0;
   }   
   
   getSet()->addObject( this );

	if( texRows > 1 || texCols > 1 )
		reloadRects();

   return true;
}

void DecalData::onRemove()
{
   Parent::onRemove();
}

void DecalData::initPersistFields()
{
   addGroup( "Decal" );

      addField( "size", TypeF32, Offset( size, DecalData ), 
         "Width and height of the decal in meters before scale is applied." );

      addField( "material", TypeMaterialName, Offset( materialName, DecalData ),
         "Material to use for this decal." );

      addField( "lifeSpan", TypeS32, Offset( lifeSpan, DecalData ),
         "Time (in milliseconds) before this decal will be automatically deleted." );

      addField( "fadeTime", TypeS32, Offset( fadeTime, DecalData ),
         "@brief Time (in milliseconds) over which to fade out the decal before "
         "deleting it at the end of its lifetime.\n\n"
         "@see lifeSpan" );

   endGroup( "Decal" );

   addGroup( "Rendering" );

      addField( "fadeStartPixelSize", TypeF32, Offset( fadeStartPixelSize, DecalData ), 
         "@brief LOD value - size in pixels at which decals of this type begin "
         "to fade out.\n\n"
         "This should be a larger value than #fadeEndPixelSize. However, you may "
         "also set this to a negative value to disable lod-based fading." );

      addField( "fadeEndPixelSize", TypeF32, Offset( fadeEndPixelSize, DecalData ), 
         "@brief LOD value - size in pixels at which decals of this type are "
         "fully faded out.\n\n"
         "This should be a smaller value than #fadeStartPixelSize." );

      addField( "renderPriority", TypeS8, Offset( renderPriority, DecalData ), 
         "Default renderPriority for decals of this type (determines draw "
         "order when decals overlap)." );

      addField( "clippingAngle", TypeF32, Offset( clippingAngle, DecalData ),
         "The angle in degrees used to clip geometry that faces away from the "
         "decal projection direction." );

   endGroup( "Rendering" );

   addGroup( "Texturing" );

      addField( "frame", TypeS32, Offset( frame, DecalData ),
         "Index of the texture rectangle within the imagemap to use for this decal." );

      addField( "randomize", TypeBool, Offset( randomize, DecalData ),
         "If true, a random frame from the imagemap is selected for each "
         "instance of the decal." );

      addField( "textureCoordCount", TypeS32, Offset( texCoordCount, DecalData ),
         "Number of individual frames in the imagemap (maximum 16)." );

      addField( "texRows", TypeS32, Offset( texRows, DecalData ),
         "@brief Number of rows in the supplied imagemap.\n\n"
         "Use #texRows and #texCols if the imagemap frames are arranged in a "
         "grid; use #textureCoords to manually specify UV coordinates for "
         "irregular sized frames." );

      addField( "texCols", TypeS32, Offset( texCols, DecalData ),
         "@brief Number of columns in the supplied imagemap.\n\n"
         "Use #texRows and #texCols if the imagemap frames are arranged in a "
         "grid; use #textureCoords to manually specify UV coordinates for "
         "irregular sized frames." );

      addField( "textureCoords", TypeRectF,  Offset( texRect, DecalData ), MAX_TEXCOORD_COUNT,
         "@brief An array of RectFs (topleft.x topleft.y extent.x extent.y) "
         "representing the UV coordinates for each frame in the imagemap.\n\n"
         "@note This field should only be set if the imagemap frames are "
         "irregular in size. Otherwise use the #texRows and #texCols fields "
         "and the UV coordinates will be calculated automatically." );

   endGroup( "Texturing" );

   Parent::initPersistFields();
}

void DecalData::onStaticModified( const char *slotName, const char *newValue )
{
   Parent::onStaticModified( slotName, newValue );

   if ( !isProperlyAdded() )
      return;

   // To allow changing materials live.
   if ( dStricmp( slotName, "material" ) == 0 )
   {
      materialName = newValue;
      _updateMaterial();
   }
   // To allow changing name live.
   else if ( dStricmp( slotName, "name" ) == 0 )
   {
      lookupName = getName();
   }
   else if ( dStricmp( slotName, "renderPriority" ) == 0 )
   {
      renderPriority = getMax( renderPriority, (U8)1 );
   }
}

bool DecalData::preload( bool server, String &errorStr )
{
   if (Parent::preload(server, errorStr) == false)
      return false;

   // Server assigns name to lookupName,
   // client assigns lookupName in unpack.
   if ( server )
      lookupName = getName();

   return true;
}

void DecalData::packData( BitStream *stream )
{
   Parent::packData( stream );

   stream->write( lookupName );
   stream->write( size );
   stream->write( materialName );
   stream->write( lifeSpan );
   stream->write( fadeTime );
	stream->write( texCoordCount );

   for (S32 i = 0; i < texCoordCount; i++)
      mathWrite( *stream, texRect[i] );

   stream->write( fadeStartPixelSize );
   stream->write( fadeEndPixelSize );
   stream->write( renderPriority );
   stream->write( clippingMasks );
   stream->write( clippingAngle );
   
	stream->write( texRows );
   stream->write( texCols );
	stream->write( frame );
	stream->write( randomize );
}

void DecalData::unpackData( BitStream *stream )
{
   Parent::unpackData( stream );

   stream->read( &lookupName );
   assignName(lookupName);
   stream->read( &size );  
   stream->read( &materialName );
   _updateMaterial();
   stream->read( &lifeSpan );
   stream->read( &fadeTime );
	stream->read( &texCoordCount );

   for (S32 i = 0; i < texCoordCount; i++)
      mathRead(*stream, &texRect[i]);

   stream->read( &fadeStartPixelSize );
   stream->read( &fadeEndPixelSize );
   stream->read( &renderPriority );
   stream->read( &clippingMasks );
   stream->read( &clippingAngle );
   
	stream->read( &texRows );
   stream->read( &texCols );
	stream->read( &frame );
	stream->read( &randomize );
}

void DecalData::_initMaterial()
{
   SAFE_DELETE( matInst );

   if ( material )
      matInst = material->createMatInstance();
   else
      matInst = MATMGR->createMatInstance( "WarningMaterial" );

   GFXStateBlockDesc desc;
   desc.setZReadWrite( true, false );
   //desc.zFunc = GFXCmpLess;
   matInst->addStateBlockDesc( desc );

   matInst->init( MATMGR->getDefaultFeatures(), getGFXVertexFormat<DecalVertex>() );
   if( !matInst->isValid() )
   {
      Con::errorf( "DecalData::_initMaterial - failed to create material instance for '%s'", materialName.c_str() );
      SAFE_DELETE( matInst );
      matInst = MATMGR->createMatInstance( "WarningMaterial" );
      matInst->init( MATMGR->getDefaultFeatures(), getGFXVertexFormat< DecalVertex >() );
   }
}

void DecalData::_updateMaterial()
{
   if ( materialName.isEmpty() )
      return;

   Material *pMat = NULL;
   if ( !Sim::findObject( materialName, pMat ) )
   {
      Con::printf( "DecalData::unpackUpdate, failed to find Material of name %s!", materialName.c_str() );
      return;
   }

   material = pMat;

   // Only update material instance if we have one allocated.
   if ( matInst )
      _initMaterial();
}

Material* DecalData::getMaterial()
{
   if ( !material )
   {
      _updateMaterial();
      if ( !material )
         material = static_cast<Material*>( Sim::findObject("WarningMaterial") );
   }

   return material;
}

BaseMatInstance* DecalData::getMaterialInstance()
{
   if ( !material || !matInst || matInst->getMaterial() != material )
      _initMaterial();

   return matInst;
}

DecalData* DecalData::findDatablock( String searchName )
{
   StringTableEntry className = DecalData::getStaticClassRep()->getClassName();
   DecalData *pData;
   SimSet *set = getSet();
   SimSetIterator iter( set );

   for ( ; *iter; ++iter )
   {
      if ( (*iter)->getClassName() != className )
      {
         Con::errorf( "DecalData::findDatablock - found a class %s object in DecalDataSet!", (*iter)->getClassName() );
         continue;
      }

      pData = static_cast<DecalData*>( *iter );
      if ( pData->lookupName.equal( searchName, String::NoCase ) )
         return pData;
   }

   return NULL;
}

void DecalData::inspectPostApply()
{ 
	reloadRects();
}

void DecalData::reloadRects()
{ 
	F32 rowsBase = 0;
	F32 colsBase = 0;
	bool canRenderRowsByFrame = false;
	bool canRenderColsByFrame = false;
	S32 id = 0;
	
	texRect[id].point.x = 0.f;
	texRect[id].extent.x = 1.f;
	texRect[id].point.y = 0.f;
	texRect[id].extent.y = 1.f;
	
	texCoordCount = (texRows * texCols) - 1;

	if( texCoordCount > 16 )
	{
		Con::warnf("Coordinate max must be lower than 16 to be a valid decal !");
		texRows = 1;
		texCols = 1;
		texCoordCount = 1;
	}

	// use current datablock information in order to build a template to extract
	// coordinates from. 
	if( texRows > 1 )
	{
		rowsBase = ( 1.f / texRows );
		canRenderRowsByFrame = true;
	}
	if( texCols > 1 )
	{
		colsBase = ( 1.f / texCols );
		canRenderColsByFrame = true;
	}

	// if were able, lets enter the loop
   if( frame >= 0 && (canRenderRowsByFrame || canRenderColsByFrame) )
	{
		// columns first then rows
		for ( S32 colId = 1; colId <= texCols; colId++ )
		{
			for ( S32 rowId = 1; rowId <= texRows; rowId++, id++ )
			{
				// if were over the coord count, lets go
				if(id > texCoordCount)
					return;

				// keep our dimensions correct
				if(rowId > texRows)
					rowId = 1;

				if(colId > texCols)
					colId = 1;

				// start setting our rect values per frame
				if( canRenderRowsByFrame )
				{
					texRect[id].point.x = rowsBase * ( rowId - 1 );
					texRect[id].extent.x = rowsBase;
				}
				
				if( canRenderColsByFrame )
				{
					texRect[id].point.y = colsBase * ( colId - 1 );
					texRect[id].extent.y = colsBase;
				}
			}
		}
	}
}

DefineEngineMethod(DecalData, postApply, void, (),,
   "Recompute the imagemap sub-texture rectangles for this DecalData.\n"
   "@tsexample\n"
   "// Inform the decal object to reload its imagemap and frame data.\n"
   "%decalData.texRows = 4;\n"
   "%decalData.postApply();\n"
   "@endtsexample\n")
{
   object->inspectPostApply();
}
