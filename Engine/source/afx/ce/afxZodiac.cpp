
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

#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "core/frameAllocator.h"
#include "terrain/terrRender.h"
#include "gfx/primBuilder.h"

#include "afx/ce/afxZodiac.h"

GFX_ImplementTextureProfile(AFX_GFXZodiacTextureProfile, 
                            GFXTextureProfile::DiffuseMap, 
                            GFXTextureProfile::Static | GFXTextureProfile::NoMipmap | GFXTextureProfile::PreserveSize,  
                            GFXTextureProfile::NONE);

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxZodiacData::convertGradientRangeFromDegrees(Point2F& gradrange, const Point2F& gradrange_deg)
{
  F32 x = mCos(mDegToRad(gradrange_deg.x));
  F32 y = mCos(mDegToRad(gradrange_deg.y));
  if (y > x)
    gradrange.set(x, y);
  else
    gradrange.set(y, x);
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxZodiacData

IMPLEMENT_CO_DATABLOCK_V1(afxZodiacData);

ConsoleDocClass( afxZodiacData,
   "@brief A datablock that specifies a decal-like Zodiac effect.\n\n"

   "Zodiacs are special-purpose decal textures, often circular, that are always projected vertically onto the ground. Parameters "
   "control dynamic rotation and scale as well as texture, color, and blending style."
   "\n\n"

   "Zodiacs render on objects of type TerrainBlock, InteriorInstance, GroundPlane, MeshRoad, and TSStatic. They are very "
   "effective as spellcasting lighting rings, explosion shockwaves, scorched earth decals, and selection indicators."
   "\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

StringTableEntry afxZodiacData::GradientRangeSlot;
bool afxZodiacData::sPreferDestinationGradients = false;

afxZodiacData::afxZodiacData()
{
  txr_name = ST_NULLSTRING;
  radius_xy = 1;
  vert_range.set(0.0f, 0.0f);
  start_ang = 0;
  ang_per_sec = 0;
  color.set(1,1,1,1);
  grow_in_time = 0.0f; 
  shrink_out_time = 0.0f;
  growth_rate = 0.0f;
  blend_flags = BLEND_NORMAL;
  terrain_ok = true;
  interiors_ok = true;
  reflected_ok = false;
  non_reflected_ok = true;
  respect_ori_cons = false;
  scale_vert_range = true;

  interior_h_only = false;
  interior_v_ignore = false;
  interior_back_ignore = false;
  interior_opaque_ignore = false;
  interior_transp_ignore = true;

  altitude_max = 0.0f;
  altitude_falloff = 0.0f;
  altitude_shrinks = false;
  altitude_fades = false;

  distance_max = 75.0f;
  distance_falloff = 30.0f;

  use_grade_range = false;
  prefer_dest_grade = sPreferDestinationGradients;
  grade_range_user.set(0.0f, 45.0f);
  afxZodiacData::convertGradientRangeFromDegrees(grade_range, grade_range_user);
  inv_grade_range = false;
}

afxZodiacData::afxZodiacData(const afxZodiacData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  txr_name = other.txr_name;
  txr = other.txr;
  radius_xy = other.radius_xy;
  vert_range = other.vert_range;
  start_ang = other.start_ang;
  ang_per_sec = other.ang_per_sec;
  grow_in_time = other.grow_in_time;
  shrink_out_time = other.shrink_out_time;
  growth_rate = other.growth_rate;
  color = other.color;

  altitude_max = other.altitude_max;
  altitude_falloff = other.altitude_falloff;
  altitude_shrinks = other.altitude_shrinks;
  altitude_fades = other.altitude_fades;

  distance_max = other.distance_max;
  distance_falloff = other.distance_falloff;

  use_grade_range = other.use_grade_range;
  prefer_dest_grade = other.prefer_dest_grade;
  grade_range = other.grade_range;
  inv_grade_range = other.inv_grade_range;

  zflags = other.zflags;
  expand_zflags();
}

ImplementEnumType( afxZodiac_BlendType, "Possible zodiac blend types.\n" "@ingroup afxZodiac\n\n" )
   { afxZodiacData::BLEND_NORMAL,      "normal",         "..." },
   { afxZodiacData::BLEND_ADDITIVE,    "additive",       "..." },
   { afxZodiacData::BLEND_SUBTRACTIVE, "subtractive",    "..." },
EndImplementEnumType;

void afxZodiacData::initPersistFields()
{
  addField("texture",               TypeFilename,   Offset(txr_name,          afxZodiacData),
    "An image to use as the zodiac's texture.");
  addField("radius",                TypeF32,        Offset(radius_xy,         afxZodiacData),
    "The zodiac's radius in scene units.");
  addField("verticalRange",         TypePoint2F,    Offset(vert_range,        afxZodiacData),
    "For interior zodiacs only, verticalRange specifies distances above and below the "
    "zodiac's position. If both values are 0.0, the radius is used.");
  addField("scaleVerticalRange",    TypeBool,       Offset(scale_vert_range,  afxZodiacData),
    "Specifies if the zodiac's verticalRange should scale according to changes in the "
    "radius. When a zodiacs is used as an expanding shockwave, this value should be set "
    "to false, otherwise the zodiac can expand to cover an entire interior.");
  addField("startAngle",            TypeF32,        Offset(start_ang,         afxZodiacData),
    "The starting angle in degrees of the zodiac's rotation.");
  addField("rotationRate",          TypeF32,        Offset(ang_per_sec,       afxZodiacData),
    "The rate of rotation in degrees-per-second. Zodiacs with a positive rotationRate "
    "rotate clockwise, while those with negative values turn counter-clockwise.");
  addField("growInTime",            TypeF32,        Offset(grow_in_time,      afxZodiacData),
    "A duration of time in seconds over which the zodiac grows from a zero size to its "
    "full size as specified by the radius.");
  addField("shrinkOutTime",         TypeF32,        Offset(shrink_out_time,   afxZodiacData),
    "A duration of time in seconds over which the zodiac shrinks from full size to "
    "invisible.");
  addField("growthRate",            TypeF32,        Offset(growth_rate,       afxZodiacData),
    "A rate in meters-per-second at which the zodiac grows in size. A negative value will "
    "shrink the zodiac.");
  addField("color",                 TypeColorF,     Offset(color,             afxZodiacData),
    "A color value for the zodiac.");

  addField("blend", TYPEID<BlendType>(), Offset(blend_flags, afxZodiacData),
    "A blending style for the zodiac. Possible values: normal, additive, or subtractive.");

  addField("showOnTerrain",         TypeBool,       Offset(terrain_ok,        afxZodiacData),
    "Specifies if the zodiac should be rendered on terrain or terrain-like surfaces.");
  addField("showOnInteriors",       TypeBool,       Offset(interiors_ok,      afxZodiacData),
    "Specifies if the zodiac should be rendered on interior or interior-like surfaces.");
  addField("showInReflections",     TypeBool,       Offset(reflected_ok,      afxZodiacData),
    "Specifies if the zodiac should be rendered on the reflection rendering pass of the "
    "object it will be projected onto.");
  addField("showInNonReflections",  TypeBool,       Offset(non_reflected_ok,  afxZodiacData),
    "Specifies if the zodiac should be rendered on the non-reflection rendering pass of "
    "the object it will be projected onto.");
  addField("trackOrientConstraint", TypeBool,       Offset(respect_ori_cons,  afxZodiacData),
    "Specifies if the zodiac's rotation should be defined by its constrained "
    "transformation.");

  addField("interiorHorizontalOnly",    TypeBool,   Offset(interior_h_only,        afxZodiacData),
    "Specifies if interior zodiacs should be rendered exclusively on perfectly horizontal "
    "interior surfaces.");
  addField("interiorIgnoreVertical",    TypeBool,   Offset(interior_v_ignore,      afxZodiacData),
    "Specifies if interior zodiacs should not be rendered on perfectly vertical interior "
    "surfaces.");
  addField("interiorIgnoreBackfaces",   TypeBool,   Offset(interior_back_ignore,   afxZodiacData),
    "Specifies if interior zodiacs should not be rendered on interior surface which are "
    "backfacing to the zodiac's center.");
  addField("interiorIgnoreOpaque",      TypeBool,   Offset(interior_opaque_ignore, afxZodiacData),
    "");
  addField("interiorIgnoreTransparent", TypeBool,   Offset(interior_transp_ignore, afxZodiacData),
    "");

  addField("altitudeMax",           TypeF32,      Offset(altitude_max, afxZodiacData),
    "The altitude at which zodiac becomes invisible as the result of fading out or "
    "becoming too small.");
  addField("altitudeFalloff",       TypeF32,      Offset(altitude_falloff, afxZodiacData),
    "The altitude at which zodiac begins to fade and/or shrink.");
  addField("altitudeShrinks",       TypeBool,     Offset(altitude_shrinks, afxZodiacData),
    "When true, zodiac becomes smaller as altitude increases.");
  addField("altitudeFades",         TypeBool,     Offset(altitude_fades, afxZodiacData),
    "When true, zodiac fades out as altitude increases.");

  addField("distanceMax",           TypeF32,      Offset(distance_max, afxZodiacData),
    "The distance from camera at which the zodiac becomes invisible as the result of "
    "fading out.");
  addField("distanceFalloff",       TypeF32,      Offset(distance_falloff, afxZodiacData),
    "The distance from camera at which the zodiac begins to fade out.");

  addField("useGradientRange",      TypeBool,     Offset(use_grade_range, afxZodiacData),
    "When true, gradientRange will be used to determine on which polygons the zodiac will "
    "render.");
  addField("preferDestinationGradients", TypeBool,  Offset(prefer_dest_grade, afxZodiacData),
    "When true, a gradientRange specified on an InteriorInstance or TSStatic will be used "
    "instead of the zodiac's gradientRange.");
  addField("gradientRange",              TypePoint2F,  Offset(grade_range_user, afxZodiacData),
    "Zodiac will render on polygons with gradients within the range specified by "
    "gradientRange. 0 for floor polys, 90 for wall polys, 180 for ceiling polys.");
  addField("invertGradientRange",        TypeBool,     Offset(inv_grade_range,   afxZodiacData),
    "When true, the zodiac will render on polygons with gradients outside of the range "
    "specified by gradientRange.");

  Parent::initPersistFields();

  GradientRangeSlot = StringTable->lookup("gradientRange");
  Con::addVariable("pref::afxZodiac::preferDestinationGradients", TypeBool, &sPreferDestinationGradients);
}

bool afxZodiacData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  if (altitude_falloff > altitude_max)
    altitude_falloff = altitude_max;

  if (distance_falloff > distance_max)
    distance_falloff = distance_max;

  return true;
}

void afxZodiacData::packData(BitStream* stream)
{
	Parent::packData(stream);

  merge_zflags();

  stream->writeString(txr_name);
  stream->write(radius_xy);
  stream->write(vert_range.x);
  stream->write(vert_range.y);
  stream->write(grade_range.x);
  stream->write(grade_range.y);
  stream->write(start_ang);
  stream->write(ang_per_sec);
  stream->write(grow_in_time);
  stream->write(shrink_out_time);
  stream->write(growth_rate);
  stream->write(color);
  stream->write(zflags);
  stream->write(altitude_max);
  stream->write(altitude_falloff);
  stream->writeFlag(altitude_shrinks);
  stream->writeFlag(altitude_fades);
  stream->write(distance_max);
  stream->write(distance_falloff);
}

void afxZodiacData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  txr_name = stream->readSTString();
  txr = GFXTexHandle();
  stream->read(&radius_xy);
  stream->read(&vert_range.x);
  stream->read(&vert_range.y);
  stream->read(&grade_range.x);
  stream->read(&grade_range.y);
  stream->read(&start_ang);
  stream->read(&ang_per_sec);
  stream->read(&grow_in_time);
  stream->read(&shrink_out_time);
  stream->read(&growth_rate);
  stream->read(&color);
  stream->read(&zflags);
  stream->read(&altitude_max);
  stream->read(&altitude_falloff);
  altitude_shrinks = stream->readFlag();
  altitude_fades = stream->readFlag();
  stream->read(&distance_max);
  stream->read(&distance_falloff);

  expand_zflags();
}

