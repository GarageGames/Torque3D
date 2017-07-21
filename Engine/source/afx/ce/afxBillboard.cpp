
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"

#include "gfx/gfxAPI.h"
#include "math/mathIO.h"

#include "afx/afxChoreographer.h"
#include "afx/ce/afxBillboard.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxBillboardData

IMPLEMENT_CO_DATABLOCK_V1(afxBillboardData);

ConsoleDocClass( afxBillboardData,
   "@brief A datablock that specifies a Billboard effect.\n\n"

   "A Billboard effect is a textured quadrangle which is always aligned to face towards the camera. It is much like a single "
   "static particle and is rendered in a similar fashion."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxBillboardData::afxBillboardData()
{
  color.set(1.0f, 1.0f, 1.0f, 1.0f);
  txr_name = ST_NULLSTRING;
  dimensions.set(1.0f, 1.0f);
  texCoords[0].set(0.0f, 0.0f);
  texCoords[1].set(0.0f, 1.0f);
  texCoords[2].set(1.0f, 1.0f);
  texCoords[3].set(1.0f, 0.0f);
  blendStyle = BlendUndefined;
  srcBlendFactor = BLEND_UNDEFINED;
  dstBlendFactor = BLEND_UNDEFINED;
  texFunc = TexFuncModulate;
}

afxBillboardData::afxBillboardData(const afxBillboardData& other, bool temp_clone)
  : GameBaseData(other, temp_clone)
{
  color = other.color;
  txr_name = other.txr_name;
  txr = other.txr;
  dimensions = other.dimensions;
  texCoords[0] = other.texCoords[0];
  texCoords[1] = other.texCoords[1];
  texCoords[2] = other.texCoords[2];
  texCoords[3] = other.texCoords[3];
  blendStyle = other.blendStyle;
  srcBlendFactor = other.srcBlendFactor;
  dstBlendFactor = other.dstBlendFactor;
  texFunc = other.texFunc;
}

#define myOffset(field) Offset(field, afxBillboardData)

extern EnumTable srcBlendFactorTable;
extern EnumTable dstBlendFactorTable;

ImplementEnumType( afxBillboard_BlendStyle, "Possible blending types.\n" "@ingroup afxBillboard\n\n" )
    { afxBillboardData::BlendNormal,         "NORMAL",         "..." },
    { afxBillboardData::BlendAdditive,       "ADDITIVE",       "..." },
    { afxBillboardData::BlendSubtractive,    "SUBTRACTIVE",    "..." },
    { afxBillboardData::BlendPremultAlpha,   "PREMULTALPHA",   "..." },
EndImplementEnumType;

ImplementEnumType( afxBillboard_TexFuncType, "Possible texture function types.\n" "@ingroup afxBillboard\n\n" )
    { afxBillboardData::TexFuncReplace,   "replace",     "..." },
    { afxBillboardData::TexFuncModulate,  "modulate",    "..." },
    { afxBillboardData::TexFuncAdd,       "add",         "..." },
EndImplementEnumType;

void afxBillboardData::initPersistFields()
{
  addField("color",           TypeColorF,     myOffset(color),
    "The color assigned to the quadrangle geometry. The way it combines with the given "
    "texture varies according to the setting of the textureFunction field.");
  addField("texture",         TypeFilename,   myOffset(txr_name),
    "An image to use as the billboard's texture.");
  addField("dimensions",      TypePoint2F,    myOffset(dimensions),
    "A value-pair that specifies the horizontal and vertical dimensions of the billboard "
    "in scene units.");
  addField("textureCoords",   TypePoint2F,    myOffset(texCoords),  4,
    "An array of four value-pairs that specify the UV texture coordinates for the four "
    "corners of the billboard's quadrangle.");

  addField("blendStyle",      TYPEID<afxBillboardData::BlendStyle>(),   myOffset(blendStyle),
    "Selects a common blend factor preset. When set to 'user', srcBlendFactor and "
    "dstBlendFactor can be used to set additional blend factor combinations.\n"
    "Possible values: normal, additive, subtractive, premultalpha, or user.");
  addField("srcBlendFactor",  TYPEID<GFXBlend>(),   myOffset(srcBlendFactor),
    "Specifies source blend factor when blendStyle is set to 'user'.\n"
    "Possible values: GFXBlendZero, GFXBlendOne, GFXBlendDestColor, GFXBlendInvDestColor, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha, GFXBlendDestAlpha, GFXBlendInvDestAlpha, or GFXBlendSrcAlphaSat");
  addField("dstBlendFactor",  TYPEID<GFXBlend>(),   myOffset(dstBlendFactor),
    "Specifies destination blend factor when blendStyle is set to 'user'.\n"
    "Possible values: GFXBlendZero, GFXBlendOne, GFXBlendSrcColor, GFXBlendInvSrcColor, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha, GFXBlendDestAlpha, or GFXBlendInvDestAlpha");

  addField("textureFunction", TYPEID<afxBillboardData::TexFuncType>(),  myOffset(texFunc),
    "Selects a texture function that determines how the texture pixels are combined "
    "with the shaded color of the billboard's quadrangle geometry.\n"
    "Possible values: replace, modulate, or add.");

  Parent::initPersistFields();
}

void afxBillboardData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->write(color);
  stream->writeString(txr_name);
  mathWrite(*stream, dimensions);
  mathWrite(*stream, texCoords[0]);
  mathWrite(*stream, texCoords[1]);
  mathWrite(*stream, texCoords[2]);
  mathWrite(*stream, texCoords[3]);

  stream->writeInt(srcBlendFactor, 4);
  stream->writeInt(dstBlendFactor, 4);
  stream->writeInt(texFunc, 4);
}

