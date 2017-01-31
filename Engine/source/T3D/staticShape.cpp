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
#include "core/dnet.h"
#include "core/stream/bitStream.h"
#include "app/game.h"
#include "math/mMath.h"
#include "console/simBase.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "T3D/gameBase/moveManager.h"
#include "ts/tsShapeInstance.h"
#include "T3D/staticShape.h"
#include "math/mathIO.h"
#include "sim/netConnection.h"
#include "scene/sceneObjectLightingPlugin.h"

extern void wireCube(F32 size,Point3F pos);

static const U32 sgAllowedDynamicTypes = 0xffffff;

//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(StaticShapeData);

ConsoleDocClass( StaticShapeData,
	"@brief The most basic ShapeBaseData derrived shape datablock available in Torque 3D.\n\n"

	"When it comes to placing 3D objects in the scene, you effectively have two options:\n\n"
	"1. TSStatic objects\n\n"
	"2. ShapeBase derived objects\n\n"

	"Since ShapeBase and ShapeBaseData are not meant to be instantiated in script, you "
	"will use one of its child classes instead. Several game related objects are derived "
	"from ShapeBase: Player, Vehicle, Item, and so on.\n\n"

	"When you need a 3D object with datablock capabilities, you will use an object derived "
	"from ShapeBase. When you need an object with extremely low overhead, and with no other "
	"purpose than to be a 3D object in the scene, you will use TSStatic.\n\n"

	"The most basic child of ShapeBase is StaticShape. It does not introduce any of the "
	"additional functionality you see in Player, Item, Vehicle or the other game play "
	"heavy classes. At its core, it is comparable to a TSStatic, but with a datbalock.  Having "
   "a datablock provides a location for common variables as well as having access to "
   "various ShapeBaseData, GameBaseData and SimDataBlock callbacks.\n\n"

	"@tsexample\n"
	   "// Create a StaticShape using a datablock\n"
	   "datablock StaticShapeData(BasicShapeData)\n" 
	   "{\n"
	   "	shapeFile = \"art/shapes/items/kit/healthkit.dts\";\n"
	   "	testVar = \"Simple string, not a stock variable\";\n"
	   "};\n\n"
	   "new StaticShape()\n"
	   "{\n"
	   "	dataBlock = \"BasicShapeData\";\n"
	   "	position = \"0.0 0.0 0.0\";\n"
	   "	rotation = \"1 0 0 0\";\n"
	   "	scale = \"1 1 1\";\n"
	   "};\n"
	"@endtsexample\n\n"

	"@see StaticShape\n"
   "@see ShapeBaseData\n"
	"@see TSStatic\n\n"

	"@ingroup gameObjects\n"
	"@ingroup Datablocks");

StaticShapeData::StaticShapeData()
{
   dynamicTypeField     = 0;

   shadowEnable = true;

   noIndividualDamage = false;
}

void StaticShapeData::initPersistFields()
{
   addField("noIndividualDamage",   TypeBool, Offset(noIndividualDamage,   StaticShapeData), "Deprecated\n\n @internal");
   addField("dynamicType",          TypeS32,  Offset(dynamicTypeField,     StaticShapeData), 
      "@brief An integer value which, if speficied, is added to the value retured by getType().\n\n"
      "This allows you to extend the type mask for a StaticShape that uses this datablock.  Type masks "
      "are used for container queries, etc.");

   Parent::initPersistFields();
}

void StaticShapeData::packData(BitStream* stream)
{
   Parent::packData(stream);
   stream->writeFlag(noIndividualDamage);
   stream->write(dynamicTypeField);
}

void StaticShapeData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   noIndividualDamage = stream->readFlag();
   stream->read(&dynamicTypeField);
}


//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(StaticShape);

