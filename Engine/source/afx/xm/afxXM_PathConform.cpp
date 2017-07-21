
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

#include "math/mathIO.h"
#include "math/mathUtils.h"

#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/xm/afxXfmMod.h"
#include "afx/util/afxPath3D.h"
#include "afx/util/afxPath.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// PATH CONFORM

class afxPathData;

class afxXM_PathConformData : public afxXM_WeightedBaseData
{
  typedef afxXM_WeightedBaseData Parent;

public:
  StringTableEntry      paths_string;       // 
  Vector<afxPathData*>  pathDataBlocks;     // datablocks for paths
  Vector<U32>           pathDataBlockIds;   // datablock IDs which correspond to the pathDataBlocks
  F32                   path_mult;
  F32                   path_time_offset;
  bool                  orient_to_path;

public:
  /*C*/           afxXM_PathConformData();
  /*C*/           afxXM_PathConformData(const afxXM_PathConformData&, bool = false);

  bool            onAdd();
  bool            preload(bool server, String &errorStr);
  void            packData(BitStream* stream);
  void            unpackData(BitStream* stream);

  virtual bool    allowSubstitutions() const { return true; }

  static void     initPersistFields();

  afxXM_Base*     create(afxEffectWrapper* fx, bool on_server);

  DECLARE_CONOBJECT(afxXM_PathConformData);
  DECLARE_CATEGORY("AFX");
};

class afxPath3D;
class afxAnimCurve;

class afxXM_PathConform : public afxXM_WeightedBase
{
  typedef afxXM_WeightedBase Parent;

  afxXM_PathConformData*  db;

  Vector<afxPath3D*>      paths;
  Vector<F32>             path_mults;
  Vector<F32>             path_time_offsets;
  Vector<afxAnimCurve*>   rolls;

  void          init_paths(F32 lifetime);

public:
  /*C*/         afxXM_PathConform(afxXM_PathConformData*, afxEffectWrapper*);
  /*D*/         ~afxXM_PathConform();

  virtual void  start(F32 timestamp);
  virtual void  updateParams(F32 dt, F32 elapsed, afxXM_Params& params);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

IMPLEMENT_CO_DATABLOCK_V1(afxXM_PathConformData);

ConsoleDocClass( afxXM_PathConformData,
   "@brief An xmod datablock.\n\n"

   "@ingroup afxXMods\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxXM_PathConformData::afxXM_PathConformData()
{
  paths_string = ST_NULLSTRING;
  pathDataBlocks.clear();
  pathDataBlockIds.clear();
  path_mult = 1.0f;
  path_time_offset = 0.0f;
  orient_to_path = false;
}

afxXM_PathConformData::afxXM_PathConformData(const afxXM_PathConformData& other, bool temp_clone) : afxXM_WeightedBaseData(other, temp_clone)
{
  paths_string = other.paths_string;
  pathDataBlocks = other.pathDataBlocks;
  pathDataBlockIds = other.pathDataBlockIds;
  path_mult = other.path_mult;
  path_time_offset = other.path_time_offset;
  orient_to_path = other.orient_to_path;
}

void afxXM_PathConformData::initPersistFields()
{
  addField("paths",           TypeString,   Offset(paths_string, afxXM_PathConformData),
    "...");
  addField("pathMult",        TypeF32,      Offset(path_mult, afxXM_PathConformData),
    "...");
  addField("pathTimeOffset",  TypeF32,      Offset(path_time_offset, afxXM_PathConformData),
    "...");
  addField("orientToPath",    TypeBool,     Offset(orient_to_path, afxXM_PathConformData),
    "...");

  Parent::initPersistFields();

  // disallow some field substitutions
  disableFieldSubstitutions("paths");
}

void afxXM_PathConformData::packData(BitStream* stream)
{
  Parent::packData(stream);

  stream->write(pathDataBlockIds.size());
  for (int i = 0; i < pathDataBlockIds.size(); i++)
    stream->write(pathDataBlockIds[i]);
  stream->write(path_mult);
  stream->write(path_time_offset);
  stream->write(orient_to_path);
}

void afxXM_PathConformData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  U32 n_db;
  stream->read(&n_db);
  pathDataBlockIds.setSize(n_db);
  for (U32 i = 0; i < n_db; i++)
    stream->read(&pathDataBlockIds[i]);
  stream->read(&path_mult);
  stream->read(&path_time_offset);
  stream->read(&orient_to_path);
}

bool afxXM_PathConformData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  if (paths_string != ST_NULLSTRING) 
  {
     //Con::printf("afxEffectWrapperData: Path string found: %s", paths_string);
  }   
  