void afxBillboardData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&color);
  txr_name = stream->readSTString();
  txr = GFXTexHandle();
  mathRead(*stream, &dimensions);
  mathRead(*stream, &texCoords[0]);
  mathRead(*stream, &texCoords[1]);
  mathRead(*stream, &texCoords[2]);
  mathRead(*stream, &texCoords[3]);

  srcBlendFactor = (GFXBlend) stream->readInt(4);
  dstBlendFactor = (GFXBlend) stream->readInt(4);
  texFunc = stream->readInt(4);
}

bool afxBillboardData::preload(bool server, String &errorStr)
{
  if (!Parent::preload(server, errorStr))
    return false;

  if (!server)
  {
    if (txr_name && txr_name[0] != '\0')
    {
      txr.set(txr_name, &GFXDefaultStaticDiffuseProfile, "Billboard Texture");
    }
  }

   // if blend-style is set to User, check for defined blend-factors
   if (blendStyle == BlendUser && (srcBlendFactor == BLEND_UNDEFINED || dstBlendFactor == BLEND_UNDEFINED))
   {
      blendStyle = BlendUndefined;
      Con::warnf(ConsoleLogEntry::General, "afxBillboardData(%s) incomplete blend factor specification.", getName());
   }

   // silently switch Undefined blend-style to User if blend factors are both defined
   if (blendStyle == BlendUndefined && srcBlendFactor != BLEND_UNDEFINED && dstBlendFactor != BLEND_UNDEFINED)
   {
      blendStyle = BlendUser;
   }

   // set pre-defined blend-factors 
   switch (blendStyle)
   {
   case BlendNormal:
      srcBlendFactor = GFXBlendSrcAlpha;
      dstBlendFactor = GFXBlendInvSrcAlpha;
      break;
   case BlendSubtractive:
      srcBlendFactor = GFXBlendZero;
      dstBlendFactor = GFXBlendInvSrcColor;
      break;
   case BlendPremultAlpha:
      srcBlendFactor = GFXBlendOne;
      dstBlendFactor = GFXBlendInvSrcAlpha;
      break;
   case BlendUser:
      break;
   case BlendAdditive:
      srcBlendFactor = GFXBlendSrcAlpha;
      dstBlendFactor = GFXBlendOne;
      break;
   case BlendUndefined:
   default:
      blendStyle = BlendNormal;
      srcBlendFactor = GFXBlendSrcAlpha;
      dstBlendFactor = GFXBlendInvSrcAlpha;
      break;
   }

  return true;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxBillboard

IMPLEMENT_CO_NETOBJECT_V1(afxBillboard);

ConsoleDocClass( afxBillboard,
   "@brief A Billboard effect as defined by an afxBillboardData datablock.\n\n"

   "A Billboard effect is a textured quadrangle which is always aligned to "
   "face towards the camera. It is much like a single static particle and is rendered "
   "in a similar fashion.\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
);

afxBillboard::afxBillboard()
{
  mNetFlags.clear();
  mNetFlags.set(IsGhost);

  mDataBlock = 0;
  fade_amt = 1.0f;
  is_visible = true;
  sort_priority = 0;
  live_color.set(1.0f, 1.0f, 1.0f, 1.0f);
}

afxBillboard::~afxBillboard()
{
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

bool afxBillboard::onNewDataBlock(GameBaseData* dptr, bool reload)
{
  mDataBlock = dynamic_cast<afxBillboardData*>(dptr);
  if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  live_color = mDataBlock->color;

  return true;
}

bool afxBillboard::onAdd()
{
  if(!Parent::onAdd())
    return false;

  F32 width = mDataBlock->dimensions.x * 0.5f;
  F32 height = mDataBlock->dimensions.y * 0.5f;
  mObjBox = Box3F(Point3F(-width, -0.01f, -height), Point3F(width, 0.01f, height));
  
  addToScene();
  
  return true;
}

void afxBillboard::onRemove()
{
  removeFromScene();
  
  Parent::onRemove();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//