ConsoleDocClass( StaticShape,
	"@brief The most basic 3D shape with a datablock available in Torque 3D.\n\n"

	"When it comes to placing 3D objects in the scene, you technically have two options:\n\n"
	"1. TSStatic objects\n\n"
	"2. ShapeBase derived objects\n\n"

	"Since ShapeBase and ShapeBaseData are not meant to be instantiated in script, you "
	"will use one of its child classes instead. Several game related objects are derived "
	"from ShapeBase: Player, Vehicle, Item, and so on.\n\n"

	"When you need a 3D object with datablock capabilities, you will use an object derived "
	"from ShapeBase. When you need an object with extremely low overhead, and with no other "
	"purpose than to be a 3D object in the scene, you will use TSStatic.\n\n"

	"The most basic child of ShapeBase is StaticShape. It does not introduce any of the "
	"additional functionality you see in Player, Item, Vehicle or the other game play "
	"heavy classes. At its core, it is comparable to a TSStatic, but with a datbalock.  Having "
   "a datablock provides a location for common variables as well as having access to "
   "various ShapeBaseData, GameBaseData and SimDataBlock callbacks.\n\n"

	"@tsexample\n"
	   "// Create a StaticShape using a datablock\n"
	   "datablock StaticShapeData(BasicShapeData)\n" 
	   "{\n"
	   "	shapeFile = \"art/shapes/items/kit/healthkit.dts\";\n"
	   "	testVar = \"Simple string, not a stock variable\";\n"
	   "};\n\n"
	   "new StaticShape()\n"
	   "{\n"
	   "	dataBlock = \"BasicShapeData\";\n"
	   "	position = \"0.0 0.0 0.0\";\n"
	   "	rotation = \"1 0 0 0\";\n"
	   "	scale = \"1 1 1\";\n"
	   "};\n"
	"@endtsexample\n\n"

	"@see StaticShapeData\n"
	"@see ShapeBase\n"
	"@see TSStatic\n\n"

	"@ingroup gameObjects\n");

StaticShape::StaticShape()
{
   mTypeMask |= StaticShapeObjectType | StaticObjectType;
   mDataBlock = 0;
}

StaticShape::~StaticShape()
{
}


//----------------------------------------------------------------------------

void StaticShape::initPersistFields()
{
   Parent::initPersistFields();
}

bool StaticShape::onAdd()
{
   if(!Parent::onAdd() || !mDataBlock)
      return false;

   // We need to modify our type mask based on what our datablock says...
   mTypeMask |= (mDataBlock->dynamicTypeField & sgAllowedDynamicTypes);

   addToScene();

   if (isServerObject())
      scriptOnAdd();
   return true;
}

bool StaticShape::onNewDataBlock(GameBaseData* dptr, bool reload)
{
   mDataBlock = dynamic_cast<StaticShapeData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;

   scriptOnNewDataBlock();
   return true;
}

void StaticShape::onRemove()
{
   scriptOnRemove();
   removeFromScene();
   Parent::onRemove();
}


//----------------------------------------------------------------------------

void StaticShape::processTick(const Move* move)
{
   Parent::processTick(move);

   // Image Triggers
   if (move && mDamageState == Enabled) {
      setImageTriggerState(0,move->trigger[0]);
      setImageTriggerState(1,move->trigger[1]);
   }
}

void StaticShape::setTransform(const MatrixF& mat)
{
   Parent::setTransform(mat);
   setMaskBits(PositionMask);
}

void StaticShape::onUnmount(SceneObject*,S32)
{
   // Make sure the client get's the final server pos.
   setMaskBits(PositionMask);
}


//----------------------------------------------------------------------------

U32 StaticShape::packUpdate(NetConnection *connection, U32 mask, BitStream *bstream)
{
   U32 retMask = Parent::packUpdate(connection,mask,bstream);
   if (bstream->writeFlag(mask & (PositionMask | ExtendedInfoMask)))
   {

      // Write the transform (do _not_ use writeAffineTransform.  Since this is a static
      //  object, the transform must be RIGHT THE *&)*$&^ ON or it will goof up the
      //  synchronization between the client and the server.
      mathWrite(*bstream,mObjToWorld);
      mathWrite(*bstream, mObjScale);
   }

   // powered?
   bstream->writeFlag(mPowered);

   if (mLightPlugin) 
   {
      retMask |= mLightPlugin->packUpdate(this, ExtendedInfoMask, connection, mask, bstream);
   }

   return retMask;
}

void StaticShape::unpackUpdate(NetConnection *connection, BitStream *bstream)
{
   Parent::unpackUpdate(connection,bstream);
   if (bstream->readFlag())
   {
      MatrixF mat;
      mathRead(*bstream,&mat);
      Parent::setTransform(mat);
      Parent::setRenderTransform(mat);

      VectorF scale;
      mathRead(*bstream, &scale);
      setScale(scale);
   }

   // powered?
   mPowered = bstream->readFlag();

   if (mLightPlugin)
   {
      mLightPlugin->unpackUpdate(this, connection, bstream);
   }
}


//----------------------------------------------------------------------------
// This appears to be legacy T2 stuff
// Marked internal, as this is flagged to be deleted
// [8/1/2010 mperry]
DefineConsoleMethod( StaticShape, setPoweredState, void, (bool isPowered), , "(bool isPowered)"
			  "@internal")
{
   if(!object->isServerObject())
      return;
   object->setPowered(isPowered);
}

DefineConsoleMethod( StaticShape, getPoweredState, bool, (), , "@internal")
{
   if(!object->isServerObject())
      return(false);
   return(object->isPowered());
}