  if (paths_string != ST_NULLSTRING && paths_string[0] == '\0')
  {
     Con::warnf(ConsoleLogEntry::General, "afxEffectWrapperData(%s) empty paths string, invalid datablock", getName());
     return false;
  }

  if (paths_string != ST_NULLSTRING && dStrlen(paths_string) > 255) 
  {
     Con::errorf(ConsoleLogEntry::General, "afxEffectWrapperData(%s) paths string too long [> 255 chars]", getName());
     return false;
  }

  if (paths_string != ST_NULLSTRING) 
  {
    Vector<char*> dataBlocks(__FILE__, __LINE__);
    char* tokCopy = new char[dStrlen(paths_string) + 1];
    dStrcpy(tokCopy, paths_string);
    
    char* currTok = dStrtok(tokCopy, " \t");
    while (currTok != NULL) 
    {
      dataBlocks.push_back(currTok);
      currTok = dStrtok(NULL, " \t");
    }
    if (dataBlocks.size() == 0) 
    {
      Con::warnf(ConsoleLogEntry::General, "afxEffectWrapperData(%s) invalid paths string.  No datablocks found", getName());
      delete [] tokCopy;
      return false;
    }
    pathDataBlocks.clear();
    pathDataBlockIds.clear();
    
    for (U32 i = 0; i < dataBlocks.size(); i++) 
    {
      afxPathData* pData = NULL;
      if (Sim::findObject(dataBlocks[i], pData) == false) 
      {
        Con::warnf(ConsoleLogEntry::General, "afxEffectWrapperData(%s) unable to find path datablock: %s", getName(), dataBlocks[i]);
      } 
      else 
      {
        pathDataBlocks.push_back(pData);
        pathDataBlockIds.push_back(pData->getId());
      }
    }
    delete [] tokCopy;
    if (pathDataBlocks.size() == 0) 
    {
      Con::warnf(ConsoleLogEntry::General, "afxEffectWrapperData(%s) unable to find any path datablocks", getName());
      return false;
    }
  }


  return true;
}

bool afxXM_PathConformData::preload(bool server, String &errorStr)
{
  if (!Parent::preload(server, errorStr))
    return false;
  
  pathDataBlocks.clear();
  for (U32 i = 0; i < pathDataBlockIds.size(); i++) 
  {
    afxPathData* pData = NULL;
    if (Sim::findObject(pathDataBlockIds[i], pData) == false)
    {
      Con::warnf(ConsoleLogEntry::General, 
                 "afxEffectWrapperData(%s) unable to find path datablock: %d", 
                 getName(), pathDataBlockIds[i]);
    }
    else
      pathDataBlocks.push_back(pData);
  }

  return true;
}