bool afxZodiacData::preload(bool server, String &errorStr)
{
  if (!Parent::preload(server, errorStr))
    return false;

  if (!server)
  {
    if (txr_name && txr_name[0] != '\0')
    {
      txr.set(txr_name, &AFX_GFXZodiacTextureProfile, "Zodiac Texture");
    }
  }

  if (vert_range.x == 0.0f && vert_range.y == 0.0f)
    vert_range.x = vert_range.y = radius_xy;

  return true;
}

void afxZodiacData::onStaticModified(const char* slot, const char* newValue)
{
  Parent::onStaticModified(slot, newValue);
  if (slot == GradientRangeSlot)
  {
    convertGradientRangeFromDegrees(grade_range, grade_range_user);
    return;
  }
  merge_zflags();
}

void afxZodiacData::onPerformSubstitutions() 
{ 
    if (txr_name && txr_name[0] != '\0')
    {
      txr.set(txr_name, &AFX_GFXZodiacTextureProfile, "Zodiac Texture");
    }
}

F32 afxZodiacData::calcRotationAngle(F32 elapsed, F32 rate_factor)
{
  F32 angle = start_ang + elapsed*ang_per_sec*rate_factor;
  angle = mFmod(angle, 360.0f);

  return angle;
}

void afxZodiacData::expand_zflags()
{
  blend_flags = (zflags & BLEND_MASK);
  terrain_ok = ((zflags & SHOW_ON_TERRAIN) != 0);
  interiors_ok = ((zflags & SHOW_ON_INTERIORS) != 0);
  reflected_ok = ((zflags & SHOW_IN_REFLECTIONS) != 0);
  non_reflected_ok = ((zflags & SHOW_IN_NON_REFLECTIONS) != 0);
  respect_ori_cons = ((zflags & RESPECT_ORIENTATION) != 0);
  scale_vert_range = ((zflags & SCALE_VERT_RANGE) != 0);
  interior_h_only = ((zflags & INTERIOR_HORIZ_ONLY) != 0);
  interior_v_ignore = ((zflags & INTERIOR_VERT_IGNORE) != 0);
  interior_back_ignore = ((zflags & INTERIOR_BACK_IGNORE) != 0);
  interior_opaque_ignore = ((zflags & INTERIOR_OPAQUE_IGNORE) != 0);
  interior_transp_ignore = ((zflags & INTERIOR_TRANSP_IGNORE) != 0);
  use_grade_range = ((zflags & USE_GRADE_RANGE) != 0);
  prefer_dest_grade = ((zflags & PREFER_DEST_GRADE) != 0);
  inv_grade_range = ((zflags & INVERT_GRADE_RANGE) != 0);
}

