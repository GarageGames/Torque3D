
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
#include "math/mathIO.h"

#include "afx/util/afxPath.h"
#include "afx/util/afxPath3D.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxPathData

IMPLEMENT_CO_DATABLOCK_V1(afxPathData);

ConsoleDocClass( afxPathData,
   "@brief A datablock for specifiying a 3D path for use with AFX.\n\n"

   "@ingroup afxUtil\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

StringTableEntry afxPathData::POINTS_FIELD;
StringTableEntry afxPathData::ROLL_FIELD;
StringTableEntry afxPathData::TIMES_FIELD;

ImplementEnumType( afxPath3DLoopType, "Possible loop types for an afxPath.\n" "@ingroup afxPath\n\n" )
   { afxPath3D::LOOP_CONSTANT,  "constant",     "..." },
   { afxPath3D::LOOP_CYCLE,     "cycle",        "..." },   
   { afxPath3D::LOOP_OSCILLATE, "oscillate",    "..." }, 
EndImplementEnumType;

afxPathData::afxPathData()
{
  if (POINTS_FIELD == 0)
  {
    POINTS_FIELD = StringTable->insert("points");
    ROLL_FIELD = StringTable->insert("roll");
    TIMES_FIELD = StringTable->insert("times");
  }

  loop_string   = ST_NULLSTRING;
  delay = 0;
  lifetime = 0;
  loop_type = 0;
  mult = 1.0f;
  time_offset = 0.0f;
  resolved = false;
  reverse = false;
  offset.zero();
  echo = false;
  concentric = false;

  points_string = ST_NULLSTRING;
  points = 0;
  num_points = 0;
  update_points = true;

  roll_string   = ST_NULLSTRING;
  rolls = 0;
  update_rolls = true;

  times_string = ST_NULLSTRING;
  times = 0;
  update_times = true;
}

afxPathData::afxPathData(const afxPathData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  points_string = other.points_string;
  roll_string = other.roll_string;
  loop_string = other.loop_string;
  delay = other.delay;
  lifetime = other.lifetime;
  loop_type = other.loop_type; // --
  mult = other.mult;
  time_offset = other.time_offset;
  resolved = other.resolved; // --
  reverse = other.reverse;
  offset = other.offset;
  echo = other.echo;
  concentric = other.concentric;
  times_string = other.times_string;

  num_points = other.num_points; // --
  if (other.points && num_points > 0)
  {
    points = new Point3F[num_points];
    dMemcpy(points, other.points, sizeof(Point3F)*num_points); // --
  }
  else
    points = 0;
  if (other.rolls && num_points > 0)
  {
    rolls = new F32[num_points];
    dMemcpy(rolls, other.rolls, sizeof(F32)*num_points); // --
  }
  else
    rolls = 0;
  if (other.times && num_points > 0)
  {
    times = new F32[num_points];
    dMemcpy(times, other.times, sizeof(F32)*num_points); // --
  }
  else
    times = 0;

  update_points = other.update_points; // --
  update_rolls = other.update_rolls; // --
  update_times = other.update_times; // --
}

afxPathData::~afxPathData()
{
  clear_arrays();
}

void afxPathData::initPersistFields()
{
  addField("points",     TypeString,  Offset(points_string, afxPathData),
    "...");
  addField("roll",       TypeString,  Offset(roll_string,   afxPathData),
    "...");
  addField("times",      TypeString,  Offset(times_string,  afxPathData),
    "...");
  addField("loop",       TypeString,  Offset(loop_string,   afxPathData),
    "...");
  addField("mult",       TypeF32,     Offset(mult,          afxPathData),
    "...");
  addField("delay",      TypeF32,     Offset(delay,         afxPathData),
    "...");
  addField("lifetime",   TypeF32,     Offset(lifetime,      afxPathData),
    "...");
  addField("timeOffset", TypeF32,     Offset(time_offset,   afxPathData),
    "...");
  addField("reverse",    TypeBool,    Offset(reverse,       afxPathData),
    "...");
  addField("offset",     TypePoint3F, Offset(offset,        afxPathData),
    "...");
  addField("echo",       TypeBool,    Offset(echo,          afxPathData),
    "...");
  addField("concentric", TypeBool,    Offset(concentric,    afxPathData),
    "...");

  Parent::initPersistFields();
}

bool afxPathData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  update_derived_values();

  return true;
}

