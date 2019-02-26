
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

#ifndef _AFX_FORCE_H_
#define _AFX_FORCE_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxForce Data
class afxForceDesc;

class afxForceData : public GameBaseData
{
  typedef GameBaseData Parent;

public:
  StringTableEntry  force_set_name;
  afxForceDesc*     force_desc;

public:
  /*C*/         afxForceData();
  /*C*/         afxForceData(const afxForceData&, bool = false);

  virtual bool  onAdd();
  virtual void  packData(BitStream* stream);
  virtual void  unpackData(BitStream* stream);

  virtual bool  allowSubstitutions() const { return true; }
  virtual afxForceData* cloneAndPerformSubstitutions(const SimObject*, S32 index=0)=0;

  static void   initPersistFields();
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxForce

class afxForce
{
  afxForceData*         datablock;

protected:
  F32                   fade_amt;

public:
  /*C*/                 afxForce();
  /*D*/                 ~afxForce();

  virtual bool          onNewDataBlock(afxForceData* dptr, bool reload);
  void                  setFadeAmount(F32 amt) { fade_amt = amt; }

  virtual void          start() {};
  virtual void          update(F32 dt) {};
  virtual Point3F       evaluate(Point3F pos, Point3F v, F32 mass) { return Point3F(0,0,0); }; //=0;
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxForceDesc
{
private:
  static Vector<afxForceDesc*>* forces;

public:
  /*C*/         afxForceDesc();

  virtual bool  testForceType(const SimDataBlock*) const=0;

  virtual afxForce* create() const=0;

  static bool   identifyForce(afxForceData*);
};


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_FORCE_H_