void afxZodiacData::merge_zflags()
{
  zflags = (blend_flags & BLEND_MASK);
  if (terrain_ok)
    zflags |= SHOW_ON_TERRAIN;
  if (interiors_ok)
    zflags |= SHOW_ON_INTERIORS;
  if (reflected_ok)
    zflags |= SHOW_IN_REFLECTIONS;
  if (non_reflected_ok)
    zflags |= SHOW_IN_NON_REFLECTIONS;
  if (respect_ori_cons)
    zflags |= RESPECT_ORIENTATION;
  if (scale_vert_range)
    zflags |= SCALE_VERT_RANGE;
  if (interior_h_only)
    zflags |= INTERIOR_HORIZ_ONLY;
  if (interior_v_ignore)
    zflags |= INTERIOR_VERT_IGNORE;
  if (interior_back_ignore)
    zflags |= INTERIOR_BACK_IGNORE;
  if (interior_opaque_ignore)
    zflags |= INTERIOR_OPAQUE_IGNORE;
  if (interior_transp_ignore)
    zflags |= INTERIOR_TRANSP_IGNORE;
  if (use_grade_range)
    zflags |= USE_GRADE_RANGE;
  if (prefer_dest_grade)
    zflags |= PREFER_DEST_GRADE;
  if (inv_grade_range)
    zflags |= INVERT_GRADE_RANGE;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