void afxPathData::onRemove()
{
  clear_arrays();
  loop_type = 0;

  Parent::onRemove();
}

void afxPathData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->write(num_points);
  if (num_points > 0)
  {
    for (U32 i = 0; i < num_points; i++)
      mathWrite(*stream, points[i]);
    if (stream->writeFlag(rolls != 0))
    {
      for (U32 i = 0; i < num_points; i++)
        stream->write(rolls[i]);
    }
    if (stream->writeFlag(times != 0))
    {
      for (U32 i = 0; i < num_points; i++)
        stream->write(times[i]);
    }
  }

  stream->writeString(loop_string);
  stream->write(delay);
  stream->write(lifetime);
  stream->write(time_offset);
  stream->write(mult);
  stream->writeFlag(reverse);
  mathWrite(*stream, offset);
  stream->writeFlag(echo);
  stream->writeFlag(concentric);
}

void afxPathData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  clear_arrays();

  // read the points and rolls
  stream->read(&num_points);
  if (num_points > 0)
  {
    points = new Point3F[num_points];
    for (U32 i = 0; i < num_points; i++)
      mathRead(*stream, &points[i]);
    update_points = false;
    if (stream->readFlag())
    {
      rolls = new F32[num_points]; 
      for (U32 i = 0; i < num_points; i++)
        stream->read(&rolls[i]);
      update_rolls = false;
    }
    if (stream->readFlag())
    {
      times = new F32[num_points]; 
      for (U32 i = 0; i < num_points; i++)
        stream->read(&times[i]);
      update_times = false;
    }
  }

  loop_string = stream->readSTString();
  stream->read(&delay);
  stream->read(&lifetime);
  stream->read(&time_offset);
  stream->read(&mult);
  reverse = stream->readFlag();
  mathRead(*stream, &offset);
  echo = stream->readFlag();
  concentric = stream->readFlag();
}

void afxPathData::update_derived_values()
{
  U32 num_rolls = (rolls != 0) ? num_points : 0;
  U32 num_times = (times != 0) ? num_points : 0;

  if (update_points)
  {
    derive_points_array();
    update_points = false;
  }

  if (update_rolls || num_rolls != num_points)
  {
    derive_rolls_array();
    update_rolls = false;
  }

  if (update_times || num_times != num_points)
  {
    derive_times_array();
    update_times = false;
  }

  // CAUTION: The following block of code is fragile and tricky since it depends
  // on the underlying structures defined by ImplementEnumType/EndImplementEnumType.
  // This done because the enum text is parsed from a longer string.
  if (loop_string != ST_NULLSTRING) 
  {
     for (unsigned int i = 0; i < _afxPath3DLoopType::_sEnumTable.getNumValues(); i++)
     {
        if (dStricmp(_afxPath3DLoopType::_sEnumTable[i].mName, loop_string) == 0)
        {
           loop_type = _afxPath3DLoopType::_sEnumTable[i].mInt;
           break;
        }
     }
  }
  else 
  {
     loop_string = _afxPath3DLoopType::_sEnumTable[0].mName;
     loop_type   = _afxPath3DLoopType::_sEnumTable[0].mInt;
  }  
}

bool afxPathData::preload(bool server, String &errorStr)
{
  if (!Parent::preload(server, errorStr))
    return false;

  update_derived_values();

  return true;
}

void afxPathData::onStaticModified(const char* slot, const char* newValue)
{
   Parent::onStaticModified(slot, newValue);

  if (slot == POINTS_FIELD)
  {
    update_points = true;
    return;
  }
  if (slot == ROLL_FIELD)
  {
    update_rolls = true;
    return;
  }
  if (slot == TIMES_FIELD)
  {
    update_times = true;
    return;
  }
}

void afxPathData::extract_floats_from_string(Vector<F32>& values, const char* points_str)
{
  values.clear();

  if (!points_str)
    return;

  // make a copy of points_str for tokenizing
  char* tokCopy = dStrdup(points_str);

  // extract each token, convert to float, add to values[]
  char* currTok = dStrtok(tokCopy, " \t");
  while (currTok != NULL) 
  {
    F32 value = dAtof(currTok);
    values.push_back(value);
    currTok = dStrtok(NULL, " \t");
  }

  dFree(tokCopy); 
}