afxXM_Base* afxXM_PathConformData::create(afxEffectWrapper* fx, bool on_server)
{
  afxXM_PathConformData* datablock = this;

  if (getSubstitutionCount() > 0)
  {
    datablock = new afxXM_PathConformData(*this, true);
    this->performSubstitutions(datablock, fx->getChoreographer(), fx->getGroupIndex());
  }

  return new afxXM_PathConform(datablock, fx);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxXM_PathConform::afxXM_PathConform(afxXM_PathConformData* db, afxEffectWrapper* fxw) 
 : afxXM_WeightedBase(db, fxw)
{ 
  this->db = db; 
}

afxXM_PathConform::~afxXM_PathConform()
{ 
  for (S32 i = 0; i < paths.size(); i++)
    if (paths[i])
      delete paths[i];

  for (S32 j = 0; j < paths.size(); j++)
    if (rolls[j])
      delete rolls[j];
}

void afxXM_PathConform::start(F32 timestamp)
{
  init_paths(fx_wrapper->getFullLifetime());
}

void afxXM_PathConform::init_paths(F32 lifetime)
{
  for( U32 i=0; i < db->pathDataBlocks.size(); i++ )
  {
    afxPathData* pd = db->pathDataBlocks[i];
    if (!pd)
      continue;

    if (pd->getSubstitutionCount() > 0)
    {
      afxPathData* orig_db = pd;
      pd = new afxPathData(*orig_db, true);
      orig_db->performSubstitutions(pd, fx_wrapper->getChoreographer(), fx_wrapper->getGroupIndex());
    }

    if (pd->num_points > 0)
    {
      if (pd->lifetime == 0) 
      {
        if (lifetime <= 0)
        {
          // Is this possible or is the lifetime always inherited properly???
        } 
        else 
        {
          pd->lifetime = lifetime;
        }
      }

      F32 pd_delay = pd->delay*time_factor;
      F32 pd_lifetime = pd->lifetime*time_factor;
      F32 pd_time_offset = pd->time_offset*time_factor;    

      afxPath3D* path = new afxPath3D();
      if (pd->times)
        path->buildPath( pd->num_points, pd->points, pd->times, pd_delay, time_factor );
      else
        path->buildPath( pd->num_points, pd->points, pd_delay, pd_delay+pd_lifetime ); 

      path->setLoopType( pd->loop_type );

      paths.push_back(path);
      // path->print();

      path_mults.push_back( db->path_mult * pd->mult );

      path_time_offsets.push_back( db->path_time_offset + pd_time_offset ); 

      if (pd->roll_string != ST_NULLSTRING && pd->rolls != NULL)
      {
        afxAnimCurve* roll_curve = new afxAnimCurve();
        for (U32 j=0; j<pd->num_points; j++ )
        {
          roll_curve->addKey( path->getPointTime(j), pd->rolls[j] );
        }
        roll_curve->sort();
        // roll_curve->print();

        rolls.push_back(roll_curve);
      }
      else 
      {
        rolls.push_back( NULL );
      }
    }
    else
    {
      Con::warnf("afxXM_PathConform::init_paths() -- paths datablock (%d) has no points.", i);
    }

    if (pd->isTempClone())
      delete pd;
  }
}

void afxXM_PathConform::updateParams(F32 dt, F32 elapsed, afxXM_Params& params)
{
  F32 wt_factor = calc_weight_factor(elapsed); 

  // compute path offset
  VectorF path_offset(0,0,0);  
  for( U32 i=0; i < paths.size(); i++ )
    path_offset += path_mults[i]*paths[i]->evaluateAtTime(elapsed + path_time_offsets[i])*wt_factor;

  params.ori.mulV(path_offset);
  params.pos += path_offset;

  if (db->orient_to_path && paths.size())
  {
    VectorF path_v = paths[0]->evaluateTangentAtTime(elapsed+path_time_offsets[0]);
    path_v.normalize();

    MatrixF mat(true);    

    if( rolls[0] )
    {
      F32 roll_angle = rolls[0]->evaluate(elapsed+path_time_offsets[0]);
      roll_angle = mDegToRad( roll_angle );
      //Con::printf( "roll: %f", roll_angle );
      
      Point3F roll_axis = path_v;
      AngAxisF rollRot(roll_axis, roll_angle);
      MatrixF roll_mat(true);
      rollRot.setMatrix(&roll_mat);
     
      mat.mul(roll_mat);
    }

    mat.mul(MathUtils::createOrientFromDir(path_v));
    params.ori.mul(mat);
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