Point3F* afxPathData::build_points_array(Vector<F32>& values, U32& n_points, bool reverse)
{
  AssertFatal(values.size() > 0, "Values array is empty.");
  AssertFatal(values.size()%3 == 0, "Values array is not a multiple of 3.");

  n_points = values.size()/3;
  Point3F* points = new Point3F[n_points];

  if (reverse)
  {    
    U32 p_i = 0;
    for (S32 i = values.size()-1; i > 1; i-=3)
      points[p_i++].set(values[i-2], values[i-1], values[i]);        
  }
  else
  {
    U32 p_i = 0;
    for (U32 i = 0; i < values.size(); i+=3)
      points[p_i++].set(values[i], values[i+1], values[i+2]);
  }

  return points;
}

F32* afxPathData::build_floats_array(Vector<F32>& values, U32& n_floats, bool reverse)
{
  AssertFatal(values.size() > 0, "Values array is empty.");

  n_floats = values.size();
  F32* floats = new F32[n_floats];

  if (reverse)
  {    
    F32* f = floats;
    for (S32 i = values.size()-1; i >= 0; i--)
    {
      *f = values[i]; 
      f++;
    }
  }
  else
  {
    for (U32 i = 0; i < values.size(); i++)
     floats[i] = values[i]; 
  }

  return floats;
}

void afxPathData::clear_arrays()
{
  num_points = 0;
  if (points)
  {
    delete [] points;
    points = 0;
  }
  if (rolls)
  {
    delete [] rolls;
    rolls = 0;
  }
  if (times)
  {
    delete [] times;
    times = 0;
  }
  update_points = true;
  update_rolls = true;
  update_times = true;
}

void afxPathData::derive_points_array()
{
  if (points_string == ST_NULLSTRING) 
    return;

  if (points)
  {
    delete [] points;
    points = 0;
  }
  num_points = 0;

  Vector<F32> values;
  extract_floats_from_string(values, points_string);
  if (values.size() == 0)
  {
    Con::warnf(ConsoleLogEntry::General, "afxPathData(%s) empty points field, datablock is invalid.", getName());
    return;
  }
  if (values.size()%3 != 0)
  {
    Con::warnf(ConsoleLogEntry::General, "afxPathData(%s) total points values is not a multiple of 3, datablock is invalid.", getName());
    return;
  }

  points = build_points_array(values, num_points, reverse);

  if (offset.x != 0.0f || offset.y != 0.0f || offset.z != 0.0f)
  {
    // add offset here for efficiency (saves an addition from afxXM_PathConform)
    for (U32 i = 0; i < num_points; i++)
      points[i] += offset;
  }
}

void afxPathData::derive_rolls_array()
{
  if (roll_string == ST_NULLSTRING) 
    return;

  if (rolls)
  {
    delete [] rolls;
    rolls = 0;
  }
  
  Vector<F32> values;
  extract_floats_from_string(values, roll_string);
  if (values.size() == 0)
    return;

  if (values.size() != num_points)
  {
    Con::warnf(ConsoleLogEntry::General, "afxPathData(%s) total roll values is not equal to total points, rolls ignored.", getName());
    return;
  }

  U32 num_rolls = 0;
  rolls = build_floats_array(values, num_rolls, reverse);

  AssertFatal(num_rolls == num_points, "Unexpected error: num_rolls disagrees with num_points.");
}

void afxPathData::derive_times_array()
{
  if (times_string == ST_NULLSTRING) 
    return;

  if (times)
  {
    delete [] times;
    times = 0;
  }
  
  Vector<F32> values;
  extract_floats_from_string(values, times_string);
  if (values.size() == 0)
    return;

  if (values.size() != num_points)
  {
    Con::warnf(ConsoleLogEntry::General, "afxPathData(%s) total time values is not equal to total points, times ignored", getName());
    return;
  }

  U32 num_times = 0;
  times = build_floats_array(values, num_times, reverse);

  AssertFatal(num_times == num_points, "Unexpected error: num_times disagrees with num_points.");
}

void afxPathData::onPerformSubstitutions()
{
  Parent::onPerformSubstitutions();
  update_derived_values();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
